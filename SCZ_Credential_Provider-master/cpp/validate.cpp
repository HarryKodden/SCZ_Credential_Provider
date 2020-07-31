#include <stdio.h>
#include <string.h>
#include <string>
#include <curl/curl.h>
#include <tchar.h>

#include "debug.h"
#include "config.h"
#include "data.h"
#include "validate.h"
#include "helpers.h"

#include "document.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "wldap32.lib")
#pragma comment(lib, "normaliz.lib")

using std::string;
using std::wstring;

/* holder for curl fetch */

struct curl_fetch_st {
    char* payload;
    size_t size;
};

/* callback for curl fetch */

size_t curl_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;                             /* calculate buffer size */
    struct curl_fetch_st* p = (struct curl_fetch_st*) userp;   /* cast pointer to fetch struct */
    /* expand buffer */

    p->payload = (char*)realloc(p->payload, p->size + realsize + 1);

    /* check buffer */
    if (p->payload == NULL) {
        /* this isn't good */

        debug("ERROR: Failed to expand buffer in curl_callback");

        /* free buffer */
        free(p->payload);

        /* return */
        return (size_t) -1;
    }

    /* copy contents to buffer */
    memcpy(&(p->payload[p->size]), contents, realsize);

    /* set new buffer size */
    p->size += realsize;

    /* ensure null termination */
    p->payload[p->size] = 0;

    /* return size */
    return realsize;
}

/*

Its simple.
1: install windows SDK, VS with VC,
2: download latest curl release
3: Win+R cmd
4: cd to to [your vs installation]\VC\bin[amd64 or x86_amd64], use the folder according to the type of build you want (i used amd64 on a 64bit machine to use curl dll on my 64 win 8.1
5: run vcvars64 or vcvars32 accordingly
6: cd to [extracted CURL dir]\winbuild
7: run:
nmake /f makefile.vc ENABLE_WINSSL=yes mode=dll MACHINE=x64 VC=13 use x64 and VC version according to your installation. e.g. VS 2013 is 13

It will take some time and build a dll in \CURL\builds\libcurl-vc13-x64-release-dll-ipv6-sspi-winssl or \CURL\builds\libcurl-vc13-x86-release-dll-ipv6-sspi-winssl

Sample:

- "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
- cd curl-git-location\winbuild
- nmake /f makefile.vc ENABLE_WINSSL=yes mode=dll MACHINE=x64 VC=19

x86
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars32.bat"
NMAKE /F Makefile.vc ENABLE_WINSSL=yes mode=static MACHINE=x86 VC=15
x64
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
NMAKE /F Makefile.vc ENABLE_WINSSL=yes mode=static MACHINE=x64 VC=15

*/

string ws_to_s(wstring in) {
    string out(in.begin(), in.end());
    return out;
}

PWSTR s_to_ws(const char *a) {
    PWSTR b;
    int count = 0;

    count = MultiByteToWideChar(CP_ACP, 0, a , strlen(a), NULL , 0);
    if(count > 0) { 
      b = SysAllocStringLen(0, count);
      MultiByteToWideChar(CP_ACP, 0, a , strlen(a), b , count);
    } else {
        b = _wcsdup(L"");
    }

    return b;
}

/* 
int main(){
    char* a = "hello world!";
    PWSTR b;
    char* c;
    int count = 0;
    count = MultiByteToWideChar(CP_ACP, 0, a , strlen(a), NULL , 0);
    if(count > 0) { 
      b = SysAllocStringLen(0, count);
      MultiByteToWideChar(CP_ACP, 0, a , strlen(a), b , count);
    }
   count = WideCharToMultiByte(CP_ACP, 0, b, count, 0, 0, NULL, NULL);
   if(count > 0) {
      c = new char[count + 1];
      WideCharToMultiByte(CP_ACP, 0, b, count, c,  count + 1 , NULL, NULL);
      c[count] = '\0';
   }
    //MessageBox(NULL, b, TEXT("test") , 0); // Unicode
    MessageBox(NULL, c, TEXT("test") , 0);   // MBCS
    SysFreeString(b);
    return 0;
}
*/

#define URL_LEN 1024
#define DATA_LEN 1024

HRESULT token_validate()
{
    HRESULT hr = E_UNEXPECTED;
    CURL* curl;
    CURLcode rc;
    char url[URL_LEN];
    char data[DATA_LEN];
    struct curl_fetch_st curl_fetch;
    struct curl_fetch_st* fetch = &curl_fetch;
    
    // Validate required data elements...
    if (EMPTY(Data::Gui::Get()->user_name)) {
        debug("Error: No username !\n");
        return E_FAIL;
    }
    
    if (EMPTY(Data::Gui::Get()->token)) {
        debug("Error: No token !\n");
        return E_FAIL;
    }
    
    // Prepare full url...
    sprintf_s((char *)url, URL_LEN, "https://%s", Configuration::Get()->hostname);

    if (!Configuration::Get()->path || *(Configuration::Get()->path) != '/') {
        strcat_s(url, URL_LEN, "/");
    }

    strcat_s(url, URL_LEN, Configuration::Get()->path);

    debug("URL: %s\n", url);

    // Prepare Headers...
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");
  
    // Prepare input data...
    sprintf_s((char *)data, DATA_LEN, "{\"user\":\"%S\",\"token\":\"%S\"}",
        Data::Gui::Get()->user_name,
        Data::Gui::Get()->token
    );

    debug("JSON: %s\n", data);

    // Prepare payload for response...
    fetch->payload = (char*)calloc(1, sizeof(fetch->payload));
    fetch->size = 0;

    // Prepare API request...
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)fetch);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "CredentialProvider");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data, strlen(data));
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(data));

        /* Perform the request */
        if ((rc = curl_easy_perform(curl) ) == CURLE_OK) {
            hr = S_OK;
        }
        else {
            debug("curl_easy_perform() failed: %s\n",
                curl_easy_strerror(rc));
        }
            
        /* check payload */
         if (fetch->payload != NULL) {
            using namespace rapidjson;

            Document document;

            debug("CURL Returned: \n%s\n", fetch->payload);

            document.Parse(fetch->payload);
            if (document.HasMember("username") && document.HasMember("password") && document.HasMember("domain")) {
                const char *username = document["username"].GetString();
                const char *password = document["password"].GetString();
                const char *domain = document["domain"].GetString();
                
                Data::Credential::Get()->user_name = s_to_ws(username);
                Data::Credential::Get()->ldap_password = s_to_ws(password);
                Data::Credential::Get()->domain_name = s_to_ws(domain);

                debug("Credentials saved: User: %S at Domain: %S", 
                    Data::Credential::Get()->user_name, 
                    Data::Credential::Get()->domain_name
                );
            }

             free(fetch->payload);
         }
        
        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    return hr;
}
