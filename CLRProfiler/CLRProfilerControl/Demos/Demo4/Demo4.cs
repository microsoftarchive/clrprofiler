using System;
using System.Collections;

namespace Demo4
{
    /// <summary>
    /// Demo turning call logging on and off to get a call graph between two points.
    /// 
    /// Instructions:
    ///  - In CLRProfiler, uncheck Profile:/Calls. Start Demo4.exe.
    ///  - After the program has terminated, select "View/Call Graph". This gives you a call graph for everything
    ///    that has been called between the two points.
    /// </summary>
    class Demo4
    {
        static Hashtable ht;

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            // Turn call logging ON
            CLRProfilerControl.CallLoggingActive = true;
            ht = new Hashtable();
            for (int i = 0; i < 1000; i++)
            {
                ht[i] = string.Format("Value {0}", i);
            }
            // Turn allocation logging OFF
            CLRProfilerControl.CallLoggingActive = false;
        }
    }
}
