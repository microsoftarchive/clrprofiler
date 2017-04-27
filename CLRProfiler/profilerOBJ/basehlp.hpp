// ==++==
// 
//   Copyright (c) Microsoft Corporation.  All rights reserved.
// 
// ==--==
/****************************************************************************************
 * File:
 *  basehlp.hpp
 *
 * Description:
 *  
 *
 *
 ***************************************************************************************/

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

/***************************************************************************************
 ********************                                               ********************
 ********************          BaseException Implementation         ********************
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

DECLSPEC
/* public */
BaseException::BaseException( const char *reason ) :
    m_reason( NULL )
{
    SIZE_T length = strlen( reason );
    
    
    m_reason = new char[(length + 1)];
    if (m_reason != NULL)
        strcpy_s( m_reason, length + 1, reason );
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
DECLSPEC
/* virtual public */
BaseException::~BaseException() 
{
    if ( m_reason != NULL )
        delete[] m_reason;

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
DECLSPEC
/* virtual public */
void BaseException::ReportFailure()
{
    TEXT_OUTLN( m_reason );
    
} // BaseException::ReportFailure


/***************************************************************************************
 ********************                                               ********************
 ********************            Synchronize Implementation         ********************
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
DECLSPEC
/* public */
Synchronize::Synchronize( CRITICAL_SECTION &criticalSection ) : 
    m_block( criticalSection )
{
    EnterCriticalSection( &m_block );
    
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
DECLSPEC
/* public */
Synchronize::~Synchronize()
{
    LeaveCriticalSection( &m_block );

} // dtor


/***************************************************************************************
 ********************                                               ********************
 ********************            BASEHELPER Implementation          ********************
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
DECLSPEC
/* static public */
DWORD BASEHELPER::FetchEnvironment( const char *environment )
{
    DWORD retVal = -1;
    char buffer[MAX_LENGTH];
    
    
    if ( GetEnvironmentVariableA( environment, buffer, MAX_LENGTH ) > 0 )
        retVal = BASEHELPER::GetEnvVarValue( buffer );
                
    
    return retVal;

} // BASEHELPER::FetchEnvironemnt


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
void BASEHELPER::DDebug( const char *format, ... )
{

    static DWORD debugShow = -1;

    if (debugShow == -1)
        debugShow = BASEHELPER::FetchEnvironment( DEBUG_ENVIRONMENT );

    if ( (debugShow == 2) || (debugShow == 3) ) 
    {
        va_list args;
        DWORD dwLength;
        char buffer[MAX_LENGTH];
   

        va_start( args, format );    
        dwLength = vsprintf_s( buffer, ARRAY_LEN(buffer), format, args );

        printf( "%s\n", buffer );
    }
   
} // BASEHELPER::DDebug


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
void BASEHELPER::Display( const char *format, ... )
{
    va_list args;
    DWORD dwLength;
    char buffer[MAX_LENGTH];


    va_start( args, format );    
    dwLength = vsprintf_s( buffer, ARRAY_LEN(buffer), format, args );

    printf( "%s\n", buffer );       
   
} // BASEHELPER::Display


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
void BASEHELPER::LogToFile( const char *format, ... )
{
    va_list args;

    static DWORD dwVarValue = -1;

    if (dwVarValue == -1)
        dwVarValue = BASEHELPER::FetchEnvironment( LOG_ENVIRONMENT );
    
    va_start( args, format );        
    switch ( dwVarValue )
    {
        case 0x00:
        case 0xFFFFFFFF:
             vprintf( format, args );
             break;


        case 0xFF:
            break;


        default:
            {
                static int count = 0;
                static CRITICAL_SECTION criticalSection = { 0 };

                
                if ( count++ == 0 )
                    InitializeCriticalSectionEx( &criticalSection, 10000, 0 /*flags*/ );
                
                
                {
                    FILE *stream;
                    Synchronize guard( criticalSection );
                

                    //
                    // first time create otherwise append
                    //
                    char *flags = (count == 1) ? "w" : "a+";
                    if ( fopen_s( &stream, "output.log", flags ) != 0 && stream !=NULL)
                    {
                        vfprintf( stream, format, args );
                        fflush( stream );
                        fclose( stream );
                    }
                    else
                       TEXT_OUTLN( "Unable to open log file" )
                }
            }            
            break;
            
    } // switch

} // BASEHELPER::LogToFile


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
int BASEHELPER::String2Number( const char *number )
{
    WCHAR ch;
    int base; 
    int iIndex = 1;
    BOOL errorOccurred = FALSE;


    // check to see if this is a valid number
    // if the first digit is '0', then this is 
    // a hex or octal number
    if ( number[0] == '0' )
    {
        //
        // hex
        //
        if ( (number[1] == 'x') || (number[1] == 'X') )
        {
            iIndex++;
            
            base = 16;
            while ( (errorOccurred == FALSE) &&
                    ((ch = number[iIndex++]) != '\0') )
            {   
                if ( ((ch >= '0') && (ch <= '9'))  ||
                     ((ch >= 'a') && (ch <= 'f'))  ||
                     ((ch >= 'A') && (ch <= 'F')) )
                {
                    continue;
                }
                
                errorOccurred = TRUE;
            }
        }
        //
        // octal
        //
        else
        {
            base = 8;
            while ( (errorOccurred == FALSE) &&
                    ((ch = number[iIndex++]) != '\0') )
            {   
                if ( (ch >= '0') && (ch <= '7') )
                    continue;
                
                
                errorOccurred = TRUE;
            }
        }
    }
    //
    // decimal
    //
    else
    {
        base = 10;
        while  ( (errorOccurred == FALSE) &&
                 ((ch = number[iIndex++]) != '\0') )
        {   
            if ( (ch >= '0') && (ch <= '9') )
                continue;
            
            
            errorOccurred = TRUE;
        }
    }
    
    
    return ((errorOccurred == TRUE) ? -1 : base);

} // BASEHELPER::String2Number


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
DWORD BASEHELPER::GetEnvVarValue( const char *value )
{   
    DWORD retValue = -1;
    int base = BASEHELPER::String2Number( value );


    if ( base != -1 )
        retValue = (DWORD)strtoul( value, NULL, base );


    return retValue;

} // BASEHELPER::GetEnvVarValue


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
void BASEHELPER::Indent( DWORD indent )
{
    for ( DWORD i = 0; i < indent; i++ )
        LOG_TO_FILE( ("   ") )

} // BASEHELPER::Indent


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
ULONG BASEHELPER::GetElementType( PCCOR_SIGNATURE pSignature, CorElementType *pType, BOOL bDeepParse )
{
    ULONG index = 0;
    mdToken typeRef;
    ULONG elementType;
    ULONG tempType;
    
    
    // picking apart primitive types is easy;  
    // the ones below require a bit more processing
    index += CorSigUncompressData( &pSignature[index], &elementType );                   
    switch ( elementType )
    {
        // SENTINEL, PINNED and BYREF are not types, just modifiers
        case ELEMENT_TYPE_SENTINEL:
        case ELEMENT_TYPE_BYREF:    
        case ELEMENT_TYPE_PINNED:
                DEBUG_OUT( ("**** PROCESSING SENTINEL/PINNED/BYREF ****") )
                index += GetElementType( &pSignature[index], (CorElementType *)&elementType, bDeepParse );
                break;


        case ELEMENT_TYPE_PTR:
        case ELEMENT_TYPE_SZARRAY:  
                DEBUG_OUT( ("**** PROCESSING PTR/SZARRAY ****") )
                if ( bDeepParse )
                    index += GetElementType( &pSignature[index], (CorElementType *)&elementType );
                else
                    index += GetElementType( &pSignature[index], (CorElementType *)&tempType );

                break;
                        
                        
        case ELEMENT_TYPE_CLASS:
        case ELEMENT_TYPE_VALUETYPE:                                             
                DEBUG_OUT( ("**** PROCESSING CLASS/OBJECT/VALUECLASS ****") )
                index += CorSigUncompressToken( &pSignature[index], &typeRef );
                break;                   
                    

        case ELEMENT_TYPE_CMOD_OPT:
        case ELEMENT_TYPE_CMOD_REQD:                                                            
                DEBUG_OUT( ("**** PROCESSING CMOD_OPT/CMOD_REQD ****") )
                index += CorSigUncompressToken( &pSignature[index], &typeRef ); 
                if ( bDeepParse )
                    index += GetElementType( &pSignature[index], (CorElementType *)&elementType );                                                                                                                                 
                else
                    index += GetElementType( &pSignature[index], (CorElementType *)&tempType );

                break;                                            


        case ELEMENT_TYPE_ARRAY:     
                DEBUG_OUT( ("**** PROCESSING ARRAY ****") )
                if ( bDeepParse )
                    index += ProcessArray( &pSignature[index], (CorElementType *)&elementType );                                                                                                                                   
                else
                    index += ProcessArray( &pSignature[index], (CorElementType *)&tempType );

                break;


        case ELEMENT_TYPE_FNPTR:     
                DEBUG_OUT( ("**** PROCESSING FNPTR ****") )

                // !!! this will throw exception !!!
                index += ProcessMethodDefRef( &pSignature[index], (CorElementType *)&tempType );                                                                                                                                   

                break;                                            
                
    } // switch

    *pType = (CorElementType)elementType;
    

    return index;

} // BASEHELPER::GetElementType


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
ULONG BASEHELPER::ProcessArray( PCCOR_SIGNATURE pSignature, CorElementType *pType )
{
    ULONG index = 0;
    ULONG rank = 0;


    index += GetElementType( &pSignature[index], pType );                                                                                                                                  
    index += CorSigUncompressData( &pSignature[index], &rank );
    if ( rank > 0 )
    {
        UINT i;
        ULONG sizes = 0;
        ULONG lowers = 0;


        index += CorSigUncompressData( &pSignature[index], &sizes );
        for ( i = 0; i < sizes; i++ ) 
        {
            ULONG dimension;


            index += CorSigUncompressData( &pSignature[index], &dimension );
        } // for

        
        index += CorSigUncompressData( &pSignature[index], &lowers );
        for ( i = 0; i < lowers; i++ )
        {
            int lowerBound;


            index += CorSigUncompressSignedInt( &pSignature[index], &lowerBound );
        } // for
    }


    return index;
    
} // BASEHELPER::ProcessArray


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
ULONG BASEHELPER::ProcessMethodDefRef( PCCOR_SIGNATURE pSignature, CorElementType *pType )
{
    _THROW_EXCEPTION( "**** ELEMENT_TYPE_FNPTR not supported by the framework ****" )

    return 0;
    
} // BASEHELPER::ProcessArray

// End of File
 
