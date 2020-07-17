//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// SCZ_Provider implements ICredentialProvider, which is the main
// interface that logonUI uses to decide which tiles to display.
// In this sample, we will display one tile that uses each of the nine
// available UI controls.
#include <initguid.h>
#include <windows.h>
#include <strsafe.h>
#include <Wtsapi32.h>                       
#include <Lm.h>
#include <tchar.h>

#include "scz_provider.h"
#include "scz_credential.h"
#include "config.h"
#include "scenarios.h"
#include "helpers.h"
#include "debug.h"
#include "guid.h"
#include "scenarios.h"
#include "data.h"

SCZ_Provider::SCZ_Provider():
    _cRef(1),
    _pkiulSetSerialization(nullptr),
    _pCredential(nullptr),
    _bAutoSubmitSetSerializationCred(false),
    _dwSetSerializationCred(CREDENTIAL_PROVIDER_NO_DEFAULT)
{
    debug("SCZ_Provider Constructor !\n");

    // Initialize config
    Configuration::Init();
    Configuration::Read();
    Data::Provider::Init();

    DllAddRef();
}

SCZ_Provider::~SCZ_Provider()
{
    debug("SCZ_Provider Destructor !\n");

    if (_pCredential != nullptr)
    {
        _pCredential->Release();
        _pCredential = nullptr;
    }

    Data::Provider::Deinit();

    DllRelease();
}

void SCZ_Provider::_CleanupSetSerialization()
{
    debug("SCZ_Provider _CleanupSetSerialization\n");

    if (_pkiulSetSerialization)
    {
        KERB_INTERACTIVE_LOGON* pkil = &_pkiulSetSerialization->Logon;
        SecureZeroMemory(_pkiulSetSerialization,
            sizeof(*_pkiulSetSerialization) +
            pkil->LogonDomainName.MaximumLength +
            pkil->UserName.MaximumLength +
            pkil->Password.MaximumLength);
        HeapFree(GetProcessHeap(), 0, _pkiulSetSerialization);
    }
}

// SetUsageScenario is the provider's cue that it's going to be asked for tiles
// in a subsequent call.
HRESULT SCZ_Provider::SetUsageScenario(
    CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
    DWORD dwFlags)
{
    HRESULT hr;

    debug("SCZ_Provider SetUsageScenario\n");

    Configuration::PrintConfig();

    Data::Provider::Get()->credPackFlags = dwFlags;
    Data::Provider::Get()->usage_scenario = cpus;

    // Decide which scenarios to support here. Returning E_NOTIMPL simply tells the caller
    // that we're not designed for that scenario.
    switch (cpus)
    {
    case CPUS_LOGON:
    case CPUS_UNLOCK_WORKSTATION:
        // The reason why we need _fRecreateEnumeratedCredentials is because ICredentialProviderSetUserArray::SetUserArray() is called after ICredentialProvider::SetUsageScenario(),
        // while we need the ICredentialProviderUserArray during enumeration in ICredentialProvider::GetCredentialCount()
        hr = S_OK;
        break;

    case CPUS_CREDUI:
        hr = S_OK;
        break;

    case CPUS_CHANGE_PASSWORD:
    case CPUS_PLAP:
        hr = E_NOTIMPL;
        break;

    default:
        hr = E_INVALIDARG;
        break;
    }

    return hr;
}

