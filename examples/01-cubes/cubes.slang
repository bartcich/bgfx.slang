uniform float4x4 u_modelViewProj;

struct VertexInput {
  float3 position : POSITION;
  float4 color : COLOR;
};

struct Vertex {
  float4 sv_position : SV_Position;
  float4 color : COLOR;
};

[shader("vertex")]
Vertex vertexMain(VertexInput input) {
  Vertex output;

  output.sv_position = mul(u_modelViewProj, float4(input.position, 1.0));
  output.color = input.color;
  return output;
}

[shader("fragment")]
float4 fragmentMain(Vertex vertex) : SV_Target { return vertex.color; }
