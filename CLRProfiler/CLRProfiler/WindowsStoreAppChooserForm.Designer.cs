namespace CLRProfiler
{
    partial class WindowsStoreAppChooserForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.listBoxPackages = new System.Windows.Forms.ListBox();
            this.groupBoxPackages = new System.Windows.Forms.GroupBox();
            this.textBoxLocation = new System.Windows.Forms.TextBox();
            this.label17 = new System.Windows.Forms.Label();
            this.labelVersion = new System.Windows.Forms.Label();
            this.labelIdlName = new System.Windows.Forms.Label();
            this.labelPublisher = new System.Windows.Forms.Label();
            this.label11 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.labelArchitecture = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.buttonOK = new System.Windows.Forms.Button();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.listBoxApps = new System.Windows.Forms.ListBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.groupBoxPackages.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // listBoxPackages
            // 
            this.listBoxPackages.FormattingEnabled = true;
            this.listBoxPackages.Location = new System.Drawing.Point(10, 20);
            this.listBoxPackages.Name = "listBoxPackages";
            this.listBoxPackages.Size = new System.Drawing.Size(524, 121);
            this.listBoxPackages.TabIndex = 1;
            this.listBoxPackages.SelectedIndexChanged += new System.EventHandler(this.listBoxPackages_SelectedIndexChanged);
            // 
            // groupBoxPackages
            // 
            this.groupBoxPackages.Controls.Add(this.textBoxLocation);
            this.groupBoxPackages.Controls.Add(this.label17);
            this.groupBoxPackages.Controls.Add(this.labelVersion);
            this.groupBoxPackages.Controls.Add(this.listBoxPackages);
            this.groupBoxPackages.Controls.Add(this.labelIdlName);
            this.groupBoxPackages.Controls.Add(this.labelPublisher);
            this.groupBoxPackages.Controls.Add(this.label11);
            this.groupBoxPackages.Controls.Add(this.label8);
            this.groupBoxPackages.Controls.Add(this.label6);
            this.groupBoxPackages.Controls.Add(this.labelArchitecture);
            this.groupBoxPackages.Controls.Add(this.label2);
            this.groupBoxPackages.Location = new System.Drawing.Point(16, 12);
            this.groupBoxPackages.Name = "groupBoxPackages";
            this.groupBoxPackages.Size = new System.Drawing.Size(540, 260);
            this.groupBoxPackages.TabIndex = 2;
            this.groupBoxPackages.TabStop = false;
            this.groupBoxPackages.Text = "1: Select a package to profile";
            // 
            // textBoxLocation
            // 
            this.textBoxLocation.BackColor = System.Drawing.SystemColors.Control;
            this.textBoxLocation.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.textBoxLocation.Location = new System.Drawing.Point(108, 144);
            this.textBoxLocation.Multiline = true;
            this.textBoxLocation.Name = "textBoxLocation";
            this.textBoxLocation.ReadOnly = true;
            this.textBoxLocation.Size = new System.Drawing.Size(426, 20);
            this.textBoxLocation.TabIndex = 18;
            this.textBoxLocation.Text = "----";
            // 
            // label17
            // 
            this.label17.AutoSize = true;
            this.label17.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label17.Location = new System.Drawing.Point(7, 144);
            this.label17.Name = "label17";
            this.label17.Size = new System.Drawing.Size(90, 13);
            this.label17.TabIndex = 17;
            this.label17.Text = "Installed Location";
            // 
            // labelVersion
            // 
            this.labelVersion.AutoEllipsis = true;
            this.labelVersion.AutoSize = true;
            this.labelVersion.Cursor = System.Windows.Forms.Cursors.Default;
            this.labelVersion.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelVersion.Location = new System.Drawing.Point(105, 229);
            this.labelVersion.Name = "labelVersion";
            this.labelVersion.Size = new System.Drawing.Size(19, 13);
            this.labelVersion.TabIndex = 15;
            this.labelVersion.Text = "----";
            // 
            // labelIdlName
            // 
            this.labelIdlName.AutoEllipsis = true;
            this.labelIdlName.AutoSize = true;
            this.labelIdlName.Cursor = System.Windows.Forms.Cursors.Default;
            this.labelIdlName.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelIdlName.Location = new System.Drawing.Point(105, 185);
            this.labelIdlName.Name = "labelIdlName";
            this.labelIdlName.Size = new System.Drawing.Size(19, 13);
            this.labelIdlName.TabIndex = 13;
            this.labelIdlName.Text = "----";
            // 
            // labelPublisher
            // 
            this.labelPublisher.AutoEllipsis = true;
            this.labelPublisher.AutoSize = true;
            this.labelPublisher.Cursor = System.Windows.Forms.Cursors.Default;
            this.labelPublisher.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelPublisher.Location = new System.Drawing.Point(105, 207);
            this.labelPublisher.Name = "labelPublisher";
            this.labelPublisher.Size = new System.Drawing.Size(19, 13);
            this.labelPublisher.TabIndex = 11;
            this.labelPublisher.Text = "----";
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label11.Location = new System.Drawing.Point(6, 229);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(42, 13);
            this.label11.TabIndex = 9;
            this.label11.Text = "Version";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label8.Location = new System.Drawing.Point(6, 185);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(93, 13);
            this.label8.TabIndex = 6;
            this.label8.Text = "Package Id Name";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label6.Location = new System.Drawing.Point(6, 207);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(50, 13);
            this.label6.TabIndex = 4;
            this.label6.Text = "Publisher";
            // 
            // labelArchitecture
            // 
            this.labelArchitecture.AutoEllipsis = true;
            this.labelArchitecture.AutoSize = true;
            this.labelArchitecture.Cursor = System.Windows.Forms.Cursors.Default;
            this.labelArchitecture.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelArchitecture.Location = new System.Drawing.Point(105, 164);
            this.labelArchitecture.Name = "labelArchitecture";
            this.labelArchitecture.Size = new System.Drawing.Size(19, 13);
            this.labelArchitecture.TabIndex = 0;
            this.labelArchitecture.Text = "----";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label2.Location = new System.Drawing.Point(6, 164);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(64, 13);
            this.label2.TabIndex = 0;
            this.label2.Text = "Architecture";
            // 
            // buttonOK
            // 
            this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOK.Location = new System.Drawing.Point(383, 403);
            this.buttonOK.Name = "buttonOK";
            this.buttonOK.Size = new System.Drawing.Size(92, 23);
            this.buttonOK.TabIndex = 3;
            this.buttonOK.Text = "Start Profiling";
            this.buttonOK.UseVisualStyleBackColor = true;
            // 
            // buttonCancel
            // 
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonCancel.Location = new System.Drawing.Point(481, 403);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 4;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            // 
            // listBoxApps
            // 
            this.listBoxApps.FormattingEnabled = true;
            this.listBoxApps.Location = new System.Drawing.Point(10, 28);
            this.listBoxApps.Name = "listBoxApps";
            this.listBoxApps.Size = new System.Drawing.Size(524, 56);
            this.listBoxApps.TabIndex = 1;
            this.listBoxApps.SelectedIndexChanged += new System.EventHandler(this.listboxApps_SelectedIndexChanged);
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.listBoxApps);
            this.groupBox2.Location = new System.Drawing.Point(16, 293);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(540, 104);
            this.groupBox2.TabIndex = 18;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "2: Select an app to profile";
            // 
            // WindowsStoreAppChooserForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(568, 441);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.buttonCancel);
            this.Controls.Add(this.buttonOK);
            this.Controls.Add(this.groupBoxPackages);
            this.Name = "WindowsStoreAppChooserForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Select package and application";
            this.groupBoxPackages.ResumeLayout(false);
            this.groupBoxPackages.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListBox listBoxPackages;
        private System.Windows.Forms.GroupBox groupBoxPackages;
        private System.Windows.Forms.Button buttonOK;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label labelVersion;
        private System.Windows.Forms.Label labelIdlName;
        private System.Windows.Forms.Label labelPublisher;
        private System.Windows.Forms.Label labelArchitecture;
        private System.Windows.Forms.Label label17;
        private System.Windows.Forms.ListBox listBoxApps;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.TextBox textBoxLocation;
    }
}