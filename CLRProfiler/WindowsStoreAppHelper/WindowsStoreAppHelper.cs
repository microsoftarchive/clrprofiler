/* ==++==
 * 
 *   Copyright (c) Microsoft Corporation.  All rights reserved.
 * 
 * ==--==
 *
 * Class:  WindowsStoreAppHelper
 *
 * Description: Set of static helper methods used when profiling Windows
 * Store apps.  This assembly should only be loaded on Windows 8 and higher,
 * so it is used indirectly, via WindowsStoreAppHelperWrapper, which dynamically
 * chooses to load this assembly and call this type via reflection.
 */

using AppxPackaging;                    // TLBimp file manually referenced
using CLRProfiler;
using CLRProfilerWindowsStoreAppHelper;
using Microsoft.Win32.SafeHandles;
using Shell;                            // TLBimp file manually referenced
using System;
using System.Configuration;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Security;
using System.Security.AccessControl;
using System.Security.Principal;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml;
using System.Xml.XPath;
using Windows;
using Windows.ApplicationModel;
using Windows.Foundation;
using Windows.Management.Core;
using Windows.Management.Deployment;
using Windows.Storage;

namespace CLRProfilerWindowsStoreAppHelper
{
    public class WindowsStoreAppHelper
    {
#pragma warning disable 0649
        private struct SID_AND_ATTRIBUTES
        {
            public IntPtr Sid;
            public uint Attributes;
        }

        private struct TOKEN_MANDATORY_LABEL
        {
            public SID_AND_ATTRIBUTES Label;
        }
#pragma warning restore 0649

        [DllImport("shlwapi.dll", CharSet = CharSet.Unicode)]
        private static extern uint SHCreateStreamOnFileEx(string fileName, uint grfMode, uint attributes, bool create, AppxPackaging.IStream reserved, out AppxPackaging.IStream stream);

        [DllImport("userenv.dll", CharSet=CharSet.Unicode)]
        private static extern int DeriveAppContainerSidFromAppContainerName(string packageFamilyName, out IntPtr acPSID);

        [DllImport("Advapi32.dll", CharSet=CharSet.Unicode)]
        private static extern int ConvertSidToStringSid(IntPtr psid, out string sidString);

        [DllImport("kernel32.dll", CharSet = CharSet.Unicode)]
        private static extern uint GetPackageFamilyName(IntPtr hProcess, ref uint packageFamilyNameLength, StringBuilder packageFamilyName);

        [DllImport("kernel32.dll", CharSet = CharSet.Unicode)]
        private static extern uint GetPackageFullName(IntPtr hProcess, ref uint packageFullNameLength, StringBuilder packageFullName);

        [DllImport("advapi32.dll", CharSet = CharSet.Unicode)]
        private static extern IntPtr GetSidSubAuthorityCount(IntPtr psid);

        [DllImport("advapi32.dll", CharSet = CharSet.Unicode, SetLastError=true)]
        private static extern int GetTokenInformation(IntPtr hToken, int tokenInformationClass, IntPtr tokenInformation, uint tokenInformationLength, out uint returnLength);

        [DllImport("advapi32.dll", CharSet = CharSet.Unicode)]
        private static extern IntPtr GetSidSubAuthority(IntPtr psid, uint subAuthorityIndex);

        private const uint APPMODEL_ERROR_NO_PACKAGE = 15700;
        private const uint ERROR_SUCCESS = 0;
        private const int ERROR_INSUFFICIENT_BUFFER = 122;
        private const uint SECURITY_MANDATORY_HIGH_RID = 0x00003000;
        private const uint TOKEN_READ = 0x00020008;

