using System;
using System.Collections;

namespace Demo3
{
    /// <summary>
    /// Demo turning allocation logging on and off to measure allocation between two points.
    /// 
    /// Instructions:
    ///  - Start with "Profiling Active" checkbox in CLRProfiler.exe UI in its *unchecked* state.
    ///  - The allocation graph that comes up after the run just shows what has been allocated
    ///    between turning the logging on and off.
    ///  - Start with "Profiling Active" checkbox the *checked* state for comparison.
    ///  - Now you also get all the allocations before the program even made it to Main.
    ///
    /// </summary>
    class Demo3
    {
        static Hashtable ht;

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            Console.WriteLine("Allocating some stuff with allocation logging off");
            for (int i = 0; i < 10000; i++)
            {
                Demo3 demo3 = new Demo3();
            }
            Console.WriteLine("Turning allocation logging on");
            // Turn allocation logging ON
            CLRProfilerControl.AllocationLoggingActive = true;
            ht = new Hashtable();
            for (int i = 0; i < 1000; i++)
            {
                ht[i] = string.Format("Value {0}", i);
            }
            // Turn allocation logging OFF
            CLRProfilerControl.AllocationLoggingActive = false;
        }
    }
}
