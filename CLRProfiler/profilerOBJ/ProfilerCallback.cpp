// ==++==
// 
//   Copyright (c) Microsoft Corporation.  All rights reserved.
// 
// ==--==
/****************************************************************************************
 * File:
 *  ProfilerCallBack.cpp
 *
 * Description:
 *  Implements ICorProfilerCallback. Logs every event of interest to a file on disc.
 *
 ***************************************************************************************/ 

#include <windows.h>
#include <share.h>
#include <strsafe.h>

#include "basehlp.h"
#include "basehlp.hpp"

#include "avlnode.h"
#include "avlnode.hpp"

#include "ProfilerInfo.h"

#include "ProfilerCallback.h"

CRITICAL_SECTION g_criticalSection;
ProfilerCallback *g_pCallbackObject;        // global reference to callback object
/***************************************************************************************
 ********************                                               ********************
 ********************   Global Functions Used for Thread Support    ********************
 ********************                                               ********************
 ***************************************************************************************/

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
/* static __stdcall */
DWORD __stdcall ThreadStub( void *pObject )
{    
    ((ProfilerCallback *)pObject)->_ThreadStubWrapper();

    return 0;
                       
} // ThreadStub


DWORD __stdcall DetachThreadStub( void *pObject )
{    
    ((ProfilerCallback *)pObject)->DetachThreadStub();

    return 0;
                       
}



/***************************************************************************************
 ********************                                               ********************
 ********************   Global Functions Used by Function Hooks     ********************
 ********************                                               ********************
 ***************************************************************************************/

//
// The functions EnterStub, LeaveStub and TailcallStub are wrappers. The use of 
// of the extended attribute "__declspec( naked )" does not allow a direct call
// to a profiler callback (e.g., ProfilerCallback::Enter( functionID )).
//
// The enter/leave function hooks must necessarily use the extended attribute
// "__declspec( naked )". Please read the corprof.idl for more details. 
//

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
EXTERN_C void __stdcall EnterStub( FunctionID functionID )
{
    ProfilerCallback::Enter( functionID );
    
} // EnterStub


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
EXTERN_C void __stdcall LeaveStub( FunctionID functionID )
{
    ProfilerCallback::Leave( functionID );
    
} // LeaveStub


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
EXTERN_C void __stdcall TailcallStub( FunctionID functionID )
{
    ProfilerCallback::Tailcall( functionID );
    
} // TailcallStub


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
#ifdef _X86_
void __declspec( naked ) EnterNaked()
{
    __asm
    {
        push eax
        push ecx
        push edx
        push [esp + 16]
        call EnterStub
        pop edx
        pop ecx
        pop eax
        ret 4
    }
} // EnterNaked


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
void __declspec( naked ) LeaveNaked()
{
    __asm
    {
        push eax
        push ecx
        push edx
        push [esp + 16]
        call LeaveStub
        pop edx
        pop ecx
        pop eax
        ret 4
    }
} // LeaveNaked


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
void __declspec( naked ) TailcallNaked()
{
    __asm
    {
        push eax
        push ecx
        push edx
        push [esp + 16]
        call TailcallStub
        pop edx
        pop ecx
        pop eax
        ret 4
    }
} // TailcallNaked


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
void __declspec( naked ) EnterNaked2(FunctionID funcId, 
                                     UINT_PTR clientData, 
                                     COR_PRF_FRAME_INFO func, 
                                     COR_PRF_FUNCTION_ARGUMENT_INFO *argumentInfo)
{
    __asm
    {
        push eax
        push ecx
        push edx
        push [esp + 16]
        call EnterStub
        pop edx
        pop ecx
        pop eax
        ret 16
    }
} // EnterNaked


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
void __declspec( naked ) LeaveNaked2(FunctionID funcId, 
                                     UINT_PTR clientData, 
                                     COR_PRF_FRAME_INFO func, 
                                     COR_PRF_FUNCTION_ARGUMENT_RANGE *retvalRange)
{
    __asm
    {
        push eax
        push ecx
        push edx
        push [esp + 16]
        call LeaveStub
        pop edx
        pop ecx
        pop eax
        ret 16
    }
} // LeaveNaked


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
void __declspec( naked ) TailcallNaked2(FunctionID funcId, 
                                        UINT_PTR clientData, 
                                        COR_PRF_FRAME_INFO func)
{
    __asm
    {
        push eax
        push ecx
        push edx
        push [esp + 16]
        call TailcallStub
        pop edx
        pop ecx
        pop eax
        ret 12
    }
} // TailcallNaked


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
void __declspec( naked ) EnterNaked3(FunctionIDOrClientID functionIDOrClientID)
{
    __asm
    {
        push eax
        push ecx
        push edx
        push [esp + 16]
        call EnterStub
        pop edx
        pop ecx
        pop eax
        ret 4
    }
} // EnterNaked


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
void __declspec( naked ) LeaveNaked3(FunctionIDOrClientID functionIDOrClientID)
{
    __asm
    {
        push eax
        push ecx
        push edx
        push [esp + 16]
        call LeaveStub
        pop edx
        pop ecx
        pop eax
        ret 4
    }
} // LeaveNaked


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
void __declspec( naked ) TailcallNaked3(FunctionIDOrClientID functionIDOrClientID)
{
    __asm
    {
        push eax
        push ecx
        push edx
        push [esp + 16]
        call TailcallStub
        pop edx
        pop ecx
        pop eax
        ret 4
    }
} // TailcallNaked

#elif defined(_AMD64_)
// these are linked in AMD64 assembly (amd64\asmhelpers.asm)
EXTERN_C void EnterNaked2(FunctionID funcId, 
                          UINT_PTR clientData, 
                          COR_PRF_FRAME_INFO func, 
                          COR_PRF_FUNCTION_ARGUMENT_INFO *argumentInfo);
EXTERN_C void LeaveNaked2(FunctionID funcId, 
                          UINT_PTR clientData, 
                          COR_PRF_FRAME_INFO func, 
                          COR_PRF_FUNCTION_ARGUMENT_RANGE *retvalRange);
EXTERN_C void TailcallNaked2(FunctionID funcId, 
                             UINT_PTR clientData, 
                             COR_PRF_FRAME_INFO func);

EXTERN_C void EnterNaked3(FunctionIDOrClientID functionIDOrClientID);
EXTERN_C void LeaveNaked3(FunctionIDOrClientID functionIDOrClientID);
EXTERN_C void TailcallNaked3(FunctionIDOrClientID functionIDOrClientID);
#endif // _X86_    



DWORD flsIndex;

/***************************************************************************************
 ********************                                               ********************
 ********************     ProfilerCallBack Implementation           ********************
 ********************                                               ********************
 ***************************************************************************************/


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
/* public */

ProfilerCallback::ProfilerCallback() :
    PrfInfo(),
    m_condemnedGenerationIndex( 0 ),
    m_path( NULL ),
    m_hPipe( NULL ),
    m_dwMode( 0x3 ),
    m_refCount( 0 ),
    m_stream( NULL ),
    m_lastTickCount( 0 ),
    m_dwShutdown( 0 ),
    m_totalClasses( 1 ),
    m_totalModules( 0 ),
    m_dwSkipObjects( 0 ),
    m_bInitialized( FALSE ),
    m_bShutdown( FALSE ),
    m_totalFunctions( 0 ),
    m_dwProcessId( NULL ),
    m_bDumpGCInfo( FALSE ),
    m_classToMonitor( NULL ),
    m_bDumpCompleted( FALSE ),
    m_bTrackingObjects( FALSE ),
    m_bTrackingCalls( FALSE ),
    m_totalObjectsAllocated( 0 ),
    m_dwFramesToPrint( 0xFFFFFFFF ),
    m_bIsTrackingStackTrace( FALSE ),
    m_callStackCount( 0 ),
    m_bTargetV2CLR( FALSE ),
    m_dwSentinelHandle(SENTINEL_HANDLE),
	m_bWindowsStoreApp(FALSE)
{
} // ctor

HRESULT ProfilerCallback::Init(ProfConfig * pProfConfig)
{
    HRESULT hr = S_OK;
    memset(&m_GCcounter, 0, sizeof(m_GCcounter));
    memset(&m_condemnedGeneration, 0, sizeof(m_condemnedGeneration));

    FunctionInfo *pFunctionInfo = NULL;

    
    TEXT_OUTLN( "CLR Object Profiler Tool - turning off profiling for child processes" )
    SetEnvironmentVariableW(L"Cor_Enable_Profiling", L"0x0");
    SetEnvironmentVariableW(L"CoreCLR_Enable_Profiling", L"0x0");
    
    //
    // initializations
    //
    if (!m_bWindowsStoreApp)
    {
        m_timerResolution = BeginTimer();
    }
    m_firstTickCount = _GetTickCount();

    if (!InitializeCriticalSectionEx( &m_criticalSection, 10000, 0 /*flags*/ ))
        hr = E_FAIL;
    if (!InitializeCriticalSectionEx( &g_criticalSection, 10000, 0 /*flags*/ ))
        hr = E_FAIL;
    g_pCallbackObject = this;

    //
    // get the processID and connect to the Pipe of the UI
    //
    m_dwProcessId = GetCurrentProcessId();
	sprintf_s( m_logFileName, ARRAY_LEN(m_logFileName), "pipe_%d.log", m_dwProcessId );
	
    // For desktop apps, shake hands with GUI via pipes.  Windows Store app handshake is much
    // simpler.  No pipes.  ProfilerObj.dll just creates the log file, and
    // the GUI polls to see if the log file has been created
    if (!m_bWindowsStoreApp)
    {
        _ConnectToUI();
    }

    //
    // set the event and callback names
    //
    if (!SUCCEEDED(_InitializeNamesForEventsAndCallbacks()))
        hr = E_FAIL;

    if ( SUCCEEDED(hr) )
    {
        //
        // look if the user specified another path to save the output file
        //
        if ( pProfConfig->szFileName[0] != '\0' )
        {
            // room for buffer chars + '\' + logfilename chars + '\0':
            const size_t len = strlen(pProfConfig->szFileName) + 1;
            m_path = new char[len];
            if ( m_path != NULL )
                strcpy_s( m_path, len, pProfConfig->szFileName ); 
        }
        else if ( pProfConfig->szPath[0] != '\0' )
        {
            // room for buffer chars + '\' + logfilename chars + '\0':
            const size_t len = strlen(pProfConfig->szPath) + strlen(m_logFileName) + 2;
            m_path = new char[len];
            if ( m_path != NULL )
                sprintf_s( m_path, len, "%s\\%s", pProfConfig->szPath, m_logFileName ); 
        }

        if (m_bWindowsStoreApp)
        {
            // For Windows Store app handshake, ProfilerObj.dll creates the events now
            // (while the GUI waits for the log file to be created), then the GUI
            // connects to the events after the log file is created below.
            hr = _InitializeThreadsAndEvents();
            if ( FAILED( hr ) )
                Failure( "Unable to initialize the threads and handles, No profiling" );
        }

        //
        // open the correct file stream fo dump the logging information
        //
        m_stream = _fsopen((m_path != NULL) ? m_path : m_logFileName, "w+", _SH_DENYWR);
        hr = ( m_stream == NULL ) ? E_FAIL : S_OK;
        if ( SUCCEEDED( hr ) )
        {
            setvbuf(m_stream, NULL, _IOFBF, 32768);
            //
            // add an entry for the stack trace in case of managed to unamanged transitions
            //
            pFunctionInfo = new FunctionInfo( NULL, m_totalFunctions );     
            hr = ( pFunctionInfo == NULL ) ? E_FAIL : S_OK;
            if ( SUCCEEDED( hr ) )
            {
                wcscpy_s( pFunctionInfo->m_functionName, ARRAY_LEN(pFunctionInfo->m_functionName), L"NATIVE FUNCTION" );
                wcscpy_s( pFunctionInfo->m_functionSig, ARRAY_LEN(pFunctionInfo->m_functionSig), L"( UNKNOWN ARGUMENTS )" );

                m_pFunctionTable->AddEntry( pFunctionInfo, NULL );
                LogToAny( "f %Id %S %S 0 0\n", 
                    pFunctionInfo->m_internalID, 
                    pFunctionInfo->m_functionName,
                    pFunctionInfo->m_functionSig );

                m_totalFunctions ++;
            }
            else
                TEXT_OUTLN( "Unable To Allocate Memory For FunctionInfo" )
        }
        else
            TEXT_OUTLN( "Unable to open log file - No log will be produced" )
    }

    flsIndex = FlsAlloc(NULL /* PFLS_CALLBACK_FUNCTION */);
    if (flsIndex == FLS_OUT_OF_INDEXES)
        hr = E_FAIL;

    if ( FAILED( hr ) )
        m_dwEventMask = COR_PRF_MONITOR_NONE;

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
/* public */
ProfilerCallback::~ProfilerCallback()
{
    if (!m_bInitialized)
    {
        return;
    }

    _ShutdownAllThreads();

    if ( m_path != NULL )
    {
        delete[] m_path;
        m_path = NULL;
    }
    
    if ( m_classToMonitor != NULL )
    {
        delete[] m_classToMonitor;
        m_classToMonitor = NULL;    
    }

    if ( m_stream != NULL )
    {
        fclose( m_stream );
        m_stream = NULL;
    }

    for ( DWORD i=GC_HANDLE; i<m_dwSentinelHandle; i++ )
    {
        if ( m_NamedEvents[i] != NULL )
        {
            delete[] m_NamedEvents[i];
            m_NamedEvents[i] = NULL;
        }

        if ( m_CallbackNamedEvents[i] != NULL )
        {
            delete[] m_CallbackNamedEvents[i];
            m_CallbackNamedEvents[i] = NULL;
        }
    }
    
    DeleteCriticalSection( &m_criticalSection );
    DeleteCriticalSection( &g_criticalSection );
    if (!m_bWindowsStoreApp)
    {
        EndTimer(m_timerResolution);
    }
    g_pCallbackObject = NULL;

} // dtor

        
UINT ProfilerCallback::BeginTimer()
{
#ifdef _X86_
	if (!m_bWindowsStoreApp)
	{
		const UINT TARGET_RESOLUTION = 1;         // 1-millisecond target resolution

		TIMECAPS tc;
		UINT     wTimerRes;

		if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR) 
		{
			return 0;
		}

		wTimerRes = min(max(tc.wPeriodMin, TARGET_RESOLUTION), tc.wPeriodMax);
		timeBeginPeriod(wTimerRes);

		return wTimerRes;
	}
#endif // _X86_    
    return 0;
}

void ProfilerCallback::EndTimer(UINT wTimerRes)
{
#ifdef _X86_
	if (!m_bWindowsStoreApp)
	{
		if (wTimerRes != 0)
			timeEndPeriod(wTimerRes);
	}
#endif // _X86_    
}