        static public void GetPackagesForCurrentUser(ref List<PackageInfo> packageInfos)
        {
            string currentUserSID = WindowsIdentity.GetCurrent().User.ToString();

            //---------------------------------------------------------------------------------------
            // Get iterator over all packages installed for this user

            // First, we need the PackageManager
    
            IAppxFactory appxFactory = (IAppxFactory) new AppxFactory();

            PackageManager packageManager = new PackageManager();

            IEnumerable<Package> packages = packageManager.FindPackagesForUser(currentUserSID);
            int cPackages = packages.Count();
            if (cPackages == 0)
                return;

            packageInfos = new List<PackageInfo>(cPackages);
            foreach (Package package in packages)
            {
                try
                {
                    // Find and load manifest
                    string manifestPath = package.InstalledLocation.Path + "\\AppxManifest.xml";

                    AppxPackaging.IStream manifestStream;
                    if (SHCreateStreamOnFileEx(
                        manifestPath,
                        0x00000040,     // STGM_READ | STGM_SHARE_DENY_NONE
                        0,              // file creation attributes
                        false,          // fCreate
                        null,           // reserved
                        out manifestStream) < 0)
                    {
                        // If we can't open the manifest for this package, skip it
                        continue;
                    }

                    IAppxManifestReader manifestReader;
                    manifestReader = appxFactory.CreateManifestReader(manifestStream);
                
                    IAppxManifestApplicationsEnumerator appsEnum = manifestReader.GetApplications();
                    if (appsEnum.GetHasCurrent() == 0)
                    {
                        // Packages with no apps exist, and are uninteresting.  Skip.
                        continue;
                    }

                    // Grab info from Package
                    PackageInfo packageInfo = new PackageInfo();
                    packageInfo.installedLocation = package.InstalledLocation.Path;

                    // Grab info from PackageID
                    PackageId packageId = package.Id;
                    packageInfo.architecture = packageId.Architecture.ToString();
                    packageInfo.fullName = packageId.FullName;
                    packageInfo.name = packageId.Name;
                    packageInfo.version = String.Format(
                        "{0}.{1}.{2}.{3}",
                        packageId.Version.Major,
                        packageId.Version.Minor,
                        packageId.Version.Build,
                        packageId.Version.Revision);

                    IAppxManifestProperties props = manifestReader.GetProperties();
                    packageInfo.publisher = props.GetStringValue("PublisherDisplayName");

                    // Figure out temp folder path

                    if (!GetACInfo(packageId.FamilyName, out packageInfo.acSid, out packageInfo.tempDir))
                        continue;

                    // Check to see if CLRHost.dll is listed as an InProcessServer extension.  If so,
                    // it may be a JS/CLR hybrid package, in which case we'll include all the
                    // AppUserModelIds we find later on.
                    bool usesClrHostExtension = false;
                    try
                    {
                        XmlDocument xmlDoc = new XmlDocument();
                        xmlDoc.Load(manifestPath);
                        XPathNavigator docNavigator = xmlDoc.CreateNavigator();
                        XmlNamespaceManager mgr = new XmlNamespaceManager(docNavigator.NameTable);
                        mgr.AddNamespace("pm", "http://schemas.microsoft.com/appx/2010/manifest");
                        XPathNodeIterator iterator = docNavigator.Select("/pm:Package/pm:Extensions/pm:Extension/pm:InProcessServer/pm:Path", mgr);
                        foreach (XPathNavigator selectedNodeNavigator in iterator)
                        {
                            if (selectedNodeNavigator.Value.IndexOf("clrhost.dll", StringComparison.CurrentCultureIgnoreCase) != -1)
                            {
                                usesClrHostExtension = true;
                                break;
                            }
                        }
                    }
                    catch (Exception)
                    {
                        // Intentionally fall through. Any error encountered determining
                        // whether this is a hybrid package should just result in us
                        // assuming it isn't one.
                        Debug.Assert(!usesClrHostExtension);
                    }

                    // For each app in the package, get its App User Model ID
                    packageInfo.appInfoList = new List<AppInfo>(5);
                    while (appsEnum.GetHasCurrent() != 0)
                    {
                        IAppxManifestApplication app = appsEnum.GetCurrent();
                        AppInfo appInfo = new AppInfo();
                        appInfo.userModelId = app.GetAppUserModelId();
                        appInfo.exeName = app.GetStringValue("Executable");
                        IAppxManifestResourcesEnumerator resourcesEnum = manifestReader.GetResources();
                        if ((appInfo.userModelId != null) && 
                            ((appInfo.exeName != null) || usesClrHostExtension))
                        {
                            // Only care about apps with an app user model ID
                            // (which we need for activation) and either an exe name (to display to user) or
                            // evidence that a CLR extension is used
                            if (appInfo.exeName == null)
                            {
                                appInfo.exeName = "(no executable)";
                            }
                            packageInfo.appInfoList.Add(appInfo);
                        }
                        appsEnum.MoveNext();
                    }

                    if (packageInfo.appInfoList.Count > 0)
                    {
                        // Only care about packages that contain apps we care about
                        packageInfos.Add(packageInfo);
                    }
                }
                catch (Exception)
                {
                    // If there are any problems with the package we're currently
                    // iterating over, just skip it and continue with the next package
                    // in the enumeration. For example, can't open the manifest
                    // or can't find installed location for the package (b/c the
                    // developer manually moved files around), skip it
                }
            }
        }

