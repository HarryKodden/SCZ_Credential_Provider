//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#include <windows.h>
#include <strsafe.h>
#include <new>

#include "helpers.h"
#include "config.h"
#include "scz_credential.h"

enum SERIALIZATION_AVAILABLE_FOR { 
    SAF_USERNAME, 
    SAF_PASSWORD, 
    SAF_DOMAIN 
};

class SCZ_Provider :    public ICredentialProvider
{
  public:
    // IUnknown
    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return ++_cRef;
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        long cRef = --_cRef;
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    IFACEMETHODIMP QueryInterface(_In_ REFIID riid, _COM_Outptr_ void **ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(SCZ_Provider, ICredentialProvider), // IID_ICredentialProvider
            {0},
        };
        return QISearch(this, qit, riid, ppv);
    }

  public:
    IFACEMETHODIMP SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags);
    IFACEMETHODIMP SetSerialization(_In_ CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION const *pcpcs);

    IFACEMETHODIMP Advise(_In_ ICredentialProviderEvents *pcpe, _In_ UINT_PTR upAdviseContext);
    IFACEMETHODIMP UnAdvise();

    IFACEMETHODIMP GetFieldDescriptorCount(_Out_ DWORD *pdwCount);
    IFACEMETHODIMP GetFieldDescriptorAt(DWORD dwIndex,  _Outptr_result_nullonfailure_ CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR **ppcpfd);

    IFACEMETHODIMP GetCredentialCount(_Out_ DWORD *pdwCount,
                                      _Out_ DWORD *pdwDefault,
                                      _Out_ BOOL *pbAutoLogonWithDefault);
    IFACEMETHODIMP GetCredentialAt(DWORD dwIndex,
                                   _Outptr_result_nullonfailure_ ICredentialProviderCredential **ppcpc);

    friend HRESULT Provider_CreateInstance(_In_ REFIID riid, _Outptr_ void** ppv);

  protected:
    SCZ_Provider();
    __override ~SCZ_Provider();

  private:
    HRESULT _EnumerateOneCredential(__in DWORD dwCredientialIndex, __in PCWSTR pwzUsername);
    void _ReleaseEnumeratedCredentials();
    void _CleanupSetSerialization();

    void _GetSerializedCredentials(PWSTR *username, PWSTR *password, PWSTR *domain);
    bool _SerializationAvailable(SERIALIZATION_AVAILABLE_FOR checkFor);

private:
    long                                    _cRef;              // Used for reference counting.
    SCZ_Credential                           *_pCredential;     // SCZ_Credential reference
    bool                                    _fRecreateEnumeratedCredentials;
    KERB_INTERACTIVE_UNLOCK_LOGON*          _pkiulSetSerialization;
    DWORD                                   _dwSetSerializationCred; //index into rgpCredentials for the SetSerializationCred
    bool                                    _bAutoSubmitSetSerializationCred;
};
