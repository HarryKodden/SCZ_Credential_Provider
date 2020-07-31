#ifndef _DATA_H
#define _DATA_H
#pragma once

#include "helpers.h"

#define MAX_BUFFER_SIZE_PASSWORD 2048
#define MAX_BUFFER_SIZE_NAMES 256

namespace Data
{	
	namespace Gui
	{
		struct GUI
		{
			wchar_t user_name[MAX_BUFFER_SIZE_NAMES];
			wchar_t domain_name[MAX_BUFFER_SIZE_NAMES];
			wchar_t token[MAX_BUFFER_SIZE_NAMES];
		};

		GUI*& Get();
		void Init();
		void Deinit();
		void Default();
	}

	namespace Provider
	{
		struct PROVIDER
		{
			ICredentialProviderEvents* _pcpe;
			UINT_PTR _upAdviseContext;

			CREDENTIAL_PROVIDER_USAGE_SCENARIO usage_scenario;
			DWORD credPackFlags;
		};

		PROVIDER*& Get();
		void Init();
		void Deinit();
	}

	namespace Credential
	{
		struct CREDENTIAL
		{
			PWSTR user_name;
			PWSTR domain_name;
			PWSTR ldap_password;
			
			IQueryContinueWithStatus* pqcws = NULL;
		};

		CREDENTIAL*& Get();
		void Init();
		void Deinit();
		void Default();
	}
}

#endif