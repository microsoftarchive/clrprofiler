// ==++==
// 
//   Copyright (c) Microsoft Corporation.  All rights reserved.
// 
// ==--==
//
// WindowsStoreAppThreadResumer acts as a debugger for the purpose of launching
// a Windows Store app with customized environment variables for
// loading ProfilerObj.dll as the managed profiler.  The facility
// provided by Windows to allow environment variables to be passed
// to a Windows Store app requires a debugger to be involved, even
// an extremely simple debugger such as this (all it does
// is resume the thread it's given and exit).  The environment variables
// are passed before WindowsStoreAppThreadResumer ever gets involved, when
// MainForm calls SpawnWindowsStoreAppProcess, which in turn calls the OS's
// IPackageDebugSettings::EnableDebugging.  Note that, since WindowsStoreAppThreadResumer
// returns immediately, a user is allowed to attach a "real" debugger to
// the profilee later on, if desired.

#include "stdafx.h"



// The OS invokes this executable like it would a debugger, passing the
// command-line parameters as in this example:
//		CLRProfilerWindowsStoreAppThreadResumer.exe -p 1336 -tid 1424
// where -p 1336 means Process ID 1336, and -tid 1424 means Thread ID 1424


int wmain(int argc, wchar_t* argv[])
{
	// Scan for -tid argument.  The next argument will be the ThreadID
	int nThreadID = 0;
	for (int i=0; i < argc - 1; i++)
		// Don't check last element, cuz "-tid" can't be the last element, since
		// the thread ID value must follow it
	{
		if ((argv[i][0] != L'-') && (argv[i][0] != L'/'))
			continue;

		if (_wcsicmp(&(argv[i][1]), L"tid") != 0)
			continue;

		nThreadID = _wtoi(argv[i+1]);
		break;
	}

	if (nThreadID == 0)
	{
		// Command line invalid
		return 1;
	}

    HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE /* bInheritHandle */, nThreadID);
    if (hThread == NULL)
    {
		return 1;
	}

	ResumeThread(hThread);
	CloseHandle(hThread);
	return 0;
}