        static private bool GetACInfo(string packageFamilyName, out string acSid, out string tempDir)
        {
            IntPtr acPSID;
            acSid = null;
            tempDir = null;

            if (DeriveAppContainerSidFromAppContainerName(packageFamilyName, out acPSID) < 0)
                return false;

            if (ConvertSidToStringSid(acPSID, out acSid) == 0)
                return false;

            ApplicationData appData = ApplicationDataManager.CreateForPackageFamily(packageFamilyName);
            tempDir = appData.TemporaryFolder.Path;

            return true;
        }

        unsafe static public bool IsRunningElevated()
        {
            // Pre-vista, nothing ran elevated
            if (Environment.OSVersion.Version.Major < 6)
            {
                return false;
            }

            IntPtr processToken = WindowsIdentity.GetCurrent().Token;

            uint returnLength;
            if (GetTokenInformation(
                processToken,
                25,                     // TokenIntegrityLevel,
                IntPtr.Zero,            // TokenInformation
                0,                      // tokenInformationLength
                out returnLength) != 0)
            {
                return false;
            }

            if (Marshal.GetLastWin32Error() != ERROR_INSUFFICIENT_BUFFER)
            {
                return false;
            }

            uint length = returnLength;
            byte[] tokenInformation = new byte[length];

            uint integLevel;
            fixed (byte* tokenInformationPtr = tokenInformation)
            {
                if (GetTokenInformation(
                    processToken,
                    25,                             // TokenIntegrityLevel,
                    (IntPtr)tokenInformationPtr,    // TokenInformation
                    length,                         // tokenInformationLength
                    out returnLength) == 0)
                {
                    return false;
                }

                TOKEN_MANDATORY_LABEL label = (TOKEN_MANDATORY_LABEL) Marshal.PtrToStructure((IntPtr) tokenInformationPtr, typeof(TOKEN_MANDATORY_LABEL));
                IntPtr psid = label.Label.Sid;

                IntPtr subAuthCountPtr = GetSidSubAuthorityCount(psid);
                byte subAuthCount = Marshal.ReadByte(subAuthCountPtr);
                uint subAuthIndex = (uint)(subAuthCount - 1);
                IntPtr integLevelPtr = GetSidSubAuthority(psid, subAuthIndex);
                integLevel = (uint)Marshal.ReadInt32(integLevelPtr);
            }

            return (integLevel >= SECURITY_MANDATORY_HIGH_RID);
        }