ULONGLONG ProfilerCallback::_GetTickCount()
{
    // We have a bit of a problem here - GetTickCount() doesn't quite
    // have enough resolution, and QueryPerformanceCounter gives inconsistent
    // results on some machines, so we use the multimedia timer functions and 
    // attempt to bump up the resolution to 1 millisecond.
#ifdef _X86_
	if (!m_bWindowsStoreApp)
	{
	    return timeGetTime();
	}
#endif
    return GetTickCount64();
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
/* public */
ULONG ProfilerCallback::AddRef() 
{

    return InterlockedIncrement( &m_refCount );

} // ProfilerCallback::AddRef


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
/* public */
ULONG ProfilerCallback::Release() 
{
    long refCount;


    refCount = InterlockedDecrement( &m_refCount );
    if ( refCount == 0 )
        delete this;
     

    return refCount;

} // ProfilerCallback::Release


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
/* public */
HRESULT ProfilerCallback::QueryInterface( REFIID riid, void **ppInterface )
{
    if ( riid == IID_IUnknown )
        *ppInterface = static_cast<IUnknown *>( this ); 
    else if ( riid == IID_ICorProfilerCallback )
        *ppInterface = static_cast<ICorProfilerCallback *>( this );
    else if ( riid == IID_ICorProfilerCallback2 )
        *ppInterface = static_cast<ICorProfilerCallback2 *>( this );
    else if ( riid == IID_ICorProfilerCallback3 )
        *ppInterface = static_cast<ICorProfilerCallback3 *>( this );
    else
    {
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }
    
    reinterpret_cast<IUnknown *>( *ppInterface )->AddRef();

    return S_OK;

} // ProfilerCallback::QueryInterface 


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
/* public static */
HRESULT ProfilerCallback::CreateObject( REFIID riid, void **ppInterface )
{
    HRESULT hr = E_NOINTERFACE;
    
     
    *ppInterface = NULL;
    if (   (riid == IID_IUnknown)
        || (riid == IID_ICorProfilerCallback)
        || (riid == IID_ICorProfilerCallback2) 
        || (riid == IID_ICorProfilerCallback3))
    {           
        ProfilerCallback *pProfilerCallback;
        
                
        pProfilerCallback = new ProfilerCallback();
        if ( pProfilerCallback != NULL )
        {
            hr = S_OK;
            
            pProfilerCallback->AddRef();

            *ppInterface = static_cast<ICorProfilerCallback *>( pProfilerCallback );
        }
        else
            hr = E_OUTOFMEMORY;
    }    
    

    return hr;

} // ProfilerCallback::CreateObject


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
/* public */

HRESULT ProfilerCallback::Initialize( IUnknown *pICorProfilerInfoUnk )
{     
    HRESULT hr;
    //
    // define in which mode you will operate
    //
    ProfConfig profConfig;
    _GetProfConfigFromEnvironment(&profConfig);
    _ProcessProfConfig(&profConfig);

    hr = pICorProfilerInfoUnk->QueryInterface( IID_ICorProfilerInfo,
                                               (void **)&m_pProfilerInfo );   
    if (FAILED(hr))
        return hr;

    hr = pICorProfilerInfoUnk->QueryInterface( IID_ICorProfilerInfo2,
                                               (void **)&m_pProfilerInfo2 );
    if (FAILED(hr))
        return hr;

    pICorProfilerInfoUnk->QueryInterface( IID_ICorProfilerInfo3,
                                          (void **)&m_pProfilerInfo3 );

    if (m_bTargetV2CLR)
    {
        if (m_pProfilerInfo3 != NULL)
        {
            SendMessageToUI("v4 CLR is loaded.  CLRProfiler that targets v2 CLR is waiting for v2 CLR.");
            return CORPROF_E_PROFILER_CANCEL_ACTIVATION;
        }
    }
    else 
    {
        if (m_pProfilerInfo3 == NULL)
        {
            SendMessageToUI("v2 CLR is loaded.  CLRProfiler that targets v4 CLR is waiting for v4 CLR.");
            return E_FAIL;
        }
    }

    hr = m_pProfilerInfo->SetEventMask( m_dwEventMask );
    if (FAILED(hr))
    {
        Failure( "SetEventMask for Profiler Test FAILED" );           
        return hr;
    }

    if (m_bTargetV2CLR)
    {
#if defined(_X86_)
        hr = m_pProfilerInfo2->SetEnterLeaveFunctionHooks2( EnterNaked2,
                                                            LeaveNaked2,
                                                            TailcallNaked2 );
#elif defined(_AMD64_)
        hr = m_pProfilerInfo2->SetEnterLeaveFunctionHooks2( (FunctionEnter2 *)EnterNaked2,
                                                            (FunctionLeave2 *)LeaveNaked2,
                                                            (FunctionTailcall2 *)TailcallNaked2 );
#endif
    }
    else
    {
#if defined(_X86_)
        hr = m_pProfilerInfo3->SetEnterLeaveFunctionHooks3( EnterNaked3,
                                                            LeaveNaked3,
                                                            TailcallNaked3 );
#elif defined(_AMD64_)
        hr = m_pProfilerInfo3->SetEnterLeaveFunctionHooks3( (FunctionEnter3 *)EnterNaked3,
                                                            (FunctionLeave3 *)LeaveNaked3,
                                                            (FunctionTailcall3 *)TailcallNaked3 );
#endif
    }

    if (FAILED(hr))
    {
        Failure( "ICorProfilerInfo::SetEnterLeaveFunctionHooks() FAILED" );
        return hr;
    }

    hr = Init(&profConfig);
    if ( FAILED( hr ) )
    {
        Failure( "CLRProfiler initialization FAILED" );
        return hr;
    }

    // For Windows Store app, the threads/events were created in Init()
    if (!m_bWindowsStoreApp)
    {
        hr = _InitializeThreadsAndEvents();
        if ( FAILED( hr ) )
            Failure( "Unable to initialize the threads and handles, No profiling" );
        Sleep(100); // Give the threads a chance to read any signals that are already set.
    }

    return S_OK;

 } // ProfilerCallback::Initialize


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
/* public */
HRESULT ProfilerCallback::Shutdown()
{
    m_dwShutdown++;

    return S_OK;          

} // ProfilerCallback::Shutdown


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
/* public */
HRESULT ProfilerCallback::DllDetachShutdown()
{
    //
    // If no shutdown occurs during DLL_DETACH, release the callback
    // interface pointer. This scenario will more than likely occur
    // with any interop related program (e.g., a program that is 
    // comprised of both managed and unmanaged components).
    //
    m_dwShutdown++;
    if ( (m_dwShutdown == 1) && (g_pCallbackObject != NULL) )
    {
        g_pCallbackObject->Release();   
        g_pCallbackObject = NULL;
    }

    
    return S_OK;          

} // ProfilerCallback::DllDetachShutdown


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
/* public */

__forceinline ThreadInfo *ProfilerCallback::GetThreadInfo()
{
    DWORD lastError = GetLastError();
    ThreadInfo *threadInfo = (ThreadInfo *)FlsGetValue(flsIndex);
    if (threadInfo != NULL)
    {
        SetLastError(lastError);
        return threadInfo;
    }

    ThreadID threadID = 0;
    HRESULT hr = g_pCallbackObject->m_pProfilerInfo->GetCurrentThreadID(&threadID);
    if (SUCCEEDED(hr))
    {
        threadInfo = g_pCallbackObject->m_pThreadTable->Lookup( threadID );
        if (threadInfo == NULL)
        {
            g_pCallbackObject->AddThread( threadID );
            threadInfo = g_pCallbackObject->m_pThreadTable->Lookup( threadID );
        }
        FlsSetValue(flsIndex, threadInfo);
    }

    SetLastError(lastError);

    return threadInfo;
}

__forceinline void ProfilerCallback::Enter( FunctionID functionID )
{
#if 0
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( g_pCallbackObject->m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    try
    {
        g_pCallbackObject->UpdateCallStack( functionID, PUSH );

        //
        // log tracing info if requested
        //
        if ( g_pCallbackObject->m_dwMode & (DWORD)TRACE )
            g_pCallbackObject->_LogCallTrace( functionID );

    }
    catch ( BaseException *exception )
    {       
        exception->ReportFailure();
        delete exception;
        
        g_pCallbackObject->Failure();               
    }
#else
    ThreadInfo *pThreadInfo = GetThreadInfo();

    if (pThreadInfo != NULL)
        pThreadInfo->m_pThreadCallStack->Push( functionID );

    //
    // log tracing info if requested
    //
    // g_pCallbackObject->LogToAny("mode: %d\n", g_pCallbackObject->m_dwMode);
    if ( g_pCallbackObject->m_dwMode & (DWORD)TRACE )
        g_pCallbackObject->_LogCallTrace( functionID );
#endif
} // ProfilerCallback::Enter


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
/* public */
__forceinline void ProfilerCallback::Leave( FunctionID functionID )
{
#if 0
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( g_pCallbackObject->m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    try
    {
        g_pCallbackObject->UpdateCallStack( functionID, POP );
    }
    catch ( BaseException *exception )
    {       
        exception->ReportFailure();
        delete exception;
        
        g_pCallbackObject->Failure();               
    }
#else
    ThreadInfo *pThreadInfo = GetThreadInfo();
    if (pThreadInfo != NULL)
        pThreadInfo->m_pThreadCallStack->Pop();
#endif
} // ProfilerCallback::Leave


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
/* public */
void ProfilerCallback::Tailcall( FunctionID functionID )
{
#if 0
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( g_pCallbackObject->m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    try
    {
        g_pCallbackObject->UpdateCallStack( functionID, POP );
    }
    catch ( BaseException *exception )
    {       
        exception->ReportFailure();
        delete exception;
        
        g_pCallbackObject->Failure();               
    }
#else
    ThreadInfo *pThreadInfo = GetThreadInfo();
    if (pThreadInfo != NULL)
        pThreadInfo->m_pThreadCallStack->Pop();
#endif
} // ProfilerCallback::Tailcall

static bool ContainsHighUnicodeCharsOrQuoteChar(__in_ecount(strLen) WCHAR *str, size_t strLen, WCHAR quoteChar)
{
    for (size_t i = 0; i < strLen; i++)
    {
        WCHAR c = str[i];
        if (c == quoteChar || c == '\\' || c >= 0x100)
            return true;
        if (c == 0)
            break;
    }
    return false;
}

static void InsertEscapeChars(__inout_ecount(strLen) WCHAR *str, size_t strLen, WCHAR quoteChar)
{
    WCHAR quotedName[4*MAX_LENGTH];

    size_t count = 0;
    for (size_t i = 0; i < strLen; i++)
    {
        WCHAR c = str[i];
        if (c == 0)
            break;
        if (c == quoteChar || c == L'\\')
        {
            quotedName[count++] = L'\\';
            quotedName[count++] = c;
        }
        else if (c >= 0x80)
        {
            static WCHAR hexChar[17] = L"0123456789abcdef";

            quotedName[count++] = '\\';
            quotedName[count++] = 'u';
            quotedName[count++] = hexChar[(c >> 12) & 0x0f];
            quotedName[count++] = hexChar[(c >>  8) & 0x0f];
            quotedName[count++] = hexChar[(c >>  4) & 0x0f];
            quotedName[count++] = hexChar[(c >>  0) & 0x0f];
        }
        else
        {
            quotedName[count++] = c;
        }
        if (count >= ARRAY_LEN(quotedName)-7 || count >= strLen - 7)
            break;
    }
    quotedName[count++] = 0;
    wcsncpy_s(str, strLen, quotedName, _TRUNCATE);
}

WCHAR *SanitizeUnicodeName(__inout_ecount(strLen) WCHAR *str, size_t strLen, WCHAR quoteChar)
{
    if (ContainsHighUnicodeCharsOrQuoteChar(str, strLen, quoteChar))
        InsertEscapeChars(str, strLen, quoteChar);

    return str;
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
/* public */
HRESULT ProfilerCallback::ModuleLoadFinished( ModuleID moduleID,
                                              HRESULT hrStatus )
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  
    try
    {           
        ModuleInfo *pModuleInfo = NULL;


        AddModule( moduleID, m_totalModules );       
        pModuleInfo = m_pModuleTable->Lookup( moduleID );                                               

        _ASSERTE( pModuleInfo != NULL );

        SIZE_T stackTraceId = _StackTraceId();

        LogToAny( "m %Id %S 0x%p %Id\n", 
                  pModuleInfo->m_internalID, 
                  pModuleInfo->m_moduleName,
                  pModuleInfo->m_loadAddress,
                  stackTraceId);
        
        InterlockedIncrement( &m_totalModules );
    }
    catch ( BaseException *exception )
    {
        exception->ReportFailure();
        delete exception;
       
        Failure();    
    }

    return S_OK;

} // ProfilerCallback::ModuleLoadFinished


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
/* public */
HRESULT ProfilerCallback::JITCompilationStarted( FunctionID functionID,
                                                 BOOL fIsSafeToBlock )
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    try
    {           
        AddFunction( functionID, m_totalFunctions );       
        m_totalFunctions++;
    }
    catch ( BaseException *exception )
    {
        exception->ReportFailure();
        delete exception;
       
        Failure();    
    }


    return S_OK;
    
} // ProfilerCallback::JITCompilationStarted


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
/* public */
HRESULT ProfilerCallback::JITCachedFunctionSearchStarted( FunctionID functionID,
                                                          BOOL *pbUseCachedFunction )
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    // use the pre-jitted function
    *pbUseCachedFunction = TRUE;

    try
    {           
        AddFunction( functionID, m_totalFunctions );       
        m_totalFunctions++;
    }
    catch ( BaseException *exception )
    {
        exception->ReportFailure();
        delete exception;
       
        Failure();    
    }


    return S_OK;
       
} // ProfilerCallback::JITCachedFunctionSearchStarted


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
/* public */
HRESULT ProfilerCallback::JITCompilationFinished( FunctionID functionID,
                                                  HRESULT hrStatus,
                                                  BOOL fIsSafeToBlock )
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  


    HRESULT hr;
    ULONG size;
    LPCBYTE address;
    FunctionInfo *pFunctionInfo = NULL;


    pFunctionInfo = m_pFunctionTable->Lookup( functionID );                                             

    _ASSERTE( pFunctionInfo != NULL );
    hr = m_pProfilerInfo->GetCodeInfo( functionID, &address, &size );
    if ( FAILED( hr ) )
    {
        address = NULL;
        size = 0;
//      This can actually happen unfortunately due to EE limitations
//      Failure( "ICorProfilerInfo::GetCodeInfo() FAILED" );
    }

    ModuleID moduleID = 0;
    ModuleInfo *pModuleInfo = NULL;

    hr = m_pProfilerInfo->GetFunctionInfo( functionID, NULL, &moduleID, NULL );
    if ( SUCCEEDED( hr ) )
    {
        pModuleInfo = m_pModuleTable->Lookup( moduleID );
    }

    SIZE_T stackTraceId = _StackTraceId();

    LogToAny( "f %Id %S %S 0x%p %ld %Id %Id\n", 
                pFunctionInfo->m_internalID, 
                SanitizeUnicodeName(pFunctionInfo->m_functionName, ARRAY_LEN(pFunctionInfo->m_functionName), L' '),
                SanitizeUnicodeName(pFunctionInfo->m_functionSig,  ARRAY_LEN(pFunctionInfo->m_functionSig), L'\0'),
                address,
                size,
                pModuleInfo ? pModuleInfo->m_internalID : 0,
                stackTraceId);

    return S_OK;
    
} // ProfilerCallback::JITCompilationFinished


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
/* public */
HRESULT ProfilerCallback::JITCachedFunctionSearchFinished( FunctionID functionID,
                                                           COR_PRF_JIT_CACHE result )
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  


    if ( result == COR_PRF_CACHED_FUNCTION_FOUND )
    {
        HRESULT hr;
        ULONG size;
        LPCBYTE address;
        FunctionInfo *pFunctionInfo = NULL;


        pFunctionInfo = m_pFunctionTable->Lookup( functionID );                                             

        _ASSERTE( pFunctionInfo != NULL );
        hr = m_pProfilerInfo->GetCodeInfo( functionID, &address, &size );
        if ( FAILED( hr ) )
        {
            address = NULL;
            size = 0;
    //      This can actually happen unfortunately due to EE limitations
    //      Failure( "ICorProfilerInfo::GetCodeInfo() FAILED" );
        }
        ModuleID moduleID = 0;
        ModuleInfo *pModuleInfo = NULL;

        hr = m_pProfilerInfo->GetFunctionInfo( functionID, NULL, &moduleID, NULL );
        if ( SUCCEEDED( hr ) )
        {
            pModuleInfo = m_pModuleTable->Lookup( moduleID );
        }
        SIZE_T stackTraceId = _StackTraceId();

        LogToAny( "f %Id %S %S 0x%p %Id %Id %Id\n", 
                    pFunctionInfo->m_internalID, 
                    pFunctionInfo->m_functionName,
                    pFunctionInfo->m_functionSig,
                    address,
                    size,
                    pModuleInfo ? pModuleInfo->m_internalID : 0,
                    stackTraceId);
    }

    return S_OK;
      
} // ProfilerCallback::JITCachedFunctionSearchFinished


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
/* public */
HRESULT ProfilerCallback::ExceptionUnwindFunctionEnter( FunctionID functionID )
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( g_pCallbackObject->m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    if ( functionID != NULL )
    {
        try
        {
            UpdateUnwindStack( &functionID, PUSH );
        }
        catch ( BaseException *exception )
        {       
            exception->ReportFailure();
            delete exception;
            
            Failure();              
        }
    }
    else
        Failure( "ProfilerCallback::ExceptionUnwindFunctionEnter returned NULL functionID FAILED" );


    return S_OK;

} // ProfilerCallback::ExceptionUnwindFunctionEnter


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
/* public */
HRESULT ProfilerCallback::ExceptionUnwindFunctionLeave( )
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    FunctionID poppedFunctionID = NULL;


    try
    {
        UpdateUnwindStack( &poppedFunctionID, POP );
        UpdateCallStack( poppedFunctionID, POP );
    }
    catch ( BaseException *exception )
    {       
        exception->ReportFailure();
        delete exception;
        
        Failure();              
    }


    return S_OK;

} // ProfilerCallback::ExceptionUnwindFunctionLeave


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
/* public */ 
HRESULT ProfilerCallback::ThreadCreated( ThreadID threadID )
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( g_pCallbackObject->m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    try
    {
        AddThread( threadID ); 
    }
    catch ( BaseException *exception )
    {       
        exception->ReportFailure();
        delete exception;
        
        Failure();              
    }


    return S_OK; 
    
} // ProfilerCallback::ThreadCreated


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
/* public */
HRESULT ProfilerCallback::ThreadDestroyed( ThreadID threadID )
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( g_pCallbackObject->m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    try
    {
        RemoveThread( threadID ); 
        FlsSetValue(flsIndex, NULL);
    }
    catch ( BaseException *exception )
    {       
        exception->ReportFailure();
        delete exception;
        
        Failure();              
    }
        

    return S_OK;
    
} // ProfilerCallback::ThreadDestroyed

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
/* public */
HRESULT ProfilerCallback::ThreadAssignedToOSThread( ThreadID managedThreadID,
                                                    DWORD osThreadID ) 
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( g_pCallbackObject->m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    if ( managedThreadID != NULL )
    {
        if ( osThreadID != NULL )
        {
            try
            {
                UpdateOSThreadID( managedThreadID, osThreadID ); 
            }
            catch ( BaseException *exception )
            {       
                exception->ReportFailure();
                delete exception;
                
                Failure();              
            }
        }
        else
            Failure( "ProfilerCallback::ThreadAssignedToOSThread() returned NULL OS ThreadID" );
    }
    else
        Failure( "ProfilerCallback::ThreadAssignedToOSThread() returned NULL managed ThreadID" );


    return S_OK;
    
} // ProfilerCallback::ThreadAssignedToOSThread


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
/* public */
HRESULT ProfilerCallback::UnmanagedToManagedTransition( FunctionID functionID,
                                                        COR_PRF_TRANSITION_REASON reason )
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    if ( reason == COR_PRF_TRANSITION_RETURN )
    {
        try
        {
            // you need to pop the pseudo function Id from the stack
            UpdateCallStack( functionID, POP );
        }
        catch ( BaseException *exception )
        {       
            exception->ReportFailure();
            delete exception;
            
            Failure();              
        }
    }


    return S_OK;

} // ProfilerCallback::UnmanagedToManagedTransition


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
/* public */
HRESULT ProfilerCallback::ManagedToUnmanagedTransition( FunctionID functionID,
                                                        COR_PRF_TRANSITION_REASON reason )
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    if ( reason == COR_PRF_TRANSITION_CALL )
    {
        try
        {
            // record the start of an unmanaged chain
            UpdateCallStack( NULL, PUSH );
            //
            // log tracing info if requested
            //
            if ( m_dwMode & (DWORD)TRACE )
                _LogCallTrace( NULL );
            
        }
        catch ( BaseException *exception )
        {       
            exception->ReportFailure();
            delete exception;
            
            Failure();              
        }
    }

    return S_OK;

} // ProfilerCallback::ManagedToUnmanagedTransition


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
/* public */

