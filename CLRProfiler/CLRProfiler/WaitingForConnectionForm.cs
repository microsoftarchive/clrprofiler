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

namespace CLRProfiler
{
    /// <summary>
    /// Summary description for WaitingForConnectionn.
    /// </summary>
    public class WaitingForConnectionForm : System.Windows.Forms.Form
    {
        private System.Windows.Forms.Button cancelButton;
        private RichTextBox messageTextBox;
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.Container components = null;

        public WaitingForConnectionForm()
        {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();

            //
            // TODO: Add any constructor code after InitializeComponent call
            //
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

        public void addMessage(string message)
        {
            messageTextBox.Text += "\n" + message;
        }

        public void setMessage(string message)
        {
            messageTextBox.Text = message;
        }

        #region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.cancelButton = new System.Windows.Forms.Button();
            this.messageTextBox = new System.Windows.Forms.RichTextBox();
            this.SuspendLayout();
            // 
            // cancelButton
            // 
            this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancelButton.Location = new System.Drawing.Point(192, 171);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(75, 23);
            this.cancelButton.TabIndex = 0;
            this.cancelButton.Text = "Cancel";
            // 
            // messageTextBox
            // 
            this.messageTextBox.Location = new System.Drawing.Point(12, 12);
            this.messageTextBox.Name = "messageTextBox";
            this.messageTextBox.ReadOnly = true;
            this.messageTextBox.Size = new System.Drawing.Size(429, 153);
            this.messageTextBox.TabIndex = 0;
            this.messageTextBox.Text = "Waiting for application to start common language runtime";
            // 
            // WaitingForConnectionForm
            // 
            this.CancelButton = this.cancelButton;
            this.ClientSize = new System.Drawing.Size(457, 200);
            this.Controls.Add(this.messageTextBox);
            this.Controls.Add(this.cancelButton);
            this.Name = "WaitingForConnectionForm";
            this.Text = "Waiting For Connection...";
            this.ResumeLayout(false);
            this.CancelButton = this.cancelButton;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
        }
        #endregion
    }
}
