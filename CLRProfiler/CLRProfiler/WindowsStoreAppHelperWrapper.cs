/* ==++==
 * 
 *   Copyright (c) Microsoft Corporation.  All rights reserved.
 * 
 * ==--==
 *
 * Class:  WindowsStoreAppHelperWrapper
 *
 * Description: Wrapper class that dynamically loads the real WindowsStoreApp helper
 * class and invokes its methods via Reflection.  The reason the real
 * CLRProfileWindowsStoreAppHelper.dll is separated in this way is to allow CLRProfiler.exe to be
 * compiled with no static dependencies on Windows 8 (CLRProfiler's
 * TargetPlatformVersion need not be set to 8).  The real
 * CLRProfilerWindowsStoreAppHelper.dll DOES have TargetPlatformVersion set to 8, and
 * does have static references to Windows 8-only DLLs.
 */

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Reflection;
using System.Windows.Forms;

namespace CLRProfiler
{
    // Objects of these types are exchanged between and 
    // WindowsStoreAppHelperWrapper

    public struct AppInfo
    {
        public string exeName;
        public string userModelId;
    }

    public struct PackageInfo
    {
        public string installedLocation;
        public string architecture;
        public string fullName;
        public string name;
        public string publisher;
        public string version;
        public string tempDir;
        public string acSid;
        public List<AppInfo> appInfoList; 
    }

    // Warning: Not multi-thread aware.  Designed to be used serially, primarily from main
    // UI thread (MainForm.cs)
    class WindowsStoreAppHelperWrapper
    {
        private static bool s_isInitialized = false;
        private static bool s_isWindowsStoreAppSupported = false;
        private static Type s_windowsStoreAppHelper = null;

        public static void Init()
        {
            const string helperAsmName = "CLRProfilerWindowsStoreAppHelper.dll";
            const string windowsStoreAppHelperType = "CLRProfilerWindowsStoreAppHelper.WindowsStoreAppHelper";

            if (s_isInitialized)
                return;

            s_isWindowsStoreAppSupported = false;
            s_isInitialized = true;

            Version osVer = Environment.OSVersion.Version;
            if ((osVer.Major < 6) || (osVer.Minor < 2))
            {
                MessageBox.Show("This feature is only available on Windows 8 and higher.");
                return;
            }

            // Dynamically load CLRProfilerWindowsStoreAppHelper.dll
            
            Assembly windowsStoreAppHelperAsm = Assembly.LoadFrom(
                Path.GetDirectoryName(Assembly.GetCallingAssembly().Location) +
                    "\\" +
                    helperAsmName);

            // Do a simple check to catch an innocently mismatched CLRProfiler and 
            // CLRProfilerWindowsStoreAppHelper.  Note that, if CLRProfilerWindowsStoreAppHelper had
            // a strong name, we could just specify its full strong name to Assembly.Load, and
            // version checking would be done for free (and would be much more accurate).  Since
            // we're doing this check to catch an innocent mistake, and not as a security guarantee,
            // we'll just do the version check manually.
            AssemblyName asmName = new AssemblyName(windowsStoreAppHelperAsm.FullName);
            if ((asmName.Version.Major != 1) || (asmName.Version.Minor != 0))
            {
                MessageBox.Show(
                    string.Format(
                        "A wrong version of {0} was found.  Expected version 1.0, found {1}.{2}",
                        helperAsmName,
                        asmName.Version.Major,
                        asmName.Version.Minor));
                return;
            }

            // Grab the master type
            Module windowsStoreAppHelper = windowsStoreAppHelperAsm.ManifestModule;
            s_windowsStoreAppHelper = windowsStoreAppHelper.GetType(windowsStoreAppHelperType);
            if (s_windowsStoreAppHelper == null)
            {
                MessageBox.Show(
                    string.Format(
                        "The wrong {0} was found.  Cannot find type '{1}'",
                        helperAsmName,
                        windowsStoreAppHelperType));
                return;
            }

            s_isWindowsStoreAppSupported = true;
        }

        public static bool IsWindowsStoreAppSupported()
        {
            Init();
            return s_isWindowsStoreAppSupported;
        }

