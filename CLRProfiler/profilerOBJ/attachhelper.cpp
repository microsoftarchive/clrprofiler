#include <windows.h>
#include <share.h>

#include "basehlp.h"

#include "avlnode.h"

#include "ProfilerInfo.h"

#include "ProfilerCallback.h"

#include "metahost.h"

/***************************************************************************************
 *  Method:
 *
 *
 *  Purpose:
 *
 *
 *  Parameters: 
 *
 *
 *  Return value:
 *
 *
 *  Notes:
 *
 ***************************************************************************************/

HRESULT EnableDebugPrivilege();

BOOL g_fConsoleMode = TRUE;

void Log(LPCWSTR wszFormat, ...)
{
    va_list insertionArgs;
    va_start(insertionArgs, wszFormat);
    if (g_fConsoleMode)
    {
        vwprintf(wszFormat, insertionArgs);
    }
    else
    {
        WCHAR wszMessage[1024];
#ifdef _ARM_ // @ARMTODO: undo this if we link full CRT
        _vsnwprintf_s(wszMessage, 1024, _TRUNCATE, wszFormat, insertionArgs);
#else
        _vswprintf_p(wszMessage, 1024, wszFormat, insertionArgs);
#endif
        MessageBoxW(NULL, wszMessage, L"CLRProfiler", MB_OK);
    }

    va_end(insertionArgs);
}


/***************************************************************************************
 *  Method:
 *
 *
 *  Purpose:
 *
 *
 *  Parameters: 
 *
 *
 *  Return value:
 *
 *
 *  Notes:
 *
 ***************************************************************************************/

