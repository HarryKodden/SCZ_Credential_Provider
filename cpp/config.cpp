#include <tchar.h>

#include "config.h"
#include "helpers.h"
#include "data.h"

DWORD readRegistryValueString(__in LPCSTR value, __in LPCSTR key, __in int buffer_size, __deref_out_opt char* data) 
{
	//char lszValue[1024];
	HKEY hKey;
	LONG returnStatus;
	DWORD dwType = REG_SZ;
	DWORD dwSize = 0;

	char baseKey[sizeof(REGISTRY_BASE_KEY) / sizeof(char) + 100];
	ZeroMemory(baseKey, sizeof(baseKey));

	strncat_s(baseKey, sizeof(baseKey) / sizeof(char), REGISTRY_BASE_KEY, sizeof(REGISTRY_BASE_KEY));

	if (key && key[0])
	{
		strncat_s(baseKey, sizeof(baseKey) / sizeof(char), "\\", sizeof("\\"));
		strncat_s(baseKey, sizeof(baseKey) / sizeof(char), key, strlen(key));
	}

	returnStatus = RegOpenKeyExA(HKEY_LOCAL_MACHINE, baseKey, NULL, KEY_QUERY_VALUE, &hKey);
	if (returnStatus == ERROR_SUCCESS)
	{
		dwSize = buffer_size;

		returnStatus = RegQueryValueExA(hKey, value, NULL, &dwType, (LPBYTE)data, &dwSize);
		if (returnStatus != ERROR_SUCCESS)
		{
			dwSize = 0;
		}

		RegCloseKey(hKey);
	}

	return dwSize;
}

DWORD readRegistryValueString(__in CONF_VALUE conf_value, __in int buffer_size, __deref_out_opt char* data) 
{
	LPCSTR confValueName = s_CONF_VALUES[conf_value];
	return readRegistryValueString(confValueName, NULL, buffer_size, data);
}

DWORD readRegistryValueInteger(__in LPCSTR value, __in LPCSTR key, __deref_out_opt int* data) 
{
	DWORD lszValue;
	HKEY hKey;
	LONG returnStatus;
	DWORD dwType = REG_DWORD;
	DWORD dwSize = 0;

	char baseKey[sizeof(REGISTRY_BASE_KEY) / sizeof(char) + 100];
	ZeroMemory(baseKey, sizeof(baseKey));

	strncat_s(baseKey, sizeof(baseKey) / sizeof(char), REGISTRY_BASE_KEY, sizeof(REGISTRY_BASE_KEY));

	if (key && key[0])
	{
		strncat_s(baseKey, sizeof(baseKey) / sizeof(char), "\\", sizeof("\\"));
		strncat_s(baseKey, sizeof(baseKey) / sizeof(char), key, strlen(key));
	}

	returnStatus = RegOpenKeyExA(HKEY_LOCAL_MACHINE, baseKey, NULL, KEY_QUERY_VALUE, &hKey);
	if (returnStatus == ERROR_SUCCESS)
	{
		dwSize = sizeof(DWORD);

		returnStatus = RegQueryValueExA(hKey, value, NULL, &dwType, reinterpret_cast<LPBYTE>(&lszValue), &dwSize);
		if (returnStatus == ERROR_SUCCESS)
		{
			*data = lszValue;
		}
		else
		{
			dwSize = 0;
		}

		RegCloseKey(hKey);
	}

	return dwSize;
}

DWORD readRegistryValueInteger(__in CONF_VALUE conf_value, __deref_out_opt int* data) 
{
	LPCSTR confValueName = s_CONF_VALUES[conf_value];
	return readRegistryValueInteger(confValueName, NULL, data);
}


