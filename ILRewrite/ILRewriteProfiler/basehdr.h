// ==++==

//   Copyright (c) Microsoft Corporation.  All rights reserved.

// ==--==

#ifndef __BASEHDR_H__
#define __BASEHDR_H__

#define _WIN32_DCOM

//************************************************************************************************//

//******************                      common  includes                      ******************//

//************************************************************************************************//

#include "math.h"
#include "time.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "limits.h"
#include "malloc.h"
#include "string.h"

#include "windows.h"
#include "winreg.h"
#include "wincrypt.h"
#include "winbase.h"
#include "objbase.h"

#include "cor.h"
#include "corhdr.h"
#include "corhlpr.h"
#include "corerror.h"

#include "corpub.h"
#include "corprof.h"
#include "cordebug.h"
#include "SpecStrings.h"

//************************************************************************************************//

//******************                     compiler  warnings                     ******************//

//************************************************************************************************//

// Compiler complains about the exception not being used in the exception handler - it's rethrown.
#pragma warning (disable: 4101)

// Compiler complains about not having an implementation for a base class where the derived class
// exports everything and the base class is a template.
#pragma warning (disable: 4275)

// Compiler complains about "a unary minus operator applied to unsigned type ...", when importing
// mscorlib.tlb for the debugger service test.
#pragma warning (disable: 4146)

#ifdef _PREFAST_
// Suppress prefast warning #6255: alloca indicates failure by raising a stack overflow exception.
#pragma warning(disable:6255)
#endif

//************************************************************************************************//

//******************                        basic macros                        ******************//

//************************************************************************************************//

// alias' for COM method signatures
#define COM_METHOD(TYPE) TYPE STDMETHODCALLTYPE

// max length for arrays
#define MAX_LENGTH 256

// export functions
#ifdef _USE_DLL_
#if defined _EXPORT_
#define DECLSPEC __declspec(dllexport)
#elif defined _IMPORT_
#define DECLSPEC __declspec(dllimport)
#endif
#else
#define DECLSPEC
#endif

// DebugBreak
#undef _DbgBreak
#ifdef _X86_
#define _DbgBreak() __asm { int 3 }
#else
#define _DbgBreak() DebugBreak()
#endif

#endif