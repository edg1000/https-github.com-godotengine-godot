using System;

namespace Godot
{
    /// <summary>
    /// Attribute that changes the RPC mode for the annotated <c>method</c> to the given <see cref="Mode"/>,
    /// optionally specifying the <see cref="TransferMode"/> and <see cref="TransferChannel"/> (on supported peers).
    /// See <see cref="RPCMode"/> and <see cref="TransferMode"/>. By default, methods are not exposed to networking
    /// (and RPCs).
    /// </summary>
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = false)]
    public class RPCAttribute : Attribute
    {
        /// <summary>
        /// RPC mode for the annotated method.
        /// </summary>
        public RPCMode Mode { get; } = RPCMode.Disabled;

        /// <summary>
        /// If the method will also be called locally; otherwise, it is only called remotely.
        /// </summary>
        public bool CallLocal { get; set; } = false;

        /// <summary>
        /// Transfer mode for the annotated method.
        /// </summary>
        public TransferMode TransferMode { get; set; } = TransferMode.Reliable;

        /// <summary>
        /// Transfer channel for the annotated mode.
        /// </summary>
        public int TransferChannel { get; set; } = 0;

        /// <summary>
        /// Constructs a <see cref="RPCAttribute"/> instance.
        /// </summary>
        /// <param name="mode">The RPC mode to use.</param>
        public RPCAttribute(RPCMode mode = RPCMode.Authority)
        {
            Mode = mode;
        }
    }
}
