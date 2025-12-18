uniform float4x4 u_viewProj;

struct VertexInput {
  float3 position : POSITION;
  float4 color : COLOR;
  float4 data0 : TEXCOORD7;
  float4 data1 : TEXCOORD6;
  float4 data2 : TEXCOORD5;
  float4 data3 : TEXCOORD4;
  float4 data4 : TEXCOORD3;
};

struct Vertex {
  float4 sv_position : SV_Position;
  float4 color : COLOR;
};

[shader("vertex")]
Vertex vertexMain(VertexInput input) {
  Vertex output;

  float4x4 model = transpose(float4x4(input.data0, input.data1, input.data2, input.data3));

  float4 worldPos = mul(model, float4(input.position, 1.0));

  output.sv_position = mul(u_viewProj, worldPos);
  output.color = input.color * input.data4;

  return output;
}

[shader("fragment")]
float4 fragmentMain(Vertex vertex) : SV_Target { return vertex.color; }
