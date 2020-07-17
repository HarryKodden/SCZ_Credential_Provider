#include "hooks.h"
#include "config.h"
#include "data.h"
#include "debug.h"
#include "helpers.h"
#include "scenarios.h"
#include "resource.h"

namespace Hook
{

	namespace Serialization
	{

		DATA*& Get()
		{
			static struct DATA *data = NULL;

			return data;
		}

		void Default()
		{
			struct DATA*& data = Get();

			if (data == NULL)
				return;

			data->pcpcs = NULL;
			data->pcpgsr = NULL;
			data->status_icon = NULL;
			data->status_text = NULL;
			data->pCredProvCredentialEvents = NULL;
			data->pCredProvCredential = NULL;
		}

		void Init()
		{
			struct DATA*& data = Get();

			data = (struct DATA*) malloc(sizeof(struct DATA));

			Default();
		}

		void Deinit()
		{
			struct DATA*& data = Get();

			Default();

			free(data);
			data = NULL;
		}

		HRESULT Initialization(
			/*
			CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE*& pcpgsr,
			CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION*& pcpcs,
			PWSTR*& ppwszOptionalStatusText,
			CREDENTIAL_PROVIDER_STATUS_ICON*& pcpsiOptionalStatusIcon,
			ICredentialProviderCredentialEvents*& pCredProvCredentialEvents,
			ICredentialProviderCredential* pCredProvCredential
			*/)
		{
			debug(__FUNCTION__);

			if (Get() == NULL)
				Init();

			if (Get() == NULL)
				return HOOK_CRITICAL_FAILURE;

			Default();

			/*
			Hook::Serialization::Init(
				pcpgsr,
				pcpcs,
				ppwszOptionalStatusText,
				pcpsiOptionalStatusIcon,
				pCredProvCredentialEvents,
				pCredProvCredential);
			*/

			return S_OK;
		}

		HRESULT EndpointInitialization()
		{
			debug(__FUNCTION__);

			return S_OK;
		}

		HRESULT DataInitialization()
		{
			debug(__FUNCTION__);

			if (Data::Gui::Get() == NULL)
				Data::Gui::Init();

			if (Data::Gui::Get() == NULL || Data::Provider::Get() == NULL)
				return HOOK_CRITICAL_FAILURE;

			// copy GUI fields to internal datastructures (we don't want to touch the GUI values)
			switch (Data::Provider::Get()->usage_scenario)
			{
			case CPUS_LOGON:
			case CPUS_UNLOCK_WORKSTATION:
				if (NOT_EMPTY(Data::Credential::Get()->user_name))
				{
					debug("Loading username from external credential");

					wcscpy_s(Data::Gui::Get()->user_name, sizeof(Data::Gui::Get()->user_name) / sizeof(wchar_t), Data::Credential::Get()->user_name);

					if (NOT_EMPTY(Data::Credential::Get()->domain_name))
					{
						debug("Loading domainname from external credential");
						wcscpy_s(Data::Gui::Get()->domain_name, sizeof(Data::Gui::Get()->domain_name) / sizeof(wchar_t), Data::Credential::Get()->domain_name);
					}
				}
				else
				{
					debug("Loading username/domainname from GUI");

//					SplitDomainAndUsername(Serialization::Get()->field_strings[LUFI_OTP_USERNAME], Data::Gui::Get()->domain_name, Data::Gui::Get()->user_name);

					Helper::SeparateUserAndDomainName(Serialization::Get()->field_strings[LUFI_OTP_USERNAME],
						Data::Gui::Get()->user_name, sizeof(Data::Gui::Get()->user_name) / sizeof(wchar_t),
						Data::Gui::Get()->domain_name, sizeof(Data::Gui::Get()->domain_name) / sizeof(wchar_t)
					);

					if (EMPTY(Data::Gui::Get()->domain_name) && NOT_EMPTY(Data::Credential::Get()->domain_name))
					{
						debug("Loading domainname from external credential, because not provided in GUI");
						// user's choice has always precedence
						wcscpy_s(Data::Gui::Get()->domain_name, sizeof(Data::Gui::Get()->domain_name) / sizeof(wchar_t), Data::Credential::Get()->domain_name);
					}
				}

				debug("Loading OTP from GUI");
				wcscpy_s(Data::Gui::Get()->token, sizeof(Data::Gui::Get()->token) / sizeof(wchar_t), Serialization::Get()->field_strings[LUFI_OTP_TOKEN]);

				break;

			case CPUS_CREDUI:
				debug("Loading username/domainname from GUI");

				Helper::SeparateUserAndDomainName(Serialization::Get()->field_strings[CFI_OTP_USERNAME],
					Data::Gui::Get()->user_name, sizeof(Data::Gui::Get()->user_name) / sizeof(wchar_t),
					Data::Gui::Get()->domain_name, sizeof(Data::Gui::Get()->domain_name) / sizeof(wchar_t)
				);

				if (EMPTY(Data::Gui::Get()->domain_name) && NOT_EMPTY(Data::Credential::Get()->domain_name))
				{
					debug("Loading domainname from external credential, because not provided in GUI");
					// user's choice has always precedence
					wcscpy_s(Data::Gui::Get()->domain_name, sizeof(Data::Gui::Get()->domain_name) / sizeof(wchar_t), Data::Credential::Get()->domain_name);
				}
				
				debug("Loading OTP from GUI");
				wcscpy_s(Data::Gui::Get()->token, sizeof(Data::Gui::Get()->token) / sizeof(wchar_t), Serialization::Get()->field_strings[CFI_OTP_TOKEN]);

				break;

			case CPUS_CHANGE_PASSWORD:
				return E_NOTIMPL;

			default:
				return E_INVALIDARG;
			}

			return S_OK;
		}

