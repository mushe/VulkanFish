#version 450
float FISH_SCALE = 0.035;



layout(binding = 0)
uniform UniformBufferObject
{
    mat4 view;
    mat4 proj;
} ubo;

struct Particle
{
    vec3 pos;
    vec3 vel;
    vec3 rgb;
};

layout(std140, binding = 2) readonly buffer ParticleData
{
    Particle particles[];
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 outFragColor;
layout(location = 1) out vec2 outFragTexCoord;



// q_1q_2  =  (w_1w_2 - v_1・v_2, w_1v_2 + w_2v_1  + v_1×v_2)
vec4 multiplyQuaternion(vec4 a, vec4 b)
{
    float w1 = a.x;
    float w2 = b.x;
    vec3 v1 = a.yzw;
    vec3 v2 = b.yzw;

    return vec4
    (
        w1*w2 - dot(v1, v2),
        w1*v2 + w2*v1 + cross(v1, v2)
    );
}

// q = (v, v)
// q^-1 = (w,-v)
vec4 invertQuaternion(vec4 q)
{
    return vec4(q.x, -q.y, -q.z, -q.w);
}
// p´ = qpq^{-1}
vec3 rotate(vec3 pos, vec4 q)
{
    vec4 qp = multiplyQuaternion(q, vec4(0, pos));
    vec4 q_inv = invertQuaternion(q);
    vec4 qpq_inv = multiplyQuaternion(qp, q_inv);
    return qpq_inv.yzw;
}

vec4 fromToQuaternion(vec3 p1, vec3 p2)
{
    p1 = normalize(p1);
    p2 = normalize(p2);
    vec3 n = cross(p1, p2);
    float co = dot(p1, p2);
    return normalize( vec4(1+co, n) );
}

vec4 lookAtQuaternion(vec3 eyePos, vec3 targetPos)
{
    return fromToQuaternion(vec3(0,1,0), targetPos - eyePos);
}



void main()
{
    vec4 q = lookAtQuaternion(vec3(0,0,0), particles[gl_InstanceIndex].vel);
    
    gl_Position = ubo.proj * ubo.view  * vec4(rotate(inPosition * FISH_SCALE, q) * 0.5 + particles[gl_InstanceIndex].pos, 1.0);
    
    outFragColor = particles[gl_InstanceIndex].rgb;
    outFragTexCoord = inTexCoord;
}