        static public void GetWindowsStoreAppInfoFromProcessId(int pid, out string acSid, out string tempFolderPath, out string packageFullName)
        {
            acSid = null;
            tempFolderPath = null;
            packageFullName = null;

            Process process = Process.GetProcessById(pid);
            
            uint packageFamilyNameLength = 0;
            StringBuilder packageFamilyName = new StringBuilder();

            uint ret = GetPackageFamilyName(
                process.Handle,
                ref packageFamilyNameLength,
                packageFamilyName);

            if ((ret == APPMODEL_ERROR_NO_PACKAGE) || (packageFamilyNameLength == 0))
            {
                // Not a WindowsStoreApp process
                return;
            }

            // Call again, now that we know the size
            packageFamilyName = new StringBuilder((int)packageFamilyNameLength);
            ret = GetPackageFamilyName(
                process.Handle,
                ref packageFamilyNameLength,
                packageFamilyName);
            if (ret != ERROR_SUCCESS)
                return;

            // PACKAGE FULL NAME

            uint packageFullNameLength = 0;
            StringBuilder packageFullNameBld = new StringBuilder();

            ret = GetPackageFullName(
                process.Handle,
                ref packageFullNameLength,
                packageFullNameBld);

            if ((ret == APPMODEL_ERROR_NO_PACKAGE) || (packageFullNameLength == 0))
            {
                // Not a WindowsStoreApp process
                return;
            }

            // Call again, now that we know the size
            packageFullNameBld = new StringBuilder((int)packageFullNameLength);
            ret = GetPackageFullName(
                process.Handle,
                ref packageFullNameLength,
                packageFullNameBld);
            if (ret != ERROR_SUCCESS)
                return;
            
            if (!GetACInfo(packageFamilyName.ToString(), out acSid, out tempFolderPath))
            {
                acSid = null;
                return;
            }

            packageFullName = packageFullNameBld.ToString();
        }

        // Tells the OS to put the package in "debugging" mode, which ensures it does not
        // get suspended.  This is mainly used for attach, where environment variables are
        // not to be passed to the profilee.  For launch-profiling, SpawnWindowsStoreAppProcess()
        // also puts the package in debugging mode, but passes environment variables as well.
        static public void EnableDebuggingForPackage(string packageFullName)
        {
            IApplicationActivationManager appActivationMgr = new ApplicationActivationManager();
            IPackageDebugSettings pkgDebugSettings = new PackageDebugSettings();
            pkgDebugSettings.EnableDebugging(packageFullName, null /* debugger cmd line */, IntPtr.Zero /* environment */);
            pkgDebugSettings.Resume(packageFullName);
        }

        // Tells the OS to take a package out of "debugging" mode.  This resets the intent
        // to pass environment variables to the package and allows the package to be suspended
        // by the OS in the future, as usual.
        static public void DisableDebuggingForPackage(string packgeFullName)
        {
            IApplicationActivationManager appActivationMgr = new ApplicationActivationManager();
            IPackageDebugSettings pkgDebugSettings = new PackageDebugSettings();
            pkgDebugSettings.DisableDebugging(packgeFullName);
        }
        
        // Interop goo that takes a C#-friendly string array and turns it into an
        // IPackageDebugSettings::EnableDebugging-friendly environment block string (with
        // embedded NULLs, and a terminating double-NULL)
        static private ushort[] StringArrayToPzz(string[] strs)
        {
            List<ushort> pzz = new List<ushort>();
            for (int i = 0; i < strs.Length; i++)
            {
                for (int j = 0; j < strs[i].Length; j++)
                {
                    pzz.Add((ushort)strs[i][j]);
                }
                // Null-terminate each contained string
                pzz.Add(0);
            }
            // Extra-null-terminate the entire pzz
            pzz.Add(0);

            return pzz.ToArray();
        }

