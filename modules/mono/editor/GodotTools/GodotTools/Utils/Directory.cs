using System.IO;
using Godot;

namespace GodotTools.Utils
{
    public static class Directory
    {
        private static string GlobalizePath(this string path) => ProjectSettings.GlobalizePath(path);

        public static bool Exists(string path) => System.IO.Directory.Exists(path.GlobalizePath());

        /// Create directory recursively
        public static DirectoryInfo CreateDirectory(string path) =>
            System.IO.Directory.CreateDirectory(path.GlobalizePath());

        public static void Delete(string path, bool recursive) =>
            System.IO.Directory.Delete(path.GlobalizePath(), recursive);


        public static string[] GetDirectories(string path, string searchPattern, SearchOption searchOption) =>
            System.IO.Directory.GetDirectories(path.GlobalizePath(), searchPattern, searchOption);

        public static string[] GetFiles(string path, string searchPattern, SearchOption searchOption) =>
            System.IO.Directory.GetFiles(path.GlobalizePath(), searchPattern, searchOption);
    }
}
