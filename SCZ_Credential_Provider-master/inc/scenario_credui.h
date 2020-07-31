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

#ifndef _SCENARIO_CREDUI
#define _SCENARIO_CREDUI
#pragma once

#include "field_state_pair.h"
#include "field_initializor.h"

// The indexes of each of the fields in our credential provider's appended tiles.
enum CREDUI_FIELD_ID
{
	CFI_OTP_LOGO = 0,
	CFI_OTP_LARGE_TEXT = 1,
	CFI_OTP_SMALL_TEXT = 2,
	CFI_OTP_USERNAME = 3,
	CFI_OTP_TOKEN = 4,
	CFI_OTP_SUBMIT_BUTTON = 5,
	CFI_NUM_FIELDS = 6,
};

static const FIELD_INITIALIZOR s_rgScenarioCredUiFieldInitializors[] =
{
	{ FIT_NONE, NULL },
	{ FIT_VALUE_OR_LOGIN_TEXT, L"" },
	{ FIT_VALUE_OR_LOCKED_TEXT, L"" },
	{ FIT_VALUE, L"" },
	{ FIT_VALUE, L"" },
	{ FIT_VALUE, L"Submit" },
};

// These two arrays are seperate because a credential provider might
// want to set up a credential with various combinations of field state pairs 
// and field descriptors.

// The field state value indicates whether the field is displayed
// in the selected tile, the deselected tile, or both.
// The Field interactive state indicates when 
static const FIELD_STATE_PAIR s_rgScenarioCredUiFieldStatePairs[] =
{
	{ CPFS_DISPLAY_IN_BOTH, CPFIS_NONE },					// CFI_OTP_LOGO
	{ CPFS_DISPLAY_IN_BOTH, CPFIS_NONE },					// CFI_OTP_LARGE_TEXT
	{ CPFS_HIDDEN, CPFIS_NONE },							// CFI_OTP_SMALL_TEXT
	{ CPFS_DISPLAY_IN_SELECTED_TILE, CPFIS_FOCUSED },		// CFI_OTP_USERNAME
	{ CPFS_DISPLAY_IN_SELECTED_TILE, CPFIS_NONE },			// CFI_OTP_PASS
	{ CPFS_DISPLAY_IN_SELECTED_TILE, CPFIS_NONE },			// CFI_OTP_SUBMIT_BUTTON
};

// Field descriptors for unlock and logon.
// The first field is the index of the field.
// The second is the type of the field.
// The third is the name of the field, NOT the value which will appear in the field.
static const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR s_rgScenarioCredUiCredProvFieldDescriptors[] =
{
	{ CFI_OTP_LOGO, CPFT_TILE_IMAGE, L"Logo" },
	{ CFI_OTP_LARGE_TEXT, CPFT_LARGE_TEXT, L"LargeText" },
	{ CFI_OTP_SMALL_TEXT, CPFT_SMALL_TEXT, L"SmallText" },
	{ CFI_OTP_USERNAME, CPFT_EDIT_TEXT, L"Username" },
	{ CFI_OTP_TOKEN, CPFT_PASSWORD_TEXT, L"Token" },
	{ CFI_OTP_SUBMIT_BUTTON, CPFT_SUBMIT_BUTTON, L"Submit" },
};

static PWSTR s_rgScenarioCredUiComboBoxModeStrings[] =
{
	L"Nothing", // default
};

#endif
