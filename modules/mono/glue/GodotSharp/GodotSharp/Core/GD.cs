using System;
using System.Collections.Generic;
using Godot.NativeInterop;

namespace Godot
{
    /// <summary>
    /// Godot's global functions.
    /// </summary>
    public static partial class GD
    {
        /// <summary>
        /// Decodes a byte array back to a <c>Variant</c> value.
        /// If <paramref name="allowObjects"/> is <see langword="true"/> decoding objects is allowed.
        ///
        /// WARNING: Deserialized object can contain code which gets executed.
        /// Do not set <paramref name="allowObjects"/> to <see langword="true"/>
        /// if the serialized object comes from untrusted sources to avoid
        /// potential security threats (remote code execution).
        /// </summary>
        /// <param name="bytes">Byte array that will be decoded to a <c>Variant</c>.</param>
        /// <param name="allowObjects">If objects should be decoded.</param>
        /// <returns>The decoded <c>Variant</c>.</returns>
        public static Variant BytesToVar(Span<byte> bytes, bool allowObjects = false)
        {
            using var varBytes = Marshaling.ConvertSystemArrayToNativePackedByteArray(bytes);
            NativeFuncs.godotsharp_bytes_to_var(varBytes, allowObjects.ToGodotBool(), out godot_variant ret);
            return Variant.CreateTakingOwnershipOfDisposableValue(ret);
        }

        /// <summary>
        /// Converts from a <c>Variant</c> type to another in the best way possible.
        /// The <paramref name="type"/> parameter uses the <see cref="Variant.Type"/> values.
        /// </summary>
        /// <example>
        /// <code>
        /// var a = new Vector2(1, 0);
        /// // Prints 1
        /// GD.Print(a.Length());
        /// var b = GD.Convert(a, Variant.Type.String)
        /// // Prints 6 as "(1, 0)" is 6 characters
        /// GD.Print(b.Length);
        /// </code>
        /// </example>
        /// <returns>The <c>Variant</c> converted to the given <paramref name="type"/>.</returns>
        public static Variant Convert(Variant what, Variant.Type type)
        {
            NativeFuncs.godotsharp_convert((godot_variant)what.NativeVar, (int)type, out godot_variant ret);
            return Variant.CreateTakingOwnershipOfDisposableValue(ret);
        }

        private static string[] GetPrintParams(object[] parameters)
        {
            if (parameters == null)
            {
                return new[] { "null" };
            }

            return Array.ConvertAll(parameters, x => x?.ToString() ?? "null");
        }

        /// <summary>
        /// Returns the integer hash of the variable passed.
        /// </summary>
        /// <example>
        /// <code>
        /// GD.Print(GD.Hash("a")); // Prints 177670
        /// </code>
        /// </example>
        /// <param name="var">Variable that will be hashed.</param>
        /// <returns>Hash of the variable passed.</returns>
        public static int Hash(Variant var)
        {
            return NativeFuncs.godotsharp_hash((godot_variant)var.NativeVar);
        }

        /// <summary>
        /// Returns the <see cref="Object"/> that corresponds to <paramref name="instanceId"/>.
        /// All Objects have a unique instance ID.
        /// </summary>
        /// <example>
        /// <code>
        /// public class MyNode : Node
        /// {
        ///     public string foo = "bar";
        ///
        ///     public override void _Ready()
        ///     {
        ///         ulong id = GetInstanceId();
        ///         var inst = (MyNode)GD.InstanceFromId(Id);
        ///         GD.Print(inst.foo); // Prints bar
        ///     }
        /// }
        /// </code>
        /// </example>
        /// <param name="instanceId">Instance ID of the Object to retrieve.</param>
        /// <returns>The <see cref="Object"/> instance.</returns>
        public static Object InstanceFromId(ulong instanceId)
        {
            return InteropUtils.UnmanagedGetManaged(NativeFuncs.godotsharp_instance_from_id(instanceId));
        }

