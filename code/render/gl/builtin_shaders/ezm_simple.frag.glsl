#version 450 core
#extension GL_ARB_bindless_texture : require

//////////////////////////////////////////////////////////
//~ ins
in V2P
{
    vec3 pos;
    vec3 normal;
    vec2 uv;
} v2p;

//////////////////////////////////////////////////////////
//~ outs
out vec4 outColor;

//////////////////////////////////////////////////////////
//~ ssbo
layout (binding = 0, std430) readonly buffer TextureSamplers
{ 
    sampler2D textures[];
};

//////////////////////////////////////////////////////////
//~ uniforms
uniform uint albedoTexId;
uniform vec4 shadeColor;

//////////////////////////////////////////////////////////
//~ main
void main()
{
    vec3 norm = normalize(v2p.normal);
    
    vec3 light_pos = vec3(4.f, 4.f, 4.f);
    float ambient_factor = 0.1f;
    vec3 ambient = ambient_factor * vec3(1.f, 1.f, 1.f);
    
    vec3 light_dir = normalize(light_pos - v2p.pos);
    float diffuse_factor = max(dot(norm, light_dir), 0.f);
    vec3 diffuse = diffuse_factor * vec3(0.8f, 0.8f, 0.8f);
    
    sampler2D albedo = textures[albedoTexId];
    
    vec4 color = texture(albedo, v2p.uv) * vec4((diffuse + ambient), 1.f) * shadeColor;
    outColor = color;
    
    //outColor = vec4(v2p.normal, 1.f);
} 