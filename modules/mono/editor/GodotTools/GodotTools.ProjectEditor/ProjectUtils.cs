using System;
using Microsoft.Build.Construction;

namespace GodotTools.ProjectEditor
{
    public sealed class MSBuildProject
    {
        internal ProjectRootElement Root { get; set; }

        public bool HasUnsavedChanges { get; set; }

        public void Save() => Root.Save();

        public MSBuildProject(ProjectRootElement root)
        {
            Root = root;
        }
    }

    public static class ProjectUtils
    {
        public static void MSBuildLocatorRegisterDefaults(out Version version, out string path)
        {
            var instance = Microsoft.Build.Locator.MSBuildLocator.RegisterDefaults();
            version = instance.Version;
            path = instance.MSBuildPath;
        }

        public static void MSBuildLocatorRegisterMSBuildPath(string msbuildPath)
            => Microsoft.Build.Locator.MSBuildLocator.RegisterMSBuildPath(msbuildPath);

        public static MSBuildProject Open(string path)
        {
            var root = ProjectRootElement.Open(path);
            return root != null ? new MSBuildProject(root) : null;
        }

        public static void MigrateToProjectSdksStyle(MSBuildProject project, string projectName)
        {
            var origRoot = project.Root;

            if (!string.IsNullOrEmpty(origRoot.Sdk))
                return;

            project.Root = ProjectGenerator.GenGameProject(projectName);
            project.Root.FullPath = origRoot.FullPath;
            project.HasUnsavedChanges = true;
        }

        public static void EnsureGodotSdkIsUpToDate(MSBuildProject project)
        {
            var root = project.Root;
            string godotSdkAttrValue = ProjectGenerator.GodotSdkAttrValue;

            if (!string.IsNullOrEmpty(root.Sdk) &&
                root.Sdk.Trim().Equals(godotSdkAttrValue, StringComparison.OrdinalIgnoreCase))
                return;

            root.Sdk = godotSdkAttrValue;
            project.HasUnsavedChanges = true;
        }
    }
}
