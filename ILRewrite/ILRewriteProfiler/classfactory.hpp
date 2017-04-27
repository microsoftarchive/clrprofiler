// ==++==

//   Copyright (c) Microsoft Corporation.  All rights reserved.

// ==--==

#ifndef __CLASSFACTORY_HPP__
#define __CLASSFACTORY_HPP__

#include "stdafx.h"
#include "ProfilerCallback.h"

// Helpers/Registration
HINSTANCE g_hInst;        // instance handle to this piece of code
const int g_iVersion = 1; // version of coclasses.

static const char *g_szProgIDPrefix   = PROGID_PREFIX;
static const char *g_szThreadingModel = THREADING_MODEL;
static const char *g_szCoclassDesc    = COCLASS_DESCRIPTION;

// create a new instance of an object.
typedef HRESULT (__stdcall * PFN_CREATE_OBJ)(REFIID riid, void **ppInterface);

//************************************************************************************//

//******************         COCLASS_REGISTER Declaration          ******************//

//************************************************************************************//

struct COCLASS_REGISTER
{   
	const GUID *pClsid;             // Class ID of the coclass
	const char *szProgID;           // Prog ID of the class
	PFN_CREATE_OBJ pfnCreateObject; // function to create instance
};

// this map contains the list of coclasses which are exported from this module
const COCLASS_REGISTER g_CoClasses[] = {
	&CLSID_PROFILER,
	PROFILER_GUID,          
	ProfilerCallback::CreateObject,
	NULL,               
	NULL,               
	NULL
};

//************************************************************************************//

//******************          CClassFactory Declaration            ******************//

//************************************************************************************//

class CClassFactory :
	public IClassFactory
{
private:
	CClassFactory();                        

public:
	CClassFactory(const COCLASS_REGISTER *pCoClass);
	~CClassFactory();

public:
	// IUnknown 
	COM_METHOD(ULONG) AddRef();       
	COM_METHOD(ULONG) Release();
	COM_METHOD(HRESULT) QueryInterface(REFIID riid, void **ppInterface);            

	// IClassFactory 
	COM_METHOD(HRESULT) LockServer(BOOL fLock);
	COM_METHOD(HRESULT) CreateInstance(IUnknown *pUnkOuter,
		REFIID riid,
		void **ppInterface);

private:
	long m_refCount;                        
	const COCLASS_REGISTER *m_pCoClass;     

};

//************************************************************************************//

//******************          CClassFactory Implementation         ******************//

//************************************************************************************//

// <EMPTY> [private] CCLassFactory construction. Does nothing.
CClassFactory::CClassFactory()
{    
}

// [public] CClassFactory construction, registering one item as pCoClass.
CClassFactory::CClassFactory(const COCLASS_REGISTER *pCoClass) :
	m_refCount(1), 
	m_pCoClass(pCoClass)
{    
}

// <EMPTY> [public] CClassFactory destruction. Does nothing (nothing to release anymore!).
CClassFactory::~CClassFactory()
{
}

// [public] Adds a reference to this ClassFactory.
ULONG CClassFactory::AddRef()
{
	return InterlockedIncrement(&m_refCount);
}

// [public] Removes a reference to this ClassFactory and, if no longer referenced, deletes it.
ULONG CClassFactory::Release()
{    
	long refCount = InterlockedDecrement(&m_refCount);
	if (refCount == 0) 
		delete this;

	return refCount;
}

// [public] Quesries the type of interface of this object.
HRESULT CClassFactory::QueryInterface(REFIID riid, void **ppInterface)
{    
	if (riid == IID_IUnknown)
	{
		*ppInterface = static_cast<IUnknown *>(this); 
	}
	else if (riid == IID_IClassFactory)
	{
		*ppInterface = static_cast<IClassFactory *>(this);
	}
	else
	{
		*ppInterface = NULL;                  
		return E_NOINTERFACE;
	}

	reinterpret_cast<IUnknown *>(*ppInterface)->AddRef();

	return S_OK;
}

// [public] Cretes an instance of the ppInstance.
HRESULT CClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppInstance)
{       
	// aggregation is not supported by these objects
	if (pUnkOuter != NULL)
		return CLASS_E_NOAGGREGATION;

	// ask the object to create an instance of itself, and check the iid.
	return (*m_pCoClass->pfnCreateObject)(riid, ppInstance);
}

// <EMPTY> [public] We are not required to hook any logic since this is always an in-process server.
HRESULT CClassFactory::LockServer(BOOL fLock)
{    
	return S_OK;
}

#endif