        /// <summary>
        /// Loads a resource from the filesystem located at <paramref name="path"/>.
        /// The resource is loaded on the method call (unless it's referenced already
        /// elsewhere, e.g. in another script or in the scene), which might cause slight delay,
        /// especially when loading scenes. To avoid unnecessary delays when loading something
        /// multiple times, either store the resource in a variable.
        ///
        /// Note: Resource paths can be obtained by right-clicking on a resource in the FileSystem
        /// dock and choosing "Copy Path" or by dragging the file from the FileSystem dock into the script.
        ///
        /// Important: The path must be absolute, a local path will just return <see langword="null"/>.
        /// This method is a simplified version of <see cref="ResourceLoader.Load"/>, which can be used
        /// for more advanced scenarios.
        /// </summary>
        /// <example>
        /// <code>
        /// // Load a scene called main located in the root of the project directory and cache it in a variable.
        /// var main = GD.Load("res://main.tscn"); // main will contain a PackedScene resource.
        /// </code>
        /// </example>
        /// <param name="path">Path of the <see cref="Resource"/> to load.</param>
        /// <returns>The loaded <see cref="Resource"/>.</returns>
        public static Resource Load(string path)
        {
            return ResourceLoader.Load(path);
        }

        /// <summary>
        /// Loads a resource from the filesystem located at <paramref name="path"/>.
        /// The resource is loaded on the method call (unless it's referenced already
        /// elsewhere, e.g. in another script or in the scene), which might cause slight delay,
        /// especially when loading scenes. To avoid unnecessary delays when loading something
        /// multiple times, either store the resource in a variable.
        ///
        /// Note: Resource paths can be obtained by right-clicking on a resource in the FileSystem
        /// dock and choosing "Copy Path" or by dragging the file from the FileSystem dock into the script.
        ///
        /// Important: The path must be absolute, a local path will just return <see langword="null"/>.
        /// This method is a simplified version of <see cref="ResourceLoader.Load"/>, which can be used
        /// for more advanced scenarios.
        /// </summary>
        /// <example>
        /// <code>
        /// // Load a scene called main located in the root of the project directory and cache it in a variable.
        /// var main = GD.Load&lt;PackedScene&gt;("res://main.tscn"); // main will contain a PackedScene resource.
        /// </code>
        /// </example>
        /// <param name="path">Path of the <see cref="Resource"/> to load.</param>
        /// <typeparam name="T">The type to cast to. Should be a descendant of <see cref="Resource"/>.</typeparam>
        public static T Load<T>(string path) where T : class
        {
            return ResourceLoader.Load<T>(path);
        }

        /// <summary>
        /// Pushes an error message to Godot's built-in debugger and to the OS terminal.
        ///
        /// Note: Errors printed this way will not pause project execution.
        /// </summary>
        /// <example>
        /// <code>
        /// GD.PushError("test_error"); // Prints "test error" to debugger and terminal as error call
        /// </code>
        /// </example>
        /// <param name="message">Error message.</param>
        public static void PushError(string message)
        {
            using var godotStr = Marshaling.ConvertStringToNative(message);
            NativeFuncs.godotsharp_pusherror(godotStr);
        }

        /// <summary>
        /// Pushes a warning message to Godot's built-in debugger and to the OS terminal.
        /// </summary>
        /// <example>
        /// GD.PushWarning("test warning"); // Prints "test warning" to debugger and terminal as warning call
        /// </example>
        /// <param name="message">Warning message.</param>
        public static void PushWarning(string message)
        {
            using var godotStr = Marshaling.ConvertStringToNative(message);
            NativeFuncs.godotsharp_pushwarning(godotStr);
        }

        /// <summary>
        /// Converts one or more arguments of any type to string in the best way possible
        /// and prints them to the console.
        ///
        /// Note: Consider using <see cref="PushError(string)"/> and <see cref="PushWarning(string)"/>
        /// to print error and warning messages instead of <see cref="Print(object[])"/>.
        /// This distinguishes them from print messages used for debugging purposes,
        /// while also displaying a stack trace when an error or warning is printed.
        /// </summary>
        /// <example>
        /// <code>
        /// var a = new int[] { 1, 2, 3 };
        /// GD.Print("a", "b", a); // Prints ab[1, 2, 3]
        /// </code>
        /// </example>
        /// <param name="what">Arguments that will be printed.</param>
        public static void Print(params object[] what)
        {
            string str = string.Concat(GetPrintParams(what));
            using var godotStr = Marshaling.ConvertStringToNative(str);
            NativeFuncs.godotsharp_print(godotStr);
        }

