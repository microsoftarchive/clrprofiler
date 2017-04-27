# Writing Your Own Profiler
On the latest [release:Release Page](97738), you will find the source code for both **CLRProfiler** and **ILRewrite**.  The former demonstrates how to write a complete memory profiler, including support for profiling Windows Store apps.  The latter demonstrates how to use the ReJIT methods of the CLR Profiling API to rewrite (i.e., instrument) IL.

We also encourage you to familiarize yourself with the MSDN documentation on the CLR Profiling API:
[Profiling (Unmanaged API Reference)](http://msdn.microsoft.com/en-us/library/ms404386.aspx)
[CLR Profiling API Blog](http://blogs.msdn.com/b/davbr)
[Building Development and Diagnostic Tools for .Net Forum](http://social.msdn.microsoft.com/forums/en-US/netfxtoolsdev/threads)

Other samples:
[CLR V4 Profiling API Attach Trigger Sample](http://archive.msdn.microsoft.com/ProfilerAttacher)
[Test harness for loading in-process side-by-side CLR instances](http://archive.msdn.microsoft.com/RunSxS)
[SigParse - Sample code to parse CLR metadata signatures](http://archive.msdn.microsoft.com/sigparse)
