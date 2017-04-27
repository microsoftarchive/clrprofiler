// ==++==
// 
//   Copyright (c) Microsoft Corporation.  All rights reserved.
// 
// ==--==


using System;
using System.Threading;
using System.IO;
using System.Text;
using System.Runtime.InteropServices;

[assembly: System.Security.SecurityCritical]
[assembly: System.Security.AllowPartiallyTrustedCallers]

namespace ILRewriteProfilerHelper
{
    /// <summary>
    /// Instrumentation will dynamically add calls into this assembly
    /// </summary>
    public class ProfilerHelper
    {
        // P/Invokes into native profiler DLL

        [DllImport("ILRewriteProfiler.dll")]
        public static extern void NtvEnteredFunction([In] IntPtr moduleIDCur, UInt32 mdCur, int nVersionCur);

        [DllImport("ILRewriteProfiler.dll")]
        public static extern void NtvExitedFunction([In] IntPtr moduleIDCur, UInt32 mdCur, int nVersionCur);


        //---------------------------------------------------------------------------------------
        // Entrypoints that instrumented code calls into

        // Function enter probes

        [System.Security.SecuritySafeCritical]
        public static void MgdEnteredFunction32(UInt32 moduleIDCur, UInt32 mdCur, int nVersionCur)
        {
            NtvEnteredFunction((IntPtr)moduleIDCur, mdCur, nVersionCur);
        }

        [System.Security.SecuritySafeCritical]
        public static void MgdEnteredFunction64(UInt64 moduleIDCur, UInt32 mdCur, int nVersionCur)
        {
            NtvEnteredFunction((IntPtr)moduleIDCur, mdCur, nVersionCur);
        }

        // Function exit probes

        [System.Security.SecuritySafeCritical]
        public static void MgdExitedFunction32(UInt32 moduleIDCur, UInt32 mdCur, int nVersionCur)
        {
            NtvExitedFunction((IntPtr)moduleIDCur, mdCur, nVersionCur);
        }

        [System.Security.SecuritySafeCritical]
        public static void MgdExitedFunction64(UInt64 moduleIDCur, UInt32 mdCur, int nVersionCur)
        {
            NtvExitedFunction((IntPtr)moduleIDCur, mdCur, nVersionCur);
        }
    }
}


