//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// SCZ_Credential is our implementation of ICredentialProviderCredential.
// ICredentialProviderCredential is what LogonUI uses to let a credential
// provider specify what a user tile looks like and then tell it what the
// user has entered into the tile.  ICredentialProviderCredential is also
// responsible for packaging up the users credentials into a buffer that
// LogonUI then sends on to LSA.

#pragma once

#include <windows.h>
#include <strsafe.h>
#include <shlwapi.h>
//#include <shlguid.h>
//#include <propkey.h>
#include <new>

#include "dll.h"
#include "resource.h"
#include "field_state_pair.h"
#include "scenarios.h"


class SCZ_Credential : public IConnectableCredentialProviderCredential
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
            QITABENT(SCZ_Credential, ICredentialProviderCredential), // IID_ICredentialProviderCredential
            QITABENT(SCZ_Credential, IConnectableCredentialProviderCredential), // IID_IConnectableCredentialProviderCredential
            {0},
        };

        return QISearch(this, qit, riid, ppv);
    }

  public:
    IFACEMETHODIMP Advise(__in ICredentialProviderCredentialEvents* pcpce);
    IFACEMETHODIMP UnAdvise();

    IFACEMETHODIMP SetSelected(__out BOOL* pbAutoLogon);
    IFACEMETHODIMP SetDeselected();

    IFACEMETHODIMP GetFieldState(__in DWORD dwFieldID,
        __out CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
        __out CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis);

    IFACEMETHODIMP GetStringValue(__in DWORD dwFieldID, __deref_out PWSTR* ppwsz);
    IFACEMETHODIMP GetBitmapValue(__in DWORD dwFieldID, __out HBITMAP* phbmp);
    IFACEMETHODIMP GetCheckboxValue(__in DWORD dwFieldID, __out BOOL* pbChecked, __deref_out PWSTR* ppwszLabel);
    IFACEMETHODIMP GetComboBoxValueCount(__in DWORD dwFieldID, __out DWORD* pcItems, __out_range(< , *pcItems) DWORD* pdwSelectedItem);
    IFACEMETHODIMP GetComboBoxValueAt(__in DWORD dwFieldID, __in DWORD dwItem, __deref_out PWSTR* ppwszItem);
    IFACEMETHODIMP GetSubmitButtonValue(__in DWORD dwFieldID, __out DWORD* pdwAdjacentTo);

    IFACEMETHODIMP SetStringValue(__in DWORD dwFieldID, __in PCWSTR pwz);
    IFACEMETHODIMP SetCheckboxValue(__in DWORD dwFieldID, __in BOOL bChecked);
    IFACEMETHODIMP SetComboBoxSelectedValue(__in DWORD dwFieldID, __in DWORD dwSelectedItem);
    IFACEMETHODIMP CommandLinkClicked(__in DWORD dwFieldID);
    
    IFACEMETHODIMP GetSerialization(__out CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr,
        __out CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs,
        __deref_out_opt PWSTR* ppwszOptionalStatusText,
        __out CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);
    IFACEMETHODIMP ReportResult(__in NTSTATUS ntsStatus,
        __in NTSTATUS ntsSubstatus,
        __deref_out_opt PWSTR* ppwszOptionalStatusText,
        __out CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);

public:
    IFACEMETHODIMP Connect(__in IQueryContinueWithStatus *pqcws);
    IFACEMETHODIMP Disconnect();

    SCZ_Credential();

    virtual ~SCZ_Credential();

public:
    HRESULT Initialize(//__in CProvider* pProvider,
        __in const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgcpfd,
        __in const FIELD_STATE_PAIR* rgfsp,
        __in_opt PWSTR user_name,
        __in_opt PWSTR domain_name,
        __in_opt PWSTR password);

    HRESULT EndpointCallback(__in DWORD dwFlag);

private:
    LONG                                    _cRef;

    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR    _rgCredProvFieldDescriptors[MAX_NUM_FIELDS];    // An array holding the type and 
                                                                                            // name of each field in the tile.

    FIELD_STATE_PAIR                        _rgFieldStatePairs[MAX_NUM_FIELDS];          // An array holding the state of 
                                                                                         // each field in the tile.

    wchar_t*                                _rgFieldStrings[MAX_NUM_FIELDS];             // An array holding the string 
                                                                                         // value of each field. This is 
                                                                                         // different from the name of 
                                                                                         // the field held in 
                                                                                         // _rgCredProvFieldDescriptors.
    ICredentialProviderCredentialEvents*    _pCredProvCredentialEvents;

    DWORD                                   _dwComboIndex;                               // Tracks the current index 
                                                                                         // of our combobox.

};
