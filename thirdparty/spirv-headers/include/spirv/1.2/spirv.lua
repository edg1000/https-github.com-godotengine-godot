-- Copyright (c) 2014-2018 The Khronos Group Inc.
-- 
-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and/or associated documentation files (the "Materials"),
-- to deal in the Materials without restriction, including without limitation
-- the rights to use, copy, modify, merge, publish, distribute, sublicense,
-- and/or sell copies of the Materials, and to permit persons to whom the
-- Materials are furnished to do so, subject to the following conditions:
-- 
-- The above copyright notice and this permission notice shall be included in
-- all copies or substantial portions of the Materials.
-- 
-- MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS KHRONOS
-- STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS SPECIFICATIONS AND
-- HEADER INFORMATION ARE LOCATED AT https://www.khronos.org/registry/ 
-- 
-- THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
-- OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
-- THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
-- FROM,OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE USE OR OTHER DEALINGS
-- IN THE MATERIALS.

-- This header is automatically generated by the same tool that creates
-- the Binary Section of the SPIR-V specification.

-- Enumeration tokens for SPIR-V, in various styles:
--   C, C++, C++11, JSON, Lua, Python
-- 
-- - C will have tokens with a "Spv" prefix, e.g.: SpvSourceLanguageGLSL
-- - C++ will have tokens in the "spv" name space, e.g.: spv::SourceLanguageGLSL
-- - C++11 will use enum classes in the spv namespace, e.g.: spv::SourceLanguage::GLSL
-- - Lua will use tables, e.g.: spv.SourceLanguage.GLSL
-- - Python will use dictionaries, e.g.: spv['SourceLanguage']['GLSL']
-- 
-- Some tokens act like mask values, which can be OR'd together,
-- while others are mutually exclusive.  The mask-like ones have
-- "Mask" in their name, and a parallel enum that has the shift
-- amount (1 << x) for each corresponding enumerant.

