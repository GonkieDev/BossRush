#ifndef BASE_TYPES_INCLUDE_C
#define BASE_TYPES_INCLUDE_C

inline_function b32
U32InArray_(u32 *array, u32 arraySize, u32 target, u32 goUpUntil)
{
 b32 result = 0;
 for (u32 i = 0; (i < arraySize) && (i < goUpUntil); ++i)
 {
  if (array[i] == target)
  {
   result = 1;
   break;
  }
 }
 return result;
}



///////////////////////////////////////////////
//~ End of file
#endif BASE_TYPES_INCLUDE_C // #ifndef BASE_TYPES_INCLUDE_C
