// ==++==
// 
//   Copyright (c) Microsoft Corporation.  All rights reserved.
// 
// ==--==
/****************************************************************************************
 * File:
 *  ProfilerInfo.h
 *
 * Description:
 *
 *
 *
 ***************************************************************************************/
#ifndef __PROFILER_INFO_H__
#define __PROFILER_INFO_H__


#include "avlnode.h"
#include "basehlp.h"
#include "container.hpp"



#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

#define BYTESIZE 8

//
// env variables
//
#define OMV_PATH                "OMV_PATH"
#define OMV_FILENAME            "OMV_FILENAME"
#define OMV_SKIP                "OMV_SKIP"
#define OMV_CLASS               "OMV_CLASS"
#define OMV_STACK               "OMV_STACK"
#define OMV_USAGE               "OMV_USAGE"
#define OMV_FRAMENUMBER         "OMV_FRAMES"
#define OMV_DYNAMIC             "OMV_DynamicObjectTracking"
#define OMV_FORMAT              "OMV_FORMAT"
#define	OMV_INITIAL_SETTING     "OMV_INITIAL_SETTING"
#define OMV_TARGET_CLR_VERSION  "OMV_TargetCLRVersion"
#define OMV_WINDOWSSTOREAPP     "OMV_WINDOWSSTOREAPP"


/***************************************************************************************
 ********************                                               ********************
 ********************              Enum Definitions                 ********************
 ********************                                               ********************
 ***************************************************************************************/
//
// enums
//
enum Operation
{ 
    OBJECT  = 0x1,
    TRACE   = 0x2, 
    BOTH    = 0x3,
    DYNOBJECT = 0x4,
};


enum StackAction
{ 
    PUSH, 
    POP, 
};


enum ObjHandles
{ 
    GC_HANDLE = 0, 
    OBJ_HANDLE = 1,
    CALL_HANDLE = 2,
    TRIGGER_GC_HANDLE = 3,
    DETACH_HANDLE = 4,
    SENTINEL_HANDLE = 5,
};

/***************************************************************************************
 ********************                                               ********************
 ********************                   LightStack                  ********************
 ********************                                               ********************
 ***************************************************************************************/

//
// This stack is lightweight, uses an array to store simple SIZE_Ts and it does not have
// critical section locking. It is supposed to be used from areas of code that we are thread safe
// already.  
//
class LStack
{
    public:
    
        LStack( ULONG size );
        virtual ~LStack();

        LStack( const LStack &source );


    public:

        __forceinline void    Push( SIZE_T item )
        {
            if ( m_Count < m_Size )
            {
                m_Array[m_Count] = item;
                m_Count++;
            }
            else
                GrowStackPush( item );
        }

        void    GrowStackPush( SIZE_T item );

        __forceinline SIZE_T  Pop()
        {
            SIZE_T item = -1;

            if ( m_Count != 0 )
            {
                m_Count--;
                item = m_Array[m_Count];
            }
            return item;
        }

        SIZE_T  Top();
        BOOL    Empty();
        ULONG   Count();


    private:
    
        ULONG   m_Count;
        ULONG   m_Size;
        
    public:
        
        SIZE_T  *m_Array;

}; // LStack


/***************************************************************************************
 ********************                                               ********************
 ********************             BaseInfo Declaration              ********************
 ********************                                               ********************
 ***************************************************************************************/
class BaseInfo
{
    public:
    
        BaseInfo( SIZE_T id, SIZE_T internal = 0 );         
        virtual ~BaseInfo();
                
        
    public:
            
        virtual void Dump();
        template<typename T> BOOL Compare( T key );
        template<typename T> Comparison CompareEx( T key );
        
        
    public:
    
        SIZE_T m_id;
        SIZE_T m_internalID;
    
}; // BaseInfo


/***************************************************************************************
 ********************                                               ********************
 ********************              StackTrace Declaration           ********************
 ********************                                               ********************
 ***************************************************************************************/
