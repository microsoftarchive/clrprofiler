**** INTRODUCTION ****

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!! NOTE: This is intended as a sample only, and only as a sample for how to modify metadata &  !!! 
!!! IL, and use ReJIT.  This is not heavily tested or hardened to stressful or multithreaded    !!!
!!! conditions.  It may have bugs and could crash the program being profiled, or worse.         !!! 
!!! Use at your own risk.  And take care if you choose to adopt any of this code into           !!! 
!!! your own projects: add judicious error checking and do lots of stress testing.              !!! 
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

This is a sample intended to demonstrate how a ReJITing, IL-rewriting / instrumenting profiler
works.  The instrumentation it performs is to add hooks at the beginning and ending
of each method to take function timings.  However, this can be extended, as the
instrumentation infrastructure in this sample is fairly general-purpose.

**** COMPONENTS ****

ILRewriteGUI: A WPF app designed to control the profiler from out-of-process.  The
profiler user interacts with this app to turn profiling on and off, and to customize
which methods are instrumented.

ILRewriteProfiler: This is the profiler.  It is an unmanaged DLL that loads in-process into the
profilee (the process being profiled).  It consumes the CLR Profiling API to force
ReJITing, read original IL, and rewrite the IL instrumented with probes added.

ProfilerHelper: This is a managed assembly containing implementations of the managed
probes that are inserted into the profilee during instrumentation.

SampleApp: Just a C# sample app you can use as a profilee.  Any managed app can be
used as a profilee, though, not just this one.

**** MANAGED EXECUTION FLOW AFTER INSTRUMENTATION ****

Once the profiler has rewritten the IL, the execution flow looks like this:

Profilee Code (e.g., SampleApp) contains calls to probes -->
    Those probes (MgdEnteredFunction32, MgdExitedFunction32, etc.) are implemented
    in ProfilerHelper.dll if the GUI is set to "module", or
    in mscorlib.dll (dynamically) if the GUI is set to "mscorlib".  They call -->
        P/Invokes (NtvEnteredFunction, NtvExitedFunction), which are implemented
        in ProfilerHelper.dll if the GUI is set to "module", or
        in mscorlib.dll (dynamically) if the GUI is set to "mscorlib".  They call -->
            Native entrypoints (NtvEnteredFunction, NtvExitedFunction), which are implemented
            in the profiler, ILRewriteProfiler.dll.  These native entrypoints do the work of
            maintaining a shadow-stack and calculating the function timings.

**** REQUIREMENTS ****

This sample requires .NET Framework 4.5 or later.

**** RUNNING ****

To successfully run the profiler in your environment, you will need to do two things first:
1) Run regsvr32 on the appropriate bitness of the built ILRewriteProfiler.dll
2) Run gacutil on the appropriate bitness of the built ProfilerHelper.dll
    Examples:
    "C:\Program Files (x86)\Microsoft SDKs\Windows\v8.0A\bin\NETFX 4.0 Tools\gacutil.exe" /i <path>\ProfilerHelper\bin\x86\Debug\ProfilerHelper.dll
    "C:\Program Files (x86)\Microsoft SDKs\Windows\v8.0A\bin\NETFX 4.0 Tools\x64\gacutil.exe" /i <path>\ProfilerHelper\bin\x64\Debug\ProfilerHelper.dll

Once your environment is prepared, run ILRewriteGUI.exe
Click Open, and navigate to the app you wish to profile.  For this example, assume you chose SampleApp.exe
Click Launch, and the profilee will begin.
SampleApp.exe will spew some garbage to the console, and at some point will settle down and ask, "ARE YOU THERE? PRESS SOMETHING:"
ILRWP_RESULT.htm will be generated in the same directory as SampleApp.exe.
Open it in a web browser.  For now, it will be empty.
Now, let's pick some methods to instrument.  In ILRewriteGUI.exe, fill out these fields:
    Module: (leave as SampleApp.exe)
    Class: SampleApp.Two
    Functions: one two jaz lul
    Leave the buttons set to "rejitting" and "module"
In ILRewriteGUI.exe, hit "Send" (ReJIT request is sent)
Then in SampleApp.exe, press a key.
Let it run for a couple seconds as it prints garbage on the command prompt, and then
    refresh ILRWP_RESULT.htm in your browser.  You'll see a call graph with timings.
In ILRewriteGUI.exe, toggle "rejitting" to "reverting"
In ILRewriteGUI.exe, hit "Send"
Then in SampleApp.exe, press a key.
Let it run for a couple seconds, and then refresh ILRWP_RESULT.htm in your browser.  You'll notice no new entries in the call graph (only the prior entries remain).

You can also try the above with the "module" button toggled to "mscorlib".  "module"
means that instrumented code will call into ProfilerHelper.dll.  "mscorlib" means that
instrumented code will call into probes dynamically added into mscorlib.dll.

You can also open ILRWP_session.log (found in the same directory as SampleApp.exe) to read more verbose logging
done by the profiler.

**** GENERATED LOG FILES ****

The following log files are generated by the profiler as it runs.  These are all located in the same
dir as the exe.

ILRWP_RESULT.htm
This is the primary, human-readable output generated by the profiler, containing the call graphs and timings.
Target audience is the user of the profiler.

ILRWP_watchercommands.log
For communication from GUI -> Profiler.  GUI writes the user commands into this file, and the profiler
reads the commands from this file and runs them.

ILRWP_watcherresponse.log
For communication from Profiler -> GUI.  After running a command, the profiler writes its response into
this file, and the GUI reads the response from this file to show in its Status Log control.

ILRWP_session.log
Low-level diagnostics logging of what's going on in the profiler itself.  If the profiler encounters a
failure, this will contain info on what it was doing.
Target audience is the developer of the profiler.