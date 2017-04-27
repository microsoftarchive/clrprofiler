// ==++==
// 
//   Copyright (c) Microsoft Corporation.  All rights reserved.
// 
// ==--==
/****************************************************************************************
 * File:
 *  ProfilerInfo.cpp
 *
 * Description:
 *
 *
 *
 ***************************************************************************************/
#include "avlnode.h"
#include "basehlp.h"

#include "ProfilerInfo.h"


/***************************************************************************************
 ********************                                               ********************
 ********************             LStack Implementation             ********************
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
LStack::LStack( ULONG size ) :
    m_Count( 0 ),
    m_Size( 0 ),
    m_Array( NULL )    
{    
    if (size < 0x10000000)
    {
        m_Array = new SIZE_T[size];
        m_Size = size;
    }
    else
    {
        _THROW_EXCEPTION( "Allocation for m_Array FAILED" )
    }
} // ctor


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
LStack::~LStack()
{
    if ( m_Array != NULL )
    {
        delete[] m_Array;
        m_Array = NULL;
    }

} // dtor


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
LStack::LStack( const LStack &source ) 
{
    m_Size = source.m_Size;
    m_Count = source.m_Count;
    m_Array = source.m_Array;
    
} // copy ctor


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
ULONG LStack::Count() 
{
    return m_Count;
    
} // LStack::Count


SIZE_T *GrowStack(ULONG newSize, ULONG currentSize, SIZE_T *stack)
{
    // Paranoia & shut up PREFAST
    if (newSize <= currentSize)
        return stack;

    SIZE_T *newStack = NULL;
    
    if (newSize < 0x10000000)
        newStack = new SIZE_T[newSize];

    if ( newStack != NULL )
    {
        //
        // copy all the elements
        //
        for (ULONG i =0; i < currentSize; i++ )
        {
            newStack[i] = stack[i];
        }

        delete[] stack;
        
        return newStack;
    }
    else
        _THROW_EXCEPTION( "Allocation for m_Array FAILED" )
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
void LStack::GrowStackPush( SIZE_T item )
{
    if ( m_Count == m_Size )
    {
        m_Array = GrowStack(2*m_Count, m_Count, m_Array);
        m_Size = 2*m_Count;
    }
    m_Array[m_Count] = item;
    m_Count++;
} // LStack::GrowStackPush


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
SIZE_T LStack::Top()
{        

    if ( m_Count == 0 )
        return -1;
    
    else
        return m_Array[m_Count-1];
    
} // LStack::Top


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
BOOL LStack::Empty() 
{
    return (BOOL)(m_Count == NULL);
    
} // LStack::Empty


/***************************************************************************************
 ********************                                               ********************
 ********************             BaseInfo Implementation           ********************
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
BaseInfo::BaseInfo( SIZE_T id, SIZE_T internal ) : 
    m_id( id ),
    m_internalID( internal )
{   
} // ctor         


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
/* public virtual */
BaseInfo::~BaseInfo()
{       
} // dtor


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
template<typename T>
BOOL BaseInfo::Compare( T in_key )
{
    SIZE_T key = (SIZE_T)in_key;

    return (BOOL)(m_id == key);
    
} // BaseInfo::Compare


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
template<typename T>
Comparison BaseInfo::CompareEx( T in_key )
{
    Comparison res = EQUAL_TO;
    SIZE_T key = (SIZE_T)in_key;

    if ( key > m_id )
        res =  GREATER_THAN;
    
    else if ( key < m_id )
        res = LESS_THAN;


    return res;

} // BaseInfo::CompareEx


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
void BaseInfo::Dump( )
{} // BaseInfo::Dump


/***************************************************************************************
 ********************                                               ********************
 ********************            ThreadInfo Implementation          ********************
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
ThreadInfo::ThreadInfo( ThreadID threadID, SIZE_T internal ) : 
    BaseInfo( threadID, internal ),
    m_win32ThreadID( 0 )
{
    m_pThreadCallStack = new LStack( MAX_LENGTH );
    m_pLatestUnwoundFunction = new LStack( MAX_LENGTH );
    m_pLatestStackTraceInfo = NULL;
} // ctor         


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
/* public virtual */
ThreadInfo::~ThreadInfo()
{
    if ( m_pThreadCallStack != NULL )
    {      
        delete m_pThreadCallStack;
        m_pThreadCallStack = NULL;
    }
    
    if ( m_pLatestUnwoundFunction != NULL )
    {
        delete m_pLatestUnwoundFunction; 
        m_pLatestUnwoundFunction = NULL;
    }

} // dtor
        

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
void ThreadInfo::Dump()
{} // ThreadInfo::Dump


/***************************************************************************************
 ********************                                               ********************
 ********************          FunctionInfo Implementation          ********************
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
FunctionInfo::FunctionInfo( FunctionID functionID, SIZE_T internal ) : 
    BaseInfo( functionID, internal )    
{
    wcscpy_s( m_functionName, ARRAY_LEN(m_functionName), L"UNKNOWN" );
    wcscpy_s( m_functionSig, ARRAY_LEN(m_functionSig), L"" );
            
    
} // ctor         


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
/* public virtual */
FunctionInfo::~FunctionInfo()
{  
} // dtor
        

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
void FunctionInfo::Dump()
{} // FunctionInfo::Dump


