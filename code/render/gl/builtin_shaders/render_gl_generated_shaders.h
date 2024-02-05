#ifndef RENDERGLGENERATEDSHADERS_INCLUDE_H
#define  RENDERGLGENERATEDSHADERS_INCLUDE_H

global const u8 *triangle_vert_glsl_shader = 	"#version 330 core\n"
	"layout (location = 0) in vec3 aPos;\n"
	"\n"
	"layout (std140) uniform GlobalUBO\n"
	"{\n"
	"    mat4 proj;\n"
	"    mat4 cam;\n"
	"};\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = proj * cam * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
	"}\n"
"\0";

global const u8 *triangle_frag_glsl_shader = 	"#version 330 core\n"
	"out vec4 FragColor;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
	"} \n"
"\0";

global const u8 *ezmsimple_vert_glsl_shader = 	"#version 450 core
	"#extension GL_ARB_shader_storage_buffer_object : require
	"
	"//////////////////////////////////////////////////////////
	"//~ structs
	"struct EzmVertex
	"{
	"    // NOTE(gsp): floats are used here to avoid vec3 being padded to vec4
	"    float p[3];
	"    float n[3];
	"    float uv[2];
	"};
	"
	"//////////////////////////////////////////////////////////
	"//~ ssbos
	"layout (binding = 1, std430) readonly buffer IndexBuffer  { uint      indices[];  };
	"layout (binding = 2, std430) readonly buffer VertexBuffer { EzmVertex vertices[]; };
	"
	"//////////////////////////////////////////////////////////
	"//~ outs
	"out V2P
	"{
	"    vec3 pos;
	"    vec3 normal;
	"    vec2 uv;
	"} v2p;
	"
	"
	"//////////////////////////////////////////////////////////
	"//~ uniforms
	"layout (binding = 0, std140) uniform GlobalUBO
	"{
	"    mat4 cam;
	"    mat4 proj;
	"    vec2 viewport;
	"    vec2 pad;
	"};
	"
	"uniform mat4 model;
	"
	"//////////////////////////////////////////////////////////
	"//~ Fwd decls
	"vec3 GetPos(uint index);
	"vec3 GetNormal(uint index);
	"vec2 GetUV(uint index);
	"
	"//////////////////////////////////////////////////////////
	"//~ main
	"void main()
	"{
	"    uint inIndex   = indices[gl_VertexID];
	"    vec3 in_pos    = GetPos(inIndex);
	"    vec3 in_normal = GetNormal(inIndex);
	"    vec2 in_uv     = GetUV(inIndex);
	"    
	"    vec4 worldPos = model * vec4(in_pos, 1.f);
	"    vec4 final_vert_pos = proj * cam * worldPos;
	"    gl_Position = final_vert_pos;
	"    
	"    v2p.pos    = worldPos.xyz;
	"    v2p.normal = in_normal;
	"    v2p.uv     = in_uv;
	"}
	"
	"//////////////////////////////////////////////////////////
	"//~ Helpers
	"
	"vec3 GetPos(uint index)
	"{
	"    return vec3(vertices[index].p[0],
	"                vertices[index].p[1],
	"                vertices[index].p[2]);
	"}
	"
	"vec3 GetNormal(uint index)
	"{
	"    return vec3(vertices[index].n[0],
	"                vertices[index].n[1],
	"                vertices[index].n[2]);
	"}
	"
	"vec2 GetUV(uint index)
	"{
	"    return vec2(vertices[index].uv[0],
	"                vertices[index].uv[1]);
	"}\n"
"\0";

