#ifndef _CONFIG_H
#define _CONFIG_H
#pragma once

#include <windows.h>
#include <winreg.h>
#include <stdio.h>
#include <credentialprovider.h>

#include "helpers.h"
#include "version.h"
#include "debug.h"

#include <string>

using namespace std;

#define REGISTRY_BASE_KEY "SOFTWARE\\SURF\\SCZ-CP"

enum CONF_VALUE
{
	CONF_HOSTNAME = 0,
	CONF_PATH = 1,
	CONF_LOGIN_TEXT = 2,
	CONF_OTP_TEXT = 3,

	CONF_V1_BITMAP_PATH = 4,
	CONF_V2_BITMAP_PATH = 5,

	CONF_NUM_VALUES = 6 // LAST
};

static const LPCSTR s_CONF_VALUES[] =
{
	"hostname", //0
	"path", // 1
	"login_text", // 2 
	"otp_text", // 3
	"v1_bitmap_path", // 4
	"v2_bitmap_path", // 5
};

DWORD readRegistryValueString(__in LPCSTR value, __in LPCSTR key, __in int buffer_size, __deref_out_opt char* data);
DWORD readRegistryValueString( __in CONF_VALUE conf_value, __in int buffer_size, __deref_out_opt char* data);

DWORD readRegistryValueInteger(__in LPCSTR value, __in LPCSTR key, __deref_out_opt int* data);
DWORD readRegistryValueInteger( __in CONF_VALUE conf_value, __deref_out_opt int* data );

DWORD readRegistryValueBinary(__in LPCSTR value, __in LPCSTR key, __in int buffer_size, __deref_out_opt unsigned char* data);

DWORD writeRegistryValueString(__in LPCSTR value, __in LPCSTR key, __in char* data, __in int buffer_size);
DWORD writeRegistryValueString( __in CONF_VALUE conf_value, __in char* data, __in int buffer_size);

DWORD writeRegistryValueInteger(__in LPCSTR value, __in LPCSTR key, __in int data);
DWORD writeRegistryValueInteger( __in CONF_VALUE conf_value, __in int data );

DWORD writeRegistryValueBinary(__in LPCSTR value, __in LPCSTR key, __in unsigned char* data, __in int buffer_size);

namespace Configuration
{
	#define CONFIG_DEFAULT_LOGIN_TEXT ENDPOINT_NAME" Login"
	#define CONFIG_DEFAULT_OTP_TEXT "One-Time Password"
	#define CONFIG_DEFAULT_EMPTY_PATH ""

	struct CONFIGURATION
	{
		char hostname[1024];
		char path[1024];

		char login_text[64];
		char otp_text[64];

		char v1_bitmap_path[1024];
		char v2_bitmap_path[1024];

		int win_ver_major;
		int win_ver_minor;
	};

	CONFIGURATION*& Get();
	void Default();
	void Init();
	void Deinit();

	wstring getRegistry(wstring name);
	void Read();
	void PrintConfig();
	DWORD SaveValueString(CONF_VALUE conf_value, char* value, int size);
	DWORD SaveValueInteger(CONF_VALUE conf_value, int value);
}

#endif
