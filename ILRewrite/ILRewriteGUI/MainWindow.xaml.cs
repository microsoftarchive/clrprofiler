// ==++==

//   Copyright (c) Microsoft Corporation.  All rights reserved.

// ==--==

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.IO;
using System.Security.Permissions;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Documents;
using System.Windows.Threading;

namespace ILRewriteGUI
{
	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	[PermissionSet(SecurityAction.Demand, Name = "FullTrust")]
	public partial class MainWindow : Window
	{
		#region Strings

		// Error messages.
		private const string INVALIDDATAERRORMESSAGE = "File input does not match expected format.";
		private const string IOEXCEPTIONRETRYMESSAGE = "File in use. Waiting, then trying again...";
		private const string IOEXCEPTIONFAILEDMESSAGE = "Error: could not open log.";
		private const string TIMEOUTRESPONSEMESSAGE = "Error: No response from the native watcher.";

		// Control strings, used to issue metacommands to interact directly with the managed listener.

		// Command strings
		private const string MSCORLIBCOMMAND = "Inserting into mscorlib: ";
		private const string QUITCOMMAND = "0>qa";
		private const string REJITCOMMAND = "pf";
		private const string REVERTCOMMAND = "rf";

		// Successful response strings
		private const string QUITRESPONSE = "0>qs";
		private const string REJITRESPONSE = "ps";
		private const string REVERTRESPONSE = "rs";

		// Unsuccessful response strings
		private const string QUITFAILED = "0>qf";
		private const string REJITFAILED = "pf";
		private const string REVERTFAILED = "rf";

		private const string QUITTEXT = @"\quit";
		private const string QUITOVERRIDETEXT = "Could not contact watcher. Shutting down anyways.";
		private const string EXITTEXT1 = "Native watcher shut down successfully.";
		private const string EXITTEXT2 = "Shutting down...";

		// Control characters.
		private const string WIPERESPONSECH = "-";
		private const string COMMANDIDSYMB = ">";
		private const string COMMANDSEPARATOR = "\t";

		// Command file will not change name. It shares the same directory as ResponseFile.
		private const string COMMANDFILE = @"ILRWP_watchercommands.log";
		private const string RESPONSEFILE = @"ILRWP_watcherresponse.log";

		#endregion

		#region Members

		private FileSystemWatcher responsewatcher;
		private uint commandID;
		private readonly Application application;

		// Paths for the files to watch and update.
		private string LogDirectory = @"";

		private Process app;

		#endregion

		public MainWindow()
		{
			InitializeComponent();

			// Start with a fresh log.
			commandID = 1;

			application = Application.Current;
		}

		#region Events

		/// <summary>
		/// Event handler for when the file being monitored is changed.
		/// </summary>
		/// <param name="sender">The FileSystemWatcher moitoring the file.</param>
		/// <param name="e">Information about the file that caused the event to be thrown.</param>
		private void responsewatcher_Changed(object sender, FileSystemEventArgs e)
		{
			string[] response = ReadResponseLog();
			for (int i = 0; i < response.Length; i++)
			{
				Log(response[i]);
			}
		}

		/// <summary>
		/// Event handler for when the file being monitored is deleted.
		/// </summary>
		/// <param name="sender">The FileSystemWatcher moitoring the file.</param>
		/// <param name="e">Information about the file that caused the event to be thrown.</param>
		private void responsewatcher_Deleted(object sender, FileSystemEventArgs e)
		{
			Log("Log wiped successfully.");
		}

		/// <summary>
		/// Event handler for when the browse button is clicked.
		/// </summary>
		/// <param name="sender">What triggered this event.</param>
		/// <param name="e">The routed information about this event.</param>
		private void browseButton_Click(object sender, RoutedEventArgs e)
		{
			// Configure open file dialog box
			Microsoft.Win32.OpenFileDialog openCommandsDialog = new Microsoft.Win32.OpenFileDialog();
			openCommandsDialog.FileName = @"";
			openCommandsDialog.DefaultExt = @".exe"; // Default file extension
			openCommandsDialog.Filter = @"Programs|*.exe"; // Filter files by extension

			// Show open file dialog box
			bool? result = openCommandsDialog.ShowDialog();

			// Process open file dialog box results
			if (result == true)
			{
				appPath.Text = openCommandsDialog.FileName;
			}
		}

		/// <summary>
		/// Event handler for when the launch button is clicked. Launches/terminates the app.
		/// </summary>
		/// <param name="sender">What triggered this event.</param>
		/// <param name="e">The routed information about this event.</param>
		private void launchButton_Click(object sender, RoutedEventArgs e)
		{
			// Logic flipped bc it flips it before calling the click event.
			if (launchButton.IsChecked == false)
			{
				// Launch the application
				Log("Launching " + appPath.Text + "...");
				initializeFile(appPath.Text);

				app = new Process();
				app.StartInfo.FileName = appPath.Text;
				app.StartInfo.UseShellExecute = false;
				app.StartInfo.EnvironmentVariables.Add("COR_ENABLE_PROFILING", "1");
				app.StartInfo.EnvironmentVariables.Add("COR_PROFILER", "{FA8F1DFF-0B62-4F84-887F-ECAC69A65DD3}");
				app.Start();
			}
			else
			{
				try
				{
					// Shut down the application
					Log("Shutting down " + appPath.Text + "...");
					responsewatcher.EnableRaisingEvents = false;
					responsewatcher = null;
					app.Kill();
				}
				catch (InvalidOperationException)
				{
					// Already gone!
				}
			}
		}