static char *puthex(__out_ecount(32) char *p, SIZE_T val)
{
    static char hexDig[]  = "0123456789abcdef";

    *p++ = ' ';
    *p++ = '0';
    *p++ = 'x';

    char    digStack[sizeof(val)*2];

    int digCount = 0;
    do
    {
        #pragma warning(suppress:4068) // suppress "Unknown pragma" warning from compiler
        #pragma prefast(suppress:394)  // suppress "Potential buffer overrun while writing to buffer 'digStack' warning from prefast
        digStack[digCount++] = hexDig[val % 16];
        val /= 16;
    }
    while (val != 0);

    do
    {
        *p++ = digStack[--digCount];
    }
    while (digCount > 0);

    return p;
}

static char *putdec(__out_ecount(32) char *p, SIZE_T val)
{
    *p++ = ' ';

    char    digStack[sizeof(val)*3];

    int digCount = 0;
    do
    {
        SIZE_T newval = val / 10;
        #pragma warning(suppress:4068) // suppress "Unknown pragma" warning from compiler
        #pragma prefast(suppress:394)  // suppress "Potential buffer overrun while writing to buffer 'digStack' warning from prefast
        digStack[digCount++] = (char)(val - newval*10 + '0');
        val = newval;
    }
    while (val != 0);

    do
    {
        *p++ = digStack[--digCount];
    }
    while (digCount > 0);

    return p;
}

#ifdef _X86_
#define CLOCK_TICK_INC    (500*1000)
#else
#define CLOCK_TICK_INC    (1)
#endif

void ProfilerCallback::_LogTickCount()
{
    ULONGLONG tickCount = _GetTickCount();
    if (tickCount != m_lastTickCount)
    {
        m_lastTickCount = tickCount;
        LogToAny("i %u\n", (DWORD) (tickCount - m_firstTickCount));
    }
}

HRESULT ProfilerCallback::ObjectAllocated( ObjectID objectID,
                                           ClassID classID )
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    HRESULT hr = S_OK;
    
    try
    {
        ULONG mySize = 0;

        ThreadInfo *pThreadInfo = GetThreadInfo();

        if ( pThreadInfo != NULL )
        {
            hr = m_pProfilerInfo->GetObjectSize( objectID, &mySize );
            if ( SUCCEEDED( hr ) )
            {
                if (_GetTickCount() - m_lastTickCount >= CLOCK_TICK_INC)
                    _LogTickCount();

                SIZE_T stackTraceId = _StackTraceId(classID, mySize);
#if 1
                char buffer[128];
                char *p = buffer;
                if (m_oldFormat)
                {
                    *p++ = 'a';
                }
                else
                {
                    *p++ = '!';
                    p = putdec(p, pThreadInfo->m_win32ThreadID);
                }
                p = puthex(p, objectID);
                p = putdec(p, stackTraceId);
                *p++ = '\n';
                fwrite(buffer, p - buffer, 1, m_stream);
#else
                if (m_oldFormat)
                {
                    LogToAny( "a 0x%p %Id\n", objectID, stackTraceId );
                }
                else
                {
                    LogToAny("! %Id 0x%p %Id\n", pThreadInfo->m_win32ThreadID, objectID, stackTraceId);
                }
#endif
            }
        }
        else
            Failure( "ERROR: ICorProfilerInfo::GetObjectSize() FAILED" );

        m_totalObjectsAllocated++;
    }
    catch ( BaseException *exception )
    {
        exception->ReportFailure();
        delete exception;
       
        Failure();
    }    

    return S_OK;

} // ProfilerCallback::ObjectAllocated


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
/* public */
HRESULT ProfilerCallback::ObjectReferences( ObjectID objectID,
                                            ClassID classID,
                                            ULONG objectRefs,
                                            ObjectID objectRefIDs[] )
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    if (m_bAttachLoaded && m_bWaitingForTheFirstGC)
        return S_OK;

    //
    // dump only in the following cases:
    //      case 1: if the user requested through a ForceGC or
    //      case 2: if you operate in stand-alone mode dump at all times
    //
    if (   (m_bDumpGCInfo == TRUE) 
        || ( ( (m_dwMode & DYNOBJECT) == 0 ) && ( (m_dwMode & OBJECT) == 1) ) )
    {
        HRESULT hr = S_OK;
        ClassInfo *pClassInfo = NULL;
        
        
        // mark the fact that the callback was received
        m_bDumpCompleted = TRUE;
        
        // dump all the information properly
        hr = _InsertGCClass( &pClassInfo, classID );
        if ( SUCCEEDED( hr ) )
        {
            //
            // remember the stack trace only if you requested the class
            //
            if ( (m_classToMonitor == NULL) || (wcsstr( pClassInfo->m_className, m_classToMonitor ) != NULL) )
            {
                ULONG size = 0;

                hr =  m_pProfilerInfo->GetObjectSize( objectID, &size );
                if ( SUCCEEDED( hr ) )
                {
#if 1
                    char    buffer[1024];

                    char *p = buffer;
                    *p++ = 'o';
                    p = puthex(p, objectID);
                    p = putdec(p, pClassInfo->m_internalID);
                    p = putdec(p, size);
                    for(ULONG i = 0; i < objectRefs; i++)
                    {
                        p = puthex(p, objectRefIDs[i]);
                        if (p - buffer > ARRAY_LEN(buffer) - 32)
                        {
                            fwrite(buffer, p - buffer, 1, m_stream);
                            p = buffer;
                        }
                    }
                    *p++ = '\n';
                    fwrite(buffer, p - buffer, 1, m_stream);
#else
                    char refs[MAX_LENGTH];

                    
                    LogToAny( "o 0x%p %Id %d ", objectID, pClassInfo->m_internalID, size );
                    refs[0] = NULL;
                    for( ULONG i=0, index=0; i < objectRefs; i++, index++ )
                    {
                        char objToString[sizeof(objectRefIDs[i])*2+5];

                        
                        sprintf_s( objToString, ARRAY_LEN(objToString), "0x%p ", (void *)objectRefIDs[i] );
                        strcat_s( refs, ARRAY_LEN(refs), objToString );
                        //
                        // buffer overrun control for next iteration
                        // every loop adds 11 chars to the array
                        //
                        if ( ((index+1)*(sizeof(objectRefIDs[i])*2+5)) >= (MAX_LENGTH-1) )
                        {
                            LogToAny( "%s ", refs );
                            refs[0] = NULL;
                            index = 0;          
                        }
                    }
                    LogToAny( "%s\n",refs );
#endif
                }
                else
                    Failure( "ERROR: ICorProfilerInfo::GetObjectSize() FAILED" );
            }
        }
        else
            Failure( "ERROR: _InsertGCClass FAILED" );
    }
    else
    {
        // to stop runtime from enumerating
        return E_FAIL;
    }

    return S_OK;

} // ProfilerCallback::ObjectReferences


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
/* public */
HRESULT ProfilerCallback::RootReferences( ULONG rootRefs,
                                          ObjectID rootRefIDs[] )
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    if (m_bAttachLoaded && m_bWaitingForTheFirstGC)
        return S_OK;

    //
    // dump only in the following cases:
    //      case 1: if the user requested through a ForceGC or
    //      case 2: if you operate in stand-alone mode dump at all times
    //
    if (   (m_bDumpGCInfo == TRUE) 
        || ( ( (m_dwMode & DYNOBJECT) == 0 ) && ( (m_dwMode & OBJECT) == 1) ) )
    {
        char rootsToString[MAX_LENGTH];


        // mark the fact that the callback was received
        m_bDumpCompleted = TRUE;
        
        // dump all the information properly
        LogToAny( "r " );
        rootsToString[0] = NULL;
        for( ULONG i=0, index=0; i < rootRefs; i++,index++ )
        {
            char objToString[sizeof(rootRefIDs[i])*2+5];

            
            sprintf_s( objToString, ARRAY_LEN(objToString), "0x%p ", (void *)rootRefIDs[i] );
            strcat_s( rootsToString, ARRAY_LEN(rootsToString), objToString );
            //
            // buffer overrun control for next iteration
            // every loop adds 16 chars to the array
            //
            if ( ((index+1)*(sizeof(rootRefIDs[i])*2+5)) >= (MAX_LENGTH-1) )
            {
                LogToAny( "%s ", rootsToString );
                rootsToString[0] = NULL;            
                index = 0;
            }
        }
        LogToAny( "%s\n",rootsToString );
    }
    else
    {
        // to stop runtime from enumerating
        return E_FAIL;
    }


    return S_OK;

} // ProfilerCallback::RootReferences


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
/* public */
HRESULT ProfilerCallback::RuntimeSuspendStarted( COR_PRF_SUSPEND_REASON suspendReason )
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( g_pCallbackObject->m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    // if we are shutting down , terminate all the threads
    if ( suspendReason == COR_PRF_SUSPEND_FOR_SHUTDOWN )
    {
        //
        // cleanup the events and threads
        //
        _ShutdownAllThreads();
    }

    return S_OK;
    
} // ProfilerCallback::RuntimeSuspendStarted


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
/* public */
HRESULT ProfilerCallback::RuntimeResumeFinished()
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    //
    // identify if this is the first Object allocated callback
    // after dumping the objects as a result of a Gc and revert the state
    //
    if ( m_bDumpGCInfo == TRUE && m_bDumpCompleted == TRUE )
    {
        // reset
        m_bDumpGCInfo = FALSE;
        m_bDumpCompleted = FALSE;

        // flush the log file so the dump is complete there, too
        fflush(m_stream);

        // give a callback to the user that the GC has been completed
        SetEvent( m_hArrayCallbacks[GC_HANDLE] );
    }

    return S_OK;

} // ProfilerCallback::RuntimeResumeFinished