DWORD readRegistryValueBinary(__in LPCSTR value, __in LPCSTR key, __in int buffer_size, __deref_out_opt unsigned char* data)
{
	//char lszValue[1024];
	HKEY hKey;
	LONG returnStatus;
	DWORD dwType = REG_BINARY;
	DWORD dwSize = 0;

	char baseKey[sizeof(REGISTRY_BASE_KEY) / sizeof(char) + 100];
	ZeroMemory(baseKey, sizeof(baseKey));

	strncat_s(baseKey, sizeof(baseKey) / sizeof(char), REGISTRY_BASE_KEY, sizeof(REGISTRY_BASE_KEY));

	if (key && key[0])
	{
		strncat_s(baseKey, sizeof(baseKey) / sizeof(char), "\\", sizeof("\\"));
		strncat_s(baseKey, sizeof(baseKey) / sizeof(char), key, strlen(key));
	}

	returnStatus = RegOpenKeyExA(HKEY_LOCAL_MACHINE, baseKey, NULL, KEY_QUERY_VALUE, &hKey);
	if (returnStatus == ERROR_SUCCESS)
	{
		dwSize = buffer_size;

		returnStatus = RegQueryValueExA(hKey, value, NULL, &dwType, data, &dwSize);
		if (returnStatus != ERROR_SUCCESS)
		{
			dwSize = 0;
		}

		RegCloseKey(hKey);
	}

	return dwSize;
}

