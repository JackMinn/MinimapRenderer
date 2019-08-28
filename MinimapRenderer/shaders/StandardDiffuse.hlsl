

cbuffer PerDraw : register(b1)
{
    float4x4 localToWorld;
    float4x4 worldToLocal;
}

cbuffer PerFrame : register(b2)
{
    float4x4 viewMatrix;
    float4 worldCameraPos;
}

cbuffer PerApplication : register(b3)
{
    float4x4 projectionMatrix;
	float4x4 invProjectionMatrix;
}

Texture2D diffuse;
SamplerState diffuseSampler;

float4 tex_ST;

struct VertexInputType
{
    float4 position : POSITION;
	float4 normal : NORMAL;
    float4 uv 		: TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
	float3 normal : NORMAL;
    float2 uv 		: TEXCOORD0;
	float3 worldPos : TEXCOORD1;
};

PixelInputType VSMain(VertexInputType input)
{
    PixelInputType output;
    
    input.position.w = 1.0f;
	//input.position.xyz *= tex_ST.x;

    output.position = mul(input.position, localToWorld);
	output.worldPos = output.position.xyz;
    output.position = mul(output.position, viewMatrix); //this viewMatrix is designed for directX clip space, cannot extract camera position from it
    output.position = mul(output.position, projectionMatrix); 
		
	// Is this the inverse or the inverse transpose?
	output.normal = mul((float3x3) worldToLocal, input.normal.xyz);
	output.uv = float2(input.uv.xy);
    
    return output;
}

float4 PSMain(PixelInputType input) : SV_TARGET
{
	float4 result;

	float4 sample = diffuse.Sample(diffuseSampler, input.uv);
    result.rgb = sample.rgb;
	result.a = 1;
	
	clip(sample.a < 0.25 ? -1 : 1);
	
	float3 viewDir = normalize(input.worldPos - worldCameraPos.xyz);
	float3 normal = normalize(input.normal);
	
	float attenuation = dot(-viewDir, normal);
	attenuation = pow(attenuation, 0.8);
	//result.rgb = attenuation;
	result.rgb *= attenuation;
	
	//result.rgba = 1;
	
    return result;
}