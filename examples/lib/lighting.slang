module lighting;

uniform float4 u_lightPosRadius[4];
uniform float4 u_lightRgbInnerR[4];

float2 blinn(float3 _lightDir, float3 _normal, float3 _viewDir) {
  float ndotl = dot(_normal, _lightDir);
  float3 reflected = 2.0 * ndotl * _normal - _lightDir;
  float rdotv = dot(reflected, _viewDir);
  return float2(ndotl, rdotv);
}

float fresnel(float _ndotl, float _bias, float _pow) {
  float facing = 1.0 - _ndotl;
  return max(_bias + (1.0 - _bias) * pow(facing, _pow), 0.0);
}

float4 lit(float _ndotl, float _rdotv, float _m) {
  float diff = max(0.0, _ndotl);
  float spec = step(0.0, _ndotl) * max(0.0, _rdotv * _m);
  return float4(1.0, diff, spec, 1.0);
}

float4 powRgba(float4 _rgba, float _pow) {
  float4 result;
  result.xyz = pow(_rgba.xyz, _pow);
  result.w = _rgba.w;
  return result;
}

public float3 calcLight(int _idx, float3x3 _tbn, float3 _wpos, float3 _normal, float3 _view) {
  float3 lp = u_lightPosRadius[_idx].xyz - _wpos;
  float attn = 1.0 - smoothstep(u_lightRgbInnerR[_idx].w, 1.0, length(lp) / u_lightPosRadius[_idx].w);
  float3 lightDir = mul(normalize(lp), _tbn);
  float2 bln = blinn(lightDir, _normal, _view);
  float4 lc = lit(bln.x, bln.y, 1.0);
  float3 rgb = u_lightRgbInnerR[_idx].xyz * saturate(lc.y) * attn;
  return rgb;
}
