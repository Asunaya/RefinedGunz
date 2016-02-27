matrix  g_mWorldView;
matrix  g_mProj;

void VSPos( float4 Pos : POSITION, out float3 oPos : TEXCOORD0)
{
    oPos = mul( float4( mul( Pos, g_mWorldView ), 1.0f ), g_mProj );
}

void PSPos( float3 Pos : TEXCOORD0, out float4 oPos : COLOR0 )
{
    //
    // Output view position
    //
    oPos = float4( Pos, 1.0f );
}

technique PostProcess
{
    pass p0
    {
        VertexShader = compile vs_2_0 VSPos();
        PixelShader = compile ps_2_0 PSPos();
        ZEnable = true;
    }
}