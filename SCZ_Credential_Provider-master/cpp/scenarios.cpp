#include "config.h"
#include "data.h"
#include "scenarios.h"
#include "debug.h"
#include "guid.h"

#define MAX_SIZE_DOMAIN 100

namespace scenarios
{

	HRESULT SetScenarioBasedFieldStates(
	        __in ICredentialProviderCredential* self,
	        __in ICredentialProviderCredentialEvents* pCPCE,
	        __in SCENARIO scenario
	        )
	{
	        HRESULT hr = S_OK;
	        switch (scenario)
	        {
	        case SCENARIO_LOGON_BASE:
	                hr = SetFieldStatePairBatch(self, pCPCE, s_rgScenarioLogonUnlockFieldStatePairs);
	                break;

	        case SCENARIO_UNLOCK_BASE:
	                hr = SetFieldStatePairBatch(self, pCPCE, s_rgScenarioLogonUnlockFieldStatePairsUnlock);
	                break;

	        case SCENARIO_SECOND_STEP:
	                hr = SetFieldStatePairBatch(self, pCPCE, s_rgScenarioLogonUnlockFieldStatePairsSecondStep);
	                break;

	        case SCENARIO_CHANGE_PASSWORD:
//	                hr = SetFieldStatePairBatch(self, pCPCE, s_rgScenarioChangePasswordFieldStatePairs);
	                break;
	        case SCENARIO_UNLOCK_TWO_STEP:
//	                hr = SetFieldStatePairBatch(self, pCPCE, s_rgScenarioLogonUnlockFieldStatePairsUnlockTwoStep);
	                break;
	        case SCENARIO_LOGON_TWO_STEP:
//	                hr = SetFieldStatePairBatch(self, pCPCE, s_rgScenarioLogonUnlockFieldStatePairsTwoStep);
	                break;
	        case SCENARIO_NO_CHANGE:
	        default:
	                break;
	        }

	        return hr;
	}

    HRESULT SetScenarioBasedTextFields(
            __inout int &largeTextFieldId,
            __inout int &smallTextFieldId,
            __in CREDENTIAL_PROVIDER_USAGE_SCENARIO scenario
            )
    {
            switch (scenario)
            {
            case CPUS_LOGON:
            case CPUS_UNLOCK_WORKSTATION:
                    largeTextFieldId = LUFI_OTP_LARGE_TEXT;
                    smallTextFieldId = LUFI_OTP_SMALL_TEXT;
                    break;
            case CPUS_CREDUI:
                    largeTextFieldId = CFI_OTP_LARGE_TEXT;
                    smallTextFieldId = CFI_OTP_SMALL_TEXT;
                    break;
            default:
                    break;
            }

            return S_OK;
    }

    const FIELD_STATE_PAIR* GetFieldStatePairFor(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus)
    {       
            if (Data::Provider::Get()->usage_scenario == CPUS_LOGON)
            {       
//                    if (Configuration::Get()->two_step_hide_otp) {
//                            return s_rgScenarioLogonUnlockFieldStatePairsTwoStep;
//                    }
                    return s_rgScenarioLogonUnlockFieldStatePairs;
            }
            else if (Data::Provider::Get()->usage_scenario == CPUS_UNLOCK_WORKSTATION)
            {       
//                    if (Configuration::Get()->two_step_hide_otp) {
//                            return s_rgScenarioLogonUnlockFieldStatePairsUnlockTwoStep;
//                    }
                    return s_rgScenarioLogonUnlockFieldStatePairsUnlock;
            }
            
            return s_rgCredProvBaseFieldStatePairsFor[cpus];
    }
                                
    const FIELD_STATE_PAIR* GetFieldStatePairFor(SCENARIO scenario)
    {                       
            switch (scenario)
            {       
            case SCENARIO_LOGON_BASE:
                    return s_rgScenarioLogonUnlockFieldStatePairs;
                    break;
                    
            case SCENARIO_UNLOCK_BASE:
                    return s_rgScenarioLogonUnlockFieldStatePairsUnlock;
                    break;
                    
            case SCENARIO_SECOND_STEP:
//                    return s_rgScenarioLogonUnlockFieldStatePairsSecondStep;
//                    break;
                            
            case SCENARIO_CHANGE_PASSWORD:
//                    return s_rgScenarioChangePasswordFieldStatePairs;
//                    break;  
                            
            case SCENARIO_LOGON_TWO_STEP:
//                    return s_rgScenarioLogonUnlockFieldStatePairsTwoStep;
//                    break;  
                    
            case SCENARIO_UNLOCK_TWO_STEP:
//                    return s_rgScenarioLogonUnlockFieldStatePairsUnlockTwoStep;
//                    break;

            case SCENARIO_NO_CHANGE:
            default:
                    return NULL;
                    break;
            }       
    }       

