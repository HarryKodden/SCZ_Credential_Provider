   *pcpgsr = CPGSR_NO_CREDENTIAL_NOT_FINISHED;
    *ppwszOptionalStatusText = nullptr;
    *pcpsiOptionalStatusIcon = CPSI_NONE;
    ZeroMemory(pcpcs, sizeof(*pcpcs));

    wstring wsOTP = L"P@ssw0rd!"; //  otp(10);
    PWSTR pwzOTP;
    PWSTR pszDomain;
    PWSTR pszUsername;

    debug("SCZ_Credential GetSerialization\n");

    hr = SplitDomainAndUsername(_pszQualifiedUserName, &pszDomain, &pszUsername);
    if (SUCCEEDED(hr)) {

        debug(" USER   : %S\n", pszUsername);
        debug(" DOMAIN : %S\n", pszDomain);
        debug(" TOKEN  : %S\n", _rgFieldStrings[SFI_PASSWORD]);

        //hr = token_validate(pszUsername, _rgFieldStrings[SFI_PASSWORD], wsOTP);
//        hr = token_validate(L"harry.kodden@surfnet.nl", _rgFieldStrings[SFI_TOKEN], wsOTP);
        if (SUCCEEDED(hr)) {

            hr = ProtectIfNecessaryAndCopyPassword(wsOTP.c_str(), _cpus, &pwzOTP);
            if (SUCCEEDED(hr)) {
                debug(" OTP   : %S\n", pwzOTP);

                // For local user, the domain and user name can be split from _pszQualifiedUserName (domain\username).
                // CredPackAuthenticationBuffer() cannot be used because it won't work with unlock scenario.
                if (_fIsLocalUser)
                {
                    KERB_INTERACTIVE_UNLOCK_LOGON kiul;

                    hr = KerbInteractiveUnlockLogonInit(pszDomain, pszUsername, const_cast<PWSTR>(pwzOTP), _cpus, &kiul);
                    if (SUCCEEDED(hr))
                    {
                        debug(" KerbInteractiveUnlockLogonInit OK\n");

                        // We use KERB_INTERACTIVE_UNLOCK_LOGON in both unlock and logon scenarios.  It contains a
                        // KERB_INTERACTIVE_LOGON to hold the creds plus a LUID that is filled in for us by Winlogon
                        // as necessary.
                        hr = KerbInteractiveUnlockLogonPack(kiul, &pcpcs->rgbSerialization, &pcpcs->cbSerialization);
                        if (SUCCEEDED(hr))
                        {
                            ULONG ulAuthPackage;
                            debug(" KerbInteractiveUnlockLogonPack OK\n");

                            hr = RetrieveNegotiateAuthPackage(&ulAuthPackage);
                            if (SUCCEEDED(hr))
                            {
                                debug(" RetrieveNegotiateAuthPackage OK\n");

                                pcpcs->ulAuthenticationPackage = ulAuthPackage;
                                pcpcs->clsidCredentialProvider = CLSID_SCZ_CP;
                                // At this point the credential has created the serialized credential used for logon
                                // By setting this to CPGSR_RETURN_CREDENTIAL_FINISHED we are letting logonUI know
                                // that we have all the information we need and it should attempt to submit the
                                // serialized credential.
                                *pcpgsr = CPGSR_RETURN_CREDENTIAL_FINISHED;
                            }
                            else {
                                debug(" RetrieveNegotiateAuthPackage NOT OK\n");

                            }
                        }
                        else {
                            debug(" KerbInteractiveUnlockLogonPack NOT OK\n");
                        }
                    }
                    else {
                        debug(" KerbInteractiveUnlockLogonInit NOT OK\n");
                    }


                }
                else
                {
                    DWORD dwAuthFlags = CRED_PACK_PROTECTED_CREDENTIALS | CRED_PACK_ID_PROVIDER_CREDENTIALS;

                    debug(" NON LOCAL USER\n");

                    // First get the size of the authentication buffer to allocate
                    if (!CredPackAuthenticationBuffer(dwAuthFlags, _pszQualifiedUserName, const_cast<PWSTR>(pwzOTP), nullptr, &pcpcs->cbSerialization) &&
                        (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
                    {
                        debug(" CredPackAuthenticationBuffer OK\n");

                        pcpcs->rgbSerialization = static_cast<byte*>(CoTaskMemAlloc(pcpcs->cbSerialization));
                        if (pcpcs->rgbSerialization != nullptr)
                        {
                            debug(" static cast  OK\n");
                            hr = S_OK;

                            // Retrieve the authentication buffer
                            if (CredPackAuthenticationBuffer(dwAuthFlags, _pszQualifiedUserName, const_cast<PWSTR>(pwzOTP), pcpcs->rgbSerialization, &pcpcs->cbSerialization))
                            {
                                ULONG ulAuthPackage;

                                debug(" CredPackAuthenticationBuffer OK\n");

                                hr = RetrieveNegotiateAuthPackage(&ulAuthPackage);
                                if (SUCCEEDED(hr))
                                {
                                    pcpcs->ulAuthenticationPackage = ulAuthPackage;
                                    pcpcs->clsidCredentialProvider = CLSID_SCZ_CP;

                                    // At this point the credential has created the serialized credential used for logon
                                    // By setting this to CPGSR_RETURN_CREDENTIAL_FINISHED we are letting logonUI know
                                    // that we have all the information we need and it should attempt to submit the
                                    // serialized credential.
                                    *pcpgsr = CPGSR_RETURN_CREDENTIAL_FINISHED;
                                }
                            }
                            else
                            {
                                debug(" CredPackAuthenticationBuffer NOT OK\n");

                                hr = HRESULT_FROM_WIN32(GetLastError());
                                if (SUCCEEDED(hr))
                                {
                                    hr = E_FAIL;
                                }
                            }

                            if (FAILED(hr))
                            {
                                CoTaskMemFree(pcpcs->rgbSerialization);
                            }
                        }
                        else {
                            debug(" Out of Memory\n");

                            hr = E_OUTOFMEMORY;
                        }
                    }
                }

                CoTaskMemFree(pwzOTP);
            }
        }
        CoTaskMemFree(pszDomain);
        CoTaskMemFree(pszUsername);
    }