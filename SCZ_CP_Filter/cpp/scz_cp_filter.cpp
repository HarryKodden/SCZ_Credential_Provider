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

#ifndef WIN32_NO_STATUS
#include <ntstatus.h>
#define WIN32_NO_STATUS
#endif
#include <unknwn.h>

#include "scz_cp_filter.h"
#include "guid.h"
#include "debug.h"
#include "dll.h"

HRESULT SCZ_ProviderFilter::Filter(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,DWORD dwFlags,GUID* rgclsidProviders,BOOL* rgbAllow,DWORD cProviders)
{
	debug("SCZ_ProviderFilter Filter !\n");

	switch (cpus) 
    { 
        case CPUS_LOGON: 
		case CPUS_UNLOCK_WORKSTATION:
			for (DWORD i = 0; i < cProviders; i++) 
            { 
				if ( i < dwFlags )
				{ } 
				if (IsEqualGUID(rgclsidProviders[i], CLSID_SCZ_CP)) {
					rgbAllow[i] = TRUE; 
				} else {
					rgbAllow[i] = FALSE; 
				}
            }
            return S_OK; 
			break;         
        case CPUS_CREDUI: 
        case CPUS_CHANGE_PASSWORD: 
            return E_NOTIMPL; 
        default: 
            return E_INVALIDARG; 
    }     
}

SCZ_ProviderFilter::SCZ_ProviderFilter():
    _cRef(1)
{
	debug("SCZ_ProviderFilter Constructor !\n");

    DllAddRef();
}

SCZ_ProviderFilter::~SCZ_ProviderFilter()
{
	debug("SCZ_ProviderFilter Destructor !\n");

	DllRelease();
}

HRESULT SCZ_ProviderFilter::UpdateRemoteCredential(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION *pcpcsIn, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION *pcpcsOut)
{
	//UNREFERENCED_PARAMETER(pcpsIn);
	//UNREFERENCED_PARAMETER(pcpcsOut);

	debug("SCZ_ProviderFilter UpdateRemoteCredential !\n");

	if (!pcpcsIn) // no point continuing as there are no credentials
		return E_NOTIMPL;

	// copy contents from pcpcsIn to pcpcsOut
	pcpcsOut->ulAuthenticationPackage = pcpcsIn->ulAuthenticationPackage;
	pcpcsOut->cbSerialization = pcpcsIn->cbSerialization;
	pcpcsOut->rgbSerialization = pcpcsIn->rgbSerialization;

	// set target CP to our CP
	pcpcsOut->clsidCredentialProvider = CLSID_SCZ_CP;

	// copy the buffer contents if needed
	if (pcpcsOut->cbSerialization > 0 && (pcpcsOut->rgbSerialization = (BYTE*)CoTaskMemAlloc(pcpcsIn->cbSerialization)) != NULL)
	{
		CopyMemory(pcpcsOut->rgbSerialization, pcpcsIn->rgbSerialization, pcpcsIn->cbSerialization);
		return S_OK;
	}
	else
		return E_NOTIMPL;
}

// Boilerplate code to create our provider.
HRESULT Provider_CreateInstance(__in REFIID riid, __deref_out void** ppv)
{
    HRESULT hr;

	debug("SCZ_ProviderFilter Provider_CreateInstance !\n");

    SCZ_ProviderFilter* pProvider = new SCZ_ProviderFilter();

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
