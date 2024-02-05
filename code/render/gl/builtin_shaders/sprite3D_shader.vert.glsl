#version 450 core

//////////////////////////////////////////////////////////
//~ structs
struct VertInfo
{
    // NOTE(gsp): floats are used here to avoid vec3 being padded to vec4
    float uv[4];
    float color[4];
    uint  texId;
    uint  pad[3];
};

struct InstanceInfo
{
    // NOTE(gsp): floats are used here to avoid vec3 being padded to vec4
    mat4 model;
};


//////////////////////////////////////////////////////////
//~ ssbos
layout (binding = 1, std430) readonly buffer Data { VertInfo data[]; };
layout (binding = 2, std430) readonly buffer InstanceData { InstanceInfo instance_data[]; };

//////////////////////////////////////////////////////////
//~ outs

// vertex2pixel
out V2P
{
    smooth vec4 color;
    smooth vec2 pos;
    flat   vec2 center;
    flat   vec2 halfSize;
    smooth vec2 uv;
    flat   uint texId;
} v2p;

////////////////////
//~ uniforms
layout (binding = 0, std140) uniform GlobalUBO
{
    mat4 cam;
    mat4 proj;
    vec2 viewport;
    vec2 pad;
};

//////////////////////////////////////////////////////////
//~ Fwd decls
vec4 GetUV(uint index);
vec4 GetColor(uint index);
uint GetTexId(uint index);
mat4 GetModelMat(uint index);

//////////////////////////////////////////////////////////
//~
void main()
{
    const vec2 vertices[6] = vec2[6](
                                     vec2(-1, -1),
                                     vec2( 1, -1),
                                     vec2( 1,  1),
                                     
                                     vec2(-1, -1),
                                     vec2( 1,  1),
                                     vec2(-1,  1)
                                     );
    
    // inputs
    const uint index   = gl_VertexID + gl_InstanceID*6;
    const vec4 inUV    = GetUV(index);
    const vec4 inColor = GetColor(index);
    const uint inTexId = GetTexId(index);
    const mat4 model   = GetModelMat(gl_InstanceID);
    
    vec2 p0 = vec2( 0.5f, 1.f);
    vec2 p1 = vec2(-0.5f, 0.f);
    
    // verts
    vec2 dst_halfSize = (p1 - p0) / 2.f;
    vec2 dst_center   = (p1 + p0) / 2.f;
    vec2 dst_pos      = dst_center + dst_halfSize * vertices[gl_VertexID];
    
    vec4 pos    = proj * cam * model *vec4(dst_pos, 0.f, 1.f);
    gl_Position = pos;
    
    // v2p_uv
    vec2 uvp0 = inUV.xy;
    vec2 uvp1 = inUV.zw;
    vec2 uv_halfSize = (uvp1 - uvp0) / 2.f;
    vec2 uv_center   = (uvp1 + uvp0) / 2.f;
    
    // Package output
    v2p.color    = inColor;
    v2p.pos      = dst_pos;
    v2p.center   = dst_center;
    v2p.halfSize = dst_halfSize;
    v2p.uv       = uv_center + uv_halfSize * vertices[gl_VertexID];
    v2p.texId    = inTexId;
}

//////////////////////////////////////////////////////////
//~ Helpers
vec4 GetUV(uint index)
{
    return vec4(data[index].uv[0],
                data[index].uv[1],
                data[index].uv[2],
                data[index].uv[3]);
}

vec4 GetColor(uint index)
{
    return vec4(data[index].color[0],
                data[index].color[1],
                data[index].color[2],
                data[index].color[3]);
}

uint GetTexId(uint index)
{
    return data[index].texId;
}

mat4 GetModelMat(uint index)
{
    return instance_data[index].model;
}