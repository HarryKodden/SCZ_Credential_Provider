//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//

#ifndef _DLL_H
#define _DLL_H
#pragma once

#pragma once

// global dll hinstance
extern HINSTANCE g_hinst;
#define HINST_THISDLL g_hinst

void DllAddRef();
void DllRelease();

#endif
