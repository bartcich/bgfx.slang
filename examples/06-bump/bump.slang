// NOTE: This is only implementation of non-instanced 06-bump example from bgfx. It won't work with m_instancingSupported flag set to true.

import lighting;

uniform float4x4 u_view;
uniform float4x4 u_viewProj;
uniform float4x4 u_model[32];

uniform Texture2D s_texColor : register(t0);
uniform Texture2D s_texNormal : register(t1);
SamplerState sampler : register(s0);

struct VertInput {
  float3 position : POSITION;
  float4 normal : NORMAL;
  float4 tangent : TANGENT;
  float2 texcoord0 : TEXCOORD0;
};

struct Vertex {
  float4 sv_position : SV_Position = float4(0.0, 0.0, 0.0, 0.0);
  float2 texcoord0 : TEXCOORD0 = float2(0.0, 0.0);
  float3 wpos : TEXCOORD1 = float3(0.0, 0.0, 0.0);
  float3 view : TEXCOORD2 = float3(0.0, 0.0, 0.0);
  float3 normal : NORMAL = float3(0.0, 0.0, 1.0);
  float3 tangent : TANGENT = float3(1.0, 0.0, 0.0);
  float3 bitangent : BITANGENT = float3(0.0, 1.0, 0.0);
};

float4 toLinear(float4 _rgba) {
  float4 result;
  result.xyz = pow(_rgba.xyz, 2.2);
  result.w = _rgba.w;
  return result;
}

float4 toGamma(float4 _rgba) {
  float4 result;
  result.xyz = pow(_rgba.xyz, 1.0 / 2.2);
  result.w = _rgba.w;
  return result;
}

[shader("vertex")]
Vertex vertexMain(VertInput input) {
  Vertex output;

  float3 wpos = mul(u_model[0], float4(input.position, 1.0)).xyz;
  output.wpos = wpos;
  output.sv_position = mul(u_viewProj, float4(wpos, 1.0));

  float4 normal = input.normal * 2.0 - 1.0;
  float4 tangent = input.tangent * 2.0 - 1.0;

  float3 wnormal = mul(u_model[0], float4(normal.xyz, 0.0)).xyz;
  float3 wtangent = mul(u_model[0], float4(tangent.xyz, 0.0)).xyz;

  output.normal = normalize(wnormal);
  output.tangent = normalize(wtangent);
  output.bitangent = cross(output.normal, output.tangent) * tangent.w;

  float3x3 tbn = transpose(float3x3(output.tangent, output.bitangent, output.normal));
  float3 weyepos = mul(float4(0.0, 0.0, 0.0, 1.0), u_view).xyz;

  output.view = mul(weyepos - wpos, tbn);
  output.texcoord0 = input.texcoord0;

  return output;
}

[shader("fragment")]
float4 fragmentMain(Vertex vertex) : SV_Target {

  float3x3 tbn = transpose(float3x3(vertex.tangent, vertex.bitangent, vertex.normal));

  float3 normal;
  normal.xyz = s_texNormal.Sample(sampler, vertex.texcoord0).xyz * 2.0 - 1.0;
  normal.z = sqrt(1.0 - dot(normal.xy, normal.xy));
  float3 view = normalize(vertex.view);

  float3 lightColor;
  lightColor = calcLight(0, tbn, vertex.wpos, normal, view);
  lightColor += calcLight(1, tbn, vertex.wpos, normal, view);
  lightColor += calcLight(2, tbn, vertex.wpos, normal, view);
  lightColor += calcLight(3, tbn, vertex.wpos, normal, view);

  float4 color = toLinear(s_texColor.Sample(sampler, vertex.texcoord0));

  color.xyz = max(float3(0.05), lightColor.xyz) * color.xyz;
  color.w = 1.0;
  color = toGamma(color);

  return color;
}
