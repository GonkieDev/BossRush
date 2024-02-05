
//~ API
function EzmModel *
EzmModelFromFile(M_Arena *arena, String8 filepath)
{
    Assert(Str8EndsIn(filepath, S8Lit(".ezm")));
    EzmModel *model = 0;
    
    OS_Handle ezmFileHandle = OS_FileOpen(arena, filepath, OS_FileAccess_Read);
    String8 ezm = OS_FileReadEntire(arena, ezmFileHandle);
    if (ezm.size > 0)
    {
        u8 *ptr = ezm.str;
        EzmMeta *meta = (EzmMeta *)ptr;
        if (Str8StrictMatch(Str8(meta->magic, EZM_MAGIC_LEN-1), S8Lit(EZM_MAGIC)))
        {
            switch(meta->version)
            {
                case 1: model = EzmModelFromFileV1(arena, ezm); break;
                default: LogError(EZM_LOG_FILE, "Ezm file's version doesn't exist. ('%.*s')", S8VArg(filepath));
            }
        }
        else
        {
            LogError(EZM_LOG_FILE, "'Ezm' file doesn't non valid magic ('%.*s')", S8VArg(filepath));
        }
    }
    else
    {
        LogError(EZM_LOG_FILE, "Failed to read ezm file ('%.*s')", S8VArg(filepath));
    }
    OS_FileClose(ezmFileHandle);
    return model;
}

//~ Version reads
function EzmModel *
EzmModelFromFileV1(M_Arena *arena, String8 ezm)
{
    u8 *ptr = ezm.str + sizeof(EzmMeta);
    EzmModel *model = M_PushStruct(arena, EzmModel);
    
#define EZM_DerefAdvPtr(ptr, type, count) *((type*)(ptr)); (ptr) += sizeof(type)*(count)
    model->meshesCount   = EZM_DerefAdvPtr(ptr, u16, 1);
    model->verticesCount = EZM_DerefAdvPtr(ptr, u16, 1);
    model->indicesCount  = EZM_DerefAdvPtr(ptr, u16, 1);
    ptr += sizeof(u16); // pad
    
#define EZM_AssignAdvPtr(ptr, type, count) ((type*)(ptr)); (ptr) += sizeof(type)*(count)
    model->meshes   = EZM_AssignAdvPtr(ptr, EzmMesh,   model->meshesCount);
    model->vertices = EZM_AssignAdvPtr(ptr, EzmVertex, model->verticesCount);
    model->indices  = EZM_AssignAdvPtr(ptr, u16,       model->indicesCount);
    
    return model;
}


///////////////////////////////////////////////////////////////////////////
//~ WRITE API
#if defined(EZM_WRITE_IMPLEMENTATION)

//- EzmMetaMake
function EzmMeta
EzmMetaMake(void)
{
    EzmMeta meta = {0};
    for (u32 i = 0; i < EZM_MAGIC_LEN; ++i)
        meta.magic[i] = EZM_MAGIC[i];
    meta.version = ezm_version;
    return meta;
}

//-
function b32
EzmWriteModelToFile(EzmModel *model, String8 path, String8 modelName)
{
    b32 result = 0;
    if (model)
    {
        M_Arena *buffer = M_ArenaAlloc(GB(2));
        buffer->alignment = 1;
        EzmMeta meta = EzmMetaMake();
        
        M_ArenaTemp scratch = GetScratch(0, 0);
        
        u64 initialPos = buffer->pos;
        u8 *bufferPtr = ((u8*)buffer) + buffer->pos;
        
        //path = Str8PathChopPastLastSlash(path);
        String8 outFilepath = PushStr8Fmt(scratch.arena, "%.*s/%.*s.ezm", S8VArg(path), S8VArg(modelName));
        
        M_AppendStruct(buffer, EzmMeta, &meta);
        //M_AppendImmediate(buffer, u64, modelName.size);
        //M_AppendArray(buffer, u8, modelName.str, modelName.size);
        M_AppendImmediate(buffer, u16, model->meshesCount);
        M_AppendImmediate(buffer, u16, model->verticesCount);
        M_AppendImmediate(buffer, u16, model->indicesCount);
        M_AppendImmediate(buffer, u16, 0); // pad
        M_AppendArray(buffer, EzmMesh, model->meshes, model->meshesCount);
        M_AppendArray(buffer, EzmVertex, model->vertices, model->verticesCount);
        M_AppendArray(buffer, u16, model->indices, model->indicesCount);
        
        u64 bufferSize = buffer->pos - initialPos;
        OS_Handle file = OS_FileOpen(scratch.arena, outFilepath,
                                     OS_FileAccess_Write | 
                                     OS_FileAccess_CreateIfNotExists | 
                                     OS_FileAccess_Overwrite);
        u64 bytesWritten = OS_FileWrite(file, Str8(bufferPtr, bufferSize));
        if (bytesWritten == bufferSize)
        {
            LogInfo(EZM_LOG_FILE, "Wrote successfully to %.*s. (m: %d,\tv: %d,\ti: %d)", 
                    S8VArg(outFilepath), model->meshesCount, model->verticesCount, model->indicesCount);
        }
        else
        {
            LogError(EZM_LOG_FILE, "Failed to write to %.*s", S8VArg(outFilepath));
        }
        
        OS_FileClose(file);
        M_ArenaClear(buffer);
        EndScratch(scratch);
    }
    return result;
}

#endif // #if defined(EZM_WRITE_IMPLEMENTATION)