global const u8 *ezmsimple_frag_glsl_shader = 	"#version 450 core
	"#extension GL_ARB_bindless_texture : require
	"
	"//////////////////////////////////////////////////////////
	"//~ ins
	"in V2P
	"{
	"    vec3 pos;
	"    vec3 normal;
	"    vec2 uv;
	"} v2p;
	"
	"//////////////////////////////////////////////////////////
	"//~ outs
	"out vec4 outColor;
	"
	"//////////////////////////////////////////////////////////
	"//~ ssbo
	"layout (binding = 0, std430) readonly buffer TextureSamplers
	"{ 
	"    sampler2D textures[];
	"};
	"
	"//////////////////////////////////////////////////////////
	"//~ uniforms
	"uniform uint albedoTexId;
	"uniform vec4 shadeColor;
	"
	"//////////////////////////////////////////////////////////
	"//~ main
	"void main()
	"{
	"    vec3 norm = normalize(v2p.normal);
	"    
	"    vec3 light_pos = vec3(4.f, 4.f, 4.f);
	"    float ambient_factor = 0.1f;
	"    vec3 ambient = ambient_factor * vec3(1.f, 1.f, 1.f);
	"    
	"    vec3 light_dir = normalize(light_pos - v2p.pos);
	"    float diffuse_factor = max(dot(norm, light_dir), 0.f);
	"    vec3 diffuse = diffuse_factor * vec3(0.8f, 0.8f, 0.8f);
	"    
	"    sampler2D albedo = textures[albedoTexId];
	"    
	"    vec4 color = texture(albedo, v2p.uv) * vec4((diffuse + ambient), 1.f) * shadeColor;
	"    outColor = color;
	"    
	"    //outColor = vec4(v2p.normal, 1.f);
	"} \n"
"\0";

global const u8 *uishader_vert_glsl_shader = 	"#version 450 core
	"
	"//////////////////////////////////////////////////////////
	"//~ structs
	"struct QuadInfo
	"{
	"    // NOTE(gsp): floats are used here to avoid vec3 being padded to vec4
	"    float rect[4];
	"    float uv[4];
	"    float color[4];
	"    uint  texId;
	"    uint  pad[3];
	"};
	"
	"//////////////////////////////////////////////////////////
	"//~ ssbos
	"layout (binding = 1, std430) readonly buffer Data { QuadInfo data[]; };
	"
	"//////////////////////////////////////////////////////////
	"//~ outs
	"
	"// vertex2pixel
	"out V2P
	"{
	"    smooth vec4 color;
	"    smooth vec2 pos;
	"    flat   vec2 center;
	"    flat   vec2 halfSize;
	"    smooth vec2 uv;
	"    flat   uint texId;
	"} v2p;
	"
	"////////////////////
	"//~ uniforms
	"layout (binding = 0, std140) uniform GlobalUBO
	"{
	"    mat4 cam;
	"    mat4 proj;
	"    vec2 viewport;
	"    vec2 pad;
	"};
	"
	"//////////////////////////////////////////////////////////
	"//~ Fwd decls
	"vec4 GetRect(uint index);
	"vec4 GetUV(uint index);
	"vec4 GetColor(uint index);
	"uint GetTexId(uint index);
	"
	"//////////////////////////////////////////////////////////
	"//~
	"void main()
	"{
	"    const vec2 vertices[6] = vec2[6](
	"                                     vec2(-1, -1),
	"                                     vec2(-1, +1),
	"                                     vec2(+1, -1),
	"                                     vec2(+1, +1),
	"                                     
	"                                     vec2(+1, +1),
	"                                     vec2(-1, -1)
	"                                     );
	"    
	"    // inputs
	"    const vec4 inRect  = GetRect(gl_VertexID);
	"    const vec4 inUV    = GetUV(gl_VertexID);
	"    const vec4 inColor = GetColor(gl_VertexID);
	"    const uint inTexId = GetTexId(gl_VertexID);
	"    
	"    vec2 p0 = inRect.xy;
	"    vec2 p1 = inRect.zw;
	"    
	"    // 'dst' -> on screen
	"    vec2 dst_halfSize = (p1 - p0) / 2.f;
	"    vec2 dst_center   = (p1 + p0) / 2.f;
	"    vec2 dst_pos = dst_center + dst_halfSize * vertices[gl_VertexID % 6];
	"    
	"    // v2p_uv -> tex coords
	"    vec2 uvp0 = inUV.xy;
	"    vec2 uvp1 = inUV.zw;
	"    vec2 uv_halfSize = (uvp1 - uvp0) / 2.f;
	"    vec2 uv_center   = (uvp1 + uvp0) / 2.f;
	"    
	"    // NOTE(gsp): 2* (x) - 1.f puts origin at top left corner)
	"    gl_Position = vec4(2.f * dst_pos.x / viewport.x - 1.f,
	"                       -2.f * dst_pos.y / viewport.y + 1.f,
	"                       0.f, 1.f);
	"    
	"    // Package output
	"    v2p.color    = inColor;
	"    v2p.pos      = dst_pos;
	"    v2p.center   = dst_center;
	"    v2p.halfSize = dst_halfSize;
	"    v2p.uv       = uv_center + uv_halfSize * vertices[gl_VertexID % 6];
	"    v2p.texId = inTexId;
	"}
	"
	"//////////////////////////////////////////////////////////
	"//~ Helpers
	"vec4 GetRect(uint index)
	"{
	"    return vec4(data[index].rect[0],
	"                data[index].rect[1],
	"                data[index].rect[2],
	"                data[index].rect[3]);
	"}
	"
	"vec4 GetUV(uint index)
	"{
	"    return vec4(data[index].uv[0],
	"                data[index].uv[1],
	"                data[index].uv[2],
	"                data[index].uv[3]);
	"}
	"
	"vec4 GetColor(uint index)
	"{
	"    return vec4(data[index].color[0],
	"                data[index].color[1],
	"                data[index].color[2],
	"                data[index].color[3]);
	"}
	"
	"uint GetTexId(uint index)
	"{
	"    return data[index].texId;
	"}
	"