struct StackTrace
{
    DWORD m_count;
    SIZE_T *m_stack;
    SIZE_T m_key;
    SIZE_T m_typeId;
    SIZE_T m_typeSize;

    StackTrace(DWORD count, SIZE_T *stack, SIZE_T typeId=0, SIZE_T typeSize=0)
    {
        m_count = count;
        m_stack = stack;
        m_typeId = typeId;
        m_typeSize = typeSize;

        SIZE_T key = (count*137 + typeId)*137 + typeSize;
        for (DWORD i = 0; i < count; i++)
            key = key*137 + stack[i];
        m_key = key;
    }

    operator SIZE_T()
    {
        return m_key;
    }
}; // StackTrace


/***************************************************************************************
 ********************                                               ********************
 ********************              StackTraceInfo Declaration       ********************
 ********************                                               ********************
 ***************************************************************************************/

class StackTraceInfo
{
public:
    DWORD m_count;
    SIZE_T *m_stack;
    SIZE_T m_key;
    SIZE_T m_typeId;
    SIZE_T m_typeSize;

public:
    SIZE_T m_internalId;

    StackTraceInfo(SIZE_T internalId, DWORD count, SIZE_T *stack, SIZE_T typeId=0, SIZE_T typeSize=0)
    {
        m_internalId = internalId;
        m_count = count;
        m_stack = NULL;
        if (count < 0x10000000)
            m_stack = new SIZE_T[count];
        if (m_stack == NULL)
            _THROW_EXCEPTION( "Allocation for StackTraceInfo FAILED" );
        m_typeId = typeId;
        m_typeSize = typeSize;

        SIZE_T key = (count*137 + typeId)*137 + typeSize;
        for (DWORD i = 0; i < count; i++)
        {
            key = key*137 + stack[i];
            m_stack[i] = stack[i];
        }
        m_key = key;
    }

    BOOL Compare( const StackTrace &stackTrace )
    {
        if (m_key != stackTrace.m_key || m_count != stackTrace.m_count || m_typeId != stackTrace.m_typeId || m_typeSize != stackTrace.m_typeSize)
        {
            return FALSE;
        }
        for (int i = m_count; --i >= 0; )
        {
            if (m_stack[i] != stackTrace.m_stack[i])
            {
                return FALSE;
            }
        }
        return TRUE;
    }
};

/***************************************************************************************
 ********************                                               ********************
 ********************            ThreadInfo Declaration             ********************
 ********************                                               ********************
 ***************************************************************************************/
class ThreadInfo :
    public BaseInfo
{
    public:

        ThreadInfo( ThreadID threadID, SIZE_T internal = 0 );         
        virtual ~ThreadInfo();
        

    public:
    
        virtual void Dump();
                
        
    public:    
                
        DWORD  m_win32ThreadID;
        LStack *m_pThreadCallStack;
        LStack *m_pLatestUnwoundFunction;

        StackTraceInfo *m_pLatestStackTraceInfo;

}; // ThreadInfo


/***************************************************************************************
 ********************                                               ********************
 ********************          FunctionInfo Declaration             ********************
 ********************                                               ********************
 ***************************************************************************************/
class FunctionInfo :
    public BaseInfo
{
    public:

        FunctionInfo( FunctionID functionID, SIZE_T internal = 0 );
        virtual ~FunctionInfo();
      

    public:
    
        void Dump();     
        
        
    public:    
                                        
        WCHAR    m_functionName[MAX_LENGTH];
        WCHAR    m_functionSig[4*MAX_LENGTH];
        
}; // FunctionInfo


/***************************************************************************************
 ********************                                               ********************
 ********************             ClassInfo Declaration             ********************
 ********************                                               ********************
 ***************************************************************************************/
class ClassInfo :
    public BaseInfo
{
    public:

        ClassInfo( ClassID classID, SIZE_T internal = 0 );
        virtual ~ClassInfo();
      

    public:
    
        void Dump();        
        
        
    public:    
                   
        ULONG     m_objectsAllocated;
        WCHAR     m_className[4*MAX_LENGTH];

}; // ClassInfo