        /// <summary>
        /// Converts one or more arguments of any type to string in the best way possible
        /// and prints them to the console.
        /// The following BBCode tags are supported: b, i, u, s, indent, code, url, center,
        /// right, color, bgcolor, fgcolor.
        /// Color tags only support named colors such as <c>red</c>, not hexadecimal color codes.
        /// Unsupported tags will be left as-is in standard output.
        /// When printing to standard output, the supported subset of BBCode is converted to
        /// ANSI escape codes for the terminal emulator to display. Displaying ANSI escape codes
        /// is currently only supported on Linux and macOS. Support for ANSI escape codes may vary
        /// across terminal emulators, especially for italic and strikethrough.
        ///
        /// Note: Consider using <see cref="PushError(string)"/> and <see cref="PushWarning(string)"/>
        /// to print error and warning messages instead of <see cref="Print(object[])"/> or
        /// <see cref="PrintRich(object[])"/>.
        /// This distinguishes them from print messages used for debugging purposes,
        /// while also displaying a stack trace when an error or warning is printed.
        /// </summary>
        /// <example>
        /// <code>
        /// GD.PrintRich("[b]Hello world![/b]"); // Prints out "Hello world!" in bold.
        /// </code>
        /// </example>
        /// <param name="what">Arguments that will be printed.</param>
        public static void PrintRich(params object[] what)
        {
            string str = string.Concat(GetPrintParams(what));
            using var godotStr = Marshaling.ConvertStringToNative(str);
            NativeFuncs.godotsharp_print_rich(godotStr);
        }

        /// <summary>
        /// Prints the current stack trace information to the console.
        /// </summary>
        public static void PrintStack()
        {
            Print(System.Environment.StackTrace);
        }

        /// <summary>
        /// Prints one or more arguments to strings in the best way possible to standard error line.
        /// </summary>
        /// <example>
        /// <code>
        /// GD.PrintErr("prints to stderr");
        /// </code>
        /// </example>
        /// <param name="what">Arguments that will be printed.</param>
        public static void PrintErr(params object[] what)
        {
            string str = string.Concat(GetPrintParams(what));
            using var godotStr = Marshaling.ConvertStringToNative(str);
            NativeFuncs.godotsharp_printerr(godotStr);
        }

        /// <summary>
        /// Prints one or more arguments to strings in the best way possible to console.
        /// No newline is added at the end.
        ///
        /// Note: Due to limitations with Godot's built-in console, this only prints to the terminal.
        /// If you need to print in the editor, use another method, such as <see cref="Print(object[])"/>.
        /// </summary>
        /// <example>
        /// <code>
        /// GD.PrintRaw("A");
        /// GD.PrintRaw("B");
        /// // Prints AB
        /// </code>
        /// </example>
        /// <param name="what">Arguments that will be printed.</param>
        public static void PrintRaw(params object[] what)
        {
            string str = string.Concat(GetPrintParams(what));
            using var godotStr = Marshaling.ConvertStringToNative(str);
            NativeFuncs.godotsharp_printraw(godotStr);
        }

        /// <summary>
        /// Prints one or more arguments to the console with a space between each argument.
        /// </summary>
        /// <example>
        /// <code>
        /// GD.PrintS("A", "B", "C"); // Prints A B C
        /// </code>
        /// </example>
        /// <param name="what">Arguments that will be printed.</param>
        public static void PrintS(params object[] what)
        {
            string str = string.Join(' ', GetPrintParams(what));
            using var godotStr = Marshaling.ConvertStringToNative(str);
            NativeFuncs.godotsharp_prints(godotStr);
        }

        /// <summary>
        /// Prints one or more arguments to the console with a tab between each argument.
        /// </summary>
        /// <example>
        /// <code>
        /// GD.PrintT("A", "B", "C"); // Prints A       B       C
        /// </code>
        /// </example>
        /// <param name="what">Arguments that will be printed.</param>
        public static void PrintT(params object[] what)
        {
            string str = string.Join('\t', GetPrintParams(what));
            using var godotStr = Marshaling.ConvertStringToNative(str);
            NativeFuncs.godotsharp_printt(godotStr);
        }

        /// <summary>
        /// Returns a random floating point value between <c>0.0</c> and <c>1.0</c> (inclusive).
        /// </summary>
        /// <example>
        /// <code>
        /// GD.Randf(); // Returns e.g. 0.375671
        /// </code>
        /// </example>
        /// <returns>A random <see langword="float"/> number.</returns>
        public static float Randf()
        {
            return NativeFuncs.godotsharp_randf();
        }