/***************************************************************************************
 ********************                                               ********************
 ********************          ModuleInfo Implementation            ********************
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
ModuleInfo::ModuleInfo( ModuleID moduleID, SIZE_T internal ) : 
    BaseInfo( moduleID, internal ),
    m_loadAddress( 0 )    
{
    wcscpy_s( m_moduleName, ARRAY_LEN(m_moduleName), L"UNKNOWN" );
    
} // ctor         


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
/* public virtual */
ModuleInfo::~ModuleInfo()
{  
} // dtor
        

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
void ModuleInfo::Dump()
{} // ModuleInfo::Dump


/***************************************************************************************
 ********************                                               ********************
 ********************            ClassInfo Implementation           ********************
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
ClassInfo::ClassInfo( ClassID classID, SIZE_T internal ) : 
    BaseInfo( classID, internal ),
    m_objectsAllocated( 0 )
{
    wcscpy_s( m_className, ARRAY_LEN(m_className), L"UNKNOWN" ); 

} // ctor         


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
/* public virtual */
ClassInfo::~ClassInfo()
{} // dtor
        

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
void ClassInfo::Dump()
{} // ClassInfo::Dump


/***************************************************************************************
 ********************                                               ********************
 ********************              PrfInfo Implementation           ********************
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
PrfInfo::PrfInfo() :         
    m_pProfilerInfo( NULL ),
    m_pProfilerInfo2( NULL ),
    m_pProfilerInfo3( NULL ),
    m_bAttachLoaded( FALSE ),
    m_bWaitingForTheFirstGC( FALSE ),
    m_dwEventMask( 0 ),
    m_pClassTable( NULL ),
    m_pThreadTable( NULL ),
    m_pFunctionTable( NULL ),
    m_pStackTraceTable( NULL )
{
    // initialize tables
    m_pClassTable = new HashTable<ClassInfo *, ClassID>();
    m_pThreadTable = new SList<ThreadInfo *, ThreadID>();
    m_pModuleTable = new Table<ModuleInfo *, ModuleID>();
    m_pFunctionTable = new Table<FunctionInfo *, FunctionID>();
    m_pStackTraceTable = new HashTable<StackTraceInfo *, StackTrace>();

} // ctor


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
/* virtual public */
PrfInfo::~PrfInfo()
{
    if ( m_pProfilerInfo != NULL )
        m_pProfilerInfo->Release();
       
    if ( m_pProfilerInfo2 != NULL )
        m_pProfilerInfo2->Release();

    if ( m_pProfilerInfo3 != NULL )
        m_pProfilerInfo3->Release();
       
    // clean up tables      
    if ( m_pClassTable != NULL )
    {    
        delete m_pClassTable;
        m_pClassTable = NULL;
    }
    

    if ( m_pThreadTable != NULL )
    {    
        delete m_pThreadTable;
        m_pThreadTable = NULL;
    }
    

    if ( m_pFunctionTable != NULL )
    {    
        delete m_pFunctionTable;
        m_pFunctionTable = NULL;
    }


    if ( m_pModuleTable != NULL )
    {    
        delete m_pModuleTable;
        m_pModuleTable = NULL;
    }

    if ( m_pStackTraceTable != NULL )
    {
        delete m_pStackTraceTable;
        m_pStackTraceTable = NULL;
    }

} // dtor 


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
/* throws BaseException */
void PrfInfo::AddThread( ThreadID threadID )
{    
    HRESULT hr;
    ThreadID myThreadID;


    hr = m_pProfilerInfo->GetCurrentThreadID( &myThreadID );
    if ( SUCCEEDED( hr ) )
    {        
        if ( threadID == myThreadID )
        {
            ThreadInfo *pThreadInfo;
            
            
            pThreadInfo = new ThreadInfo( threadID );
            if ( pThreadInfo != NULL )
            {
                hr = m_pProfilerInfo->GetThreadInfo( pThreadInfo->m_id, &(pThreadInfo->m_win32ThreadID) );
                if ( SUCCEEDED( hr ) )
                    m_pThreadTable->AddEntry( pThreadInfo, threadID );
                else
                {
                    delete pThreadInfo;
                    _THROW_EXCEPTION( "ICorProfilerInfo::GetThreadInfo() FAILED" )
                }
            }
            else
                _THROW_EXCEPTION( "Allocation for ThreadInfo Object FAILED" )
        }
        else
            _THROW_EXCEPTION( "Thread ID's do not match FAILED" )
    }
    else
        _THROW_EXCEPTION( "ICorProfilerInfo::GetCurrentThreadID() FAILED" )
                    
} // PrfInfo::AddThread


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
/* throws BaseException */
void PrfInfo::RemoveThread( ThreadID threadID )
{    
    if ( threadID != NULL )
    {
        ThreadInfo *pThreadInfo;

        
        pThreadInfo = m_pThreadTable->Lookup( threadID );
        if ( pThreadInfo != NULL )
            m_pThreadTable->DelEntry( threadID );

        else
            _THROW_EXCEPTION( "Thread was not found in the Thread Table" )
    }
    else
        _THROW_EXCEPTION( "ThreadID is NULL" )
                    
} // PrfInfo::RemoveThread


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
/* throws BaseException */
void PrfInfo::AddFunction( FunctionID functionID, SIZE_T internalID )
{    
    if ( functionID != NULL )
    {
        FunctionInfo *pFunctionInfo;
        

        pFunctionInfo = m_pFunctionTable->Lookup( functionID );
        if ( pFunctionInfo == NULL )
        {
            pFunctionInfo = new FunctionInfo( functionID, internalID );
            if ( pFunctionInfo != NULL )
            {
                try
                {
                    _GetFunctionSig( &pFunctionInfo );
                    m_pFunctionTable->AddEntry( pFunctionInfo, functionID );
                }
                catch ( BaseException *exception )    
                {
                    delete pFunctionInfo;
                    throw;            
                }
            }        
            else
                _THROW_EXCEPTION( "Allocation for FunctionInfo Object FAILED" )
        }         
    }
    else
        _THROW_EXCEPTION( "FunctionID is NULL" )
          
} // PrfInfo::AddFunction


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
/* throws BaseException */
void PrfInfo::RemoveFunction( FunctionID functionID )
{    
    if ( functionID != NULL )
    {
        FunctionInfo *pFunctionInfo;
        

        pFunctionInfo = m_pFunctionTable->Lookup( functionID );
        if ( pFunctionInfo != NULL )
            m_pFunctionTable->DelEntry( functionID );
      
        else
            _THROW_EXCEPTION( "Function was not found in the Function Table" )
    }
    else
        _THROW_EXCEPTION( "FunctionID is NULL" )
            
} // PrfInfo::RemoveFunction


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
/* throws BaseException */
void PrfInfo::AddModule( ModuleID moduleID, SIZE_T internalID )
{    
    if ( moduleID != NULL )
    {
        ModuleInfo *pModuleInfo;
        

        pModuleInfo = m_pModuleTable->Lookup( moduleID );
        if ( pModuleInfo == NULL )
        {
            pModuleInfo = new ModuleInfo( moduleID, internalID );
            if ( pModuleInfo != NULL )
            {
                HRESULT hr;
                ULONG dummy;
                
                                
                hr = m_pProfilerInfo->GetModuleInfo( moduleID,
                                                     &(pModuleInfo->m_loadAddress),
                                                     MAX_LENGTH,
                                                     &dummy, 
                                                     pModuleInfo->m_moduleName,
                                                     NULL );
                if ( SUCCEEDED( hr ) )
                {
                    m_pModuleTable->AddEntry( pModuleInfo, moduleID );
                }
                else
                {
                    delete pModuleInfo;
                    _THROW_EXCEPTION( "ICorProfilerInfo::GetModuleInfo() FAILED" )
                }
            }        
            else
                _THROW_EXCEPTION( "Allocation for ModuleInfo Object FAILED" )
        }     
    }
    else
        _THROW_EXCEPTION( "ModuleID is NULL" )
          
} // PrfInfo::AddModule


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
/* throws BaseException */
void PrfInfo::RemoveModule( ModuleID moduleID )
{    
    if ( moduleID != NULL )
    {
        ModuleInfo *pModuleInfo;
        

        pModuleInfo = m_pModuleTable->Lookup( moduleID );
        if ( pModuleInfo != NULL )
            m_pModuleTable->DelEntry( moduleID );
      
        else
            _THROW_EXCEPTION( "Module was not found in the Module Table" )
    }
    else
        _THROW_EXCEPTION( "ModuleID is NULL" )

} // PrfInfo::RemoveModule


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
void PrfInfo::UpdateOSThreadID( ThreadID managedThreadID, DWORD osThreadID )
{
    ThreadInfo *pThreadInfo;


    pThreadInfo = m_pThreadTable->Lookup( managedThreadID );
    if ( pThreadInfo != NULL )
        pThreadInfo->m_win32ThreadID = osThreadID;
   
    else
        _THROW_EXCEPTION( "Thread does not exist in the thread table" )
                              
} // PrfInfo::UpdateOSThreadID

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
void PrfInfo::UpdateUnwindStack( FunctionID *functionID, StackAction action )
{
    HRESULT hr;
    ThreadID threadID;


    hr = m_pProfilerInfo->GetCurrentThreadID( &threadID );
    if ( SUCCEEDED(hr) )
    {
        ThreadInfo *pThreadInfo = m_pThreadTable->Lookup( threadID );

        if ( pThreadInfo == NULL )
        {
            // Sadly, sometimes we see threads that weren't announced via
            // CreateThread. Add them now...
            AddThread( threadID );
            pThreadInfo = m_pThreadTable->Lookup( threadID );
        }

        if ( pThreadInfo != NULL )
        {
            switch ( action )
            {
                case PUSH:
                    (pThreadInfo->m_pLatestUnwoundFunction)->Push( *functionID );
                    break;

                case POP:
                    *functionID = (pThreadInfo->m_pLatestUnwoundFunction)->Pop();
                    break;
            }
        }
        else 
            _THROW_EXCEPTION( "Thread Structure was not found in the thread list" )
    }
    else
        _THROW_EXCEPTION( "ICorProfilerInfo::GetCurrentThreadID() FAILED" )
          
} // PrfInfo::UpdateUnwindStack


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
void PrfInfo::UpdateCallStack( FunctionID functionID, StackAction action )
{
    HRESULT hr = S_OK;
    ThreadID threadID;


    hr = m_pProfilerInfo->GetCurrentThreadID(&threadID);
    if ( SUCCEEDED(hr) )
    {
        ThreadInfo *pThreadInfo = m_pThreadTable->Lookup( threadID );

        if ( pThreadInfo == NULL )
        {
            // Sadly, sometimes we see threads that weren't announced via
            // CreateThread. Add them now...
            AddThread( threadID );
            pThreadInfo = m_pThreadTable->Lookup( threadID );
        }

        if ( pThreadInfo != NULL )
        {

            switch ( action )
            {
                case PUSH:
                    (pThreadInfo->m_pThreadCallStack)->Push( functionID );
                    break;

                case POP:
                    (pThreadInfo->m_pThreadCallStack)->Pop();
                    break;
            }
        }
        else 
            _THROW_EXCEPTION( "Thread Structure was not found in the thread list" )
    }
    else
        _THROW_EXCEPTION( "ICorProfilerInfo::GetCurrentThreadID() FAILED" )


} // PrfInfo::UpdateCallStack


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
HRESULT PrfInfo::GetNameFromClassID( ClassID classID, __out WCHAR className[] )
{
    HRESULT hr = E_FAIL;
    
    
    if ( m_pProfilerInfo != NULL )
    {
        ModuleID moduleID;
        mdTypeDef classToken;

        
        hr = m_pProfilerInfo->GetClassIDInfo( classID, 
                                              &moduleID,  
                                              &classToken );                                                                                                                                              
        if ( SUCCEEDED( hr ) )
        {             
            IMetaDataImport *pMDImport = NULL;
                
            
            hr = m_pProfilerInfo->GetModuleMetaData( moduleID, 
                                                     (ofRead | ofWrite),
                                                     IID_IMetaDataImport, 
                                                     (IUnknown **)&pMDImport );
            if ( SUCCEEDED( hr ) )
            {
                if ( classToken != mdTypeDefNil )
                {
                    ClassID *classTypeArgs = NULL;
                    ULONG32 classTypeArgCount = 0;
#ifdef mdGenericPar
                    if (m_pProfilerInfo2 != NULL)
                    {
                        hr = m_pProfilerInfo2->GetClassIDInfo2(classID,
                                                               NULL,
                                                               NULL,
                                                               NULL,
                                                               0,
                                                               &classTypeArgCount,
                                                               NULL);
                        
                        if (SUCCEEDED(hr) && classTypeArgCount > 0)
                        {
                            classTypeArgs = (ClassID *)_alloca(classTypeArgCount*sizeof(classTypeArgs[0]));

                            hr = m_pProfilerInfo2->GetClassIDInfo2(classID,
                                                                   NULL,
                                                                   NULL,
                                                                   NULL,
                                                                   classTypeArgCount,
                                                                   &classTypeArgCount,
                                                                   classTypeArgs);
                        }
                        if (!SUCCEEDED(hr))
                            classTypeArgs = NULL;
                    }
#endif // mdGenericPar
                    DWORD dwTypeDefFlags = 0;
                    ULONG genericArgCount = 0;
                    hr = GetClassName(pMDImport, classToken, className, classTypeArgs, &genericArgCount);
                    if ( FAILED( hr ) )
                        Failure( "_GetClassNameHelper() FAILED" );
                }
                else
                    DEBUG_OUT( ("The class token is mdTypeDefNil, class does NOT have MetaData info") );


                pMDImport->Release ();
            }
            else
            {
//                Failure( "IProfilerInfo::GetModuleMetaData() => IMetaDataImport FAILED" );
                wcscpy_s(className, MAX_LENGTH, L"???");
                hr = S_OK;
            }
        }
        else    
            Failure( "ICorProfilerInfo::GetClassIDInfo() FAILED" );
    }
    else
        Failure( "ICorProfilerInfo Interface has NOT been Initialized" );


    return hr;

} // PrfHelper::GetNameFromClassID


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
/* throws BaseException */
void PrfInfo::_GetFunctionSig( FunctionInfo **ppFunctionInfo )
{
    HRESULT hr;
    
    
    BOOL isStatic;
    ULONG argCount;
    WCHAR returnType[MAX_LENGTH];
    WCHAR functionName[MAX_LENGTH];
    WCHAR functionParameters[10 * MAX_LENGTH];


    //
    // init strings
    //
    returnType[0] = L'\0';
    functionName[0] = L'\0';
    functionParameters[0] = L'\0';
    (*ppFunctionInfo)->m_functionSig[0] = L'\0';

    // get the sig of the function and
    // use utilcode to get the parameters you want
    GetFunctionProperties( (*ppFunctionInfo)->m_id,
                           &isStatic,
                           &argCount,
                           returnType, 
                           ARRAY_LEN(returnType),
                           functionParameters,
                           ARRAY_LEN(functionParameters),
                           functionName,
                           ARRAY_LEN(functionName));

    const size_t sigLen = ARRAY_LEN((*ppFunctionInfo)->m_functionSig);
    _snwprintf_s( (*ppFunctionInfo)->m_functionSig,
                sigLen,
                sigLen-1,
                L"%s%s (%s)",
                (isStatic ? L"static " : L""),
                returnType,
                functionParameters );

    WCHAR *index = (*ppFunctionInfo)->m_functionSig;

    while ( *index != L'\0' )
    {
        if ( *index == '+' )
           *index = ' ';
        index++;
    }

    //
    // update the function name if it is not yet set
    //
    if ( wcsstr( (*ppFunctionInfo)->m_functionName, L"UNKNOWN" ) != NULL )
    {
        const size_t nameLen = ARRAY_LEN((*ppFunctionInfo)->m_functionName);

        wcsncpy_s((*ppFunctionInfo)->m_functionName, nameLen, functionName, nameLen-1);
    }

    
} // PrfInfo::_GetFunctionSig


