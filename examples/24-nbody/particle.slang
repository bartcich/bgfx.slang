uniform float4x4 u_view;
uniform float4x4 u_proj;
uniform float4 u_params[3];

#define u_timeStep u_params[0].x
#define u_dispatchSize floatBitsToUint(u_params[0].y)
#define u_gravity u_params[0].z
#define u_damping u_params[0].w

#define u_particleIntensity u_params[1].x
#define u_particleSize u_params[1].y
#define u_baseSeed floatBitsToUint(u_params[1].z)
#define u_particlePower u_params[1].w

#define u_initialSpeed u_params[2].x
#define u_initialShape floatBitsToUint(u_params[2].y)
#define u_maxAcceleration u_params[2].z

struct VertInput {
  float2 position : POSITION;
  float4 data0 : TEXCOORD7;
};

struct Vertex {
  float4 sv_position : SV_Position = float4(0.0, 0.0, 0.0, 0.0);
  float3 texcoord : TEXCOORD0 = float3(0.0, 0.0, 0.0);
};

[shader("vertex")]
Vertex vertexMain(VertInput input) {
  Vertex output;

  float3 eye = mul(u_view, float4(input.data0.xyz, 1.0)).xyz;
  float3 up = normalize(cross(eye, float3(1.0, 0.0, 0.0)));
  float3 right = normalize(cross(up, eye));
  float size = u_particleSize;
  float3 position = eye + size * right * input.position.x + size * up * input.position.y;

  output.texcoord.xy = input.position;
  output.texcoord.z = input.data0.w;
  output.sv_position = mul(u_proj, float4(position, 1.0));

  return output;
}

[shader("fragment")]
float4 fragmentMain(Vertex vertex) : SV_Target {

  float3 color = lerp(float3(0.1, 0.8, 0.2), 0.5 * float3(0.3, 0.25, 0.05), vertex.texcoord.z);
  float f = u_particleIntensity * pow(max(1.0 - length(vertex.texcoord.xy), 0.0), u_particlePower);
  return float4(color * f, 1.0);
}
