using System;
using System.Runtime.InteropServices;
using System.Diagnostics;

public class CLRProfilerControl
{
    [DllImport("ProfilerOBJ.dll", CharSet=CharSet.Unicode)]
    private static extern void LogComment(string comment);

    [DllImport("ProfilerOBJ.dll")]
    private static extern bool GetAllocationLoggingActive();

    [DllImport("ProfilerOBJ.dll")]
    private static extern void SetAllocationLoggingActive(bool active);

    [DllImport("ProfilerOBJ.dll")]
    private static extern bool GetCallLoggingActive();

    [DllImport("ProfilerOBJ.dll")]
    private static extern void SetCallLoggingActive(bool active);

    [DllImport("ProfilerOBJ.dll")]
    private static extern bool DumpHeap(uint timeOut);

    private static bool processIsUnderProfiler;

    public static void LogWriteLine(string comment)
    {
        if (processIsUnderProfiler)
        {
            LogComment(comment);
			if (comment == killProcessMarker)
				Process.GetCurrentProcess().Kill();
        }
    }

    public static void LogWriteLine(string format, params object[] args)
    {
        if (processIsUnderProfiler)
        {
            LogComment(string.Format(format, args));
        }
    }

    public static bool AllocationLoggingActive
    {
        get
        {
            if (processIsUnderProfiler)
                return GetAllocationLoggingActive();
            else
                return false;
        }
        set
        {
            if (processIsUnderProfiler)
                SetAllocationLoggingActive(value);
        }
    }

    public static bool CallLoggingActive
    {
        get
        {
            if (processIsUnderProfiler)
                return GetCallLoggingActive();
            else
                return false;
        }
        set
        {
            if (processIsUnderProfiler)
                SetCallLoggingActive(value);
        }
    }

    public static void DumpHeap()
    {
        if (processIsUnderProfiler)
        {
            if (!DumpHeap(60*1000))
                throw new Exception("Failure to dump heap");
        }
    }

    public static bool ProcessIsUnderProfiler
    {
        get { return processIsUnderProfiler; }
    }

	static string killProcessMarker;

    static CLRProfilerControl()
    {
        try
        {
            // if AllocationLoggingActive does something, this implies profilerOBJ.dll is attached
            // and initialized properly
            bool active = GetAllocationLoggingActive();
            SetAllocationLoggingActive(!active);
            processIsUnderProfiler = active != GetAllocationLoggingActive();
            SetAllocationLoggingActive(active);
			killProcessMarker = Environment.GetEnvironmentVariable("OMV_KILLPROCESS_MARKER");
        }
        catch (DllNotFoundException)
        {
        }
    }
}