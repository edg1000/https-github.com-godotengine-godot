using System;

#nullable enable

namespace Godot
{
    /// <summary>
    /// Hide the target from the create Node dialog
    /// </summary>
    [AttributeUsage(AttributeTargets.Class)]
    public sealed class HideAttribute : Attribute { }
}
