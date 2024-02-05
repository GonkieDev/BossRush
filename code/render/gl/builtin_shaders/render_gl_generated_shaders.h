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

global const u8 *ezmsimple_vert_glsl_shader = 	"#version 450 core\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ structs\n"
	"struct EzmVertex\n"
	"{\n"
	"    // NOTE(gsp): floats are used here to avoid vec3 being padded to vec4\n"
	"    float p[3];\n"
	"    float n[3];\n"
	"    float uv[2];\n"
	"};\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ ssbos\n"
	"layout (binding = 1, std430) readonly buffer IndexBuffer  { uint      indices[];  };\n"
	"layout (binding = 2, std430) readonly buffer VertexBuffer { EzmVertex vertices[]; };\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ outs\n"
	"out V2P\n"
	"{\n"
	"    vec3 pos;\n"
	"    vec3 normal;\n"
	"    vec2 uv;\n"
	"} v2p;\n"
	"\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ uniforms\n"
	"layout (binding = 0, std140) uniform GlobalUBO\n"
	"{\n"
	"    mat4 cam;\n"
	"    mat4 proj;\n"
	"    vec2 viewport;\n"
	"    vec2 pad;\n"
	"};\n"
	"\n"
	"uniform mat4 model;\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ Fwd decls\n"
	"vec3 GetPos(uint index);\n"
	"vec3 GetNormal(uint index);\n"
	"vec2 GetUV(uint index);\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ main\n"
	"void main()\n"
	"{\n"
	"    uint inIndex   = indices[gl_VertexID];\n"
	"    vec3 in_pos    = GetPos(inIndex);\n"
	"    vec3 in_normal = GetNormal(inIndex);\n"
	"    vec2 in_uv     = GetUV(inIndex);\n"
	"    \n"
	"    vec4 worldPos = model * vec4(in_pos, 1.f);\n"
	"    vec4 final_vert_pos = proj * cam * worldPos;\n"
	"    gl_Position = final_vert_pos;\n"
	"    \n"
	"    v2p.pos    = worldPos.xyz;\n"
	"    v2p.normal = in_normal;\n"
	"    v2p.uv     = in_uv;\n"
	"}\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ Helpers\n"
	"\n"
	"vec3 GetPos(uint index)\n"
	"{\n"
	"    return vec3(vertices[index].p[0],\n"
	"                vertices[index].p[1],\n"
	"                vertices[index].p[2]);\n"
	"}\n"
	"\n"
	"vec3 GetNormal(uint index)\n"
	"{\n"
	"    return vec3(vertices[index].n[0],\n"
	"                vertices[index].n[1],\n"
	"                vertices[index].n[2]);\n"
	"}\n"
	"\n"
	"vec2 GetUV(uint index)\n"
	"{\n"
	"    return vec2(vertices[index].uv[0],\n"
	"                vertices[index].uv[1]);\n"
	"}\n"
"\0";

global const u8 *ezmsimple_frag_glsl_shader = 	"#version 450 core\n"
	"#extension GL_ARB_bindless_texture : require\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ ins\n"
	"in V2P\n"
	"{\n"
	"    vec3 pos;\n"
	"    vec3 normal;\n"
	"    vec2 uv;\n"
	"} v2p;\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ outs\n"
	"out vec4 outColor;\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ ssbo\n"
	"layout (binding = 0, std430) readonly buffer TextureSamplers\n"
	"{ \n"
	"    sampler2D textures[];\n"
	"};\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ uniforms\n"
	"uniform uint albedoTexId;\n"
	"uniform vec4 shadeColor;\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ main\n"
	"void main()\n"
	"{\n"
	"    vec3 norm = normalize(v2p.normal);\n"
	"    \n"
	"    vec3 light_pos = vec3(4.f, 4.f, 4.f);\n"
	"    float ambient_factor = 0.1f;\n"
	"    vec3 ambient = ambient_factor * vec3(1.f, 1.f, 1.f);\n"
	"    \n"
	"    vec3 light_dir = normalize(light_pos - v2p.pos);\n"
	"    float diffuse_factor = max(dot(norm, light_dir), 0.f);\n"
	"    vec3 diffuse = diffuse_factor * vec3(0.8f, 0.8f, 0.8f);\n"
	"    \n"
	"    sampler2D albedo = textures[albedoTexId];\n"
	"    \n"
	"    vec4 color = texture(albedo, v2p.uv) * vec4((diffuse + ambient), 1.f) * shadeColor;\n"
	"    outColor = color;\n"
	"    \n"
	"    //outColor = vec4(v2p.normal, 1.f);\n"
	"} \n"
"\0";