/***************************************************************************************
 ********************                                               ********************
 ********************     Callbacks With Default Implementation     ********************
 ********************                                               ********************
 ***************************************************************************************/


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
/* public */
HRESULT ProfilerCallback::AppDomainCreationStarted( AppDomainID appDomainID )
{
    
    return S_OK;

} // ProfilerCallback::AppDomainCreationStarted


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
/* public */
HRESULT ProfilerCallback::AppDomainCreationFinished( AppDomainID appDomainID,
                                                     HRESULT hrStatus )
{

    return S_OK;

} // ProfilerCallback::AppDomainCreationFinished


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
/* public */
HRESULT ProfilerCallback::AppDomainShutdownStarted( AppDomainID appDomainID )
{

    return S_OK;

} // ProfilerCallback::AppDomainShutdownStarted

      

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
/* public */
HRESULT ProfilerCallback::AppDomainShutdownFinished( AppDomainID appDomainID,
                                                     HRESULT hrStatus )
{

    return S_OK;

} // ProfilerCallback::AppDomainShutdownFinished


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
/* public */
HRESULT ProfilerCallback::AssemblyLoadStarted( AssemblyID assemblyId )
{
    return S_OK;
} // ProfilerCallback::AssemblyLoadStarted


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
/* public */
HRESULT ProfilerCallback::AssemblyLoadFinished( AssemblyID assemblyId,
                                                HRESULT hrStatus )
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  
    try
    {
        ULONG size;
        WCHAR name[2048];
        ModuleID moduleId;
        AppDomainID appDomainId;
        if(SUCCEEDED(m_pProfilerInfo->GetAssemblyInfo(assemblyId, 2048, &size, name, &appDomainId, &moduleId)))
        {
            HRESULT hr = E_FAIL;
            ThreadInfo *pThreadInfo = GetThreadInfo();
            if(pThreadInfo != NULL)
            {
                LogToAny("y %d 0x%p %S\n", pThreadInfo->m_win32ThreadID, assemblyId, name);
            }
        }
    }
    catch ( BaseException *exception )
    {
        exception->ReportFailure();
        delete exception;
       
        Failure();    
    }

    return S_OK;
} // ProfilerCallback::AssemblyLoadFinished


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
/* public */
HRESULT ProfilerCallback::AssemblyUnloadStarted( AssemblyID assemblyID )
{

    return S_OK;

} // ProfilerCallback::AssemblyUnLoadStarted

      
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
/* public */
HRESULT ProfilerCallback::AssemblyUnloadFinished( AssemblyID assemblyID,
                                                  HRESULT hrStatus )
{

    return S_OK;

} // ProfilerCallback::AssemblyUnLoadFinished


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
/* public */
HRESULT ProfilerCallback::ModuleLoadStarted( ModuleID moduleID )
{

    return S_OK;

} // ProfilerCallback::ModuleLoadStarted


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
/* public */
HRESULT ProfilerCallback::ModuleUnloadStarted( ModuleID moduleID )
{

    return S_OK;

} // ProfilerCallback::ModuleUnloadStarted
      

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
/* public */
HRESULT ProfilerCallback::ModuleUnloadFinished( ModuleID moduleID,
                                                HRESULT hrStatus )
{

    return S_OK;

} // ProfilerCallback::ModuleUnloadFinished


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
/* public */
HRESULT ProfilerCallback::ModuleAttachedToAssembly( ModuleID moduleID,
                                                    AssemblyID assemblyID )
{

    return S_OK;

} // ProfilerCallback::ModuleAttachedToAssembly


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
/* public */
HRESULT ProfilerCallback::ClassLoadStarted( ClassID classID )
{

    return S_OK;

} // ProfilerCallback::ClassLoadStarted


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
/* public */
HRESULT ProfilerCallback::ClassLoadFinished( ClassID classID, 
                                             HRESULT hrStatus )
{
    /*
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  
    try
    {
        SIZE_T size;
        WCHAR name[2048];
        ModuleID moduleId;
        AppDomainID appDomainId;
        if(SUCCEEDED(m_pProfilerInfo->GetAssemblyInfo(assemblyId, 2048, &size, name, &appDomainId, &moduleId)))
        {
            HRESULT hr = E_FAIL;
            ThreadID threadID = NULL;

            hr = m_pProfilerInfo->GetCurrentThreadID(&threadID);
            if(SUCCEEDED(hr))
            {
                ThreadInfo *pThreadInfo = GetThreadInfo(threadID);
                if(pThreadInfo != NULL)
                {
                    SIZE_T stackTraceId = _StackTraceId();
                    LogToAny("d %d 0x%p %S\n", pThreadInfo->m_win32ThreadID, assemblyId, name);
                }
            }
        }
    }
    catch ( BaseException *exception )
    {
        exception->ReportFailure();
        delete exception;
       
        Failure();    
    }
    /* */

    return S_OK;
} // ProfilerCallback::ClassLoadFinished


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
/* public */
HRESULT ProfilerCallback::ClassUnloadStarted( ClassID classID )
{

    return S_OK;

} // ProfilerCallback::ClassUnloadStarted


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
/* public */
HRESULT ProfilerCallback::ClassUnloadFinished( ClassID classID, 
                                               HRESULT hrStatus )
{

    return S_OK;

} // ProfilerCallback::ClassUnloadFinished


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
/* public */
HRESULT ProfilerCallback::FunctionUnloadStarted( FunctionID functionID )
{

    return S_OK;

} // ProfilerCallback::FunctionUnloadStarted


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
/* public */
HRESULT ProfilerCallback::JITFunctionPitched( FunctionID functionID )
{
    
    return S_OK;
    
} // ProfilerCallback::JITFunctionPitched


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
/* public */
HRESULT ProfilerCallback::JITInlining( FunctionID callerID,
                                       FunctionID calleeID,
                                       BOOL *pfShouldInline )
{

    return S_OK;

} // ProfilerCallback::JITInlining


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
/* public */
HRESULT ProfilerCallback::RemotingClientInvocationStarted()
{

    return S_OK;
    
} // ProfilerCallback::RemotingClientInvocationStarted


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
/* public */
HRESULT ProfilerCallback::RemotingClientSendingMessage( GUID *pCookie,
                                                        BOOL fIsAsync )
{

    return S_OK;
    
} // ProfilerCallback::RemotingClientSendingMessage


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
/* public */
HRESULT ProfilerCallback::RemotingClientReceivingReply( GUID *pCookie,
                                                        BOOL fIsAsync )
{

    return S_OK;
    
} // ProfilerCallback::RemotingClientReceivingReply


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
/* public */
HRESULT ProfilerCallback::RemotingClientInvocationFinished()
{

   return S_OK;
    
} // ProfilerCallback::RemotingClientInvocationFinished


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
/* public */
HRESULT ProfilerCallback::RemotingServerReceivingMessage( GUID *pCookie,
                                                          BOOL fIsAsync )
{

    return S_OK;
    
} // ProfilerCallback::RemotingServerReceivingMessage


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
/* public */
HRESULT ProfilerCallback::RemotingServerInvocationStarted()
{

    return S_OK;
    
} // ProfilerCallback::RemotingServerInvocationStarted


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
/* public */
HRESULT ProfilerCallback::RemotingServerInvocationReturned()
{

    return S_OK;
    
} // ProfilerCallback::RemotingServerInvocationReturned


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
/* public */
HRESULT ProfilerCallback::RemotingServerSendingReply( GUID *pCookie,
                                                      BOOL fIsAsync )
{

    return S_OK;

} // ProfilerCallback::RemotingServerSendingReply


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
/* public */
HRESULT ProfilerCallback::RuntimeSuspendFinished()
{

    return S_OK;
    
} // ProfilerCallback::RuntimeSuspendFinished


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
/* public */
HRESULT ProfilerCallback::RuntimeSuspendAborted()
{

    return S_OK;
    
} // ProfilerCallback::RuntimeSuspendAborted


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
/* public */
HRESULT ProfilerCallback::RuntimeResumeStarted()
{

    return S_OK;
    
} // ProfilerCallback::RuntimeResumeStarted


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
/* public */
HRESULT ProfilerCallback::RuntimeThreadSuspended( ThreadID threadID )
{

    return S_OK;
    
} // ProfilerCallback::RuntimeThreadSuspended


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
/* public */
HRESULT ProfilerCallback::RuntimeThreadResumed( ThreadID threadID )
{

    return S_OK;
    
} // ProfilerCallback::RuntimeThreadResumed


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
/* public */
HRESULT ProfilerCallback::MovedReferences( ULONG cmovedObjectIDRanges,
                                           ObjectID oldObjectIDRangeStart[],
                                           ObjectID newObjectIDRangeStart[],
                                           ULONG cObjectIDRangeLength[] )
{
    if ((m_dwMode & OBJECT) == 0)
        return S_OK;

    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    if (m_bAttachLoaded && m_bWaitingForTheFirstGC)
        return S_OK;

    for (ULONG i = 0; i < cmovedObjectIDRanges; i++)
    {
#if 1
        char buffer[256];
        char *p = buffer;
        *p++ = 'u';
        p = puthex(p, oldObjectIDRangeStart[i]);
        p = puthex(p, newObjectIDRangeStart[i]);
        p = putdec(p, cObjectIDRangeLength[i]);
        *p++ = '\n';
        fwrite(buffer, p - buffer, 1, m_stream);
#else
        LogToAny("u 0x%p 0x%p %u\n", oldObjectIDRangeStart[i], newObjectIDRangeStart[i], cObjectIDRangeLength[i]);
#endif
    }

    return S_OK;

} // ProfilerCallback::MovedReferences


HRESULT ProfilerCallback::SurvivingReferences( ULONG cmovedObjectIDRanges,
                                               ObjectID objectIDRangeStart[],
                                               ULONG cObjectIDRangeLength[] )
{
    if ((m_dwMode & OBJECT) == 0)
        return S_OK;

    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    if (m_bAttachLoaded && m_bWaitingForTheFirstGC)
        return S_OK;

    for (ULONG i = 0; i < cmovedObjectIDRanges; i++)
    {
#if 1
        char buffer[256];
        char *p = buffer;
        *p++ = 'v';
        p = puthex(p, objectIDRangeStart[i]);
        p = putdec(p, cObjectIDRangeLength[i]);
        *p++ = '\n';
        fwrite(buffer, p - buffer, 1, m_stream);
#else
        LogToAny("v 0x%p %u\n", objectIDRangeStart[i], cObjectIDRangeLength[i]);
#endif
    }

    return S_OK;

} // ProfilerCallback::SurvivingReferences

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
/* public */
HRESULT ProfilerCallback::ObjectsAllocatedByClass( ULONG classCount,
                                                   ClassID classIDs[],
                                                   ULONG objects[] )
{

    return S_OK;

} // ProfilerCallback::ObjectsAllocatedByClass


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
/* public */
HRESULT ProfilerCallback::ExceptionThrown( ObjectID thrownObjectID )
{

    return S_OK;

} // ProfilerCallback::ExceptionThrown 


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
/* public */
HRESULT ProfilerCallback::ExceptionSearchFunctionEnter( FunctionID functionID )
{

    return S_OK;

} // ProfilerCallback::ExceptionSearchFunctionEnter


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
/* public */
HRESULT ProfilerCallback::ExceptionSearchFunctionLeave()
{

    return S_OK;

} // ProfilerCallback::ExceptionSearchFunctionLeave


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
/* public */
HRESULT ProfilerCallback::ExceptionSearchFilterEnter( FunctionID functionID )
{

    return S_OK;

} // ProfilerCallback::ExceptionSearchFilterEnter


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
/* public */
HRESULT ProfilerCallback::ExceptionSearchFilterLeave()
{

    return S_OK;

} // ProfilerCallback::ExceptionSearchFilterLeave 


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
/* public */
HRESULT ProfilerCallback::ExceptionSearchCatcherFound( FunctionID functionID )
{

    return S_OK;

} // ProfilerCallback::ExceptionSearchCatcherFound


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
/* public */
HRESULT ProfilerCallback::ExceptionCLRCatcherFound()
{
    return S_OK;
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
/* public */
HRESULT ProfilerCallback::ExceptionCLRCatcherExecute()
{
    return S_OK;
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
/* public */
HRESULT ProfilerCallback::ExceptionOSHandlerEnter( FunctionID functionID )
{

    return S_OK;

} // ProfilerCallback::ExceptionOSHandlerEnter

    
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
/* public */
HRESULT ProfilerCallback::ExceptionOSHandlerLeave( FunctionID functionID )
{

    return S_OK;

} // ProfilerCallback::ExceptionOSHandlerLeave


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
/* public */
HRESULT ProfilerCallback::ExceptionUnwindFinallyEnter( FunctionID functionID )
{

    return S_OK;

} // ProfilerCallback::ExceptionUnwindFinallyEnter


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
/* public */
HRESULT ProfilerCallback::ExceptionUnwindFinallyLeave()
{

    return S_OK;

} // ProfilerCallback::ExceptionUnwindFinallyLeave


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
/* public */
HRESULT ProfilerCallback::ExceptionCatcherEnter( FunctionID functionID,
                                                 ObjectID objectID )
{

    return S_OK;

} // ProfilerCallback::ExceptionCatcherEnter


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
/* public */
HRESULT ProfilerCallback::ExceptionCatcherLeave()
{

    return S_OK;

} // ProfilerCallback::ExceptionCatcherLeave


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
/* public */
HRESULT ProfilerCallback::COMClassicVTableCreated( ClassID wrappedClassID,
                                                   REFGUID implementedIID,
                                                   void *pVTable,
                                                   ULONG cSlots )
{

    return S_OK;

} // ProfilerCallback::COMClassicWrapperCreated


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
/* public */
HRESULT ProfilerCallback::COMClassicVTableDestroyed( ClassID wrappedClassID,
                                                     REFGUID implementedIID,
                                                     void *pVTable )
{

    return S_OK;

} // ProfilerCallback::COMClassicWrapperDestroyed


/* public */
HRESULT ProfilerCallback::ThreadNameChanged( 
            /* [in] */ ThreadID threadId,
            /* [in] */ ULONG cchName,
            /* [in] */ __in_ecount_opt(cchName) WCHAR name[  ])
{
    return S_OK;
} // ProfilerCallback::ThreadNameChanged


/* public */
HRESULT ProfilerCallback::FinalizeableObjectQueued( 
            /* [in] */ DWORD finalizerFlags,
            /* [in] */ ObjectID objectID)
{
    if ((m_dwMode & OBJECT) == 0)
        return S_OK;

    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    if (m_bAttachLoaded && m_bWaitingForTheFirstGC)
        return S_OK;

    LogToAny("l %u 0x%p\n", finalizerFlags, objectID);

    return S_OK;
}

/* public */
HRESULT ProfilerCallback::RootReferences2( 
            /* [in] */ ULONG cRootRefs,
            /* [size_is][in] */ ObjectID rootRefIds[  ],
            /* [size_is][in] */ COR_PRF_GC_ROOT_KIND rootKinds[  ],
            /* [size_is][in] */ COR_PRF_GC_ROOT_FLAGS rootFlags[  ],
            /* [size_is][in] */ UINT_PTR rootIds[  ])
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    if (m_bAttachLoaded && m_bWaitingForTheFirstGC)
        return S_OK;

    //
    // dump only in the following cases:
    //      case 1: if the user requested through a ForceGC or
    //      case 2: if you operate in stand-alone mode dump at all times
    //
    if (   (m_bDumpGCInfo == TRUE) 
        || ( ( (m_dwMode & DYNOBJECT) == 0 ) && ( (m_dwMode & OBJECT) == 1) ) )
    {
        for (ULONG i = 0; i < cRootRefs; i++)
        {
            if (rootKinds[i] == COR_PRF_GC_ROOT_STACK)
            {
                FunctionInfo *pFunctionInfo = m_pFunctionTable->Lookup( rootIds[i] );
                if ( pFunctionInfo != NULL )
                {
                    LogToAny("e 0x%p %u 0x%x %Iu\n", rootRefIds[i], rootKinds[i], rootFlags[i], pFunctionInfo->m_internalID);
                    continue;
                }
            }
            LogToAny("e 0x%p %u 0x%x 0x%p\n", rootRefIds[i], rootKinds[i], rootFlags[i], rootIds[i]);
        }
    }

    return S_OK;
}

/* public */
HRESULT ProfilerCallback::HandleCreated(
            /* [in] */ UINT_PTR handleId,
            /* [in] */ ObjectID initialObjectId)
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    HRESULT hr = E_FAIL;
    DWORD win32ThreadID = 0;
    SIZE_T stackTraceId = 0;

    ThreadInfo *pThreadInfo = GetThreadInfo();

    if ( pThreadInfo != NULL )
    {
        win32ThreadID = pThreadInfo->m_win32ThreadID;
        stackTraceId = _StackTraceId();
    }

    LogToAny( "h %d 0x%p 0x%p %Id\n", win32ThreadID, handleId, initialObjectId, stackTraceId );

    return S_OK;
}

