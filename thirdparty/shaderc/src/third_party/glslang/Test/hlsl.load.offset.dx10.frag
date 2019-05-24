SamplerState       g_sSamp : register(s0);

uniform Texture1D <float4> g_tTex1df4 : register(t0);
Texture1D <int4>   g_tTex1di4;
Texture1D <uint4>  g_tTex1du4;

Texture2D <float4> g_tTex2df4;
Texture2D <int4>   g_tTex2di4;
Texture2D <uint4>  g_tTex2du4;

Texture3D <float4> g_tTex3df4;
Texture3D <int4>   g_tTex3di4;
Texture3D <uint4>  g_tTex3du4;

TextureCube <float4> g_tTexcdf4;
TextureCube <int4>   g_tTexcdi4;
TextureCube <uint4>  g_tTexcdu4;

Texture1DArray <float4> g_tTex1df4a;
Texture1DArray <int4>   g_tTex1di4a;
Texture1DArray <uint4>  g_tTex1du4a;

Texture2DArray <float4> g_tTex2df4a;
Texture2DArray <int4>   g_tTex2di4a;
Texture2DArray <uint4>  g_tTex2du4a;

TextureCubeArray <float4> g_tTexcdf4a;
TextureCubeArray <int4>   g_tTexcdi4a;
TextureCubeArray <uint4>  g_tTexcdu4a;

struct PS_OUTPUT
{
    float4 Color : SV_Target0;
    float  Depth : SV_Depth;
};

uniform int   c1;
uniform int2  c2;
uniform int3  c3;
uniform int4  c4;

uniform int   o1;
uniform int2  o2;
uniform int3  o3;
uniform int4  o4;

PS_OUTPUT main()
{
   PS_OUTPUT psout;

   // 1D
   g_tTex1df4.Load(c2, o1);
   g_tTex1di4.Load(c2, o1);
   g_tTex1du4.Load(c2, o1);

   // 2D
   g_tTex2df4.Load(c3, o2);
   g_tTex2di4.Load(c3, o2);
   g_tTex2du4.Load(c3, o2);

   // 3D
   g_tTex3df4.Load(c4, o3);
   g_tTex3di4.Load(c4, o3);
   g_tTex3du4.Load(c4, o3);

   // Offset has no Cube or CubeArray forms

   // TODO:
   // Load, SampleIndex
   // Load, SampleIndex, Offset

   psout.Color = 1.0;
   psout.Depth = 1.0;

   return psout;
}
