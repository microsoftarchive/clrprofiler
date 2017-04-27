CLRProfilerControl
=========================

This is an optional tiny managed dll that allows you to control profiling with CLRProfiler from the application being profiled.

It has the following interface:

public class CLRProfilerControl
{
    // These methods write comments into the CLRProfiler log file.
    // You can use this to log details of your test (e.g. "Testing version 0.99 on May 9, 2003")
    // or to mark instants in time (e.g. "About to enter foo.bar()")
    // In CLRProfiler, these show up in View/Comments (as Text) and 
    // View/Time Line (as thin green vertical lines).
    // In View/Time Line, you can see the text if hover the mouse over the line.
    public static void LogWriteLine(string comment)
    public static void LogWriteLine(string format, params object[] args)

    // Read/write property that controls whether (managed) allocations get logged
    public static bool AllocationLoggingActive { get { }  set { } }

    // Read/write property that controls whether (managed) method calls get logged
    public static bool CallLoggingActive { get { } set { } }

    // Dump the objects on the (managed) heap - later visible in CLRProfiler via View/Heap Graph
    public static void DumpHeap() { }

    // Readonly property that allows the application to check whether it's being
    // profiled by CLRProfiler.
    public static bool ProcessIsUnderProfiler { get {  } }
}


See the Demos folder for examples demonstrating the usage of these APIs.