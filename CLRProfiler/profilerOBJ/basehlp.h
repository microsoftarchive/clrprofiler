// ==++==
// 
//   Copyright (c) Microsoft Corporation.  All rights reserved.
// 
// ==--==
/***************************************************************************************
 * File:
 *  basehlp.h
 *
 * Description:
 *  
 *
 *
 ***************************************************************************************/
#ifndef __BASEHLP_H__
#define __BASEHLP_H__

#include "basehdr.h"


//
// exception macro
//
#define _THROW_EXCEPTION( message ) \
{ \
    _ASSERTE(!(message));  \
    throw new BaseException( message ); \
} \


/***************************************************************************************
 ********************                                               ********************
 ********************           BaseException Declaration           ********************
 ********************                                               ********************
 ***************************************************************************************/
class DECLSPEC BaseException
{
    public:
        
        BaseException( const char *reason );
        virtual ~BaseException();
        

        virtual void ReportFailure();


    private:

        char *m_reason;
        
}; // BaseException


/***************************************************************************************
 ********************                                               ********************
 ********************            Synchronize Declaration            ********************
 ********************                                               ********************
 ***************************************************************************************/
class DECLSPEC Synchronize 
{
    public:
    
        Synchronize( CRITICAL_SECTION &criticalSection );
        ~Synchronize();
        
        
    private:
    
        CRITICAL_SECTION &m_block;
        
}; // Synchronize


/***************************************************************************************
 ********************                                               ********************
 ********************          BASEHELPER Declaration               ********************
 ********************                                               ********************
 ***************************************************************************************/
class DECLSPEC BASEHELPER
{   
    public:
    
        //
        // debug dumper
        //
        static void DDebug( const char *format, ... );


        //
        // unconditional dumper
        //
        static void Display( const char *format, ... );


        //
        // obtain the value of the given environment var
        //
        static DWORD FetchEnvironment( const char *environment );
        
        
        //
        // logs to a specified file
        // 
        static void LogToFile( const char *format, ... );


        //
        // obtain numeric value of environment value
        //               
        static DWORD GetEnvVarValue( const char *value );


        //
        // convert a string to a number
        //
        static int String2Number( const char *number );
        
        //
        // print indentation 
        //                                        
        static void Indent( DWORD indent );
        
        //
        // decodes a type from the signature.
        // the type returned will be, depending on the last parameter, 
        // either the outermost type, (e.g. ARRAY for an array of I4s)
        // or the innermost (I4 in the example above),
        //
        static ULONG GetElementType( PCCOR_SIGNATURE pSignature, 
                                    CorElementType *pType, 
                                    BOOL bDeepParse = FALSE );


        //
        // helper function for decoding arrays
        //
        static ULONG ProcessArray( PCCOR_SIGNATURE pSignature, CorElementType *pType );


        //
        // helper function for decoding FNPTRs (NOT IMPL)
        //
        static ULONG ProcessMethodDefRef( PCCOR_SIGNATURE pSignature, CorElementType *pType );


}; // BASEHELPER

#endif // __BASEHLP_H__

// End of File