STDAPI AttachProfiler(int pid, LPCWSTR wszTargetVersion, LPCWSTR wszProfilerPath, ProfConfig * pProfConfig, BOOL fConsoleMode)
{
    HMODULE hModule = NULL;
    LPVOID pvClientData = NULL;
    DWORD cbClientData = 0;
    HRESULT hr;

    ICLRMetaHost * pMetaHost = NULL;
    IEnumUnknown * pEnum = NULL;
    IUnknown * pUnk = NULL;
    ICLRRuntimeInfo * pRuntime = NULL;
    ICLRProfiling * pProfiling = NULL;

    g_fConsoleMode = fConsoleMode;

    DWORD dwProfileeProcessID = (DWORD) pid;

    CLSID clsidProfiler;
    CLSIDFromString(PROFILER_GUID_WCHAR, &clsidProfiler);
    
    DWORD dwMillisecondsMax = pProfConfig->dwDefaultTimeoutMs;

    bool fCLRFound = false;

    //---------------------------------------------------------------------------------------
    // GET AND CALL API
    //---------------------------------------------------------------------------------------

    hModule = LoadLibrary(L"mscoree.dll");
    if (hModule == NULL)
    {
        Log(L"LoadLibrary mscoree.dll failed.  hr=0x%x.\n", hr = HRESULT_FROM_WIN32(GetLastError()));
        goto Cleanup;
    }

    // Note: This is the ONLY C export we need out of mscoree.dll.  This enables us to
    // get started with the metahost interfaces, and it's all COM from that point
    // forward.
    CLRCreateInstanceFnPtr pfnCreateInstance = 
        (CLRCreateInstanceFnPtr)GetProcAddress(hModule, "CLRCreateInstance");
    if (pfnCreateInstance == NULL)
    {
        Log(L"GetProcAddress on 'CLRCreateInstance' failed.  hr=0x%x.\n", hr = HRESULT_FROM_WIN32(GetLastError()));
        goto Cleanup;
    }

    hr = (*pfnCreateInstance)(CLSID_CLRMetaHost, IID_ICLRMetaHost, (LPVOID *)&pMetaHost);
    if (FAILED(hr))
    {
        Log(L"CLRCreateInstance IID_ICLRMetaHost' failed.  hr=0x%x.\n", hr);
        goto Cleanup;
    }

    // Cross-user attach requires the SE_DEBUG_NAME privilege, so attempt to enable it
    // now.  Even if the privilege is not found, the CLRProfiler still continues to attach the target process.
    // We'll just fail later on if we do try to attach to a different-user target process.
    HRESULT hrEnableDebugPrivilege = EnableDebugPrivilege();
    
    HANDLE hndProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProfileeProcessID);
    if (hndProcess == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        //If EnableDebugPrivilege is not successful, let the customer know running CLRProfiler as administrator may solve the problem.
        if (hrEnableDebugPrivilege == E_FAIL) 
        {
            Log(L"CLRProfiler can not open the target process %d (error: 0x%x), probably because CLRProfiler could not enable the debug privilege (error: 0x%x).  \n"
                L"Please run the CLRProfiler as administrator and try again.", dwProfileeProcessID, hr, hrEnableDebugPrivilege);
        }
        else
        {
            Log(L"OpenProcess failed.  hr=0x%x.\n", hr);
        }
        goto Cleanup;
    }

    // One process may have multiple versions of the CLR loaded.  Grab an enumerator to
    // get back all the versions currently loaded.
    hr = pMetaHost->EnumerateLoadedRuntimes(hndProcess, &pEnum);
    if (FAILED(hr))
    {
        Log(L"EnumerateLoadedRuntimes' failed.  hr=0x%x.\n", hr);
        goto Cleanup;
    }

    while (pEnum->Next(1, &pUnk, NULL) == S_OK)
    {
        hr = pUnk->QueryInterface(IID_ICLRRuntimeInfo, (LPVOID *) &pRuntime);
        if (FAILED(hr))
            goto LoopCleanup;
       
        WCHAR wszVersion[30];
        DWORD cchVersion = ARRAY_LEN(wszVersion);
        hr = pRuntime->GetVersionString(wszVersion, &cchVersion);
        if (SUCCEEDED(hr) && _wcsnicmp(wszVersion, wszTargetVersion, min(cchVersion, wcslen(wszTargetVersion))) == 0)
        {
            fCLRFound = true;
            hr = pRuntime->GetInterface(CLSID_CLRProfiling, IID_ICLRProfiling, (LPVOID *)&pProfiling);
            if (FAILED(hr))
            {
                Log(L"Can not get the ICLRProfiling interface. Error: 0x%x.", hr);
                break;
            }
            // Here it is!  Attach the profiler!
            // The profilee may not have access to the profler dll 
            // Give it a try.
            hr = pProfiling->AttachProfiler(dwProfileeProcessID,
                                            dwMillisecondsMax,
                                            &clsidProfiler,
                                            wszProfilerPath,
                                            (LPVOID)pProfConfig,
                                            sizeof(*pProfConfig));


            if(FAILED(hr))
            {
                if (hr == ERROR_TIMEOUT)//ERROR_TIMEOUT 
                {
                    Log(L"CLRProfiler timed out to attach to the process.\nPlease check the event log to find out whether the attach succeeded or failed.");
                }
                else if (hr == COR_E_UNAUTHORIZEDACCESS)//0x80070005
                {
                    Log(L"CLRProfiler failed to attach to the process with error code 0x80070005(COR_E_UNAUTHORIZEDACCESS).\n"
                        L"This may occur if the target process(%d) does not have access to ProfilerOBJ.dll or the directory in which ProfilerOBJ.dll is located.\n"
                        L"Please check event log for more details.", pid);
                }
                else if (hr == CORPROF_E_CONCURRENT_GC_NOT_PROFILABLE)
                {
                    Log(L"Profiler initialization failed because the target process is running with concurrent GC enabled. Either\n"
                        L"  1) turn off concurrent GC in the application's configuration file before launching the application, or\n" 
                        L"  2) simply start the application from CLRProfiler rather than trying to attach CLRProfiler after the application has already started.");
                }
                else
                {
                    Log(L"Attach Profiler Failed 0x%x, please check the event log for more details.", hr);
                }
                    
            }

            pProfiling->Release();
            pProfiling = NULL;
            break;
        }

LoopCleanup:
        if (pRuntime != NULL)
        {
            pRuntime->Release();
            pRuntime = NULL;
        }

        if (pUnk != NULL)
        {
            pUnk->Release();
            pUnk = NULL;
        }
    }

    if (fCLRFound == false)
    {
        Log(L"No CLR Version %s is loaded into the target process.", wszTargetVersion);
        hr = E_FAIL;
    }



Cleanup:

    if (pRuntime != NULL)
    {
        pRuntime->Release();
        pRuntime = NULL;
    }

    if (pUnk != NULL)
    {
        pUnk->Release();
        pUnk = NULL;
    }

    if (pEnum != NULL)
    {
        pEnum->Release();
        pEnum = NULL;
    }

    if (pMetaHost != NULL)
    {
        pMetaHost->Release();
        pMetaHost = NULL;
    }

    if (hModule != NULL)
    {
        FreeLibrary(hModule);
        hModule = NULL;
    }

    return hr;
}

