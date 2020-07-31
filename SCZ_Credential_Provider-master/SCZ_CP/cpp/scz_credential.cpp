//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//

#ifndef WIN32_NO_STATUS
#include <ntstatus.h>
#define WIN32_NO_STATUS
#endif

#include <unknwn.h>

#include "scz_credential.h"
#include "config.h"
#include "helpers.h"
#include "guid.h"
#include "debug.h"
#include "validate.h"
#include "data.h"
#include "hooks.h"
#include "scenarios.h"


SCZ_Credential::SCZ_Credential():
    _cRef(1),
    _pCredProvCredentialEvents(nullptr)
{
    DllAddRef();

    ZeroMemory(_rgCredProvFieldDescriptors, sizeof(_rgCredProvFieldDescriptors));
    ZeroMemory(_rgFieldStatePairs, sizeof(_rgFieldStatePairs));
    ZeroMemory(_rgFieldStrings, sizeof(_rgFieldStrings));

    Data::Credential::Init();
}

SCZ_Credential::~SCZ_Credential()
{
    debug("SCZ_Credential Finished !\n");

    scenarios::Clear(_rgFieldStrings, _rgCredProvFieldDescriptors, this, NULL, CLEAR_FIELDS_ALL_DESTROY);

    Configuration::Deinit();

    Data::Credential::Deinit();
    
    DllRelease();
}

// Initializes one credential with the field information passed in.
// Set the value of the SFI_LARGE_TEXT field to pwzUsername.
HRESULT SCZ_Credential::Initialize(
    __in const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgcpfd,
    __in const FIELD_STATE_PAIR* rgfsp,
    __in_opt PWSTR user_name,
    __in_opt PWSTR domain_name,
    __in_opt PWSTR ldap_password)
{
    HRESULT hr = S_OK;

    debug("SCZ_Credential Initialize...\n");

    if (NOT_EMPTY(user_name))
    {
        debug("Copying user_name to credential\n");
        Data::Credential::Get()->user_name = _wcsdup(user_name);
    }

    if (NOT_EMPTY(domain_name))
    {
        debug("Copying domain_name to credential\n");
        Data::Credential::Get()->domain_name = _wcsdup(domain_name);
    }

    if (NOT_EMPTY(ldap_password))
    {
        debug("Copying ldap_password to credential\n");
        Data::Credential::Get()->ldap_password = _wcsdup(ldap_password);
    }

    for (DWORD i = 0; SUCCEEDED(hr) && i < scenarios::GetCurrentNumFields(); i++)
    {
        debug("Copy field: %d\n", i+1);
        _rgFieldStatePairs[i] = rgfsp[i];

        hr = FieldDescriptorCopy(rgcpfd[i], &_rgCredProvFieldDescriptors[i]);

        if (FAILED(hr))
            break;

        if (s_rgCredProvFieldInitializorsFor[Data::Provider::Get()->usage_scenario] != NULL)
            scenarios::InitializeField(_rgFieldStrings, s_rgCredProvFieldInitializorsFor[Data::Provider::Get()->usage_scenario][i], i);
    }

    debug("Init result: ");
    if (SUCCEEDED(hr))
        debug("OK\n");
    else
        debug("FAIL !\n");

    return hr;
}

// LogonUI calls this in order to give us a callback in case we need to notify it of anything.
HRESULT SCZ_Credential::Advise(_In_ ICredentialProviderCredentialEvents *pcpce)
{
    debug("SCZ_Credential Advise\n");

    if (_pCredProvCredentialEvents != nullptr)
    {
        _pCredProvCredentialEvents->Release();
    }
    _pCredProvCredentialEvents = pcpce;
    _pCredProvCredentialEvents->AddRef();

    return S_OK;
}

// LogonUI calls this to tell us to release the callback.
HRESULT SCZ_Credential::UnAdvise()
{
    debug("SCZ_Credential UnAdvise\n");

    if (_pCredProvCredentialEvents)
    {
        _pCredProvCredentialEvents->Release();
    }
    _pCredProvCredentialEvents = nullptr;
    return S_OK;
}

