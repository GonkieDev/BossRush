#ifndef EZM_INCLUDE
#define EZM_INCLUDE

#if !defined(EZM_LOG_FILE)
# define EZM_LOG_FILE "ezm_log.txt"
#endif

//- Basic EZM Types
typedef struct EzmVertex EzmVertex;
struct EzmVertex
{
    HMM_Vec3 p;  // position
    HMM_Vec3 n;  // normal
    HMM_Vec2 uv; // uv coordinates
};

typedef struct EzmMesh EzmMesh;
struct EzmMesh
{
    // NOTE: use XOffset to index 'EzmModel.X'
    u32 verticesOffset;
    u32 verticesCount;
    u32 indicesOffset;
    u32 indicesCount;  // 0 if this mesh contains no indices
};

typedef struct EzmModel EzmModel; 
struct EzmModel
{
    // NOTE: indices count and indices will be 0 if model contains no indices
    EzmMesh   *meshes;
    EzmVertex *vertices;
    u16       *indices;
    
    u16 meshesCount;
    u16 verticesCount;
    u16 indicesCount;
    PAD16(1);
};

//- File structure types
#define EZM_MAGIC_LEN 8 // 8th character is null char '0'
#define EZM_MAGIC "EZMESH0"
StaticAssert(sizeof(EZM_MAGIC) == EZM_MAGIC_LEN, "EZM_MAGIC string must be size of EZM_MAGIC_LEN");

read_only global u32 ezm_version = 1;

typedef struct EzmMeta EzmMeta; 
struct EzmMeta
{
    u8 magic[EZM_MAGIC_LEN];
    u32 version;
    u32 ___reserved[3];
};

//~ READ API
function EzmModel *EzmModelFromFile(M_Arena *arena, String8 filepath);

//~ Version reads
function EzmModel *EzmModelFromFileV1(M_Arena *arena, String8 ezmFile);

//~
#if defined(EZM_WRITE_IMPLEMENTATION)

function EzmMeta EzmMetaMake(void);
function b32     EzmWriteModelToFile(EzmModel *model, String8 path, String8 modelName);

#endif // #if defined(EZM_WRITE_IMPLEMENTATION)

///////////////////////////////////////////////////////////////////////////
//~ END OF FILE
#endif //EZM_INCLUDE
