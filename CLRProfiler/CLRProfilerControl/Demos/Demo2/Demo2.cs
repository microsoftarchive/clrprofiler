using System;
using System.Collections;

namespace Demo2
{
    /// <summary>
    /// Demo detecting memory leak with comments in the file and heap dump
    /// 
    /// Instructions:
    ///  - Run under CLRProfiler
    ///  - After program has terminated, select "View/Heap Graph".
    ///  - Right-click and select "Show Object Allocated between..."
    ///  - Select "Before loop..." as the start and "After loop..." as the end of the range.
    ///  - Note how the hash table, the buckets and the boxed ints have survived, but the strings have not,
    ///    as the second loop has replaced them.
    ///  - Close that graph, then in the original heap graph select "Show Object Allocated between..." again.
    ///  - This time select the range "After loop..." to "After second loop...".
    ///    Note how in this graph there are only numbers for the string objects (only these were allocated),
    ///    the hash table and the buckets only hold on to the strings, but weren't allocated in the period.
    ///  - Selecting "View/Comments" gives you a list of the comments in the log file. In this example, you should see
    ///      Before loop - time = 820 milliseconds
    ///      After loop - time = 840 milliseconds
    ///      After second loop - time = 871 milliseconds
    ///    with the milliseconds value differing between runs, obviously.
    ///  - Selecting "View/Time Line" shows the comments time points as thin green lines. Hovering the mouse cursor over them
    ///    shows the comment text in the tool tip that appears.
    /// </summary>
    class Demo2
    {
        static Hashtable ht;

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            // Set our "before" marker in the log file
            CLRProfilerControl.LogWriteLine("Before loop - time = {0} milliseconds", DateTime.Now.Millisecond);

            ht = new Hashtable();
            for (int i = 0; i < 1000; i++)
            {
                ht[i] = string.Format("Value {0}", i);
            }

            // Set our "after" marker in the log file
            CLRProfilerControl.LogWriteLine("After loop - time = {0} milliseconds", DateTime.Now.Millisecond);

            // Do some more to make things more interesting...
            for (int i = 0; i < 1000; i++)
            {
                ht[i] = string.Format("Value {0}", i);
            }
            
            // Set another "after" marker in the log file
            CLRProfilerControl.LogWriteLine("After second loop - time = {0} milliseconds", DateTime.Now.Millisecond);

            // The memory is retained because ht is a static - enable the following statement
            // to make sure the garbage collector can clean up
            // ht = null;

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
