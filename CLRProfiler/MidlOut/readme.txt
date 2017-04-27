CLRProfilerWindowsStoreAppHelper uses some Windows Runtime and COM APIs from managed code.
This directory includes copies of a couple IDL files, plus generated
type libraries and assemblies added as references to CLRProfilerWindowsStoreAppHelper.dll

*** ShObjIdlModified.idl:

This is a mildly modified version of ShObjIdl.idl (found in
\Program Files (x86)\Windows Kits\8.0\Include\um), where
IPackageDebugSettings::EnableDebugging is changed to accept type
void * for its "environment" parameter.  This is done simply to make
C# interop easier.  No semantic change results.

The C# assembly was generated via:
midl ShObjIdlModified.idl /out <path>\CLRProfiler\MidlOut
tlbimp ShObjIdlModified.tlb /namespace:Shell /out:ShObjIdlModifiedTlb.dll

*** AppxPackaging.idl:

A duplicate of the IDL normally found in
\Program Files (x86)\Windows Kits\8.0\Include\winrt.

The C# assembly was generated via:
midl AppxPackaging.idl /out <path>\CLRProfiler\MidlOut
tlbimp AppxPackaging.tlb /namespace:AppxPackaging /out:AppxPackagingTlb.dll