        /// <summary>
        /// Returns a normally-distributed pseudo-random number, using Box-Muller transform with the specified <c>mean</c> and a standard <c>deviation</c>.
        /// This is also called Gaussian distribution.
        /// </summary>
        /// <returns>A random normally-distributed <see langword="float"/> number.</returns>
        public static double Randfn(double mean, double deviation)
        {
            return NativeFuncs.godotsharp_randfn(mean, deviation);
        }

        /// <summary>
        /// Returns a random unsigned 32-bit integer.
        /// Use remainder to obtain a random value in the interval <c>[0, N - 1]</c> (where N is smaller than 2^32).
        /// </summary>
        /// <example>
        /// <code>
        /// GD.Randi();           // Returns random integer between 0 and 2^32 - 1
        /// GD.Randi() % 20;      // Returns random integer between 0 and 19
        /// GD.Randi() % 100;     // Returns random integer between 0 and 99
        /// GD.Randi() % 100 + 1; // Returns random integer between 1 and 100
        /// </code>
        /// </example>
        /// <returns>A random <see langword="uint"/> number.</returns>
        public static uint Randi()
        {
            return NativeFuncs.godotsharp_randi();
        }

        /// <summary>
        /// Randomizes the seed (or the internal state) of the random number generator.
        /// Current implementation reseeds using a number based on time.
        ///
        /// Note: This method is called automatically when the project is run.
        /// If you need to fix the seed to have reproducible results, use <see cref="Seed(ulong)"/>
        /// to initialize the random number generator.
        /// </summary>
        public static void Randomize()
        {
            NativeFuncs.godotsharp_randomize();
        }

        /// <summary>
        /// Returns a random floating point value on the interval between <paramref name="from"/>
        /// and <paramref name="to"/> (inclusive).
        /// </summary>
        /// <example>
        /// <code>
        /// GD.PrintS(GD.RandRange(-10.0, 10.0), GD.RandRange(-10.0, 10.0)); // Prints e.g. -3.844535 7.45315
        /// </code>
        /// </example>
        /// <returns>A random <see langword="double"/> number inside the given range.</returns>
        public static double RandRange(double from, double to)
        {
            return NativeFuncs.godotsharp_randf_range(from, to);
        }

        /// <summary>
        /// Returns a random signed 32-bit integer between <paramref name="from"/>
        /// and <paramref name="to"/> (inclusive). If <paramref name="to"/> is lesser than
        /// <paramref name="from"/>, they are swapped.
        /// </summary>
        /// <example>
        /// <code>
        /// GD.Print(GD.RandRange(0, 1)); // Prints 0 or 1
        /// GD.Print(GD.RandRange(-10, 1000)); // Prints any number from -10 to 1000
        /// </code>
        /// </example>
        /// <returns>A random <see langword="int"/> number inside the given range.</returns>
        public static int RandRange(int from, int to)
        {
            return NativeFuncs.godotsharp_randi_range(from, to);
        }

        /// <summary>
        /// Returns a random unsigned 32-bit integer, using the given <paramref name="seed"/>.
        /// </summary>
        /// <param name="seed">
        /// Seed to use to generate the random number.
        /// If a different seed is used, its value will be modified.
        /// </param>
        /// <returns>A random <see langword="uint"/> number.</returns>
        public static uint RandFromSeed(ref ulong seed)
        {
            return NativeFuncs.godotsharp_rand_from_seed(seed, out seed);
        }

        /// <summary>
        /// Returns a <see cref="IEnumerable{T}"/> that iterates from
        /// <c>0</c> to <paramref name="end"/> in steps of <c>1</c>.
        /// </summary>
        /// <param name="end">The last index.</param>
        public static IEnumerable<int> Range(int end)
        {
            return Range(0, end, 1);
        }

        /// <summary>
        /// Returns a <see cref="IEnumerable{T}"/> that iterates from
        /// <paramref name="start"/> to <paramref name="end"/> in steps of <c>1</c>.
        /// </summary>
        /// <param name="start">The first index.</param>
        /// <param name="end">The last index.</param>
        public static IEnumerable<int> Range(int start, int end)
        {
            return Range(start, end, 1);
        }