// LogonUI calls this function when our tile is selected (zoomed)
// If you simply want fields to show/hide based on the selected state,
// there's no need to do anything here - you can set that up in the
// field definitions. But if you want to do something
// more complicated, like change the contents of a field when the tile is
// selected, you would do it here.
HRESULT SCZ_Credential::SetSelected(_Out_ BOOL *pbAutoLogon)
{
    *pbAutoLogon = FALSE;

    debug("SCZ_Credential SetSelected\n");

    return S_OK;
}

// Similarly to SetSelected, LogonUI calls this when your tile was selected
// and now no longer is. The most common thing to do here (which we do below)
// is to clear out the password field.
HRESULT SCZ_Credential::SetDeselected()
{
    HRESULT hr = S_OK;

    debug("SCZ_Credential SetDeSelected\n");

    return hr;
}

// Get info for a particular field of a tile. Called by logonUI to get information
// to display the tile.
HRESULT SCZ_Credential::GetFieldState(DWORD dwFieldID,
                                         _Out_ CREDENTIAL_PROVIDER_FIELD_STATE *pcpfs,
                                         _Out_ CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE *pcpfis)
{
    HRESULT hr;

    debug("SCZ_Credential GetFieldState: %d\n", dwFieldID);

    // Validate our parameters.
    if ((dwFieldID < scenarios::GetCurrentNumFields()) && pcpfs && pcpfis)
    {
        *pcpfs = _rgFieldStatePairs[dwFieldID].cpfs;
        *pcpfis = _rgFieldStatePairs[dwFieldID].cpfis;
        hr = S_OK;
    }
    else
    {
        hr = E_INVALIDARG;
    }
    return hr;
}

