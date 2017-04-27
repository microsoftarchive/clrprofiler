// ==++==
// 
//   Copyright (c) Microsoft Corporation.  All rights reserved.
// 
// ==--==
/****************************************************************************************
 * File:
 *  ProfilerCallback.h
 *
 * Description:
 *  
 *
 *
 ***************************************************************************************/
#ifndef __PROFILER_CALLBACK_H__
#define __PROFILER_CALLBACK_H__

#include "mscoree.h"
#include "ProfilerInfo.h"

//
// event names
//
#define OMV_PIPE_NAME         "\\\\.\\pipe\\OMV_PIPE"
#define OMV_LOGGING_PIPE_NAME "\\\\.\\pipe\\OMV_LOGGING_PIPE"


/////////////////////////////////////////////////////////////////////////////////////////
// Each test should provide the following blob (with a new GUID)
//
    // {8C29BC4E-1F57-461a-9B51-1200C32E6F1F}

    extern const GUID __declspec( selectany ) CLSID_PROFILER = 
    { 0x8c29bc4e, 0x1f57, 0x461a, { 0x9b, 0x51, 0x12, 0x0, 0xc3, 0x2e, 0x6f, 0x1f } };

    #define THREADING_MODEL "Both"
    #define PROGID_PREFIX "Objects Profiler"
    #define COCLASS_DESCRIPTION "Microsoft CLR Profiler Test"
    #define PROFILER_GUID "{8C29BC4E-1F57-461a-9B51-1200C32E6F1F}"
    #define PROFILER_GUID_WCHAR L"{8C29BC4E-1F57-461a-9B51-1200C32E6F1F}"
//
/////////////////////////////////////////////////////////////////////////////////////////


//
// arrays with the names of the various events and the IPC related stuff
//
static
char *NamedEvents[] = { "OMV_ForceGC",                                                             
                        "OMV_TriggerObjects",
                        "OMV_Callgraph",
                        "OMV_GC",
                        "OMV_Detach",
                      };

static
char *CallbackNamedEvents[] = { "OMV_ForceGC_Completed",                                                             
                                "OMV_TriggerObjects_Completed",
                                "OMV_Callgraph_Completed",
                                "OMV_GC_Completed",
                                "OMV_Detach_Completed",
                              };
static
char GlobalPrefixForNamedEvents[] = "Global\\";

//
// thread routines
//
DWORD __stdcall _GCThreadStub( void *pObject );
DWORD __stdcall _TriggerThreadStub( void *pObject );
DWORD __stdcall _CallstackThreadStub( void *pObject );


//
// IMPORTANT: ProfConfig structure has a counterpart managed structure defined in 
// mainform.cs.  Both must always be in sync.  
// Gather all config info in one place. On startup, this may be read from the
// environment. On attach, this is sent as client data.
//
typedef enum _OmvUsage
{
    OmvUsageNone    = 0,
    OmvUsageObjects = 1,
    OmvUsageTrace   = 2,
    OmvUsageBoth    = 3,
    OmvUsageInvalid = 4,
} OmvUsage;

struct ProfConfig
{
    OmvUsage usage;
    BOOL  bOldFormat;
    char  szPath[256];
    char  szFileName[256];
    BOOL  bDynamic;
    BOOL  bStack;
    DWORD dwFramesToPrint;
    DWORD dwSkipObjects;
    char  szClassToMonitor[256];
    DWORD dwInitialSetting;
    DWORD dwDefaultTimeoutMs;
    BOOL  bWindowsStoreApp;
};

/***************************************************************************************
 ********************                                               ********************
 ********************       ProfilerCallback Declaration            ********************
 ********************                                               ********************
 ***************************************************************************************/

