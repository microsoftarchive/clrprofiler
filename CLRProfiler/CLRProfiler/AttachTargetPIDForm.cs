// ==++==
// 
//   Copyright (c) Microsoft Corporation.  All rights reserved.
// 
// ==--==
using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;

namespace CLRProfiler
{
    /// <summary>
    /// Summary description for AttachTargetPIDForm.
    /// </summary>
    public class AttachTargetPIDForm : System.Windows.Forms.Form
    {
        private System.Windows.Forms.Button cancelButton;
        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.TextBox PIDtextBox;
        private System.Windows.Forms.Label label1;
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.Container components = null;

        public AttachTargetPIDForm()
        {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();

            //
            // TODO: Add any constructor code after InitializeComponent call
            //
        }

        [SuppressMessage("Microsoft.Security", "CA2122:DoNotIndirectlyExposeMethodsWithLinkDemands", Justification = "CLRProfiler.exe is a stand-alone tool, not a library.")]
        public int GetPID()
        {
            int pid = 0;
            try
            {
                pid = Int32.Parse(PIDtextBox.Text);
                Process.GetProcessById(pid);
            }
            catch (Exception e)
            {
                MessageBox.Show( string.Format("The process ID ({0}) is not valid : {1} ", PIDtextBox.Text, e.Message) );
                pid = 0;
            }
            return pid;
        }

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (components != null)
                {
                    components.Dispose();
                }
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
            this.cancelButton = new System.Windows.Forms.Button();
            this.okButton = new System.Windows.Forms.Button();
            this.PIDtextBox = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // cancelButton
            // 
            this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancelButton.Location = new System.Drawing.Point(168, 104);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(75, 23);
            this.cancelButton.TabIndex = 2;
            this.cancelButton.Text = "Cancel";
            // 
            // okButton
            // 
            this.okButton.DialogResult = System.Windows.Forms.DialogResult.Yes;
            this.okButton.Location = new System.Drawing.Point(27, 104);
            this.okButton.Name = "okButton";
            this.okButton.Size = new System.Drawing.Size(88, 23);
            this.okButton.TabIndex = 1;
            this.okButton.Text = "OK";
            // 
            // PIDtextBox
            // 
            this.PIDtextBox.Location = new System.Drawing.Point(127, 44);
            this.PIDtextBox.Name = "PIDtextBox";
            this.PIDtextBox.Size = new System.Drawing.Size(133, 20);
            this.PIDtextBox.TabIndex = 0;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(19, 47);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(96, 13);
            this.label1.TabIndex = 9;
            this.label1.Text = "Attach Target PID:";
            // 
            // AttachTargetPIDForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(288, 163);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.PIDtextBox);
            this.Controls.Add(this.cancelButton);
            this.Controls.Add(this.okButton);
            this.Name = "AttachTargetPIDForm";
            this.Text = "Attach Target";
            this.ResumeLayout(false);
            this.PerformLayout();
            this.AcceptButton = this.okButton;
            this.CancelButton = this.cancelButton;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;

        }

        #endregion

    }
}
