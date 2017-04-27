# CLRProfiler
CLRProfiler includes a number of useful views of the allocation profile, including a histogram of allocated types, a full heap graph, allocation call stacks, and a time line showing GCs of various generations and the resulting state of the managed heap after those collections.

## Installation
1) Download the CLRProfiler Binaries zip file from the latest [release:Release Page](97738).

2) Right-click the downloaded ZIP file, choose Properties, go to the General tab, and click **Unblock**.  This tells Windows that you trust the binaries contained in the ZIP file.

3) Copy the contents of the 32 and 64 folders onto your hard drive, and launch CLRProfiler.exe of the appropriate bitness to get started.  (The bitness of CLRProfiler.exe should match the bitness of the app you wish to profile.)

Note: When profiling Windows Store apps, be sure to copy the CLRProfiler binaries to subdirectories under **Program Files** and **Program Files (x86)** to ensure the Windows Store apps have access to the CLRProfiler binaries.

## More Information
For complete documentation, see the Word document included in the download.

For more information on using CLRProfiler, see the article [Profiling the .NET Garbage-Collected Heap](http://msdn.microsoft.com/en-us/magazine/ee309515.aspx).

Previous versions can be found on the Microsoft Download Center:
[CLRProfiler 1.1](http://www.microsoft.com/en-us/download/details.aspx?id=14727)
[CLRProfiler 2.0](http://www.microsoft.com/en-us/download/details.aspx?id=13382)
[CLRProfiler 4.0](http://www.microsoft.com/en-us/download/details.aspx?id=16273)

## Source Code
Each release also contains a download for the source code, which is of use to those of you writing your own profilers.  The source code is deployed as a Visual Studio 2012 solution.