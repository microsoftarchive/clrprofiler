// ==++==

//   Copyright (c) Microsoft Corporation.  All rights reserved.

// ==--==

#ifndef __DLLMAIN_HPP__
#define __DLLMAIN_HPP__

#include "metahost.h"
#include "ProfilerCallback.h"
#include "regutil.hpp"
#include "classfactory.hpp"

// forward declarations
HINSTANCE GetModuleInst();
STDAPI DllRegisterServer();
STDAPI DllUnregisterServer();

#pragma warning (push)
#pragma warning (disable: 4985) // Windows annotates with declspecs
STDAPI DllGetClassObject(REFCLSID rclsid,    // class desired
								 REFIID riid,       // interface desired
								 LPVOID FAR *ppv);  // return interface pointer
#pragma warning (pop)

//************************************************************************************************//

//******************                   DllMain Implementation                   ******************//

//************************************************************************************************//

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Save off the instance handle for later use
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		g_hInst = hInstance;
		DisableThreadLibraryCalls(hInstance);
		break;

	case DLL_PROCESS_DETACH:
		// lpReserved == NULL means that we called FreeLibrary(). In that case, do nothing.
		if ((lpReserved != NULL) && (g_pCallbackObject != NULL))
		{
			g_pCallbackObject->DllDetachShutdown();
		}
		break;

	default:
		break;
	}

	return TRUE;
}

STDAPI DllRegisterServer()
{
	HRESULT hr = S_OK;
	char  rcModule[_MAX_PATH];
	const COCLASS_REGISTER *pCoClass;

	DllUnregisterServer();
	GetModuleFileNameA(GetModuleInst(), rcModule, _MAX_PATH);

	// Register each item in the coclass list
	for (pCoClass = g_CoClasses; (SUCCEEDED(hr) && (pCoClass->pClsid != NULL)); pCoClass++)
	{
		// register the class with default values
		hr = REGUTIL::RegisterCOMClass(*pCoClass->pClsid,
			g_szCoclassDesc,
			g_szProgIDPrefix,
			g_iVersion,
			pCoClass->szProgID,
			g_szThreadingModel,
			rcModule);
	}

	if (FAILED(hr))
		DllUnregisterServer();

	return hr;
}

STDAPI DllUnregisterServer()
{
	const COCLASS_REGISTER *pCoClass;

	// unregister each item in the coclass list
	for (pCoClass = g_CoClasses; pCoClass->pClsid != NULL; pCoClass++)
	{
		REGUTIL::UnregisterCOMClass(*pCoClass->pClsid,
			g_szProgIDPrefix,
			g_iVersion,
			pCoClass->szProgID);
	}

	return S_OK;
}

#pragma warning(push)
#pragma warning(suppress:6014) // suppress "Leaking memory 'pClassFactory'" warning from prefast
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv)
{
	CClassFactory *pClassFactory;
	const COCLASS_REGISTER *pCoClass;
	HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

	// scan for the right one
	for (pCoClass = g_CoClasses; pCoClass->pClsid != NULL; pCoClass++)
	{
		if (*pCoClass->pClsid == rclsid)
		{
			pClassFactory = new CClassFactory(pCoClass);
			if (pClassFactory != NULL)
			{
				hr = pClassFactory->QueryInterface(riid, ppv);

				pClassFactory->Release();
				break;
			}
			else
			{
				hr = E_OUTOFMEMORY;
				break;
			}
		}
	}

	return hr;
}
#pragma warning(pop)

STDAPI_(char *) GetGUIDAsString()
{
	return PROFILER_GUID;
}

HINSTANCE GetModuleInst()
{
	return g_hInst;
}

#endif