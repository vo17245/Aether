using System.Runtime.InteropServices;

namespace ScriptFixture;

public static class EntryPoints
{
    [UnmanagedFunctionPointer(CallingConvention.Winapi)]
    private delegate int NativeBinary(int left, int right);
    private static NativeBinary? s_Native;
    private static int s_Count;

    [UnmanagedCallersOnly]
    public static int Add(int left, int right) => unchecked(left + right);

    [UnmanagedCallersOnly]
    public static void Notify() => s_Count++;

    [UnmanagedCallersOnly]
    public static int Count() => s_Count;

    [UnmanagedCallersOnly]
    public static int Utf8Length(IntPtr bytes, int length)
    {
        try
        {
            return Marshal.PtrToStringUTF8(bytes, length)?.Length ?? -1;
        }
        catch { return -2; }
    }

    [UnmanagedCallersOnly]
    public static int RegisterNative(IntPtr callback)
    {
        if (callback == IntPtr.Zero) return -1;
        try
        {
            s_Native = Marshal.GetDelegateForFunctionPointer<NativeBinary>(callback);
            return 0;
        }
        catch { return -2; }
    }

    [UnmanagedCallersOnly]
    public static int RejectNative(IntPtr callback) => -17;

    [UnmanagedCallersOnly]
    public static int CallNative(int left, int right)
    {
        try { return s_Native?.Invoke(left, right) ?? -1; }
        catch { return -2; }
    }

    public static int OrdinaryMethod() => 42;
}
