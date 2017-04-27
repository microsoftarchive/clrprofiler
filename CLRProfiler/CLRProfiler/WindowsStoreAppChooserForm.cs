/* ==++==
 * 
 *   Copyright (c) Microsoft Corporation.  All rights reserved.
 * 
 * ==--==
 *
 * Class:  WindowsStoreAppChooserForm
 *
 * Description: Lists WindowsStoreApp packages installed for the current user,
 * and allows the user to pick a package + app to profile.
 */

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace CLRProfiler
{
    public partial class WindowsStoreAppChooserForm : Form
    {
        private List<PackageInfo> packageInfos;
        private string selectedAppUserModelId;
        private string selectedPackageFullName;
        private string selectedPackageTempDir;
        private string selectedAcSidString;
        private string selectedProcessFileName;

        public string SelectedAppUserModelId
        {
            get { return selectedAppUserModelId; }
        }

        public string SelectedPackageFullName
        {
            get { return selectedPackageFullName; }
        }

        public string SelectedPackageTempDir
        {
            get { return selectedPackageTempDir; }
        }

        public string SelectedAcSidString
        {
            get { return selectedAcSidString; }
        }

        public string SelectedProcessFileName
        {
            get { return selectedProcessFileName; }
        }

        public WindowsStoreAppChooserForm()
        {
            // Query OS for packages
            packageInfos = WindowsStoreAppHelperWrapper.GetPackagesForCurrentUser();
            
            // Initialize controls
            InitializeComponent();

            AcceptButton = buttonOK;
            CancelButton = buttonCancel;

            // Populate Package listbox with package info
            listBoxPackages.BeginUpdate();
            for (int i = 0; i < packageInfos.Count; i++)
            {
                listBoxPackages.Items.Add(packageInfos[i].fullName);
            }
            listBoxPackages.EndUpdate();
            listBoxPackages.SelectedIndex = 0;
        }


        private void listBoxPackages_SelectedIndexChanged(object sender, EventArgs e)
        {
            int iPackage = listBoxPackages.SelectedIndex;

            // Populate static text controls with details about the package
            textBoxLocation.Text = packageInfos[iPackage].installedLocation;
            labelArchitecture.Text = packageInfos[iPackage].architecture;
            labelIdlName.Text = packageInfos[iPackage].name;
            labelPublisher.Text = packageInfos[iPackage].publisher;
            labelVersion.Text = packageInfos[iPackage].version;

            // Populate App listbox with apps from package
            listBoxApps.BeginUpdate();
            listBoxApps.Items.Clear();
            for (int i = 0; i < packageInfos[iPackage].appInfoList.Count; i++)
            {
                listBoxApps.Items.Add(packageInfos[iPackage].appInfoList[i].exeName);
            }
            listBoxApps.EndUpdate();
            listBoxApps.SelectedIndex = 0;
            selectedPackageFullName = packageInfos[iPackage].fullName;
            selectedPackageTempDir = packageInfos[iPackage].tempDir;
            selectedAcSidString = packageInfos[iPackage].acSid;
        }

        private void listboxApps_SelectedIndexChanged(object sender, EventArgs e)
        {
            selectedAppUserModelId = packageInfos[listBoxPackages.SelectedIndex].appInfoList[listBoxApps.SelectedIndex].userModelId;
            selectedProcessFileName = Path.Combine(
                packageInfos[listBoxPackages.SelectedIndex].installedLocation,
                packageInfos[listBoxPackages.SelectedIndex].appInfoList[listBoxApps.SelectedIndex].exeName);
        }
    }
}