	void SetScenario(
		__in ICredentialProviderCredential* self,
		__in ICredentialProviderCredentialEvents* pCPCE,
		__in_opt PWSTR large_text,
		__in_opt PWSTR small_text
	)
	{
		SetScenario(self, pCPCE, SCENARIO_NO_CHANGE, large_text, small_text);
	}

	void SetScenario(
		__in ICredentialProviderCredential* self,
		__in ICredentialProviderCredentialEvents* pCPCE,
		__in SCENARIO scenario
	)
	{
		SetScenario(self, pCPCE, scenario, NULL, NULL);
	}

	void SetScenario(
		__in ICredentialProviderCredential* self,
		__in ICredentialProviderCredentialEvents* pCPCE,
		__in SCENARIO scenario,
		__in_opt PWSTR large_text,
		__in_opt PWSTR small_text
	)
	{
		debug(__FUNCTION__);

		HRESULT hr = S_OK;
		hr = SetScenarioBasedFieldStates(self, pCPCE, scenario);


		// Set text fields separately
		int largeTextFieldId = 0, smallTextFieldId = 0;

		hr = SetScenarioBasedTextFields(largeTextFieldId, smallTextFieldId, Data::Provider::Get()->usage_scenario);
		
		if (large_text)
		{
			debug("Large Text: %S\n", large_text);
			pCPCE->SetFieldString(self, largeTextFieldId, large_text);
		}

		if (small_text)
		{
			debug("Small Text: %s\n", small_text);
			pCPCE->SetFieldString(self, smallTextFieldId, small_text);
			//pCPCE->SetFieldState(self, smallTextFieldId, CPFS_DISPLAY_IN_SELECTED_TILE);
		}
		else
		{
			debug("Small Text: Empty");
			pCPCE->SetFieldString(self, smallTextFieldId, L"");
			pCPCE->SetFieldState(self, smallTextFieldId, CPFS_HIDDEN);
		}
	}

	HRESULT SetFieldStatePairBatch(
		__in ICredentialProviderCredential* self,
		__in ICredentialProviderCredentialEvents* pCPCE,
		__in const FIELD_STATE_PAIR* pFSP
	) {
		debug(__FUNCTION__);

		HRESULT hr = S_OK;

		if (!pCPCE || !pFSP)
			return E_INVALIDARG;

		for (unsigned int i = 0; i < GetCurrentNumFields() && SUCCEEDED(hr); i++)
		{
			hr = pCPCE->SetFieldState(self, i, pFSP[i].cpfs);

			if (SUCCEEDED(hr))
				hr = pCPCE->SetFieldInteractiveState(self, i, pFSP[i].cpfis);
		}

		return hr;
	}

	HRESULT Clear(wchar_t* (&field_strings)[MAX_NUM_FIELDS], CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR(&pcpfd)[MAX_NUM_FIELDS],
		ICredentialProviderCredential* pcpc, ICredentialProviderCredentialEvents* pcpce, char clear)
	{
		debug(__FUNCTION__);

		HRESULT hr = S_OK;

		for (unsigned int i = 0; i < GetCurrentNumFields() && SUCCEEDED(hr); i++)
		{
			char do_something = 0;

			if ((pcpfd[i].cpft == CPFT_PASSWORD_TEXT && clear >= CLEAR_FIELDS_CRYPT) || (pcpfd[i].cpft == CPFT_EDIT_TEXT && clear >= CLEAR_FIELDS_EDIT_AND_CRYPT))
			{
				if (field_strings[i])
				{
					// CoTaskMemFree (below) deals with NULL, but StringCchLength does not.
					size_t len = lstrlen(field_strings[i]);
					SecureZeroMemory(field_strings[i], len * sizeof(*field_strings[i]));

					do_something = 1;
				}
			}

			if (do_something || clear >= CLEAR_FIELDS_ALL)
			{
				CoTaskMemFree(field_strings[i]);
				hr = SHStrDupW(L"", &field_strings[i]);

				if (pcpce)
					pcpce->SetFieldString(pcpc, i, field_strings[i]);

				if (clear == CLEAR_FIELDS_ALL_DESTROY)
					CoTaskMemFree(pcpfd[i].pszLabel);
			}
		}

		return hr;
	}

