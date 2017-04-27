// ==++==
// 
//   Copyright (c) Microsoft Corporation.  All rights reserved.
// 
// ==--==
using System;
using System.Runtime.InteropServices;

namespace CLRProfiler
{
    /// <summary>
    /// Summary description for NamedManualResetEvent.
    /// </summary>
    public class NamedManualResetEvent: IDisposable
    {
        // Track whether Dispose has been called.
        private bool disposed = false;

        // Implement IDisposable.
        // Do not make this method virtual.
        // A derived class should not be able to override this method.
        public void Dispose()
        {
            Dispose(true);

            // This object will be cleaned up by the Dispose method.
            // Therefore, you should call GC.SupressFinalize to
            // take this object off the finalization queue
            // and prevent finalization code for this object
            // from executing a second time.
            GC.SuppressFinalize(this);
        }

        // Dispose(bool disposing) executes in two distinct scenarios.
        // If disposing equals true, the method has been called directly
        // or indirectly by a user's code. Managed and unmanaged resources
        // can be disposed.
        // If disposing equals false, the method has been called by the
        // runtime from inside the finalizer and you should not reference
        // other objects. Only unmanaged resources can be disposed.
        private void Dispose(bool disposing)
        {
            // Check to see if Dispose has already been called.
            if (!this.disposed)
            {
                if (eventHandle != IntPtr.Zero)
                    CloseHandle(eventHandle);

                eventHandle = IntPtr.Zero;

                // Note disposing has been done.
                disposed = true;
            }
        }

        ~NamedManualResetEvent()
        {
            Dispose(false);
        }

        private IntPtr eventHandle;

        struct SECURITY_ATTRIBUTES
        {
            public uint   nLength; 
            public IntPtr lpSecurityDescriptor; 
            public int    bInheritHandle; 
        }; 

        public NamedManualResetEvent(string eventName, bool initialState, bool createEvent)
        {
            SECURITY_ATTRIBUTES sa;
            sa.nLength = 12;
            sa.bInheritHandle = 0;
            if (!ConvertStringSecurityDescriptorToSecurityDescriptor("D: (A;OICI;GRGWGXSDWDWO;;;AU)", 1, out sa.lpSecurityDescriptor, IntPtr.Zero))
                throw new Exception("ConvertStringSecurityDescriptorToSecurityDescriptor returned error");

            if (createEvent)
            {
                eventHandle = CreateEvent(ref sa, true, initialState, eventName);
                LocalFree(sa.lpSecurityDescriptor);
                if (eventHandle == IntPtr.Zero)
                {
                    eventHandle = OpenEvent(0x00100002, false, eventName);
                    if (eventHandle == IntPtr.Zero)
                        throw new Exception(string.Format("Couldn't create or open event {0}", eventName));
                }
            }
            else
            {
                eventHandle = OpenEvent(0x00100002, false, eventName);
            }
        }

        public bool IsValid()
        {
            return eventHandle != IntPtr.Zero;
        }

        public bool Reset()
        {
            if (eventHandle == IntPtr.Zero)
                throw new Exception("Event handle is not valid");

            return ResetEvent(eventHandle);
        }

        public bool Set()
        {
            if (eventHandle == IntPtr.Zero)
                throw new Exception("Event handle is not valid");

            return SetEvent(eventHandle);
        }

        public bool Wait(int timeOut)
        {
            if (eventHandle == IntPtr.Zero)
                throw new Exception("Event handle is not valid");

            return WaitForSingleObject(eventHandle, timeOut) == 0;
        }

        [DllImport("Advapi32.dll")]
        private static extern bool ConvertStringSecurityDescriptorToSecurityDescriptor(
            string StringSecurityDescriptor,
            uint StringSDRevision,
            out IntPtr SecurityDescriptor,
            IntPtr SecurityDescriptorSize
            );

        [DllImport("Kernel32.dll")]
        private static extern bool LocalFree(IntPtr ptr);

        [DllImport("Kernel32.dll", CharSet=CharSet.Auto)]
        private static extern IntPtr CreateEvent(ref SECURITY_ATTRIBUTES eventAttributes, bool manualReset, bool initialState, string eventName);

        [DllImport("Kernel32.dll", CharSet=CharSet.Auto)]
        private static extern IntPtr OpenEvent(uint desiredAccess, bool inheritHandle, string eventName);

        [DllImport("Kernel32.dll")]
        private static extern bool ResetEvent(IntPtr eventHandle);

        [DllImport("Kernel32.dll")]
        private static extern bool SetEvent(IntPtr eventHandle);

        [DllImport("Kernel32.dll")]
        private static extern bool CloseHandle(IntPtr eventHandle);

        [DllImport("Kernel32.dll")]
        private static extern int WaitForSingleObject(IntPtr handle, int milliseconds);
    }
}
