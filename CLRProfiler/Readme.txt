CLRProfiler solution
====================

This solution consists of the following projects:

- CLRProfiler builds the GUI executable that is the profiler
- CLRProfilerWindowsStoreAppHelper contains all code that requires
  Windows 8 or higher, to deal with Windows Store apps.
- CLRProfilerWindowsStoreAppThreadResumer is a tiny component
  used to enable running a Windows Store app in debug mode so that
  environment variables may be passed to it.
- ProfilerOBJ builds a helper dll that is loaded into
  the application being profiled
- CLRProfilerControl is an optional component that allows
  the application being profiled to control the profiling.