	unsigned int GetCurrentNumFields()
	{
		//debug(__FUNCTION__);

		int numFields = 0;

		if (Data::Provider::Get() != NULL)
		{
			/*
			switch (Data::Provider::Get()->usage_scenario)
			{
			case CPUS_LOGON:
			case CPUS_UNLOCK_WORKSTATION:
				numFields = LUFI_NUM_FIELDS;
				break;
			case CPUS_CHANGE_PASSWORD:
				numFields = CPFI_NUM_FIELDS;
				break;
			default:
				break;
			}
			*/

			numFields = s_rgCredProvNumFieldsFor[Data::Provider::Get()->usage_scenario];

		}

		//debug(numFields);

		return numFields;
	}

#pragma warning( disable : 4456 )
	HRESULT InitializeField(LPWSTR *rgFieldStrings, const FIELD_INITIALIZOR initializor, DWORD field_index)
	{
		HRESULT hr = E_INVALIDARG;
		switch (initializor.type)
		{
		case FIT_VALUE:
			debug("...FIT_VALUE");
			hr = SHStrDupW(initializor.value, &rgFieldStrings[field_index]);
			//debug(rgFieldStrings[field_index]);
			break;
		case FIT_USERNAME:
			//debug("...FIT_USERNAME");
			if (NOT_EMPTY(Data::Credential::Get()->user_name))
				hr = SHStrDupW(Data::Credential::Get()->user_name, &rgFieldStrings[field_index]);
			else
				hr = SHStrDupW(L"", &rgFieldStrings[field_index]);
			//debug(rgFieldStrings[field_index]);
			break;
		case FIT_USERNAME_AND_DOMAIN:
			//debug("...FIT_USERNAME_AND_DOMAIN");
			if (NOT_EMPTY(Data::Credential::Get()->user_name) && NOT_EMPTY(Data::Credential::Get()->domain_name))
			{
				INIT_ZERO_WCHAR(username_domainname, 129);

				wcscat_s(username_domainname, sizeof(username_domainname) / sizeof(wchar_t), Data::Credential::Get()->user_name);
				wcscat_s(username_domainname, sizeof(username_domainname) / sizeof(wchar_t), L"@");
				wcscat_s(username_domainname, sizeof(username_domainname) / sizeof(wchar_t), Data::Credential::Get()->domain_name);

				hr = SHStrDupW(username_domainname, &rgFieldStrings[field_index]);
			}
			else if (NOT_EMPTY(Data::Credential::Get()->user_name))
				hr = SHStrDupW(Data::Credential::Get()->user_name, &rgFieldStrings[field_index]);
			else
				hr = SHStrDupW(L"", &rgFieldStrings[field_index]);
			//debug(rgFieldStrings[field_index]);
			break;
		case FIT_LOGIN_TEXT:
			//debug("...FIT_LOGIN_TEXT");
			wchar_t value[sizeof(Configuration::Get()->login_text)];
			Helper::CharToWideChar(Configuration::Get()->login_text, sizeof(Configuration::Get()->login_text), value);
			hr = SHStrDupW(value, &rgFieldStrings[field_index]);
			//debug(rgFieldStrings[field_index]);
			break;
		case FIT_VALUE_OR_LOGIN_TEXT:
			//debug("...FIT_VALUE_OR_LOGIN_TEXT");
			if (NOT_EMPTY(Configuration::Get()->login_text))
			{
				//debug("......Configuration::Get()->login_text");
				wchar_t value[sizeof(Configuration::Get()->login_text)];

				Helper::CharToWideChar(Configuration::Get()->login_text, sizeof(Configuration::Get()->login_text), value);
				//debug(value);
				hr = SHStrDupW(value, &rgFieldStrings[field_index]);
			}
			else {
				hr = SHStrDupW(initializor.value, &rgFieldStrings[field_index]);
			}
			//debug(rgFieldStrings[field_index]);
			break;
		case FIT_VALUE_OR_LOCKED_TEXT:
			//debug("...FIT_VALUE_OR_LOCKED_TEXT");
			//if (Data::Provider::Get()->usage_scenario == CPUS_UNLOCK_WORKSTATION && NOT_EMPTY(WORKSTATION_LOCKED))
			if (Data::Provider::Get()->usage_scenario == CPUS_UNLOCK_WORKSTATION && NOT_EMPTY(Data::Credential::Get()->user_name))
			{
				//debug("......Data::Provider::Get()->usage_scenario == CPUS_UNLOCK_WORKSTATION");
				//hr = SHStrDupW(WORKSTATION_LOCKED, &rgFieldStrings[field_index]);
				if (NOT_EMPTY(Data::Credential::Get()->domain_name))
				{
					INIT_ZERO_WCHAR(username_domainname, 129);

					wcscat_s(username_domainname, sizeof(username_domainname) / sizeof(wchar_t), Data::Credential::Get()->user_name);
					wcscat_s(username_domainname, sizeof(username_domainname) / sizeof(wchar_t), L"@");
					wcscat_s(username_domainname, sizeof(username_domainname) / sizeof(wchar_t), Data::Credential::Get()->domain_name);

					hr = SHStrDupW(username_domainname, &rgFieldStrings[field_index]);
				}
				else if (NOT_EMPTY(Data::Credential::Get()->user_name))
					hr = SHStrDupW(Data::Credential::Get()->user_name, &rgFieldStrings[field_index]);
			}
			else if (NOT_EMPTY(Data::Credential::Get()->user_name)) {
				hr = SHStrDupW(Data::Credential::Get()->user_name, &rgFieldStrings[field_index]);
			}
			else
				hr = SHStrDupW(value, &rgFieldStrings[field_index]);
			//debug(rgFieldStrings[field_index]);
			break;
		case FIT_NONE:
			//debug("...FIT_NONE");
			break;
		default:
			hr = SHStrDupW(L"", &rgFieldStrings[field_index]);
			//debug("default:");
			//debug(rgFieldStrings[field_index]);
			break;
		}

		return hr;
	}