// Sets ppwsz to the string value of the field at the index dwFieldID
HRESULT SCZ_Credential::GetStringValue(DWORD dwFieldID, _Outptr_result_nullonfailure_ PWSTR *ppwsz)
{
    HRESULT hr;
    *ppwsz = nullptr;

    debug("SCZ_Credential GetStringValue: %d\n", dwFieldID);

    // Check to make sure dwFieldID is a legitimate index
    if (dwFieldID < scenarios::GetCurrentNumFields() && ppwsz)
    {
        // Make a copy of the string and return that. The caller
        // is responsible for freeing it.
        hr = SHStrDupW(_rgFieldStrings[dwFieldID], ppwsz);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

// Get the image to show in the user tile
HRESULT SCZ_Credential::GetBitmapValue(DWORD dwFieldID, _Outptr_result_nullonfailure_ HBITMAP *phbmp)
{
    HRESULT hr;

    debug("SCZ_Credential GetBitmapValue: %d\n", dwFieldID);

    hr = Hook::CredentialHooks::GetBitmapValue(HINST_THISDLL, dwFieldID, phbmp);

    return hr;
}

// Sets pdwAdjacentTo to the index of the field the submit button should be
// adjacent to. We recommend that the submit button is placed next to the last
// field which the user is required to enter information in. Optional fields
// should be below the submit button.
HRESULT SCZ_Credential::GetSubmitButtonValue(DWORD dwFieldID, _Out_ DWORD *pdwAdjacentTo)
{
    debug("SCZ_Credential GetSubmitButtonValue: %d\n", dwFieldID);

    HRESULT hr = Hook::CredentialHooks::GetSubmitButtonValue(dwFieldID, pdwAdjacentTo);

    return hr;
}

// Sets the value of a field which can accept a string as a value.
// This is called on each keystroke when a user types into an edit field
HRESULT SCZ_Credential::SetStringValue(DWORD dwFieldID, _In_ PCWSTR pwz)
{
    HRESULT hr;

    debug("SCZ_Credential SetStringValue: %d: %S\n", dwFieldID, pwz);

    // Validate parameters.
    if (dwFieldID < scenarios::GetCurrentNumFields() &&
        (CPFT_EDIT_TEXT == _rgCredProvFieldDescriptors[dwFieldID].cpft ||
        CPFT_PASSWORD_TEXT == _rgCredProvFieldDescriptors[dwFieldID].cpft))
    {
        PWSTR *ppwszStored = &_rgFieldStrings[dwFieldID];
        CoTaskMemFree(*ppwszStored);
        hr = SHStrDupW(pwz, ppwszStored);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

// Returns whether a checkbox is checked or not as well as its label.
HRESULT SCZ_Credential::GetCheckboxValue(DWORD dwFieldID, _Out_ BOOL *pbChecked, _Outptr_result_nullonfailure_ PWSTR *ppwszLabel)
{
    HRESULT hr;

    debug("SCZ_Credential GetCheckboxValue: %d\n", dwFieldID);

    // Validate parameters.
    if (dwFieldID < scenarios::GetCurrentNumFields() &&
        (CPFT_CHECKBOX == _rgCredProvFieldDescriptors[dwFieldID].cpft))
    {
        hr = Hook::CredentialHooks::GetCheckboxValue(this, _pCredProvCredentialEvents, _rgFieldStrings, dwFieldID, pbChecked, ppwszLabel);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

// Sets whether the specified checkbox is checked or not.
HRESULT SCZ_Credential::SetCheckboxValue(DWORD dwFieldID, BOOL bChecked)
{
    HRESULT hr;

    debug("SCZ_Credential SetCheckboxValue: %d\n", dwFieldID);

    // Validate parameters.
    if (dwFieldID < scenarios::GetCurrentNumFields() &&
        (CPFT_CHECKBOX == _rgCredProvFieldDescriptors[dwFieldID].cpft))
    {
        hr = Hook::CredentialHooks::SetCheckboxValue(this, _pCredProvCredentialEvents, dwFieldID, bChecked);
    }
    else
    {
        hr = E_INVALIDARG;
    }


    return hr;
}

// Returns the number of items to be included in the combobox (pcItems), as well as the
// currently selected item (pdwSelectedItem).
HRESULT SCZ_Credential::GetComboBoxValueCount(DWORD dwFieldID, _Out_ DWORD *pcItems, _Deref_out_range_(<, *pcItems) _Out_ DWORD *pdwSelectedItem)
{
    HRESULT hr;
    *pcItems = 0;
    *pdwSelectedItem = 0;

    debug("SCZ_Credential GetComboBoxValueCount: %d\n", dwFieldID);

    if (dwFieldID < scenarios::GetCurrentNumFields() &&
        (CPFT_COMBOBOX == _rgCredProvFieldDescriptors[dwFieldID].cpft))
    {
        hr = Hook::CredentialHooks::GetComboBoxValueCount(dwFieldID, pcItems, pdwSelectedItem);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

// Called iteratively to fill the combobox with the string (ppwszItem) at index dwItem.
HRESULT SCZ_Credential::GetComboBoxValueAt(DWORD dwFieldID, DWORD dwItem, _Outptr_result_nullonfailure_ PWSTR *ppwszItem)
{
    HRESULT hr;
    *ppwszItem = nullptr;

    debug("SCZ_Credential GetComboBoxValueAt: %d\n", dwFieldID);

    // Validate parameters.
    if (dwFieldID < scenarios::GetCurrentNumFields() &&
        (CPFT_COMBOBOX == _rgCredProvFieldDescriptors[dwFieldID].cpft))
    {
        hr = Hook::CredentialHooks::GetComboBoxValueAt(dwFieldID, dwItem, ppwszItem);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

// Called when the user changes the selected item in the combobox.
HRESULT SCZ_Credential::SetComboBoxSelectedValue(DWORD dwFieldID, DWORD dwSelectedItem)
{
    HRESULT hr;

    debug("SCZ_Credential SetComboBoxSelectedValue: %d\n", dwFieldID);

    // Validate parameters.
    if (dwFieldID < scenarios::GetCurrentNumFields() &&
        (CPFT_COMBOBOX == _rgCredProvFieldDescriptors[dwFieldID].cpft))
    {
        hr = Hook::CredentialHooks::SetComboBoxSelectedValue(this, _pCredProvCredentialEvents, dwFieldID, dwSelectedItem, _dwComboIndex);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

// Called when the user clicks a command link.
HRESULT SCZ_Credential::CommandLinkClicked(DWORD dwFieldID)
{
    UNREFERENCED_PARAMETER(dwFieldID);

    debug("SCZ_Credential CommandLinkClicked: %d\n", dwFieldID);

    return E_NOTIMPL;
}

// Collect the username and password into a serialized credential for the correct usage scenario
// (logon/unlock is what's demonstrated in this sample).  LogonUI then passes these credentials
// back to the system to log on.
HRESULT SCZ_Credential::GetSerialization(_Out_ CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr,
    _Out_ CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs,
    _Outptr_result_maybenull_ PWSTR* ppwszOptionalStatusText,
    _Out_ CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon)
{
    HRESULT hr = E_FAIL, retVal = S_OK;

    debug("SCZ_Credential GetSerialization\n");

    *pcpgsr = CPGSR_RETURN_NO_CREDENTIAL_FINISHED;

    // reference parameters to internal datastructures (we need them in the hooks)
    HOOK_CHECK_CRITICAL(Hook::Serialization::Initialization(), CleanUpAndReturn);

    Hook::Serialization::Get()->pCredProvCredentialEvents = _pCredProvCredentialEvents;
    Hook::Serialization::Get()->pCredProvCredential = this;

    Hook::Serialization::Get()->pcpcs = pcpcs;
    Hook::Serialization::Get()->pcpgsr = pcpgsr;

    Hook::Serialization::Get()->status_icon = pcpsiOptionalStatusIcon;
    Hook::Serialization::Get()->status_text = ppwszOptionalStatusText;

    Hook::Serialization::Get()->field_strings = _rgFieldStrings;
    Hook::Serialization::Get()->num_field_strings = scenarios::GetCurrentNumFields();

    if (true)
    {
        if (Data::Provider::Get()->usage_scenario == CPUS_CREDUI) {
            debug("--scenarios::CredPackAuthentication");

            hr = scenarios::CredPackAuthentication(pcpgsr, pcpcs, Data::Provider::Get()->usage_scenario,
                Data::Credential::Get()->user_name,
                Data::Credential::Get()->ldap_password,
                Data::Credential::Get()->domain_name
            );
        }
        else {
            debug("--scenarios::KerberosLogon");

            hr = scenarios::KerberosLogon(pcpgsr, pcpcs, Data::Provider::Get()->usage_scenario,
                Data::Credential::Get()->user_name,
                Data::Credential::Get()->ldap_password,
                Data::Credential::Get()->domain_name
            );
        }

        if (SUCCEEDED(hr))
        {
            debug("--Hook::Serialization::KerberosCallSuccessfull(");
            HOOK_CHECK_CRITICAL(Hook::Serialization::KerberosCallSuccessfull(), CleanUpAndReturn);
        }
        else
        {
            debug("--Hook::Serialization::KerberosCallFailed()");
            HOOK_CHECK_CRITICAL(Hook::Serialization::KerberosCallFailed(), CleanUpAndReturn);
            retVal = S_FALSE;
        }

        goto CleanUpAndReturn; // because we dont want to hit the pcpgsr selection switch below
    }

    *pcpgsr = CPGSR_NO_CREDENTIAL_FINISHED;

    goto CleanUpAndReturn;

CleanUpAndReturn:
    Hook::Serialization::DataDeinitialization();

    scenarios::Clear(_rgFieldStrings, _rgCredProvFieldDescriptors, this, _pCredProvCredentialEvents, CLEAR_FIELDS_CRYPT);

    Hook::Serialization::BeforeReturn();
    if (pcpgsr) {
        if (*pcpgsr == CPGSR_NO_CREDENTIAL_FINISHED) { debug("CPGSR_NO_CREDENTIAL_FINISHED\n"); }
        if (*pcpgsr == CPGSR_NO_CREDENTIAL_NOT_FINISHED) { debug("CPGSR_NO_CREDENTIAL_NOT_FINISHED\n"); }
        if (*pcpgsr == CPGSR_RETURN_CREDENTIAL_FINISHED) { debug("CPGSR_RETURN_NO_CREDENTIAL_FINISHED\n"); }
        if (*pcpgsr == CPGSR_RETURN_NO_CREDENTIAL_FINISHED) { debug("CPGSR_RETURN_NO_CREDENTIAL_FINISHED\n"); }
    }
    else { debug("pcpgsr is a nullpointer!\n"); }

    debug("SCZ_Credential GetSerialization - END\n");

    return retVal;
}

HRESULT SCZ_Credential::Connect(__in IQueryContinueWithStatus *pqcws)
{
    debug("SCZ_Credential Connect\n");

    Data::Credential::Get()->pqcws = pqcws;

    HOOK_CHECK_CRITICAL(Hook::Serialization::Initialization(), Exit);

    Hook::Serialization::Get()->pCredProvCredential = this;
    Hook::Serialization::Get()->pCredProvCredentialEvents = _pCredProvCredentialEvents;
    Hook::Serialization::Get()->field_strings = _rgFieldStrings;
    Hook::Serialization::Get()->num_field_strings = scenarios::GetCurrentNumFields();;

    HOOK_CHECK_CRITICAL(Hook::Serialization::EndpointInitialization(), Exit);
    HOOK_CHECK_CRITICAL(Hook::Serialization::DataInitialization(), Exit);

    if (Data::Provider::Get()->usage_scenario == CPUS_UNLOCK_WORKSTATION || Data::Provider::Get()->usage_scenario == CPUS_LOGON || Data::Provider::Get()->usage_scenario == CPUS_CREDUI)
    {
            token_validate();
    }

    if (pqcws->QueryContinue() != S_OK)
    {
        debug("User cancelled\n");
    }

Exit:
    Data::Credential::Get()->pqcws = NULL;
    debug("Connect - END");

    return S_OK; // always S_OK
}

HRESULT SCZ_Credential::Disconnect()
{
    debug("SCZ_Credential Disconnect\n");

    return E_NOTIMPL;
}

struct REPORT_RESULT_STATUS_INFO
{
    NTSTATUS ntsStatus;
    NTSTATUS ntsSubstatus;
    PWSTR     pwzMessage;
    CREDENTIAL_PROVIDER_STATUS_ICON cpsi;
};

static const REPORT_RESULT_STATUS_INFO s_rgLogonStatusInfo[] =
{
    { STATUS_LOGON_FAILURE, STATUS_SUCCESS, L"Incorrect password or username.", CPSI_ERROR, },
    { STATUS_ACCOUNT_RESTRICTION, STATUS_ACCOUNT_DISABLED, L"The account is disabled.", CPSI_WARNING },
};

// ReportResult is completely optional.  Its purpose is to allow a credential to customize the string
// and the icon displayed in the case of a logon failure.  For example, we have chosen to
// customize the error shown in the case of bad username/password and in the case of the account
// being disabled.
HRESULT SCZ_Credential::ReportResult(NTSTATUS ntsStatus,
                                        NTSTATUS ntsSubstatus,
                                        _Outptr_result_maybenull_ PWSTR *ppwszOptionalStatusText,
                                        _Out_ CREDENTIAL_PROVIDER_STATUS_ICON *pcpsiOptionalStatusIcon)
{
    *ppwszOptionalStatusText = nullptr;
    *pcpsiOptionalStatusIcon = CPSI_NONE;

    DWORD dwStatusInfo = (DWORD)-1;

    debug("SCZ_Credential ReportResult\n");

    // Look for a match on status and substatus.
    for (DWORD i = 0; i < ARRAYSIZE(s_rgLogonStatusInfo); i++)
    {
        if (s_rgLogonStatusInfo[i].ntsStatus == ntsStatus && s_rgLogonStatusInfo[i].ntsSubstatus == ntsSubstatus)
        {
            dwStatusInfo = i;
            break;
        }
    }

    if ((DWORD)-1 != dwStatusInfo)
    {
        if (SUCCEEDED(SHStrDupW(s_rgLogonStatusInfo[dwStatusInfo].pwzMessage, ppwszOptionalStatusText)))
        {
            *pcpsiOptionalStatusIcon = s_rgLogonStatusInfo[dwStatusInfo].cpsi;
        }
    }

    return S_OK;
}