DWORD writeRegistryValueString(__in LPCSTR value, __in LPCSTR key, __in char* data, __in int buffer_size)
{
	HKEY hKey;
	LONG returnStatus;
	DWORD dwType = REG_SZ;

	returnStatus = RegCreateKeyExA(HKEY_LOCAL_MACHINE, REGISTRY_BASE_KEY, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
	if (returnStatus == ERROR_SUCCESS)
	{
		returnStatus = RegSetKeyValueA(hKey, key, value, dwType, data, buffer_size);
		RegCloseKey(hKey);
	}

	return returnStatus;
}

DWORD writeRegistryValueString(__in CONF_VALUE conf_value, __in char* data, __in int buffer_size)
{
	LPCSTR confValueName = s_CONF_VALUES[conf_value];
	return writeRegistryValueString(confValueName, NULL, data, buffer_size);
}

DWORD writeRegistryValueInteger(__in LPCSTR value, __in LPCSTR key, __in int data)
{
	HKEY hKey;
	LONG returnStatus;
	DWORD dwType = REG_DWORD;

	returnStatus = RegCreateKeyExA(HKEY_LOCAL_MACHINE, REGISTRY_BASE_KEY, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
	if (returnStatus == ERROR_SUCCESS)
	{
		returnStatus = RegSetKeyValueA(hKey, key, value, dwType, &data, sizeof(int));
		RegCloseKey(hKey);
	}

	return returnStatus;
}

DWORD writeRegistryValueInteger(__in CONF_VALUE conf_value, __in int data)
{
	LPCSTR confValueName = s_CONF_VALUES[conf_value];
	return writeRegistryValueInteger(confValueName, NULL, data);
}

DWORD writeRegistryValueBinary(__in LPCSTR value, __in LPCSTR key, __in unsigned char* data, __in int buffer_size)
{
	HKEY hKey;
	LONG returnStatus;
	DWORD dwType = REG_BINARY;

	returnStatus = RegCreateKeyExA(HKEY_LOCAL_MACHINE, REGISTRY_BASE_KEY, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
	if (returnStatus == ERROR_SUCCESS)
	{
		returnStatus = RegSetKeyValueA(hKey, key, value, dwType, data, buffer_size);
		RegCloseKey(hKey);
	}

	return returnStatus;
}

namespace Configuration
{

	CONFIGURATION*& Get()
	{
		static struct CONFIGURATION *conf = NULL;

		return conf;
	}

	void Default()
	{
		struct CONFIGURATION*& conf = Get();

		ZERO(conf->hostname);
		ZERO(conf->path);
		ZERO(conf->login_text);
		ZERO(conf->otp_text);
		ZERO(conf->v1_bitmap_path);
		ZERO(conf->v2_bitmap_path);

		conf->win_ver_major = 0;
		conf->win_ver_minor = 0;
	}

	void Init()
	{
		debug(__FUNCTION__);

		struct CONFIGURATION*& conf = Get();

		conf = (struct CONFIGURATION*) malloc(sizeof(struct CONFIGURATION));

		Default();
	}

	void Deinit()
	{
		debug(__FUNCTION__);

		struct CONFIGURATION*& conf = Get();

		Default();

		free(conf);
		conf = NULL;
	}

	void Read()
	{
		debug(__FUNCTION__);

		struct CONFIGURATION*& conf = Get();

		// Read config
		readRegistryValueString(CONF_HOSTNAME, sizeof(conf->hostname), conf->hostname);

		if (readRegistryValueString(CONF_PATH, sizeof(conf->path), conf->path) <= 1) // 1 = size of a char NULL-terminator in byte
			strcpy_s(conf->path, sizeof(conf->path), CONFIG_DEFAULT_EMPTY_PATH);

		if (readRegistryValueString(CONF_LOGIN_TEXT, sizeof(conf->login_text), conf->login_text) <= 1) // 1 = size of a char NULL-terminator in byte
			strcpy_s(conf->login_text, sizeof(conf->login_text), CONFIG_DEFAULT_LOGIN_TEXT);

		if (readRegistryValueString(CONF_OTP_TEXT, sizeof(conf->otp_text), conf->otp_text) <= 1) // 1 = size of a char NULL-terminator in byte
			strcpy_s(conf->otp_text, sizeof(conf->otp_text), CONFIG_DEFAULT_OTP_TEXT);

		readRegistryValueString(CONF_V1_BITMAP_PATH, sizeof(conf->v1_bitmap_path), conf->v1_bitmap_path);
		readRegistryValueString(CONF_V2_BITMAP_PATH, sizeof(conf->v2_bitmap_path), conf->v2_bitmap_path);

		// Read and save the windows version
		OSVERSIONINFOEX info;
		ZeroMemory(&info, sizeof(OSVERSIONINFOEX));
		info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		#pragma warning( disable : 4996 )
		GetVersionEx((LPOSVERSIONINFO)&info);
		conf->win_ver_major = info.dwMajorVersion;
		conf->win_ver_minor = info.dwMinorVersion;
	}

	wstring Configuration::getRegistry(wstring name)
	{
		DWORD dwRet;
		HKEY hKey;

		dwRet = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,
			_T(REGISTRY_BASE_KEY),
			NULL,
			KEY_QUERY_VALUE,
			&hKey);
		if (dwRet != ERROR_SUCCESS)
		{
			return L"";
		}

		const DWORD SIZE = 1024;
		TCHAR szValue[SIZE] = _T("");
		DWORD dwValue = SIZE;
		DWORD dwType = 0;
		dwRet = RegQueryValueEx(
			hKey,
			name.c_str(),
			NULL,
			&dwType,
			(LPBYTE)&szValue,
			&dwValue);
		if (dwRet != ERROR_SUCCESS)
		{
			return L"";
		}

		if (dwType != REG_SZ)
		{
			return L"";
		}
		RegCloseKey(hKey);
		hKey = NULL;
		return wstring(szValue);
	}

	void PrintConfig() {
		debug("SCZ_CP Version: %s\n", VER_FILE_VERSION_STR);
		debug("Windows Version Major: %d, Minor: %d\n", Get()->win_ver_major, Get()->win_ver_minor);
		debug("Hostname: %s\n", Get()->hostname);
		debug("Path: %S\n", Get()->path);
		debug("Login text: %S\n", Get()->login_text);
		debug("OTP text: %S\n", Get()->otp_text);
	}

	DWORD SaveValueString(CONF_VALUE conf_value, char* value, int size)
	{
		debug(__FUNCTION__);

		return writeRegistryValueString(conf_value, value, size);
	}

	DWORD SaveValueInteger(CONF_VALUE conf_value, int value)
	{
		debug(__FUNCTION__);

		return writeRegistryValueInteger(conf_value, value);
	}

} // Namespace Configuration