        /// <summary>
        /// Returns a <see cref="IEnumerable{T}"/> that iterates from
        /// <paramref name="start"/> to <paramref name="end"/> in steps of <paramref name="step"/>.
        /// </summary>
        /// <param name="start">The first index.</param>
        /// <param name="end">The last index.</param>
        /// <param name="step">The amount by which to increment the index on each iteration.</param>
        public static IEnumerable<int> Range(int start, int end, int step)
        {
            if (end < start && step > 0)
                yield break;

            if (end > start && step < 0)
                yield break;

            if (step > 0)
            {
                for (int i = start; i < end; i += step)
                    yield return i;
            }
            else
            {
                for (int i = start; i > end; i += step)
                    yield return i;
            }
        }

        /// <summary>
        /// Sets seed for the random number generator.
        /// </summary>
        /// <param name="seed">Seed that will be used.</param>
        public static void Seed(ulong seed)
        {
            NativeFuncs.godotsharp_seed(seed);
        }

        /// <summary>
        /// Converts one or more arguments of any type to string in the best way possible.
        /// </summary>
        /// <param name="what">Arguments that will converted to string.</param>
        /// <returns>The string formed by the given arguments.</returns>
        public static string Str(params Variant[] what)
        {
            using var whatGodot = new Godot.Collections.Array(what);
            NativeFuncs.godotsharp_str((godot_array)whatGodot.NativeValue, out godot_string ret);
            using (ret)
                return Marshaling.ConvertStringToManaged(ret);
        }

        /// <summary>
        /// Converts a formatted string that was returned by <see cref="VarToStr(Variant)"/> to the original value.
        /// </summary>
        /// <example>
        /// <code>
        /// string a = "{\"a\": 1, \"b\": 2 }";
        /// var b = (Godot.Collections.Dictionary)GD.StrToVar(a);
        /// GD.Print(b["a"]); // Prints 1
        /// </code>
        /// </example>
        /// <param name="str">String that will be converted to Variant.</param>
        /// <returns>The decoded <c>Variant</c>.</returns>
        public static Variant StrToVar(string str)
        {
            using var godotStr = Marshaling.ConvertStringToNative(str);
            NativeFuncs.godotsharp_str_to_var(godotStr, out godot_variant ret);
            return Variant.CreateTakingOwnershipOfDisposableValue(ret);
        }

        /// <summary>
        /// Encodes a <c>Variant</c> value to a byte array.
        /// If <paramref name="fullObjects"/> is <see langword="true"/> encoding objects is allowed
        /// (and can potentially include code).
        /// Deserialization can be done with <see cref="BytesToVar(Span{byte}, bool)"/>.
        /// </summary>
        /// <param name="var">Variant that will be encoded.</param>
        /// <param name="fullObjects">If objects should be serialized.</param>
        /// <returns>The <c>Variant</c> encoded as an array of bytes.</returns>
        public static byte[] VarToBytes(Variant var, bool fullObjects = false)
        {
            NativeFuncs.godotsharp_var_to_bytes((godot_variant)var.NativeVar, fullObjects.ToGodotBool(), out var varBytes);
            using (varBytes)
                return Marshaling.ConvertNativePackedByteArrayToSystemArray(varBytes);
        }

        /// <summary>
        /// Converts a <c>Variant</c> <paramref name="var"/> to a formatted string that
        /// can later be parsed using <see cref="StrToVar(string)"/>.
        /// </summary>
        /// <example>
        /// <code>
        /// var a = new Godot.Collections.Dictionary { ["a"] = 1, ["b"] = 2 };
        /// GD.Print(GD.VarToStr(a));
        /// // Prints
        /// // {
        /// //    "a": 1,
        /// //    "b": 2
        /// // }
        /// </code>
        /// </example>
        /// <param name="var">Variant that will be converted to string.</param>
        /// <returns>The <c>Variant</c> encoded as a string.</returns>
        public static string VarToStr(Variant var)
        {
            NativeFuncs.godotsharp_var_to_str((godot_variant)var.NativeVar, out godot_string ret);
            using (ret)
                return Marshaling.ConvertStringToManaged(ret);
        }

        /// <summary>
        /// Get the <see cref="Variant.Type"/> that corresponds for the given <see cref="Type"/>.
        /// </summary>
        /// <returns>The <see cref="Variant.Type"/> for the given <paramref name="type"/>.</returns>
        public static Variant.Type TypeToVariantType(Type type)
        {
            return Marshaling.ConvertManagedTypeToVariantType(type, out bool _);
        }
    }
}