		/// <summary>
		/// Event handler for when the path is changed. tries to launch the listener.
		/// </summary>
		/// <param name="sender">What triggered this event.</param>
		/// <param name="e">The routed information about this event.</param>
		private void appPath_TextChanged(object sender, System.Windows.Controls.TextChangedEventArgs e)
		{
			launchButton.IsEnabled = File.Exists(appPath.Text);

			if (launchButton.IsEnabled && ModuleName.Text.Equals(String.Empty))
			{
				string[] temp = appPath.Text.Split('\\');

				// Auto-fill common names.
				ModuleName.Text = temp[temp.Length - 1];
				ClassName.Text = ModuleName.Text.Split('.')[0] + ".Program";
				FuncNames.Text = "Main";
			}
		}

		/// <summary>
		/// Event handler for when one of the text options has been changed. Checks to see if the Send button should be enabled.
		/// </summary>
		/// <param name="sender">The FileSystemWatcher moitoring the file.</param>
		/// <param name="e">Information about the file that caused the event to be thrown.</param>
		private void TextChanged(object sender, System.Windows.Controls.TextChangedEventArgs e)
		{
			SendButton.IsEnabled = !ModuleName.Text.Equals(String.Empty) &&
				!ClassName.Text.Equals(String.Empty) &&
				!FuncNames.Text.Equals(String.Empty);
		}

		/// <summary>
		/// Writes CommandText's text to the file being monitored by the native watcher.
		/// </summary>
		/// <param name="sender">What triggered this event.</param>
		/// <param name="e">The routed information about this event.</param>
		private void SendButton_Click(object sender, RoutedEventArgs e)
		{
			// Send the command to the native watcher.
			WriteCommandLog(DoReJIT.IsChecked.Value, ModuleName.Text, ClassName.Text, FuncNames.Text);
		}

		/// <summary>
		/// Event handler for pressing return inside a textbox to confirm send.
		/// </summary>
		/// <param name="sender">What triggered this event.</param>
		/// <param name="e">The routed information about this event.</param>
		private void TextBoxKeyDown(object sender, System.Windows.Input.KeyEventArgs e)
		{
			if (e.Key == System.Windows.Input.Key.Enter && SendButton.IsEnabled)
			{
				SendButton_Click(sender, e);
			}
		}

		/// <summary>
		/// Begins the process of exiting the watchers.
		/// </summary>
		/// <param name="sender">What triggered this event.</param>
		/// <param name="e">The routed information about this event.</param>
		private void QuitButton_Click(object sender, RoutedEventArgs e)
		{
			// Shut down the application
			try
			{
				Log("Shutting down " + appPath.Text + "...");
				responsewatcher = null;
                if (app != null)
                {
                    app.Kill();
                    app.Close();
                    app = null;
                }
                Close();
			}
			catch (Exception)
			{
				// Already done!
			}

			// All done!
			Application.Current.Shutdown();
		}

		#endregion

		#region Command Processing

