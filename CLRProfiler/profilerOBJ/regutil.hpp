// ==++==
// 
//   Copyright (c) Microsoft Corporation.  All rights reserved.
// 
// ==--==
/****************************************************************************************
 * File:
 *  regutil.hpp
 *
 * Description:
 *  
 *
 *
 ***************************************************************************************/
#ifndef __REGUTIL_HPP__
#define __REGUTIL_HPP__

#include "regutil.h"


typedef HRESULT __stdcall DLLGETCLASSOBJECT(REFCLSID rclsid,
                                            REFIID   riid,
                                            void   **ppv);

IID IID_ICF = {0x00000001, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};

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
 *      Set an entry in the registry of the form:
 *          HKEY_CLASSES_ROOT\szKey\szSubkey = szValue
 *
 *      If szSubkey or szValue are NULL, omit them from the above expression.
 *
 ***************************************************************************************/
/* static public */
BOOL REGUTIL::SetKeyAndValue( const char *szKey,
                              const char *szSubkey,
                              const char *szValue )
{
    HKEY hKey;              // handle to the new reg key.
    const char *rcKey;      // pointer to the full key name.


    // init the key with the base key name.
    rcKey = szKey;

    // append the subkey name (if there is one).
    if ( szSubkey != NULL )
    {
        // if there is a subkey, allocate a big enough buffer for the key, a \, the
        // subkey, and a terminating 0 character.
        size_t fullKeyLen = strlen(szKey) + 1 + strlen(szSubkey) + 1;
        char *szFullKey = (char *)_alloca(fullKeyLen);
        if ( szFullKey == NULL )
            return FALSE;
        
        strcpy_s( szFullKey, fullKeyLen, szKey );
        strcat_s( szFullKey, fullKeyLen, "\\" );
        strcat_s( szFullKey, fullKeyLen, szSubkey );
        rcKey = szFullKey;
    }

    // create the registration key.
    if ( RegCreateKeyExA( HKEY_CLASSES_ROOT, 
                          rcKey, 
                          0, 
                          NULL,
                          REG_OPTION_NON_VOLATILE, 
                          KEY_ALL_ACCESS, 
                          NULL,
                          &hKey, 
                          NULL ) == ERROR_SUCCESS )
    {
        // set the value (if there is one).
        if ( szValue != NULL )
        {
            RegSetValueExA( hKey, 
                            NULL, 
                            0, 
                            REG_SZ, 
                            (BYTE *)szValue,
                            (DWORD)((strlen( szValue ) + 1) * sizeof ( char )) );
        }
        
        RegCloseKey( hKey );
    
        
        return TRUE;
    }   


    return FALSE;   

} // REGUTIL::SetKeyAndValue


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
 *      Delete an entry in the registry of the form:
 *          HKEY_CLASSES_ROOT\szKey\szSubkey = szValue
 *
 ***************************************************************************************/
/* static public */
BOOL REGUTIL::DeleteKey( const char *szKey,
                         const char *szSubkey )
{
    const char *rcKey; // pointer to the the full key name.

    // init the key with the base key name.
    rcKey = szKey;

    // append the subkey name (if there is one).
    if ( szSubkey != NULL )
    {
        // if there is a subkey, allocate a big enough buffer for the key, a \, the
        // subkey, and a terminating 0 character.
        size_t fullKeyLen = strlen(szKey) + 1 + strlen(szSubkey) + 1;
        char *szFullKey = (char *)_alloca(fullKeyLen);
        if ( szFullKey == NULL )
            return FALSE;
        strcpy_s( szFullKey, fullKeyLen, szKey );
        strcat_s( szFullKey, fullKeyLen, "\\" );
        strcat_s( szFullKey, fullKeyLen, szSubkey );
        rcKey = szFullKey;
    }

    // delete the registration key.
    RegDeleteKeyA( HKEY_CLASSES_ROOT, rcKey );
    
    
    return TRUE;

} // REGUTIL::DeleteKey


/***************************************************************************************
 *  Method:
 *
 *
 *  Purpose:
 *      Open the key, create a new keyword and value pair under it.
 *
 *  Parameters: 
 *
 *
 *  Return value:
 *
 *
 *  Notes:
 *                 
 *
 ***************************************************************************************/