/***************************************************************************************
 *  Method:
 *
 *
 *  Purpose:
 *
 *
 *  Parameters: 
 *
 *
 *  Return value:
 *
 *
 *  Notes:
 *
 ***************************************************************************************/
// Attempts to enable the SE_DEBUG_NAME privilege.  If the privilege is unavailable,
// this intentionally returns S_OK, as same-user attach is still supported without need
// for the SE_DEBUG_NAME privilege
HRESULT EnableDebugPrivilege()
{
    HRESULT hr;
    HANDLE hProcessToken = NULL;
    LPBYTE pbTokenInformation = NULL;

    if (!OpenProcessToken(
        GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
        &hProcessToken))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        Log(L"Error returned from OpenProcessToken.  hr = 0x%x.\n", hr);
        goto Cleanup;
    }

    DWORD cbTokenInformationOut;
    if (!GetTokenInformation(
        hProcessToken,
        TokenPrivileges,
        NULL,               // TokenInformation
        0,                  // TokenInformationLength,
        &cbTokenInformationOut))
    {
        DWORD dwLastError = GetLastError();
        if (dwLastError != ERROR_INSUFFICIENT_BUFFER)
        {
            hr = HRESULT_FROM_WIN32(dwLastError);
            Log(L"Error returned from GetTokenInformation.  hr = 0x%x.\n", hr);
            goto Cleanup;
        }
    }

    DWORD cbTokenInformation = cbTokenInformationOut;
    pbTokenInformation = new BYTE[cbTokenInformation];
    if (pbTokenInformation == NULL)
    {
        hr = E_OUTOFMEMORY;
        Log(L"Unable to allocate %d bytes for token information.\n", cbTokenInformation);
        goto Cleanup;
    }

    if (!GetTokenInformation(
        hProcessToken,
        TokenPrivileges,
        pbTokenInformation,
        cbTokenInformation,
        &cbTokenInformationOut))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        Log(L"Error returned from GetTokenInformation.  hr = 0x%x.\n", hr);
        goto Cleanup;
    }

    TOKEN_PRIVILEGES * pPrivileges = (TOKEN_PRIVILEGES *) pbTokenInformation;
    BOOL fFoundDebugPrivilege = FALSE;
    LUID_AND_ATTRIBUTES * pLuidAndAttrs = NULL;

    for (DWORD i=0; i < pPrivileges->PrivilegeCount; i++)
    {
        pLuidAndAttrs = &(pPrivileges->Privileges[i]);
        WCHAR wszPrivilegeName[100];
        DWORD cchPrivilegeName = ARRAY_LEN(wszPrivilegeName);
        if (!LookupPrivilegeName(
            NULL,       // lpSystemName
            &(pLuidAndAttrs->Luid),
            &(wszPrivilegeName[0]),
            &cchPrivilegeName))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            Log(L"Error returned from LookupPrivilegeName.  hr = 0x%x.\n", hr);
            goto Cleanup;
        }

        if (wcscmp(wszPrivilegeName, SE_DEBUG_NAME) == 0)
        {
            fFoundDebugPrivilege = TRUE;
            break;
        }
    }

    if (!fFoundDebugPrivilege)
    {
        //Unable to find SeDebugPrivilege; user may not be able to profile higher integrity proceses. 
        //return silently and give it a try.
        //if the attach failed, let the customer know they can run CLRProfiler as administrator and try again. 
        hr = E_FAIL;
        goto Cleanup;
    }

    if ((pLuidAndAttrs->Attributes & SE_PRIVILEGE_ENABLED) != 0)
    {
        // Privilege already enabled.  Nothing to do.
        hr = S_OK;
        // Log(L"SeDebugPrivilege is already enabled.\n");
        goto Cleanup;
    }

    // Log(L"SeDebugPrivilege available but disabled.  Attempting to enable it...\n");
    pLuidAndAttrs->Attributes |= SE_PRIVILEGE_ENABLED;
    if (!AdjustTokenPrivileges(
        hProcessToken,
        FALSE,              // DisableAllPrivileges,
        pPrivileges,
        cbTokenInformationOut,
        NULL,               // PreviousState,
        NULL                // ReturnLength
        ))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        Log(L"Error returned from AdjustTokenPrivileges.  hr = 0x%x.\n", hr);
        goto Cleanup;
    }
    
    hr = S_OK;

Cleanup:
    if (hProcessToken != NULL)
    {
        CloseHandle(hProcessToken);
        hProcessToken = NULL;
    }

    if (pbTokenInformation != NULL)
    {
        delete [] pbTokenInformation;
        pbTokenInformation = NULL;
    }
	
    return hr;
}