		/// <summary>
		/// Get the new responses from the native watcher.
		/// </summary>
		/// <returns>Array of all the lines that were related to the present commandID.</returns>
		private string[] ReadResponseLog()
		{
			// Make sure there was time to finish writing.
			Thread.Sleep(100);

			string[] lines = null;

			bool done = false;
			for (int i = 0; i < 3 && !done; i++)
			{
				try
				{
					// We have gotten a new response from the native watcher.
					FileStream file = new FileStream(LogDirectory + RESPONSEFILE,
						FileMode.Open, FileAccess.Read, FileShare.ReadWrite);

					// Get the lines, using readonly file sharing
					string[] temp = new StreamReader(new FileStream(
						LogDirectory + RESPONSEFILE,
						FileMode.Open,
						FileAccess.Read,
						FileShare.ReadWrite))
						.ReadToEnd()
						.Split(new char[] { '\n', '\r' });

					lines = (from str in temp where !str.Equals(string.Empty) select str).ToArray<string>();

					if (lines.Length > 1 && lines[lines.Length - 1].Split('\t').Length < 4)
					{
						Thread.Sleep(100);
					}
					else
					{
						done = true;
					}
				}
				catch (System.IO.IOException)
				{
					// Should only be thrown from a concurrent file access issue. Try again in a second.
					Log(IOEXCEPTIONRETRYMESSAGE);

					// Wait a bit to let other process finish accessing the file.
					Thread.Sleep(1000);
				}
			}

			// Make sure we did in fact get something.
			if (!done || lines == null)
			{
				return new string[] { IOEXCEPTIONFAILEDMESSAGE };
			}

			// Check to see if native watcher quit.
			if (lines[0].Equals(QUITRESPONSE))
			{
				Log(EXITTEXT1);
				Log(EXITTEXT2);

				Thread.Sleep(1000); // Time for the user to read the quitting status.
				this.Dispatcher.Invoke(DispatcherPriority.Normal, new Action(() => {
					Application.Current.Shutdown();
				}));
			}

			char splitchar = COMMANDIDSYMB.ToCharArray()[0];

			uint commandCount = 0;

			// Read all the lines that were sent with the current commandID.
			List<string> response = new List<string>();
			for (int i = lines.Length - 1; i >= 0; i--)
			{
				string[] temp = lines[i].Split(splitchar);

				// We need there to be at least two fields; [0] = riid, others = result.
				if (temp.Length < 2)
				{
					continue;
				}

				// Get the riid to check if it's a line we want.
				uint riid;
				bool worked = UInt32.TryParse(temp[0], out riid);
				if (worked)
				{
					if (riid == commandID)
					{
						// This is a response to the current commandID

						// Rebuild the command, since we may have split too many times.
						string thisresponse = temp[1];
						for (int j = 2; j < temp.Length; j++)
						{
							thisresponse += splitchar + temp[j];
						}

						response.Insert(0, thisresponse);
						commandCount++;
					}
				}
				else
				{
					throw new InvalidDataException(INVALIDDATAERRORMESSAGE);
				}
			}

			// Command completed. We can send new commands again, with a new riid.
			if (response.Count > 0)
			{
				commandID += commandCount;
			}

			return response.ToArray();
		}

		/// <summary>
		/// Write a command to the command file.
		/// </summary>
		/// <param name="command">The command to be written to file. The commandID is not part of this.</param>
		private void WriteCommandLog(bool dorejit, string modulename, string classname, string funcnames)
		{
			string[] functions = funcnames.Split(' ');

			// Format along the lines of "...>pf\tmodule.dll\tclass\t..."
			string commandstart = COMMANDIDSYMB + (dorejit ? REJITCOMMAND : REVERTCOMMAND) +
				COMMANDSEPARATOR + modulename + COMMANDSEPARATOR + classname + COMMANDSEPARATOR;

			// Create the command strings
			string[] commands = new string[functions.Length];
			for (int i = 0; i < functions.Length; i++)
			{
				commands[i] = (commandID + i).ToString() + commandstart + functions[i];
			}

			// Write the commands to file
			using (StreamWriter writer = new StreamWriter(new FileStream(LogDirectory + COMMANDFILE,
				FileMode.Append, FileAccess.Write, FileShare.Read)))
			{
				for (int i = 0; i < commands.Length; i++)
				{
					writer.WriteLine(commands[i]);
				}
			}
		}

		#endregion

		#region Helper Methods

		/// <summary>
		/// Attempts to launch the response listener for the given file.
		/// </summary>
		/// <param name="filename">The full path to the application to watch.</param>
		/// <returns>Whether the launch was succesful.</returns>
		private bool initializeFile(string filename)
		{
			if (!File.Exists(filename))
				return false;

			try
			{
				// Open document
				appPath.Text = filename;

				// Extract the directory path and save the file name.
				LogDirectory = Path.GetDirectoryName(appPath.Text) + @"\";

				// Initialize the file watcher.
				responsewatcher = new FileSystemWatcher(LogDirectory, RESPONSEFILE);
				responsewatcher.NotifyFilter = NotifyFilters.LastWrite | NotifyFilters.Attributes;
				responsewatcher.Created += responsewatcher_Changed;
				responsewatcher.Changed += responsewatcher_Changed;
				responsewatcher.Deleted += responsewatcher_Deleted;
				responsewatcher.EnableRaisingEvents = true;

                File.WriteAllText(LogDirectory + COMMANDFILE, MSCORLIBCOMMAND + (DoMscorlib.IsChecked == true ? "t\r\n" : "f\r\n"));

				if (!ModuleName.Text.Equals(String.Empty) &&
					!ClassName.Text.Equals(String.Empty) &&
					!FuncNames.Text.Equals(String.Empty))
				{
					WriteCommandLog(DoReJIT.IsChecked.Value, ModuleName.Text, ClassName.Text, FuncNames.Text);
				}

				return true;
			}
			catch (FileNotFoundException)
			{
				return false;
			}
		}

		/// <summary>
		/// Append the text to the log.
		/// </summary>
		/// <param name="text">The string to be written to the log.</param>
		private void Log(string text)
		{
			// Command is sent from the watcher events (out of thread), so we need to use Dispatchers.
			ResponseText.Dispatcher.Invoke(DispatcherPriority.Normal, new Action(() => {
				ResponseText.Text = DateTime.Now.ToLongTimeString() + COMMANDIDSYMB + COMMANDSEPARATOR +
					text + Environment.NewLine + ResponseText.Text;
			}));
		}

		#endregion
	}
}