#version 450 core
#extension GL_ARB_bindless_texture : require

//////////////////////////////////////////////////////////
//~ in

// vertex2pixel
in V2P
{
    smooth vec4 color;
    smooth vec2 pos;
    flat   vec2 center;
    flat   vec2 halfSize;
    smooth vec2 uv;
    flat   uint texId;
} v2p;

//////////////////////////////////////////////////////////
//~ ssbo
layout (binding = 0, std430) readonly buffer TextureSamplers
{ 
    sampler2D textures[];
};

//////////////////////////////////////////////////////////
//~ out
out vec4 outColor;

//~ @main
void main()
{
    outColor  = v2p.color;
    //outColor *= texture(textures[0], v2p.uv);
    outColor *= texture(textures[v2p.texId], v2p.uv);
}