/* static public */
BOOL REGUTIL::SetRegValue( const char *szKeyName,
                           const char *szKeyword,
                           const char *szValue )
{
    HKEY hKey; // handle to the new reg key.


    // create the registration key.
    if ( RegCreateKeyExA( HKEY_CLASSES_ROOT, 
                          szKeyName, 
                          0, 
                          NULL,
                          REG_OPTION_NON_VOLATILE, 
                          KEY_ALL_ACCESS, 
                          NULL,
                          &hKey, 
                          NULL) == ERROR_SUCCESS )
    {
        // set the value (if there is one).
        if ( szValue != NULL )
        {
            RegSetValueExA( hKey, 
                            szKeyword, 
                            0, 
                            REG_SZ, 
                            (BYTE *)szValue, 
                            (DWORD)((strlen( szValue ) + 1) * sizeof ( char )) );
        }

        RegCloseKey( hKey );

        
        return TRUE;
    }

    
    return FALSE;

} // REGUTIL::SetRegValue


/***************************************************************************************
 *  Method:
 *
 *
 *  Purpose:
 *      Does standard registration of a CoClass with a progid.
 *
 *  Parameters: 
 *
 *
 *  Return value:
 *
 *
 *  Notes:
 *                 
 *
 ***************************************************************************************/
/* static public */
HRESULT REGUTIL::RegisterCOMClass( REFCLSID rclsid,
                                   const char *szDesc,                  
                                   const char *szProgIDPrefix,  
                                   int  iVersion,               
                                   const char *szClassProgID,   
                                   const char *szThreadingModel,
                                   const char *szModule ) 
{
    HRESULT hr;
    char rcCLSID[MAX_LENGTH];           // CLSID\\szID.
    char rcProgID[MAX_LENGTH];          // szProgIDPrefix.szClassProgID
    char rcIndProgID[MAX_LENGTH];       // rcProgID.iVersion
    char rcInproc[MAX_LENGTH + 2];      // CLSID\\InprocServer32


    // format the prog ID values.
    _snprintf_s( rcIndProgID, NumItems(rcIndProgID), NumItems(rcIndProgID)-1, "%s.%s", szProgIDPrefix, szClassProgID ) ;
    _snprintf_s( rcProgID, NumItems(rcProgID), NumItems(rcProgID)-1, "%s.%d", rcIndProgID, iVersion );
    rcProgID[NumItems(rcProgID)-1] = 0;

    // do the initial portion.
    hr =  REGUTIL::_RegisterClassBase( rclsid, 
                                       szDesc, 
                                       rcProgID, 
                                       rcIndProgID, 
                                       rcCLSID,
                                       NumItems(rcCLSID) );
    if ( SUCCEEDED( hr ) )
    {
        // set the server path.
        SetKeyAndValue( rcCLSID, "InprocServer32", szModule );

        // add the threading model information.
        _snprintf_s( rcInproc, NumItems(rcInproc), NumItems(rcInproc)-1, "%s\\%s", rcCLSID, "InprocServer32" );
        SetRegValue( rcInproc, "ThreadingModel", szThreadingModel );
    }   
    

    return hr;

} // REGUTIL::RegisterCOMClass


/***************************************************************************************
 *  Method:
 *
 *
 *  Purpose:
 *      Unregister the basic information in the system registry for a given object class
 *
 *  Parameters: 
 *
 *
 *  Return value:
 *
 *
 *  Notes:
 *                 
 *
 ***************************************************************************************/
/* static public */
HRESULT REGUTIL::UnregisterCOMClass( REFCLSID rclsid,          
                                     const char *szProgIDPrefix,
                                     int iVersion,            
                                     const char *szClassProgID )
{
    char szID[64];         // the class ID to unregister.
    char rcCLSID[64];      // CLSID\\szID.
    OLECHAR szWID[64];     // helper for the class ID to unregister.
    char rcProgID[128];    // szProgIDPrefix.szClassProgID
    char rcIndProgID[128]; // rcProgID.iVersion


    // format the prog ID values.
    _snprintf_s( rcProgID, NumItems(rcProgID), NumItems(rcProgID)-1, "%s.%s", szProgIDPrefix, szClassProgID );
    _snprintf_s( rcIndProgID, NumItems(rcIndProgID), NumItems(rcIndProgID)-1, "%s.%d", rcProgID, iVersion );

    REGUTIL::_UnregisterClassBase( rclsid, rcProgID, rcIndProgID, rcCLSID, NumItems(rcCLSID) );
    DeleteKey( rcCLSID, "InprocServer32" );

    StringFromGUID2(rclsid, szWID, NumItems( szWID ) );
    WideCharToMultiByte( CP_ACP, 
                         0, 
                         szWID, 
                         -1, 
                         szID, 
                         sizeof( szID ), 
                         NULL, 
                         NULL );

    DeleteKey( "CLSID", rcCLSID );
    
    
    return S_OK;

} // REGUTIL::UnregisterCOMClass



