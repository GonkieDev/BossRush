#ifndef RENDER_TYPES_INCLUDE_C
#define RENDER_TYPES_INCLUDE_C


read_only HMM_Vec3 r_quad_vertex_positions[] = {
    // First triangle
    { .x =  0.5f, .y =  0.5f }, // top right
    { .x =  0.5f, .y = -0.5f }, // bottom right
    { .x = -0.5f, .y =  0.5f }, // top left
    
    // Second triangle
    { .x =  0.5f, .y = -0.5f }, // bottom right
    { .x = -0.5f, .y = -0.5f }, // bottom left
    { .x = -0.5f, .y =  0.5f }, // top left
};


#if 0
read_only HMM_Vec3 r_cube_vertex_positions[8] = {
    {-1.000000, -1.000000, 1.000000},
    {-1.000000, 1.000000, 1.000000},
    {-1.000000, -1.000000, -1.000000},
    {-1.000000, 1.000000, -1.000000},
    {1.000000, -1.000000, 1.000000},
    {1.000000, 1.000000, 1.000000},
    {1.000000, -1.000000, -1.000000},
    {1.000000, 1.000000, -1.000000},
};
// 0 1 2 and 0 2 3
read_only u32 r_cube_indices[12*3] = {
    0, 1, 3,
    0, 3, 2,
    2, 3, 7,
    2, 7, 6,
    6, 7, 5,
    6, 5, 4,
    4, 5, 1,
    4, 1, 0,
    2, 6, 4,
    2, 4, 0,
    7, 3, 1,
    7, 1, 5
};
#endif


/////////////////////////////////////////
//~ End of file
#endif // #ifndef RENDER_TYPES_INCLUDE_C