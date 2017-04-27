// ==++==

//   Copyright (c) Microsoft Corporation.  All rights reserved.

// ==--==

//  Implements ICorProfilerCallback. Logs every event of interest to a file on disk.

#include "stdafx.h"
#include "dllmain.hpp"
#include "mscoree.h"
#include "ProfilerCallback.h"
#include <fstream>
#include <iostream>
#include <share.h>
#include <vector>
#include <windows.h>
#include <io.h>
#include <dos.h>
#include <Windows.h>

#undef _WIN32_WINNT
#define _WIN32_WINNT	0x0403

#undef NTDDI_VERSION
#define NTDDI_VERSION	0x04030000

// Maximum buffer size for input from file
#define BUFSIZE 2048

// String for mscorlib methods or not.
#define MSCORLIBCOMMAND L"Inserting into mscorlib: %c\r\n"

// Command strings for communicating out-of-process.
#define CMD_REJITFUNC L"pf"
#define CMD_REVERTFUNC L"rf"
#define CMD_QUIT L"qa"

// Response strings for communicating out-of-process.
#define RSP_REJITSUCCESS L"ps"
#define RSP_REVERTSUCCESS L"rs"
#define RSP_REJITFAILURE L"pf"
#define RSP_REVERTFAILURE L"rf"
#define RSP_QUITSUCCESS L"qs"

// Make sure the probe type matches the computer's architecture
#ifdef _WIN64
LPCWSTR k_wszEnteredFunctionProbeName = L"MgdEnteredFunction64";
LPCWSTR k_wszExitedFunctionProbeName = L"MgdExitedFunction64";
#else // Win32
LPCWSTR k_wszEnteredFunctionProbeName = L"MgdEnteredFunction32";
LPCWSTR k_wszExitedFunctionProbeName = L"MgdExitedFunction32";
#endif

// When pumping managed helpers into mscorlib, stick them into this pre-existing mscorlib type
LPCWSTR k_wszHelpersContainerType = L"System.CannotUnloadAppDomainException";

// Note: Generally you should not have a single, global callback implementation, as that
// prevents your profiler from analyzing multiply loaded in-process side-by-side CLRs.
// However, this profiler implements the "profile-first" alternative of dealing with
// multiple in-process side-by-side CLR instances. First CLR to try to load us into this
// process wins; so there can only be one callback implementation created. (See
// ProfilerCallback::CreateObject.)
ProfilerCallback * g_pCallbackObject = NULL;

IDToInfoMap<ModuleID, ModuleInfo> m_moduleIDToInfoMap;

BOOL g_bShouldExit = FALSE;
BOOL g_bSafeToExit = FALSE;
UINT g_nLastRefid = 0;
std::wofstream g_wLogFile;
std::wofstream g_wResultFile;
BOOL g_fLogFilePathsInitiailized = FALSE;

// I read this file written by GUI to tell me what to do
WCHAR g_wszCmdFilePath[MAX_PATH] = { L'\0' };

// I write to this file to respond to the GUI
WCHAR g_wszResponseFilePath[MAX_PATH] = { L'\0' };

// I write additional diagnostic info to this file
WCHAR g_wszLogFilePath[MAX_PATH] = { L'\0' };

// I write the human-readable profiling results to this (HTML) file
WCHAR g_wszResultFilePath[MAX_PATH] = { L'\0' };

#define HEX(HR) L"0x" << std::hex << std::uppercase << HR << std::dec
#define RESULT_APPEND(EXPR) do { g_wResultFile.open(g_wszResultFilePath, std::ios::app); g_wResultFile << L"\n" << EXPR; g_wResultFile.close(); } while(0)
#define RESPONSE_LITERAL(EXPR) do { std::wofstream ofsLog; ofsLog.open(g_wszResponseFilePath, std::ios::app); ofsLog << EXPR; ofsLog.close(); } while(0)
#define RESPONSE_APPEND(EXPR) RESPONSE_LITERAL(g_nLastRefid << L">" << EXPR << L"\n")
#define RESPONSE_IS(REFID, EXPR, MODULE, CLASS, FUNC) RESPONSE_LITERAL(REFID << L">" << EXPR << L"\t" << MODULE << L"\t" << CLASS << L"\t" << FUNC << L"\n")
#define RESPONSE_ERROR(EXPR) RESPONSE_APPEND(L"ERROR\tError: " << EXPR)
#define LOG_APPEND(EXPR) do { g_wLogFile.open(g_wszLogFilePath, std::ios::app); g_wLogFile << L"\n" << EXPR; g_wLogFile.close(); } while(0)
#define LOG_IFFAILEDRET(HR, EXPR) do { if (FAILED(HR)) { LOG_APPEND(EXPR << L", hr = " << HEX(HR)); return HR; } } while(0)


// [extern] ilrewriter function for rewriting a module's IL
extern HRESULT RewriteIL(
	ICorProfilerInfo * pICorProfilerInfo,
	ICorProfilerFunctionControl * pICorProfilerFunctionControl,
	ModuleID moduleID,
	mdMethodDef methodDef,
	int nVersion,
	mdToken mdEnterProbeRef,
	mdToken mdExitProbeRef);

// [extern] ilrewriter function for setting helper IL
extern HRESULT SetILForManagedHelper(
	ICorProfilerInfo * pICorProfilerInfo,
	ModuleID moduleID,
	mdMethodDef mdHelperToAdd,
	mdMethodDef mdIntPtrExplicitCast,
	mdMethodDef mdPInvokeToCall);

// Struct used to hold the arguments for creating the file watcher thread. 
// Used since threadstart uses a LPVOID parameter for the called function.
struct threadargs
{
	ICorProfilerCallback * m_pCallback;
	LPCWSTR m_wszpath;
	IDToInfoMap<ModuleID, ModuleInfo> * m_iMap;
};

//************************************************************************************************//

//******************                    Forward Declarations                    ******************//            

//************************************************************************************************//

// [private] Checks to see if the given file has any changes, and if so executes the new commands.
DWORD WINAPI MonitorFile(LPVOID pFileAndModuleMap);

// [private] Checks to see if the given file exists.
bool FileExists(const PCWSTR wszFilepath);

// [private] Reads and executes a command from the file.
void ReadFile(FILE * fFile, LPVOID args);

// [private] Gets the MethodDef from the module, class and function names.
BOOL GetTokensFromNames(IDToInfoMap<ModuleID, ModuleInfo> * mMap, LPCWSTR wszModule, LPCWSTR wszClass, LPCWSTR wszFunction, ModuleID * moduleIDs, mdMethodDef * methodDefs, int cElementsMax,  int * pcMethodsFound);

// [private] Returns TRUE iff wszContainer ends with wszProspectiveEnding (case-insensitive).
BOOL ContainsAtEnd(LPCWSTR wszContainer, LPCWSTR wszProspectiveEnding);

// [private] Gets the number of spaces needed for padding.
static LPCWSTR GetPaddingString(int cSpaces)
{
	static const WCHAR k_wszSpaces[] =
		L"                                                                                           ";
	if (cSpaces > _countof(k_wszSpaces) - 1)
	{
		return k_wszSpaces;
	}

	return &(k_wszSpaces[(_countof(k_wszSpaces) - 1) - cSpaces]);
}

//************************************************************************************************//

//******************              ProfilerCallBack  Implementation              ******************//            

//************************************************************************************************//

// [public] Creates a new ProfilerCallback instance
HRESULT ProfilerCallback::CreateObject(REFIID riid, void **ppInterface)
{
    if (!g_fLogFilePathsInitiailized)
    {
        // Determine full paths to the various log files we read and write
        // based on the profilee executable path

        WCHAR wszExeDir[MAX_PATH];

        if (!GetModuleFileName(NULL /* Get exe module */, wszExeDir, _countof(wszExeDir)))
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        LPWSTR wszFinalSeparator = wcsrchr(wszExeDir, L'\\');
        if (wszFinalSeparator == NULL)
            return E_UNEXPECTED;

        *wszFinalSeparator = L'\0';

        if (wcscpy_s(g_wszCmdFilePath, _countof(g_wszCmdFilePath), wszExeDir) != 0)
            return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        if (wcscat_s(g_wszCmdFilePath, _countof(g_wszCmdFilePath), L"\\ILRWP_watchercommands.log"))
            return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);

        if (wcscpy_s(g_wszResponseFilePath, _countof(g_wszResponseFilePath), wszExeDir) != 0)
            return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        if (wcscat_s(g_wszResponseFilePath, _countof(g_wszResponseFilePath), L"\\ILRWP_watcherresponse.log"))
            return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);

        if (wcscpy_s(g_wszLogFilePath, _countof(g_wszLogFilePath), wszExeDir) != 0)
            return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        if (wcscat_s(g_wszLogFilePath, _countof(g_wszLogFilePath), L"\\ILRWP_session.log"))
            return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);

        if (wcscpy_s(g_wszResultFilePath, _countof(g_wszResultFilePath), wszExeDir) != 0)
            return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        if (wcscat_s(g_wszResultFilePath, _countof(g_wszResultFilePath), L"\\ILRWP_RESULT.htm"))
            return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);

        g_fLogFilePathsInitiailized = TRUE;
    }

    *ppInterface = NULL;
    if ((riid != IID_IUnknown) &&
        (riid != IID_ICorProfilerCallback5) &&
        (riid != IID_ICorProfilerCallback) &&
        (riid != IID_ICorProfilerCallback2) &&
        (riid != IID_ICorProfilerCallback3) &&
        (riid != IID_ICorProfilerCallback4))
    {
        return E_NOINTERFACE;
    }

    // This profiler implements the "profile-first" alternative of dealing
    // with multiple in-process side-by-side CLR instances.  First CLR
    // to try to load us into this process wins
    {
        static volatile LONG s_nFirstTime = 1;
        if (s_nFirstTime == 0)
        {
            return CORPROF_E_PROFILER_CANCEL_ACTIVATION;
        }

        // Dirty-read says this is the first load.  Double-check that
        // with a clean-read
        if (InterlockedCompareExchange(&s_nFirstTime, 0, 1) == 0)
        {
            // Someone beat us to it
            return CORPROF_E_PROFILER_CANCEL_ACTIVATION;
        }
    }

    ProfilerCallback * pProfilerCallback = new ProfilerCallback();
    if (pProfilerCallback == NULL)
    {
        return E_OUTOFMEMORY;
    }

    pProfilerCallback->AddRef();
    *ppInterface = static_cast<ICorProfilerCallback2 *>(pProfilerCallback);

    LOG_APPEND(L"Profiler succesfully entered.");

    return S_OK;
}