/* public */
HRESULT ProfilerCallback::HandleDestroyed(
            /* [in] */ UINT_PTR handleId)
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    HRESULT hr = E_FAIL;
    ThreadID threadID = NULL;
    DWORD win32ThreadID = 0;
    SIZE_T stackTraceId = 0;

    ThreadInfo *pThreadInfo = GetThreadInfo();

    if ( pThreadInfo != NULL )
    {
        win32ThreadID = pThreadInfo->m_win32ThreadID;
        stackTraceId = _StackTraceId();
    }

    LogToAny( "j %d 0x%p %Id\n", win32ThreadID, handleId, stackTraceId );

    return S_OK;
}


void ProfilerCallback::_GenerationBounds(BOOL beforeGarbageCollection, BOOL inducedGc, int collectedGeneration)
{
    if ((m_dwMode & OBJECT) == 0)
        return;

    // we want the log entry on its own tick
    while (_GetTickCount() == m_lastTickCount)
        Sleep(0);
    _LogTickCount();

    ULONG maxObjectRanges = 100;
    ULONG cObjectRanges;
    while (true)
    {
        COR_PRF_GC_GENERATION_RANGE *ranges = new COR_PRF_GC_GENERATION_RANGE[maxObjectRanges];
    
        cObjectRanges = 0;
        HRESULT hr = m_pProfilerInfo2->GetGenerationBounds(maxObjectRanges,
                                                          &cObjectRanges,
                                                           ranges);
        if (hr != S_OK)
            break;

        if (cObjectRanges <= maxObjectRanges)
        {
            LogToAny("b %u %u %u", beforeGarbageCollection, inducedGc, collectedGeneration);
            for (ULONG i = 0; i < cObjectRanges; i++)
            {
                LogToAny(" 0x%p %Iu %Iu %d", ranges[i].rangeStart, ranges[i].rangeLength, ranges[i].rangeLengthReserved, ranges[i].generation);
            }
            LogToAny("\n");
        }

        delete[] ranges;

        if (cObjectRanges <= maxObjectRanges)
            break;

        maxObjectRanges *= 2;
    }
}

HRESULT ProfilerCallback::GarbageCollectionStarted(
    /* [in] */int cGenerations,
    /*[in, size_is(cGenerations), length_is(cGenerations)]*/ BOOL generationCollected[],
    /*[in]*/ COR_PRF_GC_REASON reason)
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    if (m_bAttachLoaded && m_bWaitingForTheFirstGC)
        m_bWaitingForTheFirstGC = FALSE;

    int generation = COR_PRF_GC_GEN_1;
    for ( ; generation < COR_PRF_GC_LARGE_OBJECT_HEAP; generation++)
        if (!generationCollected[generation])
            break;
    generation--;

    if (m_condemnedGenerationIndex >= ARRAY_LEN(m_condemnedGeneration))
    {
       _THROW_EXCEPTION( "Logic error!  m_condemnedGenerationIndex is out of legal range." );
    }

    m_condemnedGeneration[m_condemnedGenerationIndex++] = generation;

    _GenerationBounds(TRUE, reason == COR_PRF_GC_INDUCED, generation);

    if (((m_dwMode & OBJECT) == OBJECT) && (_GetTickCount() - m_lastTickCount >= CLOCK_TICK_INC))
        _LogTickCount();

    return S_OK;
}


/*
 * The CLR calls GarbageCollectionFinished after a garbage
 * collection has completed and all GC callbacks have been
 * issued for it.
 */
HRESULT  ProfilerCallback::GarbageCollectionFinished()
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    if (m_bAttachLoaded && m_bWaitingForTheFirstGC)
        return S_OK;

    if (m_condemnedGenerationIndex == 0)
    {
       _THROW_EXCEPTION( "Logic error!  m_condemnedGenerationIndex is out of legal range." );
    }

    int collectedGeneration = m_condemnedGeneration[--m_condemnedGenerationIndex];
    _GenerationBounds(FALSE, FALSE, collectedGeneration);

    for (int i = 0 ; i <= collectedGeneration ; i++)
        m_GCcounter[i]++;

    if ((m_dwMode & OBJECT) == OBJECT)
        LogToAny( "g %Id %Id %Id\n", m_GCcounter[COR_PRF_GC_GEN_0], m_GCcounter[COR_PRF_GC_GEN_1], m_GCcounter[COR_PRF_GC_GEN_2]);

    return S_OK;
}

HRESULT ProfilerCallback::InitializeForAttach( 
            /* [in] */ IUnknown *pICorProfilerInfoUnk,
            /* [in] */ void *pvClientData,
            /* [in] */ UINT cbClientData)
{
    HRESULT hr;
    if (cbClientData != sizeof(ProfConfig))
    {
        Failure( "ProfilerCallback::InitializeForAttach: Client data bogus!\n" );
        return E_INVALIDARG;
    }

    m_bAttachLoaded = TRUE;
    m_bWaitingForTheFirstGC = TRUE;

    ProfConfig * pProfConfig = (ProfConfig *) pvClientData;
    _ProcessProfConfig(pProfConfig);

    if ( (m_dwEventMask & (~COR_PRF_ALLOWABLE_AFTER_ATTACH)) != 0 )
    {
        Failure( "ProfilerCallback::InitializeForAttach: Unsupported event mode for attach" );
        return E_INVALIDARG;
    }
    hr = pICorProfilerInfoUnk->QueryInterface( IID_ICorProfilerInfo,
                                               (void **)&m_pProfilerInfo );   

    if ( SUCCEEDED( hr ) )
    {
        hr = pICorProfilerInfoUnk->QueryInterface( IID_ICorProfilerInfo2,
                                                   (void **)&m_pProfilerInfo2 );
    }

    if ( SUCCEEDED( hr ) )
    {
        hr = pICorProfilerInfoUnk->QueryInterface( IID_ICorProfilerInfo3,
                                                   (void **)&m_pProfilerInfo3 );
    }
    if ( SUCCEEDED( hr ) )
    {
        hr = m_pProfilerInfo->SetEventMask( m_dwEventMask );

        if ( SUCCEEDED( hr ) )
        {
            hr = Init(pProfConfig);
            if (FAILED(hr))
            {
                return hr;
            }

            // For Windows Store app, the threads/events were created in Init()
            if (!m_bWindowsStoreApp)
            {
                hr = _InitializeThreadsAndEvents();
                if ( FAILED( hr ) )
                    Failure( "Unable to initialize the threads and handles, No profiling" );
                Sleep(100); // Give the threads a chance to read any signals that are already set.
            }
        }
        else   
        {
            Failure( "SetEventMask for Profiler Test FAILED" );
        }
    }       
    else
        Failure( "Allocation for Profiler Test FAILED" );
    return hr;
}
        
HRESULT ProfilerCallback::ProfilerAttachComplete()
{
    TEXT_OUTLN( "Attach completed" ); 
    return S_OK;
}

HRESULT ProfilerCallback::ProfilerDetachSucceeded()
{
    SetEvent( m_hArrayCallbacks[DETACH_HANDLE] );
    TEXT_OUTLN( "Detach succeeded" ); 
    return S_OK;
}

/***************************************************************************************
 ********************                                               ********************
 ********************              Private Functions                ********************
 ********************                                               ********************
 ***************************************************************************************/ 

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
/* public */
void ProfilerCallback::_GetProfConfigFromEnvironment(ProfConfig * pProfConfig)
{
    char buffer[2*MAX_LENGTH];
    memset(pProfConfig, 0, sizeof(*pProfConfig));

    //
    // read the mode under which the tool is going to operate
    //
    buffer[0] = '\0';
    pProfConfig->usage = OmvUsageInvalid;
    if ( GetEnvironmentVariableA( OMV_USAGE, buffer, MAX_LENGTH ) > 0 )
    {
        if ( _stricmp( "objects", buffer ) == 0 )
        {
            pProfConfig->usage = OmvUsageObjects;
        }
        else if ( _stricmp( "trace", buffer ) == 0 )
        {
            pProfConfig->usage = OmvUsageTrace;
        }
        else if ( _stricmp( "both", buffer ) == 0 )
        {
            pProfConfig->usage = OmvUsageBoth;
        }
        else
        {
            pProfConfig->usage = OmvUsageNone;
        }
    }

    // retrieve the format
    buffer[0] = '\0';
    pProfConfig->bOldFormat = TRUE;
    if ( GetEnvironmentVariableA(OMV_FORMAT, buffer, MAX_LENGTH) > 0 )
    {
        if( _stricmp("v2", buffer) == 0 )
        {
            pProfConfig->bOldFormat = FALSE;
        }
    }

    // am I Windows Store app?
    buffer[0] = '\0';
    pProfConfig->bWindowsStoreApp = FALSE;
    if ( GetEnvironmentVariableA(OMV_WINDOWSSTOREAPP, buffer, MAX_LENGTH) > 0 )
    {
        if( _stricmp("1", buffer) == 0 )
        {
            pProfConfig->bWindowsStoreApp = TRUE;
        }
    }

    //
    // look if the user specified another path to save the output file
    //
    if ( GetEnvironmentVariableA( OMV_PATH, pProfConfig->szPath, ARRAY_LEN(pProfConfig->szPath) ) == 0 )
    {
        pProfConfig->szPath[0] = '\0';
    }

    if ( GetEnvironmentVariableA( OMV_FILENAME, pProfConfig->szFileName, ARRAY_LEN(pProfConfig->szFileName) ) == 0 )
    {
        pProfConfig->szFileName[0] = '\0';
    }

    if ( (pProfConfig->usage == OmvUsageObjects) || (pProfConfig->usage == OmvUsageBoth) )
    {
        //
        // check if the user is going to dynamically enable
        // object tracking
        //
        buffer[0] = '\0';
        if ( GetEnvironmentVariableA( OMV_DYNAMIC, buffer, MAX_LENGTH ) > 0 )
        {
            pProfConfig->bDynamic = TRUE;
        }

        //
        // check to see if the user requires stack trace
        //
        DWORD value1 = BASEHELPER::FetchEnvironment( OMV_STACK );

        if ( (value1 != 0x0) && (value1 != 0xFFFFFFFF) )
        {
            pProfConfig->bStack = TRUE;
        
            //
            // decide how many frames to print
            //
            pProfConfig->dwFramesToPrint = BASEHELPER::FetchEnvironment( OMV_FRAMENUMBER );
        }

        //
        // how many objects you wish to skip
        //
        DWORD dwTemp = BASEHELPER::FetchEnvironment( OMV_SKIP );
        pProfConfig->dwSkipObjects = ( dwTemp != 0xFFFFFFFF ) ? dwTemp : 0;


        //
        // in which class you are interested in
        //
        if ( GetEnvironmentVariableA( OMV_CLASS, pProfConfig->szClassToMonitor, ARRAY_LEN(pProfConfig->szClassToMonitor) ) == 0 )
        {
            pProfConfig->szClassToMonitor[0] = '\0';
        }

    }   

    //
    // check to see if there is an inital setting for tracking allocations and calls
    //
    pProfConfig->dwInitialSetting = BASEHELPER::FetchEnvironment( OMV_INITIAL_SETTING );
}


