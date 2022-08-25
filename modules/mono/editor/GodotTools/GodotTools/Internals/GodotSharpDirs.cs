using System.IO;
using Godot;
using Godot.NativeInterop;
using GodotTools.Core;
using static GodotTools.Internals.Globals;

namespace GodotTools.Internals
{
    public static class GodotSharpDirs
    {
        public static string ResMetadataDir
        {
            get
            {
                Internal.godot_icall_GodotSharpDirs_ResMetadataDir(out godot_string dest);
                using (dest)
                    return Marshaling.ConvertStringToManaged(dest);
            }
        }

        public static string MonoUserDir
        {
            get
            {
                Internal.godot_icall_GodotSharpDirs_MonoUserDir(out godot_string dest);
                using (dest)
                    return Marshaling.ConvertStringToManaged(dest);
            }
        }

        public static string BuildLogsDirs
        {
            get
            {
                Internal.godot_icall_GodotSharpDirs_BuildLogsDirs(out godot_string dest);
                using (dest)
                    return Marshaling.ConvertStringToManaged(dest);
            }
        }

        public static string DataEditorToolsDir
        {
            get
            {
                Internal.godot_icall_GodotSharpDirs_DataEditorToolsDir(out godot_string dest);
                using (dest)
                    return Marshaling.ConvertStringToManaged(dest);
            }
        }

        public static void RegisterProjectSettings()
        {
            GlobalDef("dotnet/project/assembly_name", "");
            GlobalDef("dotnet/project/solution_directory", "");
            GlobalDef("dotnet/project/c#_project_directory", "");
        }

        private static void DetermineProjectLocation()
        {
            static string DetermineProjectName()
            {
                string projectAssemblyName = (string)ProjectSettings.GetSetting("application/config/name");
                projectAssemblyName = projectAssemblyName.ToSafeDirName();
                if (string.IsNullOrEmpty(projectAssemblyName))
                    projectAssemblyName = "UnnamedProject";
                return projectAssemblyName;
            }

            _projectAssemblyName = (string)ProjectSettings.GetSetting("dotnet/project/assembly_name");
            if (string.IsNullOrEmpty(_projectAssemblyName))
            {
                _projectAssemblyName = DetermineProjectName();
                ProjectSettings.SetSetting("dotnet/project/assembly_name", _projectAssemblyName);
            }

            string slnParentDir = (string)ProjectSettings.GetSetting("dotnet/project/solution_directory");
            if (string.IsNullOrEmpty(slnParentDir))
                slnParentDir = "res://";

            string csprojParentDir = (string)ProjectSettings.GetSetting("dotnet/project/c#_project_directory");
            if (string.IsNullOrEmpty(csprojParentDir))
                csprojParentDir = "res://";

            _projectSlnPath = Path.Combine(ProjectSettings.GlobalizePath(slnParentDir),
                string.Concat(_projectAssemblyName, ".sln"));

            _projectCsProjPath = Path.Combine(ProjectSettings.GlobalizePath(csprojParentDir),
                string.Concat(_projectAssemblyName, ".csproj"));
        }

        private static string _projectAssemblyName;
        private static string _projectSlnPath;
        private static string _projectCsProjPath;

        public static string ProjectAssemblyName
        {
            get
            {
                if (_projectAssemblyName == null)
                    DetermineProjectLocation();
                return _projectAssemblyName;
            }
        }

        public static string ProjectSlnPath
        {
            get
            {
                if (_projectSlnPath == null)
                    DetermineProjectLocation();
                return _projectSlnPath;
            }
        }

        public static string ProjectCsProjPath
        {
            get
            {
                if (_projectCsProjPath == null)
                    DetermineProjectLocation();
                return _projectCsProjPath;
            }
        }
    }
}