/***************************************************************************************
 *  Method:
 *
 *
 *  Purpose:
 *      Register the basics for a in proc server.
 *
 *  Parameters: 
 *
 *
 *  Return value:
 *
 *
 *  Notes:
 *                 
 *
 ***************************************************************************************/
/* static private */
HRESULT REGUTIL::_RegisterClassBase( REFCLSID rclsid,
                                     const char *szDesc,                    
                                     const char *szProgID,              
                                     const char *szIndepProgID,         
                                     __out_ecount(cchOutCLSID) char *szOutCLSID,
                                     size_t cchOutCLSID)                
{
    char szID[64];     // the class ID to register.
    OLECHAR szWID[64]; // helper for the class ID to register.


    StringFromGUID2( rclsid, szWID, NumItems( szWID ) );
    WideCharToMultiByte( CP_ACP, 
                         0, 
                         szWID, 
                         -1, 
                         szID, 
                         sizeof( szID ), 
                         NULL, 
                         NULL );

    // check if the output buffer is big enough to hold the result
    size_t nLen = strlen("CLSID\\") + strlen(szID) + 1;
    if ( cchOutCLSID < nLen )
    {
        szOutCLSID[0] = '\0';
        return E_INVALIDARG;
    }

    strcpy_s( szOutCLSID, cchOutCLSID, "CLSID\\" );
    strcat_s( szOutCLSID, cchOutCLSID, szID );

    // create ProgID keys.
    SetKeyAndValue( szProgID, NULL, szDesc );
    SetKeyAndValue( szProgID, "CLSID", szID );

    // create VersionIndependentProgID keys.
    SetKeyAndValue( szIndepProgID, NULL, szDesc );
    SetKeyAndValue( szIndepProgID, "CurVer", szProgID );
    SetKeyAndValue( szIndepProgID, "CLSID", szID );

    // create entries under CLSID.
    SetKeyAndValue( szOutCLSID, NULL, szDesc );
    SetKeyAndValue( szOutCLSID, "ProgID", szProgID );
    SetKeyAndValue( szOutCLSID, "VersionIndependentProgID", szIndepProgID );
    SetKeyAndValue( szOutCLSID, "NotInsertable", NULL );
    
    
    return S_OK;

} // REGUTIL::_RegisterClassBase


/***************************************************************************************
 *  Method:
 *
 *
 *  Purpose:
 *      Delete the basic settings for an inproc server.
 *
 *  Parameters: 
 *
 *
 *  Return value:
 *
 *
 *  Notes:
 *                 
 *
 ***************************************************************************************/
/* static private */
HRESULT REGUTIL::_UnregisterClassBase( REFCLSID rclsid,
                                       const char *szProgID,
                                       const char *szIndepProgID,
                                       __out_ecount(cchOutCLSID) char *szOutCLSID,
                                       size_t cchOutCLSID )
{
    char szID[64];     // the class ID to register.
    OLECHAR szWID[64]; // helper for the class ID to register.


    StringFromGUID2( rclsid, szWID, NumItems( szWID ) );
    WideCharToMultiByte( CP_ACP, 
                         0, 
                         szWID, 
                         -1, 
                         szID, 
                         sizeof( szID ), 
                         NULL, 
                         NULL );

    // check if the output buffer is big enough to hold the result
    size_t nLen = strlen("CLSID\\") + strlen(szID) + 1;
    if ( cchOutCLSID < nLen )
    {
        szOutCLSID[0] = '\0';
        return E_INVALIDARG;
    }

    strcpy_s( szOutCLSID, cchOutCLSID, "CLSID\\" );
    strcat_s( szOutCLSID, cchOutCLSID, szID );

    // delete the version independant prog ID settings.
    DeleteKey( szIndepProgID, "CurVer" );
    DeleteKey( szIndepProgID, "CLSID" );
    RegDeleteKeyA( HKEY_CLASSES_ROOT, szIndepProgID );

    // delete the prog ID settings.
    DeleteKey( szProgID, "CLSID" );
    RegDeleteKeyA( HKEY_CLASSES_ROOT, szProgID );

    // delete the class ID settings.
    DeleteKey( szOutCLSID, "ProgID" );
    DeleteKey( szOutCLSID, "VersionIndependentProgID" );
    DeleteKey( szOutCLSID, "NotInsertable" );
    RegDeleteKeyA( HKEY_CLASSES_ROOT, szOutCLSID );
    
    
    return S_OK;

} // REGUTIL::_UnregisterClassBase

#endif // __REGUTIL_HPP__

// End of File
