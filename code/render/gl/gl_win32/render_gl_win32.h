#ifndef RENDER_GL_WIN32_INCLUDE_H
#define RENDER_GL_WIN32_INCLUDE_H

typedef BOOL WINAPI wgl_choose_pixel_format_arb(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef HGLRC WINAPI wgl_create_context_attribs_arb(HDC hdc, HGLRC hShareContext, const int *attribList);

#define WGL_SWAP_INTERVAL_EXT_CREATOR(name) BOOL name(int interval)
typedef WGL_SWAP_INTERVAL_EXT_CREATOR(wgl_swap_interval_ext);

function b32 R_GL_W32_InitGLDummyContext(void);

function WGL_SWAP_INTERVAL_EXT_CREATOR(wglSwapIntervalEXT_Stub)
{ UnreferencedParameter(interval); return 0; }

////////////////////////////////////////
//~ END OF FILE
#endif //RENDER_GL_WIN32_INCLUDE_H