spv = {
    MagicNumber = 0x07230203,
    Version = 0x00010200,
    Revision = 2,
    OpCodeMask = 0xffff,
    WordCountShift = 16,

    SourceLanguage = {
        Unknown = 0,
        ESSL = 1,
        GLSL = 2,
        OpenCL_C = 3,
        OpenCL_CPP = 4,
        HLSL = 5,
    },

    ExecutionModel = {
        Vertex = 0,
        TessellationControl = 1,
        TessellationEvaluation = 2,
        Geometry = 3,
        Fragment = 4,
        GLCompute = 5,
        Kernel = 6,
    },

    AddressingModel = {
        Logical = 0,
        Physical32 = 1,
        Physical64 = 2,
    },

    MemoryModel = {
        Simple = 0,
        GLSL450 = 1,
        OpenCL = 2,
    },

    ExecutionMode = {
        Invocations = 0,
        SpacingEqual = 1,
        SpacingFractionalEven = 2,
        SpacingFractionalOdd = 3,
        VertexOrderCw = 4,
        VertexOrderCcw = 5,
        PixelCenterInteger = 6,
        OriginUpperLeft = 7,
        OriginLowerLeft = 8,
        EarlyFragmentTests = 9,
        PointMode = 10,
        Xfb = 11,
        DepthReplacing = 12,
        DepthGreater = 14,
        DepthLess = 15,
        DepthUnchanged = 16,
        LocalSize = 17,
        LocalSizeHint = 18,
        InputPoints = 19,
        InputLines = 20,
        InputLinesAdjacency = 21,
        Triangles = 22,
        InputTrianglesAdjacency = 23,
        Quads = 24,
        Isolines = 25,
        OutputVertices = 26,
        OutputPoints = 27,
        OutputLineStrip = 28,
        OutputTriangleStrip = 29,
        VecTypeHint = 30,
        ContractionOff = 31,
        Initializer = 33,
        Finalizer = 34,
        SubgroupSize = 35,
        SubgroupsPerWorkgroup = 36,
        SubgroupsPerWorkgroupId = 37,
        LocalSizeId = 38,
        LocalSizeHintId = 39,
        PostDepthCoverage = 4446,
        StencilRefReplacingEXT = 5027,
    },

    StorageClass = {
        UniformConstant = 0,
        Input = 1,
        Uniform = 2,
        Output = 3,
        Workgroup = 4,
        CrossWorkgroup = 5,
        Private = 6,
        Function = 7,
        Generic = 8,
        PushConstant = 9,
        AtomicCounter = 10,
        Image = 11,
        StorageBuffer = 12,
    },

    Dim = {
        Dim1D = 0,
        Dim2D = 1,
        Dim3D = 2,
        Cube = 3,
        Rect = 4,
        Buffer = 5,
        SubpassData = 6,
    },

    SamplerAddressingMode = {
        None = 0,
        ClampToEdge = 1,
        Clamp = 2,
        Repeat = 3,
        RepeatMirrored = 4,
    },

    SamplerFilterMode = {
        Nearest = 0,
        Linear = 1,
    },

    ImageFormat = {
        Unknown = 0,
        Rgba32f = 1,
        Rgba16f = 2,
        R32f = 3,
        Rgba8 = 4,
        Rgba8Snorm = 5,
        Rg32f = 6,
        Rg16f = 7,
        R11fG11fB10f = 8,
        R16f = 9,
        Rgba16 = 10,
        Rgb10A2 = 11,
        Rg16 = 12,
        Rg8 = 13,
        R16 = 14,
        R8 = 15,
        Rgba16Snorm = 16,
        Rg16Snorm = 17,
        Rg8Snorm = 18,
        R16Snorm = 19,
        R8Snorm = 20,
        Rgba32i = 21,
        Rgba16i = 22,
        Rgba8i = 23,
        R32i = 24,
        Rg32i = 25,
        Rg16i = 26,
        Rg8i = 27,
        R16i = 28,
        R8i = 29,
        Rgba32ui = 30,
        Rgba16ui = 31,
        Rgba8ui = 32,
        R32ui = 33,
        Rgb10a2ui = 34,
        Rg32ui = 35,
        Rg16ui = 36,
        Rg8ui = 37,
        R16ui = 38,
        R8ui = 39,
    },

    ImageChannelOrder = {
        R = 0,
        A = 1,
        RG = 2,
        RA = 3,
        RGB = 4,
        RGBA = 5,
        BGRA = 6,
        ARGB = 7,
        Intensity = 8,
        Luminance = 9,
        Rx = 10,
        RGx = 11,
        RGBx = 12,
        Depth = 13,
        DepthStencil = 14,
        sRGB = 15,
        sRGBx = 16,
        sRGBA = 17,
        sBGRA = 18,
        ABGR = 19,
    },

    ImageChannelDataType = {
        SnormInt8 = 0,
        SnormInt16 = 1,
        UnormInt8 = 2,
        UnormInt16 = 3,
        UnormShort565 = 4,
        UnormShort555 = 5,
        UnormInt101010 = 6,
        SignedInt8 = 7,
        SignedInt16 = 8,
        SignedInt32 = 9,
        UnsignedInt8 = 10,
        UnsignedInt16 = 11,
        UnsignedInt32 = 12,
        HalfFloat = 13,
        Float = 14,
        UnormInt24 = 15,
        UnormInt101010_2 = 16,
    },

    ImageOperandsShift = {
        Bias = 0,
        Lod = 1,
        Grad = 2,
        ConstOffset = 3,
        Offset = 4,
        ConstOffsets = 5,
        Sample = 6,
        MinLod = 7,
    },

    ImageOperandsMask = {
        MaskNone = 0,
        Bias = 0x00000001,
        Lod = 0x00000002,
        Grad = 0x00000004,
        ConstOffset = 0x00000008,
        Offset = 0x00000010,
        ConstOffsets = 0x00000020,
        Sample = 0x00000040,
        MinLod = 0x00000080,
    },

    FPFastMathModeShift = {
        NotNaN = 0,
        NotInf = 1,
        NSZ = 2,
        AllowRecip = 3,
        Fast = 4,
    },

    FPFastMathModeMask = {
        MaskNone = 0,
        NotNaN = 0x00000001,
        NotInf = 0x00000002,
        NSZ = 0x00000004,
        AllowRecip = 0x00000008,
        Fast = 0x00000010,
    },

    FPRoundingMode = {
        RTE = 0,
        RTZ = 1,
        RTP = 2,
        RTN = 3,
    },

    LinkageType = {
        Export = 0,
        Import = 1,
    },

    AccessQualifier = {
        ReadOnly = 0,
        WriteOnly = 1,
        ReadWrite = 2,
    },

    FunctionParameterAttribute = {
        Zext = 0,
        Sext = 1,
        ByVal = 2,
        Sret = 3,
        NoAlias = 4,
        NoCapture = 5,
        NoWrite = 6,
        NoReadWrite = 7,
    },

    Decoration = {
        RelaxedPrecision = 0,
        SpecId = 1,
        Block = 2,
        BufferBlock = 3,
        RowMajor = 4,
        ColMajor = 5,
        ArrayStride = 6,
        MatrixStride = 7,
        GLSLShared = 8,
        GLSLPacked = 9,
        CPacked = 10,
        BuiltIn = 11,
        NoPerspective = 13,
        Flat = 14,
        Patch = 15,
        Centroid = 16,
        Sample = 17,
        Invariant = 18,
        Restrict = 19,
        Aliased = 20,
        Volatile = 21,
        Constant = 22,
        Coherent = 23,
        NonWritable = 24,
        NonReadable = 25,
        Uniform = 26,
        SaturatedConversion = 28,
        Stream = 29,
        Location = 30,
        Component = 31,
        Index = 32,
        Binding = 33,
        DescriptorSet = 34,
        Offset = 35,
        XfbBuffer = 36,
        XfbStride = 37,
        FuncParamAttr = 38,
        FPRoundingMode = 39,
        FPFastMathMode = 40,
        LinkageAttributes = 41,
        NoContraction = 42,
        InputAttachmentIndex = 43,
        Alignment = 44,
        MaxByteOffset = 45,
        AlignmentId = 46,
        MaxByteOffsetId = 47,
        ExplicitInterpAMD = 4999,
        OverrideCoverageNV = 5248,
        PassthroughNV = 5250,
        ViewportRelativeNV = 5252,
        SecondaryViewportRelativeNV = 5256,
        HlslCounterBufferGOOGLE = 5634,
        HlslSemanticGOOGLE = 5635,
    },

    BuiltIn = {
        Position = 0,
        PointSize = 1,
        ClipDistance = 3,
        CullDistance = 4,
        VertexId = 5,
        InstanceId = 6,
        PrimitiveId = 7,
        InvocationId = 8,
        Layer = 9,
        ViewportIndex = 10,
        TessLevelOuter = 11,
        TessLevelInner = 12,
        TessCoord = 13,
        PatchVertices = 14,
        FragCoord = 15,
        PointCoord = 16,
        FrontFacing = 17,
        SampleId = 18,
        SamplePosition = 19,
        SampleMask = 20,
        FragDepth = 22,
        HelperInvocation = 23,
        NumWorkgroups = 24,
        WorkgroupSize = 25,
        WorkgroupId = 26,
        LocalInvocationId = 27,
        GlobalInvocationId = 28,
        LocalInvocationIndex = 29,
        WorkDim = 30,
        GlobalSize = 31,
        EnqueuedWorkgroupSize = 32,
        GlobalOffset = 33,
        GlobalLinearId = 34,
        SubgroupSize = 36,
        SubgroupMaxSize = 37,
        NumSubgroups = 38,
        NumEnqueuedSubgroups = 39,
        SubgroupId = 40,
        SubgroupLocalInvocationId = 41,
        VertexIndex = 42,
        InstanceIndex = 43,
        SubgroupEqMaskKHR = 4416,
        SubgroupGeMaskKHR = 4417,
        SubgroupGtMaskKHR = 4418,
        SubgroupLeMaskKHR = 4419,
        SubgroupLtMaskKHR = 4420,
        BaseVertex = 4424,
        BaseInstance = 4425,
        DrawIndex = 4426,
        DeviceIndex = 4438,
        ViewIndex = 4440,
        BaryCoordNoPerspAMD = 4992,
        BaryCoordNoPerspCentroidAMD = 4993,
        BaryCoordNoPerspSampleAMD = 4994,
        BaryCoordSmoothAMD = 4995,
        BaryCoordSmoothCentroidAMD = 4996,
        BaryCoordSmoothSampleAMD = 4997,
        BaryCoordPullModelAMD = 4998,
        FragStencilRefEXT = 5014,
        ViewportMaskNV = 5253,
        SecondaryPositionNV = 5257,
        SecondaryViewportMaskNV = 5258,
        PositionPerViewNV = 5261,
        ViewportMaskPerViewNV = 5262,
    },

    SelectionControlShift = {
        Flatten = 0,
        DontFlatten = 1,
    },

    SelectionControlMask = {
        MaskNone = 0,
        Flatten = 0x00000001,
        DontFlatten = 0x00000002,
    },

    LoopControlShift = {
        Unroll = 0,
        DontUnroll = 1,
        DependencyInfinite = 2,
        DependencyLength = 3,
    },

    LoopControlMask = {
        MaskNone = 0,
        Unroll = 0x00000001,
        DontUnroll = 0x00000002,
        DependencyInfinite = 0x00000004,
        DependencyLength = 0x00000008,
    },

    FunctionControlShift = {
        Inline = 0,
        DontInline = 1,
        Pure = 2,
        Const = 3,
    },

    FunctionControlMask = {
        MaskNone = 0,
        Inline = 0x00000001,
        DontInline = 0x00000002,
        Pure = 0x00000004,
        Const = 0x00000008,
    },

    MemorySemanticsShift = {
        Acquire = 1,
        Release = 2,
        AcquireRelease = 3,
        SequentiallyConsistent = 4,
        UniformMemory = 6,
        SubgroupMemory = 7,
        WorkgroupMemory = 8,
        CrossWorkgroupMemory = 9,
        AtomicCounterMemory = 10,
        ImageMemory = 11,
    },

    MemorySemanticsMask = {
        MaskNone = 0,
        Acquire = 0x00000002,
        Release = 0x00000004,
        AcquireRelease = 0x00000008,
        SequentiallyConsistent = 0x00000010,
        UniformMemory = 0x00000040,
        SubgroupMemory = 0x00000080,
        WorkgroupMemory = 0x00000100,
        CrossWorkgroupMemory = 0x00000200,
        AtomicCounterMemory = 0x00000400,
        ImageMemory = 0x00000800,
    },

    MemoryAccessShift = {
        Volatile = 0,
        Aligned = 1,
        Nontemporal = 2,
    },

    MemoryAccessMask = {
        MaskNone = 0,
        Volatile = 0x00000001,
        Aligned = 0x00000002,
        Nontemporal = 0x00000004,
    },

    Scope = {
        CrossDevice = 0,
        Device = 1,
        Workgroup = 2,
        Subgroup = 3,
        Invocation = 4,
    },

    GroupOperation = {
        Reduce = 0,
        InclusiveScan = 1,
        ExclusiveScan = 2,
    },

    KernelEnqueueFlags = {
        NoWait = 0,
        WaitKernel = 1,
        WaitWorkGroup = 2,
    },

    KernelProfilingInfoShift = {
        CmdExecTime = 0,
    },

    KernelProfilingInfoMask = {
        MaskNone = 0,
        CmdExecTime = 0x00000001,
    },

    Capability = {
        Matrix = 0,
        Shader = 1,
        Geometry = 2,
        Tessellation = 3,
        Addresses = 4,
        Linkage = 5,
        Kernel = 6,
        Vector16 = 7,
        Float16Buffer = 8,
        Float16 = 9,
        Float64 = 10,
        Int64 = 11,
        Int64Atomics = 12,
        ImageBasic = 13,
        ImageReadWrite = 14,
        ImageMipmap = 15,
        Pipes = 17,
        Groups = 18,
        DeviceEnqueue = 19,
        LiteralSampler = 20,
        AtomicStorage = 21,
        Int16 = 22,
        TessellationPointSize = 23,
        GeometryPointSize = 24,
        ImageGatherExtended = 25,
        StorageImageMultisample = 27,
        UniformBufferArrayDynamicIndexing = 28,
        SampledImageArrayDynamicIndexing = 29,
        StorageBufferArrayDynamicIndexing = 30,
        StorageImageArrayDynamicIndexing = 31,
        ClipDistance = 32,
        CullDistance = 33,
        ImageCubeArray = 34,
        SampleRateShading = 35,
        ImageRect = 36,
        SampledRect = 37,
        GenericPointer = 38,
        Int8 = 39,
        InputAttachment = 40,
        SparseResidency = 41,
        MinLod = 42,
        Sampled1D = 43,
        Image1D = 44,
        SampledCubeArray = 45,
        SampledBuffer = 46,
        ImageBuffer = 47,
        ImageMSArray = 48,
        StorageImageExtendedFormats = 49,
        ImageQuery = 50,
        DerivativeControl = 51,
        InterpolationFunction = 52,
        TransformFeedback = 53,
        GeometryStreams = 54,
        StorageImageReadWithoutFormat = 55,
        StorageImageWriteWithoutFormat = 56,
        MultiViewport = 57,
        SubgroupDispatch = 58,
        NamedBarrier = 59,
        PipeStorage = 60,
        SubgroupBallotKHR = 4423,
        DrawParameters = 4427,
        SubgroupVoteKHR = 4431,
        StorageBuffer16BitAccess = 4433,
        StorageUniformBufferBlock16 = 4433,
        StorageUniform16 = 4434,
        UniformAndStorageBuffer16BitAccess = 4434,
        StoragePushConstant16 = 4435,
        StorageInputOutput16 = 4436,
        DeviceGroup = 4437,
        MultiView = 4439,
        VariablePointersStorageBuffer = 4441,
        VariablePointers = 4442,
        AtomicStorageOps = 4445,
        SampleMaskPostDepthCoverage = 4447,
        ImageGatherBiasLodAMD = 5009,
        FragmentMaskAMD = 5010,
        StencilExportEXT = 5013,
        ImageReadWriteLodAMD = 5015,
        SampleMaskOverrideCoverageNV = 5249,
        GeometryShaderPassthroughNV = 5251,
        ShaderViewportIndexLayerEXT = 5254,
        ShaderViewportIndexLayerNV = 5254,
        ShaderViewportMaskNV = 5255,
        ShaderStereoViewNV = 5259,
        PerViewAttributesNV = 5260,
        SubgroupShuffleINTEL = 5568,
        SubgroupBufferBlockIOINTEL = 5569,
        SubgroupImageBlockIOINTEL = 5570,
    },

    Op = {
        OpNop = 0,
        OpUndef = 1,
        OpSourceContinued = 2,
        OpSource = 3,
        OpSourceExtension = 4,
        OpName = 5,
        OpMemberName = 6,
        OpString = 7,
        OpLine = 8,
        OpExtension = 10,
        OpExtInstImport = 11,
        OpExtInst = 12,
        OpMemoryModel = 14,
        OpEntryPoint = 15,
        OpExecutionMode = 16,
        OpCapability = 17,
        OpTypeVoid = 19,
        OpTypeBool = 20,
        OpTypeInt = 21,
        OpTypeFloat = 22,
        OpTypeVector = 23,
        OpTypeMatrix = 24,
        OpTypeImage = 25,
        OpTypeSampler = 26,
        OpTypeSampledImage = 27,
        OpTypeArray = 28,
        OpTypeRuntimeArray = 29,
        OpTypeStruct = 30,
        OpTypeOpaque = 31,
        OpTypePointer = 32,
        OpTypeFunction = 33,
        OpTypeEvent = 34,
        OpTypeDeviceEvent = 35,
        OpTypeReserveId = 36,
        OpTypeQueue = 37,
        OpTypePipe = 38,
        OpTypeForwardPointer = 39,
        OpConstantTrue = 41,
        OpConstantFalse = 42,
        OpConstant = 43,
        OpConstantComposite = 44,
        OpConstantSampler = 45,
        OpConstantNull = 46,
        OpSpecConstantTrue = 48,
        OpSpecConstantFalse = 49,
        OpSpecConstant = 50,
        OpSpecConstantComposite = 51,
        OpSpecConstantOp = 52,
        OpFunction = 54,
        OpFunctionParameter = 55,
        OpFunctionEnd = 56,
        OpFunctionCall = 57,
        OpVariable = 59,
        OpImageTexelPointer = 60,
        OpLoad = 61,
        OpStore = 62,
        OpCopyMemory = 63,
        OpCopyMemorySized = 64,
        OpAccessChain = 65,
        OpInBoundsAccessChain = 66,
        OpPtrAccessChain = 67,
        OpArrayLength = 68,
        OpGenericPtrMemSemantics = 69,
        OpInBoundsPtrAccessChain = 70,
        OpDecorate = 71,
        OpMemberDecorate = 72,
        OpDecorationGroup = 73,
        OpGroupDecorate = 74,
        OpGroupMemberDecorate = 75,
        OpVectorExtractDynamic = 77,
        OpVectorInsertDynamic = 78,
        OpVectorShuffle = 79,
        OpCompositeConstruct = 80,
        OpCompositeExtract = 81,
        OpCompositeInsert = 82,
        OpCopyObject = 83,
        OpTranspose = 84,
        OpSampledImage = 86,
        OpImageSampleImplicitLod = 87,
        OpImageSampleExplicitLod = 88,
        OpImageSampleDrefImplicitLod = 89,
        OpImageSampleDrefExplicitLod = 90,
        OpImageSampleProjImplicitLod = 91,
        OpImageSampleProjExplicitLod = 92,
        OpImageSampleProjDrefImplicitLod = 93,
        OpImageSampleProjDrefExplicitLod = 94,
        OpImageFetch = 95,
        OpImageGather = 96,
        OpImageDrefGather = 97,
        OpImageRead = 98,
        OpImageWrite = 99,
        OpImage = 100,
        OpImageQueryFormat = 101,
        OpImageQueryOrder = 102,
        OpImageQuerySizeLod = 103,
        OpImageQuerySize = 104,
        OpImageQueryLod = 105,
        OpImageQueryLevels = 106,
        OpImageQuerySamples = 107,
        OpConvertFToU = 109,
        OpConvertFToS = 110,
        OpConvertSToF = 111,
        OpConvertUToF = 112,
        OpUConvert = 113,
        OpSConvert = 114,
        OpFConvert = 115,
        OpQuantizeToF16 = 116,
        OpConvertPtrToU = 117,
        OpSatConvertSToU = 118,
        OpSatConvertUToS = 119,
        OpConvertUToPtr = 120,
        OpPtrCastToGeneric = 121,
        OpGenericCastToPtr = 122,
        OpGenericCastToPtrExplicit = 123,
        OpBitcast = 124,
        OpSNegate = 126,
        OpFNegate = 127,
        OpIAdd = 128,
        OpFAdd = 129,
        OpISub = 130,
        OpFSub = 131,
        OpIMul = 132,
        OpFMul = 133,
        OpUDiv = 134,
        OpSDiv = 135,
        OpFDiv = 136,
        OpUMod = 137,
        OpSRem = 138,
        OpSMod = 139,
        OpFRem = 140,
        OpFMod = 141,
        OpVectorTimesScalar = 142,
        OpMatrixTimesScalar = 143,
        OpVectorTimesMatrix = 144,
        OpMatrixTimesVector = 145,
        OpMatrixTimesMatrix = 146,
        OpOuterProduct = 147,
        OpDot = 148,
        OpIAddCarry = 149,
        OpISubBorrow = 150,
        OpUMulExtended = 151,
        OpSMulExtended = 152,
        OpAny = 154,
        OpAll = 155,
        OpIsNan = 156,
        OpIsInf = 157,
        OpIsFinite = 158,
        OpIsNormal = 159,
        OpSignBitSet = 160,
        OpLessOrGreater = 161,
        OpOrdered = 162,
        OpUnordered = 163,
        OpLogicalEqual = 164,
        OpLogicalNotEqual = 165,
        OpLogicalOr = 166,
        OpLogicalAnd = 167,
        OpLogicalNot = 168,
        OpSelect = 169,
        OpIEqual = 170,
        OpINotEqual = 171,
        OpUGreaterThan = 172,
        OpSGreaterThan = 173,
        OpUGreaterThanEqual = 174,
        OpSGreaterThanEqual = 175,
        OpULessThan = 176,
        OpSLessThan = 177,
        OpULessThanEqual = 178,
        OpSLessThanEqual = 179,
        OpFOrdEqual = 180,
        OpFUnordEqual = 181,
        OpFOrdNotEqual = 182,
        OpFUnordNotEqual = 183,
        OpFOrdLessThan = 184,
        OpFUnordLessThan = 185,
        OpFOrdGreaterThan = 186,
        OpFUnordGreaterThan = 187,
        OpFOrdLessThanEqual = 188,
        OpFUnordLessThanEqual = 189,
        OpFOrdGreaterThanEqual = 190,
        OpFUnordGreaterThanEqual = 191,
        OpShiftRightLogical = 194,
        OpShiftRightArithmetic = 195,
        OpShiftLeftLogical = 196,
        OpBitwiseOr = 197,
        OpBitwiseXor = 198,
        OpBitwiseAnd = 199,
        OpNot = 200,
        OpBitFieldInsert = 201,
        OpBitFieldSExtract = 202,
        OpBitFieldUExtract = 203,
        OpBitReverse = 204,
        OpBitCount = 205,
        OpDPdx = 207,
        OpDPdy = 208,
        OpFwidth = 209,
        OpDPdxFine = 210,
        OpDPdyFine = 211,
        OpFwidthFine = 212,
        OpDPdxCoarse = 213,
        OpDPdyCoarse = 214,
        OpFwidthCoarse = 215,
        OpEmitVertex = 218,
        OpEndPrimitive = 219,
        OpEmitStreamVertex = 220,
        OpEndStreamPrimitive = 221,
        OpControlBarrier = 224,
        OpMemoryBarrier = 225,
        OpAtomicLoad = 227,
        OpAtomicStore = 228,
        OpAtomicExchange = 229,
        OpAtomicCompareExchange = 230,
        OpAtomicCompareExchangeWeak = 231,
        OpAtomicIIncrement = 232,
        OpAtomicIDecrement = 233,
        OpAtomicIAdd = 234,
        OpAtomicISub = 235,
        OpAtomicSMin = 236,
        OpAtomicUMin = 237,
        OpAtomicSMax = 238,
        OpAtomicUMax = 239,
        OpAtomicAnd = 240,
        OpAtomicOr = 241,
        OpAtomicXor = 242,
        OpPhi = 245,
        OpLoopMerge = 246,
        OpSelectionMerge = 247,
        OpLabel = 248,
        OpBranch = 249,
        OpBranchConditional = 250,
        OpSwitch = 251,
        OpKill = 252,
        OpReturn = 253,
        OpReturnValue = 254,
        OpUnreachable = 255,
        OpLifetimeStart = 256,
        OpLifetimeStop = 257,
        OpGroupAsyncCopy = 259,
        OpGroupWaitEvents = 260,
        OpGroupAll = 261,
        OpGroupAny = 262,
        OpGroupBroadcast = 263,
        OpGroupIAdd = 264,
        OpGroupFAdd = 265,
        OpGroupFMin = 266,
        OpGroupUMin = 267,
        OpGroupSMin = 268,
        OpGroupFMax = 269,
        OpGroupUMax = 270,
        OpGroupSMax = 271,
        OpReadPipe = 274,
        OpWritePipe = 275,
        OpReservedReadPipe = 276,
        OpReservedWritePipe = 277,
        OpReserveReadPipePackets = 278,
        OpReserveWritePipePackets = 279,
        OpCommitReadPipe = 280,
        OpCommitWritePipe = 281,
        OpIsValidReserveId = 282,
        OpGetNumPipePackets = 283,
        OpGetMaxPipePackets = 284,
        OpGroupReserveReadPipePackets = 285,
        OpGroupReserveWritePipePackets = 286,
        OpGroupCommitReadPipe = 287,
        OpGroupCommitWritePipe = 288,
        OpEnqueueMarker = 291,
        OpEnqueueKernel = 292,
        OpGetKernelNDrangeSubGroupCount = 293,
        OpGetKernelNDrangeMaxSubGroupSize = 294,
        OpGetKernelWorkGroupSize = 295,
        OpGetKernelPreferredWorkGroupSizeMultiple = 296,
        OpRetainEvent = 297,
        OpReleaseEvent = 298,
        OpCreateUserEvent = 299,
        OpIsValidEvent = 300,
        OpSetUserEventStatus = 301,
        OpCaptureEventProfilingInfo = 302,
        OpGetDefaultQueue = 303,
        OpBuildNDRange = 304,
        OpImageSparseSampleImplicitLod = 305,
        OpImageSparseSampleExplicitLod = 306,
        OpImageSparseSampleDrefImplicitLod = 307,
        OpImageSparseSampleDrefExplicitLod = 308,
        OpImageSparseSampleProjImplicitLod = 309,
        OpImageSparseSampleProjExplicitLod = 310,
        OpImageSparseSampleProjDrefImplicitLod = 311,
        OpImageSparseSampleProjDrefExplicitLod = 312,
        OpImageSparseFetch = 313,
        OpImageSparseGather = 314,
        OpImageSparseDrefGather = 315,
        OpImageSparseTexelsResident = 316,
        OpNoLine = 317,
        OpAtomicFlagTestAndSet = 318,
        OpAtomicFlagClear = 319,
        OpImageSparseRead = 320,
        OpSizeOf = 321,
        OpTypePipeStorage = 322,
        OpConstantPipeStorage = 323,
        OpCreatePipeFromPipeStorage = 324,
        OpGetKernelLocalSizeForSubgroupCount = 325,
        OpGetKernelMaxNumSubgroups = 326,
        OpTypeNamedBarrier = 327,
        OpNamedBarrierInitialize = 328,
        OpMemoryNamedBarrier = 329,
        OpModuleProcessed = 330,
        OpExecutionModeId = 331,
        OpDecorateId = 332,
        OpSubgroupBallotKHR = 4421,
        OpSubgroupFirstInvocationKHR = 4422,
        OpSubgroupAllKHR = 4428,
        OpSubgroupAnyKHR = 4429,
        OpSubgroupAllEqualKHR = 4430,
        OpSubgroupReadInvocationKHR = 4432,
        OpGroupIAddNonUniformAMD = 5000,
        OpGroupFAddNonUniformAMD = 5001,
        OpGroupFMinNonUniformAMD = 5002,
        OpGroupUMinNonUniformAMD = 5003,
        OpGroupSMinNonUniformAMD = 5004,
        OpGroupFMaxNonUniformAMD = 5005,
        OpGroupUMaxNonUniformAMD = 5006,
        OpGroupSMaxNonUniformAMD = 5007,
        OpFragmentMaskFetchAMD = 5011,
        OpFragmentFetchAMD = 5012,
        OpSubgroupShuffleINTEL = 5571,
        OpSubgroupShuffleDownINTEL = 5572,
        OpSubgroupShuffleUpINTEL = 5573,
        OpSubgroupShuffleXorINTEL = 5574,
        OpSubgroupBlockReadINTEL = 5575,
        OpSubgroupBlockWriteINTEL = 5576,
        OpSubgroupImageBlockReadINTEL = 5577,
        OpSubgroupImageBlockWriteINTEL = 5578,
        OpDecorateStringGOOGLE = 5632,
        OpMemberDecorateStringGOOGLE = 5633,
    },

}