        public static List<PackageInfo> GetPackagesForCurrentUser()
        {
            MethodInfo getPackagesForCurrentUser = s_windowsStoreAppHelper.GetMethod("GetPackagesForCurrentUser");
            if (getPackagesForCurrentUser == null)
            {
                MessageBox.Show("Cannot find GetPackagesForCurrentUser()");
                s_isWindowsStoreAppSupported = false;
                return null;
            }

            object[] parameters = new object[] { null };
            getPackagesForCurrentUser.Invoke(null /* this */, parameters);
            return (List<PackageInfo>) parameters[0];
        }

        static public void SpawnWindowsStoreAppProcess(string packgeFullName, string appUserModelId, string appArgs, string[] environment, out uint pid)
        {
            pid = 0;
            MethodInfo spawnWindowsStoreAppProcess = s_windowsStoreAppHelper.GetMethod("SpawnWindowsStoreAppProcess");
            if (spawnWindowsStoreAppProcess == null)
            {
                MessageBox.Show("Cannot find SpawnWindowsStoreAppProcess()");
                s_isWindowsStoreAppSupported = false;
                return;
            }

            object[] parameters = new object[] { packgeFullName, appUserModelId, appArgs, environment, pid };
            spawnWindowsStoreAppProcess.Invoke(null /* this */, parameters);
            pid = (uint)parameters[4];
        }

        static public bool IsWindowsStoreAppAccessEnabledForProfiler(string dir)
        {
            MethodInfo isWindowsStoreAppAccessEnabledForProfiler = s_windowsStoreAppHelper.GetMethod("IsWindowsStoreAppAccessEnabledForProfiler");
            if (isWindowsStoreAppAccessEnabledForProfiler == null)
            {
                MessageBox.Show("Cannot find IsWindowsStoreAppAccessEnabledForProfiler()");
                s_isWindowsStoreAppSupported = false;
                return false;
            }

            object[] parameters = new object[] { dir };
            return (bool) isWindowsStoreAppAccessEnabledForProfiler.Invoke(null /* this */, parameters);
        }

        static public void GetWindowsStoreAppInfoFromProcessId(int pid, out string acSid, out string acFolderPath, out string packageFullName)
        {
            acSid = null;
            acFolderPath = null;
            packageFullName = null;
            MethodInfo getWindowsStoreAppInfoFromProcessId = s_windowsStoreAppHelper.GetMethod("GetWindowsStoreAppInfoFromProcessId");
            if (getWindowsStoreAppInfoFromProcessId == null)
            {
                MessageBox.Show("Cannot find GetWindowsStoreAppInfoFromProcessId()");
                s_isWindowsStoreAppSupported = false;
                return;
            }

            object[] parameters = new object[] { pid, acSid, acFolderPath, packageFullName };
            getWindowsStoreAppInfoFromProcessId.Invoke(null /* this */, parameters);
            acSid = (string)parameters[1];
            acFolderPath = (string)parameters[2];
            packageFullName = (string)parameters[3];
        }

        static public void DisableDebuggingForPackage(string packgeFullName)
        {
            MethodInfo disableDebuggingForPackage = s_windowsStoreAppHelper.GetMethod("DisableDebuggingForPackage");
            if (disableDebuggingForPackage == null)
            {
                MessageBox.Show("Cannot find DisableDebuggingForPackage()");
                s_isWindowsStoreAppSupported = false;
                return;
            }

            object[] parameters = new object[] { packgeFullName };
            disableDebuggingForPackage.Invoke(null /* this */, parameters);
        }

        static public void EnableDebuggingForPackage(string packgeFullName)
        {
            MethodInfo enableDebuggingForPackage = s_windowsStoreAppHelper.GetMethod("EnableDebuggingForPackage");
            if (enableDebuggingForPackage == null)
            {
                MessageBox.Show("Cannot find EnableDebuggingForPackage()");
                s_isWindowsStoreAppSupported = false;
                return;
            }

            object[] parameters = new object[] { packgeFullName };
            enableDebuggingForPackage.Invoke(null /* this */, parameters);
        }

        static public bool IsRunningElevated()
        {
            MethodInfo isRunningElevated = s_windowsStoreAppHelper.GetMethod("IsRunningElevated");
            if (isRunningElevated == null)
            {
                MessageBox.Show("Cannot find IsRunningElevated()");
                s_isWindowsStoreAppSupported = false;
                return false;
            }

            return (bool)isRunningElevated.Invoke(null /* this */, null /* parameters */);
        }
    }
}