global const u8 *uishader_vert_glsl_shader = 	"#version 450 core\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ structs\n"
	"struct QuadInfo\n"
	"{\n"
	"    // NOTE(gsp): floats are used here to avoid vec3 being padded to vec4\n"
	"    float rect[4];\n"
	"    float uv[4];\n"
	"    float color[4];\n"
	"    uint  texId;\n"
	"    uint  pad[3];\n"
	"};\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ ssbos\n"
	"layout (binding = 1, std430) readonly buffer Data { QuadInfo data[]; };\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ outs\n"
	"\n"
	"// vertex2pixel\n"
	"out V2P\n"
	"{\n"
	"    smooth vec4 color;\n"
	"    smooth vec2 pos;\n"
	"    flat   vec2 center;\n"
	"    flat   vec2 halfSize;\n"
	"    smooth vec2 uv;\n"
	"    flat   uint texId;\n"
	"} v2p;\n"
	"\n"
	"////////////////////\n"
	"//~ uniforms\n"
	"layout (binding = 0, std140) uniform GlobalUBO\n"
	"{\n"
	"    mat4 cam;\n"
	"    mat4 proj;\n"
	"    vec2 viewport;\n"
	"    vec2 pad;\n"
	"};\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ Fwd decls\n"
	"vec4 GetRect(uint index);\n"
	"vec4 GetUV(uint index);\n"
	"vec4 GetColor(uint index);\n"
	"uint GetTexId(uint index);\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~\n"
	"void main()\n"
	"{\n"
	"    const vec2 vertices[6] = vec2[6](\n"
	"                                     vec2(-1, -1),\n"
	"                                     vec2(-1, +1),\n"
	"                                     vec2(+1, -1),\n"
	"                                     vec2(+1, +1),\n"
	"                                     \n"
	"                                     vec2(+1, +1),\n"
	"                                     vec2(-1, -1)\n"
	"                                     );\n"
	"    \n"
	"    // inputs\n"
	"    const vec4 inRect  = GetRect(gl_VertexID);\n"
	"    const vec4 inUV    = GetUV(gl_VertexID);\n"
	"    const vec4 inColor = GetColor(gl_VertexID);\n"
	"    const uint inTexId = GetTexId(gl_VertexID);\n"
	"    \n"
	"    vec2 p0 = inRect.xy;\n"
	"    vec2 p1 = inRect.zw;\n"
	"    \n"
	"    // 'dst' -> on screen\n"
	"    vec2 dst_halfSize = (p1 - p0) / 2.f;\n"
	"    vec2 dst_center   = (p1 + p0) / 2.f;\n"
	"    vec2 dst_pos = dst_center + dst_halfSize * vertices[gl_VertexID % 6];\n"
	"    \n"
	"    // v2p_uv -> tex coords\n"
	"    vec2 uvp0 = inUV.xy;\n"
	"    vec2 uvp1 = inUV.zw;\n"
	"    vec2 uv_halfSize = (uvp1 - uvp0) / 2.f;\n"
	"    vec2 uv_center   = (uvp1 + uvp0) / 2.f;\n"
	"    \n"
	"    // NOTE(gsp): 2* (x) - 1.f puts origin at top left corner)\n"
	"    gl_Position = vec4(2.f * dst_pos.x / viewport.x - 1.f,\n"
	"                       -2.f * dst_pos.y / viewport.y + 1.f,\n"
	"                       0.f, 1.f);\n"
	"    \n"
	"    // Package output\n"
	"    v2p.color    = inColor;\n"
	"    v2p.pos      = dst_pos;\n"
	"    v2p.center   = dst_center;\n"
	"    v2p.halfSize = dst_halfSize;\n"
	"    v2p.uv       = uv_center + uv_halfSize * vertices[gl_VertexID % 6];\n"
	"    v2p.texId = inTexId;\n"
	"}\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ Helpers\n"
	"vec4 GetRect(uint index)\n"
	"{\n"
	"    return vec4(data[index].rect[0],\n"
	"                data[index].rect[1],\n"
	"                data[index].rect[2],\n"
	"                data[index].rect[3]);\n"
	"}\n"
	"\n"
	"vec4 GetUV(uint index)\n"
	"{\n"
	"    return vec4(data[index].uv[0],\n"
	"                data[index].uv[1],\n"
	"                data[index].uv[2],\n"
	"                data[index].uv[3]);\n"
	"}\n"
	"\n"
	"vec4 GetColor(uint index)\n"
	"{\n"
	"    return vec4(data[index].color[0],\n"
	"                data[index].color[1],\n"
	"                data[index].color[2],\n"
	"                data[index].color[3]);\n"
	"}\n"
	"\n"
	"uint GetTexId(uint index)\n"
	"{\n"
	"    return data[index].texId;\n"
	"}\n"
	"\n"
"\0";

global const u8 *uishader_frag_glsl_shader = 	"#version 450 core\n"
	"#extension GL_ARB_bindless_texture : require\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ in\n"
	"\n"
	"// vertex2pixel\n"
	"in V2P\n"
	"{\n"
	"    smooth vec4 color;\n"
	"    smooth vec2 pos;\n"
	"    flat   vec2 center;\n"
	"    flat   vec2 halfSize;\n"
	"    smooth vec2 uv;\n"
	"    flat   uint texId;\n"
	"} v2p;\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ ssbo\n"
	"layout (binding = 0, std430) readonly buffer TextureSamplers\n"
	"{ \n"
	"    sampler2D textures[];\n"
	"};\n"
	"\n"
	"//////////////////////////////////////////////////////////\n"
	"//~ out\n"
	"out vec4 outColor;\n"
	"\n"
	"//~ @main\n"
	"void main()\n"
	"{\n"
	"    outColor  = v2p.color;\n"
	"    //outColor *= texture(textures[0], v2p.uv);\n"
	"    outColor *= texture(textures[v2p.texId], v2p.uv);\n"
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
