using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;

namespace Aether.Script;

// Only this bridge lives in the default context. Each native Runtime owns a
// collectible context; no script types are cached outside its session.
public static unsafe class Bridge
{
    private sealed class Context : AssemblyLoadContext
    {
        internal readonly Dictionary<string, Assembly> LoadedAssemblies = new(StringComparer.Ordinal);
        private readonly List<AssemblyDependencyResolver> resolvers = new();
        internal readonly List<MethodInfo> Methods = new();
        internal Context() : base(isCollectible: true) { }
        internal Assembly LoadFile(string path)
        {
            path = Path.GetFullPath(path);
            var name = AssemblyName.GetAssemblyName(path).Name!;
            if (LoadedAssemblies.TryGetValue(name, out var existing)) return existing;
            resolvers.Add(new AssemblyDependencyResolver(path));
            using var stream = File.OpenRead(path);
            var assembly = LoadFromStream(stream); // releases DLL locks immediately
            LoadedAssemblies.Add(name, assembly);
            return assembly;
        }
        protected override Assembly? Load(AssemblyName name)
        {
            if (name.Name != null && LoadedAssemblies.TryGetValue(name.Name, out var existing)) return existing;
            foreach (var resolver in resolvers)
            {
                var path = resolver.ResolveAssemblyToPath(name);
                if (path != null) return LoadFile(path);
            }
            return null; // framework assemblies are shared with CoreCLR
        }
        protected override IntPtr LoadUnmanagedDll(string name)
        {
            foreach (var resolver in resolvers)
            {
                var path = resolver.ResolveUnmanagedDllToPath(name);
                if (path != null) return LoadUnmanagedDllFromPath(path);
            }
            return IntPtr.Zero;
        }
    }
    private static Context Session(IntPtr handle) => (Context)GCHandle.FromIntPtr(handle).Target!;
    private static string Text(byte* value) => Marshal.PtrToStringUTF8((IntPtr)value)!;
    private static int Error(Exception ex, byte* buffer, int length)
    {
        if (buffer != null && length > 0)
        {
            var bytes = System.Text.Encoding.UTF8.GetBytes(ex.Message);
            var count = Math.Min(bytes.Length, length - 1);
            bytes.AsSpan(0, count).CopyTo(new Span<byte>(buffer, count));
            buffer[count] = 0;
        }
        return -1;
    }
    [UnmanagedCallersOnly]
    public static IntPtr Create()
    {
        try { return GCHandle.ToIntPtr(GCHandle.Alloc(new Context())); }
        catch { return IntPtr.Zero; }
    }
    [UnmanagedCallersOnly]
    public static int Load(IntPtr handle, byte* path, byte* error, int length)
    {
        try { Session(handle).LoadFile(Text(path)); return 0; }
        catch (Exception ex) { return Error(ex, error, length); }
    }
    [UnmanagedCallersOnly]
    public static int Resolve(IntPtr handle, byte* typeName, byte* methodName, IntPtr* address, byte* error, int length)
    {
        try
        {
            var session = Session(handle);
            var type = Type.GetType(Text(typeName),
                name => session.LoadedAssemblies.TryGetValue(name.Name!, out var a) ? a : session.LoadFromAssemblyName(name),
                (assembly, name, ignoreCase) => assembly?.GetType(name, false, ignoreCase), true)!;
            var method = type.GetMethod(Text(methodName), BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic)
                ?? throw new MissingMethodException(type.FullName, Text(methodName));
            if (method.ContainsGenericParameters || method.GetCustomAttribute<UnmanagedCallersOnlyAttribute>() == null)
                throw new InvalidOperationException("Script entry must be static, nongeneric and [UnmanagedCallersOnly]");
            *address = method.MethodHandle.GetFunctionPointer();
            session.Methods.Add(method); // root the entry while native callable values exist
            return 0;
        }
        catch (Exception ex) { *address = IntPtr.Zero; return Error(ex, error, length); }
    }
    [UnmanagedCallersOnly]
    public static void Release(IntPtr handle)
    {
        try
        {
            var gc = GCHandle.FromIntPtr(handle);
            var session = (Context)gc.Target!;
            gc.Free();
            session.Methods.Clear();
            session.LoadedAssemblies.Clear();
            session.Unload(); // cooperative; final collection is performed by CoreCLR
        }
        catch { } // destructor boundary
    }
}
