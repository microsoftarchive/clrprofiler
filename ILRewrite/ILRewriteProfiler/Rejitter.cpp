// ==++==
// 
//   Copyright (c) Microsoft Corporation.  All rights reserved.
// 
// ==--==

#include "stdafx.h"
#include "ProfilerCallback.h"

//---------------------------------------------------------------------------------------
// Exports that managed code from ProfilerHelper.dll will P/Invoke into
// 
// NOTE: Must keep these signatures in sync with the DllImports in ProfilerHelper.cs!
//---------------------------------------------------------------------------------------

EXTERN_C void STDAPICALLTYPE NtvEnteredFunction(
	ModuleID moduleIDCur,
	mdMethodDef mdCur,
	int nVersionCur)
{
	g_pCallbackObject->NtvEnteredFunction(moduleIDCur, mdCur, nVersionCur);
}

EXTERN_C void STDAPICALLTYPE NtvExitedFunction(
	ModuleID moduleIDCur,
	mdMethodDef mdCur,
	int nVersionCur)
{
	g_pCallbackObject->NtvExitedFunction(moduleIDCur, mdCur, nVersionCur);
}
