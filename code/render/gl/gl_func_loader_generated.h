#if !defined(gl_func_loader_generated_INCLUDE_H)
#define gl_func_loader_generated_INCLUDE_H





/////////////////////////////////////////////////////////
//~ Function typedefs
typedef void GLGENBUFFERS(GLsizei n, GLuint * buffers);
typedef void GLBINDBUFFER(GLenum target, 	GLuint buffer);
typedef void GLBUFFERDATA(GLenum target, 	GLsizeiptr size, 	const void * data, 	GLenum usage);
typedef GLuint GLCREATESHADER(GLenum shaderType);
typedef void GLSHADERSOURCE(GLuint shader, 	GLsizei count, 	const GLchar **string, 	const GLint *length);
typedef void GLCOMPILESHADER(GLuint shader);
typedef GLuint GLCREATEPROGRAM(void);
typedef void GLATTACHSHADER(GLuint program, GLuint shader);
typedef void GLLINKPROGRAM(GLuint program);
typedef void GLUSEPROGRAM(GLuint program);
typedef void GLDELETESHADER(GLuint shader);
typedef void GLVERTEXATTRIBPOINTER(GLuint index, 	GLint size, 	GLenum type, 	GLboolean normalized, 	GLsizei stride, 	const void * pointer);
typedef void GLENABLEVERTEXATTRIBARRAY(GLuint index);
typedef void GLGENVERTEXARRAYS(GLsizei n, 	GLuint *arrays);
typedef void GLBINDVERTEXARRAY(GLuint array);
typedef GLuint GLGETUNIFORMBLOCKINDEX(GLuint program, const GLchar *uniformBlockName);
typedef void GLUNIFORMBLOCKBINDING(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
typedef void GLBINDBUFFERRANGE(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef void GLBUFFERSUBDATA(GLenum target, GLintptr offset, GLsizeiptr size, const void * data);
typedef void GLUNIFORMMATRIX4FV(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef GLint GLGETUNIFORMLOCATION(GLuint program, const GLchar *name);
typedef void GLACTIVETEXTURE(GLenum texture);
typedef void GLTEXSTORAGE2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
typedef GLuint64 GLGETTEXTUREHANDLEARB(GLuint texture);
typedef void GLMAKETEXTUREHANDLERESIDENTARB(GLuint64 handle);
typedef void GLVERTEXATTRIBLPOINTER(GLuint index, 	GLint size, 	GLenum type, 	GLsizei stride, 	const void * pointer);
typedef void GLUNIFORMHANDLEUI64ARB(GLint location, GLuint64 value);
typedef void GLUNIFORM2UI(GLint location, GLuint v0, GLint v1);
typedef void GLCREATETEXTURES(GLenum target, GLsizei n, GLuint *textures);
typedef void GLTEXTURESTORAGE2D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
typedef void GLTEXTURESUBIMAGE2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
typedef void GLTEXTUREPARAMETERI(GLuint texture, GLenum pname, GLint param);
typedef void GLCREATEBUFFERS(GLsizei n, GLuint *buffers);
typedef void GLNAMEDBUFFERSTORAGE(GLuint buffer, GLsizeiptr size, const void *data, GLbitfield flags);
typedef void GLNAMEDBUFFERSUBDATA(GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data);
typedef void GLBINDBUFFERBASE(GLenum target, 	GLuint index, 	GLuint buffer);
typedef void GLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCE(GLenum mode, 	GLsizei count, 	GLenum type, 	void *indices, 	GLsizei instancecount, 	GLint basevertex, 	GLuint baseinstance);
typedef void GLUNIFORM1UI(GLint location, GLuint v0);
typedef void GLUNIFORM4F(GLint location, 	GLfloat v0, 	GLfloat v1, 	GLfloat v2, 	GLfloat v3);
typedef void GLDRAWARRAYSINSTANCED(GLenum mode, 	GLint first, 	GLsizei count, 	GLsizei instancecount);
#if defined(BUILD_DEBUG)
typedef void GLGETSHADERIV(GLuint shader, 	GLenum pname, 	GLint *params);
typedef void GLGETSHADERINFOLOG(GLuint shader,	GLsizei maxLength, 	GLsizei *length, 	GLchar *infoLog);
typedef void GLGETPROGRAMIV(GLuint program, GLenum pname, 	GLint *params);
typedef void GLGETPROGRAMINFOLOG(GLuint program, 	GLsizei maxLength, 	GLsizei *length, 	GLchar *infoLog);
typedef void GLDEBUGMESSAGEINSERT(GLenum source, 	GLenum type, 	GLuint id, 	GLenum severity, 	GLsizei length, 	const char *message);
typedef void GLDEBUGMESSAGECALLBACK(DEBUGPROC callback, 	const void * userParam);
typedef void GLDEBUGMESSAGECONTROL(GLenum source, 	GLenum type, 	GLenum severity, 	GLsizei count, 	const GLuint *ids, 	GLboolean enabled);
typedef void GLOBJECTLABEL(GLenum identifier, GLuint name, GLsizei length, const char *label);
typedef void GLOBJECTPTRLABEL(void *ptr, GLsizei length, const char *label);
typedef void GLGETOBJECTLABEL(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, char *label);
#endif // BUILD_DEBUG




/////////////////////////////////////////////////////////
//~ Function pointers
#define gl_func static

gl_func GLGENBUFFERS *glGenBuffers;
gl_func GLBINDBUFFER *glBindBuffer;
gl_func GLBUFFERDATA *glBufferData;
gl_func GLCREATESHADER *glCreateShader;
gl_func GLSHADERSOURCE *glShaderSource;
gl_func GLCOMPILESHADER *glCompileShader;
gl_func GLCREATEPROGRAM *glCreateProgram;
gl_func GLATTACHSHADER *glAttachShader;
gl_func GLLINKPROGRAM *glLinkProgram;
gl_func GLUSEPROGRAM *glUseProgram;
gl_func GLDELETESHADER *glDeleteShader;
gl_func GLVERTEXATTRIBPOINTER *glVertexAttribPointer;
gl_func GLENABLEVERTEXATTRIBARRAY *glEnableVertexAttribArray;
gl_func GLGENVERTEXARRAYS *glGenVertexArrays;
gl_func GLBINDVERTEXARRAY *glBindVertexArray;
gl_func GLGETUNIFORMBLOCKINDEX *glGetUniformBlockIndex;
gl_func GLUNIFORMBLOCKBINDING *glUniformBlockBinding;
gl_func GLBINDBUFFERRANGE *glBindBufferRange;
gl_func GLBUFFERSUBDATA *glBufferSubData;
gl_func GLUNIFORMMATRIX4FV *glUniformMatrix4fv;
gl_func GLGETUNIFORMLOCATION *glGetUniformLocation;
gl_func GLACTIVETEXTURE *glActiveTexture;
gl_func GLTEXSTORAGE2D *glTexStorage2D;
gl_func GLGETTEXTUREHANDLEARB *glGetTextureHandleARB;
gl_func GLMAKETEXTUREHANDLERESIDENTARB *glMakeTextureHandleResidentARB;
gl_func GLVERTEXATTRIBLPOINTER *glVertexAttribLPointer;
gl_func GLUNIFORMHANDLEUI64ARB *glUniformHandleui64ARB;
gl_func GLUNIFORM2UI *glUniform2ui;
gl_func GLCREATETEXTURES *glCreateTextures;
gl_func GLTEXTURESTORAGE2D *glTextureStorage2D;
gl_func GLTEXTURESUBIMAGE2D *glTextureSubImage2D;
gl_func GLTEXTUREPARAMETERI *glTextureParameteri;
gl_func GLCREATEBUFFERS *glCreateBuffers;
gl_func GLNAMEDBUFFERSTORAGE *glNamedBufferStorage;
gl_func GLNAMEDBUFFERSUBDATA *glNamedBufferSubData;
gl_func GLBINDBUFFERBASE *glBindBufferBase;
gl_func GLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCE *glDrawElementsInstancedBaseVertexBaseInstance;
gl_func GLUNIFORM1UI *glUniform1ui;
gl_func GLUNIFORM4F *glUniform4f;
gl_func GLDRAWARRAYSINSTANCED *glDrawArraysInstanced;
#if defined(BUILD_DEBUG)
gl_func GLGETSHADERIV *glGetShaderiv;
gl_func GLGETSHADERINFOLOG *glGetShaderInfoLog;
gl_func GLGETPROGRAMIV *glGetProgramiv;
gl_func GLGETPROGRAMINFOLOG *glGetProgramInfoLog;
gl_func GLDEBUGMESSAGEINSERT *glDebugMessageInsert;
gl_func GLDEBUGMESSAGECALLBACK *glDebugMessageCallback;
gl_func GLDEBUGMESSAGECONTROL *glDebugMessageControl;
gl_func GLOBJECTLABEL *glObjectLabel;
gl_func GLOBJECTPTRLABEL *glObjectPtrLabel;
gl_func GLGETOBJECTLABEL *glGetObjectLabel;
#endif // BUILD_DEBUG




/////////////////////////////////////////////////////////
//~ Loader Function
inline_function int
R_GL_InitFunctionPointers(void)
{
	int result = 1;
	glGenBuffers = (GLGENBUFFERS *)R_GL_GetGLFuncAddress("glGenBuffers");
result &= (0 != glGenBuffers);
	glBindBuffer = (GLBINDBUFFER *)R_GL_GetGLFuncAddress("glBindBuffer");
result &= (0 != glBindBuffer);
	glBufferData = (GLBUFFERDATA *)R_GL_GetGLFuncAddress("glBufferData");
result &= (0 != glBufferData);
	glCreateShader = (GLCREATESHADER *)R_GL_GetGLFuncAddress("glCreateShader");
result &= (0 != glCreateShader);
	glShaderSource = (GLSHADERSOURCE *)R_GL_GetGLFuncAddress("glShaderSource");
result &= (0 != glShaderSource);
	glCompileShader = (GLCOMPILESHADER *)R_GL_GetGLFuncAddress("glCompileShader");
result &= (0 != glCompileShader);
	glCreateProgram = (GLCREATEPROGRAM *)R_GL_GetGLFuncAddress("glCreateProgram");
result &= (0 != glCreateProgram);
	glAttachShader = (GLATTACHSHADER *)R_GL_GetGLFuncAddress("glAttachShader");
result &= (0 != glAttachShader);
	glLinkProgram = (GLLINKPROGRAM *)R_GL_GetGLFuncAddress("glLinkProgram");
result &= (0 != glLinkProgram);
	glUseProgram = (GLUSEPROGRAM *)R_GL_GetGLFuncAddress("glUseProgram");
result &= (0 != glUseProgram);
	glDeleteShader = (GLDELETESHADER *)R_GL_GetGLFuncAddress("glDeleteShader");
result &= (0 != glDeleteShader);
	glVertexAttribPointer = (GLVERTEXATTRIBPOINTER *)R_GL_GetGLFuncAddress("glVertexAttribPointer");
result &= (0 != glVertexAttribPointer);
	glEnableVertexAttribArray = (GLENABLEVERTEXATTRIBARRAY *)R_GL_GetGLFuncAddress("glEnableVertexAttribArray");
result &= (0 != glEnableVertexAttribArray);
	glGenVertexArrays = (GLGENVERTEXARRAYS *)R_GL_GetGLFuncAddress("glGenVertexArrays");
result &= (0 != glGenVertexArrays);
	glBindVertexArray = (GLBINDVERTEXARRAY *)R_GL_GetGLFuncAddress("glBindVertexArray");
result &= (0 != glBindVertexArray);
	glGetUniformBlockIndex = (GLGETUNIFORMBLOCKINDEX *)R_GL_GetGLFuncAddress("glGetUniformBlockIndex");
result &= (0 != glGetUniformBlockIndex);
	glUniformBlockBinding = (GLUNIFORMBLOCKBINDING *)R_GL_GetGLFuncAddress("glUniformBlockBinding");
result &= (0 != glUniformBlockBinding);
	glBindBufferRange = (GLBINDBUFFERRANGE *)R_GL_GetGLFuncAddress("glBindBufferRange");
result &= (0 != glBindBufferRange);
	glBufferSubData = (GLBUFFERSUBDATA *)R_GL_GetGLFuncAddress("glBufferSubData");
result &= (0 != glBufferSubData);
	glUniformMatrix4fv = (GLUNIFORMMATRIX4FV *)R_GL_GetGLFuncAddress("glUniformMatrix4fv");
result &= (0 != glUniformMatrix4fv);
	glGetUniformLocation = (GLGETUNIFORMLOCATION *)R_GL_GetGLFuncAddress("glGetUniformLocation");
result &= (0 != glGetUniformLocation);
	glActiveTexture = (GLACTIVETEXTURE *)R_GL_GetGLFuncAddress("glActiveTexture");
result &= (0 != glActiveTexture);
	glTexStorage2D = (GLTEXSTORAGE2D *)R_GL_GetGLFuncAddress("glTexStorage2D");
result &= (0 != glTexStorage2D);
	glGetTextureHandleARB = (GLGETTEXTUREHANDLEARB *)R_GL_GetGLFuncAddress("glGetTextureHandleARB");
result &= (0 != glGetTextureHandleARB);
	glMakeTextureHandleResidentARB = (GLMAKETEXTUREHANDLERESIDENTARB *)R_GL_GetGLFuncAddress("glMakeTextureHandleResidentARB");
result &= (0 != glMakeTextureHandleResidentARB);
	glVertexAttribLPointer = (GLVERTEXATTRIBLPOINTER *)R_GL_GetGLFuncAddress("glVertexAttribLPointer");
result &= (0 != glVertexAttribLPointer);
	glUniformHandleui64ARB = (GLUNIFORMHANDLEUI64ARB *)R_GL_GetGLFuncAddress("glUniformHandleui64ARB");
result &= (0 != glUniformHandleui64ARB);
	glUniform2ui = (GLUNIFORM2UI *)R_GL_GetGLFuncAddress("glUniform2ui");
result &= (0 != glUniform2ui);
	glCreateTextures = (GLCREATETEXTURES *)R_GL_GetGLFuncAddress("glCreateTextures");
result &= (0 != glCreateTextures);
	glTextureStorage2D = (GLTEXTURESTORAGE2D *)R_GL_GetGLFuncAddress("glTextureStorage2D");
result &= (0 != glTextureStorage2D);
	glTextureSubImage2D = (GLTEXTURESUBIMAGE2D *)R_GL_GetGLFuncAddress("glTextureSubImage2D");
result &= (0 != glTextureSubImage2D);
	glTextureParameteri = (GLTEXTUREPARAMETERI *)R_GL_GetGLFuncAddress("glTextureParameteri");
result &= (0 != glTextureParameteri);
	glCreateBuffers = (GLCREATEBUFFERS *)R_GL_GetGLFuncAddress("glCreateBuffers");
result &= (0 != glCreateBuffers);
	glNamedBufferStorage = (GLNAMEDBUFFERSTORAGE *)R_GL_GetGLFuncAddress("glNamedBufferStorage");
result &= (0 != glNamedBufferStorage);
	glNamedBufferSubData = (GLNAMEDBUFFERSUBDATA *)R_GL_GetGLFuncAddress("glNamedBufferSubData");
result &= (0 != glNamedBufferSubData);
	glBindBufferBase = (GLBINDBUFFERBASE *)R_GL_GetGLFuncAddress("glBindBufferBase");
result &= (0 != glBindBufferBase);
	glDrawElementsInstancedBaseVertexBaseInstance = (GLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCE *)R_GL_GetGLFuncAddress("glDrawElementsInstancedBaseVertexBaseInstance");
result &= (0 != glDrawElementsInstancedBaseVertexBaseInstance);
	glUniform1ui = (GLUNIFORM1UI *)R_GL_GetGLFuncAddress("glUniform1ui");
result &= (0 != glUniform1ui);
	glUniform4f = (GLUNIFORM4F *)R_GL_GetGLFuncAddress("glUniform4f");
result &= (0 != glUniform4f);
	glDrawArraysInstanced = (GLDRAWARRAYSINSTANCED *)R_GL_GetGLFuncAddress("glDrawArraysInstanced");
result &= (0 != glDrawArraysInstanced);
#if defined(BUILD_DEBUG)
	glGetShaderiv = (GLGETSHADERIV *)R_GL_GetGLFuncAddress("glGetShaderiv");
result &= (0 != glGetShaderiv);
	glGetShaderInfoLog = (GLGETSHADERINFOLOG *)R_GL_GetGLFuncAddress("glGetShaderInfoLog");
result &= (0 != glGetShaderInfoLog);
	glGetProgramiv = (GLGETPROGRAMIV *)R_GL_GetGLFuncAddress("glGetProgramiv");
result &= (0 != glGetProgramiv);
	glGetProgramInfoLog = (GLGETPROGRAMINFOLOG *)R_GL_GetGLFuncAddress("glGetProgramInfoLog");
result &= (0 != glGetProgramInfoLog);
	glDebugMessageInsert = (GLDEBUGMESSAGEINSERT *)R_GL_GetGLFuncAddress("glDebugMessageInsert");
result &= (0 != glDebugMessageInsert);
	glDebugMessageCallback = (GLDEBUGMESSAGECALLBACK *)R_GL_GetGLFuncAddress("glDebugMessageCallback");
result &= (0 != glDebugMessageCallback);
	glDebugMessageControl = (GLDEBUGMESSAGECONTROL *)R_GL_GetGLFuncAddress("glDebugMessageControl");
result &= (0 != glDebugMessageControl);
	glObjectLabel = (GLOBJECTLABEL *)R_GL_GetGLFuncAddress("glObjectLabel");
result &= (0 != glObjectLabel);
	glObjectPtrLabel = (GLOBJECTPTRLABEL *)R_GL_GetGLFuncAddress("glObjectPtrLabel");
result &= (0 != glObjectPtrLabel);
	glGetObjectLabel = (GLGETOBJECTLABEL *)R_GL_GetGLFuncAddress("glGetObjectLabel");
result &= (0 != glGetObjectLabel);
#endif // BUILD_DEBUG
return result;
}




/////////////////////////////////////////////////////////
//~ End of generated file
#endif // gl_func_loader_generated_INCLUDE_H
