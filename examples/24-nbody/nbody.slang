uniform float4 u_params[3];

uint floatBitsToUint(float _x) { return asuint(_x); }

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

StructuredBuffer<float4> prevPositionBuffer : register(t0);
StructuredBuffer<float4> currPositionBuffer : register(t1);
RWStructuredBuffer<float4> outPrevPositionBuffer : register(u2);
RWStructuredBuffer<float4> outCurrPositionBuffer : register(u3);

#define GROUP_SIZE 512
groupshared float3 otherEntries[GROUP_SIZE];

float3 calcAcceleration(float3 _currPosition, float3 _otherPosition) {
  float3 difference = _otherPosition - _currPosition;
  float dist2 = dot(difference, difference);
  float dist6 = dist2 * dist2 * dist2;
  float invDist3 = 1.0 / (sqrt(dist6) + 0.1);
  return u_gravity * u_gravity * invDist3 * difference;
}

[shader("compute")]
[numthreads(GROUP_SIZE, 1, 1)]
void update_instances(uint3 threadId: SV_DispatchThreadID, uint groupIndex: SV_GroupIndex) {
  float3 prevPosition = prevPositionBuffer[threadId.x].xyz;
  float3 currPosition = currPositionBuffer[threadId.x].xyz;

  float3 newAcceleration = float3(0.0);

  for (int j = 0; j < int(u_dispatchSize); ++j) {
    otherEntries[groupIndex] = currPositionBuffer[j * GROUP_SIZE + int(groupIndex)].xyz;

    GroupMemoryBarrierWithGroupSync();
    for (int i = 0; i < GROUP_SIZE; ++i) {
      newAcceleration += calcAcceleration(currPosition, otherEntries[i]);
    }
  }

  newAcceleration += (prevPosition - currPosition) * u_damping;
  float accelerationMagnitude = length(newAcceleration);
  float color = pow(min(accelerationMagnitude / 3.0, 1.0), 0.25);
  if (accelerationMagnitude > 0.0) {
    newAcceleration = normalize(newAcceleration) * min(accelerationMagnitude, u_maxAcceleration);
  }

  float3 newPosition = 2.0 * currPosition - prevPosition + newAcceleration * u_timeStep;

  outPrevPositionBuffer[threadId.x] = float4(currPosition, 0.0);
  outCurrPositionBuffer[threadId.x] = float4(newPosition, color);
}