static void StrAppend(__out_ecount(cchBuffer) char *buffer, const char *str, size_t cchBuffer)
{
    size_t bufLen = strlen(buffer) + 1;
    if (bufLen <= cchBuffer)
        strncat_s(buffer, cchBuffer, str, cchBuffer-bufLen);
}

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

void PrfInfo::AppendTypeArgName(ULONG argIndex, ClassID *actualClassTypeArgs, ClassID *actualMethodTypeArgs, BOOL methodFormalArg, __out_ecount(cchBuffer) char *buffer, size_t cchBuffer)
{
    char argName[MAX_LENGTH];

    argName[0] = '\0';

    ClassID classId = 0;
    if (methodFormalArg && actualMethodTypeArgs != NULL)
        classId = actualMethodTypeArgs[argIndex];
    if (!methodFormalArg && actualClassTypeArgs != NULL)
        classId = actualClassTypeArgs[argIndex];

    if (classId != 0)
    {
        WCHAR className[MAX_LENGTH];

        HRESULT hr = GetNameFromClassID(classId, className);
        if (SUCCEEDED(hr))
            _snprintf_s( argName, ARRAY_LEN(argName), ARRAY_LEN(argName)-1, "%S", className);
    }

    if (argName[0] == '\0')
    {
        char argStart = methodFormalArg ? 'M' : 'T';
        if (argIndex <= 6)
        {
            // the first 7 parameters are printed as M, N, O, P, Q, R, S 
            // or as T, U, V, W, X, Y, Z 
            sprintf_s( argName, ARRAY_LEN(argName), "%c", argIndex + argStart);
        }
        else
        {
            // everything after that as M7, M8, ... or T7, T8, ...
            sprintf_s( argName, ARRAY_LEN(argName), "%c%u", argStart, argIndex);
        }
    }

    StrAppend( buffer, argName, cchBuffer);
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
DECLSPEC
/* static public */
PCCOR_SIGNATURE PrfInfo::ParseElementType( IMetaDataImport *pMDImport,
                                           PCCOR_SIGNATURE signature,
                                           ClassID *classTypeArgs,
                                           ClassID *methodTypeArgs,
                                           __out_ecount(cchBuffer) char *buffer,
                                           size_t cchBuffer)
{   
    switch ( *signature++ ) 
    {   
        case ELEMENT_TYPE_VOID:
            StrAppend( buffer, "void", cchBuffer);   
            break;                  
        
        
        case ELEMENT_TYPE_BOOLEAN:  
            StrAppend( buffer, "bool", cchBuffer);   
            break;  
        
        
        case ELEMENT_TYPE_CHAR:
            StrAppend( buffer, "wchar", cchBuffer);  
            break;      
                    
        
        case ELEMENT_TYPE_I1:
            StrAppend( buffer, "int8", cchBuffer );   
            break;      
        
        
        case ELEMENT_TYPE_U1:
            StrAppend( buffer, "unsigned int8", cchBuffer );  
            break;      
        
        
        case ELEMENT_TYPE_I2:
            StrAppend( buffer, "int16", cchBuffer );  
            break;      
        
        
        case ELEMENT_TYPE_U2:
            StrAppend( buffer, "unsigned int16", cchBuffer ); 
            break;          
        
        
        case ELEMENT_TYPE_I4:
            StrAppend( buffer, "int32", cchBuffer );  
            break;
            
        
        case ELEMENT_TYPE_U4:
            StrAppend( buffer, "unsigned int32", cchBuffer ); 
            break;      
        
        
        case ELEMENT_TYPE_I8:
            StrAppend( buffer, "int64", cchBuffer );  
            break;      
        
        
        case ELEMENT_TYPE_U8:
            StrAppend( buffer, "unsigned int64", cchBuffer ); 
            break;      
        
        
        case ELEMENT_TYPE_R4:
            StrAppend( buffer, "float32", cchBuffer );    
            break;          
        
        
        case ELEMENT_TYPE_R8:
            StrAppend( buffer, "float64", cchBuffer );    
            break;      
        
        
        case ELEMENT_TYPE_U:
            StrAppend( buffer, "unsigned int_ptr", cchBuffer );   
            break;       
        
        
        case ELEMENT_TYPE_I:
            StrAppend( buffer, "int_ptr", cchBuffer );    
            break;            
        
        
        case ELEMENT_TYPE_OBJECT:
            StrAppend( buffer, "Object", cchBuffer ); 
            break;       
        
        
        case ELEMENT_TYPE_STRING:
            StrAppend( buffer, "String", cchBuffer ); 
            break;       
        
        
        case ELEMENT_TYPE_TYPEDBYREF:
            StrAppend( buffer, "refany", cchBuffer ); 
            break;                     

        case ELEMENT_TYPE_CLASS:    
        case ELEMENT_TYPE_VALUETYPE:
        case ELEMENT_TYPE_CMOD_REQD:
        case ELEMENT_TYPE_CMOD_OPT:
            {   
                mdToken token;  
                char classname[MAX_LENGTH];


                classname[0] = '\0';
                signature += CorSigUncompressToken( signature, &token ); 
                if ( TypeFromToken( token ) != mdtTypeRef )
                {
                    HRESULT hr;
                    WCHAR zName[MAX_LENGTH];
                    
                    
                    hr = pMDImport->GetTypeDefProps( token, 
                                                     zName,
                                                     MAX_LENGTH,
                                                     NULL,
                                                     NULL,
                                                     NULL );
                    if ( SUCCEEDED( hr ) )
                    {
                        size_t convertedChars;
                        wcstombs_s( &convertedChars, classname, ARRAY_LEN(classname), zName, ARRAY_LEN(zName) );
                    }
                }
                    
                StrAppend( buffer, classname, cchBuffer );        
            }
            break;  
        
        
        case ELEMENT_TYPE_SZARRAY:   
            signature = ParseElementType( pMDImport, signature, classTypeArgs, methodTypeArgs, buffer, cchBuffer ); 
            StrAppend( buffer, "[]", cchBuffer );
            break;      
        
        
        case ELEMENT_TYPE_ARRAY:    
            {   
                ULONG rank;
                

                signature = ParseElementType( pMDImport, signature, classTypeArgs, methodTypeArgs, buffer, cchBuffer );                 
                rank = CorSigUncompressData( signature );  
                
                // The second condition is to guard against overflow bugs & shut up PREFAST
                if ( rank == 0 || rank >= 65536 ) 
                    StrAppend( buffer, "[?]", cchBuffer );

                else 
                {
                    ULONG *lower;   
                    ULONG *sizes;   
                    ULONG numsizes; 
                    ULONG arraysize = (sizeof ( ULONG ) * 2 * rank);
                    
                                         
                    lower = (ULONG *)_alloca( arraysize );                                                        
                    memset( lower, 0, arraysize ); 
                    sizes = &lower[rank];

                    numsizes = CorSigUncompressData( signature );   
                    if ( numsizes <= rank )
                    {
                        ULONG numlower;
                        ULONG i;                        
                        
                        for (i = 0; i < numsizes; i++ )  
                            sizes[i] = CorSigUncompressData( signature );   
                        
                        
                        numlower = CorSigUncompressData( signature );   
                        if ( numlower <= rank )
                        {
                            for ( i = 0; i < numlower; i++) 
                                lower[i] = CorSigUncompressData( signature ); 
                            
                            
                            StrAppend( buffer, "[", cchBuffer );  
                            for ( i = 0; i < rank; i++ )    
                            {   
                                if ( (sizes[i] != 0) && (lower[i] != 0) )   
                                {   
                                    char sizeBuffer[100];
                                    if ( lower[i] == 0 )
                                        sprintf_s ( sizeBuffer, ARRAY_LEN(sizeBuffer), "%d", sizes[i] ); 
                                    else    
                                    {   
                                        sprintf_s( sizeBuffer, ARRAY_LEN(sizeBuffer), "%d...", lower[i] );  
                                        
                                        if ( sizes[i] != 0 )    
                                            sprintf_s( sizeBuffer, ARRAY_LEN(sizeBuffer), "%d...%d", lower[i], (lower[i] + sizes[i] + 1) ); 
                                    }   
                                    StrAppend( buffer, sizeBuffer, cchBuffer );    
                                }
                                    
                                if ( i < (rank - 1) ) 
                                    StrAppend( buffer, ",", cchBuffer );  
                            }   
                            
                            StrAppend( buffer, "]", cchBuffer );  
                        }                       
                    }
                }
            } 
            break;  

        
        case ELEMENT_TYPE_PINNED:
            signature = ParseElementType( pMDImport, signature, classTypeArgs, methodTypeArgs, buffer, cchBuffer ); 
            StrAppend( buffer, "pinned", cchBuffer ); 
            break;  
         
        
        case ELEMENT_TYPE_PTR:   
            signature = ParseElementType( pMDImport, signature, classTypeArgs, methodTypeArgs, buffer, cchBuffer ); 
            StrAppend( buffer, "*", cchBuffer );  
            break;   
        
        
        case ELEMENT_TYPE_BYREF:   
            signature = ParseElementType( pMDImport, signature, classTypeArgs, methodTypeArgs, buffer, cchBuffer ); 
            StrAppend( buffer, "&", cchBuffer );  
            break;              

#ifdef mdGenericPar
		case ELEMENT_TYPE_VAR:
            AppendTypeArgName(CorSigUncompressData(signature), classTypeArgs, methodTypeArgs, FALSE, buffer, cchBuffer);
            break;

        case ELEMENT_TYPE_MVAR:
            AppendTypeArgName(CorSigUncompressData(signature), classTypeArgs, methodTypeArgs, TRUE, buffer, cchBuffer);
            break;
#endif // mdGenericPar

        default:    
        case ELEMENT_TYPE_END:  
        case ELEMENT_TYPE_SENTINEL: 
            StrAppend( buffer, "<UNKNOWN>", cchBuffer );  
            break;                                                              
                            
    } // switch 
    
    
    return signature;

} // PrfInfo::ParseElementType


DECLSPEC
/* static public */
HRESULT PrfInfo::GetClassName(IMetaDataImport *pMDImport, mdToken classToken, __out WCHAR className[], ClassID *classTypeArgs, ULONG *totalGenericArgCount)
{
    DWORD dwTypeDefFlags = 0;
    HRESULT hr = S_OK;
    hr = pMDImport->GetTypeDefProps( classToken, 
                                     className, 
                                     MAX_LENGTH,
                                     NULL, 
                                     &dwTypeDefFlags, 
                                     NULL ); 
    if ( FAILED( hr ) )
    {
        return hr;
    }
    *totalGenericArgCount = 0;
    if (IsTdNested(dwTypeDefFlags))
    {
//      printf("%S is a nested class\n", className);
        mdToken enclosingClass = mdTokenNil;
        hr = pMDImport->GetNestedClassProps(classToken, &enclosingClass);
        if ( FAILED( hr ) )
        {
            return hr;
        }
//      printf("Enclosing class for %S is %d\n", className, enclosingClass);
        hr = GetClassName(pMDImport, enclosingClass, className, classTypeArgs, totalGenericArgCount);
//      printf("Enclosing class name %S\n", className);
        if (FAILED(hr))
            return hr;
        size_t length = wcslen(className);
        if (length + 2 < MAX_LENGTH)
        {
            className[length++] = '.';
            hr = pMDImport->GetTypeDefProps( classToken, 
                                            className + length, 
                                            (ULONG)(MAX_LENGTH - length),
                                            NULL, 
                                            NULL, 
                                            NULL );
            if ( FAILED( hr ) )
            {
                return hr;
            }
//          printf("%S is a nested class\n", className);
        }
    }

    WCHAR *backTick = wcschr(className, L'`');
    if (backTick != NULL)
    {
        *backTick = L'\0';
        ULONG genericArgCount = wcstoul(backTick+1, NULL, 10);

        if (genericArgCount >0)
        {
            char typeArgText[MAX_LENGTH];
            typeArgText[0] = '\0';

            StrAppend(typeArgText, "<", MAX_LENGTH);
            for (ULONG i = *totalGenericArgCount; i < *totalGenericArgCount + genericArgCount; i++)
            {
                if (i != *totalGenericArgCount)
                    StrAppend(typeArgText, ",", MAX_LENGTH);
                AppendTypeArgName(i, classTypeArgs, NULL, FALSE, typeArgText, MAX_LENGTH);
            }
            StrAppend(typeArgText, ">", MAX_LENGTH);

            *totalGenericArgCount += genericArgCount;
    
            _snwprintf_s(className, MAX_LENGTH, MAX_LENGTH-1, L"%s%S", className, typeArgText);
        }
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
DECLSPEC
/* static public */
HRESULT PrfInfo::GetFunctionProperties( FunctionID functionID,
                                        BOOL *isStatic,
                                        ULONG *argCount,
                                        __out_ecount(returnTypeStrLen) WCHAR *returnTypeStr, 
                                        size_t returnTypeStrLen,
                                        __out_ecount(functionParametersLen) WCHAR *functionParameters,
                                        size_t functionParametersLen,
                                        __out_ecount(functionNameLen) WCHAR *functionName,
                                        size_t functionNameLen )
{
    HRESULT hr = E_FAIL; // assume success
            
    _ASSERTE(returnTypeStr != NULL && returnTypeStrLen > 0);
    returnTypeStr[0] = 0;

    _ASSERTE(functionParameters != NULL && functionParametersLen > 0);
    functionParameters[0] = 0;

    _ASSERTE(functionName != NULL && functionNameLen > 0);
    functionName[0] = 0;

    if ( functionID != NULL )
    {
        mdToken funcToken = mdTypeDefNil;
        IMetaDataImport *pMDImport = NULL;      
        WCHAR funName[MAX_LENGTH] = L"UNKNOWN";
                
                
        
        //
        // Get the MetadataImport interface and the metadata token 
        //
        hr = m_pProfilerInfo->GetTokenAndMetaDataFromFunction( functionID, 
                                                               IID_IMetaDataImport, 
                                                               (IUnknown **)&pMDImport,
                                                               &funcToken );
        if ( SUCCEEDED( hr ) )
        {
            mdTypeDef classToken = mdTypeDefNil;
            DWORD methodAttr = 0;
            PCCOR_SIGNATURE sigBlob = NULL;

            hr = pMDImport->GetMethodProps( funcToken,
                                            &classToken,
                                            funName,
                                            MAX_LENGTH,
                                            0,
                                            &methodAttr,
                                            &sigBlob,
                                            NULL,
                                            NULL, 
                                            NULL );
            if ( SUCCEEDED( hr ) )
            {
                WCHAR className[MAX_LENGTH] = L"UNKNOWN";
                ClassID classId =0;

                if (m_pProfilerInfo2 != NULL)
                {
                    hr = m_pProfilerInfo2->GetFunctionInfo2(functionID,
                                                            0,
                                                            &classId,
                                                            NULL,
                                                            NULL,
                                                            0,
                                                            NULL,
                                                            NULL);
                    if (!SUCCEEDED(hr))
                        classId = 0;
                }

                if (classId == 0)
                {
                    hr = m_pProfilerInfo->GetFunctionInfo(functionID,
                                                          &classId,
                                                          NULL,
                                                          NULL);
                }
                if (SUCCEEDED(hr) && classId != 0)
                {
                    hr = GetNameFromClassID(classId, className);
                }
                else if (classToken != mdTypeDefNil)
                {
                    ULONG classGenericArgCount = 0;
                    hr = GetClassName(pMDImport, classToken, className, NULL, &classGenericArgCount);
                }
                _snwprintf_s( functionName, functionNameLen, functionNameLen-1, L"%s::%s", className, funName );                    


                ULONG callConv;


                //
                // Is the method static ?
                //
                (*isStatic) = (BOOL)((methodAttr & mdStatic) != 0);

                //
                // Make sure we have a method signature.
                //
                char buffer[2 * MAX_LENGTH];
                
                
                sigBlob += CorSigUncompressData( sigBlob, &callConv );
                if ( callConv != IMAGE_CEE_CS_CALLCONV_FIELD )
                {
                    static char* callConvNames[8] = 
                    {   
                        "", 
                        "unmanaged cdecl ", 
                        "unmanaged stdcall ",  
                        "unmanaged thiscall ", 
                        "unmanaged fastcall ", 
                        "vararg ",  
                        "<error> "  
                        "<error> "  
                    };  
                    buffer[0] = '\0';
                    if ( (callConv & 7) != 0 )
                        sprintf_s( buffer, ARRAY_LEN(buffer), "%s ", callConvNames[callConv & 7]);   
                    
                    ULONG genericArgCount = 0;
                    ClassID *methodTypeArgs = NULL;
                    UINT32 methodTypeArgCount = 0;
                    ClassID *classTypeArgs = NULL;
                    ULONG32 classTypeArgCount = 0;
#ifdef mdGenericPar
                    if ((callConv & IMAGE_CEE_CS_CALLCONV_GENERIC) != 0)
                    {
                        //
                        // Grab the generic type argument count
                        //
                        sigBlob += CorSigUncompressData( sigBlob, &genericArgCount );
                    }
                    if (m_pProfilerInfo2 != NULL)
                    {
                        methodTypeArgs = (ClassID *)_alloca(genericArgCount*sizeof(methodTypeArgs[0]));

                        hr = m_pProfilerInfo2->GetFunctionInfo2( functionID,
                                                                 0,
                                                                 &classId,
                                                                 NULL,
                                                                 NULL,
                                                                 genericArgCount,
                                                                 &methodTypeArgCount,
                                                                 methodTypeArgs);

                        _ASSERTE(!SUCCEEDED(hr) || genericArgCount == methodTypeArgCount);
                        if (!SUCCEEDED(hr))
                            methodTypeArgs = NULL;
                        else
                        {
                            hr = m_pProfilerInfo2->GetClassIDInfo2(classId,
                                                                   NULL,
                                                                   NULL,
                                                                   NULL,
                                                                   0,
                                                                   &classTypeArgCount,
                                                                   NULL);
                        }
                        
                        if (SUCCEEDED(hr) && classTypeArgCount > 0)
                        {
                            classTypeArgs = (ClassID *)_alloca(classTypeArgCount*sizeof(classTypeArgs[0]));

                            hr = m_pProfilerInfo2->GetClassIDInfo2(classId,
                                                                   NULL,
                                                                   NULL,
                                                                   NULL,
                                                                   classTypeArgCount,
                                                                   &classTypeArgCount,
                                                                   classTypeArgs);
                        }
                        if (!SUCCEEDED(hr))
                            classTypeArgs = NULL;
                        hr = S_OK;
                    }
#endif // mdGenericPar
                    //
                    // Grab the argument count
                    //
                    sigBlob += CorSigUncompressData( sigBlob, argCount );

                    //
                    // Get the return type
                    //
                    sigBlob = ParseElementType( pMDImport, sigBlob, classTypeArgs, methodTypeArgs, buffer, ARRAY_LEN(buffer) );

                    // The second condition is to guard against overflow bugs & shut up PREFAST
                    if (genericArgCount != 0 && genericArgCount < 65536)
                    {
                        StrAppend(buffer, " <", ARRAY_LEN(buffer));
                        for (ULONG i = 0; i < genericArgCount; i++)
                        {
                            if (i != 0)
                                StrAppend(buffer, ",", ARRAY_LEN(buffer));
                            AppendTypeArgName(i, classTypeArgs, methodTypeArgs, TRUE, buffer, ARRAY_LEN(buffer));
                        }
                        StrAppend(buffer, ">", ARRAY_LEN(buffer));
                    }
                    //
                    // if the return typ returned back empty, write void
                    //
                    if ( buffer[0] == '\0' )
                        sprintf_s( buffer, ARRAY_LEN(buffer), "void" );

                    _snwprintf_s( returnTypeStr, returnTypeStrLen, returnTypeStrLen-1, L"%S",buffer );
                
                    //
                    // Get the parameters
                    //                              
                    for ( ULONG i = 0; 
                          (SUCCEEDED( hr ) && (sigBlob != NULL) && (i < (*argCount))); 
                          i++ )
                    {
                        buffer[0] = '\0';

                        sigBlob = ParseElementType( pMDImport, sigBlob, classTypeArgs, methodTypeArgs, buffer, ARRAY_LEN(buffer)-1 );
                        buffer[ARRAY_LEN(buffer)-1] = '\0';

                        if ( i == 0 ) 
                        {
                            _snwprintf_s( functionParameters, functionParametersLen, functionParametersLen-1, L"%S", buffer );
                        }
                        else if ( sigBlob != NULL )
                        {
                            _snwprintf_s( functionParameters, functionParametersLen, functionParametersLen-1, L"%s+%S", functionParameters, buffer );
                        }
                    
                        else
                            hr = E_FAIL;
                    }
                }
                else
                {
                    //
                    // Get the return type
                    //
                    buffer[0] = '\0';
                    sigBlob = ParseElementType( pMDImport, sigBlob, NULL, NULL, buffer, ARRAY_LEN(buffer)-1 );
                    buffer[ARRAY_LEN(buffer)-1] = L'\0';
                    _snwprintf_s( returnTypeStr, returnTypeStrLen, returnTypeStrLen-1, L"%s %S",returnTypeStr, buffer );
                }
            } 

            pMDImport->Release();
        }
    } 
    //
    // This corresponds to an unmanaged frame
    //
    else
    {
        //
        // Set up return parameters
        //
        hr = S_OK;
        *argCount = 0;
        *isStatic = FALSE;
        wcscpy_s(functionName, functionNameLen, L"UNMANAGED FRAME" );   
    }

    
    return hr;

} // PrfInfo::GetFunctionProperties


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
void PrfInfo::Failure( const char *message )
{
    if ( message == NULL )     
        message = "**** SEVERE FAILURE: TURNING OFF APPLICABLE PROFILING EVENTS ****";  
    
    
    //
    // Display the error message and discontinue monitoring CLR events, except the 
    // IMMUTABLE ones. Turning off the IMMUTABLE events can cause crashes. The only
    // place that we can safely enable or disable immutable events is the Initialize
    // callback.
    //
    TEXT_OUTLN( message )
    m_pProfilerInfo->SetEventMask( (m_dwEventMask & (DWORD)COR_PRF_MONITOR_IMMUTABLE) );    
                        
} // PrfInfo::Failure


// end of file