// SetSerialization takes the kind of buffer that you would normally return to LogonUI for
// an authentication attempt.  It's the opposite of ICredentialProviderCredential::GetSerialization.
// GetSerialization is implement by a credential and serializes that credential.  Instead,
// SetSerialization takes the serialization and uses it to create a tile.
//
// SetSerialization is called for two main scenarios.  The first scenario is in the credui case
// where it is prepopulating a tile with credentials that the user chose to store in the OS.
// The second situation is in a remote logon case where the remote client may wish to
// prepopulate a tile with a username, or in some cases, completely populate the tile and
// use it to logon without showing any UI.
//
// If you wish to see an example of SetSerialization, please see either the SampleCredentialProvider
// sample or the SampleCredUICredentialProvider sample.  [The logonUI team says, "The original sample that
// this was built on top of didn't have SetSerialization.  And when we decided SetSerialization was
// important enough to have in the sample, it ended up being a non-trivial amount of work to integrate
// it into the main sample.  We felt it was more important to get these samples out to you quickly than to
// hold them in order to do the work to integrate the SetSerialization changes from SampleCredentialProvider
// into this sample.]
HRESULT SCZ_Provider::SetSerialization(
    _In_ CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION const *pcpcs)
{

    HRESULT result = E_NOTIMPL;
    ULONG authPackage = NULL;

    result = RetrieveNegotiateAuthPackage(&authPackage);

    debug(__FUNCTION__);

    if (!SUCCEEDED(result))
    {
        debug("Failed to retrieve authPackage\n");
        return result;
    }

    if (Data::Provider::Get()->usage_scenario == CPUS_CREDUI)
    {
        debug("CPUS_CREDUI\n");

        if (((Data::Provider::Get()->credPackFlags & CREDUIWIN_IN_CRED_ONLY) || (Data::Provider::Get()->credPackFlags & CREDUIWIN_AUTHPACKAGE_ONLY))
            && authPackage != pcpcs->ulAuthenticationPackage)
        {
            debug("authPackage invalid\n");
            return E_INVALIDARG;
        }

        if (Data::Provider::Get()->credPackFlags & CREDUIWIN_AUTHPACKAGE_ONLY)
        {
            debug("CPUS_CREDUI but not CREDUIWIN_AUTHPACKAGE_ONLY\n");
            result = S_FALSE;
        }
    }

    if (authPackage == pcpcs->ulAuthenticationPackage && pcpcs->cbSerialization > 0 && pcpcs->rgbSerialization)
    {
        KERB_INTERACTIVE_UNLOCK_LOGON* pkil = (KERB_INTERACTIVE_UNLOCK_LOGON*)pcpcs->rgbSerialization;
        if (pkil->Logon.MessageType == KerbInteractiveLogon)
        {
            if (pkil->Logon.UserName.Length && pkil->Logon.UserName.Buffer)
            {
                BYTE * nativeSerialization = NULL;
                DWORD nativeSerializationSize = 0;

                debug("Serialization found from remote\n");

                if (Data::Provider::Get()->credPackFlags == CPUS_CREDUI && (Data::Provider::Get()->credPackFlags & CREDUIWIN_PACK_32_WOW))
                {
                    if (!SUCCEEDED(KerbInteractiveUnlockLogonRepackNative(pcpcs->rgbSerialization, pcpcs->cbSerialization,
                        &nativeSerialization, &nativeSerializationSize)))
                    {
                        return result;
                    }
                }
                else
                {
                    nativeSerialization = (BYTE*)LocalAlloc(LMEM_ZEROINIT, pcpcs->cbSerialization);
                    nativeSerializationSize = pcpcs->cbSerialization;

                    if (!nativeSerialization)
                        return E_OUTOFMEMORY;

                    CopyMemory(nativeSerialization, pcpcs->rgbSerialization, pcpcs->cbSerialization);
                }

                KerbInteractiveUnlockLogonUnpackInPlace((KERB_INTERACTIVE_UNLOCK_LOGON *)nativeSerialization, nativeSerializationSize);

                if (_pkiulSetSerialization)
                    LocalFree(_pkiulSetSerialization);

                _pkiulSetSerialization = (KERB_INTERACTIVE_UNLOCK_LOGON *)nativeSerialization;

                result = S_OK;
            }
        }
    }

    return result;
}

// Called by LogonUI to give you a callback.  Providers often use the callback if they
// some event would cause them to need to change the set of tiles that they enumerated.
HRESULT SCZ_Provider::Advise(
    _In_ ICredentialProviderEvents * pcpe,
    _In_ UINT_PTR upAdviseContext)
{
    debug("SCZ_Provider Advise\n");

    if (Data::Provider::Get()->_pcpe != NULL)
    {
        Data::Provider::Get()->_pcpe->Release();
    }

    Data::Provider::Get()->_pcpe = pcpe;
    Data::Provider::Get()->_pcpe->AddRef();

    Data::Provider::Get()->_upAdviseContext = upAdviseContext;

    return S_OK;
}

// Called by LogonUI when the ICredentialProviderEvents callback is no longer valid.
HRESULT SCZ_Provider::UnAdvise()
{
    debug("SCZ_Provider UnAdvise (not implemented...)\n");

    if (Data::Provider::Get()->_pcpe != NULL)
    {
        Data::Provider::Get()->_pcpe->Release();
    }

    Data::Provider::Get()->_pcpe = NULL;
    Data::Provider::Get()->_upAdviseContext = NULL;

    return S_OK;
}