        // "Unsafe" so we can very easily pass environmentPzz (a string with embedded nulls)
        // to Win32 as an IntPtr
        unsafe static public void SpawnWindowsStoreAppProcess(string packgeFullName, string appUserModelId, string appArgs, string[] environment, out uint pid)
        {
            IApplicationActivationManager appActivationMgr = new ApplicationActivationManager();
            IPackageDebugSettings pkgDebugSettings = new PackageDebugSettings();

            // NOTE: If you'd like to step through the native profiler DLL's startup, use a
            // string like this instead of the WindowsStoreAppThreadResumer string below
            //string debuggerCommandLine = "C:\\debuggers\\windbg.exe";
            
            string debuggerCommandLine = Path.Combine(
                Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName),
                "CLRProfilerWindowsStoreAppThreadResumer.exe");
            
            if (!File.Exists(debuggerCommandLine))
            {
                pid = unchecked((uint)-1);
                MessageBox.Show(
                    string.Format(
                        "Cannot launch Windows Store app, because the following file is missing: '{0}'", 
                        debuggerCommandLine));
                return;
            }

            ushort[] environmentPzz = StringArrayToPzz(environment);

            fixed (ushort* fixedEnvironmentPzz = environmentPzz)
            {
                pkgDebugSettings.EnableDebugging(packgeFullName, debuggerCommandLine, (IntPtr)fixedEnvironmentPzz);
            }

	        appActivationMgr.ActivateApplication(appUserModelId, appArgs, ACTIVATEOPTIONS.AO_NONE, out pid);
        }

        static private void ShowDevLicenseWarningIfNecessary()
        {
            CLRProfilerWindowsStoreAppHelper.Properties.Settings settings = new CLRProfilerWindowsStoreAppHelper.Properties.Settings();
                
            // Don't show warning if user unchecked the box
            if (settings.ShowDevLicenseWarning == "0")
                return;

            // Show warning
            DevLicenseWarningForm warningDlg = new DevLicenseWarningForm();
            warningDlg.ShowDialog();

            // Remember if user unchecked the box (i.e., to stop showing message)
            settings.ShowDevLicenseWarning = warningDlg.ShowThis ? "1" : "0";
            settings.Save();
        }

        // This tests permissions for WindowsStoreApps to be able to read / execute from
        // the directory containing the CLRProfiler/ProfilerObj binaries
        static public bool IsWindowsStoreAppAccessEnabledForProfiler(string dir)
        {
            ShowDevLicenseWarningIfNecessary();
            DirectoryInfo dirInfo = new DirectoryInfo(dir);
            DirectorySecurity dirSec = dirInfo.GetAccessControl();
            NTAccount appPackageId = new NTAccount("ALL APPLICATION PACKAGES");


            FileSystemAccessRule appPackageRule = new FileSystemAccessRule(
                appPackageId,
                FileSystemRights.ReadAndExecute | FileSystemRights.ListDirectory,
                InheritanceFlags.ContainerInherit | InheritanceFlags.ObjectInherit,
                PropagationFlags.None,
                AccessControlType.Allow);

            AuthorizationRuleCollection rules = dirSec.GetAccessRules(true, true, typeof(NTAccount));
            foreach (AuthorizationRule rule in rules)
            {
                if (!(rule is FileSystemAccessRule))
                    continue;

                FileSystemAccessRule fileRule = (FileSystemAccessRule)rule;
                if (!fileRule.IdentityReference.ToString().Contains(appPackageRule.IdentityReference.ToString()))
                    continue;

                if ((fileRule.FileSystemRights & appPackageRule.FileSystemRights) != appPackageRule.FileSystemRights)
                    continue;

                if (fileRule.AccessControlType != appPackageRule.AccessControlType)
                    continue;

                // Access check passes.  We're done
                return true;
            }

            // Access not enabled.  Display error message and inform caller
            MessageBox.Show(
                String.Format(
                    "CLRProfiler is being run from the directory '{0}', which the Windows Store app will be unable to access.  Please copy all of the CLRProfiler binaries to a subdirectory under Program Files or Program Files (x86), to ensure Windows Store apps can access them.",
                    dir));
            return false;
        }
    }
}
