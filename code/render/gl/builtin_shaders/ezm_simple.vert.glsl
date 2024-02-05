#version 450 core
#extension GL_ARB_shader_storage_buffer_object : require

//////////////////////////////////////////////////////////
//~ structs
struct EzmVertex
{
    // NOTE(gsp): floats are used here to avoid vec3 being padded to vec4
    float p[3];
    float n[3];
    float uv[2];
};

//////////////////////////////////////////////////////////
//~ ssbos
layout (binding = 1, std430) readonly buffer IndexBuffer  { uint      indices[];  };
layout (binding = 2, std430) readonly buffer VertexBuffer { EzmVertex vertices[]; };

//////////////////////////////////////////////////////////
//~ outs
out V2P
{
    vec3 pos;
    vec3 normal;
    vec2 uv;
} v2p;


//////////////////////////////////////////////////////////
//~ uniforms
layout (binding = 0, std140) uniform GlobalUBO
{
    mat4 cam;
    mat4 proj;
    vec2 viewport;
    vec2 pad;
};

uniform mat4 model;

//////////////////////////////////////////////////////////
//~ Fwd decls
vec3 GetPos(uint index);
vec3 GetNormal(uint index);
vec2 GetUV(uint index);

//////////////////////////////////////////////////////////
//~ main
void main()
{
    uint inIndex   = indices[gl_VertexID];
    vec3 in_pos    = GetPos(inIndex);
    vec3 in_normal = GetNormal(inIndex);
    vec2 in_uv     = GetUV(inIndex);
    
    vec4 worldPos = model * vec4(in_pos, 1.f);
    vec4 final_vert_pos = proj * cam * worldPos;
    gl_Position = final_vert_pos;
    
    v2p.pos    = worldPos.xyz;
    v2p.normal = in_normal;
    v2p.uv     = in_uv;
}

//////////////////////////////////////////////////////////
//~ Helpers

vec3 GetPos(uint index)
{
    return vec3(vertices[index].p[0],
                vertices[index].p[1],
                vertices[index].p[2]);
}

vec3 GetNormal(uint index)
{
    return vec3(vertices[index].n[0],
                vertices[index].n[1],
                vertices[index].n[2]);
}

vec2 GetUV(uint index)
{
    return vec2(vertices[index].uv[0],
                vertices[index].uv[1]);
}