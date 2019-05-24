SamplerState       g_sSamp : register(s0);
uniform sampler2D          g_sSamp2d;

Texture1D          g_tTex1df4a : register(t1);

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

struct PS_OUTPUT
{
    float4 Color : SV_Target0;
    float  Depth : SV_Depth;
};

uniform float  c1;
uniform float2 c2;
uniform float3 c3;
uniform float4 c4;

PS_OUTPUT main()
{
   PS_OUTPUT psout;

   // no 1D gathers

   float4 txval00 = g_tTex2df4 . GatherRed(g_sSamp, c2);
   int4   txval01 = g_tTex2di4 . GatherRed(g_sSamp, c2);
   uint4  txval02 = g_tTex2du4 . GatherRed(g_sSamp, c2);

   float4 txval10 = g_tTex2df4 . GatherGreen(g_sSamp, c2);
   int4   txval11 = g_tTex2di4 . GatherGreen(g_sSamp, c2);
   uint4  txval12 = g_tTex2du4 . GatherGreen(g_sSamp, c2);

   float4 txval20 = g_tTex2df4 . GatherBlue(g_sSamp, c2);
   int4   txval21 = g_tTex2di4 . GatherBlue(g_sSamp, c2);
   uint4  txval22 = g_tTex2du4 . GatherBlue(g_sSamp, c2);

   float4 txval30 = g_tTex2df4 . GatherAlpha(g_sSamp, c2);
   int4   txval31 = g_tTex2di4 . GatherAlpha(g_sSamp, c2);
   uint4  txval32 = g_tTex2du4 . GatherAlpha(g_sSamp, c2);

   // no 3D gathers

   float4 txval40 = g_tTexcdf4 . GatherRed(g_sSamp, c3);
   int4   txval41 = g_tTexcdi4 . GatherRed(g_sSamp, c3);
   uint4  txval42 = g_tTexcdu4 . GatherRed(g_sSamp, c3);

   float4 txval50 = g_tTexcdf4 . GatherGreen(g_sSamp, c3);
   int4   txval51 = g_tTexcdi4 . GatherGreen(g_sSamp, c3);
   uint4  txval52 = g_tTexcdu4 . GatherGreen(g_sSamp, c3);

   float4 txval60 = g_tTexcdf4 . GatherBlue(g_sSamp, c3);
   int4   txval61 = g_tTexcdi4 . GatherBlue(g_sSamp, c3);
   uint4  txval62 = g_tTexcdu4 . GatherBlue(g_sSamp, c3);

   float4 txval70 = g_tTexcdf4 . GatherAlpha(g_sSamp, c3);
   int4   txval71 = g_tTexcdi4 . GatherAlpha(g_sSamp, c3);
   uint4  txval72 = g_tTexcdu4 . GatherAlpha(g_sSamp, c3);

   psout.Color = 1.0;
   psout.Depth = 1.0;

   return psout;
}