// [public] Creates a new instance of the profiler and zeroes all members
ProfilerCallback::ProfilerCallback() :
	m_pProfilerInfo(NULL),
	m_fInstrumentationHooksInSeparateAssembly(TRUE),
	m_mdIntPtrExplicitCast(mdTokenNil),
	m_mdEnterPInvoke(mdTokenNil),
	m_mdExitPInvoke(mdTokenNil),
	m_mdEnter(mdTokenNil),
	m_mdExit(mdTokenNil),
	m_modidMscorlib(NULL),
	m_refCount(0),
	m_dwShadowStackTlsIndex(0),

    // Set threshold to a completely arbitrary number for demonstration purposes only. 
    // If a function's inclusive time > m_dwThresholdMs, then the profiler's output will
    // flag the function as "long"
	m_dwThresholdMs(100)
{
	// New instance, reset the global variables.
	g_bShouldExit = FALSE;
	g_bSafeToExit = FALSE;
	g_nLastRefid = 0;

	g_wLogFile.open(g_wszLogFilePath);
	g_wLogFile << L"ILRewriteProfiler Event and Error Log\n" <<
		L"-------------------------------------\n";
	g_wLogFile.close();

	g_pCallbackObject = this;
}

// Empty method.
ProfilerCallback::~ProfilerCallback()
{
}

// [public] IUnknown method, increments refcount to keep track of when to call destructor
ULONG ProfilerCallback::AddRef()
{
	return InterlockedIncrement(&m_refCount);
}

// [public] IUnknown method, decrements refcount and deletes if unreferenced
ULONG ProfilerCallback::Release()
{
	long refCount = InterlockedDecrement(&m_refCount);
	if (refCount == 0)
		delete this;

	return refCount;
}

// [public] IUnknown method, gets the interface (The profiler only supports ICorProfilerCallback5)
HRESULT ProfilerCallback::QueryInterface(REFIID riid, void **ppInterface)
{
	// Get interface from riid
	if (riid == IID_IUnknown)
		*ppInterface = static_cast<IUnknown *>(this);
	else if (riid == IID_ICorProfilerCallback5)
		*ppInterface = static_cast<ICorProfilerCallback5 *>(this);
	else if (riid == IID_ICorProfilerCallback4)
		*ppInterface = static_cast<ICorProfilerCallback4 *>(this);
	else if (riid == IID_ICorProfilerCallback3)
		*ppInterface = static_cast<ICorProfilerCallback3 *>(this);
	else if (riid == IID_ICorProfilerCallback2)
		*ppInterface = static_cast<ICorProfilerCallback2 *>(this);
	else if (riid == IID_ICorProfilerCallback)
		*ppInterface = static_cast<ICorProfilerCallback *>(this);
	else
	{
		*ppInterface = NULL;
		LOG_IFFAILEDRET(E_NOINTERFACE, L"Unsupported Callback type in ::QueryInterface");
	}

	// Interface was successfully inferred, increase its reference count.
	reinterpret_cast<IUnknown *>(*ppInterface)->AddRef();

	return S_OK;
}