class ProfilerCallback : 
    public PrfInfo,
    public ICorProfilerCallback3
{
    public:
    
        ProfilerCallback();
        ~ProfilerCallback();

        HRESULT Init(ProfConfig * pProfConfig);

    public:

        //
        // IUnknown 
        //
        COM_METHOD( ULONG ) AddRef(); 
        COM_METHOD( ULONG ) Release();
        COM_METHOD( HRESULT ) QueryInterface( REFIID riid, void **ppInterface );


        //
        // STARTUP/SHUTDOWN EVENTS
        //
        virtual COM_METHOD( HRESULT ) Initialize( IUnknown *pICorProfilerInfoUnk );
               
        HRESULT DllDetachShutdown();                           
        COM_METHOD( HRESULT ) Shutdown();
                                         

        //
        // APPLICATION DOMAIN EVENTS
        //
        COM_METHOD( HRESULT ) AppDomainCreationStarted( AppDomainID appDomainID );
        
        COM_METHOD( HRESULT ) AppDomainCreationFinished( AppDomainID appDomainID,
                                                         HRESULT hrStatus );
    
        COM_METHOD( HRESULT ) AppDomainShutdownStarted( AppDomainID appDomainID );

        COM_METHOD( HRESULT ) AppDomainShutdownFinished( AppDomainID appDomainID, 
                                                         HRESULT hrStatus );


        //
        // ASSEMBLY EVENTS
        //
        COM_METHOD( HRESULT ) AssemblyLoadStarted( AssemblyID assemblyID );
        
        COM_METHOD( HRESULT ) AssemblyLoadFinished( AssemblyID assemblyID,
                                                    HRESULT hrStatus );
    
        COM_METHOD( HRESULT ) AssemblyUnloadStarted( AssemblyID assemblyID );

        COM_METHOD( HRESULT ) AssemblyUnloadFinished( AssemblyID assemblyID, 
                                                      HRESULT hrStatus );
        
        
        //
        // MODULE EVENTS
        //
        COM_METHOD( HRESULT ) ModuleLoadStarted( ModuleID moduleID );
        
        COM_METHOD( HRESULT ) ModuleLoadFinished( ModuleID moduleID,
                                                  HRESULT hrStatus );
    
        COM_METHOD( HRESULT ) ModuleUnloadStarted( ModuleID moduleID );

        COM_METHOD( HRESULT ) ModuleUnloadFinished( ModuleID moduleID, 
                                                    HRESULT hrStatus );

        COM_METHOD( HRESULT ) ModuleAttachedToAssembly( ModuleID moduleID,
                                                        AssemblyID assemblyID );
                
        
        //
        // CLASS EVENTS
        //
        COM_METHOD( HRESULT ) ClassLoadStarted( ClassID classID );
        
        COM_METHOD( HRESULT ) ClassLoadFinished( ClassID classID,
                                                 HRESULT hrStatus );
    
        COM_METHOD( HRESULT ) ClassUnloadStarted( ClassID classID );

        COM_METHOD( HRESULT ) ClassUnloadFinished( ClassID classID, 
                                                   HRESULT hrStatus );

        COM_METHOD( HRESULT ) FunctionUnloadStarted( FunctionID functionID );
        
        
        //
        // JIT EVENTS
        //              
        COM_METHOD( HRESULT ) JITCompilationStarted( FunctionID functionID,
                                                     BOOL fIsSafeToBlock );
                                        
        COM_METHOD( HRESULT ) JITCompilationFinished( FunctionID functionID,
                                                      HRESULT hrStatus,
                                                      BOOL fIsSafeToBlock );
    
        COM_METHOD( HRESULT ) JITCachedFunctionSearchStarted( FunctionID functionID,
                                                              BOOL *pbUseCachedFunction );
        
        COM_METHOD( HRESULT ) JITCachedFunctionSearchFinished( FunctionID functionID,
                                                               COR_PRF_JIT_CACHE result );
                                                                     
        COM_METHOD( HRESULT ) JITFunctionPitched( FunctionID functionID );
        
        COM_METHOD( HRESULT ) JITInlining( FunctionID callerID,
                                           FunctionID calleeID,
                                           BOOL *pfShouldInline );

        
        //
        // THREAD EVENTS
        //
        COM_METHOD( HRESULT ) ThreadCreated( ThreadID threadID );
    
        COM_METHOD( HRESULT ) ThreadDestroyed( ThreadID threadID );

        COM_METHOD( HRESULT ) ThreadAssignedToOSThread( ThreadID managedThreadID,
                                                        DWORD osThreadID );
        //
        // REMOTING EVENTS
        //                                                      

        //
        // Client-side events
        //
        COM_METHOD( HRESULT ) RemotingClientInvocationStarted();

        COM_METHOD( HRESULT ) RemotingClientSendingMessage( GUID *pCookie,
                                                            BOOL fIsAsync );

        COM_METHOD( HRESULT ) RemotingClientReceivingReply( GUID *pCookie,
                                                            BOOL fIsAsync );

        COM_METHOD( HRESULT ) RemotingClientInvocationFinished();

        //
        // Server-side events
        //
        COM_METHOD( HRESULT ) RemotingServerReceivingMessage( GUID *pCookie,
                                                              BOOL fIsAsync );

        COM_METHOD( HRESULT ) RemotingServerInvocationStarted();

        COM_METHOD( HRESULT ) RemotingServerInvocationReturned();

        COM_METHOD( HRESULT ) RemotingServerSendingReply( GUID *pCookie,
                                                          BOOL fIsAsync );


        //
        // CONTEXT EVENTS
        //                                                      
        COM_METHOD( HRESULT ) UnmanagedToManagedTransition( FunctionID functionID,
                                                            COR_PRF_TRANSITION_REASON reason );
    
        COM_METHOD( HRESULT ) ManagedToUnmanagedTransition( FunctionID functionID,
                                                            COR_PRF_TRANSITION_REASON reason );
                                                                  
                                                                        
        //
        // SUSPENSION EVENTS
        //    
        COM_METHOD( HRESULT ) RuntimeSuspendStarted( COR_PRF_SUSPEND_REASON suspendReason );

        COM_METHOD( HRESULT ) RuntimeSuspendFinished();

        COM_METHOD( HRESULT ) RuntimeSuspendAborted();

        COM_METHOD( HRESULT ) RuntimeResumeStarted();

        COM_METHOD( HRESULT ) RuntimeResumeFinished();

        COM_METHOD( HRESULT ) RuntimeThreadSuspended( ThreadID threadid );

        COM_METHOD( HRESULT ) RuntimeThreadResumed( ThreadID threadid );


        //
        // GC EVENTS
        //    
        COM_METHOD( HRESULT ) MovedReferences( ULONG cmovedObjectIDRanges,
                                               ObjectID oldObjectIDRangeStart[],
                                               ObjectID newObjectIDRangeStart[],
                                               ULONG cObjectIDRangeLength[] );
    
        COM_METHOD( HRESULT ) SurvivingReferences( ULONG cmovedObjectIDRanges,
                                                   ObjectID objectIDRangeStart[],
                                                   ULONG cObjectIDRangeLength[] );

        COM_METHOD( HRESULT ) ObjectAllocated( ObjectID objectID,
                                               ClassID classID );
    
        COM_METHOD( HRESULT ) ObjectsAllocatedByClass( ULONG classCount,
                                                       ClassID classIDs[],
                                                       ULONG objects[] );
    
        COM_METHOD( HRESULT ) ObjectReferences( ObjectID objectID,
                                                ClassID classID,
                                                ULONG cObjectRefs,
                                                ObjectID objectRefIDs[] );
    
        COM_METHOD( HRESULT ) RootReferences( ULONG cRootRefs,
                                              ObjectID rootRefIDs[] );
    
        
        //
        // EXCEPTION EVENTS
        //                                                         

        // Exception creation
        COM_METHOD( HRESULT ) ExceptionThrown( ObjectID thrownObjectID );

        // Search phase
        COM_METHOD( HRESULT ) ExceptionSearchFunctionEnter( FunctionID functionID );
    
        COM_METHOD( HRESULT ) ExceptionSearchFunctionLeave();
    
        COM_METHOD( HRESULT ) ExceptionSearchFilterEnter( FunctionID functionID );
    
        COM_METHOD( HRESULT ) ExceptionSearchFilterLeave();
    
        COM_METHOD( HRESULT ) ExceptionSearchCatcherFound( FunctionID functionID );
        
        COM_METHOD( HRESULT ) ExceptionCLRCatcherFound();

        COM_METHOD( HRESULT ) ExceptionCLRCatcherExecute();

        COM_METHOD( HRESULT ) ExceptionOSHandlerEnter( FunctionID functionID );
            
        COM_METHOD( HRESULT ) ExceptionOSHandlerLeave( FunctionID functionID );
    
        // Unwind phase
        COM_METHOD( HRESULT ) ExceptionUnwindFunctionEnter( FunctionID functionID );
    
        COM_METHOD( HRESULT ) ExceptionUnwindFunctionLeave();
        
        COM_METHOD( HRESULT ) ExceptionUnwindFinallyEnter( FunctionID functionID );
    
        COM_METHOD( HRESULT ) ExceptionUnwindFinallyLeave();
        
        COM_METHOD( HRESULT ) ExceptionCatcherEnter( FunctionID functionID,
                                                     ObjectID objectID );
    
        COM_METHOD( HRESULT ) ExceptionCatcherLeave();

        
        //
        // COM CLASSIC WRAPPER
        //
        COM_METHOD( HRESULT )  COMClassicVTableCreated( ClassID wrappedClassID,
                                                        REFGUID implementedIID,
                                                        void *pVTable,
                                                        ULONG cSlots );

        COM_METHOD( HRESULT )  COMClassicVTableDestroyed( ClassID wrappedClassID,
                                                          REFGUID implementedIID,
                                                          void *pVTable );

        COM_METHOD( HRESULT ) STDMETHODCALLTYPE ThreadNameChanged( 
            /* [in] */ ThreadID threadId,
            /* [in] */ ULONG cchName,
            /* [in] */ __in_ecount_opt(cchName) WCHAR name[  ]);
    
        /*
         * The CLR calls GarbageCollectionStarted before beginning a 
         * garbage collection. All GC callbacks pertaining to this
         * collection will occur between the GarbageCollectionStarted
         * callback and the corresponding GarbageCollectionFinished
         * callback, which will occur on the same thread.
         *
         *          cGenerations indicates the total number of entries in
         *                the generationCollected array
         *          generationCollected is an array of booleans, indexed
         *                by COR_PRF_GC_GENERATIONS, indicating which
         *                generations are being collected in this collection
         *          wasInduced indicates whether this GC was induced
         *                by the application calling GC.Collect().
         */
        COM_METHOD( HRESULT )  GarbageCollectionStarted(
            /* [in] */int cGenerations,
            /*[in, size_is(cGenerations), length_is(cGenerations)]*/ BOOL generationCollected[],
            /*[in]*/ COR_PRF_GC_REASON reason);

        /*
         * The CLR calls GarbageCollectionFinished after a garbage
         * collection has completed and all GC callbacks have been
         * issued for it.
         */
        COM_METHOD( HRESULT )  GarbageCollectionFinished();

        /*
         * The CLR calls FinalizeableObjectQueued to notify the code profiler
         * that an object with a finalizer (destructor in C# parlance) has
         * just been queued to the finalizer thread for execution of its
         * Finalize method.
         *
         * finalizerFlags describes aspects of the finalizer, and takes its
         *     value from COR_PRF_FINALIZER_FLAGS.
         *
         */

        COM_METHOD( HRESULT ) STDMETHODCALLTYPE FinalizeableObjectQueued(
            /* [in] */DWORD finalizerFlags,
            /* [in] */ObjectID objectID);

        COM_METHOD( HRESULT ) STDMETHODCALLTYPE RootReferences2( 
            /* [in] */ ULONG cRootRefs,
            /* [size_is][in] */ ObjectID rootRefIds[  ],
            /* [size_is][in] */ COR_PRF_GC_ROOT_KIND rootKinds[  ],
            /* [size_is][in] */ COR_PRF_GC_ROOT_FLAGS rootFlags[  ],
            /* [size_is][in] */ UINT_PTR rootIds[  ]);

        /*
         * The CLR calls HandleCreated when a gc handle has been created.
         *
         */
        COM_METHOD( HRESULT ) STDMETHODCALLTYPE HandleCreated(
            /* [in] */ UINT_PTR handleId,
            /* [in] */ ObjectID initialObjectId);

        /*
         * The CLR calls HandleDestroyed when a gc handle has been destroyed.
         *
         */
        COM_METHOD( HRESULT ) STDMETHODCALLTYPE HandleDestroyed(
            /* [in] */ UINT_PTR handleId);

        //
        // ATTACH EVENTS
        //
        COM_METHOD( HRESULT )  STDMETHODCALLTYPE InitializeForAttach( 
            /* [in] */ IUnknown *pICorProfilerInfoUnk,
            /* [in] */ void *pvClientData,
            /* [in] */ UINT cbClientData);
        
        COM_METHOD( HRESULT )  ProfilerAttachComplete();
        
        COM_METHOD( HRESULT )  ProfilerDetachSucceeded();


        //
        // instantiate an instance of the callback interface
        //
        static COM_METHOD( HRESULT) CreateObject( REFIID riid, void **ppInterface );            
        
                                                                                                     
        // used by function hooks, they have to be static
        static void  Enter( FunctionID functionID );
        static void  Leave( FunctionID functionID );
        static void  Tailcall( FunctionID functionID );
        static ThreadInfo *GetThreadInfo();

        //
        // wrapper for the threads
        //
        void _ThreadStubWrapper( );
        void DetachThreadStub();

    private:
        UINT BeginTimer();
        void EndTimer(UINT wTimerRes);
        ULONGLONG _GetTickCount();

        HRESULT _AddGCObject( BOOL bForce = FALSE );
        SIZE_T _StackTraceId(SIZE_T typeId=0, SIZE_T typeSize=0);
        void _LogTickCount();
        void _ShutdownAllThreads();
        void _GetProfConfigFromEnvironment(ProfConfig *pProfConfig);
        void _ProcessProfConfig(ProfConfig *pProfConfig);
        void LogToAny( const char *format, ... );

        HRESULT _InitializeThreadsAndEvents();
        HRESULT _LogCallTrace( FunctionID functionID );
        HRESULT _InitializeNamesForEventsAndCallbacks();
        HRESULT _InsertGCClass( ClassInfo **ppClassInfo, ClassID classID );
        HRESULT _GetNameFromElementType( CorElementType elementType, __out_ecount(buflen) WCHAR *buffer, size_t buflen );
        bool _ClassIsFinalizable(ModuleID moduleID, mdToken classToken);
        bool _ClassOverridesFinalize(IMetaDataImport *pMetaDataImport, mdToken classToken);
        void _CloseEventHandles();
        bool _ClassHasFinalizeMethod(IMetaDataImport *pMetaDataImport, mdToken classToken, DWORD *pdwAttr);
        bool _ClassReintroducesFinalize(IMetaDataImport *pMetaDataImport, mdToken classToken);
        void _GenerationBounds(BOOL beforeCollection, BOOL induced, int generation);
        //
        // pipe operations with the GUI
        //
        void _ConnectToUI();

        //
        // methods called by profiled program
        //
    public:
        bool GetAllocationLoggingActive();
        void SetAllocationLoggingActive(bool active);
        bool GetCallLoggingActive();
        void SetCallLoggingActive(bool active);
        bool DumpHeap(DWORD timeOut);
        void LogComment(const wchar_t *commentString);
        void SendMessageToUI(const char *message);
    
    private:

        ULONG  m_GCcounter[COR_PRF_GC_GEN_2 + 1];
        DWORD  m_condemnedGeneration[2];
        USHORT m_condemnedGenerationIndex;       

        // various counters
        long m_refCount;                        
        DWORD m_dwShutdown;
        DWORD m_callStackCount;

        // counters
        LONG m_totalClasses;
        LONG m_totalModules;
        LONG m_totalFunctions;
        ULONG m_totalObjectsAllocated;
        
        // operation indicators
        char *m_path;
        HANDLE m_hPipe;
        DWORD m_dwMode;
        BOOL m_bInitialized;
        BOOL m_bShutdown;
        BOOL m_bDumpGCInfo;
        DWORD m_dwProcessId;
        BOOL m_bDumpCompleted;
        DWORD m_dwSkipObjects;
        BOOL m_bMonitorParents;
        DWORD m_dwFramesToPrint;
        WCHAR *m_classToMonitor;
        BOOL m_bTrackingObjects;
        BOOL m_bTrackingCalls;
        BOOL m_bIsTrackingStackTrace;
        CRITICAL_SECTION m_criticalSection;
        BOOL m_oldFormat;
        BOOL m_bTargetV2CLR;
        DWORD m_dwSentinelHandle;
        
        // file stuff
        FILE *m_stream;
        ULONGLONG m_firstTickCount;
        ULONGLONG m_lastTickCount;
        UINT m_timerResolution;

        // event and thread handles need to be accessed by the threads
        HANDLE m_hArray[(DWORD)SENTINEL_HANDLE];
        HANDLE m_hArrayCallbacks[(DWORD)SENTINEL_HANDLE];
        HANDLE m_hThread;
        DWORD m_dwWin32ThreadID;

        // names for the events and the callbacks
        char m_logFileName[MAX_LENGTH+1];
        char *m_NamedEvents[SENTINEL_HANDLE];
        char *m_CallbackNamedEvents[SENTINEL_HANDLE];

        BOOL m_bWindowsStoreApp;
}; // ProfilerCallback

extern ProfilerCallback *g_pCallbackObject;     // global reference to callback object

#endif //  __PROFILER_CALLBACK_H__

// End of File
        
        