	HRESULT KerberosLogon(
		__out CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE*& pcpgsr,
		__out CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION*& pcpcs,
		__in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
		__in PWSTR username,
		__in PWSTR password,
		__in PWSTR domain
	)
	{
		debug(__FUNCTION__);

		HRESULT hr;

		WCHAR wsz[MAX_SIZE_DOMAIN];
		DWORD cch = ARRAYSIZE(wsz);
		BOOL  bGetCompName = false;

		if (EMPTY(domain))
			bGetCompName = GetComputerNameW(wsz, &cch);

		if (bGetCompName)
			domain = wsz;

		debug("USERNAME: %S\n", username);

		if (domain != NULL || bGetCompName)
		{
			PWSTR pwzProtectedPassword;

			debug("DOMAIN: %S\n", domain);
			hr = ProtectIfNecessaryAndCopyPassword(password, cpus, &pwzProtectedPassword);

			if (SUCCEEDED(hr))
			{
				KERB_INTERACTIVE_UNLOCK_LOGON kiul;

				debug("Protected Password !\n");

				// Initialize kiul with weak references to our credential.
				hr = KerbInteractiveUnlockLogonInit(domain, username, const_cast<PWSTR>(pwzProtectedPassword), cpus, &kiul);

				if (SUCCEEDED(hr))
				{
					debug("KerbInteractiveUnlockLogonInit !\n");
					// We use KERB_INTERACTIVE_UNLOCK_LOGON in both unlock and logon scenarios.  It contains a
					// KERB_INTERACTIVE_LOGON to hold the creds plus a LUID that is filled in for us by Winlogon
					// as necessary.
					hr = KerbInteractiveUnlockLogonPack(kiul, &pcpcs->rgbSerialization, &pcpcs->cbSerialization);


					if (SUCCEEDED(hr))
					{
						ULONG ulAuthPackage;

						debug("KerbInteractiveUnlockLogonPack !\n");

						hr = RetrieveNegotiateAuthPackage(&ulAuthPackage);

						if (SUCCEEDED(hr))
						{
							pcpcs->ulAuthenticationPackage = ulAuthPackage;
							pcpcs->clsidCredentialProvider = CLSID_SCZ_CP;

							debug("RetrieveNegotiateAuthPackage !\n");
							//debug("Packing of KERB_INTERACTIVE_UNLOCK_LOGON successful");
							// At self point the credential has created the serialized credential used for logon
							// By setting self to CPGSR_RETURN_CREDENTIAL_FINISHED we are letting logonUI know
							// that we have all the information we need and it should attempt to submit the 
							// serialized credential.
							*pcpgsr = CPGSR_RETURN_CREDENTIAL_FINISHED;
						} else
							debug(" NOT RetrieveNegotiateAuthPackage !\n");
					} else
						debug(" NOT KerbInteractiveUnlockLogonPack !\n");
				} else
					debug(" NOT KerbInteractiveUnlockLogonInit !\n");
					
				CoTaskMemFree(pwzProtectedPassword);
			} else
				debug(" NOT Protected Password !\n");
		}
		else
		{
			DWORD dwErr = GetLastError();
			hr = HRESULT_FROM_WIN32(dwErr);
		}

		return hr;
	}