		HRESULT DataDeinitialization()
		{
			debug(__FUNCTION__);

			Data::Gui::Deinit();

			return S_OK;
		}

		HRESULT KerberosCallSuccessfull() { return S_OK; }

		HRESULT KerberosCallFailed() { return S_OK; }

		HRESULT BeforeReturn()
		{
			debug(__FUNCTION__);

			Hook::Serialization::Deinit();

			return S_OK;
		}

	} // Namespace Serialization

	namespace CredentialHooks
	{
		HRESULT GetSubmitButtonValue(DWORD dwFieldID, DWORD* &pdwAdjacentTo)
		{
			debug(__FUNCTION__);

			HRESULT hr;

			if (LUFI_OTP_SUBMIT_BUTTON == dwFieldID && pdwAdjacentTo)
			{
				// pdwAdjacentTo is a pointer to the fieldID you want the submit button to appear next to.
				*pdwAdjacentTo = LUFI_OTP_TOKEN;
				hr = S_OK;
			}
			else
			{
				hr = E_INVALIDARG;
			}

			return hr;
		}

		HRESULT GetComboBoxValueCount(DWORD dwFieldID, DWORD* &pcItems, DWORD* &pdwSelectedItem)
		{
			debug(__FUNCTION__);

			UNREFERENCED_PARAMETER(dwFieldID);
			UNREFERENCED_PARAMETER(pcItems);
			UNREFERENCED_PARAMETER(pdwSelectedItem);

			HRESULT hr;

			*pcItems = 0; // ARRAYSIZE(s_rgLogonUnlockComboBoxModeStrings);
			*pdwSelectedItem = 0;
			hr = S_OK;

			return hr;
		}

		HRESULT GetComboBoxValueAt(DWORD dwFieldID, DWORD dwItem, PWSTR* &ppwszItem)
		{
			debug(__FUNCTION__);

			UNREFERENCED_PARAMETER(dwFieldID);
			UNREFERENCED_PARAMETER(dwItem);
			UNREFERENCED_PARAMETER(ppwszItem);

			HRESULT hr;

			hr = E_INVALIDARG; //SHStrDupW(s_rgLogonUnlockComboBoxModeStrings[dwItem], ppwszItem);

			return hr;
		}

		HRESULT SetComboBoxSelectedValue(ICredentialProviderCredential *pSelf, ICredentialProviderCredentialEvents *pCredProvCredentialEvents,
										 DWORD dwFieldID, DWORD dwSelectedItem, DWORD &dwSelectedItemBuffer)
		{
			debug(__FUNCTION__);

			UNREFERENCED_PARAMETER(pSelf);
			UNREFERENCED_PARAMETER(pCredProvCredentialEvents);
			UNREFERENCED_PARAMETER(dwFieldID);
			UNREFERENCED_PARAMETER(dwSelectedItem);
			UNREFERENCED_PARAMETER(dwSelectedItemBuffer);

			HRESULT hr;

			dwSelectedItemBuffer = dwSelectedItem;

			hr = S_OK;

			return hr;
		}

		HRESULT GetCheckboxValue(ICredentialProviderCredential *pSelf, ICredentialProviderCredentialEvents *pCredProvCredentialEvents, wchar_t **rgFieldStrings,
								 DWORD dwFieldID, BOOL *&pbChecked, PWSTR *&ppwszLabel)
		{
			debug(__FUNCTION__);

			UNREFERENCED_PARAMETER(pSelf);
			UNREFERENCED_PARAMETER(pCredProvCredentialEvents);
			UNREFERENCED_PARAMETER(pbChecked);

			if (Data::Gui::Get() == NULL)
				Data::Gui::Init();

			return SHStrDupW(rgFieldStrings[dwFieldID], ppwszLabel);
		}

		HRESULT SetCheckboxValue(ICredentialProviderCredential *pSelf, ICredentialProviderCredentialEvents *pCredProvCredentialEvents, DWORD dwFieldID, BOOL bChecked)
		{
			debug(__FUNCTION__);

			UNREFERENCED_PARAMETER(pSelf);
			UNREFERENCED_PARAMETER(pCredProvCredentialEvents);
			UNREFERENCED_PARAMETER(dwFieldID);
			UNREFERENCED_PARAMETER(bChecked);

			if (Data::Gui::Get() == NULL)
				Data::Gui::Init();

			return S_OK;
		}

		HRESULT GetBitmapValue(HINSTANCE hInstance, DWORD dwFieldID, HBITMAP* phbmp)
		{
			debug(__FUNCTION__);

			HRESULT hr;
			if ((LUFI_OTP_LOGO == dwFieldID) && phbmp)
			{
				HBITMAP hbmp = NULL;

				if (NOT_EMPTY(Configuration::Get()->v1_bitmap_path))
				{
					DWORD dwAttrib = GetFileAttributesA(Configuration::Get()->v1_bitmap_path);

					if (dwAttrib != INVALID_FILE_ATTRIBUTES
						&& !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
					{
						hbmp = (HBITMAP)LoadImageA(NULL, Configuration::Get()->v1_bitmap_path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

						if (hbmp == NULL)
						{
							debug("Error loading Bitmap image\n");
						}
					}
				}

				if (hbmp == NULL)
				{
					hbmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_TILE_IMAGE));
				}

				if (hbmp != NULL)
				{
					hr = S_OK;
					*phbmp = hbmp;
				}
				else
				{
					hr = HRESULT_FROM_WIN32(GetLastError());
				}
			}
			else
			{
				hr = E_INVALIDARG;
			}

			return hr;
		}
	} // Namespace CredentialHooks

	namespace Connect
	{

		HRESULT ChangePassword()
		{
			return E_NOTIMPL;
		}

	} // Namespace Connect

} // Namespace Hook