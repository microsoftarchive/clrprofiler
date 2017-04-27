namespace CLRProfiler
{
    partial class HelpForm
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
            this.helpText = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // helpText
            // 
            this.helpText.AcceptsReturn = true;
            this.helpText.AccessibleName = "helpTextBox";
            this.helpText.CausesValidation = false;
            this.helpText.Cursor = System.Windows.Forms.Cursors.Arrow;
            this.helpText.Dock = System.Windows.Forms.DockStyle.Fill;
            this.helpText.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.helpText.Location = new System.Drawing.Point(0, 0);
            this.helpText.Multiline = true;
            this.helpText.Name = "helpText";
            this.helpText.ReadOnly = true;
            this.helpText.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.helpText.Size = new System.Drawing.Size(665, 590);
            this.helpText.TabIndex = 0;
            this.helpText.TabStop = false;
            this.helpText.WordWrap = false;
            // 
            // HelpForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(665, 590);
            this.Controls.Add(this.helpText);
            this.Name = "HelpForm";
            this.Text = "CLRProfiler Help";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        public System.Windows.Forms.TextBox helpText;

    }
}