using System;
using System.Collections;

namespace Demo1
{
    /// <summary>
    /// Demo detecting whether we're running under CLRProfiler
    /// Demo triggering heap dumps
    /// Instructions:
    ///   - Run under CLRProfiler
    ///   - After the run, do View/Heap Graph
    ///   - Note history feature - growth after first heap dump is red, older stuff is faded red.
    ///   - Right click and select "Show New Objects" - shows objects allocated between the two dumps
    ///     that are still live.
    ///   - Right click and select "Show Who Allocated New Objects" to get allocation graph for the
    ///     new objects.
    /// </summary>
    class Demo1
    {
        static Hashtable ht;

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            if (CLRProfilerControl.ProcessIsUnderProfiler)
            {
                Console.WriteLine("Process is running under CLRProfiler");
            }
            else
            {
                Console.WriteLine("Process is not running under CLRProfiler - exiting");
                return;
            }

            string.Format("");
            
            // Make sure we get rid of everything we can get rid of
            GC.Collect();
            GC.WaitForPendingFinalizers();

            // This will trigger another garbage collection and dump what's still live
            // This is our "before" snapshot.
            CLRProfilerControl.DumpHeap();

            ht = new Hashtable();
            for (int i = 0; i < 1000; i++)
            {
                ht[i] = string.Format("Value {0}", i);
            }

            // The memory is retained because ht is a static - enable the following statement
            // to make sure the garbage collector can clean up
            //ht = null;

            // Make sure we get rid of everything we can get rid of
            GC.Collect();
            GC.WaitForPendingFinalizers();

            // This will trigger another garbage collection and dump what's still live
            // This is our "after" snapshot.
            CLRProfilerControl.DumpHeap();

            Console.WriteLine("Press any key to exit");
            Console.ReadLine();
        }
    }
}
