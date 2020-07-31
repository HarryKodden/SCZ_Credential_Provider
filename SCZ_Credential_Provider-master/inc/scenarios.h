//#include "common_comboboxes.h"

#ifndef _SCENARIOS_H
#define _SCENARIOS_H
#pragma once

#define WORKSTATION_LOCKED Data::Credential::Get()->user_name
#define MAX_NUM_FIELDS 10

#include "scenario_unlock_logon.h"
#include "scenario_credui.h"

#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "wtsapi32.lib")

static const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* s_rgCredProvFieldDescriptorsFor[] =
{
	NULL,												// CPUS_INVALID = 0x0000,
	s_rgScenarioLogonUnlockCredProvFieldDescriptors,	// CPUS_LOGON,
	s_rgScenarioLogonUnlockCredProvFieldDescriptors,	// CPUS_UNLOCK_WORKSTATION,
	NULL,												// CPUS_CHANGE_PASSWORD,
	s_rgScenarioCredUiCredProvFieldDescriptors,			// CPUS_CREDUI,
	NULL												// CPUS_PLAP
};

static const FIELD_INITIALIZOR* s_rgCredProvFieldInitializorsFor[] =
{
	NULL,											// CPUS_INVALID = 0x0000,
	s_rgScenarioLogonUnlockFieldInitializors,		// CPUS_LOGON,
	s_rgScenarioLogonUnlockFieldInitializors,		// CPUS_UNLOCK_WORKSTATION,
	NULL,											// CPUS_CHANGE_PASSWORD,
	s_rgScenarioCredUiFieldInitializors,			// CPUS_CREDUI,
	NULL											// CPUS_PLAP
};

static const unsigned int s_rgCredProvNumFieldsFor[] =
{
	0,					// CPUS_INVALID = 0x0000,
	LUFI_NUM_FIELDS,	// CPUS_LOGON,
	LUFI_NUM_FIELDS,	// CPUS_UNLOCK_WORKSTATION,
	0,					// CPUS_CHANGE_PASSWORD,
	CFI_NUM_FIELDS,		// CPUS_CREDUI,
	0					// CPUS_PLAP
};

static const FIELD_STATE_PAIR* s_rgCredProvBaseFieldStatePairsFor[] =
{
	NULL,												// CPUS_INVALID = 0x0000,
	s_rgScenarioLogonUnlockFieldStatePairs,				// CPUS_LOGON,
	s_rgScenarioLogonUnlockFieldStatePairsUnlock,		// CPUS_UNLOCK_WORKSTATION,
	NULL,			// CPUS_CHANGE_PASSWORD,
	s_rgScenarioCredUiFieldStatePairs,					// CPUS_CREDUI,
	NULL												// CPUS_PLAP
};

namespace scenarios
{
    enum SCENARIO
    {
            SCENARIO_NO_CHANGE = 0,
            SCENARIO_LOGON_BASE = 1,
            SCENARIO_UNLOCK_BASE = 2,
            SCENARIO_SECOND_STEP = 3,
            SCENARIO_LOGON_TWO_STEP = 4,
            SCENARIO_UNLOCK_TWO_STEP = 5,
            SCENARIO_CHANGE_PASSWORD = 6,
    };

    HRESULT SetScenarioBasedFieldStates(
            __in ICredentialProviderCredential* self,
            __in ICredentialProviderCredentialEvents* pCPCE,
            __in SCENARIO scenario
            );

    HRESULT SetScenarioBasedTextFields(
            __inout int &largeTextFieldId,
            __inout int &smallTextFieldId,
            __in CREDENTIAL_PROVIDER_USAGE_SCENARIO scenario
            );

    const FIELD_STATE_PAIR* GetFieldStatePairFor(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus);
    const FIELD_STATE_PAIR* GetFieldStatePairFor(SCENARIO scenario);

	void SetScenario(
		__in ICredentialProviderCredential* self,
		__in ICredentialProviderCredentialEvents* pCPCE,
		__in SCENARIO scenario,
		__in_opt PWSTR large_text,
		__in_opt PWSTR small_text
		);

	void SetScenario(
		__in ICredentialProviderCredential* self,
		__in ICredentialProviderCredentialEvents* pCPCE,
		__in SCENARIO scenario
		);

	void SetScenario(
		__in ICredentialProviderCredential* self,
		__in ICredentialProviderCredentialEvents* pCPCE,
		__in_opt PWSTR large_text,
		__in_opt PWSTR small_text
		);

#define CLEAR_FIELDS_CRYPT 0
#define CLEAR_FIELDS_EDIT_AND_CRYPT 1
#define CLEAR_FIELDS_ALL 2
#define CLEAR_FIELDS_ALL_DESTROY 3

	HRESULT Clear(
		wchar_t* (&field_strings)[MAX_NUM_FIELDS],
		CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR(&pcpfd)[MAX_NUM_FIELDS],
		ICredentialProviderCredential* pcpc,
		ICredentialProviderCredentialEvents* pcpce,
		char clear);

	HRESULT SetFieldStatePairBatch(
		__in ICredentialProviderCredential* self,
		__in ICredentialProviderCredentialEvents* pCPCE,
		__in const FIELD_STATE_PAIR* pFSP
		);

	unsigned int GetCurrentNumFields();

	HRESULT InitializeField(LPWSTR *rgFieldStrings, const FIELD_INITIALIZOR initializor, DWORD field_index);

	HRESULT KerberosLogon(
		__out CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE*& pcpgsr,
		__out CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION*& pcpcs,
		__in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
		__in PWSTR username,
		__in PWSTR password,
		__in PWSTR domain
		);

	HRESULT CredPackAuthentication(
		__out CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE*& pcpgsr,
		__out CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION*& pcpcs,
		__in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
		__in PWSTR username,
		__in PWSTR password,
		__in PWSTR domain
		);
}

#endif