/***************************************************************************************
 ********************                                               ********************
 ********************          ModuleInfo Declaration               ********************
 ********************                                               ********************
 ***************************************************************************************/
class ModuleInfo :
    public BaseInfo
{
    public:

        ModuleInfo( ModuleID moduleID, SIZE_T internal = 0 );
        virtual ~ModuleInfo();
      

    public:
    
        void Dump();     
        
        
    public:    
                                        
        WCHAR    m_moduleName[MAX_LENGTH];
        LPCBYTE  m_loadAddress;
        
}; // ModuleInfo


/***************************************************************************************
 ********************                                               ********************
 ********************              PrfInfo Declaration              ********************
 ********************                                               ********************
 ***************************************************************************************/
class PrfInfo
{             
    public:
    
        PrfInfo();                     
        virtual ~PrfInfo();                      
                      
                   
    public:
                    
        void AddThread( ThreadID threadID );                        
        void RemoveThread( ThreadID threadID );                        

        void AddFunction( FunctionID functionID, SIZE_T internalID = 0 ); 
        void RemoveFunction( FunctionID functionID ); 

        void AddModule( ModuleID moduleID, SIZE_T internalID = 0 ); 
        void RemoveModule( ModuleID moduleID ); 

        void UpdateCallStack( FunctionID functionID, StackAction action );
        void UpdateOSThreadID( ThreadID managedThreadID, DWORD osThreadID );
        void UpdateUnwindStack( FunctionID *functionID, StackAction action );
        HRESULT GetNameFromClassID( ClassID classID, __out WCHAR className[] );


        void Failure( const char *message = NULL );


    private:
    
        // helpers                
        void _GetFunctionSig(FunctionInfo **ppFunctionInfo );
        
        //
        // print element type
        //
        PCCOR_SIGNATURE ParseElementType( IMetaDataImport *pMDImport, 
                                          PCCOR_SIGNATURE signature, 
                                          ClassID *classTypeArgs,
                                          ClassID *methodTypeArgs,
                                          __out_ecount(cchBuffer) char *buffer,
                                          size_t cchBuffer );
                                                 
        //
        // process metadata for a function given its functionID
        //
        HRESULT GetFunctionProperties( FunctionID functionID,
                                       BOOL *isStatic,
                                       ULONG *argCount,
                                       __out_ecount(returnTypeStrLen) WCHAR *returnTypeStr, 
                                       size_t returnTypeStrLen,
                                       __out_ecount(functionParametersLen) WCHAR *functionParameters,
                                       size_t functionParametersLen,
                                       __out_ecount(functionNameLen) WCHAR *functionName,
                                       size_t functionNameLen );

        HRESULT GetClassName(IMetaDataImport *pMDImport, mdToken classToken, __out WCHAR className[], ClassID *classTypeArgs, ULONG *totalGenericArgCount);

        void AppendTypeArgName(ULONG argIndex, ClassID *actualClassTypeArgs, ClassID *actualMethodTypeArgs, BOOL methodFormalArg, __out_ecount(cchBuffer) char *buffer, size_t cchBuffer);

    protected:    
                  
        DWORD m_dwEventMask;
        ICorProfilerInfo *m_pProfilerInfo;        
        ICorProfilerInfo2 *m_pProfilerInfo2;
        ICorProfilerInfo3 *m_pProfilerInfo3;
        BOOL m_bAttachLoaded;
        LONG m_bWaitingForTheFirstGC;
        
        // tables         
        SList<ThreadInfo *, ThreadID> *m_pThreadTable;
        HashTable<ClassInfo *, ClassID> *m_pClassTable;
        Table<ModuleInfo *, ModuleID> *m_pModuleTable;
        Table<FunctionInfo *, FunctionID> *m_pFunctionTable;
        HashTable<StackTraceInfo *, StackTrace> *m_pStackTraceTable;

}; // PrfInfo


#endif // __PROFILER_INFO_H___

// End of File