void ProfilerCallback::_ProcessProfConfig(ProfConfig * pProfConfig)
{
    //
    // mask for everything
    //
    m_dwEventMask =  (DWORD) COR_PRF_MONITOR_GC
                   | (DWORD) COR_PRF_MONITOR_THREADS
                   | (DWORD) COR_PRF_MONITOR_SUSPENDS
//                   | (DWORD) COR_PRF_MONITOR_ENTERLEAVE
//                   | (DWORD) COR_PRF_MONITOR_EXCEPTIONS
                   // | (DWORD) COR_PRF_MONITOR_CLASS_LOADS
                   | (DWORD) COR_PRF_MONITOR_MODULE_LOADS
                   | (DWORD) COR_PRF_MONITOR_ASSEMBLY_LOADS
                   | (DWORD) COR_PRF_MONITOR_CACHE_SEARCHES
                   | (DWORD) COR_PRF_ENABLE_OBJECT_ALLOCATED 
                   | (DWORD) COR_PRF_MONITOR_JIT_COMPILATION
                   | (DWORD) COR_PRF_MONITOR_OBJECT_ALLOCATED
//                   | (DWORD) COR_PRF_MONITOR_CODE_TRANSITIONS
                    ;

    //
    // read the mode under which the tool is going to operate
    //
    if ( pProfConfig->usage == OmvUsageObjects )
    {
        m_bTrackingObjects = TRUE;
        m_dwMode = (DWORD)OBJECT;   
    }
    else if ( pProfConfig->usage == OmvUsageTrace )
    {
        //
        // mask for call graph, remove GC and OBJECT ALLOCATIONS
        //
        m_dwEventMask = m_dwEventMask ^(DWORD) ( COR_PRF_MONITOR_GC 
                                               | COR_PRF_MONITOR_OBJECT_ALLOCATED
                                               | COR_PRF_ENABLE_OBJECT_ALLOCATED );
        m_bTrackingCalls = TRUE;
        m_dwMode = (DWORD)TRACE;

    }
    else if ( pProfConfig->usage == OmvUsageBoth )
    {
        m_bTrackingObjects = TRUE;
        m_bTrackingCalls = TRUE;
        m_dwMode = (DWORD)BOTH; 
    }
    else
    {
        m_dwEventMask =  (DWORD) COR_PRF_MONITOR_GC
                       | (DWORD) COR_PRF_MONITOR_THREADS
                       | (DWORD) COR_PRF_MONITOR_SUSPENDS;
        m_dwMode = 0;
    }

    // retrieve the format
    m_oldFormat = pProfConfig->bOldFormat;
   
    if ( m_dwMode & (DWORD)TRACE)
        m_bIsTrackingStackTrace = TRUE;

    //
    // look further for env settings if operating under OBJECT mode
    //
    if ( m_dwMode & (DWORD)OBJECT )
    {
        //
        // check if the user is going to dynamically enable
        // object tracking
        //
        if ( pProfConfig->bDynamic )
        {
            //
            // do not track object when you start up, activate the thread that
            // is going to listen for the event
            //
            m_dwEventMask = m_dwEventMask ^ (DWORD) COR_PRF_MONITOR_OBJECT_ALLOCATED;
            m_bTrackingObjects = FALSE;
            m_dwMode = m_dwMode | (DWORD)DYNOBJECT;
        }
        

        //
        // check to see if the user requires stack trace
        //
        if ( pProfConfig->bStack )
        {
            m_bIsTrackingStackTrace = TRUE;
            m_dwEventMask = m_dwEventMask
                            | (DWORD) COR_PRF_MONITOR_ENTERLEAVE
                            | (DWORD) COR_PRF_MONITOR_EXCEPTIONS
                            | (DWORD) COR_PRF_MONITOR_CODE_TRANSITIONS;
        
            //
            // decide how many frames to print
            //
            m_dwFramesToPrint = pProfConfig->dwFramesToPrint;

        }

        //
        // how many objects you wish to skip
        //
        m_dwSkipObjects = pProfConfig->dwSkipObjects;


        //
        // in which class you are interested in
        // if the env variable does not exist copy to it the null
        // string otherwise copy its value
        //
        if ( pProfConfig->szClassToMonitor[0] != '\0')
        {
            const size_t len = strlen(pProfConfig->szClassToMonitor) + 1;
            m_classToMonitor = new WCHAR[len];
            if ( m_classToMonitor != NULL )
            {
                MultiByteToWideChar(CP_ACP, 0, pProfConfig->szClassToMonitor, (int)len, m_classToMonitor, (int)len);
            }
            else
            {
                //
                // some error has happened, do not monitor anything
                //
                printf( "Memory Allocation Error in ProfilerCallback .ctor\n" );
                printf( "**** No Profiling Will Take place **** \n" );
                m_dwEventMask = (DWORD) COR_PRF_MONITOR_NONE;           
            }
        }
    }   

    //
    // check to see if there is an inital setting for tracking allocations and calls
    //
    if ( pProfConfig->dwInitialSetting != 0xFFFFFFFF )
    {
        if (pProfConfig->dwInitialSetting & 1)
        {
            // Turning object stuff on
            m_dwEventMask = m_dwEventMask | (DWORD) COR_PRF_MONITOR_OBJECT_ALLOCATED;
            m_bTrackingObjects = TRUE;
        }
        else
        {
            // Turning object stuff of
            m_dwEventMask = m_dwEventMask & ~(DWORD) COR_PRF_MONITOR_OBJECT_ALLOCATED;
            m_bTrackingObjects = FALSE;
        }

        if (pProfConfig->dwInitialSetting & 2)
        {
            m_dwMode = m_dwMode | (DWORD)TRACE;
            m_bTrackingCalls = TRUE;
        }
        else
        {
            m_dwMode = m_dwMode & ~(DWORD)TRACE;
            m_bTrackingCalls = FALSE;
        }
    }

    char buffer[MAX_LENGTH];

    if ( ( GetEnvironmentVariableA( OMV_TARGET_CLR_VERSION, buffer, MAX_LENGTH ) > 0 ) && 
         ( _stricmp( "v2", buffer ) == 0 ) )
    {
        m_bTargetV2CLR = TRUE;
        // skip detach event when we target v2 CLR
        m_dwSentinelHandle = SENTINEL_HANDLE - 1;  
    }
    else
    {
        m_bTargetV2CLR= FALSE;
    }

    m_bWindowsStoreApp = pProfConfig->bWindowsStoreApp;

//  printf("m_bTrackingObjects = %d  m_bTrackingCalls = %d\n", m_bTrackingObjects, m_bTrackingCalls);

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
/* public */
HRESULT ProfilerCallback::_InitializeThreadsAndEvents()
{
    HRESULT hr = S_OK;


    //
    // GC and Dynamic Object triggering 
    //  Step 1. set up the IPC event
    //  Step 2. set up the IPC callback event
    //  Step 3. set up the thread
    //
    for ( DWORD i=GC_HANDLE; i<m_dwSentinelHandle; i++ )
    {

        // Step 1
        m_hArray[i] = OpenEventA( EVENT_ALL_ACCESS,  // access
            FALSE,             // do not inherit
            m_NamedEvents[i] ); // event name
        if ( m_hArray[i] == NULL )
        {
            if (!m_bAttachLoaded && (i != TRIGGER_GC_HANDLE))
                TEXT_OUTLN( "WARNING: OpenEvent() FAILED Will Attempt CreateEvent()" );

            m_hArray[i] = CreateEventExA( 
                NULL,                           // Not inherited
                m_NamedEvents[i],               // event name
                CREATE_EVENT_MANUAL_RESET,      // explicit ResetEvent() required; leave initial state unsignaled
                EVENT_ALL_ACCESS);
            if ( m_hArray[i] == NULL )
            {
                TEXT_OUTLN( "CreateEvent() Attempt FAILED" )
                    hr = E_FAIL;
                break;
            }
        }

        // Step 2
        m_hArrayCallbacks[i] = OpenEventA( EVENT_ALL_ACCESS,    // access
                                           FALSE,               // do not inherit
                                           m_CallbackNamedEvents[i] ); // event name
        if ( m_hArrayCallbacks[i] == NULL )
        {
            if (!m_bAttachLoaded && (i != TRIGGER_GC_HANDLE))
                TEXT_OUTLN( "WARNING: OpenEvent() FAILED Will Attempt CreateEvent()" )

            m_hArrayCallbacks[i] = CreateEventExA( 
				NULL,                           // Not inherited
				m_CallbackNamedEvents[i],       // event name
				CREATE_EVENT_MANUAL_RESET,      // explicit ResetEvent() required; leave initial state unsignaled
				EVENT_ALL_ACCESS);
            if ( m_hArrayCallbacks[i] == NULL )
            {
                TEXT_OUTLN( "CreateEvent() Attempt FAILED" )
                hr = E_FAIL;
                break;
            }
        }
    }   

    // Step 3
    m_hThread = ::CreateThread( NULL,                                       // security descriptor, NULL is not inherited 
                                0,                                          // stack size   
                                (LPTHREAD_START_ROUTINE) ThreadStub,        // start function pointer 
                                (void *) this,                              // parameters for the function
                                THREAD_PRIORITY_NORMAL,                     // priority 
                                &m_dwWin32ThreadID );                       // Win32threadID
    if ( m_hThread == NULL )
    {
        hr = E_FAIL;
        TEXT_OUTLN( "ERROR: CreateThread() FAILED" )
    }

    m_bInitialized = TRUE;

    return hr;

} // ProfilerCallback::_InitializeThreadsAndEvents


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
/* public */
HRESULT ProfilerCallback::_InitializeNamesForEventsAndCallbacks()
{
    HRESULT hr = S_OK;
    char * szEventPrefix = m_bWindowsStoreApp ? "" : GlobalPrefixForNamedEvents;
    SIZE_T cchEventPrefix = strlen(szEventPrefix);

    for ( DWORD i=GC_HANDLE; ( (i<m_dwSentinelHandle) && SUCCEEDED(hr) ); i++ )
    {
        //
        // initialize
        //
        m_NamedEvents[i] = NULL;
        m_CallbackNamedEvents[i] = NULL;


        //
        // allocate space
        //

        SIZE_T namedEventLen = cchEventPrefix + strlen(NamedEvents[i]) + 1 + 9;
        SIZE_T callbackNamedEventLen = cchEventPrefix + strlen(CallbackNamedEvents[i])+1+9;
        m_NamedEvents[i] = new char[namedEventLen];
        m_CallbackNamedEvents[i] = new char[callbackNamedEventLen];

        if ( (m_NamedEvents[i] != NULL) && (m_CallbackNamedEvents[i] != NULL) )
        {
            sprintf_s( m_NamedEvents[i], namedEventLen, "%s%s_%08x", szEventPrefix, NamedEvents[i], m_dwProcessId );
            sprintf_s( m_CallbackNamedEvents[i], callbackNamedEventLen, "%s%s_%08x", szEventPrefix, CallbackNamedEvents[i], m_dwProcessId );
        }
        else
            hr = E_FAIL;
    }

    //
    //  report the allocation error
    //
    if ( FAILED( hr ) )
        Failure( "ERROR: Allocation Failure" );


    return hr;

} // ProfilerCallback::_InitializeNamesForEventsAndCallbacks


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
/* public */
void ProfilerCallback::_ShutdownAllThreads()
{
    if (m_bShutdown)
    {
        return;
    }

    //
    // mark that we are shutting down
    //
    m_bShutdown = TRUE;

    //
    // look for the named events and reset them if they are set
    // notify the GUI and signal to the threads to shutdown
    //
    for ( DWORD i=GC_HANDLE; i<m_dwSentinelHandle; i++ )
    {
        SetEvent( m_hArray[i] );        
    }
    
    //
    // wait until you receive the autoreset event from the threads
    // that they have shutdown successfully
    //
    DWORD waitResult = WaitForSingleObjectEx(
        m_hThread,  // thread handle
        INFINITE,   // wait forever
        FALSE);	    // alertable

    if ( waitResult == WAIT_FAILED )
        LogToAny( "Error While Shutting Down Helper Threads: 0x%08x\n", GetLastError() );       

    //
    // loop through and close all the handles, we are done !
    //
    _CloseEventHandles();

    if ( CloseHandle( m_hThread ) == FALSE )
        LogToAny( "Error While Executing CloseHandle: 0x%08x\n", GetLastError() );      
    m_hThread = NULL;

} // ProfilerCallback::_ShutdownAllThreads
  
void ProfilerCallback::_CloseEventHandles()
{
    for ( DWORD i=GC_HANDLE; i<m_dwSentinelHandle; i++ )
    {
        if (m_hArray[i] != NULL)
        {
            if ( CloseHandle( m_hArray[i] ) == FALSE )
                LogToAny( "Error While Executing CloseHandle: 0x%08x\n", GetLastError() );      
            m_hArray[i] = NULL;
        }

        if (m_hArrayCallbacks[i] != NULL)
        {
            if ( CloseHandle( m_hArrayCallbacks[i] ) == FALSE )
                LogToAny( "Error While Executing CloseHandle: 0x%08x\n", GetLastError() );      
            m_hArrayCallbacks[i] = NULL;
        }
    }
}

bool ProfilerCallback::_ClassHasFinalizeMethod(IMetaDataImport *pMetaDataImport, mdToken classToken, DWORD *pdwAttr)
{
    HRESULT hr = S_OK;
//                      printf("got module metadata\n");
    HCORENUM hEnum = 0;
    mdMethodDef methodDefToken[100];
    ULONG methodDefTokenCount = 0;
    hr = pMetaDataImport->EnumMethodsWithName(&hEnum, classToken, L"Finalize", methodDefToken, 100, &methodDefTokenCount);
    pMetaDataImport->CloseEnum(hEnum);
    if (SUCCEEDED(hr))
    {
//                              if (methodDefTokenCount > 0)
//                                  printf("found %d finalize methods on %S\n", methodDefTokenCount, (*ppClassInfo)->m_className);
        for (ULONG i = 0; i < methodDefTokenCount; i++)
        {
            mdTypeDef classTypeDef;
            WCHAR   szMethod[MAX_CLASS_NAME];
            ULONG   cchMethod;
            PCCOR_SIGNATURE pvSigBlob;
            ULONG   cbSigBlob;
            ULONG   ulCodeRVA;
            DWORD   dwImplFlags;
            hr = pMetaDataImport->GetMethodProps(methodDefToken[i], &classTypeDef, szMethod, MAX_CLASS_NAME, &cchMethod, pdwAttr,
                                                &pvSigBlob, &cbSigBlob, &ulCodeRVA, &dwImplFlags);

            if (SUCCEEDED(hr) && !IsMdStatic(*pdwAttr) && IsMdVirtual(*pdwAttr))
            {
                hEnum = 0;
                mdParamDef params[100];
                ULONG paramCount = 0;
                hr = pMetaDataImport->EnumParams(&hEnum, methodDefToken[i], params, 100, &paramCount);
                pMetaDataImport->CloseEnum(hEnum);
                if (SUCCEEDED(hr))
                {
                    if (paramCount == 0)
                    {
//                          printf("finalize method #%d on %S has attr = %x  impl flags = %x\n", i, (*ppClassInfo)->m_className, dwAttr, dwImplFlags);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool ProfilerCallback::_ClassIsFinalizable(ModuleID moduleID, mdToken classToken)
{
    IMetaDataImport *pMetaDataImport = NULL;
    HRESULT hr = S_OK;
    hr = m_pProfilerInfo->GetModuleMetaData(moduleID, 0, IID_IMetaDataImport, (IUnknown **)&pMetaDataImport);
    if (SUCCEEDED(hr))
    {
        bool result = false;
        while (true)
        {
            WCHAR   szTypeDef[MAX_CLASS_NAME];
            ULONG   chTypeDef = 0;
            DWORD   dwTypeDefFlags = 0;
            mdToken baseToken = mdTokenNil;
            hr = pMetaDataImport->GetTypeDefProps(classToken, szTypeDef, MAX_CLASS_NAME, &chTypeDef, &dwTypeDefFlags, &baseToken);
            if (hr == S_OK)
            {
                if (IsNilToken(baseToken))
                {
//                  printf("  Class %S has no base class - base token = %u\n", szTypeDef, baseToken);
                    return result;
                }
                if (_ClassOverridesFinalize(pMetaDataImport, classToken))
                {
//                  printf("  Class %S overrides Finalize\n", szTypeDef);
                    result = true;
                }
                else if (_ClassReintroducesFinalize(pMetaDataImport, classToken))
                {
//                  printf("  Class %S reintroduces Finalize\n", szTypeDef);
                    result = false;
                }
            }
            else
            {
//              printf("  _ClassIsFinalizable got an error\n");
                return result;
            }
            
            if (TypeFromToken(baseToken) == mdtTypeRef)
            {
                WCHAR szName[MAX_CLASS_NAME];
                ULONG chName = 0;
                mdToken resolutionScope = mdTokenNil;
                hr = pMetaDataImport->GetTypeRefProps(baseToken, &resolutionScope, szName, MAX_CLASS_NAME, &chName);
                if (hr == S_OK)
                {
//                  printf("trying to resolve %S\n", szName);
                    IMetaDataImport *pMetaDataImportRef = NULL;
                    hr = pMetaDataImport->ResolveTypeRef(baseToken, IID_IMetaDataImport, (IUnknown **)&pMetaDataImportRef, &baseToken);
                    if (hr == S_OK)
                    {
                        pMetaDataImport->Release();
                        pMetaDataImport = pMetaDataImportRef;
                        classToken = baseToken;
//                      printf("successfully resolved class %S\n", szName);
                    }
                    else
                    {
                        printf("got error trying to resolve %S\n", szName);
                        return result;
                    }
                }
            }
            else
                classToken = baseToken;
        }
        pMetaDataImport->Release();
    }
    else
    {
        printf("  _ClassIsFinalizable got an error\n");
        return false;
    }
}

bool ProfilerCallback::_ClassOverridesFinalize(IMetaDataImport *pMetaDataImport, mdToken classToken)
{
    DWORD dwAttr = 0;
    return _ClassHasFinalizeMethod(pMetaDataImport, classToken, &dwAttr) && IsMdReuseSlot(dwAttr);
}

bool ProfilerCallback::_ClassReintroducesFinalize(IMetaDataImport *pMetaDataImport, mdToken classToken)
{
    DWORD dwAttr = 0;
    return _ClassHasFinalizeMethod(pMetaDataImport, classToken, &dwAttr) && IsMdNewSlot(dwAttr);
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
/* public */
HRESULT ProfilerCallback::_InsertGCClass( ClassInfo **ppClassInfo, ClassID classID )
{
    HRESULT hr = S_OK;

    *ppClassInfo = m_pClassTable->Lookup( classID );
    if ( *ppClassInfo == NULL )
    {
        *ppClassInfo = new ClassInfo( classID );
        if ( *ppClassInfo != NULL )
        {
            //
            // we have 2 cases
            // case 1: class is an array
            // case 2: class is a real class
            //
            ULONG rank = 0;
            CorElementType elementType;
            ClassID realClassID = NULL;
            WCHAR ranks[MAX_LENGTH];
            bool finalizable = false;


            // case 1 
            hr = m_pProfilerInfo->IsArrayClass( classID, &elementType, &realClassID, &rank );
            if ( hr == S_OK )
            {
                ClassID prevClassID;


                _ASSERTE( realClassID != NULL );
                ranks[0] = '\0';
                do
                {
                    prevClassID = realClassID;
                    _snwprintf_s( ranks, ARRAY_LEN(ranks), ARRAY_LEN(ranks)-1, L"%s[]", ranks);
                    hr = m_pProfilerInfo->IsArrayClass( prevClassID, &elementType, &realClassID, &rank );
                    if ( (hr == S_FALSE) || (FAILED(hr)) || (realClassID == NULL) )
                    {
                        //
                        // before you break set the realClassID to the value that it was before the 
                        // last unsuccessful call
                        //
                        realClassID = prevClassID;
                        
                        break;
                    }
                }
                while ( TRUE );
                
                if ( SUCCEEDED( hr ) )
                {
                    WCHAR className[10 * MAX_LENGTH];
                    
                    
                    className[0] = '\0';
                    if ( realClassID != NULL )
                        hr = GetNameFromClassID( realClassID, className );
                    else
                        hr = _GetNameFromElementType( elementType, className, ARRAY_LEN(className) );
                    
                    
                    if ( SUCCEEDED( hr ) )
                    {
                        const size_t len = ARRAY_LEN((*ppClassInfo)->m_className);
                        _snwprintf_s( (*ppClassInfo)->m_className, len, len-1, L"%s %s",className, ranks  );
                        (*ppClassInfo)->m_objectsAllocated++;
                        (*ppClassInfo)->m_internalID = m_totalClasses;
                        m_pClassTable->AddEntry( *ppClassInfo, classID );
                        LogToAny( "t %Id %d %S\n",(*ppClassInfo)->m_internalID,
                                                  finalizable,
                                                  SanitizeUnicodeName((*ppClassInfo)->m_className, ARRAY_LEN((*ppClassInfo)->m_className), L'\0'));
                    }
                    else
                        Failure( "ERROR: PrfHelper::GetNameFromClassID() FAILED" );
                }
                else
                    Failure( "ERROR: Looping for Locating the ClassID FAILED" );
            }
            // case 2
            else if ( hr == S_FALSE )
            {
                hr = GetNameFromClassID( classID, (*ppClassInfo)->m_className );
                if ( SUCCEEDED( hr ) )
                {
                    (*ppClassInfo)->m_objectsAllocated++;
                    (*ppClassInfo)->m_internalID = m_totalClasses;
                    m_pClassTable->AddEntry( *ppClassInfo, classID );

                    ModuleID moduleID = 0;
                    mdTypeDef typeDefToken = 0;

                    hr = m_pProfilerInfo->GetClassIDInfo(classID, &moduleID, &typeDefToken);
                    if (SUCCEEDED(hr))
                    {
//                      printf("Class %x has module %x and type def token %x\n", classID, moduleID, typeDefToken);
//                      printf("Checking class %S for finalizability\n", (*ppClassInfo)->m_className);
                        finalizable = _ClassIsFinalizable(moduleID, typeDefToken);
//                      if (finalizable)
//                          printf("Class %S is finalizable\n", (*ppClassInfo)->m_className);
//                      else
//                          printf("Class %S is not finalizable\n", (*ppClassInfo)->m_className);
                    }

                    LogToAny( "t %Id %d %S\n",(*ppClassInfo)->m_internalID,
                                              finalizable,
                                              SanitizeUnicodeName((*ppClassInfo)->m_className, ARRAY_LEN((*ppClassInfo)->m_className), L'\0'));
                }               
                else
                    Failure( "ERROR: PrfHelper::GetNameFromClassID() FAILED" );
            }
            else
                Failure( "ERROR: ICorProfilerInfo::IsArrayClass() FAILED" );
        }
        else
            Failure( "ERROR: Allocation for ClassInfo FAILED" );    

        InterlockedIncrement( &m_totalClasses );
    }
    else
        (*ppClassInfo)->m_objectsAllocated++;
        
    
    return hr;

} // ProfilerCallback::_InsertGCClass


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
/* private */
HRESULT ProfilerCallback::_AddGCObject( BOOL bForce )
{
    HRESULT hr = E_FAIL;
    ThreadID threadID = NULL;

    //
    // if you are not monitoring stack trace, do not even bother
    //
    if ( (m_bIsTrackingStackTrace == FALSE) && (bForce == FALSE) )
        return S_OK;

    ThreadInfo *pThreadInfo = GetThreadInfo();

    if ( pThreadInfo != NULL )
    {
        SIZE_T count = 0;
        
        
        count = (pThreadInfo->m_pThreadCallStack)->Count();
        if  ( count != 0 )
        {
            //
            // dump the stack when the object was allocated
            //
            SIZE_T threshold = count;


            //
            // calculate the theshold above which you log the stack trace
            //
            if ( m_dwFramesToPrint == 0xFFFFFFFF )
                threshold = 0;
            
            else if ( count<m_dwFramesToPrint )
                threshold = 0;
            
            else
                threshold = count - m_dwFramesToPrint;

            for ( SIZE_T frame = threshold; frame < count; frame++ )
            {
                SIZE_T stackElement = 0;
                FunctionInfo *pFunctionInfo = NULL;
                
                
                stackElement = (pThreadInfo->m_pThreadCallStack)->m_Array[frame];
                pFunctionInfo = m_pFunctionTable->Lookup( stackElement );
                if ( pFunctionInfo != NULL )
                    LogToAny( "%Id ", pFunctionInfo->m_internalID );
                else
                    Failure( "ERROR: Function Not Found In Function Table" );

            } // end while loop
        }
        else
        {
            LogToAny( "-1 "); /*empty stack is marked as -1*/   
        }
    }
    else                
        Failure( "ERROR: Thread Structure was not found in the thread list" );

    return hr;

} // ProfilerCallback::_AddGCObject


SIZE_T ProfilerCallback::_StackTraceId(SIZE_T typeId, SIZE_T typeSize)
{
    ThreadID threadID = NULL;

    ThreadInfo *pThreadInfo = GetThreadInfo();
    if ( pThreadInfo != NULL )
    {
        DWORD count = pThreadInfo->m_pThreadCallStack->Count();
        StackTrace stackTrace(count, pThreadInfo->m_pThreadCallStack->m_Array, typeId, typeSize);
        StackTraceInfo *latestStackTraceInfo = pThreadInfo->m_pLatestStackTraceInfo;
        if(latestStackTraceInfo != NULL && latestStackTraceInfo->Compare(stackTrace) == TRUE)
        {
            return latestStackTraceInfo->m_internalId;
        }

        StackTraceInfo *stackTraceInfo = m_pStackTraceTable->Lookup(stackTrace);
        if (stackTraceInfo != NULL)
        {
            pThreadInfo->m_pLatestStackTraceInfo = stackTraceInfo;
            return stackTraceInfo->m_internalId;
        }

        stackTraceInfo = new StackTraceInfo(++m_callStackCount, count, pThreadInfo->m_pThreadCallStack->m_Array, typeId, typeSize);
        if (stackTraceInfo == NULL)
            _THROW_EXCEPTION( "Allocation for StackTraceInfo FAILED" );

        pThreadInfo->m_pLatestStackTraceInfo = stackTraceInfo;
        m_pStackTraceTable->AddEntry(stackTraceInfo, stackTrace);

        ClassInfo *pClassInfo = NULL;
        if (typeId != 0 && typeSize != 0)
        {
            HRESULT hr = _InsertGCClass( &pClassInfo, typeId );
            if ( !SUCCEEDED( hr ) )
                Failure( "ERROR: _InsertGCClass() FAILED" );
        }

        // used to be `s` before the change of format
        LogToAny("n %d", m_callStackCount);

        int flag = 0;
        if (typeId != 0 && typeSize != 0)
        {
            flag |= 1;
        }

        int i, match = 0;
        if (latestStackTraceInfo != NULL)
        {
            match = min(latestStackTraceInfo->m_count, count);
            for(i = 0; i < match; i++)
            {
                if(latestStackTraceInfo->m_stack[i] != (pThreadInfo->m_pThreadCallStack)->m_Array[i])
                {
                    break;
                }
            }

            flag |= (4 * i);
            flag |= (latestStackTraceInfo->m_typeId != 0 && latestStackTraceInfo->m_typeSize != 0) ? 2 : 0;

            match = i;
        }
        /* */

        LogToAny(" %d", flag);

        if (typeId != 0 && typeSize != 0)
        {
            LogToAny(" %Id %d", pClassInfo->m_internalID, typeSize);
        }

        if (flag >= 4)
        {
            LogToAny(" %Id", latestStackTraceInfo->m_internalId);
        }

        for (DWORD frame = match; frame < count; frame++ )
        {               
            SIZE_T stackElement = (pThreadInfo->m_pThreadCallStack)->m_Array[frame];
            FunctionInfo *pFunctionInfo = m_pFunctionTable->Lookup( stackElement );
            if ( pFunctionInfo != NULL )
                LogToAny( " %Id", pFunctionInfo->m_internalID );
            else
                Failure( "ERROR: Function Not Found In Function Table" );
        } // end for loop
        LogToAny("\n");

        return stackTraceInfo->m_internalId;
    }
    else                
        Failure( "ERROR: Thread Structure was not found in the thread list" );

    return 0;

} // ProfilerCallback::_StackTraceId


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
/* private */
HRESULT ProfilerCallback::_LogCallTrace( FunctionID functionID )
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    HRESULT hr = E_FAIL;
    ThreadID threadID = NULL;


    ThreadInfo *pThreadInfo = GetThreadInfo();

    if ( pThreadInfo != NULL )
    {
        SIZE_T stackTraceId = _StackTraceId();
#if 1
        char buffer[128];
        char *p = buffer;
        *p++ = 'c';
        p = putdec(p, pThreadInfo->m_win32ThreadID);
        p = putdec(p, stackTraceId);
        *p++ = '\n';
        fwrite(buffer, p - buffer, 1, m_stream);
#else
        LogToAny( "c %d %Id\n", pThreadInfo->m_win32ThreadID, stackTraceId );
#endif
    }
    else                
        Failure( "ERROR: Thread Structure was not found in the thread list" );

    return hr;

} // ProfilerCallback::_LogCallTrace


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
/* private */
HRESULT ProfilerCallback::_GetNameFromElementType( CorElementType elementType, __out_ecount(buflen) WCHAR *buffer, size_t buflen )
{
    HRESULT hr = S_OK;

    switch ( elementType )
    {
        case ELEMENT_TYPE_BOOLEAN:
             wcscpy_s( buffer, buflen, L"System.Boolean" );
             break;

        case ELEMENT_TYPE_CHAR:
             wcscpy_s( buffer, buflen, L"System.Char" );
             break;

        case ELEMENT_TYPE_I1:
             wcscpy_s( buffer, buflen, L"System.SByte" );
             break;

        case ELEMENT_TYPE_U1:
             wcscpy_s( buffer, buflen, L"System.Byte" );
             break;

        case ELEMENT_TYPE_I2:
             wcscpy_s( buffer, buflen, L"System.Int16" );
             break;

        case ELEMENT_TYPE_U2:
             wcscpy_s( buffer, buflen, L"System.UInt16" );
             break;

        case ELEMENT_TYPE_I4:
             wcscpy_s( buffer, buflen, L"System.Int32" );
             break;

        case ELEMENT_TYPE_U4:
             wcscpy_s( buffer, buflen, L"System.UInt32" );
             break;

        case ELEMENT_TYPE_I8:
             wcscpy_s( buffer, buflen, L"System.Int64" );
             break;

        case ELEMENT_TYPE_U8:
             wcscpy_s( buffer, buflen, L"System.UInt64" );
             break;

        case ELEMENT_TYPE_R4:
             wcscpy_s( buffer, buflen, L"System.Single" );
             break;

        case ELEMENT_TYPE_R8:
             wcscpy_s( buffer, buflen, L"System.Double" );
             break;

        case ELEMENT_TYPE_STRING:
             wcscpy_s( buffer, buflen, L"System.String" );
             break;

        case ELEMENT_TYPE_PTR:
             wcscpy_s( buffer, buflen, L"System.IntPtr" );
             break;

        case ELEMENT_TYPE_VALUETYPE:
             wcscpy_s( buffer, buflen, L"struct" );
             break;

        case ELEMENT_TYPE_CLASS:
             wcscpy_s( buffer, buflen, L"class" );
             break;

        case ELEMENT_TYPE_ARRAY:
             wcscpy_s( buffer, buflen, L"System.Array" );
             break;

        case ELEMENT_TYPE_I:
             wcscpy_s( buffer, buflen, L"int_ptr" );
             break;

        case ELEMENT_TYPE_U:
             wcscpy_s( buffer, buflen, L"unsigned int_ptr" );
             break;

        case ELEMENT_TYPE_OBJECT:
             wcscpy_s( buffer, buflen, L"System.Object" );
             break;

        case ELEMENT_TYPE_SZARRAY:
             wcscpy_s( buffer, buflen, L"System.Array" );
             break;

        case ELEMENT_TYPE_MAX:
        case ELEMENT_TYPE_END:
        case ELEMENT_TYPE_VOID:
        case ELEMENT_TYPE_FNPTR:
        case ELEMENT_TYPE_BYREF:
        case ELEMENT_TYPE_PINNED:
        case ELEMENT_TYPE_SENTINEL:
        case ELEMENT_TYPE_CMOD_OPT:
        case ELEMENT_TYPE_MODIFIER:
        case ELEMENT_TYPE_CMOD_REQD:
        case ELEMENT_TYPE_TYPEDBYREF:
        default:
             wcscpy_s( buffer, buflen, L"<UNKNOWN>" );
             break;
    }

    return hr;
} // ProfilerCallback::_GetNameFromElementType


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
/* public */
void ProfilerCallback::LogToAny( const char *format, ... )
{
    ///////////////////////////////////////////////////////////////////////////
//  Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    va_list args;
    va_start( args, format );        
    vfprintf( m_stream, format, args );

} // ProfilerCallback::LogToAny


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
/* public */
void ProfilerCallback::_ThreadStubWrapper()
{
    //
    // loop and listen for a ForceGC event
    //
    while( TRUE )
    {
        DWORD dwResult;
        
        
        //
        // wait until someone signals an event from the GUI or the profiler
        //

        // m_dwSentinelHandle will either be the count of the m_hArray (i.e., SENTINEL_HANDLE),
        // or less if there are events we don't need to wait on (such as the detach event when
        // profiling CLR V2).
        _ASSERTE((0 <= m_dwSentinelHandle) && (m_dwSentinelHandle <= SENTINEL_HANDLE));
        dwResult = WaitForMultipleObjectsEx(m_dwSentinelHandle, m_hArray, FALSE /* WaitAll */, INFINITE, FALSE /* alertable */ );
        if ( dwResult >= WAIT_OBJECT_0 && dwResult < WAIT_OBJECT_0 + m_dwSentinelHandle)
        {
            ///////////////////////////////////////////////////////////////////////////
            Synchronize guard( g_criticalSection );
            ///////////////////////////////////////////////////////////////////////////  

            //
            // reset the event
            //
            ObjHandles type = (ObjHandles)(dwResult - WAIT_OBJECT_0);

            ResetEvent( m_hArray[type] );

            //
            // FALSE: indicates a ForceGC event arriving from the GUI
            // TRUE: indicates that the thread has to terminate
            // in both cases you need to send to the GUI an event to let it know
            // what the deal is
            //
            if ( m_bShutdown == FALSE )
            {
                //
                // what type do you have ?
                //
                switch( type )
                {
                    case GC_HANDLE:
                        //
                        // force the GC and do not worry about the result
                        //
                        if ( m_pProfilerInfo != NULL )
                        {
                            // dump the GC info on the next GC
                            m_bDumpGCInfo = TRUE;
                            m_pProfilerInfo->ForceGC();
                        }
                        break;

                    case TRIGGER_GC_HANDLE:
                        //
                        // force the GC and do not worry about the result
                        //
                        if ( m_pProfilerInfo != NULL )
                        {
                            m_pProfilerInfo->ForceGC();
                        }
                        break;

                    case OBJ_HANDLE:
                        //
                        // you need to update the set event mask, given the previous state
                        //
                        if (m_bAttachLoaded)
                        {
                            TEXT_OUTLN( "Object allocation callbacks are not supported for attach-loaded profilers" ); 
                        }

                        if ( (m_pProfilerInfo != NULL) && !m_bAttachLoaded )
                        {
                            if ( m_bTrackingObjects == FALSE )
                            {
                                // Turning object stuff on
                                m_dwEventMask = m_dwEventMask | (DWORD) COR_PRF_MONITOR_OBJECT_ALLOCATED;
                            }
                            else
                            {
                                // Turning object stuff off
                                m_dwEventMask = m_dwEventMask & ~(DWORD) COR_PRF_MONITOR_OBJECT_ALLOCATED;
                            }
                            
                            //
                            // revert the bool flag and set the bit
                            //
                            m_bTrackingObjects = !m_bTrackingObjects;
                            m_pProfilerInfo->SetEventMask( m_dwEventMask );
//                          printf("m_bTrackingObjects = %d  m_bTrackingCalls = %d\n", m_bTrackingObjects, m_bTrackingCalls);

                            // flush the log file
                            fflush(m_stream);

                        }                       
                        break;
                    
                    case CALL_HANDLE:
                        {
                            // turn off or on the logging by reversing the previous option
                            if (m_bTrackingCalls)
                            {
                                // turn off
                                m_dwMode = m_dwMode & ~(DWORD)TRACE;
                            }
                            else
                            {
                                // turn on
                                m_dwMode = m_dwMode | (DWORD)TRACE;
                            } 
                            m_bTrackingCalls = !m_bTrackingCalls;
//                          printf("m_bTrackingObjects = %d  m_bTrackingCalls = %d\n", m_bTrackingObjects, m_bTrackingCalls);

                            // flush the log file
                            fflush(m_stream);
                        }
                        break;

                    case DETACH_HANDLE:
                        {
                            // Create dedicated thread to make the detach request
                            DWORD dwWin32ThreadID;
                            HANDLE hDetachThread;
                            hDetachThread = ::CreateThread( NULL,                                       // security descriptor, NULL is not inherited 
                                0,                                          // stack size   
                                (LPTHREAD_START_ROUTINE) ::DetachThreadStub,        // start function pointer 
                                (void *) this,                              // parameters for the function
                                THREAD_PRIORITY_NORMAL,                     // priority 
                                &dwWin32ThreadID );                       // Win32threadID
                            if ( hDetachThread == NULL )
                            {
                                TEXT_OUTLN( "ERROR: CreateThread() for detach request FAILED" )
                            }
                            else
                            {
                                ::CloseHandle(hDetachThread);
                            }
                        }
                        return;


                    default:
                        _ASSERTE( !"Valid Option" );
                }

                // notify the GUI, if the request was for GC notify later
                if ( (type != GC_HANDLE) && (type != DETACH_HANDLE) )
                    SetEvent( m_hArrayCallbacks[type] );
            }
            else
            {
                //
                // Terminate
                //
                
                // notify the GUI
                SetEvent( m_hArrayCallbacks[type] );
                break;
            }

        }
        else
        {
            Failure( " WaitForSingleObjectEx TimedOut " );
            break;
        } 
    }

} // ProfilerCallback::_ThreadStubWrapper


void ProfilerCallback::DetachThreadStub()
{
    HMODULE hModule = LoadLibrary(L"ProfilerOBJ.dll");
    _ASSERTE(hModule != NULL);
    _ASSERTE(!m_bTargetV2CLR);
    _ASSERTE(m_pProfilerInfo3 != NULL);
    _CloseEventHandles();
    m_pProfilerInfo3->RequestProfilerDetach(5000);
    FreeLibraryAndExitThread(hModule, 0);
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
/* public */
void ProfilerCallback::_ConnectToUI()
{
    HRESULT hr = S_OK;
 
    // Try to open a named pipe; wait for it, if necessary. 
    while (1) 
    { 
        m_hPipe = CreateFileA( OMV_PIPE_NAME,                   // pipe name 
                              GENERIC_READ |  GENERIC_WRITE,    // read and write access 
                              0,                                // no sharing 
                              NULL,                             // no security attributes
                              OPEN_EXISTING,                    // opens existing pipe 
                              0,                                // default attributes 
                              NULL );                           // no template file 

        // Break if the pipe handle is valid. 
        if ( m_hPipe != INVALID_HANDLE_VALUE ) 
            break; 

        // Exit if an error other than ERROR_PIPE_BUSY occurs. 
        if ( GetLastError() == ERROR_PIPE_BUSY )
        {
            // All pipe instances are busy, so wait 3 minutes and then bail out
            if ( !WaitNamedPipeA( OMV_PIPE_NAME, 180000 ) )
                hr = E_FAIL;
        }
        else
            hr = E_FAIL;

        if ( FAILED( hr ) && !m_bAttachLoaded)
        {
            TEXT_OUTLN( "Warning: Could Not Open Pipe" )
            break;
        }
    } 
 
    if ( SUCCEEDED( hr ) )
    {
        DWORD dwMode; 
        BOOL fSuccess; 
        
        // The pipe connected; change to message-read mode. 
        dwMode = PIPE_READMODE_MESSAGE; 
        fSuccess = SetNamedPipeHandleState( m_hPipe,   // pipe handle 
                                            &dwMode,   // new pipe mode 
                                            NULL,      // don't set maximum bytes 
                                            NULL);     // don't set maximum time 
        if ( fSuccess == TRUE )
        {
            DWORD cbWritten;
            LPVOID lpvMessage; 
            char processIDString[BYTESIZE+1];


            // Send a message to the pipe server. 
            sprintf_s( processIDString, ARRAY_LEN(processIDString), "%08x", m_dwProcessId );
            lpvMessage = processIDString; 
            fSuccess = WriteFile( m_hPipe,                       // pipe handle 
                                  lpvMessage,                    // message 
                                  (DWORD)strlen((char*)lpvMessage) + 1, // message length 
                                  &cbWritten,                    // bytes written 
                                  NULL );                        // not overlapped 

            if ( fSuccess == TRUE )
            {
                DWORD cbRead; 
                 
                if(m_bAttachLoaded)
                {
                    //In attach mode, the CLRrofiler shell is blocked waiting for the InitializeAttach 
                    //So, do not try to looping read pipe here.
                }
                else
                {
    
                    //
                    // Read from the pipe the server's reply
                    //
                    do 
                    { 
                        // Read from the pipe. 
                        fSuccess = ReadFile( m_hPipe,           // pipe handle 
                                             m_logFileName,     // buffer to receive reply 
                                             MAX_LENGTH,        // size of buffer 
                                             &cbRead,           // number of bytes read 
                                             NULL );            // not overlapped 
                        
                        if ( (!fSuccess) && (GetLastError() != ERROR_MORE_DATA) ) 
                            break; 

                        // Make sure that the UI received some information 
                        if ( (cbRead == 0) || m_logFileName[0] == NULL )
                        {
                            //
                            // there is an error here ...
                            //
                            TEXT_OUTLN( "WARNING: FileName Was Not properly Read By The UI Will Use Default" )
                            break;
                        }
                        printf("Log file name transmitted from UI is: %s\n", m_logFileName);
                    }
                    while ( !fSuccess );  // repeat loop if ERROR_MORE_DATA 
                }
            }
            else
                TEXT_OUTLN( "Win32 WriteFile() FAILED" ); 
        }
        else 
            TEXT_OUTLN( "Win32 SetNamedPipeHandleState() FAILED" ) 
    }
        
    if ( m_hPipe != NULL )
    {
        CloseHandle( m_hPipe ); 
    }
} // ProfilerCallback::_ConnectToUI


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
/* public */
void ProfilerCallback::SendMessageToUI(const char *message)
{
    // Try to open a named pipe; wait for it, if necessary. 
    HANDLE hPipe;

    // IMPORTANT: bufferSize must match maxloggingBufferSize defined in mainform.cs.
    const int bufferSize = 512;
    char buffer[bufferSize];
    size_t messageLength = min(bufferSize-1, strlen(message)+1);
    memset(buffer, 0, bufferSize);
    StringCchCopyA(buffer, bufferSize, message);

    while (1) 
    { 
        hPipe = CreateFileA( OMV_LOGGING_PIPE_NAME,            // pipe name 
                             GENERIC_READ |  GENERIC_WRITE,    // read and write access 
                             0,                                // no sharing 
                             NULL,                             // no security attributes
                             OPEN_EXISTING,                    // opens existing pipe 
                             0,                                // default attributes 
                             NULL );                           // no template file 

        // Break if the pipe handle is valid. 
        if ( hPipe != INVALID_HANDLE_VALUE ) 
            break; 
        // Exit if an error other than ERROR_PIPE_BUSY occurs. 
        if ( GetLastError() == ERROR_PIPE_BUSY )
        {
            // All pipe instances are busy, so wait 20 seconds and then bail out
            if ( !WaitNamedPipeA( OMV_LOGGING_PIPE_NAME, 20000 ) )
                return;
        }
        else
            return;
    } 
 
    // The pipe connected; change to message-read mode. 
    DWORD dwMode = PIPE_READMODE_MESSAGE; 
    BOOL fSuccess = SetNamedPipeHandleState( hPipe,   // pipe handle 
                                             &dwMode, // new pipe mode 
                                             NULL,    // don't set maximum bytes 
                                             NULL);   // don't set maximum time 

    if ( fSuccess == TRUE )
    {
        DWORD cbWritten;
        // Send a message to the pipe server. 
        fSuccess = WriteFile( hPipe,                  // pipe handle 
                              (LPVOID)buffer,         // message 
                              (DWORD)bufferSize,   // message length 
                              &cbWritten,             // bytes written 
                              NULL );                 // not overlapped 

    }
    CloseHandle( hPipe ); 
} // ProfilerCallback::SendMessageToUI


/***************************************************************************************
 ********************                                               ********************
 ********************              Functions called from Profilee   ********************
 ********************                                               ********************
 ***************************************************************************************/ 

extern "C" BOOL __stdcall GetAllocationLoggingActive()
{
    if (g_pCallbackObject != NULL)
        return g_pCallbackObject->GetAllocationLoggingActive();
    else
        return FALSE;
}

extern "C" void __stdcall SetAllocationLoggingActive(bool active)
{
    if (g_pCallbackObject != NULL)
        g_pCallbackObject->SetAllocationLoggingActive(active);
}

extern "C" BOOL __stdcall GetCallLoggingActive()
{
    if (g_pCallbackObject != NULL)
        return g_pCallbackObject->GetCallLoggingActive();
    else
        return FALSE;
}

extern "C" void __stdcall SetCallLoggingActive(bool active)
{
    if (g_pCallbackObject != NULL)
        g_pCallbackObject->SetCallLoggingActive(active);
}

extern "C" BOOL __stdcall DumpHeap(DWORD timeOut)
{
    if (g_pCallbackObject != NULL)
        return g_pCallbackObject->DumpHeap(timeOut);
    else
        return TRUE;
}

extern "C" void __stdcall LogComment(const wchar_t *commentString)
{
    if (g_pCallbackObject != NULL)
        g_pCallbackObject->LogComment(commentString);
}

bool ProfilerCallback::GetAllocationLoggingActive()
{
    return (m_dwEventMask & (DWORD) COR_PRF_MONITOR_OBJECT_ALLOCATED) != 0;
}

void ProfilerCallback::SetAllocationLoggingActive(bool active)
{
    if ((active != GetAllocationLoggingActive()) && !m_bAttachLoaded)
    {
        if (active)
            m_dwEventMask |= (DWORD) COR_PRF_MONITOR_OBJECT_ALLOCATED;
        else
            m_dwEventMask &= ~(DWORD) COR_PRF_MONITOR_OBJECT_ALLOCATED;
        m_pProfilerInfo->SetEventMask( m_dwEventMask );
    }
}

bool ProfilerCallback::GetCallLoggingActive()
{
    return (m_dwMode & (DWORD) TRACE) != 0;
}

void ProfilerCallback::SetCallLoggingActive(bool active)
{
    if (active)
        m_dwMode |= (DWORD) TRACE;
    else
        m_dwMode &= ~ (DWORD) TRACE;
}


bool ProfilerCallback::DumpHeap(DWORD timeOut)
{
    if (m_dwMode & (DWORD) OBJECT)
    {
        SetEvent(m_hArray[GC_HANDLE]);
        return WaitForSingleObjectEx( m_hArrayCallbacks[GC_HANDLE], timeOut, FALSE /* alertable */ ) == WAIT_OBJECT_0;
    }
    else
        return false;
}

void ProfilerCallback::LogComment(const wchar_t *commentString)
{
    ///////////////////////////////////////////////////////////////////////////
    Synchronize guard( m_criticalSection );
    ///////////////////////////////////////////////////////////////////////////  

    _LogTickCount();
    LogToAny("z %ls\n", commentString);
}

/***************************************************************************************
 ********************                                               ********************
 ********************              DllMain/ClassFactory             ********************
 ********************                                               ********************
 ***************************************************************************************/ 
#include "dllmain.hpp"

// End of File