// Called by LogonUI to determine the number of fields in your tiles.  This
// does mean that all your tiles must have the same number of fields.
// This number must include both visible and invisible fields. If you want a tile
// to have different fields from the other tiles you enumerate for a given usage
// scenario you must include them all in this count and then hide/show them as desired
// using the field descriptors.
HRESULT SCZ_Provider::GetFieldDescriptorCount(
    _Out_ DWORD *pdwCount)
{
    *pdwCount = scenarios::GetCurrentNumFields();

    debug("SCZ_Provider GetFieldDescriptorCount (%d)\n", *pdwCount);

    return S_OK;
}

// Gets the field descriptor for a particular field.
HRESULT SCZ_Provider::GetFieldDescriptorAt(
    DWORD dwIndex,
    _Outptr_result_nullonfailure_ CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR **ppcpfd)
{
    HRESULT hr;

    debug("SCZ_Provider GetFieldDescriptorAt: %d\n", dwIndex);

    // Verify dwIndex is a valid field.
    if ((dwIndex <  scenarios::GetCurrentNumFields()) && ppcpfd)
    {
        hr = FieldDescriptorCoAllocCopy(s_rgCredProvFieldDescriptorsFor[Data::Provider::Get()->usage_scenario][dwIndex], ppcpfd);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

// Sets pdwCount to the number of tiles that we wish to show at this time.
// Sets pdwDefault to the index of the tile which should be used as the default.
// The default tile is the tile which will be shown in the zoomed view by default. If
// more than one provider specifies a default the last used cred prov gets to pick
// the default. If *pbAutoLogonWithDefault is TRUE, LogonUI will immediately call
// GetSerialization on the credential you've specified as the default and will submit
// that credential for authentication without showing any further UI.
HRESULT SCZ_Provider::GetCredentialCount(
    _Out_ DWORD *pdwCount,
    _Out_ DWORD *pdwDefault,
    _Out_ BOOL *pbAutoLogonWithDefault)
{
    HRESULT hr = S_OK;

    debug("SCZ_Provider GetCredentialCount\n");

    *pdwCount = 1; //_dwNumCreds;
    *pdwDefault = 0; // this means we want to be the default
    *pbAutoLogonWithDefault = FALSE;

    return hr;
}

// Returns the credential at the index specified by dwIndex. This function is called by logonUI to enumerate
// the tiles.
HRESULT SCZ_Provider::GetCredentialAt(
    DWORD dwIndex,
    _Outptr_result_nullonfailure_ ICredentialProviderCredential **ppcpc)
{
    HRESULT hr = E_FAIL;
    *ppcpc = nullptr;

    debug("SCZ_Provider GetCredentialAt: %d\n", dwIndex);

//    if ((dwIndex == 0) && ppcpc)
//    {
//        hr = _pCredential->QueryInterface(IID_PPV_ARGS(ppcpc));
//    }
//

    if (!_pCredential)
    {
        debug("Checking for serialized credentials\n");

        PWSTR serializedUser, serializedPass, serializedDomain;

        _GetSerializedCredentials(&serializedUser, &serializedPass, &serializedDomain);

        debug("Checking for missing credentials\n");

        if (Data::Provider::Get()->usage_scenario == CPUS_UNLOCK_WORKSTATION && serializedUser == NULL)
        {
            if (serializedUser == NULL)
            {
                debug("Looking-up missing user name from session\n");

                DWORD dwLen;

                if (!WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
                    WTS_CURRENT_SESSION,
                    WTSUserName,
                    &serializedUser,
                    &dwLen))
                {
                    serializedUser = NULL;
                }
            }

            if (serializedDomain == NULL)
            {
                debug("Looking-up missing domain name from session\n");

                DWORD dwLen;

                if (!WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
                    WTS_CURRENT_SESSION,
                    WTSDomainName,
                    &serializedDomain,
                    &dwLen))
                {
                    serializedDomain = NULL;
                }
            }
        }
        else if (Data::Provider::Get()->usage_scenario == CPUS_LOGON || Data::Provider::Get()->usage_scenario == CPUS_CREDUI)
        {
            if (serializedDomain == NULL)
            {
                debug("Looking-up missing domain name from computer\n");

                NETSETUP_JOIN_STATUS join_status;

                if (!NetGetJoinInformation(
                    NULL,
                    &serializedDomain,
                    &join_status) == NERR_Success || join_status == NetSetupUnjoined || join_status == NetSetupUnknownStatus)
                {
                    serializedDomain = NULL;
                }
                debug("Found domain: %S", serializedDomain);
            }
        }

        debug("Initializing SCZ_Credential\n");

        _pCredential = new SCZ_Credential();
        hr = _pCredential->Initialize(
            s_rgCredProvFieldDescriptorsFor[Data::Provider::Get()->usage_scenario],
            scenarios::GetFieldStatePairFor(Data::Provider::Get()->usage_scenario), 
            serializedUser, serializedDomain, serializedPass);
    }
    else
    {
        hr = S_OK;
    }

    debug("Checking for successful initialization\n");

    if (FAILED(hr))
    {
        debug("Initialization failed\n");
        return hr;
    }

    debug("Checking for successful instantiation\n");

    if (!_pCredential)
    {
        debug("Instantiation failed\n");
        return E_OUTOFMEMORY;
    }

    debug("Returning interface to credential\n");

    // Validate parameters.
    //if((dwIndex < _dwNumCreds) && ppcpc)
    if ((dwIndex == 0) && ppcpc)
    {
        if (Data::Provider::Get()->usage_scenario == CPUS_CREDUI)
        {
            debug("CredUI: returning an IID_ICredentialProviderCredential\n");
            hr = _pCredential->QueryInterface(IID_ICredentialProviderCredential, reinterpret_cast<void **>(ppcpc));
        }
        else
        {
            debug("Non-CredUI: returning an IID_IConnectableCredentialProviderCredential\n");
            hr = _pCredential->QueryInterface(IID_IConnectableCredentialProviderCredential, reinterpret_cast<void **>(ppcpc));
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

// Boilerplate code to create our provider.
HRESULT Provider_CreateInstance(_In_ REFIID riid, _Outptr_ void **ppv)
{
    HRESULT hr;

    debug("SCZ_Provider Provider_CreateInstance\n");

    SCZ_Provider *pProvider = new(std::nothrow) SCZ_Provider();
    if (pProvider)
    {
        hr = pProvider->QueryInterface(riid, ppv);
        pProvider->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}


void SCZ_Provider::_GetSerializedCredentials(PWSTR *username, PWSTR *password, PWSTR *domain)
{
    debug("SCZ_Provider _GetSerializedCredentials\n");

    if (username)
    {
        if (_SerializationAvailable(SAF_USERNAME))
        {
            *username = (PWSTR)LocalAlloc(LMEM_ZEROINIT, _pkiulSetSerialization->Logon.UserName.Length + sizeof(wchar_t));
            CopyMemory(*username, _pkiulSetSerialization->Logon.UserName.Buffer, _pkiulSetSerialization->Logon.UserName.Length);
        }
        else
            *username = NULL;
    }

    if (password)
    {
        if (_SerializationAvailable(SAF_PASSWORD))
        {
            *password = (PWSTR)LocalAlloc(LMEM_ZEROINIT, _pkiulSetSerialization->Logon.Password.Length + sizeof(wchar_t));
            CopyMemory(*password, _pkiulSetSerialization->Logon.Password.Buffer, _pkiulSetSerialization->Logon.Password.Length);
        }
        else
            *password = NULL;
    }

    if (domain)
    {
        if (_SerializationAvailable(SAF_DOMAIN))
        {
            *domain = (PWSTR)LocalAlloc(LMEM_ZEROINIT, _pkiulSetSerialization->Logon.LogonDomainName.Length + sizeof(wchar_t));
            CopyMemory(*domain, _pkiulSetSerialization->Logon.LogonDomainName.Buffer, _pkiulSetSerialization->Logon.LogonDomainName.Length);
        }
        else
            *domain = NULL;
    }
}

bool SCZ_Provider::_SerializationAvailable(SERIALIZATION_AVAILABLE_FOR checkFor)
{
    debug("SCZ_Provider _SerializationAvailable\n");

    bool result = false;

    if (!_pkiulSetSerialization)
    {
        debug("No serialized creds set");
    }
    else {
        switch (checkFor) {
            case SAF_USERNAME:
                result = _pkiulSetSerialization->Logon.UserName.Length && _pkiulSetSerialization->Logon.UserName.Buffer;
                break;
            case SAF_PASSWORD:
                result = _pkiulSetSerialization->Logon.Password.Length && _pkiulSetSerialization->Logon.Password.Buffer;
                break;
            case SAF_DOMAIN:
                result = _pkiulSetSerialization->Logon.LogonDomainName.Length && _pkiulSetSerialization->Logon.LogonDomainName.Buffer;
                break;
            default:
                break;
        }
    }

    return result;
}