"\0";

global const u8 *uishader_frag_glsl_shader = 	"#version 450 core
	"#extension GL_ARB_bindless_texture : require
	"
	"//////////////////////////////////////////////////////////
	"//~ in
	"
	"// vertex2pixel
	"in V2P
	"{
	"    smooth vec4 color;
	"    smooth vec2 pos;
	"    flat   vec2 center;
	"    flat   vec2 halfSize;
	"    smooth vec2 uv;
	"    flat   uint texId;
	"} v2p;
	"
	"//////////////////////////////////////////////////////////
	"//~ ssbo
	"layout (binding = 0, std430) readonly buffer TextureSamplers
	"{ 
	"    sampler2D textures[];
	"};
	"
	"//////////////////////////////////////////////////////////
	"//~ out
	"out vec4 outColor;
	"
	"//~ @main
	"void main()
	"{
	"    outColor  = v2p.color;
	"    //outColor *= texture(textures[0], v2p.uv);
	"    outColor *= texture(textures[v2p.texId], v2p.uv);
	"}\n"
"\0";

global const u8 *sprite3dshader_vert_glsl_shader = 	"#version 450 core\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ structs\n"
	"struct VertInfo\n"
	"{\n"
	"    // NOTE(gsp): floats are used here to avoid vec3 being padded to vec4\n"
	"    float uv[4];\n"
	"    float color[4];\n"
	"    uint  texId;\n"
	"    uint  pad[3];\n"
	"};\n"
	"\n"
	"struct InstanceInfo\n"
	"{\n"
	"    // NOTE(gsp): floats are used here to avoid vec3 being padded to vec4\n"
	"    mat4 model;\n"
	"};\n"
	"\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ ssbos\n"
	"layout (binding = 1, std430) readonly buffer Data { VertInfo data[]; };\n"
	"layout (binding = 2, std430) readonly buffer InstanceData { InstanceInfo instance_data[]; };\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ outs\n"
	"\n"
	"// vertex2pixel\n"
	"out V2P\n"
	"{\n"
	"    smooth vec4 color;\n"
	"    smooth vec2 pos;\n"
	"    flat   vec2 center;\n"
	"    flat   vec2 halfSize;\n"
	"    smooth vec2 uv;\n"
	"    flat   uint texId;\n"
	"} v2p;\n"
	"\n"
	"////////////////////\n"
	"//~ uniforms\n"
	"layout (binding = 0, std140) uniform GlobalUBO\n"
	"{\n"
	"    mat4 cam;\n"
	"    mat4 proj;\n"
	"    vec2 viewport;\n"
	"    vec2 pad;\n"
	"};\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ Fwd decls\n"
	"vec4 GetUV(uint index);\n"
	"vec4 GetColor(uint index);\n"
	"uint GetTexId(uint index);\n"
	"mat4 GetModelMat(uint index);\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~\n"
	"void main()\n"
	"{\n"
	"    const vec2 vertices[6] = vec2[6](\n"
	"                                     vec2(-1, -1),\n"
	"                                     vec2( 1, -1),\n"
	"                                     vec2( 1,  1),\n"
	"                                     \n"
	"                                     vec2(-1, -1),\n"
	"                                     vec2( 1,  1),\n"
	"                                     vec2(-1,  1)\n"
	"                                     );\n"
	"    \n"
	"    // inputs\n"
	"    const uint index   = gl_VertexID + gl_InstanceID*6;\n"
	"    const vec4 inUV    = GetUV(index);\n"
	"    const vec4 inColor = GetColor(index);\n"
	"    const uint inTexId = GetTexId(index);\n"
	"    const mat4 model   = GetModelMat(gl_InstanceID);\n"
	"    \n"
	"    vec2 p0 = vec2( 0.5f, 1.f);\n"
	"    vec2 p1 = vec2(-0.5f, 0.f);\n"
	"    \n"
	"    // verts\n"
	"    vec2 dst_halfSize = (p1 - p0) / 2.f;\n"
	"    vec2 dst_center   = (p1 + p0) / 2.f;\n"
	"    vec2 dst_pos      = dst_center + dst_halfSize * vertices[gl_VertexID];\n"
	"    \n"
	"    vec4 pos    = proj * cam * model *vec4(dst_pos, 0.f, 1.f);\n"
	"    gl_Position = pos;\n"
	"    \n"
	"    // v2p_uv\n"
	"    vec2 uvp0 = inUV.xy;\n"
	"    vec2 uvp1 = inUV.zw;\n"
	"    vec2 uv_halfSize = (uvp1 - uvp0) / 2.f;\n"
	"    vec2 uv_center   = (uvp1 + uvp0) / 2.f;\n"
	"    \n"
	"    // Package output\n"
	"    v2p.color    = inColor;\n"
	"    v2p.pos      = dst_pos;\n"
	"    v2p.center   = dst_center;\n"
	"    v2p.halfSize = dst_halfSize;\n"
	"    v2p.uv       = uv_center + uv_halfSize * vertices[gl_VertexID];\n"
	"    v2p.texId    = inTexId;\n"
	"}\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ Helpers\n"
	"vec4 GetUV(uint index)\n"
	"{\n"
	"    return vec4(data[index].uv[0],\n"
	"                data[index].uv[1],\n"
	"                data[index].uv[2],\n"
	"                data[index].uv[3]);\n"
	"}\n"
	"\n"
	"vec4 GetColor(uint index)\n"
	"{\n"
	"    return vec4(data[index].color[0],\n"
	"                data[index].color[1],\n"
	"                data[index].color[2],\n"
	"                data[index].color[3]);\n"
	"}\n"
	"\n"
	"uint GetTexId(uint index)\n"
	"{\n"
	"    return data[index].texId;\n"
	"}\n"
	"\n"
	"mat4 GetModelMat(uint index)\n"
	"{\n"
	"    return instance_data[index].model;\n"
	"}\n"
"\0";

global const u8 *sprite3dshader_frag_glsl_shader = 	"#version 450 core\n"
	"#extension GL_ARB_bindless_texture : require\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ in\n"
	"\n"
	"// vertex2pixel\n"
	"in V2P\n"
	"{\n"
	"    smooth vec4 color;\n"
	"    smooth vec2 pos;\n"
	"    flat   vec2 center;\n"
	"    flat   vec2 halfSize;\n"
	"    smooth vec2 uv;\n"
	"    flat   uint texId;\n"
	"} v2p;\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ ssbo\n"
	"layout (binding = 0, std430) readonly buffer TextureSamplers\n"
	"{ \n"
	"    sampler2D textures[];\n"
	"};\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ out\n"
	"out vec4 outColor;\n"
	"\n"
	"//~ @main\n"
	"void main()\n"
	"{\n"
	"    outColor  = v2p.color;\n"
	"    outColor *= texture(textures[v2p.texId], v2p.uv);\n"
	"    if (outColor.a == 0.f)\n"
	"        discard;\n"
	"}\n"
"\0";


#endif // RENDERGLGENERATEDSHADERS_INCLUDE_H