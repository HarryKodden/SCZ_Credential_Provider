/* * * * * * * * * * * * * * * * * * * * *
**
** Copyright 2012 Dominik Pretzsch
**
**    Licensed under the Apache License, Version 2.0 (the "License");
**    you may not use this file except in compliance with the License.
**    You may obtain a copy of the License at
**
**        http://www.apache.org/licenses/LICENSE-2.0
**
**    Unless required by applicable law or agreed to in writing, software
**    distributed under the License is distributed on an "AS IS" BASIS,
**    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**    See the License for the specific language governing permissions and
**    limitations under the License.
**
** * * * * * * * * * * * * * * * * * * */

#include "stdafx.h"

UINT __stdcall SanitizeDwordFromRegistry(MSIHANDLE hInstall)
{
	HRESULT hr = S_OK;
	UINT er = ERROR_SUCCESS;

	hr = WcaInitialize(hInstall, "SanitizeDword");
	ExitOnFailure(hr, "Failed to initialize");

	WcaLog(LOGMSG_STANDARD, "Initialized.");

	///
	wchar_t cPropertyName[MAX_PATH];
	wchar_t cPropertyValue[MAX_PATH];

	DWORD dwMaxLen = MAX_PATH;

	// TODO: Support multiple properties (separated by comma)
	MsiGetProperty(hInstall, L"SANITIZE_DWORD", cPropertyName, &dwMaxLen);
	MsiGetProperty(hInstall, cPropertyName, cPropertyValue, &dwMaxLen);

	if (cPropertyValue[0] == '#')
	{
		WcaLog(LOGMSG_STANDARD, "Property %s needs sanitation...", cPropertyName);
		for (unsigned int i = 1; i < dwMaxLen; i++)
		{
			cPropertyValue[i - 1] = cPropertyValue[i];
			cPropertyValue[i] = NULL;
		}
		WcaLog(LOGMSG_STANDARD, "Sanitation done.");
	}

	MsiSetProperty(hInstall, cPropertyName, cPropertyValue);
	///

LExit:
	er = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
	return WcaFinalize(er);
}

// DllMain - Initialize and cleanup WiX custom action utils.
extern "C" BOOL WINAPI DllMain(
	__in HINSTANCE hInst,
	__in ULONG ulReason,
	__in LPVOID
)
{
	switch (ulReason)
	{
	case DLL_PROCESS_ATTACH:
		WcaGlobalInitialize(hInst);
		break;

	case DLL_PROCESS_DETACH:
		WcaGlobalFinalize();
		break;
	}

	return TRUE;
}
