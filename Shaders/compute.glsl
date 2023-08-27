#version 450

struct Particle
{
    vec3 pos;
    vec3 vel;
    vec3 rgb;
};

layout (binding = 0) uniform UniformBufferObject
{
    float MAX_SPEED;
    float ATTRACTION;
    float WALL_AVOIDANCE;
    float ATTRACTION_DISTANCE;
    float ALIGNMENT_DISTANCE;
    float ALIGNMENT;
    float AVOIDANCE_DISTANCE;
    float AVOIDANCE;
    float VORTEX_FORCE;
} ubo;

layout(std140, binding = 1) readonly buffer ParticleDataRead
{
   Particle particlesRead[];
};

layout(std140, binding = 2) buffer ParticleDataWrite
{
   Particle particlesWrite[];
};

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;



int N = 30000;
float FIELD_SCALE = 1.0;

float MAX_SPEED = 0.0018;
float WALL_AVOIDANCE = 0.00002;

float ATTRACTION = 0.00042;
float ATTRACTION_DISTANCE = 0.05;

float ALIGNMENT = 0.0036;
float ALIGNMENT_DISTANCE = 0.05;

float AVOIDANCE = 0.0002;
float AVOIDANCE_DISTANCE = 0.015;

void main()
{
    uint id = gl_GlobalInvocationID.x;
    vec3 pos = particlesRead[id].pos;
    vec3 vel = particlesRead[id].vel;
    vec3 acc = vec3(0.0);
    
    
    if(pos.x > FIELD_SCALE) acc.x += -ubo.WALL_AVOIDANCE;
    if(pos.x < 0.0) acc.x += ubo.WALL_AVOIDANCE;
    if(pos.y > FIELD_SCALE) acc.y += -ubo.WALL_AVOIDANCE;
    if(pos.y < 0.0) acc.y += ubo.WALL_AVOIDANCE;
    if(pos.z > FIELD_SCALE) acc.z += -ubo.WALL_AVOIDANCE;
    if(pos.z < 0.0) acc.z += ubo.WALL_AVOIDANCE;
    
    
    vec3 attractionPosSum = vec3(0,0, 0);
    int attractionNearCnt = 0;
    vec3 alignmentVelSum = vec3(0,0,0);
    int alignmentNearCnt = 0;
    vec3 avoidanceSum = vec3(0,0,0);
    int avoidanceNearCnt = 0;
    
    for(uint i = 0 ; i < N; i++)
    {
        vec3 p = particlesRead[i].pos;
        vec3 v = particlesRead[i].vel;
        float dist = length(p - pos);
        
        if(dist < ubo.ATTRACTION_DISTANCE)
        {
            attractionPosSum += p;
            attractionNearCnt++;
        }
        
        if(dist < ubo.ALIGNMENT_DISTANCE)
        {
            alignmentVelSum += v;
            alignmentNearCnt++;
        }
        
        if(dist < ubo.AVOIDANCE_DISTANCE)
        {
            avoidanceSum += pos - p;
            avoidanceNearCnt++;
        }
    }
    
    if(attractionNearCnt > 0)
    {
        vec3 meanPos = attractionPosSum / attractionNearCnt;
        vec3 attractionForce = (meanPos - pos) * ubo.ATTRACTION;
        acc += attractionForce;
    }
    if(alignmentNearCnt > 0)
    {
        vec3 meanVel = alignmentVelSum / alignmentNearCnt;
        vec3 alignmentForce = meanVel * ubo.ALIGNMENT;
        acc += alignmentForce;
    }
    if(avoidanceNearCnt > 0)
    {
        acc += avoidanceSum * ubo.AVOIDANCE;
    }
    
    vec3 vortexForce = cross(pos - vec3(0.5, 0.5, 0.5), vec3(1.0, 0.0, 0.0));
    acc += vortexForce * ubo.VORTEX_FORCE;
    
    
    vel += acc;
    if(length(vel) > ubo.MAX_SPEED) vel = normalize(vel) *  ubo.MAX_SPEED;
    
    
    particlesWrite[id].pos = pos + vel;
    particlesWrite[id].vel = vel;
}