// [public] Initializes the profiler using the given (hopefully) instance of ICorProfilerCallback5
HRESULT ProfilerCallback::Initialize(IUnknown *pICorProfilerInfoUnk)
{
	HRESULT hr;

	// Even though there are 5 different ICorProfiler Callbacks, there are presently only 4 Infos
	hr = pICorProfilerInfoUnk->QueryInterface(IID_ICorProfilerInfo4, (void **)&m_pProfilerInfo);
	if (FAILED(hr))
	{
		LOG_IFFAILEDRET(hr, L"QueryInterface for ICorProfilerInfo4 failed in ::Initialize");
        return hr;
	}

	hr = m_pProfilerInfo->SetEventMask(
		COR_PRF_MONITOR_MODULE_LOADS    |
		COR_PRF_MONITOR_ASSEMBLY_LOADS  | 
		COR_PRF_MONITOR_APPDOMAIN_LOADS | 
		COR_PRF_MONITOR_JIT_COMPILATION | 
		COR_PRF_ENABLE_REJIT            |
		COR_PRF_DISABLE_ALL_NGEN_IMAGES);

	LOG_IFFAILEDRET(hr, L"SetEventMask failed in ::Initialize");

	m_dwShadowStackTlsIndex = TlsAlloc();

	LaunchLogListener(g_wszCmdFilePath);

	RESULT_APPEND(L"<html><body><pre>");

	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::Shutdown()
{
	return S_OK;
}

HRESULT ProfilerCallback::DllDetachShutdown()
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::AppDomainCreationStarted(AppDomainID appDomainID)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::AppDomainCreationFinished(AppDomainID appDomainID, HRESULT hrStatus)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::AppDomainShutdownStarted(AppDomainID appDomainID)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::AppDomainShutdownFinished(AppDomainID appDomainID, HRESULT hrStatus)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::AssemblyLoadStarted(AssemblyID assemblyId)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::AssemblyLoadFinished(AssemblyID assemblyId, HRESULT hrStatus)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::AssemblyUnloadStarted(AssemblyID assemblyID)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::AssemblyUnloadFinished(AssemblyID assemblyID, HRESULT hrStatus)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ModuleLoadStarted(ModuleID moduleID)
{
	return S_OK;
}

// [public] 
// A lot of work needs to happen when modules load.  Here, we
//      - add the module to the list of tracked modules for ReJIT
//      - add metadata refs to this module (in case we want to rewrite methods
//          in this module)
//      - add new methodDefs to this module if it's mscorlib.dll AND we're running
//          in the mode where we add probe implementations into mscorlib.dll rather
//          than using ProfilerHelper.dll
//      - create new ReJIT requests in case we're loading another copy of a module
//          (this time into a new unshared AppDomain), for which we'd previously
//          submitted a ReJIT request for the prior copy of the module
HRESULT ProfilerCallback::ModuleLoadFinished(ModuleID moduleID, HRESULT hrStatus)
{
	LPCBYTE pbBaseLoadAddr;
	WCHAR wszName[300];
	ULONG cchNameIn = _countof(wszName);
	ULONG cchNameOut;
	AssemblyID assemblyID;
	DWORD dwModuleFlags;

	HRESULT hr = m_pProfilerInfo->GetModuleInfo2(
		moduleID,
		&pbBaseLoadAddr,
		cchNameIn,
		&cchNameOut,
		wszName,
		&assemblyID,
		&dwModuleFlags);

	LOG_IFFAILEDRET(hr, L"GetModuleInfo2 failed for ModuleID = " << HEX(moduleID));

    if ((dwModuleFlags & COR_PRF_MODULE_WINDOWS_RUNTIME) != 0)
    {
        // Ignore any Windows Runtime modules.  We cannot obtain writeable metadata
        // interfaces on them or instrument their IL
        return S_OK;
    }

	AppDomainID appDomainID;
	ModuleID modIDDummy;
	hr = m_pProfilerInfo->GetAssemblyInfo(
		assemblyID,
		0,          // cchName,
		NULL,       // pcchName,
		NULL,       // szName[] ,
		&appDomainID,
		&modIDDummy);

	LOG_IFFAILEDRET(hr, L"GetAssemblyInfo failed for assemblyID = " << HEX(assemblyID));

	WCHAR wszAppDomainName[200];
	ULONG cchAppDomainName;
	ProcessID pProcID;
	BOOL fShared = FALSE;

	hr = m_pProfilerInfo->GetAppDomainInfo(
		appDomainID,
		_countof(wszAppDomainName),
		&cchAppDomainName,
		wszAppDomainName,
		&pProcID);

	LOG_IFFAILEDRET(hr, L"GetAppDomainInfo failed for appDomainID = " << HEX(appDomainID));

	LOG_APPEND(L"ModuleLoadFinished for " << wszName << L", ModuleID = " << HEX(moduleID) <<
		L", LoadAddress = " << HEX(pbBaseLoadAddr) << L", AppDomainID = " << HEX(appDomainID) <<
		L", ADName = " << wszAppDomainName);

    BOOL fPumpHelperMethodsIntoThisModule = FALSE;
    if (::ContainsAtEnd(wszName, L"mscorlib.dll"))
    {
        m_modidMscorlib = moduleID;
        if (!m_fInstrumentationHooksInSeparateAssembly)
        {
            fPumpHelperMethodsIntoThisModule = TRUE;
        }
    }

    // Grab metadata interfaces 

    COMPtrHolder<IMetaDataEmit> pEmit;
    {
        COMPtrHolder<IUnknown> pUnk;

        hr = m_pProfilerInfo->GetModuleMetaData(moduleID, ofWrite, IID_IMetaDataEmit, &pUnk);
        LOG_IFFAILEDRET(hr, L"IID_IMetaDataEmit: GetModuleMetaData failed for ModuleID = " <<
            HEX(moduleID) << L" (" << wszName << L")");

        hr = pUnk->QueryInterface(IID_IMetaDataEmit, (LPVOID *) &pEmit);
        LOG_IFFAILEDRET(hr, L"IID_IMetaDataEmit: QueryInterface failed for ModuleID = " <<
            HEX(moduleID) << L" (" << wszName << L")");
    }

    COMPtrHolder<IMetaDataImport> pImport;
    {
        COMPtrHolder<IUnknown> pUnk;

        hr = m_pProfilerInfo->GetModuleMetaData(moduleID, ofRead, IID_IMetaDataImport, &pUnk);
        LOG_IFFAILEDRET(hr, L"IID_IMetaDataImport: GetModuleMetaData failed for ModuleID = " << 
            HEX(moduleID) << L" (" << wszName << L")");

        hr = pUnk->QueryInterface(IID_IMetaDataImport, (LPVOID *) &pImport);
        LOG_IFFAILEDRET(hr, L"IID_IMetaDataImport: QueryInterface failed for ModuleID = " <<
            HEX(moduleID) << L" (" << wszName << L")");
    }

    if (fPumpHelperMethodsIntoThisModule)
    {
        AddHelperMethodDefs(pImport, pEmit);
    }

    // Store module info in our list

    LOG_APPEND(L"Adding module to list...");

    ModuleInfo moduleInfo = {0};
    if (wcscpy_s(moduleInfo.m_wszModulePath, _countof(moduleInfo.m_wszModulePath), wszName) != 0)
    {
        LOG_IFFAILEDRET(E_FAIL, L"Failed to store module path '" << wszName << L"'");
    }

    // Store metadata reader alongside the module in the list.
    moduleInfo.m_pImport = pImport;
    moduleInfo.m_pImport->AddRef();

    moduleInfo.m_pMethodDefToLatestVersionMap = new MethodDefToLatestVersionMap();

    if (fPumpHelperMethodsIntoThisModule)
    {
        // We're operating on mscorlib and the helper methods are being pumped directly into it.
        // So we reference (from within mscorlib) the helpers via methodDefs, not memberRefs.

        assert(m_mdEnter != mdTokenNil);
        assert(m_mdExit != mdTokenNil);
        moduleInfo.m_mdEnterProbeRef = m_mdEnter;
        moduleInfo.m_mdExitProbeRef = m_mdExit;
    }
    else
    {
        // Add the references to our helper methods.

        COMPtrHolder<IMetaDataAssemblyEmit> pAssemblyEmit;
        {
            COMPtrHolder<IUnknown> pUnk;

            hr = m_pProfilerInfo->GetModuleMetaData(moduleID, ofWrite, IID_IMetaDataAssemblyEmit, &pUnk);
            LOG_IFFAILEDRET(hr, L"IID_IMetaDataEmit: GetModuleMetaData failed for ModuleID = " <<
                HEX(moduleID) << L" (" << wszName << L")");

            hr = pUnk->QueryInterface(IID_IMetaDataAssemblyEmit, (LPVOID *) &pAssemblyEmit);
            LOG_IFFAILEDRET(hr, L"IID_IMetaDataEmit: QueryInterface failed for ModuleID = " <<
                HEX(moduleID) << L" (" << wszName << L")");
        }

        COMPtrHolder<IMetaDataAssemblyImport> pAssemblyImport;
        {
            COMPtrHolder<IUnknown> pUnk;

            hr = m_pProfilerInfo->GetModuleMetaData(moduleID, ofRead, IID_IMetaDataAssemblyImport, &pUnk);
            LOG_IFFAILEDRET(hr, L"IID_IMetaDataImport: GetModuleMetaData failed for ModuleID = " << 
                HEX(moduleID) << L" (" << wszName << L")");

            hr = pUnk->QueryInterface(IID_IMetaDataAssemblyImport, (LPVOID *) &pAssemblyImport);
            LOG_IFFAILEDRET(hr, L"IID_IMetaDataImport: QueryInterface failed for ModuleID = " <<
                HEX(moduleID) << L" (" << wszName << L")");
        }

        AddMemberRefs(pAssemblyImport, pAssemblyEmit, pEmit, &moduleInfo);
    }

    // Append to the list!
    m_moduleIDToInfoMap.Update(moduleID, moduleInfo);
    LOG_APPEND(L"Successfully added module to list.");

    // If we already rejitted functions in other modules with a matching path, then
    // pre-rejit those functions in this module as well.  This takes care of the case
    // where we rejitted functions in a module loaded in one domain, and we just now
    // loaded the same module (unshared) into another domain.  We must explicitly ask to
    // rejit those functions in this domain's copy of the module, since it's identified
    // by a different ModuleID.

    std::vector<ModuleID> rgModuleIDs;
    std::vector<mdToken> rgMethodDefs;

    // Find all modules matching the name in this script entry
    {
        ModuleIDToInfoMap::LockHolder lockHolder(&m_moduleIDToInfoMap);

        // Get the methodDef map for the Module just loaded handy
        MethodDefToLatestVersionMap * pMethodDefToLatestVersionMap = 
            m_moduleIDToInfoMap.Lookup(moduleID).m_pMethodDefToLatestVersionMap;
        assert(pMethodDefToLatestVersionMap != NULL);

        ModuleIDToInfoMap::Const_Iterator iterator;
        for (iterator = m_moduleIDToInfoMap.Begin();
            iterator != m_moduleIDToInfoMap.End();
            ++iterator)
        {
            // Skip the entry we just added for this module
            if (iterator->first == moduleID)
            {
                continue;
            }

            const ModuleInfo * pModInfo = &(iterator->second);
            LPCWSTR wszModulePathCur = &(pModInfo->m_wszModulePath[0]);

            // We only care if the full path of the module from our internal
            // module list == full path of module just loaded
            if (_wcsicmp(wszModulePathCur, wszName) != 0)
            {
                continue;
            }

            // The module is a match!
            MethodDefToLatestVersionMap::Const_Iterator iterMethodDef;
            for (iterMethodDef = pModInfo->m_pMethodDefToLatestVersionMap->Begin();
                iterMethodDef != pModInfo->m_pMethodDefToLatestVersionMap->End();
                iterMethodDef++)
            {
                if (iterMethodDef->second == 0)
                {
                    // We have reverted this method, do not pre-rejit.
                    continue;
                }

                // NOTE: We may have already added this methodDef if it was rejitted in
                // multiple modules.  That means the array will have dupes.  It would be
                // wise to eliminate dupes before forcing the CLR to iterate over the
                // same methodDef multiple times (for performance reasons), but this is
                // just a sample.  Real profilers should be better than this.
                rgModuleIDs.push_back(moduleID);
                rgMethodDefs.push_back(iterMethodDef->first);

                // Remember the latest version number for this mdMethodDef
                pMethodDefToLatestVersionMap->Update(iterMethodDef->first, iterMethodDef->second);
            }
        }
    }

    if (rgMethodDefs.size() > 0)
    {
        LOG_APPEND(L"Auto-pre-rejitting " << rgMethodDefs.size() << L"  methods for modules that have just loaded into an AppDomain different from that containing a module from a prior ReJIT request.");
        CallRequestReJIT((UINT) rgMethodDefs.size(), rgModuleIDs.data(), rgMethodDefs.data());
    }

    return S_OK;
}

// Don't forget--modules can unload!  Remove it from our records when it does.
HRESULT ProfilerCallback::ModuleUnloadStarted(ModuleID moduleID)
{
	LOG_APPEND(L"ModuleUnloadStarted: ModuleID = " << HEX(moduleID) << L".");

	ModuleIDToInfoMap::LockHolder lockHolder(&m_moduleIDToInfoMap);
	ModuleInfo moduleInfo;

	if (m_moduleIDToInfoMap.LookupIfExists(moduleID, &moduleInfo))
	{
		LOG_APPEND(L"Module found in list.  Removing...");
		m_moduleIDToInfoMap.Erase(moduleID);
	}
	else
	{
		LOG_APPEND(L"Module not found in list.  Do nothing.");
	}

	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ModuleUnloadFinished(ModuleID moduleID, HRESULT hrStatus)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ModuleAttachedToAssembly(ModuleID moduleID, AssemblyID assemblyID)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ClassLoadStarted(ClassID classID)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ClassLoadFinished(ClassID classID, HRESULT hrStatus)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ClassUnloadStarted(ClassID classID)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ClassUnloadFinished(ClassID classID, HRESULT hrStatus)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::FunctionUnloadStarted(FunctionID functionID)
{
	return S_OK;
}

// [public] Creates the IL for the managed leave/enter helpers.
void ProfilerCallback::SetILFunctionBodyForManagedHelper(ModuleID moduleID, mdMethodDef methodDef)
{
	assert(!m_fInstrumentationHooksInSeparateAssembly);
	assert(moduleID == m_modidMscorlib);
	assert((methodDef == m_mdEnter) || (methodDef == m_mdExit));

	HRESULT hr = SetILForManagedHelper(
		m_pProfilerInfo,
		moduleID,
		methodDef,
		m_mdIntPtrExplicitCast,
		(methodDef == m_mdEnter) ? m_mdEnterPInvoke : m_mdExitPInvoke);

	if (FAILED(hr))
	{
		LOG_APPEND(L"SetILForManagedHelper failed for methodDef = " << HEX(methodDef) << L"--" <<
			((methodDef == m_mdEnter) ? L"enter" : L"exit") << L", hr = " << HEX(hr));
	}
}

// [public] Checks if the module is mscorlib, and if the CLR is trying to JIT a probe we
// dynamically added to mscorlib.  If so, this function provides the IL for the probe.
HRESULT ProfilerCallback::JITCompilationStarted(FunctionID functionID, BOOL fIsSafeToBlock)
{
	HRESULT hr;
	mdToken methodDef;
	ClassID classID;
	ModuleID moduleID;

	hr = m_pProfilerInfo->GetFunctionInfo(functionID, &classID, &moduleID, &methodDef);
	LOG_IFFAILEDRET(hr, L"GetFunctionInfo failed for FunctionID = " << HEX(functionID));

	if ((moduleID == m_modidMscorlib) &&
		((methodDef == m_mdEnter) || (methodDef == m_mdExit)))
	{
		SetILFunctionBodyForManagedHelper(moduleID, methodDef);
	}

	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::JITCompilationFinished(FunctionID functionID, HRESULT hrStatus, BOOL fIsSafeToBlock)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::JITCachedFunctionSearchStarted(FunctionID functionID, BOOL *pbUseCachedFunction)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::JITCachedFunctionSearchFinished(FunctionID functionID, COR_PRF_JIT_CACHE result)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::JITFunctionPitched(FunctionID functionID)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::JITInlining(FunctionID callerID, FunctionID calleeID, BOOL *pfShouldInline)
{
	return S_OK;
}

// [public] When a ReJIT starts, profilers don't typically need to do much in this
// method.  Here, we just do some light validation and logging.
HRESULT ProfilerCallback::ReJITCompilationStarted(FunctionID functionID, ReJITID rejitId, BOOL fIsSafeToBlock)
{
	LOG_APPEND(L"ReJITScript::ReJITCompilationStarted for FunctionID '" << HEX(functionID) << 
		L"' - RejitID '" << HEX(rejitId) << L"' called");

	HRESULT hr;
	mdToken methodDef;
	ClassID classID;
	ModuleID moduleID;

	hr = m_pProfilerInfo->GetFunctionInfo(functionID, &classID, &moduleID, &methodDef);
	LOG_IFFAILEDRET(hr, L"GetFunctionInfo failed for FunctionID =" << HEX(functionID));

	ModuleInfo moduleInfo = m_moduleIDToInfoMap.Lookup(moduleID);
	int nVersion = moduleInfo.m_pMethodDefToLatestVersionMap->Lookup(methodDef);
	if (nVersion == 0)
	{
		LOG_APPEND(L"ReJITCompilationStarted called for FunctionID = " << HEX(functionID) <<
			L", which should have been reverted.");
		return E_FAIL;
	}

	LOG_APPEND(L"Found latest version number of " << nVersion <<
		L" for rejitting function. Associating it with rejitID. (FunctionID = " << HEX(functionID) <<
		L", RejitID = " << HEX(rejitId) << L", mdMethodDef = " << HEX(methodDef) << L").");


	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ReJITCompilationFinished(FunctionID functionId, ReJITID rejitId, HRESULT hrStatus, BOOL fIsSafeToBlock)
{
	return S_OK;
}

// [public] Logs any errors encountered during ReJIT.
HRESULT ProfilerCallback::ReJITError(ModuleID moduleId, mdMethodDef methodId, FunctionID functionId, HRESULT hrStatus)
{
	LOG_IFFAILEDRET(hrStatus, L"ReJITError called.  ModuleID = " << HEX(moduleId) <<
		L", methodDef = " << HEX(methodId) << L", FunctionID = " << HEX(functionId));

	return S_OK;
}

// [public] Here's where the real work happens when a method gets ReJITed.  This is
// responsible for getting the new (instrumented) IL to be compiled.
HRESULT ProfilerCallback::GetReJITParameters(ModuleID moduleId, mdMethodDef methodId, ICorProfilerFunctionControl *pFunctionControl)
{
	LOG_APPEND(L"ReJITScript::GetReJITParameters called, methodDef = " << HEX(methodId));

	ModuleInfo moduleInfo = m_moduleIDToInfoMap.Lookup(moduleId);
	HRESULT hr;

	int nVersion;
	moduleInfo.m_pMethodDefToLatestVersionMap->LookupIfExists(methodId, &nVersion);

	hr = RewriteIL(
		m_pProfilerInfo,
		pFunctionControl,
		moduleId,
		methodId,
		nVersion,
		moduleInfo.m_mdEnterProbeRef,
		moduleInfo.m_mdExitProbeRef);

	LOG_IFFAILEDRET(hr, L"RewriteIL failed for ModuleID = " << HEX(moduleId) <<
		L", methodDef = " << HEX(methodId));

	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::MovedReferences2(ULONG cMovedObjectIDRanges, ObjectID oldObjectIDRangeStart[], ObjectID newObjectIDRangeStart[], SIZE_T cObjectIDRangeLength[])
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::SurvivingReferences2(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[], SIZE_T   cObjectIDRangeLength[])
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ConditionalWeakTableElementReferences(ULONG cRootRefs, ObjectID keyRefIds[], ObjectID valueRefIds[], GCHandleID rootIds[])
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ThreadCreated(ThreadID threadID)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ThreadDestroyed(ThreadID threadID)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ThreadAssignedToOSThread(ThreadID managedThreadID, DWORD osThreadID)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::RemotingClientInvocationStarted()
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::RemotingClientSendingMessage(GUID *pCookie, BOOL fIsAsync)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::RemotingClientReceivingReply(GUID *pCookie, BOOL fIsAsync)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::RemotingClientInvocationFinished()
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::RemotingServerInvocationStarted()
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::RemotingServerReceivingMessage(GUID *pCookie, BOOL fIsAsync)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::RemotingServerSendingReply(GUID *pCookie, BOOL fIsAsync)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::RemotingServerInvocationReturned()
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::UnmanagedToManagedTransition(FunctionID functionID, COR_PRF_TRANSITION_REASON reason)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ManagedToUnmanagedTransition(FunctionID functionID, COR_PRF_TRANSITION_REASON reason)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::RuntimeSuspendStarted(COR_PRF_SUSPEND_REASON suspendReason)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::RuntimeSuspendFinished()
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::RuntimeSuspendAborted()
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::RuntimeResumeStarted()
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::RuntimeResumeFinished()
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::RuntimeThreadSuspended(ThreadID threadID)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::RuntimeThreadResumed(ThreadID threadID)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::MovedReferences(ULONG cmovedObjectIDRanges, ObjectID oldObjectIDRangeStart[], ObjectID newObjectIDRangeStart[], ULONG cObjectIDRangeLength[])
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::SurvivingReferences(ULONG cmovedObjectIDRanges, ObjectID objectIDRangeStart[], ULONG cObjectIDRangeLength[])
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ObjectsAllocatedByClass(ULONG classCount, ClassID classIDs[], ULONG objects[])
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ObjectAllocated(ObjectID objectID, ClassID classID)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ObjectReferences(ObjectID objectID, ClassID classID, ULONG objectRefs, ObjectID objectRefIDs[])
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::RootReferences(ULONG rootRefs, ObjectID rootRefIDs[])
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::GarbageCollectionStarted(int cGenerations, BOOL generationCollected[], COR_PRF_GC_REASON reason)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::GarbageCollectionFinished()
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::FinalizeableObjectQueued(DWORD finalizerFlags, ObjectID objectID)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::RootReferences2(ULONG cRootRefs, ObjectID rootRefIds[], COR_PRF_GC_ROOT_KIND rootKinds[], COR_PRF_GC_ROOT_FLAGS rootFlags[], UINT_PTR rootIds[])
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::HandleCreated(UINT_PTR handleId, ObjectID initialObjectId)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::HandleDestroyed(UINT_PTR handleId)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ExceptionThrown(ObjectID thrownObjectID)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ExceptionSearchFunctionEnter(FunctionID functionID)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ExceptionSearchFunctionLeave()
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ExceptionSearchFilterEnter(FunctionID functionID)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ExceptionSearchFilterLeave()
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ExceptionSearchCatcherFound(FunctionID functionID)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ExceptionCLRCatcherFound()
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ExceptionCLRCatcherExecute()
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ExceptionOSHandlerEnter(FunctionID functionID)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ExceptionOSHandlerLeave(FunctionID functionID)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ExceptionUnwindFunctionEnter(FunctionID functionID)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ExceptionUnwindFunctionLeave()
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ExceptionUnwindFinallyEnter(FunctionID functionID)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ExceptionUnwindFinallyLeave()
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ExceptionCatcherEnter(FunctionID functionID, ObjectID objectID)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ExceptionCatcherLeave()
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::COMClassicVTableCreated(ClassID wrappedClassID, REFGUID implementedIID, void *pVTable, ULONG cSlots)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::COMClassicVTableDestroyed(ClassID wrappedClassID, REFGUID implementedIID, void *pVTable)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ThreadNameChanged(ThreadID threadId, ULONG cchName, __in_ecount_opt(cchName) WCHAR name[])
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::InitializeForAttach(IUnknown *pICorProfilerInfoUnk, void *pvClientData, UINT cbClientData)
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ProfilerAttachComplete()
{
	return S_OK;
}

// Empty method.
HRESULT ProfilerCallback::ProfilerDetachSucceeded()
{
	return S_OK;
}

// [public] Instrumented code eventually calls into here (when function is entered)
// to do the work of maintaining the shadow stack and function timings.
void ProfilerCallback::NtvEnteredFunction(ModuleID moduleIDCur, mdMethodDef mdCur, int nVersionCur)
{
	ModuleInfo moduleInfo = m_moduleIDToInfoMap.Lookup(moduleIDCur);
	WCHAR wszTypeDefName[512];
	WCHAR wszMethodDefName[512];
	GetClassAndFunctionNamesFromMethodDef(
		moduleInfo.m_pImport,
		moduleIDCur, 
		mdCur,
		wszTypeDefName,
		_countof(wszTypeDefName),
		wszMethodDefName,
		_countof(wszMethodDefName));

	std::vector<ShadowStackFrameInfo> * pShadow = GetShadowStack();

	// Create a new ShadowStackFrameInfo for the current frame.
	ShadowStackFrameInfo pFrame;
	pFrame.m_moduleID = moduleIDCur;
	pFrame.m_methodDef = mdCur;
	pFrame.m_nVersion = nVersionCur;
	pFrame.m_ui64TickCountOnEntry = GetTickCount64();

	// Write the entry to file.
	RESULT_APPEND(L"TID " << HEX(GetCurrentThreadId()) << (GetPaddingString(((UINT) (pShadow->size()) + 1) * 4)) <<
		wszTypeDefName << L"." << wszMethodDefName << L" entered");

	// Update the shadow stack.
	pShadow->push_back(pFrame);
}

// [public] Instrumented code eventually calls into here (when function is exited)
// to do the work of maintaining the shadow stack and function timings.
void ProfilerCallback::NtvExitedFunction(ModuleID moduleIDCur, mdMethodDef mdCur, int nVersionCur)
{
	ModuleInfo moduleInfo = m_moduleIDToInfoMap.Lookup(moduleIDCur);
	WCHAR wszTypeDefName[512];
	WCHAR wszMethodDefName[512];
	GetClassAndFunctionNamesFromMethodDef(
		moduleInfo.m_pImport,
		moduleIDCur, 
		mdCur,
		wszTypeDefName,
		_countof(wszTypeDefName),
		wszMethodDefName,
		_countof(wszMethodDefName));

	std::vector<ShadowStackFrameInfo> * pShadow = GetShadowStack();

	// Update shadow stack, but verify its leaf is the function we're exiting
	if (pShadow->size() <= 0)
	{
		LOG_APPEND(L"Exiting a function with an empty shadow stack.");
		return;
	}

	ShadowStackFrameInfo pFrame = pShadow->back();

    // See how long this function took (inclusive time).  If it's longer than our
    // arbitrary m_dwThresholdMs, then flag the entry in the results log with some
    // asterisks.
	DWORD dwInclusiveMs = (GetTickCount64() - pFrame.m_ui64TickCountOnEntry) & 0xFFFFffff;
	RESULT_APPEND(
        L"TID "
        << HEX(GetCurrentThreadId()) 
        << GetPaddingString((UINT) (pShadow->size()) * 4) 
        << wszTypeDefName 
        << "." 
        << wszMethodDefName 
        << L" exited. Inclusive ms: " 
        << dwInclusiveMs 
        << 
            (
                (dwInclusiveMs > m_dwThresholdMs) ?
		        L".**** THRESHOLD EXCEEDED ****" : 
                L"."
            )
        );

	if ((pFrame.m_moduleID != moduleIDCur) ||
		(pFrame.m_methodDef != mdCur) ||
		(pFrame.m_nVersion != nVersionCur))
	{
		LOG_APPEND(L"Exited function does not map leaf of shadow stack (" << pShadow->size() <<
			L" frames high).  Leaf of shadow stack: ModuleID = " << HEX(pFrame.m_moduleID) <<
			L", methodDef = " << HEX(pFrame.m_methodDef) << L", Version = " << pFrame.m_nVersion <<
			L". Actual function exited: ModuleID = " << HEX(moduleIDCur) << L", methodDef = " <<
			HEX(mdCur) << L", Version = " << nVersionCur << L".");
	}

	// Do the pop
	pShadow->pop_back();
}

//************************************************************************************************//

//******************                      Private  Methods                      ******************//            

//************************************************************************************************//

// [private] Adds memberRefs to the managed helper into the module so that we can ReJIT later.
void ProfilerCallback::AddMemberRefs(IMetaDataAssemblyImport * pAssemblyImport, IMetaDataAssemblyEmit * pAssemblyEmit, IMetaDataEmit * pEmit, ModuleInfo * pModuleInfo)
{
	assert(pModuleInfo != NULL);

	LOG_APPEND(L"Adding memberRefs in this module to point to the helper managed methods");

	IMetaDataImport * pImport = pModuleInfo->m_pImport;

	HRESULT hr;

	// Signature for method the rewritten IL will call:
	// - public static void MgdEnteredFunction64(UInt64 moduleIDCur, UInt32 mdCur, int nVersionCur)
	// - public static void MgdEnteredFunction32(UInt32 moduleIDCur, UInt32 mdCur, int nVersionCur)

#ifdef _WIN64
	COR_SIGNATURE sigFunctionProbe[] = {
		IMAGE_CEE_CS_CALLCONV_DEFAULT,      // default calling convention
		0x03,                               // number of arguments == 3
		ELEMENT_TYPE_VOID,                  // return type == void
		ELEMENT_TYPE_U8,                    // arg 1: UInt64 moduleIDCur
		ELEMENT_TYPE_U4,                    // arg 2: UInt32 mdCur
		ELEMENT_TYPE_I4,                    // arg 3: int nVersionCur
	};
#else //  ! _WIN64 (32-bit code follows)
	COR_SIGNATURE sigFunctionProbe[] = {
		IMAGE_CEE_CS_CALLCONV_DEFAULT,      // default calling convention
		0x03,                               // number of arguments == 3
		ELEMENT_TYPE_VOID,                  // return type == void
		ELEMENT_TYPE_U4,                    // arg 1: UInt32 moduleIDCur
		ELEMENT_TYPE_U4,                    // arg 2: UInt32 mdCur
		ELEMENT_TYPE_I4,                    // arg 3: int nVersionCur
	};
#endif //_WIN64

	mdAssemblyRef assemblyRef = NULL;
	mdTypeRef typeRef = mdTokenNil;

	if (m_fInstrumentationHooksInSeparateAssembly)
	{
		// Generate assemblyRef to ProfilerHelper.dll
		BYTE rgbPublicKeyToken[] = { 0xfc, 0xb7, 0x40, 0xf6, 0x34, 0x46, 0xe2, 0xf2 };
		WCHAR wszLocale[MAX_PATH];
		wcscpy_s(wszLocale, L"neutral");

		ASSEMBLYMETADATA assemblyMetaData;
		ZeroMemory(&assemblyMetaData, sizeof(assemblyMetaData));
		assemblyMetaData.usMajorVersion = 1;
		assemblyMetaData.usMinorVersion = 0;
		assemblyMetaData.usBuildNumber = 0;
		assemblyMetaData.usRevisionNumber = 0;
		assemblyMetaData.szLocale = wszLocale;
		assemblyMetaData.cbLocale = _countof(wszLocale);

		hr = pAssemblyEmit->DefineAssemblyRef(
			(void *) rgbPublicKeyToken,
			sizeof(rgbPublicKeyToken),
			L"ProfilerHelper",
			&assemblyMetaData,
			NULL,                   // hash blob
			NULL,                   // cb of hash blob
			0,                      // flags
			&assemblyRef);

		if (FAILED(hr))
		{
			LOG_APPEND(L"DefineAssemblyRef failed, hr = " << HEX(hr));
		}
	}
	else
	{
		// Probes are being added to mscorlib. Find existing mscorlib assemblyRef.

		HCORENUM hEnum = NULL;
		mdAssemblyRef rgAssemblyRefs[20];
		ULONG cAssemblyRefsReturned;
		assemblyRef = mdTokenNil;

		do
		{
			hr = pAssemblyImport->EnumAssemblyRefs(
				&hEnum,
				rgAssemblyRefs,
				_countof(rgAssemblyRefs),
				&cAssemblyRefsReturned);

			if (FAILED(hr))
			{
				LOG_APPEND(L"EnumAssemblyRefs failed, hr = " << HEX(hr));
				return;
			}

			if (cAssemblyRefsReturned == 0)
			{
				LOG_APPEND(L"Could not find an AssemblyRef to mscorlib");
				return;
			}
		} while (!FindMscorlibReference(
			pAssemblyImport,
			rgAssemblyRefs,
			cAssemblyRefsReturned,
			&assemblyRef));

		pAssemblyImport->CloseEnum(hEnum);
		hEnum = NULL;

		assert(assemblyRef != mdTokenNil);
	}

	// Generate typeRef to ILRewriteProfilerHelper.ProfilerHelper or the pre-existing mscorlib type
	// that we're adding the managed helpers to.

	LPCWSTR wszTypeToReference = 
		m_fInstrumentationHooksInSeparateAssembly ?
		L"ILRewriteProfilerHelper.ProfilerHelper" :
	    k_wszHelpersContainerType;

	hr = pEmit->DefineTypeRefByName(
		assemblyRef,
		wszTypeToReference,
		&typeRef);

	if (FAILED(hr))
	{
		LOG_APPEND(L"DefineTypeRefByName to " << wszTypeToReference << L" failed, hr = " << HEX(hr));
	}

	hr = pEmit->DefineMemberRef(
		typeRef,
		k_wszEnteredFunctionProbeName,
		sigFunctionProbe,
		sizeof(sigFunctionProbe),
		&(pModuleInfo->m_mdEnterProbeRef));

	if (FAILED(hr))
	{
		LOG_APPEND(L"DefineMemberRef to " << k_wszEnteredFunctionProbeName <<
			L" failed, hr = " << HEX(hr));
	}

	hr = pEmit->DefineMemberRef(
		typeRef,
		k_wszExitedFunctionProbeName,
		sigFunctionProbe,
		sizeof(sigFunctionProbe),
		&(pModuleInfo->m_mdExitProbeRef));

	if (FAILED(hr))
	{
		LOG_APPEND(L"DefineMemberRef to " << k_wszExitedFunctionProbeName <<
			L" failed, hr = " << HEX(hr));
	}
}

// [private] Gets the reference to mscorlib, in case pumping helpers into it is necessary.
BOOL ProfilerCallback::FindMscorlibReference(IMetaDataAssemblyImport * pAssemblyImport, mdAssemblyRef * rgAssemblyRefs, ULONG cAssemblyRefs, mdAssemblyRef * parMscorlib)
{
	HRESULT hr;

	for (ULONG i=0; i < cAssemblyRefs; i++)
	{
		const void * pvPublicKeyOrToken;
		ULONG cbPublicKeyOrToken;
		WCHAR wszName[512];
		ULONG cchNameReturned;
		ASSEMBLYMETADATA asmMetaData;
		ZeroMemory(&asmMetaData, sizeof(asmMetaData));
		const void * pbHashValue;
		ULONG cbHashValue;
		DWORD asmRefFlags;

		hr = pAssemblyImport->GetAssemblyRefProps(
			rgAssemblyRefs[i],
			&pvPublicKeyOrToken,
			&cbPublicKeyOrToken,
			wszName,
			_countof(wszName),
			&cchNameReturned,
			&asmMetaData,
			&pbHashValue,
			&cbHashValue,
			&asmRefFlags);

		if (FAILED(hr))
		{
			LOG_APPEND(L"GetAssemblyRefProps failed, hr = " << HEX(hr));
			return FALSE;
		}

		if (::ContainsAtEnd(wszName, L"mscorlib"))
		{
			*parMscorlib = rgAssemblyRefs[i];
			return TRUE;
		}
	}

	return FALSE;
}

// [private] Adds appropriate methodDefs to mscorlib for the managed helper probes.
void ProfilerCallback::AddHelperMethodDefs(IMetaDataImport * pImport, IMetaDataEmit * pEmit)
{
	HRESULT hr;

	assert(!m_fInstrumentationHooksInSeparateAssembly);

	LOG_APPEND(L"Adding methodDefs to mscorlib metadata for managed helper probes");

	// The helpers will need to call into System.IntPtr::op_Explicit(int64), so get methodDef now
	mdTypeDef tdSystemIntPtr;
	hr = pImport->FindTypeDefByName(L"System.IntPtr", mdTypeDefNil, &tdSystemIntPtr);

	if (FAILED(hr))
	{
		LOG_APPEND(L"FindTypeDefByName(System.IntPtr) failed, hr = " << HEX(hr));
		return;
	}

	COR_SIGNATURE intPtrOpExplicitSignature[] = {
		IMAGE_CEE_CS_CALLCONV_DEFAULT,
		1,               // 1 argument
		ELEMENT_TYPE_I,  // return type is native int
		ELEMENT_TYPE_I8, // argument type is int64
	};

	hr = pImport->FindMethod(
		tdSystemIntPtr,
		L"op_Explicit",
		intPtrOpExplicitSignature,
		sizeof(intPtrOpExplicitSignature),
		&m_mdIntPtrExplicitCast);

	if (FAILED(hr))
	{
		LOG_APPEND(L"FindMethod(System.IntPtr.op_Explicit(int64)) failed, hr = " << HEX(hr));
		return;
	}

	// Put the managed helpers into this pre-existing mscorlib type
	mdTypeDef tdHelpersContainer;
	hr = pImport->FindTypeDefByName(k_wszHelpersContainerType, mdTypeDefNil, &tdHelpersContainer);

	if (FAILED(hr))
	{
		LOG_APPEND(L"FindTypeDefByName(" << k_wszHelpersContainerType <<
			L") failed, hr = " << HEX(hr));
		return;
	}

	// Get a dummy method implementation RVA (CLR doesn't like you passing 0).  Pick a
	// ctor on the same type.
	COR_SIGNATURE ctorSignature[] = 
	{
		IMAGE_CEE_CS_CALLCONV_HASTHIS, //__stdcall
		0,
		ELEMENT_TYPE_VOID 
	};

	mdMethodDef mdCtor = NULL;
	hr = pImport->FindMethod(
		tdHelpersContainer,
		L".ctor",
		ctorSignature,
		sizeof(ctorSignature),
		&mdCtor);

	if (FAILED(hr))
	{
		LOG_APPEND(L"FindMethod(" << k_wszHelpersContainerType <<
			L"..ctor) failed, hr = " << HEX(hr));
		return;
	}

	ULONG rvaCtor;
	hr = pImport->GetMethodProps(
		mdCtor,
		NULL,		   // Put method's class here. 
		NULL,		   // Put method's name here.  
		0,			   // Size of szMethod buffer in wide chars.   
		NULL,		   // Put actual size here 
		NULL,		   // Put flags here.  
		NULL,		   // [OUT] point to the blob value of meta data   
		NULL,		   // [OUT] actual size of signature blob  
		&rvaCtor,
		NULL);

	if (FAILED(hr))
	{
		LOG_APPEND(L"GetMethodProps(" << k_wszHelpersContainerType <<
			L"..ctor) failed, hr = " << HEX(hr));
		return;
	}

	// Generate reference to unmanaged profiler DLL (i.e., us)
	mdModuleRef modrefNativeExtension;
	hr = pEmit->DefineModuleRef(L"ILRewriteProfiler", &modrefNativeExtension);

	if (FAILED(hr))
	{
		LOG_APPEND(L"DefineModuleRef against the native profiler DLL failed, hr = " << HEX(hr));
		return;
	}

	// Generate the PInvokes into the profiler DLL
	AddPInvoke(
		pEmit,
		tdHelpersContainer,
		L"NtvEnteredFunction",
		modrefNativeExtension,
		&m_mdEnterPInvoke);

	AddPInvoke(
		pEmit,
		tdHelpersContainer,
		L"NtvExitedFunction",
		modrefNativeExtension,
		&m_mdExitPInvoke);

	// Generate the SafeCritical managed methods which call the PInvokes
	mdMethodDef mdSafeCritical;
	GetSecuritySafeCriticalAttributeToken(pImport, &mdSafeCritical);

	AddManagedHelperMethod(
		pEmit,
		tdHelpersContainer,
		k_wszEnteredFunctionProbeName,
		m_mdEnterPInvoke,
		rvaCtor,
		mdSafeCritical, &m_mdEnter);

	AddManagedHelperMethod(
		pEmit,
		tdHelpersContainer,
		k_wszExitedFunctionProbeName,
		m_mdExitPInvoke,
		rvaCtor,
		mdSafeCritical,
		&m_mdExit);
}

// [private] Creates a PInvoke method to inject into mscorlib.
HRESULT ProfilerCallback::AddPInvoke(IMetaDataEmit * pEmit, mdTypeDef td, LPCWSTR wszName, mdModuleRef modrefTarget, mdMethodDef * pmdPInvoke)
{
	HRESULT hr;

	//COR_SIGNATURE representation
	//   Calling convention
	//   Number of Arguments
	//   Return type
	//   Argument type
	//   ...

	COR_SIGNATURE newMethodSignature[] = { IMAGE_CEE_CS_CALLCONV_DEFAULT,   //__stdcall
		3,                               // 3 inputs
		ELEMENT_TYPE_VOID,               // No return
		ELEMENT_TYPE_I,                  // ModuleID
		ELEMENT_TYPE_U4,                 // mdMethodDef token
		ELEMENT_TYPE_I4                  // Rejit version number
	};

	hr = pEmit->DefineMethod(
		td,
		wszName,
		~mdAbstract & (mdStatic | mdPublic | mdPinvokeImpl),
		newMethodSignature,
		sizeof(newMethodSignature),
		0,
		miPreserveSig,
		pmdPInvoke);

	LOG_IFFAILEDRET(hr, L"Failed in DefineMethod when creating P/Invoke method " << wszName);

	hr = pEmit->DefinePinvokeMap(
		*pmdPInvoke,
		pmCallConvStdcall | pmNoMangle,
		wszName,
		modrefTarget);

	LOG_IFFAILEDRET(hr, L"Failed in DefinePinvokeMap when creating P/Invoke method " << wszName);

	return hr;
}

// [private] Gets the SafeCritical token to use when injecting methods into mscorlib.
HRESULT ProfilerCallback::GetSecuritySafeCriticalAttributeToken(IMetaDataImport * pImport, mdMethodDef * pmdSafeCritical)
{
	mdTypeDef tdSafeCritical;

	HRESULT hr = pImport->FindTypeDefByName(
		L"System.Security.SecuritySafeCriticalAttribute",
		mdTokenNil,
		&tdSafeCritical);

	LOG_IFFAILEDRET(hr, L"FindTypeDefByName(System.Security.SecuritySafeCriticalAttribute) failed");

	COR_SIGNATURE sigSafeCriticalCtor[] = {
		IMAGE_CEE_CS_CALLCONV_HASTHIS,
		0x00,                               // number of arguments == 0
		ELEMENT_TYPE_VOID,                  // return type == void
	};

	hr = pImport->FindMember(
		tdSafeCritical,
		L".ctor",
		sigSafeCriticalCtor,
		sizeof(sigSafeCriticalCtor),
		pmdSafeCritical);

	LOG_IFFAILEDRET(hr, L"FindMember(System.Security.SecuritySafeCriticalAttribute..ctor) failed");

	return hr;
}

// [private] Adds the managed helper methods to mscorlib.
HRESULT ProfilerCallback::AddManagedHelperMethod(IMetaDataEmit * pEmit, mdTypeDef td, LPCWSTR wszName, mdMethodDef mdTargetPInvoke, ULONG rvaDummy, mdMethodDef mdSafeCritical, mdMethodDef * pmdHelperMethod)
{
	HRESULT hr;

	COR_SIGNATURE newMethodSignature[] = {
		IMAGE_CEE_CS_CALLCONV_DEFAULT,  //__stdcall
		3,
		ELEMENT_TYPE_VOID,              // returns void
#ifdef _X86_
		ELEMENT_TYPE_U4,                // ModuleID
#elif defined(_AMD64_)
		ELEMENT_TYPE_U8,                // ModuleID
#else
#error THIS SAMPLE ONLY WORKS ON X86 AND X64
#endif
		ELEMENT_TYPE_U4,                // mdMethodDef token
		ELEMENT_TYPE_I4,                // Rejit version number
	};

	hr = pEmit->DefineMethod(
		td,
		wszName,
		mdStatic | mdPublic,
		newMethodSignature,
		sizeof(newMethodSignature),
		rvaDummy,
		miIL | miNoInlining,
		pmdHelperMethod);

	LOG_IFFAILEDRET(hr, L"Failed in DefineMethod when creating managed helper method " << wszName);

	mdToken tkCustomAttribute;
	hr = pEmit->DefineCustomAttribute(
		*pmdHelperMethod,
		mdSafeCritical,
		NULL,          //Blob, contains constructor params in this case none
		0,             //Size of the blob
		&tkCustomAttribute);

	LOG_IFFAILEDRET(hr, L"Failed in DefineCustomAttribute when applying SecuritySafeCritical to " <<
		L"new managed helper method " << wszName);

	return hr;
}

// [private] Launches the listener for file changes to ILRWP_watchercommands.log.
void ProfilerCallback::LaunchLogListener(LPCWSTR wszPathCommandFile)
{
	g_nLastRefid = 0;

	// Wipe the other log files for the new session.
	DeleteFile(g_wszResponseFilePath);
	DeleteFile(g_wszResultFilePath);
	RESPONSE_APPEND(L"New profiler session lauched.");

	// Read the method of injection (mscorlib or not)

	if (FileExists(wszPathCommandFile))
	{
		// Read and execute first command.
		FILE * fFile = _wfsopen(wszPathCommandFile, L"rt", _SH_DENYWR);
		WCHAR pumpintomscorlib = L'q';
		int nParsed = fwscanf_s(fFile, MSCORLIBCOMMAND, &pumpintomscorlib);
		fclose(fFile);

		if (nParsed == 1 && pumpintomscorlib == L't')
		{
			// We're pumping the helpers into mscorlib
			m_fInstrumentationHooksInSeparateAssembly = FALSE;
		}
		else if (nParsed != 1 || pumpintomscorlib != L'f')
		{
			LOG_APPEND(L"ERROR: Incorrect mscorlib command format, or " << g_wszCmdFilePath <<
				L" did not exist at launch time. Assuming we won't pump into mscorlib.");
		}
	}


	// Create the container for the arguments.
	threadargs * args = new threadargs();
	args->m_pCallback = g_pCallbackObject;
	args->m_wszpath = wszPathCommandFile;
	args->m_iMap = &m_moduleIDToInfoMap;

	DWORD  dwThreadID;
	HANDLE hThread = CreateThread(
		NULL,          // Default security attributes
		0,             // Default stack size
		::MonitorFile, // Function name
		(LPVOID)args,  // Function parameters, wrapped in a threadargs struct
		0,             // Start the thread immediately after creation
		&dwThreadID);  // ID of created thread

	if (hThread == NULL)
	{
		LOG_APPEND(L"Failed to create a thread that is supposed to wait on a detach pipe, hr = " <<
			HEX(HRESULT_FROM_WIN32(GetLastError())));
	}
}

// [private] Wrapper method for the ICorProfilerCallback::RequestReJIT method, managing its errors.
HRESULT ProfilerCallback::CallRequestReJIT(UINT cFunctionsToRejit, ModuleID * rgModuleIDs, mdMethodDef * rgMethodDefs)
{
	HRESULT hr = m_pProfilerInfo->RequestReJIT(cFunctionsToRejit, rgModuleIDs, rgMethodDefs);

	LOG_IFFAILEDRET(hr, L"RequestReJIT failed");

	LOG_APPEND(L"RequestReJIT successfully called with " << cFunctionsToRejit << L" methods.");
	return hr;
}

// [private] Wrapper method for the ICorProfilerCallback::RequestRevert method, managing its errors.
HRESULT ProfilerCallback::CallRequestRevert(UINT cFunctionsToRejit, ModuleID * rgModuleIDs, mdMethodDef * rgMethodDefs)
{
	HRESULT results[10];
	HRESULT hr = m_pProfilerInfo->RequestRevert(cFunctionsToRejit, rgModuleIDs, rgMethodDefs, results);

	LOG_IFFAILEDRET(hr, L"RequestRevert failed");

	LOG_APPEND(L"RequestRevert successfully called with " << cFunctionsToRejit << L" methods.");
	return hr;
}

// [private] Gets the shadow stack for the thread.
std::vector<ShadowStackFrameInfo> * ProfilerCallback::GetShadowStack()
{
	std::vector<ShadowStackFrameInfo> * pShadow = (std::vector<ShadowStackFrameInfo> *) TlsGetValue(m_dwShadowStackTlsIndex);
	if (pShadow == NULL)
	{
		pShadow = new std::vector<ShadowStackFrameInfo>;
		TlsSetValue(m_dwShadowStackTlsIndex, pShadow);
	}

	return pShadow;
}

// [private] Gets the text names from a method def.
void ProfilerCallback::GetClassAndFunctionNamesFromMethodDef(IMetaDataImport * pImport, ModuleID moduleID, mdMethodDef methodDef, LPWSTR wszTypeDefName, ULONG cchTypeDefName, LPWSTR wszMethodDefName, ULONG cchMethodDefName)
{
	HRESULT hr;
	mdTypeDef typeDef;
	ULONG cchMethodDefActual;
	DWORD dwMethodAttr;
	ULONG cchTypeDefActual;
	DWORD dwTypeDefFlags;
	mdTypeDef typeDefBase;

	hr = pImport->GetMethodProps(
		methodDef,
		&typeDef,
		wszMethodDefName,
		cchMethodDefName,
		&cchMethodDefActual,
		&dwMethodAttr,
		NULL,       // [OUT] point to the blob value of meta data
		NULL,       // [OUT] actual size of signature blob
		NULL,       // [OUT] codeRVA
		NULL);      // [OUT] Impl. Flags

	if (FAILED(hr))
	{
		LOG_APPEND(L"GetMethodProps failed in ModuleID = " <<
			HEX(moduleID) << L" for methodDef = " << HEX(methodDef) << L", hr = " << HEX(hr));
	}

	hr = pImport->GetTypeDefProps(
		typeDef,
		wszTypeDefName,
		cchTypeDefName,
		&cchTypeDefActual,
		&dwTypeDefFlags,
		&typeDefBase);

	if (FAILED(hr))
	{
		LOG_APPEND(L"GetTypeDefProps failed in ModuleID = " << HEX(moduleID) <<
			L" for typeDef = " << HEX(typeDef) << L", hr = " << HEX(hr));
	}
}

// [private] Checks to see if the command file has any changes, and if so runs the new commands.
DWORD WINAPI MonitorFile(LPVOID args)
{
	// Monitor file until app shutdown.
	while(!g_bShouldExit)
	{
		try
		{
			// We wipe the command file at launch, so wait until it's back again.
			if (FileExists(((threadargs *)args)->m_wszpath))
			{
				// Open the file in a format we can more easily read from.
				FILE * fFile = _wfsopen(((threadargs *)args)->m_wszpath, L"rt", _SH_DENYWR);
                if (fFile != NULL)
                {
				    // Skip over first line.
				    int pumpintomscorlib; // Don't actually do anything with this, but that's what the line is.
				    int nParsed = fwscanf_s(fFile, MSCORLIBCOMMAND, &pumpintomscorlib);

				    // Read and execute all new commands.
				    while (!feof(fFile))
				    {
					    ReadFile(fFile, args);
				    }

				    // All done with the file for this pass, close it.
				    fclose(fFile);
                }
			}
		}
		catch (int e) // We want this thread to be silent. Bad form, but we're logging it at least.
		{
			LOG_APPEND(L"ERROR: Command file could not be read. Error code " << e << L".");
		}

		// Delay next read, so this thread doesn't take up all the resources for the application.
		Sleep(1000);
	}

	// Begin shutdown process.

	// Remove dynamically-allocated structures.
	delete(args);

	g_nLastRefid = 0; // Return refid to 0, since we're done.

	// Clean up complete. Application can exit safely.
	RESPONSE_APPEND(RSP_QUITSUCCESS);
	g_bSafeToExit = TRUE;

	return S_OK;
}

// [private] Checks to see if the given file exists.
bool FileExists(const PCWSTR wszFilepath)
{
	std::ifstream ifsFile(wszFilepath);
	return ifsFile ? true : false;
}

// [private] Reads and runs a command from the command file.
void ReadFile(FILE * fFile, LPVOID args)
{
	// Get a line.
	unsigned int refid = 0;
	WCHAR wszCommand[BUFSIZE], wszModule[BUFSIZE], wszClass[BUFSIZE], wszFunc[BUFSIZE];
	int nParsed = fwscanf_s(fFile,
		L"%u>\t%s\t%s\t%s%s\n",
		&refid,
		wszCommand, BUFSIZE,
		wszModule, BUFSIZE,
		wszClass, BUFSIZE,
		wszFunc, BUFSIZE);

	// Need all elements, or at least 0>quitcommand.
	if (nParsed == 5 || (nParsed > 1 && refid == 0))
	{

		if (refid == 0)
		{
			g_bShouldExit = (wcscmp(wszCommand, CMD_QUIT) == 0);

			if (!g_bShouldExit)
			{
				RESPONSE_ERROR(L"\"0>\t" << wszCommand << L"\" is not a valid command.");
			}
		}
		else if (refid > g_nLastRefid)
		{
			if ((wcscmp(wszCommand, CMD_REJITFUNC) == 0) ||
                (wcscmp(wszCommand, CMD_REVERTFUNC) == 0))
			{
				// Get the information necessary to rejit / revert, and then do it
                BOOL fRejit = (wcscmp(wszCommand, CMD_REJITFUNC) == 0);
                const int MAX_METHODS = 20;
                int cMethodsFound = 0;
                ModuleID moduleIDs[MAX_METHODS] = { 0 };
                mdMethodDef methodDefs[MAX_METHODS] = { 0 };
				if  (::GetTokensFromNames(
					((threadargs *)args)->m_iMap,
					wszModule,
					wszClass,
					wszFunc,
					moduleIDs,
                    methodDefs,
                    _countof(moduleIDs),
                    &cMethodsFound))
				{

					// This is a current command. Execute it.
					g_nLastRefid = refid;

                    for (int i=0; i < cMethodsFound; i++)
                    {
					    // Update this module's version in the mapping.
					    MethodDefToLatestVersionMap * pMethodDefToLatestVersionMap = 
						    m_moduleIDToInfoMap.Lookup(moduleIDs[i]).m_pMethodDefToLatestVersionMap;
					    pMethodDefToLatestVersionMap->Update(methodDefs[i], fRejit ? g_nLastRefid : 0);
                    }

					HRESULT hr;
                    if (fRejit)
                    {
                        hr = ((ProfilerCallback *)((threadargs *)args)->m_pCallback)->
                            CallRequestReJIT(
                            cMethodsFound,          // Number of functions being rejitted
                            moduleIDs,              // Pointer to the start of the ModuleID array
                            methodDefs);            // Pointer to the start of the mdMethodDef array
                    }
                    else
                    {
                        hr = ((ProfilerCallback *)((threadargs *)args)->m_pCallback)->
                            CallRequestRevert(
                            cMethodsFound,          // Number of functions being reverted
                            moduleIDs,              // Pointer to the start of the ModuleID array
                            methodDefs);            // Pointer to the start of the mdMethodDef array
                    }

					if (FAILED(hr))
					{
						RESPONSE_IS(g_nLastRefid, RSP_REJITFAILURE, wszModule, wszClass, wszFunc);
					}
					else
					{
						RESPONSE_IS(g_nLastRefid, RSP_REJITSUCCESS, wszModule, wszClass, wszFunc);
					}

				}
				else
				{
					RESPONSE_ERROR(L"Module, class, or function not found. Maybe module is not loaded yet?");
					LOG_APPEND(L"ERROR: Module, class, or function not found. Maybe module is not loaded yet?");
				}
			}
			else
			{
				// We don't know how to deal with prof commands that aren't rejit / revert.
				RESPONSE_ERROR(L"\"" << refid << L">\t" << wszCommand << L"\" is not a valid command.");
			}
		}
	}
}

// [private] Gets the MethodDef from the module, class and function names.
BOOL GetTokensFromNames(IDToInfoMap<ModuleID, ModuleInfo> * mMap, LPCWSTR wszModule, LPCWSTR wszClass, LPCWSTR wszFunction, ModuleID * moduleIDs, mdMethodDef * methodDefs, int cElementsMax,  int * pcMethodsFound)

{
	HRESULT hr;
	HCORENUM hEnum = NULL;
	ULONG cMethodDefsReturned = 0;
	mdTypeDef typeDef;
	mdMethodDef rgMethodDefs[2];
    *pcMethodsFound = 0;

	// Find all modules matching the name in this script entry.
	ModuleIDToInfoMap::LockHolder lockHolder(mMap);

	ModuleIDToInfoMap::Const_Iterator iterator;
	for (iterator = mMap->Begin(); (iterator != mMap->End()) && (*pcMethodsFound < cElementsMax); iterator++)
	{
		LPCWSTR wszModulePathCur = &(iterator->second.m_wszModulePath[0]);

		// Only matters if we have the right module name.
		if (::ContainsAtEnd(wszModulePathCur, wszModule))
		{
			hr = iterator->second.m_pImport->FindTypeDefByName(wszClass, mdTypeDefNil, &typeDef);

			if (FAILED(hr))
			{
				LOG_APPEND(L"Failed to find class '" << wszClass << L"',  hr = " << HEX(hr));
                continue;
			}

			hr = iterator->second.m_pImport->EnumMethodsWithName(
				&hEnum,
				typeDef,
				wszFunction,
				rgMethodDefs,
				_countof(rgMethodDefs),
				&cMethodDefsReturned);

			if (FAILED(hr) || (hr == S_FALSE))
			{
				LOG_APPEND(L"Found class '" << wszClass << L"', but no member methods with name '" <<
					wszFunction << L"', hr = " << HEX(hr));
                continue;
			}

			if (cMethodDefsReturned != 1)
			{
				LOG_APPEND(L"Expected exactly 1 methodDef to match class '" << wszClass << L"', method '" <<
					wszFunction << L"', but actually found '" << cMethodDefsReturned << L"'");
                continue;
			}

			// Remember the latest version number for this mdMethodDef.
			iterator->second.m_pMethodDefToLatestVersionMap->Update(rgMethodDefs[0], g_nLastRefid);

			// Save the matching pair.
			moduleIDs[*pcMethodsFound] = iterator->first;
			methodDefs[*pcMethodsFound] = rgMethodDefs[0];

            (*pcMethodsFound)++;

			// Intentionally continue through loop to find any other matching
			// modules. This catches the case where one module is loaded (unshared)
			// into multiple AppDomains
		}
	}

	// Return whether creation was successful.
	return (*pcMethodsFound) > 0;
}

// [private] Returns TRUE iff wszContainer ends with wszProspectiveEnding (case-insensitive).
BOOL ContainsAtEnd(LPCWSTR wszContainer, LPCWSTR wszProspectiveEnding)
{
	size_t cchContainer = wcslen(wszContainer);
	size_t cchEnding = wcslen(wszProspectiveEnding);

	if (cchContainer < cchEnding)
		return FALSE;

	if (cchEnding == 0)
		return FALSE;

	if (_wcsicmp(
		wszProspectiveEnding,
		&(wszContainer[cchContainer - cchEnding])) != 0)
	{
		return FALSE;
	}

	return TRUE;
}