	HRESULT CredPackAuthentication(
		__out CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE*& pcpgsr,
		__out CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION*& pcpcs,
		__in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
		__in PWSTR username,
		__in PWSTR password,
		__in PWSTR domain
	)
	{
		debug(__FUNCTION__);

		debug("CRED USERNAME: %S\n", username);
		debug("CRED DOMAIN: %S\n", domain);

		PWSTR pwzProtectedPassword;
		HRESULT hr = ProtectIfNecessaryAndCopyPassword(password, cpus, &pwzProtectedPassword);
		
		WCHAR wsz[MAX_SIZE_DOMAIN];
		DWORD cch = ARRAYSIZE(wsz);
		BOOL  bGetCompName = false;

		if (EMPTY(domain))
			bGetCompName = GetComputerNameW(wsz, &cch);

		if (bGetCompName)
			domain = wsz;

		if (SUCCEEDED(hr))
		{
			PWSTR domainUsername = NULL;
			hr = DomainUsernameStringAlloc(domain, username, &domainUsername);

			if (SUCCEEDED(hr))
			{
				DWORD size = 0;
				BYTE* rawbits = NULL;

				if (!CredPackAuthenticationBufferW((CREDUIWIN_PACK_32_WOW & Data::Provider::Get()->credPackFlags) ? CRED_PACK_WOW_BUFFER : 0,
					domainUsername, password, rawbits, &size))
				{
					// We received the necessary size, let's allocate some rawbits
					if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
					{
						rawbits = (BYTE *)HeapAlloc(GetProcessHeap(), 0, size);

						if (!CredPackAuthenticationBufferW((CREDUIWIN_PACK_32_WOW & Data::Provider::Get()->credPackFlags) ? CRED_PACK_WOW_BUFFER : 0,
							domainUsername, password, rawbits, &size))
						{
							HeapFree(GetProcessHeap(), 0, rawbits);
							HeapFree(GetProcessHeap(), 0, domainUsername);

							hr = HRESULT_FROM_WIN32(GetLastError());
						}
						else
						{
							pcpcs->rgbSerialization = rawbits;
							pcpcs->cbSerialization = size;
						}
					}
					else
					{
						HeapFree(GetProcessHeap(), 0, domainUsername);
						hr = HRESULT_FROM_WIN32(GetLastError());
					}
				}

				if (SUCCEEDED(hr))
				{
					ULONG ulAuthPackage;
					hr = RetrieveNegotiateAuthPackage(&ulAuthPackage);

					if (SUCCEEDED(hr))
					{
						pcpcs->ulAuthenticationPackage = ulAuthPackage;
						pcpcs->clsidCredentialProvider = CLSID_SCZ_CP;

						// At self point the credential has created the serialized credential used for logon
						// By setting self to CPGSR_RETURN_CREDENTIAL_FINISHED we are letting logonUI know
						// that we have all the information we need and it should attempt to submit the 
						// serialized credential.
						*pcpgsr = CPGSR_RETURN_CREDENTIAL_FINISHED;
					}
				}
			}

			CoTaskMemFree(pwzProtectedPassword);
		}

		return hr;
	}


} // Namespace scenarios
