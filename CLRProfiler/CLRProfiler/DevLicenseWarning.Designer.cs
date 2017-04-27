namespace CLRProfiler
{
    partial class DevLicenseWarningForm
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
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.textBox2 = new System.Windows.Forms.TextBox();
            this.showThisCheckbox = new System.Windows.Forms.CheckBox();
            this.buttonOK = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // textBox1
            // 
            this.textBox1.BackColor = System.Drawing.SystemColors.Control;
            this.textBox1.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.textBox1.Location = new System.Drawing.Point(13, 13);
            this.textBox1.Multiline = true;
            this.textBox1.Name = "textBox1";
            this.textBox1.Size = new System.Drawing.Size(259, 68);
            this.textBox1.TabIndex = 1;
            this.textBox1.Text = "NOTE: If you have not yet done so, please install a Windows 8 Developer License o" +
    "n this machine.  Profiling Windows Store apps requires this license to be instal" +
    "led.";
            // 
            // textBox2
            // 
            this.textBox2.BackColor = System.Drawing.SystemColors.Control;
            this.textBox2.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.textBox2.Location = new System.Drawing.Point(12, 87);
            this.textBox2.Multiline = true;
            this.textBox2.Name = "textBox2";
            this.textBox2.Size = new System.Drawing.Size(259, 44);
            this.textBox2.TabIndex = 2;
            this.textBox2.Text = "Visual Studio installs this license automatically when you create a new Windows S" +
    "tore application project.";
            // 
            // showThisCheckbox
            // 
            this.showThisCheckbox.AutoSize = true;
            this.showThisCheckbox.Checked = true;
            this.showThisCheckbox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.showThisCheckbox.Location = new System.Drawing.Point(13, 137);
            this.showThisCheckbox.Name = "showThisCheckbox";
            this.showThisCheckbox.Size = new System.Drawing.Size(123, 17);
            this.showThisCheckbox.TabIndex = 3;
            this.showThisCheckbox.Text = "Show this every time";
            this.showThisCheckbox.UseVisualStyleBackColor = true;
            this.showThisCheckbox.CheckedChanged += new System.EventHandler(this.showThisCheckbox_CheckedChanged);
            // 
            // buttonOK
            // 
            this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOK.Location = new System.Drawing.Point(197, 149);
            this.buttonOK.Name = "buttonOK";
            this.buttonOK.Size = new System.Drawing.Size(75, 23);
            this.buttonOK.TabIndex = 0;
            this.buttonOK.Text = "OK";
            this.buttonOK.UseVisualStyleBackColor = true;
            // 
            // DevLicenseWarningForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 184);
            this.Controls.Add(this.buttonOK);
            this.Controls.Add(this.showThisCheckbox);
            this.Controls.Add(this.textBox2);
            this.Controls.Add(this.textBox1);
            this.Name = "DevLicenseWarningForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Developer License";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.TextBox textBox2;
        private System.Windows.Forms.CheckBox showThisCheckbox;
        private System.Windows.Forms.Button buttonOK;

    }
}