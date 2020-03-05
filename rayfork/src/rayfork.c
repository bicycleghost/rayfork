#include "rayfork.h"
#include "math.h"

//region internal
RF_INTERNAL void* rf_internal_realloc_wrapper(rf_allocator allocator, void* source, int old_size, int new_size)
{
    void* new_alloc = RF_ALLOC(allocator, new_size);
    if (new_alloc && source && old_size) { memcpy(new_alloc, source, old_size); }
    if (source) { RF_FREE(allocator, source); }
    return new_alloc;
}

RF_INTERNAL void* rf_internal_calloc_wrapper(rf_allocator allocator, int amount, int size)
{
    void* ptr = RF_ALLOC(allocator, amount * size);
    memset(ptr, 0, amount * size);
    return ptr;
}

#define RF_INTERNAL_STRINGS_MATCH(a, a_len, b, b_len) (a_len == b_len && (strncmp(a, b, a_len) == 0))

RF_INTERNAL bool rf_internal_is_file_extension(const char* filename, const char* ext)
{
    int filename_len = strlen(filename);
    int ext_len      = strlen(ext);

    if (filename_len < ext_len)
    {
        return false;
    }

    return RF_INTERNAL_STRINGS_MATCH(filename + filename_len - ext_len, ext_len, ext, ext_len);
}

// String pointer reverse break: returns right-most occurrence of charset in s
RF_INTERNAL const char* rf_internal_strprbrk(const char* s, const char* charset)
{
    const char* latestMatch = NULL;
    for (; s = strpbrk(s, charset), s != NULL; latestMatch = s++) { }
    return latestMatch;
}

#ifndef RF_MAX_FILEPATH_LEN
    #define RF_MAX_FILEPATH_LEN 1024
#endif

RF_INTERNAL RF_THREAD_LOCAL char rf_internal_dir_path[RF_MAX_FILEPATH_LEN];

// Get directory for a given filePath
RF_INTERNAL const char* rf_internal_get_directory_path(const char* filePath)
{
    const char* last_slash = NULL;
    memset(rf_internal_dir_path, 0, RF_MAX_FILEPATH_LEN);

    last_slash = rf_internal_strprbrk(filePath, "\\/");
    if (!last_slash) { return NULL; }

    // NOTE: Be careful, strncpy() is not safe, it does not care about '\0'
    strncpy(rf_internal_dir_path, filePath, strlen(filePath) - (strlen(last_slash) - 1));
    rf_internal_dir_path[strlen(filePath) - strlen(last_slash)] = '\0'; // Add '\0' manually

    return rf_internal_dir_path;
}

#define RF_MIN(a, b) ((a) < (b) ? (a) : (b))
#define RF_MAX(a, b) ((a) > (b) ? (a) : (b))

RF_INTERNAL const unsigned char rf_internal_base64_table[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 62, 0, 0, 0, 63, 52, 53,
    54, 55, 56, 57, 58, 59, 60, 61, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 2, 3, 4,
    5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 0, 0, 0, 0, 0, 0, 26, 27, 28,
    29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51
};
//endregion

//region stb_image

//Global thread-local alloctor for stb image. Everytime we call a function from stbi we set the allocator and then set it to null afterwards.
RF_INTERNAL RF_THREAD_LOCAL rf_allocator* rf_internal_stbi_allocator;

#define RF_SET_STBI_ALLOCATOR(allocator) rf_internal_stbi_allocator = (allocator)

//#define STBI_NO_GIF
#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC(sz)                     RF_ALLOC(*rf_internal_stbi_allocator, sz)
#define STBI_FREE(p)                        RF_FREE(*rf_internal_stbi_allocator, p)
#define STBI_REALLOC_SIZED(p, oldsz, newsz) rf_internal_realloc_wrapper(*rf_internal_stbi_allocator, p, oldsz, newsz)
#define STBI_ASSERT(it)                     RF_ASSERT(it)
#define STBIDEF                             static
#include "stb/stb_image.h"
//endregion

//region stb_image_resize

//Global thread-local alloctor for stb image. Everytime we call a function from stbi we set the allocator and then set it to null afterwards.
RF_INTERNAL RF_THREAD_LOCAL rf_allocator* rf_internal_stbir_allocator;

#define RF_SET_STBIR_ALLOCATOR(allocator) rf_internal_stbir_allocator = (allocator)

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STBIR_MALLOC(sz,c) ((void)(c), RF_ALLOC(*rf_internal_stbir_allocator, sz))
#define STBIR_FREE(p,c)      ((void)(c), RF_FREE(*rf_internal_stbir_allocator, p))
#define STBIR_ASSERT(it)     RF_ASSERT(it)
#define STBIRDEF             RF_INTERNAL
#include "stb/stb_image_resize.h"
//endregion

//region stb_rect_pack
#define STB_RECT_PACK_IMPLEMENTATION
#define STBRP_STATIC
#define STBRP_ASSERT RF_ASSERT
#include "stb/stb_rect_pack.h"
//endregion

//region stb_truetype
//Global thread-local alloctor for stb image. Everytime we call a function from stbi we set the allocator and then set it to null afterwards.
RF_INTERNAL RF_THREAD_LOCAL rf_allocator* rf_internal_stbtt_allocator;

#define RF_SET_STBTT_ALLOCATOR(allocator) rf_internal_stbtt_allocator = (allocator)

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#define STBTT_malloc(sz, u) RF_ALLOC(*rf_internal_stbtt_allocator, sz)
#define STBTT_free(p, u)    RF_FREE(*rf_internal_stbtt_allocator, p)
#define STBTT_assert(it)    RF_ASSERT(it)
#include "stb/stb_truetype.h"
//endregion

//region stb_perlin
#define STB_PERLIN_IMPLEMENTATION
#include "stb/stb_perlin.h"
//endregion

//region par shapes
//Global thread-local alloctor for stb image. Everytime we call a function from stbi we set the allocator and then set it to null afterwards.
RF_INTERNAL RF_THREAD_LOCAL rf_allocator* rf_internal_par_allocator;

#define RF_SET_PARSHAPES_ALLOCATOR(allocator) rf_internal_par_allocator = (allocator)

#define PAR_SHAPES_IMPLEMENTATION
#define PAR_MALLOC(T, N)                    ((T*)RF_ALLOC(*rf_internal_par_allocator, N * sizeof(T)))
#define PAR_CALLOC(T, N)                    ((T*)rf_internal_calloc_wrapper(*rf_internal_par_allocator, N, sizeof(T)))
#define PAR_FREE(BUF)                       RF_FREE(*rf_internal_par_allocator, BUF)
#define PAR_REALLOC(T, BUF, N, OLD_SZ)      ((T*) rf_internal_realloc_wrapper(*rf_internal_par_allocator, BUF, sizeof(T) * (N), OLD_SZ))

#include "par/par_shapes.h"
//endregion

//region tinyobj loader
//Global thread-local alloctor for stb image. Everytime we call a function from stbi we set the allocator and then set it to null afterwards.
RF_INTERNAL RF_THREAD_LOCAL rf_allocator* rf_internal_tinyobj_allocator;
RF_INTERNAL RF_THREAD_LOCAL rf_io_callbacks* rf_tinyobj_io;

#define RF_SET_TINYOBJ_ALLOCATOR(allocator) rf_internal_tinyobj_allocator = allocator
#define RF_SET_TINYOBJ_IO_CALLBACKS(io) rf_tinyobj_io = io;

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#define TINYOBJ_MALLOC(size)             RF_ALLOC(*rf_internal_tinyobj_allocator, size)
#define TINYOBJ_REALLOC(p, oldsz, newsz) rf_internal_realloc_wrapper(*rf_internal_tinyobj_allocator, p, oldsz, newsz)
#define TINYOBJ_CALLOC(amount, size)     rf_internal_calloc_wrapper(*rf_internal_tinyobj_allocator, amount, size)
#define TINYOBJ_FREE(p)                  RF_FREE(*rf_internal_tinyobj_allocator, p)

#define TINYOBJ_GET_FILE_SIZE(filename) (rf_tinyobj_io->get_file_size_proc(filename))
#define TINYOBJ_LOAD_FILE_IN_BUFFER(filename, buffer, buffer_size) (rf_tinyobj_io->read_file_into_buffer_proc(filename, buffer, buffer_size))

#include "tinyobjloader-c/tinyobj_loader_c.h"
//endregion

//region cgltf
RF_INTERNAL RF_THREAD_LOCAL rf_allocator* rf_internal_cgltf_allocator;

#define RF_SET_CGLTF_ALLOCATOR(allocator) rf_internal_cgltf_allocator = allocator

#define CGLTF_IMPLEMENTATION
#define CGLTF_MALLOC(size) RF_ALLOC(*rf_internal_cgltf_allocator, size)
#define CGLTF_FREE(ptr)    RF_FREE(*rf_internal_cgltf_allocator, ptr)

#include "cgltf/cgltf.h"

cgltf_result rf_internal_cgltf_io_read(const struct cgltf_memory_options* memory_options, const struct cgltf_file_options* file_options, const char* path, cgltf_size* size, void** data)
{
    ((void) memory_options);
    ((void) file_options);

    rf_io_callbacks* io = (rf_io_callbacks*) file_options->user_data;

    int file_size = io->get_file_size_proc(path);

    if (file_size == 0)
    {
        return cgltf_result_file_not_found;
    }

    *data = CGLTF_MALLOC(file_size);

    if (data == NULL)
    {
        return cgltf_result_out_of_memory;
    }

    if (!io->read_file_into_buffer_proc(path, *data, file_size))
    {
        CGLTF_FREE(*data);
        return cgltf_result_io_error;
    }

    return cgltf_result_success;
}

void rf_internal_cgltf_io_release(const struct cgltf_memory_options* memory_options, const struct cgltf_file_options* file_options, void* data)
{
    ((void) memory_options);
    ((void) file_options);

    CGLTF_FREE(data);
}
//endregion

//Global pointer to context struct
rf_context* rf_internal_ctx;

//region init and setup
// Set viewport for a provided width and height
RF_API void rf_setup_viewport(int width, int height)
{
    rf_internal_ctx->render_width  = width;
    rf_internal_ctx->render_height = height;

    // Set viewport width and height
    // NOTE: We consider render size and offset in case black bars are required and
    // render area does not match full global_display area (this situation is only applicable on fullscreen mode)
    rf_gfx_viewport(rf_internal_ctx->render_offset_x/2, rf_internal_ctx->render_offset_y/2, rf_internal_ctx->render_width - rf_internal_ctx->render_offset_x, rf_internal_ctx->render_height - rf_internal_ctx->render_offset_y);

    rf_gfx_matrix_mode(RF_PROJECTION); // Switch to PROJECTION matrix
    rf_gfx_load_identity(); // Reset current matrix (PROJECTION)

    // Set orthographic GL_PROJECTION to current framebuffer size
    // NOTE: Confirf_gfx_projectiongured top-left corner as (0, 0)
    rf_gfx_ortho(0, rf_internal_ctx->render_width, rf_internal_ctx->render_height, 0, 0.0f, 1.0f);

    rf_gfx_matrix_mode(RF_MODELVIEW); // Switch back to MODELVIEW matrix
    rf_gfx_load_identity(); // Reset current matrix (MODELVIEW)
}

// Define default texture used to draw shapes
RF_API void rf_set_shapes_texture(rf_texture2d texture, rf_rec source)
{
    rf_internal_ctx->tex_shapes = texture;
    rf_internal_ctx->rec_tex_shapes = source;
}

// Load the raylib default font
RF_API void rf_load_default_font(rf_allocator allocator, rf_allocator temp_allocator)
{
    // NOTE: Using UTF8 encoding table for Unicode U+0000..U+00FF Basic Latin + Latin-1 Supplement
    // http://www.utf8-chartable.de/unicode-utf8-table.pl

    rf_internal_ctx->default_font.chars_count = 224; // Number of chars included in our default font

    // Default font is directly defined here (data generated from a sprite font image)
    // This way, we reconstruct rf_font without creating large global variables
    // This data is automatically allocated to Stack and automatically deallocated at the end of this function
    unsigned int default_font_data[512] = {
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00200020, 0x0001b000, 0x00000000, 0x00000000, 0x8ef92520, 0x00020a00, 0x7dbe8000, 0x1f7df45f,
        0x4a2bf2a0, 0x0852091e, 0x41224000, 0x10041450, 0x2e292020, 0x08220812, 0x41222000, 0x10041450, 0x10f92020, 0x3efa084c, 0x7d22103c, 0x107df7de,
        0xe8a12020, 0x08220832, 0x05220800, 0x10450410, 0xa4a3f000, 0x08520832, 0x05220400, 0x10450410, 0xe2f92020, 0x0002085e, 0x7d3e0281, 0x107df41f,
        0x00200000, 0x8001b000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc0000fbe, 0xfbf7e00f, 0x5fbf7e7d, 0x0050bee8, 0x440808a2, 0x0a142fe8, 0x50810285, 0x0050a048,
        0x49e428a2, 0x0a142828, 0x40810284, 0x0048a048, 0x10020fbe, 0x09f7ebaf, 0xd89f3e84, 0x0047a04f, 0x09e48822, 0x0a142aa1, 0x50810284, 0x0048a048,
        0x04082822, 0x0a142fa0, 0x50810285, 0x0050a248, 0x00008fbe, 0xfbf42021, 0x5f817e7d, 0x07d09ce8, 0x00008000, 0x00000fe0, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x000c0180,
        0xdfbf4282, 0x0bfbf7ef, 0x42850505, 0x004804bf, 0x50a142c6, 0x08401428, 0x42852505, 0x00a808a0, 0x50a146aa, 0x08401428, 0x42852505, 0x00081090,
        0x5fa14a92, 0x0843f7e8, 0x7e792505, 0x00082088, 0x40a15282, 0x08420128, 0x40852489, 0x00084084, 0x40a16282, 0x0842022a, 0x40852451, 0x00088082,
        0xc0bf4282, 0xf843f42f, 0x7e85fc21, 0x3e0900bf, 0x00000000, 0x00000004, 0x00000000, 0x000c0180, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x04000402, 0x41482000, 0x00000000, 0x00000800,
        0x04000404, 0x4100203c, 0x00000000, 0x00000800, 0xf7df7df0, 0x514bef85, 0xbefbefbe, 0x04513bef, 0x14414500, 0x494a2885, 0xa28a28aa, 0x04510820,
        0xf44145f0, 0x474a289d, 0xa28a28aa, 0x04510be0, 0x14414510, 0x494a2884, 0xa28a28aa, 0x02910a00, 0xf7df7df0, 0xd14a2f85, 0xbefbe8aa, 0x011f7be0,
        0x00000000, 0x00400804, 0x20080000, 0x00000000, 0x00000000, 0x00600f84, 0x20080000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0xac000000, 0x00000f01, 0x00000000, 0x00000000, 0x24000000, 0x00000f01, 0x00000000, 0x06000000, 0x24000000, 0x00000f01, 0x00000000, 0x09108000,
        0x24fa28a2, 0x00000f01, 0x00000000, 0x013e0000, 0x2242252a, 0x00000f52, 0x00000000, 0x038a8000, 0x2422222a, 0x00000f29, 0x00000000, 0x010a8000,
        0x2412252a, 0x00000f01, 0x00000000, 0x010a8000, 0x24fbe8be, 0x00000f01, 0x00000000, 0x0ebe8000, 0xac020000, 0x00000f01, 0x00000000, 0x00048000,
        0x0003e000, 0x00000f00, 0x00000000, 0x00008000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000038, 0x8443b80e, 0x00203a03,
        0x02bea080, 0xf0000020, 0xc452208a, 0x04202b02, 0xf8029122, 0x07f0003b, 0xe44b388e, 0x02203a02, 0x081e8a1c, 0x0411e92a, 0xf4420be0, 0x01248202,
        0xe8140414, 0x05d104ba, 0xe7c3b880, 0x00893a0a, 0x283c0e1c, 0x04500902, 0xc4400080, 0x00448002, 0xe8208422, 0x04500002, 0x80400000, 0x05200002,
        0x083e8e00, 0x04100002, 0x804003e0, 0x07000042, 0xf8008400, 0x07f00003, 0x80400000, 0x04000022, 0x00000000, 0x00000000, 0x80400000, 0x04000002,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00800702, 0x1848a0c2, 0x84010000, 0x02920921, 0x01042642, 0x00005121, 0x42023f7f, 0x00291002,
        0xefc01422, 0x7efdfbf7, 0xefdfa109, 0x03bbbbf7, 0x28440f12, 0x42850a14, 0x20408109, 0x01111010, 0x28440408, 0x42850a14, 0x2040817f, 0x01111010,
        0xefc78204, 0x7efdfbf7, 0xe7cf8109, 0x011111f3, 0x2850a932, 0x42850a14, 0x2040a109, 0x01111010, 0x2850b840, 0x42850a14, 0xefdfbf79, 0x03bbbbf7,
        0x001fa020, 0x00000000, 0x00001000, 0x00000000, 0x00002070, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x08022800, 0x00012283, 0x02430802, 0x01010001, 0x8404147c, 0x20000144, 0x80048404, 0x00823f08, 0xdfbf4284, 0x7e03f7ef, 0x142850a1, 0x0000210a,
        0x50a14684, 0x528a1428, 0x142850a1, 0x03efa17a, 0x50a14a9e, 0x52521428, 0x142850a1, 0x02081f4a, 0x50a15284, 0x4a221428, 0xf42850a1, 0x03efa14b,
        0x50a16284, 0x4a521428, 0x042850a1, 0x0228a17a, 0xdfbf427c, 0x7e8bf7ef, 0xf7efdfbf, 0x03efbd0b, 0x00000000, 0x04000000, 0x00000000, 0x00000008,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00200508, 0x00840400, 0x11458122, 0x00014210,
        0x00514294, 0x51420800, 0x20a22a94, 0x0050a508, 0x00200000, 0x00000000, 0x00050000, 0x08000000, 0xfefbefbe, 0xfbefbefb, 0xfbeb9114, 0x00fbefbe,
        0x20820820, 0x8a28a20a, 0x8a289114, 0x3e8a28a2, 0xfefbefbe, 0xfbefbe0b, 0x8a289114, 0x008a28a2, 0x228a28a2, 0x08208208, 0x8a289114, 0x088a28a2,
        0xfefbefbe, 0xfbefbefb, 0xfa2f9114, 0x00fbefbe, 0x00000000, 0x00000040, 0x00000000, 0x00000000, 0x00000000, 0x00000020, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00210100, 0x00000004, 0x00000000, 0x00000000, 0x14508200, 0x00001402, 0x00000000, 0x00000000,
        0x00000010, 0x00000020, 0x00000000, 0x00000000, 0xa28a28be, 0x00002228, 0x00000000, 0x00000000, 0xa28a28aa, 0x000022e8, 0x00000000, 0x00000000,
        0xa28a28aa, 0x000022a8, 0x00000000, 0x00000000, 0xa28a28aa, 0x000022e8, 0x00000000, 0x00000000, 0xbefbefbe, 0x00003e2f, 0x00000000, 0x00000000,
        0x00000004, 0x00002028, 0x00000000, 0x00000000, 0x80000000, 0x00003e0f, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
    };

    int chars_height  = 10;
    int chars_divisor = 1; // Every char is separated from the consecutive by a 1 pixel divisor, horizontally and vertically

    int chars_width[224] = {
        3, 1, 4, 6, 5, 7, 6, 2, 3, 3, 5, 5, 2, 4, 1, 7, 5, 2, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1, 3, 4, 3, 6,
        7, 6, 6, 6, 6, 6, 6, 6, 6, 3, 5, 6, 5, 7, 6, 6, 6, 6, 6, 6, 7, 6, 7, 7, 6, 6, 6, 2, 7, 2, 3, 5,
        2, 5, 5, 5, 5, 5, 4, 5, 5, 1, 2, 5, 2, 5, 5, 5, 5, 5, 5, 5, 4, 5, 5, 5, 5, 5, 5, 3, 1, 3, 4, 4,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 5, 5, 5, 7, 1, 5, 3, 7, 3, 5, 4, 1, 7, 4, 3, 5, 3, 3, 2, 5, 6, 1, 2, 2, 3, 5, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 7, 6, 6, 6, 6, 6, 3, 3, 3, 3, 7, 6, 6, 6, 6, 6, 6, 5, 6, 6, 6, 6, 6, 6, 4, 6,
        5, 5, 5, 5, 5, 5, 9, 5, 5, 5, 5, 5, 2, 2, 3, 3, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 3, 5
    };

    // Re-construct image from rf_internal_ctx->default_font_data and generate OpenGL texture
    //----------------------------------------------------------------------
    const int im_width = 128;
    const int im_height = 128;

    rf_color image_pixels[128 * 128] = { 0 };

    int counter = 0; // rf_font data elements counter

    // Fill img_data with rf_internal_ctx->default_font_data (convert from bit to pixel!)
    for (int i = 0; i < im_width * im_height; i += 32)
    {
        for (int j = 31; j >= 0; j--)
        {
            const int bit_check = (default_font_data[counter]) & (1u << j);
            if (bit_check) image_pixels[i + j] = RF_WHITE;
        }

        counter++;

        if (counter > 512) counter = 0; // Security check...
    }

    rf_image im_font = rf_load_image_from_pixels(image_pixels, im_width, im_height, allocator);
    rf_image_format(&im_font, RF_UNCOMPRESSED_GRAY_ALPHA, temp_allocator);

    rf_internal_ctx->default_font.texture = rf_load_texture_from_image(im_font);

    // Reconstruct charSet using chars_width[], chars_height, chars_divisor, chars_count
    //------------------------------------------------------------------------------

    // Allocate space for our characters info data
    // NOTE: This memory should be freed at end! --> CloseWindow()
    rf_internal_ctx->default_font.chars = rf_internal_ctx->gfx_ctx.memory->default_font_chars;
    rf_internal_ctx->default_font.recs  = rf_internal_ctx->gfx_ctx.memory->default_font_recs;

    int current_line  = 0;
    int current_pos_x = chars_divisor;
    int test_pos_x    = chars_divisor;

    for (int i = 0; i < rf_internal_ctx->default_font.chars_count; i++)
    {
        rf_internal_ctx->default_font.chars[i].value = 32 + i; // First char is 32

        rf_internal_ctx->default_font.recs[i].x      = (float) current_pos_x;
        rf_internal_ctx->default_font.recs[i].y      = (float) (chars_divisor + current_line * (chars_height + chars_divisor));
        rf_internal_ctx->default_font.recs[i].width  = (float) chars_width[i];
        rf_internal_ctx->default_font.recs[i].height = (float) chars_height;

        test_pos_x += (int) (rf_internal_ctx->default_font.recs[i].width + (float)chars_divisor);

        if (test_pos_x >= rf_internal_ctx->default_font.texture.width)
        {
            current_line++;
            current_pos_x = 2 * chars_divisor + chars_width[i];
            test_pos_x = current_pos_x;

            rf_internal_ctx->default_font.recs[i].x = (float) (chars_divisor);
            rf_internal_ctx->default_font.recs[i].y = (float) (chars_divisor + current_line * (chars_height + chars_divisor));
        }
        else current_pos_x = test_pos_x;

        // NOTE: On default font character offsets and xAdvance are not required
        rf_internal_ctx->default_font.chars[i].offset_x = 0;
        rf_internal_ctx->default_font.chars[i].offset_y = 0;
        rf_internal_ctx->default_font.chars[i].advance_x = 0;

        // Fill character image data from fontClear data
        rf_internal_ctx->default_font.chars[i].image = rf_image_from_image(im_font, rf_internal_ctx->default_font.recs[i], allocator, temp_allocator);
    }

    rf_internal_ctx->default_font.base_size = (int)rf_internal_ctx->default_font.recs[0].height;

    RF_LOG_V(RF_LOG_INFO, "[TEX ID %i] Default font loaded successfully", rf_internal_ctx->default_font.texture.id);
}

// Load default material (Supports: DIFFUSE, SPECULAR, NORMAL maps)
RF_API rf_material rf_load_default_material(rf_allocator allocator)
{
    rf_material material = { 0 };
    material.allocator = allocator;
    material.maps = (rf_material_map*) RF_ALLOC(allocator, RF_MAX_MATERIAL_MAPS * sizeof(rf_material_map));
    memset(material.maps, 0, RF_MAX_MATERIAL_MAPS * sizeof(rf_material_map));

    material.shader = rf_get_default_shader();
    material.maps[RF_MAP_DIFFUSE].texture = rf_get_default_texture(); // White texture (1x1 pixel)
    //material.maps[RF_MAP_NORMAL].texture;         // NOTE: By default, not set
    //material.maps[RF_MAP_SPECULAR].texture;       // NOTE: By default, not set

    material.maps[RF_MAP_DIFFUSE].color = RF_WHITE; // Diffuse color
    material.maps[RF_MAP_SPECULAR].color = RF_WHITE; // Specular color

    return material;
}
//endregion

//region time functions

//Used to set custom functions
RF_API void rf_set_time_functions(void (*wait_proc)(float), double (*get_time_proc)(void))
{
    rf_internal_ctx->wait_proc = wait_proc;
    rf_internal_ctx->get_time_proc = get_time_proc;
}

// If the user disabled the default time functions implementation or we are compiling for a platform that does not have a default implementation for the time functions
#if defined(RF_NO_DEFAULT_TIME) || (!defined(_WIN32) && !defined(__linux__) && !defined(__MACH__))
    // Wait for some milliseconds (pauses program execution)
    RF_API void rf_wait(float it)
    {
        if (rf_internal_ctx->wait_proc)
        {
            rf_internal_ctx->wait_proc(it);
        }
    }

    // Returns elapsed time in seconds since rf_context_init
    RF_API double rf_get_time(void)
    {
        if (rf_internal_ctx->get_time_proc)
        {
            return rf_internal_ctx->get_time_proc();
        }

        return 0;
    }

#else //#if !defined(RF_NO_DEFAULT_TIME)
    //Windows only
    #ifdef _WIN32
        RF_INTERNAL long long int rf_internal_global_performance_counter_frequency;
        RF_INTERNAL bool rf_internal_global_performance_counter_frequency_initialised;

        //If windows.h is not included
        #if !defined(_WINDOWS_)
            //Definition so that we don't have to include windows.h
            #ifdef __cplusplus
                extern "C" int __stdcall QueryPerformanceCounter(long long int* lpPerformanceCount);
                extern "C" int __stdcall QueryPerformanceFrequency(long long int* lpFrequency);
                extern "C" void __stdcall Sleep(int dwMilliseconds);
            #else
                extern int __stdcall QueryPerformanceCounter(long long int* lpPerformanceCount);
                extern int __stdcall QueryPerformanceFrequency(long long int* lpFrequency);
                extern void __stdcall Sleep(int dwMilliseconds);
            #endif
        #endif

        // Returns elapsed time in seconds since program started
        RF_API double rf_get_time(void)
        {
            if (rf_internal_ctx->get_time_proc)
            {
                return rf_internal_ctx->get_time_proc();
            }

            if (!rf_internal_global_performance_counter_frequency_initialised)
            {
                #ifdef _WINDOWS_
                    RF_ASSERT(QueryPerformanceFrequency((LARGE_INTEGER*)&rf_internal_global_performance_counter_frequency) != false);
                #else
                    RF_ASSERT(QueryPerformanceFrequency(&rf_internal_global_performance_counter_frequency) != false);
                #endif
                rf_internal_global_performance_counter_frequency_initialised = true;
            }

            long long int qpc_result = {0};

            #ifdef _WINDOWS_
                RF_ASSERT(QueryPerformanceCounter((LARGE_INTEGER*)&qpc_result) != false);
            #else
                RF_ASSERT(QueryPerformanceCounter(&qpc_result) != false);
            #endif

            return (double) qpc_result / (double) rf_internal_global_performance_counter_frequency;
        }

        RF_API void rf_wait(float duration)
        {
            if (rf_internal_ctx->wait_proc)
            {
                rf_internal_ctx->wait_proc(duration);
                return;
            }

            Sleep((int) duration);
        }
    #endif

    #if defined(__linux__)
        #include <time.h>

        //Source: http://man7.org/linux/man-pages/man2/clock_gettime.2.html
        RF_API double rf_get_time(void)
        {
            if (rf_internal_ctx->get_time_proc)
            {
                return rf_internal_ctx->get_time_proc();
            }

            struct timespec result;

            RF_ASSERT(clock_gettime(CLOCK_MONOTONIC_RAW, &result) == 0);

            return (double) result.tv_sec;
        }

        RF_API void rf_wait(float duration)
        {
            if (rf_internal_ctx->wait_proc)
            {
                rf_internal_ctx->wait_proc(duration);
                return;
            }

            long milliseconds = (long) duration;
            struct timespec ts;
            ts.tv_sec = milliseconds / 1000;
            ts.tv_nsec = (milliseconds % 1000) * 1000000;
            nanosleep(&ts, NULL);
        }
    #endif //#elif defined(__linux__)

    #if defined(__MACH__)
        #include <mach/mach_time.h>
        #include <unistd.h>

        RF_INTERNAL bool rf_internal_global_mach_time_initialized;
        RF_INTERNAL uint64_t rf_internal_global_mach_time_start;
        RF_INTERNAL double rf_internal_global_mach_time_seconds_factor;

        RF_API double rf_get_time(void)
        {
            if (rf_internal_ctx->get_time_proc)
            {
                return rf_internal_ctx->get_time_proc();
            }

            uint64_t time;
            if (!rf_internal_global_mach_time_initialized)
            {
                mach_timebase_info_data_t timebase;
                mach_timebase_info(&timebase);
                rf_internal_global_mach_time_seconds_factor = 1e-9 * (double)timebase.numer / (double)timebase.denom;
                rf_internal_global_mach_time_start = mach_absolute_time();
                rf_internal_global_mach_time_initialized = true;
            }
            time = mach_absolute_time();
            return (double)(time - rf_internal_global_mach_time_start) * rf_internal_global_mach_time_seconds_factor;
        }

        RF_API void rf_wait(float duration)
        {
            if (rf_internal_ctx->wait_proc)
            {
                rf_internal_ctx->wait_proc(duration);
                return;
            }

            usleep(duration * 1000);
        }
    #endif //#if defined(__MACH__)
#endif //if !defined(RF_CUSTOM_TIME)
//endregion

//region default io & allocator
#include "malloc.h"

void* rf_malloc_wrapper(rf_allocator_mode mode, int size_to_alloc, void* pointer_to_free, void* user_data)
{
    ((void)user_data);

    switch (mode)
    {
        case RF_AM_ALLOC: return malloc(size_to_alloc);

        case RF_AM_FREE:
        {
            free(pointer_to_free);
        }
        break;
    }

    return NULL;
}

#if !defined(RF_NO_DEFAULT_IO)
#include "stdio.h"

//Get the size of the file
RF_API int rf_get_file_size(const char* filename)
{
    FILE* file = fopen(filename, "rb");

    fseek(file, 0L, SEEK_END);
    int size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    fclose(file);

    return size;
}

//Load the file into a buffer
RF_API bool rf_load_file_into_buffer(const char* filename, void* buffer, int buffer_size)
{
    FILE* file = fopen(filename, "rb");
    if (file == NULL) return false;

    fseek(file, 0L, SEEK_END);
    int file_size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    if (file_size > buffer_size) return false;

    size_t read_size = fread(buffer, sizeof(char), buffer_size, file);

    if (ferror(file) != 0) return false;
    if (read_size != file_size) return false;

    fclose(file);

    return true;
}
#endif
//endregion

//region getters
// Returns time in seconds for last frame drawn
RF_API float rf_get_frame_time()
{
    return (float) rf_internal_ctx->frame_time;
}

// Returns current FPS
RF_API int rf_get_fps()
{
    return (int) roundf(1.0f / rf_get_frame_time());
}

// Get the default font, useful to be used with extended parameters
RF_API rf_font rf_get_default_font()
{
    return rf_internal_ctx->default_font;
}

// Get default shader
RF_API rf_shader rf_get_default_shader()
{
    return rf_internal_ctx->gfx_ctx.default_shader;
}

// Get default internal texture (white texture)
RF_API rf_texture2d rf_get_default_texture()
{
    rf_texture2d texture = { 0 };
    texture.id = rf_internal_ctx->gfx_ctx.default_texture_id;
    texture.width = 1;
    texture.height = 1;
    texture.mipmaps = 1;
    texture.format = RF_UNCOMPRESSED_R8G8B8A8;

    return texture;
}

//Get the context pointer
RF_API rf_context* rf_get_context()
{
    return rf_internal_ctx;
}

// Get pixel data from GPU frontbuffer and return an rf_image (screenshot)
RF_API rf_image rf_get_screen_data(rf_allocator allocator, rf_allocator temp_allocator)
{
    rf_image image = { 0 };

    image.width = rf_internal_ctx->screen_width;
    image.height = rf_internal_ctx->screen_height;
    image.mipmaps = 1;
    image.format = RF_UNCOMPRESSED_R8G8B8A8;
    image.data = rf_gfx_read_screen_pixels(image.width, image.height, allocator, temp_allocator);

    return image;
}
//endregion

//region setters
// Set the global context pointer
RF_API void rf_set_global_context_pointer(rf_context* ctx)
{
    rf_internal_ctx = ctx;
}

// Set viewport for a provided width and height
RF_API void rf_set_viewport(int width, int height)
{
    rf_internal_ctx->render_width = width;
    rf_internal_ctx->render_height = height;

    // Set viewport width and height
    // NOTE: We consider render size and offset in case black bars are required and
    // render area does not match full global_display area (this situation is only applicable on fullscreen mode)
    rf_gfx_viewport(rf_internal_ctx->render_offset_x/2, rf_internal_ctx->render_offset_y/2, rf_internal_ctx->render_width - rf_internal_ctx->render_offset_x, rf_internal_ctx->render_height - rf_internal_ctx->render_offset_y);

    rf_gfx_matrix_mode(RF_PROJECTION); // Switch to PROJECTION matrix
    rf_gfx_load_identity(); // Reset current matrix (PROJECTION)

    // Set orthographic GL_PROJECTION to current framebuffer size
    // NOTE: Confirf_gfx_projectiongured top-left corner as (0, 0)
    rf_gfx_ortho(0, rf_internal_ctx->render_width, rf_internal_ctx->render_height, 0, 0.0f, 1.0f);

    rf_gfx_matrix_mode(RF_MODELVIEW); // Switch back to MODELVIEW matrix
    rf_gfx_load_identity(); // Reset current matrix (MODELVIEW)
}

// Set target FPS (maximum)
RF_API void rf_set_target_fps(int fps)
{
    if (fps < 1) rf_internal_ctx->target_time = RF_UNLOCKED_FPS;
    else rf_internal_ctx->target_time = 1.0 / ((double) fps);

    RF_LOG_V(RF_LOG_INFO, "Target time per frame: %02.03f milliseconds", (float) rf_internal_global_context_ptr->target_time * 1000);
}
//endregion

//region math
//region base64
RF_API int rf_get_size_base64(const unsigned char* input)
{
    int size = 0;

    for (int i = 0; input[4 * i] != 0; i++)
    {
        if (input[4 * i + 3] == '=')
        {
            if (input[4 * i + 2] == '=') size += 1;
            else size += 2;
        }
        else size += 3;
    }

    return size;
}

RF_API rf_base64_output rf_decode_base64(const unsigned char* input, rf_allocator allocator)
{
    rf_base64_output result;
    result.size      = rf_get_size_base64(input);
    result.allocator = allocator;
    result.buffer    = (unsigned char*) RF_ALLOC(allocator, result.size);
    
    for (int i = 0; i < result.size / 3; i++)
    {
        unsigned char a = rf_internal_base64_table[(int)input[4 * i + 0]];
        unsigned char b = rf_internal_base64_table[(int)input[4 * i + 1]];
        unsigned char c = rf_internal_base64_table[(int)input[4 * i + 2]];
        unsigned char d = rf_internal_base64_table[(int)input[4 * i + 3]];

        result.buffer[3 * i + 0] = (a << 2) | (b >> 4);
        result.buffer[3 * i + 1] = (b << 4) | (c >> 2);
        result.buffer[3 * i + 2] = (c << 6) | d;
    }

    int n = result.size / 3;

    if (result.size % 3 == 1)
    {
        unsigned char a = rf_internal_base64_table[(int)input[4 * n + 0]];
        unsigned char b = rf_internal_base64_table[(int)input[4 * n + 1]];

        result.buffer[result.size - 1] = (a << 2) | (b >> 4);
    }
    else if (result.size % 3 == 2)
    {
        unsigned char a = rf_internal_base64_table[(int)input[4 * n + 0]];
        unsigned char b = rf_internal_base64_table[(int)input[4 * n + 1]];
        unsigned char c = rf_internal_base64_table[(int)input[4 * n + 2]];

        result.buffer[result.size - 2] = (a << 2) | (b >> 4);
        result.buffer[result.size - 1] = (b << 4) | (c >> 2);
    }

    return result;
}

RF_API void rf_unload_base64_output(rf_base64_output it)
{
    RF_FREE(it.allocator, it.buffer);
}
//endregion

//region color

// Returns hexadecimal value for a rf_color
RF_API int rf_color_to_int(rf_color color)
{
    return (((int)color.r << 24) | ((int)color.g << 16) | ((int)color.b << 8) | (int)color.a);
}

// Returns color normalized as float [0..1]
RF_API rf_vec4 rf_color_normalize(rf_color color)
{
    rf_vec4 result;

    result.x = (float)color.r/255.0f;
    result.y = (float)color.g/255.0f;
    result.z = (float)color.b/255.0f;
    result.w = (float)color.a/255.0f;

    return result;
}

// Returns color from normalized values [0..1]
RF_API rf_color rf_color_from_normalized(rf_vec4 normalized)
{
    rf_color result;

    result.r = normalized.x*255.0f;
    result.g = normalized.y*255.0f;
    result.b = normalized.z*255.0f;
    result.a = normalized.w*255.0f;

    return result;
}

// Returns HSV values for a rf_color. Hue is returned as degrees [0..360]
RF_API rf_vec3 rf_color_to_hsv(rf_color color)
{
    rf_vec3 rgb = {(float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f };
    rf_vec3 hsv = {0.0f, 0.0f, 0.0f };
    float min, max, delta;

    min = rgb.x < rgb.y? rgb.x : rgb.y;
    min = min < rgb.z? min : rgb.z;

    max = rgb.x > rgb.y? rgb.x : rgb.y;
    max = max > rgb.z? max : rgb.z;

    hsv.z = max; // Value
    delta = max - min;

    if (delta < 0.00001f)
    {
        hsv.y = 0.0f;
        hsv.x = 0.0f; // Undefined, maybe NAN?
        return hsv;
    }

    if (max > 0.0f)
    {
        // NOTE: If max is 0, this divide would cause a crash
        hsv.y = (delta/max); // Saturation
    }
    else
    {
        // NOTE: If max is 0, then r = g = b = 0, s = 0, h is undefined
        hsv.y = 0.0f;
        hsv.x = NAN; // Undefined
        return hsv;
    }

    // NOTE: Comparing float values could not work properly
    if (rgb.x >= max) hsv.x = (rgb.y - rgb.z)/delta; // Between yellow & magenta
    else
    {
        if (rgb.y >= max) hsv.x = 2.0f + (rgb.z - rgb.x)/delta; // Between cyan & yellow
        else hsv.x = 4.0f + (rgb.x - rgb.y)/delta; // Between magenta & cyan
    }

    hsv.x *= 60.0f; // Convert to degrees

    if (hsv.x < 0.0f) hsv.x += 360.0f;

    return hsv;
}

// Returns a rf_color from HSV values. rf_color->HSV->rf_color conversion will not yield exactly the same color due to rounding errors. Implementation reference: https://en.wikipedia.org/wiki/HSL_and_HSV#Alternative_HSV_conversion
RF_API rf_color rf_color_from_hsv(rf_vec3 hsv)
{
    rf_color color = { 0, 0, 0, 255 };
    float h = hsv.x, s = hsv.y, v = hsv.z;

    // Red channel
    float k = fmod((5.0f + h/60.0f), 6);
    float t = 4.0f - k;
    k = (t < k)? t : k;
    k = (k < 1)? k : 1;
    k = (k > 0)? k : 0;
    color.r = (v - v*s*k)*255;

    // Green channel
    k = fmod((3.0f + h/60.0f), 6);
    t = 4.0f - k;
    k = (t < k)? t : k;
    k = (k < 1)? k : 1;
    k = (k > 0)? k : 0;
    color.g = (v - v*s*k)*255;

    // Blue channel
    k = fmod((1.0f + h/60.0f), 6);
    t = 4.0f - k;
    k = (t < k)? t : k;
    k = (k < 1)? k : 1;
    k = (k > 0)? k : 0;
    color.b = (v - v*s*k)*255;

    return color;
}

// Returns a rf_color struct from hexadecimal value
RF_API rf_color rf_color_from_int(int hex_value)
{
    rf_color color;

    color.r = (unsigned char)(hex_value >> 24) & 0xFF;
    color.g = (unsigned char)(hex_value >> 16) & 0xFF;
    color.b = (unsigned char)(hex_value >> 8) & 0xFF;
    color.a = (unsigned char)hex_value & 0xFF;

    return color;
}

// rf_color fade-in or fade-out, alpha goes from 0.0f to 1.0f
RF_API rf_color rf_fade(rf_color color, float alpha)
{
    if (alpha < 0.0f) alpha = 0.0f;
    else if (alpha > 1.0f) alpha = 1.0f;

    return (rf_color){color.r, color.g, color.b, (unsigned char)(255.0f*alpha)};
}

//endregion

//region camera
// Get world coordinates from screen coordinates
RF_API  rf_vec3 rf_unproject(rf_vec3 source, rf_mat proj, rf_mat view)
{
    rf_vec3 result = {0.0f, 0.0f, 0.0f };

    // Calculate unproject matrix (multiply view patrix by rf_internal_ctx->gl_ctx.projection matrix) and invert it
    rf_mat mat_viewProj = rf_mat_mul(view, proj);
    mat_viewProj = rf_mat_invert(mat_viewProj);

    // Create quaternion from source point
    rf_quaternion quat = { source.x, source.y, source.z, 1.0f };

    // Multiply quat point by unproject matrix
    quat = rf_quaternion_transform(quat, mat_viewProj);

    // Normalized world points in vectors
    result.x = quat.x/quat.w;
    result.y = quat.y/quat.w;
    result.z = quat.z/quat.w;

    return result;
}

// Returns a ray trace from mouse position
RF_API  rf_ray rf_get_mouse_ray(rf_sizei screen_size, rf_vec2 mouse_position, rf_camera3d camera)
{
    rf_ray ray;

    // Calculate normalized device coordinates
    // NOTE: y value is negative
    float x = (2.0f*mouse_position.x)/(float)screen_size.width - 1.0f;
    float y = 1.0f - (2.0f*mouse_position.y)/(float)screen_size.height;
    float z = 1.0f;

    // Store values in a vector
    rf_vec3 device_coords = {x, y, z };

    // Calculate view matrix from camera look at
    rf_mat mat_view = rf_mat_look_at(camera.position, camera.target, camera.up);

    rf_mat mat_proj = rf_mat_identity();

    if (camera.type == RF_CAMERA_PERSPECTIVE)
    {
        // Calculate GL_PROJECTION matrix from perspective
        mat_proj = rf_mat_perspective(camera.fovy * RF_DEG2RAD,
                                      ((double) screen_size.width / (double) screen_size.height), 0.01, 1000.0);
    }
    else if (camera.type == RF_CAMERA_ORTHOGRAPHIC)
    {
        float aspect = (float)screen_size.width/(float)screen_size.height;
        double top = camera.fovy/2.0;
        double right = top*aspect;

        // Calculate GL_PROJECTION matrix from orthographic
        mat_proj = rf_mat_ortho(-right, right, -top, top, 0.01, 1000.0);
    }

    // Unproject far/near points
    rf_vec3 near_point = rf_unproject((rf_vec3){device_coords.x, device_coords.y, 0.0f }, mat_proj, mat_view);
    rf_vec3 far_point = rf_unproject((rf_vec3){device_coords.x, device_coords.y, 1.0f }, mat_proj, mat_view);

    // Unproject the mouse cursor in the near plane.
    // We need this as the source position because orthographic projects, compared to perspect doesn't have a
    // convergence point, meaning that the "eye" of the camera is more like a plane than a point.
    rf_vec3 camera_plane_pointer_pos = rf_unproject((rf_vec3){device_coords.x, device_coords.y, -1.0f }, mat_proj, mat_view);

    // Calculate normalized direction vector
    rf_vec3 direction = rf_vec3_normalize(rf_vec3_sub(far_point, near_point));

    if (camera.type == RF_CAMERA_PERSPECTIVE) ray.position = camera.position;
    else if (camera.type == RF_CAMERA_ORTHOGRAPHIC) ray.position = camera_plane_pointer_pos;

    // Apply calculated vectors to ray
    ray.direction = direction;

    return ray;
}

// Get transform matrix for camera
RF_API  rf_mat rf_get_camera_matrix(rf_camera3d camera)
{
    return rf_mat_look_at(camera.position, camera.target, camera.up);
}

// Returns camera 2d transform matrix
RF_API  rf_mat rf_get_camera_matrix2d(rf_camera2d camera)
{
    rf_mat mat_transform = { 0 };
    // The camera in world-space is set by
    //   1. Move it to target
    //   2. Rotate by -rotation and scale by (1/zoom)
    //      When setting higher scale, it's more intuitive for the world to become bigger (= camera become smaller),
    //      not for the camera getting bigger, hence the invert. Same deal with rotation.
    //   3. Move it by (-offset);
    //      Offset defines target transform relative to screen, but since we're effectively "moving" screen (camera)
    //      we need to do it into opposite direction (inverse transform)

    // Having camera transform in world-space, inverse of it gives the rf_gfxobal_model_view transform.
    // Since (A*B*C)' = C'*B'*A', the rf_gfxobal_model_view is
    //   1. Move to offset
    //   2. Rotate and Scale
    //   3. Move by -target
    rf_mat mat_origin = rf_mat_translate(-camera.target.x, -camera.target.y, 0.0f);
    rf_mat mat_rotation = rf_mat_rotate((rf_vec3) {0.0f, 0.0f, 1.0f}, camera.rotation * RF_DEG2RAD);
    rf_mat mat_scale = rf_mat_scale(camera.zoom, camera.zoom, 1.0f);
    rf_mat mat_translation = rf_mat_translate(camera.offset.x, camera.offset.y, 0.0f);

    mat_transform = rf_mat_mul(rf_mat_mul(mat_origin, rf_mat_mul(mat_scale, mat_rotation)), mat_translation);

    return mat_transform;
}

// Returns the screen space position from a 3d world space position
RF_API  rf_vec2 rf_get_world_to_screen(rf_sizei screen_size, rf_vec3 position, rf_camera3d camera)
{
    // Calculate GL_PROJECTION matrix (from perspective instead of frustum
    rf_mat mat_proj = rf_mat_identity();

    if (camera.type == RF_CAMERA_PERSPECTIVE)
    {
        // Calculate GL_PROJECTION matrix from perspective
        mat_proj = rf_mat_perspective(camera.fovy * RF_DEG2RAD,
                                      ((double) screen_size.width / (double) screen_size.height), 0.01, 1000.0);
    }
    else if (camera.type == RF_CAMERA_ORTHOGRAPHIC)
    {
        float aspect = (float)screen_size.width/(float)screen_size.height;
        double top = camera.fovy/2.0;
        double right = top*aspect;

        // Calculate GL_PROJECTION matrix from orthographic
        mat_proj = rf_mat_ortho(-right, right, -top, top, 0.01, 1000.0);
    }

    // Calculate view matrix from camera look at (and transpose it)
    rf_mat mat_view = rf_mat_look_at(camera.position, camera.target, camera.up);

    // Convert world position vector to quaternion
    rf_quaternion world_pos = { position.x, position.y, position.z, 1.0f };

    // rf_transform world position to view
    world_pos = rf_quaternion_transform(world_pos, mat_view);

    // rf_transform result to GL_PROJECTION (clip space position)
    world_pos = rf_quaternion_transform(world_pos, mat_proj);

    // Calculate normalized device coordinates (inverted y)
    rf_vec3 ndc_pos = {world_pos.x / world_pos.w, -world_pos.y / world_pos.w, world_pos.z / world_pos.w };

    // Calculate 2d screen position vector
    rf_vec2 screen_position = {(ndc_pos.x + 1.0f) / 2.0f * (float)screen_size.width, (ndc_pos.y + 1.0f) / 2.0f * (float)screen_size.height };

    return screen_position;
}

// Returns the screen space position for a 2d camera world space position
RF_API  rf_vec2 rf_get_world_to_screen2d(rf_vec2 position, rf_camera2d camera)
{
    rf_mat mat_camera = rf_get_camera_matrix2d(camera);
    rf_vec3 transform = rf_vec3_transform((rf_vec3) {position.x, position.y, 0}, mat_camera);

    return (rf_vec2){transform.x, transform.y };
}

// Returns the world space position for a 2d camera screen space position
RF_API  rf_vec2 rf_get_screen_to_world2d(rf_vec2 position, rf_camera2d camera)
{
    rf_mat inv_mat_camera = rf_mat_invert(rf_get_camera_matrix2d(camera));
    rf_vec3 transform = rf_vec3_transform((rf_vec3) {position.x, position.y, 0}, inv_mat_camera);

    return (rf_vec2){transform.x, transform.y };
}
//endregion

//region vec and matrix math
//Get the buffer size of an image of a specific width and height in a given format
RF_API int rf_get_buffer_size_for_pixel_format(int width, int height, int format)
{
    int data_size = 0; // Size in bytes
    int bpp = 0; // Bits per pixel

    switch (format)
    {
        case RF_UNCOMPRESSED_GRAYSCALE: bpp = 8; break;
        case RF_UNCOMPRESSED_GRAY_ALPHA:
        case RF_UNCOMPRESSED_R5G6B5:
        case RF_UNCOMPRESSED_R5G5B5A1:
        case RF_UNCOMPRESSED_R4G4B4A4: bpp = 16; break;
        case RF_UNCOMPRESSED_R8G8B8A8: bpp = 32; break;
        case RF_UNCOMPRESSED_R8G8B8: bpp = 24; break;
        case RF_UNCOMPRESSED_R32: bpp = 32; break;
        case RF_UNCOMPRESSED_R32G32B32: bpp = 32 * 3; break;
        case RF_UNCOMPRESSED_R32G32B32A32: bpp = 32 * 4; break;
        case RF_COMPRESSED_DXT1_RGB:
        case RF_COMPRESSED_DXT1_RGBA:
        case RF_COMPRESSED_ETC1_RGB:
        case RF_COMPRESSED_ETC2_RGB:
        case RF_COMPRESSED_PVRT_RGB:
        case RF_COMPRESSED_PVRT_RGBA: bpp = 4; break;
        case RF_COMPRESSED_DXT3_RGBA:
        case RF_COMPRESSED_DXT5_RGBA:
        case RF_COMPRESSED_ETC2_EAC_RGBA:
        case RF_COMPRESSED_ASTC_4x4_RGBA: bpp = 8; break;
        case RF_COMPRESSED_ASTC_8x8_RGBA: bpp = 2; break;
        default: break;
    }

    data_size = width * height * bpp / 8; // Total data size in bytes

    return data_size;
}

// Clamp float value
RF_API float rf_clamp(float value, float min, float max)
{
    const float res = value < min ? min : value;
    return res > max ? max : res;
}

// Calculate linear interpolation between two floats
RF_API float rf_lerp(float start, float end, float amount)
{
    return start + amount * (end - start);
}

// Add two vectors (v1 + v2)
RF_API rf_vec2 rf_vec2_add(rf_vec2 v1, rf_vec2 v2)
{
    rf_vec2 result = {v1.x + v2.x, v1.y + v2.y};
    return result;
}

// Subtract two vectors (v1 - v2)
RF_API rf_vec2 rf_vec2_sub(rf_vec2 v1, rf_vec2 v2)
{
    rf_vec2 result = {v1.x - v2.x, v1.y - v2.y};
    return result;
}

// Calculate vector length
RF_API float rf_vec2_len(rf_vec2 v)
{
    float result = sqrt((v.x * v.x) + (v.y * v.y));
    return result;
}

// Calculate two vectors dot product
RF_API float rf_vec2_dot_product(rf_vec2 v1, rf_vec2 v2)
{
    float result = (v1.x * v2.x + v1.y * v2.y);
    return result;
}

// Calculate distance between two vectors
RF_API float rf_vec2_distance(rf_vec2 v1, rf_vec2 v2)
{
    float result = sqrt((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
    return result;
}

// Calculate angle from two vectors in X-axis
RF_API float rf_vec2_angle(rf_vec2 v1, rf_vec2 v2)
{
    float result = atan2f(v2.y - v1.y, v2.x - v1.x) * (180.0f / RF_PI);
    if (result < 0) result += 360.0f;
    return result;
}

// Scale vector (multiply by value)
RF_API rf_vec2 rf_vec2_scale(rf_vec2 v, float scale)
{
    rf_vec2 result = {v.x * scale, v.y * scale};
    return result;
}

// Multiply vector by vector
RF_API rf_vec2 rf_vec2_mul_v(rf_vec2 v1, rf_vec2 v2)
{
    rf_vec2 result = {v1.x * v2.x, v1.y * v2.y};
    return result;
}

// Negate vector
RF_API rf_vec2 rf_vec2_negate(rf_vec2 v)
{
    rf_vec2 result = {-v.x, -v.y};
    return result;
}

// Divide vector by a float value
RF_API rf_vec2 rf_vec2_div(rf_vec2 v, float div)
{
    rf_vec2 result = {v.x / div, v.y / div};
    return result;
}

// Divide vector by vector
RF_API rf_vec2 rf_vec2_div_v(rf_vec2 v1, rf_vec2 v2)
{
    rf_vec2 result = {v1.x / v2.x, v1.y / v2.y};
    return result;
}

// Normalize provided vector
RF_API rf_vec2 rf_vec2_normalize(rf_vec2 v)
{
    rf_vec2 result = rf_vec2_div(v, rf_vec2_len(v));
    return result;
}

// Calculate linear interpolation between two vectors
RF_API rf_vec2 rf_vec2_lerp(rf_vec2 v1, rf_vec2 v2, float amount)
{
    rf_vec2 result = {0};

    result.x = v1.x + amount * (v2.x - v1.x);
    result.y = v1.y + amount * (v2.y - v1.y);

    return result;
}

// Add two vectors
RF_API rf_vec3 rf_vec3_add(rf_vec3 v1, rf_vec3 v2)
{
    rf_vec3 result = {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
    return result;
}

// Subtract two vectors
RF_API rf_vec3 rf_vec3_sub(rf_vec3 v1, rf_vec3 v2)
{
    rf_vec3 result = {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
    return result;
}

// Multiply vector by scalar
RF_API rf_vec3 rf_vec3_mul(rf_vec3 v, float scalar)
{
    rf_vec3 result = {v.x * scalar, v.y * scalar, v.z * scalar};
    return result;
}

// Multiply vector by vector
RF_API rf_vec3 rf_vec3_mul_v(rf_vec3 v1, rf_vec3 v2)
{
    rf_vec3 result = {v1.x * v2.x, v1.y * v2.y, v1.z * v2.z};
    return result;
}

// Calculate two vectors cross product
RF_API rf_vec3 rf_vec3_cross_product(rf_vec3 v1, rf_vec3 v2)
{
    rf_vec3 result = {v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x};
    return result;
}

// Calculate one vector perpendicular vector
RF_API rf_vec3 rf_vec3_perpendicular(rf_vec3 v)
{
    rf_vec3 result = {0};

    float min = (float) fabs(v.x);
    rf_vec3 cardinalAxis = {1.0f, 0.0f, 0.0f};

    if (fabs(v.y) < min)
    {
        min = (float) fabs(v.y);
        rf_vec3 tmp = {0.0f, 1.0f, 0.0f};
        cardinalAxis = tmp;
    }

    if (fabs(v.z) < min)
    {
        rf_vec3 tmp = {0.0f, 0.0f, 1.0f};
        cardinalAxis = tmp;
    }

    result = rf_vec3_cross_product(v, cardinalAxis);

    return result;
}

// Calculate vector length
RF_API float rf_vec3_len(rf_vec3 v)
{
    float result = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return result;
}

// Calculate two vectors dot product
RF_API float rf_vec3_dot_product(rf_vec3 v1, rf_vec3 v2)
{
    float result = (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
    return result;
}

// Calculate distance between two vectors
RF_API float rf_vec3_distance(rf_vec3 v1, rf_vec3 v2)
{
    float dx = v2.x - v1.x;
    float dy = v2.y - v1.y;
    float dz = v2.z - v1.z;
    float result = sqrt(dx * dx + dy * dy + dz * dz);
    return result;
}

// Scale provided vector
RF_API rf_vec3 rf_vec3_scale(rf_vec3 v, float scale)
{
    rf_vec3 result = {v.x * scale, v.y * scale, v.z * scale};
    return result;
}

// Negate provided vector (invert direction)
RF_API rf_vec3 rf_vec3_negate(rf_vec3 v)
{
    rf_vec3 result = {-v.x, -v.y, -v.z};
    return result;
}

// Divide vector by a float value
RF_API rf_vec3 rf_vec3_div(rf_vec3 v, float div)
{
    rf_vec3 result = {v.x / div, v.y / div, v.z / div};
    return result;
}

// Divide vector by vector
RF_API rf_vec3 rf_vec3_div_v(rf_vec3 v1, rf_vec3 v2)
{
    rf_vec3 result = {v1.x / v2.x, v1.y / v2.y, v1.z / v2.z};
    return result;
}

// Normalize provided vector
RF_API rf_vec3 rf_vec3_normalize(rf_vec3 v)
{
    rf_vec3 result = v;

    float length, ilength;
    length = rf_vec3_len(v);
    if (length == 0.0f) length = 1.0f;
    ilength = 1.0f / length;

    result.x *= ilength;
    result.y *= ilength;
    result.z *= ilength;

    return result;
}

// Orthonormalize provided vectors
// Makes vectors normalized and orthogonal to each other
// Gram-Schmidt function implementation
RF_API void rf_vec3_ortho_normalize(rf_vec3 *v1, rf_vec3 *v2)
{
    *v1 = rf_vec3_normalize(*v1);
    rf_vec3 vn = rf_vec3_cross_product(*v1, *v2);
    vn = rf_vec3_normalize(vn);
    *v2 = rf_vec3_cross_product(vn, *v1);
}

// Transforms a rf_vec3 by a given rf_mat
RF_API rf_vec3 rf_vec3_transform(rf_vec3 v, rf_mat mat)
{
    rf_vec3 result = {0};
    float x = v.x;
    float y = v.y;
    float z = v.z;

    result.x = mat.m0 * x + mat.m4 * y + mat.m8 * z + mat.m12;
    result.y = mat.m1 * x + mat.m5 * y + mat.m9 * z + mat.m13;
    result.z = mat.m2 * x + mat.m6 * y + mat.m10 * z + mat.m14;

    return result;
}

// rf_transform a vector by quaternion rotation
RF_API rf_vec3 rf_vec3_rotate_by_quaternion(rf_vec3 v, rf_quaternion q)
{
    rf_vec3 result = {0};

    result.x = v.x * (q.x * q.x + q.w * q.w - q.y * q.y - q.z * q.z) + v.y * (2 * q.x * q.y - 2 * q.w * q.z) +
               v.z * (2 * q.x * q.z + 2 * q.w * q.y);
    result.y = v.x * (2 * q.w * q.z + 2 * q.x * q.y) + v.y * (q.w * q.w - q.x * q.x + q.y * q.y - q.z * q.z) +
               v.z * (-2 * q.w * q.x + 2 * q.y * q.z);
    result.z = v.x * (-2 * q.w * q.y + 2 * q.x * q.z) + v.y * (2 * q.w * q.x + 2 * q.y * q.z) +
               v.z * (q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z);

    return result;
}

// Calculate linear interpolation between two vectors
RF_API rf_vec3 rf_vec3_lerp(rf_vec3 v1, rf_vec3 v2, float amount)
{
    rf_vec3 result = {0};

    result.x = v1.x + amount * (v2.x - v1.x);
    result.y = v1.y + amount * (v2.y - v1.y);
    result.z = v1.z + amount * (v2.z - v1.z);

    return result;
}

// Calculate reflected vector to normal
RF_API rf_vec3 rf_vec3_reflect(rf_vec3 v, rf_vec3 normal)
{
    // I is the original vector
    // N is the normal of the incident plane
    // R = I - (2*N*( DotProduct[ I,N] ))

    rf_vec3 result = {0};

    float dotProduct = rf_vec3_dot_product(v, normal);

    result.x = v.x - (2.0f * normal.x) * dotProduct;
    result.y = v.y - (2.0f * normal.y) * dotProduct;
    result.z = v.z - (2.0f * normal.z) * dotProduct;

    return result;
}

// Return min value for each pair of components
RF_API rf_vec3 rf_vec3_RF_MIN(rf_vec3 v1, rf_vec3 v2)
{
    rf_vec3 result = {0};

    result.x = fminf(v1.x, v2.x);
    result.y = fminf(v1.y, v2.y);
    result.z = fminf(v1.z, v2.z);

    return result;
}

// Return max value for each pair of components
RF_API rf_vec3 rf_vec3_max(rf_vec3 v1, rf_vec3 v2)
{
    rf_vec3 result = {0};

    result.x = fmaxf(v1.x, v2.x);
    result.y = fmaxf(v1.y, v2.y);
    result.z = fmaxf(v1.z, v2.z);

    return result;
}

// Compute barycenter coordinates (u, v, w) for point p with respect to triangle (a, b, c)
// NOTE: Assumes P is on the plane of the triangle
RF_API rf_vec3 rf_vec3_barycenter(rf_vec3 p, rf_vec3 a, rf_vec3 b, rf_vec3 c)
{
//Vector v0 = b - a, v1 = c - a, v2 = p - a;

    rf_vec3 v0 = rf_vec3_sub(b, a);
    rf_vec3 v1 = rf_vec3_sub(c, a);
    rf_vec3 v2 = rf_vec3_sub(p, a);
    float d00 = rf_vec3_dot_product(v0, v0);
    float d01 = rf_vec3_dot_product(v0, v1);
    float d11 = rf_vec3_dot_product(v1, v1);
    float d20 = rf_vec3_dot_product(v2, v0);
    float d21 = rf_vec3_dot_product(v2, v1);

    float denom = d00 * d11 - d01 * d01;

    rf_vec3 result = {0};

    result.y = (d11 * d20 - d01 * d21) / denom;
    result.z = (d00 * d21 - d01 * d20) / denom;
    result.x = 1.0f - (result.z + result.y);

    return result;
}

// Compute matrix determinant
RF_API float rf_mat_determinant(rf_mat mat)
{
    float result = 0.0;

    // Cache the matrix values (speed optimization)
    float a00 = mat.m0, a01 = mat.m1, a02 = mat.m2, a03 = mat.m3;
    float a10 = mat.m4, a11 = mat.m5, a12 = mat.m6, a13 = mat.m7;
    float a20 = mat.m8, a21 = mat.m9, a22 = mat.m10, a23 = mat.m11;
    float a30 = mat.m12, a31 = mat.m13, a32 = mat.m14, a33 = mat.m15;

    result = a30 * a21 * a12 * a03 - a20 * a31 * a12 * a03 - a30 * a11 * a22 * a03 + a10 * a31 * a22 * a03 +
             a20 * a11 * a32 * a03 - a10 * a21 * a32 * a03 - a30 * a21 * a02 * a13 + a20 * a31 * a02 * a13 +
             a30 * a01 * a22 * a13 - a00 * a31 * a22 * a13 - a20 * a01 * a32 * a13 + a00 * a21 * a32 * a13 +
             a30 * a11 * a02 * a23 - a10 * a31 * a02 * a23 - a30 * a01 * a12 * a23 + a00 * a31 * a12 * a23 +
             a10 * a01 * a32 * a23 - a00 * a11 * a32 * a23 - a20 * a11 * a02 * a33 + a10 * a21 * a02 * a33 +
             a20 * a01 * a12 * a33 - a00 * a21 * a12 * a33 - a10 * a01 * a22 * a33 + a00 * a11 * a22 * a33;

    return result;
}

// Returns the trace of the matrix (sum of the values along the diagonal)
RF_API float rf_mat_trace(rf_mat mat)
{
    float result = (mat.m0 + mat.m5 + mat.m10 + mat.m15);
    return result;
}

// Transposes provided matrix
RF_API rf_mat rf_mat_transpose(rf_mat mat)
{
    rf_mat result = {0};

    result.m0 = mat.m0;
    result.m1 = mat.m4;
    result.m2 = mat.m8;
    result.m3 = mat.m12;
    result.m4 = mat.m1;
    result.m5 = mat.m5;
    result.m6 = mat.m9;
    result.m7 = mat.m13;
    result.m8 = mat.m2;
    result.m9 = mat.m6;
    result.m10 = mat.m10;
    result.m11 = mat.m14;
    result.m12 = mat.m3;
    result.m13 = mat.m7;
    result.m14 = mat.m11;
    result.m15 = mat.m15;

    return result;
}

// Invert provided matrix
RF_API rf_mat rf_mat_invert(rf_mat mat)
{
    rf_mat result = {0};

// Cache the matrix values (speed optimization)
    float a00 = mat.m0, a01 = mat.m1, a02 = mat.m2, a03 = mat.m3;
    float a10 = mat.m4, a11 = mat.m5, a12 = mat.m6, a13 = mat.m7;
    float a20 = mat.m8, a21 = mat.m9, a22 = mat.m10, a23 = mat.m11;
    float a30 = mat.m12, a31 = mat.m13, a32 = mat.m14, a33 = mat.m15;

    float b00 = a00 * a11 - a01 * a10;
    float b01 = a00 * a12 - a02 * a10;
    float b02 = a00 * a13 - a03 * a10;
    float b03 = a01 * a12 - a02 * a11;
    float b04 = a01 * a13 - a03 * a11;
    float b05 = a02 * a13 - a03 * a12;
    float b06 = a20 * a31 - a21 * a30;
    float b07 = a20 * a32 - a22 * a30;
    float b08 = a20 * a33 - a23 * a30;
    float b09 = a21 * a32 - a22 * a31;
    float b10 = a21 * a33 - a23 * a31;
    float b11 = a22 * a33 - a23 * a32;

// Calculate the invert determinant (inlined to avoid double-caching)
    float invDet = 1.0f / (b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06);

    result.m0 = (a11 * b11 - a12 * b10 + a13 * b09) * invDet;
    result.m1 = (-a01 * b11 + a02 * b10 - a03 * b09) * invDet;
    result.m2 = (a31 * b05 - a32 * b04 + a33 * b03) * invDet;
    result.m3 = (-a21 * b05 + a22 * b04 - a23 * b03) * invDet;
    result.m4 = (-a10 * b11 + a12 * b08 - a13 * b07) * invDet;
    result.m5 = (a00 * b11 - a02 * b08 + a03 * b07) * invDet;
    result.m6 = (-a30 * b05 + a32 * b02 - a33 * b01) * invDet;
    result.m7 = (a20 * b05 - a22 * b02 + a23 * b01) * invDet;
    result.m8 = (a10 * b10 - a11 * b08 + a13 * b06) * invDet;
    result.m9 = (-a00 * b10 + a01 * b08 - a03 * b06) * invDet;
    result.m10 = (a30 * b04 - a31 * b02 + a33 * b00) * invDet;
    result.m11 = (-a20 * b04 + a21 * b02 - a23 * b00) * invDet;
    result.m12 = (-a10 * b09 + a11 * b07 - a12 * b06) * invDet;
    result.m13 = (a00 * b09 - a01 * b07 + a02 * b06) * invDet;
    result.m14 = (-a30 * b03 + a31 * b01 - a32 * b00) * invDet;
    result.m15 = (a20 * b03 - a21 * b01 + a22 * b00) * invDet;

    return result;
}

// Normalize provided matrix
RF_API rf_mat rf_mat_normalize(rf_mat mat)
{
    rf_mat result = {0};

    float det = rf_mat_determinant(mat);

    result.m0 = mat.m0 / det;
    result.m1 = mat.m1 / det;
    result.m2 = mat.m2 / det;
    result.m3 = mat.m3 / det;
    result.m4 = mat.m4 / det;
    result.m5 = mat.m5 / det;
    result.m6 = mat.m6 / det;
    result.m7 = mat.m7 / det;
    result.m8 = mat.m8 / det;
    result.m9 = mat.m9 / det;
    result.m10 = mat.m10 / det;
    result.m11 = mat.m11 / det;
    result.m12 = mat.m12 / det;
    result.m13 = mat.m13 / det;
    result.m14 = mat.m14 / det;
    result.m15 = mat.m15 / det;

    return result;
}

// Returns identity matrix
RF_API rf_mat rf_mat_identity(void)
{
    rf_mat result = {1.0f, 0.0f, 0.0f, 0.0f,
                     0.0f, 1.0f, 0.0f, 0.0f,
                     0.0f, 0.0f, 1.0f, 0.0f,
                     0.0f, 0.0f, 0.0f, 1.0f};

    return result;
}

// Add two matrices
RF_API rf_mat rf_mat_add(rf_mat left, rf_mat right)
{
    rf_mat result = rf_mat_identity();

    result.m0 = left.m0 + right.m0;
    result.m1 = left.m1 + right.m1;
    result.m2 = left.m2 + right.m2;
    result.m3 = left.m3 + right.m3;
    result.m4 = left.m4 + right.m4;
    result.m5 = left.m5 + right.m5;
    result.m6 = left.m6 + right.m6;
    result.m7 = left.m7 + right.m7;
    result.m8 = left.m8 + right.m8;
    result.m9 = left.m9 + right.m9;
    result.m10 = left.m10 + right.m10;
    result.m11 = left.m11 + right.m11;
    result.m12 = left.m12 + right.m12;
    result.m13 = left.m13 + right.m13;
    result.m14 = left.m14 + right.m14;
    result.m15 = left.m15 + right.m15;

    return result;
}

// Subtract two matrices (left - right)
RF_API rf_mat rf_mat_sub(rf_mat left, rf_mat right)
{
    rf_mat result = rf_mat_identity();

    result.m0 = left.m0 - right.m0;
    result.m1 = left.m1 - right.m1;
    result.m2 = left.m2 - right.m2;
    result.m3 = left.m3 - right.m3;
    result.m4 = left.m4 - right.m4;
    result.m5 = left.m5 - right.m5;
    result.m6 = left.m6 - right.m6;
    result.m7 = left.m7 - right.m7;
    result.m8 = left.m8 - right.m8;
    result.m9 = left.m9 - right.m9;
    result.m10 = left.m10 - right.m10;
    result.m11 = left.m11 - right.m11;
    result.m12 = left.m12 - right.m12;
    result.m13 = left.m13 - right.m13;
    result.m14 = left.m14 - right.m14;
    result.m15 = left.m15 - right.m15;

    return result;
}

// Returns translation matrix
RF_API rf_mat rf_mat_translate(float x, float y, float z)
{
    rf_mat result = {1.0f, 0.0f, 0.0f, x,
                     0.0f, 1.0f, 0.0f, y,
                     0.0f, 0.0f, 1.0f, z,
                     0.0f, 0.0f, 0.0f, 1.0f};

    return result;
}

// Create rotation matrix from axis and angle
// NOTE: Angle should be provided in radians
RF_API rf_mat rf_mat_rotate(rf_vec3 axis, float angle)
{
    rf_mat result = {0};

    float x = axis.x, y = axis.y, z = axis.z;

    float length = sqrt(x * x + y * y + z * z);

    if ((length != 1.0f) && (length != 0.0f))
    {
        length = 1.0f / length;
        x *= length;
        y *= length;
        z *= length;
    }

    float sinres = sinf(angle);
    float cosres = cosf(angle);
    float t = 1.0f - cosres;

    result.m0 = x * x * t + cosres;
    result.m1 = y * x * t + z * sinres;
    result.m2 = z * x * t - y * sinres;
    result.m3 = 0.0f;

    result.m4 = x * y * t - z * sinres;
    result.m5 = y * y * t + cosres;
    result.m6 = z * y * t + x * sinres;
    result.m7 = 0.0f;

    result.m8 = x * z * t + y * sinres;
    result.m9 = y * z * t - x * sinres;
    result.m10 = z * z * t + cosres;
    result.m11 = 0.0f;

    result.m12 = 0.0f;
    result.m13 = 0.0f;
    result.m14 = 0.0f;
    result.m15 = 1.0f;

    return result;
}

// Returns xyz-rotation matrix (angles in radians)
RF_API rf_mat rf_mat_rotate_xyz(rf_vec3 ang)
{
    rf_mat result = rf_mat_identity();

    float cosz = cosf(-ang.z);
    float sinz = sinf(-ang.z);
    float cosy = cosf(-ang.y);
    float siny = sinf(-ang.y);
    float cosx = cosf(-ang.x);
    float sinx = sinf(-ang.x);

    result.m0 = cosz * cosy;
    result.m4 = (cosz * siny * sinx) - (sinz * cosx);
    result.m8 = (cosz * siny * cosx) + (sinz * sinx);

    result.m1 = sinz * cosy;
    result.m5 = (sinz * siny * sinx) + (cosz * cosx);
    result.m9 = (sinz * siny * cosx) - (cosz * sinx);

    result.m2 = -siny;
    result.m6 = cosy * sinx;
    result.m10 = cosy * cosx;

    return result;
}

// Returns x-rotation matrix (angle in radians)
RF_API rf_mat rf_mat_rotate_x(float angle)
{
    rf_mat result = rf_mat_identity();

    float cosres = cosf(angle);
    float sinres = sinf(angle);

    result.m5 = cosres;
    result.m6 = -sinres;
    result.m9 = sinres;
    result.m10 = cosres;

    return result;
}

// Returns y-rotation matrix (angle in radians)
RF_API rf_mat rf_mat_rotate_y(float angle)
{
    rf_mat result = rf_mat_identity();

    float cosres = cosf(angle);
    float sinres = sinf(angle);

    result.m0 = cosres;
    result.m2 = sinres;
    result.m8 = -sinres;
    result.m10 = cosres;

    return result;
}

// Returns z-rotation matrix (angle in radians)
RF_API rf_mat rf_mat_rotate_z(float angle)
{
    rf_mat result = rf_mat_identity();

    float cosres = cosf(angle);
    float sinres = sinf(angle);

    result.m0 = cosres;
    result.m1 = -sinres;
    result.m4 = sinres;
    result.m5 = cosres;

    return result;
}

// Returns scaling matrix
RF_API rf_mat rf_mat_scale(float x, float y, float z)
{
    rf_mat result = {x, 0.0f, 0.0f, 0.0f,
                     0.0f, y, 0.0f, 0.0f,
                     0.0f, 0.0f, z, 0.0f,
                     0.0f, 0.0f, 0.0f, 1.0f};

    return result;
}

// Returns two matrix multiplication
// NOTE: When multiplying matrices... the order matters!
RF_API rf_mat rf_mat_mul(rf_mat left, rf_mat right)
{
    rf_mat result = {0};

    result.m0 = left.m0 * right.m0 + left.m1 * right.m4 + left.m2 * right.m8 + left.m3 * right.m12;
    result.m1 = left.m0 * right.m1 + left.m1 * right.m5 + left.m2 * right.m9 + left.m3 * right.m13;
    result.m2 = left.m0 * right.m2 + left.m1 * right.m6 + left.m2 * right.m10 + left.m3 * right.m14;
    result.m3 = left.m0 * right.m3 + left.m1 * right.m7 + left.m2 * right.m11 + left.m3 * right.m15;
    result.m4 = left.m4 * right.m0 + left.m5 * right.m4 + left.m6 * right.m8 + left.m7 * right.m12;
    result.m5 = left.m4 * right.m1 + left.m5 * right.m5 + left.m6 * right.m9 + left.m7 * right.m13;
    result.m6 = left.m4 * right.m2 + left.m5 * right.m6 + left.m6 * right.m10 + left.m7 * right.m14;
    result.m7 = left.m4 * right.m3 + left.m5 * right.m7 + left.m6 * right.m11 + left.m7 * right.m15;
    result.m8 = left.m8 * right.m0 + left.m9 * right.m4 + left.m10 * right.m8 + left.m11 * right.m12;
    result.m9 = left.m8 * right.m1 + left.m9 * right.m5 + left.m10 * right.m9 + left.m11 * right.m13;
    result.m10 = left.m8 * right.m2 + left.m9 * right.m6 + left.m10 * right.m10 + left.m11 * right.m14;
    result.m11 = left.m8 * right.m3 + left.m9 * right.m7 + left.m10 * right.m11 + left.m11 * right.m15;
    result.m12 = left.m12 * right.m0 + left.m13 * right.m4 + left.m14 * right.m8 + left.m15 * right.m12;
    result.m13 = left.m12 * right.m1 + left.m13 * right.m5 + left.m14 * right.m9 + left.m15 * right.m13;
    result.m14 = left.m12 * right.m2 + left.m13 * right.m6 + left.m14 * right.m10 + left.m15 * right.m14;
    result.m15 = left.m12 * right.m3 + left.m13 * right.m7 + left.m14 * right.m11 + left.m15 * right.m15;

    return result;
}

// Returns perspective GL_PROJECTION matrix
RF_API rf_mat rf_mat_frustum(double left, double right, double bottom, double top, double near_val, double far_val)
{
    rf_mat result = {0};

    float rl = (float) (right - left);
    float tb = (float) (top - bottom);
    float fn = (float) (far_val - near_val);

    result.m0 = ((float) near_val * 2.0f) / rl;
    result.m1 = 0.0f;
    result.m2 = 0.0f;
    result.m3 = 0.0f;

    result.m4 = 0.0f;
    result.m5 = ((float) near_val * 2.0f) / tb;
    result.m6 = 0.0f;
    result.m7 = 0.0f;

    result.m8 = ((float) right + (float) left) / rl;
    result.m9 = ((float) top + (float) bottom) / tb;
    result.m10 = -((float) far_val + (float) near_val) / fn;
    result.m11 = -1.0f;

    result.m12 = 0.0f;
    result.m13 = 0.0f;
    result.m14 = -((float) far_val * (float) near_val * 2.0f) / fn;
    result.m15 = 0.0f;

    return result;
}

// Returns perspective GL_PROJECTION matrix
// NOTE: Angle should be provided in radians
RF_API rf_mat rf_mat_perspective(double fovy, double aspect, double near_val, double far_val)
{
    double top = near_val * tan(fovy * 0.5);
    double right = top * aspect;
    rf_mat result = rf_mat_frustum(-right, right, -top, top, near_val, far_val);

    return result;
}

// Returns orthographic GL_PROJECTION matrix
RF_API rf_mat rf_mat_ortho(double left, double right, double bottom, double top, double near_val, double far_val)
{
    rf_mat result = {0};

    float rl = (float) (right - left);
    float tb = (float) (top - bottom);
    float fn = (float) (far_val - near_val);

    result.m0 = 2.0f / rl;
    result.m1 = 0.0f;
    result.m2 = 0.0f;
    result.m3 = 0.0f;
    result.m4 = 0.0f;
    result.m5 = 2.0f / tb;
    result.m6 = 0.0f;
    result.m7 = 0.0f;
    result.m8 = 0.0f;
    result.m9 = 0.0f;
    result.m10 = -2.0f / fn;
    result.m11 = 0.0f;
    result.m12 = -((float) left + (float) right) / rl;
    result.m13 = -((float) top + (float) bottom) / tb;
    result.m14 = -((float) far_val + (float) near_val) / fn;
    result.m15 = 1.0f;

    return result;
}

// Returns camera look-at matrix (view matrix)
RF_API rf_mat rf_mat_look_at(rf_vec3 eye, rf_vec3 target, rf_vec3 up)
{
    rf_mat result = {0};

    rf_vec3 z = rf_vec3_sub(eye, target);
    z = rf_vec3_normalize(z);
    rf_vec3 x = rf_vec3_cross_product(up, z);
    x = rf_vec3_normalize(x);
    rf_vec3 y = rf_vec3_cross_product(z, x);
    y = rf_vec3_normalize(y);

    result.m0 = x.x;
    result.m1 = x.y;
    result.m2 = x.z;
    result.m3 = 0.0f;
    result.m4 = y.x;
    result.m5 = y.y;
    result.m6 = y.z;
    result.m7 = 0.0f;
    result.m8 = z.x;
    result.m9 = z.y;
    result.m10 = z.z;
    result.m11 = 0.0f;
    result.m12 = eye.x;
    result.m13 = eye.y;
    result.m14 = eye.z;
    result.m15 = 1.0f;

    result = rf_mat_invert(result);

    return result;
}

RF_API rf_float16 rf_mat_to_float16(rf_mat mat)
{
    rf_float16 buffer = {0};

    buffer.v[0] = mat.m0;
    buffer.v[1] = mat.m1;
    buffer.v[2] = mat.m2;
    buffer.v[3] = mat.m3;
    buffer.v[4] = mat.m4;
    buffer.v[5] = mat.m5;
    buffer.v[6] = mat.m6;
    buffer.v[7] = mat.m7;
    buffer.v[8] = mat.m8;
    buffer.v[9] = mat.m9;
    buffer.v[10] = mat.m10;
    buffer.v[11] = mat.m11;
    buffer.v[12] = mat.m12;
    buffer.v[13] = mat.m13;
    buffer.v[14] = mat.m14;
    buffer.v[15] = mat.m15;

    return buffer;
}

// Returns identity quaternion
RF_API rf_quaternion rf_quaternion_identity(void)
{
    rf_quaternion result = {0.0f, 0.0f, 0.0f, 1.0f};
    return result;
}

// Computes the length of a quaternion
RF_API float rf_quaternion_len(rf_quaternion q)
{
    float result = (float) sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    return result;
}

// Normalize provided quaternion
RF_API rf_quaternion rf_quaternion_normalize(rf_quaternion q)
{
    rf_quaternion result = {0};

    float length, ilength;
    length = rf_quaternion_len(q);
    if (length == 0.0f) length = 1.0f;
    ilength = 1.0f / length;

    result.x = q.x * ilength;
    result.y = q.y * ilength;
    result.z = q.z * ilength;
    result.w = q.w * ilength;

    return result;
}

// Invert provided quaternion
RF_API rf_quaternion rf_quaternion_invert(rf_quaternion q)
{
    rf_quaternion result = q;
    float length = rf_quaternion_len(q);
    float lengthSq = length * length;

    if (lengthSq != 0.0)
    {
        float i = 1.0f / lengthSq;

        result.x *= -i;
        result.y *= -i;
        result.z *= -i;
        result.w *= i;
    }

    return result;
}

// Calculate two quaternion multiplication
RF_API rf_quaternion rf_quaternion_mul(rf_quaternion q1, rf_quaternion q2)
{
    rf_quaternion result = {0};

    float qax = q1.x, qay = q1.y, qaz = q1.z, qaw = q1.w;
    float qbx = q2.x, qby = q2.y, qbz = q2.z, qbw = q2.w;

    result.x = qax * qbw + qaw * qbx + qay * qbz - qaz * qby;
    result.y = qay * qbw + qaw * qby + qaz * qbx - qax * qbz;
    result.z = qaz * qbw + qaw * qbz + qax * qby - qay * qbx;
    result.w = qaw * qbw - qax * qbx - qay * qby - qaz * qbz;

    return result;
}

// Calculate linear interpolation between two quaternions
RF_API rf_quaternion rf_quaternion_lerp(rf_quaternion q1, rf_quaternion q2, float amount)
{
    rf_quaternion result = {0};

    result.x = q1.x + amount * (q2.x - q1.x);
    result.y = q1.y + amount * (q2.y - q1.y);
    result.z = q1.z + amount * (q2.z - q1.z);
    result.w = q1.w + amount * (q2.w - q1.w);

    return result;
}

// Calculate slerp-optimized interpolation between two quaternions
RF_API rf_quaternion rf_quaternion_nlerp(rf_quaternion q1, rf_quaternion q2, float amount)
{
    rf_quaternion result = rf_quaternion_lerp(q1, q2, amount);
    result = rf_quaternion_normalize(result);

    return result;
}

// Calculates spherical linear interpolation between two quaternions
RF_API rf_quaternion rf_quaternion_slerp(rf_quaternion q1, rf_quaternion q2, float amount)
{
    rf_quaternion result = {0};

    float cosHalfTheta = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;

    if (fabs(cosHalfTheta) >= 1.0f) result = q1;
    else if (cosHalfTheta > 0.95f) result = rf_quaternion_nlerp(q1, q2, amount);
    else
    {
        float halfTheta = (float) acos(cosHalfTheta);
        float sinHalfTheta = (float) sqrt(1.0f - cosHalfTheta * cosHalfTheta);

        if (fabs(sinHalfTheta) < 0.001f)
        {
            result.x = (q1.x * 0.5f + q2.x * 0.5f);
            result.y = (q1.y * 0.5f + q2.y * 0.5f);
            result.z = (q1.z * 0.5f + q2.z * 0.5f);
            result.w = (q1.w * 0.5f + q2.w * 0.5f);
        } else
        {
            float ratioA = sinf((1 - amount) * halfTheta) / sinHalfTheta;
            float ratioB = sinf(amount * halfTheta) / sinHalfTheta;

            result.x = (q1.x * ratioA + q2.x * ratioB);
            result.y = (q1.y * ratioA + q2.y * ratioB);
            result.z = (q1.z * ratioA + q2.z * ratioB);
            result.w = (q1.w * ratioA + q2.w * ratioB);
        }
    }

    return result;
}

// Calculate quaternion based on the rotation from one vector to another
RF_API rf_quaternion rf_quaternion_from_vector3_to_vector3(rf_vec3 from, rf_vec3 to)
{
    rf_quaternion result = {0};

    float cos2Theta = rf_vec3_dot_product(from, to);
    rf_vec3 cross = rf_vec3_cross_product(from, to);

    result.x = cross.x;
    result.y = cross.y;
    result.z = cross.y;
    result.w = 1.0f + cos2Theta; // NOTE: Added QuaternioIdentity()

// Normalize to essentially nlerp the original and identity to 0.5
    result = rf_quaternion_normalize(result);

// Above lines are equivalent to:
//rf_quaternion result = rf_quaternion_nlerp(q, rf_quaternion_identity(), 0.5f);

    return result;
}

// Returns a quaternion for a given rotation matrix
RF_API rf_quaternion rf_quaternion_from_matrix(rf_mat mat)
{
    rf_quaternion result = {0};

    float trace = rf_mat_trace(mat);

    if (trace > 0.0f)
    {
        float s = (float) sqrt(trace + 1) * 2.0f;
        float invS = 1.0f / s;

        result.w = s * 0.25f;
        result.x = (mat.m6 - mat.m9) * invS;
        result.y = (mat.m8 - mat.m2) * invS;
        result.z = (mat.m1 - mat.m4) * invS;
    } else
    {
        float m00 = mat.m0, m11 = mat.m5, m22 = mat.m10;

        if (m00 > m11 && m00 > m22)
        {
            float s = (float) sqrt(1.0f + m00 - m11 - m22) * 2.0f;
            float invS = 1.0f / s;

            result.w = (mat.m6 - mat.m9) * invS;
            result.x = s * 0.25f;
            result.y = (mat.m4 + mat.m1) * invS;
            result.z = (mat.m8 + mat.m2) * invS;
        } else if (m11 > m22)
        {
            float s = (float) sqrt(1.0f + m11 - m00 - m22) * 2.0f;
            float invS = 1.0f / s;

            result.w = (mat.m8 - mat.m2) * invS;
            result.x = (mat.m4 + mat.m1) * invS;
            result.y = s * 0.25f;
            result.z = (mat.m9 + mat.m6) * invS;
        } else
        {
            float s = (float) sqrt(1.0f + m22 - m00 - m11) * 2.0f;
            float invS = 1.0f / s;

            result.w = (mat.m1 - mat.m4) * invS;
            result.x = (mat.m8 + mat.m2) * invS;
            result.y = (mat.m9 + mat.m6) * invS;
            result.z = s * 0.25f;
        }
    }

    return result;
}

// Returns a matrix for a given quaternion
RF_API rf_mat rf_quaternion_to_matrix(rf_quaternion q)
{
    rf_mat result = {0};

    float x = q.x, y = q.y, z = q.z, w = q.w;

    float x2 = x + x;
    float y2 = y + y;
    float z2 = z + z;

    float length = rf_quaternion_len(q);
    float lengthSquared = length * length;

    float xx = x * x2 / lengthSquared;
    float xy = x * y2 / lengthSquared;
    float xz = x * z2 / lengthSquared;

    float yy = y * y2 / lengthSquared;
    float yz = y * z2 / lengthSquared;
    float zz = z * z2 / lengthSquared;

    float wx = w * x2 / lengthSquared;
    float wy = w * y2 / lengthSquared;
    float wz = w * z2 / lengthSquared;

    result.m0 = 1.0f - (yy + zz);
    result.m1 = xy - wz;
    result.m2 = xz + wy;
    result.m3 = 0.0f;
    result.m4 = xy + wz;
    result.m5 = 1.0f - (xx + zz);
    result.m6 = yz - wx;
    result.m7 = 0.0f;
    result.m8 = xz - wy;
    result.m9 = yz + wx;
    result.m10 = 1.0f - (xx + yy);
    result.m11 = 0.0f;
    result.m12 = 0.0f;
    result.m13 = 0.0f;
    result.m14 = 0.0f;
    result.m15 = 1.0f;

    return result;
}

// Returns rotation quaternion for an angle and axis
// NOTE: angle must be provided in radians
RF_API rf_quaternion rf_quaternion_from_axis_angle(rf_vec3 axis, float angle)
{
    rf_quaternion result = {0.0f, 0.0f, 0.0f, 1.0f};

    if (rf_vec3_len(axis) != 0.0f)

        angle *= 0.5f;

    axis = rf_vec3_normalize(axis);

    float sinres = sinf(angle);
    float cosres = cosf(angle);

    result.x = axis.x * sinres;
    result.y = axis.y * sinres;
    result.z = axis.z * sinres;
    result.w = cosres;

    result = rf_quaternion_normalize(result);

    return result;
}

// Returns the rotation angle and axis for a given quaternion
RF_API void rf_quaternion_to_axis_angle(rf_quaternion q, rf_vec3 *outAxis, float *outAngle)
{
    if (fabs(q.w) > 1.0f) q = rf_quaternion_normalize(q);

    rf_vec3 resAxis = {0.0f, 0.0f, 0.0f};
    float resAngle = 0.0f;

    resAngle = 2.0f * (float) acos(q.w);
    float den = (float) sqrt(1.0f - q.w * q.w);

    if (den > 0.0001f)
    {
        resAxis.x = q.x / den;
        resAxis.y = q.y / den;
        resAxis.z = q.z / den;
    } else
    {
        // This occurs when the angle is zero.
        // Not a problem: just set an arbitrary normalized axis.
        resAxis.x = 1.0f;
    }

    *outAxis = resAxis;
    *outAngle = resAngle;
}

// Returns he quaternion equivalent to Euler angles
RF_API rf_quaternion rf_quaternion_from_euler(float roll, float pitch, float yaw)
{
    rf_quaternion q = {0};

    float x0 = cosf(roll * 0.5f);
    float x1 = sinf(roll * 0.5f);
    float y0 = cosf(pitch * 0.5f);
    float y1 = sinf(pitch * 0.5f);
    float z0 = cosf(yaw * 0.5f);
    float z1 = sinf(yaw * 0.5f);

    q.x = x1 * y0 * z0 - x0 * y1 * z1;
    q.y = x0 * y1 * z0 + x1 * y0 * z1;
    q.z = x0 * y0 * z1 - x1 * y1 * z0;
    q.w = x0 * y0 * z0 + x1 * y1 * z1;

    return q;
}

// Return the Euler angles equivalent to quaternion (roll, pitch, yaw)
// NOTE: Angles are returned in a rf_vec3 struct in degrees
RF_API rf_vec3 rf_quaternion_to_euler(rf_quaternion q)
{
    rf_vec3 result = {0};

// roll (x-axis rotation)
    float x0 = 2.0f * (q.w * q.x + q.y * q.z);
    float x1 = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    result.x = atan2f(x0, x1) * RF_RAD2DEG;

// pitch (y-axis rotation)
    float y0 = 2.0f * (q.w * q.y - q.z * q.x);
    y0 = y0 > 1.0f ? 1.0f : y0;
    y0 = y0 < -1.0f ? -1.0f : y0;
    result.y = asinf(y0) * RF_RAD2DEG;

// yaw (z-axis rotation)
    float z0 = 2.0f * (q.w * q.z + q.x * q.y);
    float z1 = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
    result.z = atan2f(z0, z1) * RF_RAD2DEG;

    return result;
}

// rf_transform a quaternion given a transformation matrix
RF_API rf_quaternion rf_quaternion_transform(rf_quaternion q, rf_mat mat)
{
    rf_quaternion result = {0};

    result.x = mat.m0 * q.x + mat.m4 * q.y + mat.m8 * q.z + mat.m12 * q.w;
    result.y = mat.m1 * q.x + mat.m5 * q.y + mat.m9 * q.z + mat.m13 * q.w;
    result.z = mat.m2 * q.x + mat.m6 * q.y + mat.m10 * q.z + mat.m14 * q.w;
    result.w = mat.m3 * q.x + mat.m7 * q.y + mat.m11 * q.z + mat.m15 * q.w;

    return result;
}
//endregion

//region collision detection

// Check if point is inside rectangle
bool rf_check_collision_point_rec(rf_vec2 point, rf_rec rec)
{
    bool collision = 0;

    if ((point.x >= rec.x) && (point.x <= (rec.x + rec.width)) && (point.y >= rec.y) &&
        (point.y <= (rec.y + rec.height)))
        collision = 1;

    return collision;
}

// Check if point is inside circle
bool rf_check_collision_point_circle(rf_vec2 point, rf_vec2 center, float radius)
{
    return rf_check_collision_circles(point, 0, center, radius);
}

// Check if point is inside a triangle defined by three points (p1, p2, p3)
bool rf_check_collision_point_triangle(rf_vec2 point, rf_vec2 p1, rf_vec2 p2, rf_vec2 p3)
{
    bool collision = 0;

    float alpha = ((p2.y - p3.y) * (point.x - p3.x) + (p3.x - p2.x) * (point.y - p3.y)) /
                  ((p2.y - p3.y) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.y - p3.y));

    float beta = ((p3.y - p1.y) * (point.x - p3.x) + (p1.x - p3.x) * (point.y - p3.y)) /
                 ((p2.y - p3.y) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.y - p3.y));

    float gamma = 1.0f - alpha - beta;

    if ((alpha > 0) && (beta > 0) & (gamma > 0)) collision = 1;

    return collision;
}

// Check collision between two rectangles
bool rf_check_collision_recs(rf_rec rec1, rf_rec rec2)
{
    bool collision = false;

    if ((rec1.x < (rec2.x + rec2.width) && (rec1.x + rec1.width) > rec2.x) &&
        (rec1.y < (rec2.y + rec2.height) && (rec1.y + rec1.height) > rec2.y))
        collision = true;

    return collision;
}

// Check collision between two circles
bool rf_check_collision_circles(rf_vec2 center1, float radius1, rf_vec2 center2, float radius2)
{
    bool collision = false;

    float dx = center2.x - center1.x; // X distance between centers
    float dy = center2.y - center1.y; // Y distance between centers

    float distance = sqrt(dx * dx + dy * dy); // Distance between centers

    if (distance <= (radius1 + radius2)) collision = true;

    return collision;
}

// Check collision between circle and rectangle
// NOTE: Reviewed version to take into account corner limit case
bool rf_check_collision_circle_rec(rf_vec2 center, float radius, rf_rec rec)
{
    int recCenterX = (int) (rec.x + rec.width / 2.0f);
    int recCenterY = (int) (rec.y + rec.height / 2.0f);

    float dx = (float) fabs(center.x - recCenterX);
    float dy = (float) fabs(center.y - recCenterY);

    if (dx > (rec.width / 2.0f + radius))
    { return 0; }
    if (dy > (rec.height / 2.0f + radius))
    { return 0; }

    if (dx <= (rec.width / 2.0f))
    { return 1; }
    if (dy <= (rec.height / 2.0f))
    { return 1; }

    float cornerDistanceSq = (dx - rec.width / 2.0f) * (dx - rec.width / 2.0f) +
                             (dy - rec.height / 2.0f) * (dy - rec.height / 2.0f);

    return (cornerDistanceSq <= (radius * radius));
}

// Get collision rectangle for two rectangles collision
rf_rec rf_get_collision_rec(rf_rec rec1, rf_rec rec2)
{
    rf_rec retRec = {0, 0, 0, 0};

    if (rf_check_collision_recs(rec1, rec2))
    {
        float dxx = (float) fabs(rec1.x - rec2.x);
        float dyy = (float) fabs(rec1.y - rec2.y);

        if (rec1.x <= rec2.x)
        {
            if (rec1.y <= rec2.y)
            {
                retRec.x = rec2.x;
                retRec.y = rec2.y;
                retRec.width = rec1.width - dxx;
                retRec.height = rec1.height - dyy;
            } else
            {
                retRec.x = rec2.x;
                retRec.y = rec1.y;
                retRec.width = rec1.width - dxx;
                retRec.height = rec2.height - dyy;
            }
        } else
        {
            if (rec1.y <= rec2.y)
            {
                retRec.x = rec1.x;
                retRec.y = rec2.y;
                retRec.width = rec2.width - dxx;
                retRec.height = rec1.height - dyy;
            } else
            {
                retRec.x = rec1.x;
                retRec.y = rec1.y;
                retRec.width = rec2.width - dxx;
                retRec.height = rec2.height - dyy;
            }
        }

        if (rec1.width > rec2.width)
        {
            if (retRec.width >= rec2.width) retRec.width = rec2.width;
        }
        else
        {
            if (retRec.width >= rec1.width) retRec.width = rec1.width;
        }

        if (rec1.height > rec2.height)
        {
            if (retRec.height >= rec2.height) retRec.height = rec2.height;
        }
        else
        {
            if (retRec.height >= rec1.height) retRec.height = rec1.height;
        }
    }

    return retRec;
}

// Detect collision between two spheres
RF_API bool rf_check_collision_spheres(rf_vec3 center_a, float radius_a, rf_vec3 center_b, float radius_b)
{
    bool collision = false;

    // Simple way to check for collision, just checking distance between two points
    // Unfortunately, sqrtf() is a costly operation, so we avoid it with following solution
    /*
    float dx = centerA.x - centerB.x;      // X distance between centers
    float dy = centerA.y - centerB.y;      // Y distance between centers
    float dz = centerA.z - centerB.z;      // Z distance between centers

    float distance = sqrtf(dx*dx + dy*dy + dz*dz);  // Distance between centers

    if (distance <= (radiusA + radiusB)) collision = true;
    */
    // Check for distances squared to avoid sqrtf()
    if (rf_vec3_dot_product(rf_vec3_sub(center_b, center_a), rf_vec3_sub(center_b, center_a)) <= (radius_a + radius_b) * (radius_a + radius_b)) collision = true;

    return collision;
}

// Detect collision between two boxes. Note: Boxes are defined by two points minimum and maximum
RF_API bool rf_check_collision_boxes(rf_bounding_box box1, rf_bounding_box box2)
{
    bool collision = true;

    if ((box1.max.x >= box2.min.x) && (box1.min.x <= box2.max.x))
    {
        if ((box1.max.y < box2.min.y) || (box1.min.y > box2.max.y)) collision = false;
        if ((box1.max.z < box2.min.z) || (box1.min.z > box2.max.z)) collision = false;
    }
    else collision = false;

    return collision;
}

// Detect collision between box and sphere
RF_API bool rf_check_collision_box_sphere(rf_bounding_box box, rf_vec3 center, float radius)
{
    bool collision = false;

    float dmin = 0;

    if (center.x < box.min.x) dmin += powf(center.x - box.min.x, 2);
    else if (center.x > box.max.x) dmin += powf(center.x - box.max.x, 2);

    if (center.y < box.min.y) dmin += powf(center.y - box.min.y, 2);
    else if (center.y > box.max.y) dmin += powf(center.y - box.max.y, 2);

    if (center.z < box.min.z) dmin += powf(center.z - box.min.z, 2);
    else if (center.z > box.max.z) dmin += powf(center.z - box.max.z, 2);

    if (dmin <= (radius*radius)) collision = true;

    return collision;
}

// Detect collision between ray and sphere
RF_API bool rf_check_collision_ray_sphere(rf_ray ray, rf_vec3 center, float radius)
{
    bool collision = false;

    rf_vec3 ray_sphere_pos = rf_vec3_sub(center, ray.position);
    float distance = rf_vec3_len(ray_sphere_pos);
    float vector = rf_vec3_dot_product(ray_sphere_pos, ray.direction);
    float d = radius*radius - (distance*distance - vector*vector);

    if (d >= 0.0f) collision = true;

    return collision;
}

// Detect collision between ray and sphere with extended parameters and collision point detection
RF_API bool rf_check_collision_ray_sphere_ex(rf_ray ray, rf_vec3 center, float radius, rf_vec3* collision_point)
{
    bool collision = false;

    rf_vec3 ray_sphere_pos = rf_vec3_sub(center, ray.position);
    float distance = rf_vec3_len(ray_sphere_pos);
    float vector = rf_vec3_dot_product(ray_sphere_pos, ray.direction);
    float d = radius*radius - (distance*distance - vector*vector);

    if (d >= 0.0f) collision = true;

    // Check if ray origin is inside the sphere to calculate the correct collision point
    float collision_distance = 0;

    if (distance < radius) collision_distance = vector + sqrtf(d);
    else collision_distance = vector - sqrtf(d);

    // Calculate collision point
    rf_vec3 c_point = rf_vec3_add(ray.position, rf_vec3_scale(ray.direction, collision_distance));

    collision_point->x = c_point.x;
    collision_point->y = c_point.y;
    collision_point->z = c_point.z;

    return collision;
}

// Detect collision between ray and bounding box
RF_API bool rf_check_collision_ray_box(rf_ray ray, rf_bounding_box box)
{
    bool collision = false;

    float t[8];
    t[0] = (box.min.x - ray.position.x)/ray.direction.x;
    t[1] = (box.max.x - ray.position.x)/ray.direction.x;
    t[2] = (box.min.y - ray.position.y)/ray.direction.y;
    t[3] = (box.max.y - ray.position.y)/ray.direction.y;
    t[4] = (box.min.z - ray.position.z)/ray.direction.z;
    t[5] = (box.max.z - ray.position.z)/ray.direction.z;
    t[6] = (float)fmax(fmax(fmin(t[0], t[1]), fmin(t[2], t[3])), fmin(t[4], t[5]));
    t[7] = (float)fmin(fmin(fmax(t[0], t[1]), fmax(t[2], t[3])), fmax(t[4], t[5]));

    collision = !(t[7] < 0 || t[6] > t[7]);

    return collision;
}

// Get collision info between ray and model
RF_API rf_ray_hit_info rf_get_collision_ray_model(rf_ray ray, rf_model model)
{
    rf_ray_hit_info result = { 0 };

    for (int m = 0; m < model.mesh_count; m++)
    {
        // Check if meshhas vertex data on CPU for testing
        if (model.meshes[m].vertices != NULL)
        {
            // model->mesh.triangle_count may not be set, vertex_count is more reliable
            int triangle_count = model.meshes[m].vertex_count/3;

            // Test against all triangles in mesh
            for (int i = 0; i < triangle_count; i++)
            {
                rf_vec3 a, b, c;
                rf_vec3* vertdata = (rf_vec3* )model.meshes[m].vertices;

                if (model.meshes[m].indices)
                {
                    a = vertdata[model.meshes[m].indices[i * 3 + 0]];
                    b = vertdata[model.meshes[m].indices[i * 3 + 1]];
                    c = vertdata[model.meshes[m].indices[i * 3 + 2]];
                }
                else
                {
                    a = vertdata[i * 3 + 0];
                    b = vertdata[i * 3 + 1];
                    c = vertdata[i * 3 + 2];
                }

                a = rf_vec3_transform(a, model.transform);
                b = rf_vec3_transform(b, model.transform);
                c = rf_vec3_transform(c, model.transform);

                rf_ray_hit_info tri_hit_info = rf_get_collision_ray_triangle(ray, a, b, c);

                if (tri_hit_info.hit)
                {
                    // Save the closest hit triangle
                    if ((!result.hit) || (result.distance > tri_hit_info.distance)) result = tri_hit_info;
                }
            }
        }
    }

    return result;
}

// Get collision info between ray and triangle. Note: Based on https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
RF_API rf_ray_hit_info rf_get_collision_ray_triangle(rf_ray ray, rf_vec3 p1, rf_vec3 p2, rf_vec3 p3)
{
    #define rf_epsilon 0.000001 // Just a small number

    rf_vec3 edge1, edge2;
    rf_vec3 p, q, tv;
    float det, inv_det, u, v, t;
    rf_ray_hit_info result = {0};

    // Find vectors for two edges sharing V1
    edge1 = rf_vec3_sub(p2, p1);
    edge2 = rf_vec3_sub(p3, p1);

    // Begin calculating determinant - also used to calculate u parameter
    p = rf_vec3_cross_product(ray.direction, edge2);

    // If determinant is near zero, ray lies in plane of triangle or ray is parallel to plane of triangle
    det = rf_vec3_dot_product(edge1, p);

    // Avoid culling!
    if ((det > -rf_epsilon) && (det < rf_epsilon)) return result;

    inv_det = 1.0f/det;

    // Calculate distance from V1 to ray origin
    tv = rf_vec3_sub(ray.position, p1);

    // Calculate u parameter and test bound
    u = rf_vec3_dot_product(tv, p) * inv_det;

    // The intersection lies outside of the triangle
    if ((u < 0.0f) || (u > 1.0f)) return result;

    // Prepare to test v parameter
    q = rf_vec3_cross_product(tv, edge1);

    // Calculate V parameter and test bound
    v = rf_vec3_dot_product(ray.direction, q) * inv_det;

    // The intersection lies outside of the triangle
    if ((v < 0.0f) || ((u + v) > 1.0f)) return result;

    t = rf_vec3_dot_product(edge2, q) * inv_det;

    if (t > rf_epsilon)
    {
        // rf_ray hit, get hit point and normal
        result.hit = true;
        result.distance = t;
        result.hit = true;
        result.normal = rf_vec3_normalize(rf_vec3_cross_product(edge1, edge2));
        result.position = rf_vec3_add(ray.position, rf_vec3_scale(ray.direction, t));
    }

    return result;
}

// Get collision info between ray and ground plane (Y-normal plane)
RF_API rf_ray_hit_info rf_get_collision_ray_ground(rf_ray ray, float ground_height)
{
    #define rf_epsilon 0.000001 // Just a small number

    rf_ray_hit_info result = { 0 };

    if (fabs(ray.direction.y) > rf_epsilon)
    {
        float distance = (ray.position.y - ground_height)/-ray.direction.y;

        if (distance >= 0.0)
        {
            result.hit = true;
            result.distance = distance;
            result.normal = (rf_vec3){0.0, 1.0, 0.0 };
            result.position = rf_vec3_add(ray.position, rf_vec3_scale(ray.direction, distance));
        }
    }

    return result;
}
//endregion
//endregion

//region drawing
//region internal functions

// Get texture to draw shapes Note(LucaSas): Do we need this?
RF_INTERNAL rf_texture2d rf_internal_get_shapes_texture()
{
    if (rf_internal_ctx->tex_shapes.id == 0)
    {
        rf_internal_ctx->tex_shapes = rf_get_default_texture(); // Use default white texture
        rf_internal_ctx->rec_tex_shapes = (rf_rec) {0.0f, 0.0f, 1.0f, 1.0f };
    }

    return rf_internal_ctx->tex_shapes;
}

// Cubic easing in-out. Note: Required for rf_draw_line_bezier()
RF_INTERNAL float rf_internal_shapes_ease_cubic_in_out(float t, float b, float c, float d)
{
    if ((t /= 0.5f*d) < 1) return 0.5f*c*t*t*t + b;

    t -= 2;

    return 0.5f*c*(t*t*t + 2.0f) + b;
}
//endregion

// Set background color (framebuffer clear color)
RF_API void rf_clear(rf_color color)
{
    rf_gfx_clear_color(color.r, color.g, color.b, color.a); // Set clear color
    rf_gfx_clear_screen_buffers(); // Clear current framebuffers
}

// Setup canvas (framebuffer) to start drawing
RF_API void rf_begin()
{
    rf_internal_ctx->current_time = rf_get_time(); // Number of elapsed seconds
    rf_internal_ctx->update_time = rf_internal_ctx->current_time - rf_internal_ctx->previous_time;
    rf_internal_ctx->previous_time = rf_internal_ctx->current_time;

    rf_gfx_load_identity(); // Reset current matrix (MODELVIEW)
    rf_gfx_mult_matrixf(rf_mat_to_float16(rf_internal_ctx->screen_scaling).v); // Apply screen scaling

    //rf_gfx_translatef(0.375, 0.375, 0);    // HACK to have 2D pixel-perfect drawing on OpenGL 1.1
    // NOTE: Not required with OpenGL 3.3+
}

// End canvas drawing and swap buffers (double buffering)
RF_API void rf_end()
{
    rf_gfx_draw();

    // Frame time control system
    rf_internal_ctx->current_time = rf_get_time();
    rf_internal_ctx->draw_time = rf_internal_ctx->current_time - rf_internal_ctx->previous_time;
    rf_internal_ctx->previous_time = rf_internal_ctx->current_time;

    rf_internal_ctx->frame_time = rf_internal_ctx->update_time + rf_internal_ctx->draw_time;

    // rf_wait for some milliseconds...
    if (rf_internal_ctx->target_time != ((double) RF_UNLOCKED_FPS) && rf_internal_ctx->frame_time < rf_internal_ctx->target_time)
    {
        rf_wait((float)(rf_internal_ctx->target_time - rf_internal_ctx->frame_time)*1000.0f);

        rf_internal_ctx->current_time = rf_get_time();
        double extraTime = rf_internal_ctx->current_time - rf_internal_ctx->previous_time;
        rf_internal_ctx->previous_time = rf_internal_ctx->current_time;

        rf_internal_ctx->frame_time += extraTime;
    }

    return;
}

// Initialize 2D mode with custom camera (2D)
RF_API void rf_begin_2d(rf_camera2d camera)
{
    rf_gfx_draw();

    rf_gfx_load_identity(); // Reset current matrix (MODELVIEW)

    // Apply screen scaling if required
    rf_gfx_mult_matrixf(rf_mat_to_float16(rf_internal_ctx->screen_scaling).v);

    // Apply 2d camera transformation to rf_gfxobal_model_view
    rf_gfx_mult_matrixf(rf_mat_to_float16(rf_get_camera_matrix2d(camera)).v);
}

// Ends 2D mode with custom camera
RF_API void rf_end_2d()
{
    rf_gfx_draw();

    rf_gfx_load_identity(); // Reset current matrix (MODELVIEW)
    rf_gfx_mult_matrixf(rf_mat_to_float16(rf_internal_ctx->screen_scaling).v); // Apply screen scaling if required
}

// Initializes 3D mode with custom camera (3D)
RF_API void rf_begin_3d(rf_camera3d camera)
{
    rf_gfx_draw();

    rf_gfx_matrix_mode(RF_PROJECTION); // Switch to GL_PROJECTION matrix
    rf_gfx_push_matrix(); // Save previous matrix, which contains the settings for the 2d ortho GL_PROJECTION
    rf_gfx_load_identity(); // Reset current matrix (PROJECTION)

    float aspect = (float) rf_internal_ctx->current_width / (float)rf_internal_ctx->current_height;

    if (camera.type == RF_CAMERA_PERSPECTIVE)
    {
        // Setup perspective GL_PROJECTION
        double top = 0.01 * tan(camera.fovy*0.5*RF_DEG2RAD);
        double right = top*aspect;

        rf_gfx_frustum(-right, right, -top, top, 0.01, 1000.0);
    }
    else if (camera.type == RF_CAMERA_ORTHOGRAPHIC)
    {
        // Setup orthographic GL_PROJECTION
        double top = camera.fovy/2.0;
        double right = top*aspect;

        rf_gfx_ortho(-right,right,-top,top, 0.01, 1000.0);
    }

    // NOTE: zNear and zFar values are important when computing depth buffer values

    rf_gfx_matrix_mode(RF_MODELVIEW); // Switch back to rf_gfxobal_model_view matrix
    rf_gfx_load_identity(); // Reset current matrix (MODELVIEW)

    // Setup rf_camera3d view
    rf_mat mat_view = rf_mat_look_at(camera.position, camera.target, camera.up);
    rf_gfx_mult_matrixf(rf_mat_to_float16(mat_view).v); // Multiply MODELVIEW matrix by view matrix (camera)

    rf_gfx_enable_depth_test(); // Enable DEPTH_TEST for 3D
}

// Ends 3D mode and returns to default 2D orthographic mode
RF_API void rf_end_3d()
{
    rf_gfx_draw(); // Process internal buffers (update + draw)

    rf_gfx_matrix_mode(RF_PROJECTION); // Switch to GL_PROJECTION matrix
    rf_gfx_pop_matrix(); // Restore previous matrix (PROJECTION) from matrix rf_gfxobal_gl_stack

    rf_gfx_matrix_mode(RF_MODELVIEW); // Get back to rf_gfxobal_model_view matrix
    rf_gfx_load_identity(); // Reset current matrix (MODELVIEW)

    rf_gfx_mult_matrixf(rf_mat_to_float16(rf_internal_ctx->screen_scaling).v); // Apply screen scaling if required

    rf_gfx_disable_depth_test(); // Disable DEPTH_TEST for 2D
}

// Initializes render texture for drawing
RF_API void rf_begin_render_to_texture(rf_render_texture2d target)
{
    rf_gfx_draw();

    rf_gfx_enable_render_texture(target.id); // Enable render target

    // Set viewport to framebuffer size
    rf_gfx_viewport(0, 0, target.texture.width, target.texture.height);

    rf_gfx_matrix_mode(RF_PROJECTION); // Switch to PROJECTION matrix
    rf_gfx_load_identity(); // Reset current matrix (PROJECTION)

    // Set orthographic GL_PROJECTION to current framebuffer size
    // NOTE: Configured top-left corner as (0, 0)
    rf_gfx_ortho(0, target.texture.width, target.texture.height, 0, 0.0f, 1.0f);

    rf_gfx_matrix_mode(RF_MODELVIEW); // Switch back to MODELVIEW matrix
    rf_gfx_load_identity(); // Reset current matrix (MODELVIEW)

    //rf_gfx_scalef(0.0f, -1.0f, 0.0f);      // Flip Y-drawing (?)

    // Setup current width/height for proper aspect ratio
    // calculation when using rf_begin_mode3d()
    rf_internal_ctx->current_width = target.texture.width;
    rf_internal_ctx->current_height = target.texture.height;
}

// Ends drawing to render texture
RF_API void rf_end_render_to_texture()
{
    rf_gfx_draw();

    rf_gfx_disable_render_texture(); // Disable render target

    // Set viewport to default framebuffer size
    rf_set_viewport(rf_internal_ctx->render_width, rf_internal_ctx->render_height);

    // Reset current screen size
    rf_internal_ctx->current_width = rf_internal_ctx->screen_width;
    rf_internal_ctx->current_height = rf_internal_ctx->screen_height;
}

// Begin scissor mode (define screen area for following drawing)
// NOTE: Scissor rec refers to bottom-left corner, we change it to upper-left
RF_API void rf_begin_scissors(int x, int y, int width, int height)
{
    rf_gfx_draw(); // Force drawing elements

    rf_gfx_enable_scissor_test();
    rf_gfx_scissor(x, rf_internal_ctx->screen_height - (y + height), width, height);
}

// End scissor mode
RF_API void rf_end_scissors()
{
    rf_gfx_draw(); // Force drawing elements
    rf_gfx_disable_scissor_test();
}

// Begin custom shader mode
RF_API void rf_begin_shader(rf_shader shader)
{
    if (rf_internal_ctx->gfx_ctx.current_shader.id != shader.id)
    {
        rf_gfx_draw();
        rf_internal_ctx->gfx_ctx.current_shader = shader;
    }
}

// End custom shader mode (returns to default shader)
RF_API void rf_end_shader()
{
    rf_begin_shader(rf_internal_ctx->gfx_ctx.default_shader);
}

// Begin blending mode (alpha, additive, multiplied). Default blend mode is alpha
RF_API void rf_begin_blend_mode(rf_blend_mode mode)
{
    rf_gfx_blend_mode(mode);
}

// End blending mode (reset to default: alpha blending)
RF_API void rf_end_blend_mode()
{
    rf_gfx_blend_mode(RF_BLEND_ALPHA);
}

// Update camera depending on selected mode
// NOTE: rf_camera3d controls depend on some raylib functions:
//       System: EnableCursor(), DisableCursor()
//       Mouse: IsMouseButtonDown(), GetMousePosition(), GetMouseWheelMove()
//       Keys:  IsKeyDown()
// TODO: Port to quaternion-based camera
RF_API void rf_update_camera3d(rf_camera3d* camera, rf_camera3d_mode mode, rf_input_state_for_update_camera input_state)
{
    // rf_camera3d mouse movement sensitivity
    #define rf_camera_mouse_move_sensitivity 0.003f
    #define rf_camera_mouse_scroll_sensitivity 1.5f

    // FREE_CAMERA
    #define rf_camera_free_mouse_sensitivity 0.01f
    #define rf_camera_free_distance_min_clamp 0.3f
    #define rf_camera_free_distance_max_clamp 120.0f
    #define rf_camera_free_min_clamp 85.0f
    #define rf_camera_free_max_clamp -85.0f
    #define rf_camera_free_smooth_zoom_sensitivity 0.05f
    #define rf_camera_free_panning_divider 5.1f

    // ORBITAL_CAMERA
    #define rf_camera_orbital_speed 0.01f // Radians per frame

    // FIRST_PERSON
    //#define CAMERA_FIRST_PERSON_MOUSE_SENSITIVITY           0.003f
    #define rf_camera_first_person_focus_distance 25.0f
    #define rf_camera_first_person_min_clamp 85.0f
    #define rf_camera_first_person_max_clamp -85.0f

    #define rf_camera_first_person_step_trigonometric_divider 5.0f
    #define rf_camera_first_person_step_divider 30.0f
    #define rf_camera_first_person_waving_divider 200.0f

    // THIRD_PERSON
    //#define CAMERA_THIRD_PERSON_MOUSE_SENSITIVITY           0.003f
    #define rf_camera_third_person_distance_clamp 1.2f
    #define rf_camera_third_person_min_clamp 5.0f
    #define rf_camera_third_person_max_clamp -85.0f
    #define rf_camera_third_person_offset (rf_vector3){ 0.4f, 0.0f, 0.0f }

    // PLAYER (used by camera)
    #define rf_player_movement_sensitivity 20.0f

    // rf_camera3d move modes (first person and third person cameras)
    typedef enum rf_camera_move
    {
        rf_move_front = 0,
        rf_move_back,
        rf_move_right,
        rf_move_left,
        rf_move_up,
        rf_move_down
    } rf_camera_move;

    RF_INTERNAL float player_eyes_position = 1.85f;

    RF_INTERNAL int swing_counter = 0; // Used for 1st person swinging movement
    RF_INTERNAL rf_vec2 previous_mouse_position = {0.0f, 0.0f };
    // TODO: CRF_INTERNAL rf_internal_ctx->gl_ctx.camera_target_distance and rf_internal_ctx->gl_ctx.camera_angle here

    // Mouse movement detection
    rf_vec2 mouse_position_delta = {0.0f, 0.0f };
    rf_vec2 mouse_position = input_state.mouse_position;
    int mouse_wheel_move = input_state.mouse_wheel_move;

    // Keys input detection
    bool pan_key = input_state.is_camera_pan_control_key_down;
    bool alt_key = input_state.is_camera_alt_control_key_down;
    bool szoom_key = input_state.is_camera_smooth_zoom_control_key;

    bool direction[6];
    direction[0] = input_state.direction_keys[0];
    direction[1] = input_state.direction_keys[1];
    direction[2] = input_state.direction_keys[2];
    direction[3] = input_state.direction_keys[3];
    direction[4] = input_state.direction_keys[4];
    direction[5] = input_state.direction_keys[5];

    // TODO: Consider touch inputs for camera

    if (rf_internal_ctx->camera_mode != RF_CAMERA_CUSTOM)
    {
        mouse_position_delta.x = mouse_position.x - previous_mouse_position.x;
        mouse_position_delta.y = mouse_position.y - previous_mouse_position.y;

        previous_mouse_position = mouse_position;
    }

    // Support for multiple automatic camera modes
    switch (rf_internal_ctx->camera_mode)
    {
        case RF_CAMERA_FREE:
        {
            // rf_camera3d zoom
            if ((rf_internal_ctx->camera_target_distance < rf_camera_free_distance_max_clamp) && (mouse_wheel_move < 0))
            {
                rf_internal_ctx->camera_target_distance -= (mouse_wheel_move*rf_camera_mouse_scroll_sensitivity);

                if (rf_internal_ctx->camera_target_distance > rf_camera_free_distance_max_clamp) rf_internal_ctx->camera_target_distance = rf_camera_free_distance_max_clamp;
            }
                // rf_camera3d looking down
                // TODO: Review, weird comparisson of rf_internal_ctx->gl_ctx.camera_target_distance == 120.0f?
            else if ((camera->position.y > camera->target.y) && (rf_internal_ctx->camera_target_distance == rf_camera_free_distance_max_clamp) && (mouse_wheel_move < 0))
            {
                camera->target.x += mouse_wheel_move*(camera->target.x - camera->position.x)*rf_camera_mouse_scroll_sensitivity/rf_internal_ctx->camera_target_distance;
                camera->target.y += mouse_wheel_move*(camera->target.y - camera->position.y)*rf_camera_mouse_scroll_sensitivity/rf_internal_ctx->camera_target_distance;
                camera->target.z += mouse_wheel_move*(camera->target.z - camera->position.z)*rf_camera_mouse_scroll_sensitivity/rf_internal_ctx->camera_target_distance;
            }
            else if ((camera->position.y > camera->target.y) && (camera->target.y >= 0))
            {
                camera->target.x += mouse_wheel_move*(camera->target.x - camera->position.x)*rf_camera_mouse_scroll_sensitivity/rf_internal_ctx->camera_target_distance;
                camera->target.y += mouse_wheel_move*(camera->target.y - camera->position.y)*rf_camera_mouse_scroll_sensitivity/rf_internal_ctx->camera_target_distance;
                camera->target.z += mouse_wheel_move*(camera->target.z - camera->position.z)*rf_camera_mouse_scroll_sensitivity/rf_internal_ctx->camera_target_distance;

                // if (camera->target.y < 0) camera->target.y = -0.001;
            }
            else if ((camera->position.y > camera->target.y) && (camera->target.y < 0) && (mouse_wheel_move > 0))
            {
                rf_internal_ctx->camera_target_distance -= (mouse_wheel_move*rf_camera_mouse_scroll_sensitivity);
                if (rf_internal_ctx->camera_target_distance < rf_camera_free_distance_min_clamp) rf_internal_ctx->camera_target_distance = rf_camera_free_distance_min_clamp;
            }
                // rf_camera3d looking up
                // TODO: Review, weird comparisson of rf_internal_ctx->gl_ctx.camera_target_distance == 120.0f?
            else if ((camera->position.y < camera->target.y) && (rf_internal_ctx->camera_target_distance == rf_camera_free_distance_max_clamp) && (mouse_wheel_move < 0))
            {
                camera->target.x += mouse_wheel_move*(camera->target.x - camera->position.x)*rf_camera_mouse_scroll_sensitivity/rf_internal_ctx->camera_target_distance;
                camera->target.y += mouse_wheel_move*(camera->target.y - camera->position.y)*rf_camera_mouse_scroll_sensitivity/rf_internal_ctx->camera_target_distance;
                camera->target.z += mouse_wheel_move*(camera->target.z - camera->position.z)*rf_camera_mouse_scroll_sensitivity/rf_internal_ctx->camera_target_distance;
            }
            else if ((camera->position.y < camera->target.y) && (camera->target.y <= 0))
            {
                camera->target.x += mouse_wheel_move*(camera->target.x - camera->position.x)*rf_camera_mouse_scroll_sensitivity/rf_internal_ctx->camera_target_distance;
                camera->target.y += mouse_wheel_move*(camera->target.y - camera->position.y)*rf_camera_mouse_scroll_sensitivity/rf_internal_ctx->camera_target_distance;
                camera->target.z += mouse_wheel_move*(camera->target.z - camera->position.z)*rf_camera_mouse_scroll_sensitivity/rf_internal_ctx->camera_target_distance;

                // if (camera->target.y > 0) camera->target.y = 0.001;
            }
            else if ((camera->position.y < camera->target.y) && (camera->target.y > 0) && (mouse_wheel_move > 0))
            {
                rf_internal_ctx->camera_target_distance -= (mouse_wheel_move*rf_camera_mouse_scroll_sensitivity);
                if (rf_internal_ctx->camera_target_distance < rf_camera_free_distance_min_clamp) rf_internal_ctx->camera_target_distance = rf_camera_free_distance_min_clamp;
            }

            // Input keys checks
            if (pan_key)
            {
                if (alt_key) // Alternative key behaviour
                {
                    if (szoom_key)
                    {
                        // rf_camera3d smooth zoom
                        rf_internal_ctx->camera_target_distance += (mouse_position_delta.y*rf_camera_free_smooth_zoom_sensitivity);
                    }
                    else
                    {
                        // rf_camera3d rotation
                        rf_internal_ctx->camera_angle.x += mouse_position_delta.x*-rf_camera_free_mouse_sensitivity;
                        rf_internal_ctx->camera_angle.y += mouse_position_delta.y*-rf_camera_free_mouse_sensitivity;

                        // Angle clamp
                        if (rf_internal_ctx->camera_angle.y > rf_camera_free_min_clamp*RF_DEG2RAD) rf_internal_ctx->camera_angle.y = rf_camera_free_min_clamp*RF_DEG2RAD;
                        else if (rf_internal_ctx->camera_angle.y < rf_camera_free_max_clamp*RF_DEG2RAD) rf_internal_ctx->camera_angle.y = rf_camera_free_max_clamp*RF_DEG2RAD;
                    }
                }
                else
                {
                    // rf_camera3d panning
                    camera->target.x += ((mouse_position_delta.x*-rf_camera_free_mouse_sensitivity)*cosf(rf_internal_ctx->camera_angle.x) + (mouse_position_delta.y*rf_camera_free_mouse_sensitivity)*sinf(rf_internal_ctx->camera_angle.x)*sinf(rf_internal_ctx->camera_angle.y))*(rf_internal_ctx->camera_target_distance/rf_camera_free_panning_divider);
                    camera->target.y += ((mouse_position_delta.y*rf_camera_free_mouse_sensitivity)*cosf(rf_internal_ctx->camera_angle.y))*(rf_internal_ctx->camera_target_distance/rf_camera_free_panning_divider);
                    camera->target.z += ((mouse_position_delta.x*rf_camera_free_mouse_sensitivity)*sinf(rf_internal_ctx->camera_angle.x) + (mouse_position_delta.y*rf_camera_free_mouse_sensitivity)*cosf(rf_internal_ctx->camera_angle.x)*sinf(rf_internal_ctx->camera_angle.y))*(rf_internal_ctx->camera_target_distance/rf_camera_free_panning_divider);
                }
            }

            // Update camera position with changes
            camera->position.x = sinf(rf_internal_ctx->camera_angle.x)*rf_internal_ctx->camera_target_distance*cosf(rf_internal_ctx->camera_angle.y) + camera->target.x;
            camera->position.y = ((rf_internal_ctx->camera_angle.y <= 0.0f)? 1 : -1)*sinf(rf_internal_ctx->camera_angle.y)*rf_internal_ctx->camera_target_distance*sinf(rf_internal_ctx->camera_angle.y) + camera->target.y;
            camera->position.z = cosf(rf_internal_ctx->camera_angle.x)*rf_internal_ctx->camera_target_distance*cosf(rf_internal_ctx->camera_angle.y) + camera->target.z;

        } break;
        case RF_CAMERA_ORBITAL:
        {
            rf_internal_ctx->camera_angle.x += rf_camera_orbital_speed; // rf_camera3d orbit angle
            rf_internal_ctx->camera_target_distance -= (mouse_wheel_move*rf_camera_mouse_scroll_sensitivity); // rf_camera3d zoom

            // rf_camera3d distance clamp
            if (rf_internal_ctx->camera_target_distance < rf_camera_third_person_distance_clamp) rf_internal_ctx->camera_target_distance = rf_camera_third_person_distance_clamp;

            // Update camera position with changes
            camera->position.x = sinf(rf_internal_ctx->camera_angle.x)*rf_internal_ctx->camera_target_distance*cosf(rf_internal_ctx->camera_angle.y) + camera->target.x;
            camera->position.y = ((rf_internal_ctx->camera_angle.y <= 0.0f)? 1 : -1)*sinf(rf_internal_ctx->camera_angle.y)*rf_internal_ctx->camera_target_distance*sinf(rf_internal_ctx->camera_angle.y) + camera->target.y;
            camera->position.z = cosf(rf_internal_ctx->camera_angle.x)*rf_internal_ctx->camera_target_distance*cosf(rf_internal_ctx->camera_angle.y) + camera->target.z;

        } break;
        case RF_CAMERA_FIRST_PERSON:
        {
            camera->position.x += (sinf(rf_internal_ctx->camera_angle.x)*direction[rf_move_back] -
                                   sinf(rf_internal_ctx->camera_angle.x)*direction[rf_move_front] -
                                   cosf(rf_internal_ctx->camera_angle.x)*direction[rf_move_left] +
                                   cosf(rf_internal_ctx->camera_angle.x)*direction[rf_move_right])/rf_player_movement_sensitivity;

            camera->position.y += (sinf(rf_internal_ctx->camera_angle.y)*direction[rf_move_front] -
                                   sinf(rf_internal_ctx->camera_angle.y)*direction[rf_move_back] +
                                   1.0f*direction[rf_move_up] - 1.0f*direction[rf_move_down])/rf_player_movement_sensitivity;

            camera->position.z += (cosf(rf_internal_ctx->camera_angle.x)*direction[rf_move_back] -
                                   cosf(rf_internal_ctx->camera_angle.x)*direction[rf_move_front] +
                                   sinf(rf_internal_ctx->camera_angle.x)*direction[rf_move_left] -
                                   sinf(rf_internal_ctx->camera_angle.x)*direction[rf_move_right])/rf_player_movement_sensitivity;

            bool is_moving = false; // Required for swinging

            for (int i = 0; i < 6; i++) if (direction[i]) { is_moving = true; break; }

            // rf_camera3d orientation calculation
            rf_internal_ctx->camera_angle.x += (mouse_position_delta.x*-rf_camera_mouse_move_sensitivity);
            rf_internal_ctx->camera_angle.y += (mouse_position_delta.y*-rf_camera_mouse_move_sensitivity);

            // Angle clamp
            if (rf_internal_ctx->camera_angle.y > rf_camera_first_person_min_clamp*RF_DEG2RAD) rf_internal_ctx->camera_angle.y = rf_camera_first_person_min_clamp*RF_DEG2RAD;
            else if (rf_internal_ctx->camera_angle.y < rf_camera_first_person_max_clamp*RF_DEG2RAD) rf_internal_ctx->camera_angle.y = rf_camera_first_person_max_clamp*RF_DEG2RAD;

            // rf_camera3d is always looking at player
            camera->target.x = camera->position.x - sinf(rf_internal_ctx->camera_angle.x)*rf_camera_first_person_focus_distance;
            camera->target.y = camera->position.y + sinf(rf_internal_ctx->camera_angle.y)*rf_camera_first_person_focus_distance;
            camera->target.z = camera->position.z - cosf(rf_internal_ctx->camera_angle.x)*rf_camera_first_person_focus_distance;

            if (is_moving) swing_counter++;

            // rf_camera3d position update
            // NOTE: On RF_CAMERA_FIRST_PERSON player Y-movement is limited to player 'eyes position'
            camera->position.y = player_eyes_position - sinf(swing_counter/rf_camera_first_person_step_trigonometric_divider)/rf_camera_first_person_step_divider;

            camera->up.x = sinf(swing_counter/(rf_camera_first_person_step_trigonometric_divider * 2))/rf_camera_first_person_waving_divider;
            camera->up.z = -sinf(swing_counter/(rf_camera_first_person_step_trigonometric_divider * 2))/rf_camera_first_person_waving_divider;


        } break;
        case RF_CAMERA_THIRD_PERSON:
        {
            camera->position.x += (sinf(rf_internal_ctx->camera_angle.x)*direction[rf_move_back] -
                                   sinf(rf_internal_ctx->camera_angle.x)*direction[rf_move_front] -
                                   cosf(rf_internal_ctx->camera_angle.x)*direction[rf_move_left] +
                                   cosf(rf_internal_ctx->camera_angle.x)*direction[rf_move_right])/rf_player_movement_sensitivity;

            camera->position.y += (sinf(rf_internal_ctx->camera_angle.y)*direction[rf_move_front] -
                                   sinf(rf_internal_ctx->camera_angle.y)*direction[rf_move_back] +
                                   1.0f*direction[rf_move_up] - 1.0f*direction[rf_move_down])/rf_player_movement_sensitivity;

            camera->position.z += (cosf(rf_internal_ctx->camera_angle.x)*direction[rf_move_back] -
                                   cosf(rf_internal_ctx->camera_angle.x)*direction[rf_move_front] +
                                   sinf(rf_internal_ctx->camera_angle.x)*direction[rf_move_left] -
                                   sinf(rf_internal_ctx->camera_angle.x)*direction[rf_move_right])/rf_player_movement_sensitivity;

            // rf_camera3d orientation calculation
            rf_internal_ctx->camera_angle.x += (mouse_position_delta.x*-rf_camera_mouse_move_sensitivity);
            rf_internal_ctx->camera_angle.y += (mouse_position_delta.y*-rf_camera_mouse_move_sensitivity);

            // Angle clamp
            if (rf_internal_ctx->camera_angle.y > rf_camera_third_person_min_clamp*RF_DEG2RAD) rf_internal_ctx->camera_angle.y = rf_camera_third_person_min_clamp*RF_DEG2RAD;
            else if (rf_internal_ctx->camera_angle.y < rf_camera_third_person_max_clamp*RF_DEG2RAD) rf_internal_ctx->camera_angle.y = rf_camera_third_person_max_clamp*RF_DEG2RAD;

            // rf_camera3d zoom
            rf_internal_ctx->camera_target_distance -= (mouse_wheel_move*rf_camera_mouse_scroll_sensitivity);

            // rf_camera3d distance clamp
            if (rf_internal_ctx->camera_target_distance < rf_camera_third_person_distance_clamp) rf_internal_ctx->camera_target_distance = rf_camera_third_person_distance_clamp;

            // TODO: It seems camera->position is not correctly updated or some rounding issue makes the camera move straight to camera->target...
            camera->position.x = sinf(rf_internal_ctx->camera_angle.x)*rf_internal_ctx->camera_target_distance*cosf(rf_internal_ctx->camera_angle.y) + camera->target.x;
            if (rf_internal_ctx->camera_angle.y <= 0.0f) camera->position.y = sinf(rf_internal_ctx->camera_angle.y)*rf_internal_ctx->camera_target_distance*sinf(rf_internal_ctx->camera_angle.y) + camera->target.y;
            else camera->position.y = -sinf(rf_internal_ctx->camera_angle.y)*rf_internal_ctx->camera_target_distance*sinf(rf_internal_ctx->camera_angle.y) + camera->target.y;
            camera->position.z = cosf(rf_internal_ctx->camera_angle.x)*rf_internal_ctx->camera_target_distance*cosf(rf_internal_ctx->camera_angle.y) + camera->target.z;

        } break;
        default: break;
    }
}

// Draw a pixel
RF_API void rf_draw_pixel(int pos_x, int pos_y, rf_color color)
{
    rf_gfx_begin(RF_LINES);
    rf_gfx_color4ub(color.r, color.g, color.b, color.a);
    rf_gfx_vertex2i(pos_x, pos_y);
    rf_gfx_vertex2i(pos_x + 1, pos_y + 1);
    rf_gfx_end();
}

// Draw a pixel (Vector version)
RF_API void rf_draw_pixel_v(rf_vec2 position, rf_color color)
{
    rf_draw_pixel(position.x, position.y, color);
}

// Draw a line
RF_API void rf_draw_line(int startPosX, int startPosY, int endPosX, int endPosY, rf_color color)
{
    rf_gfx_begin(RF_LINES);
    rf_gfx_color4ub(color.r, color.g, color.b, color.a);
    rf_gfx_vertex2i(startPosX, startPosY);
    rf_gfx_vertex2i(endPosX, endPosY);
    rf_gfx_end();
}

// Draw a line (Vector version)
RF_API void rf_draw_line_v(rf_vec2 startPos, rf_vec2 endPos, rf_color color)
{
    rf_draw_line(startPos.x, startPos.y, endPos.x, endPos.y, color);
}

// Draw a line defining thickness
RF_API void rf_draw_line_ex(rf_vec2 start_pos, rf_vec2 end_pos, float thick, rf_color color)
{
    if (start_pos.x > end_pos.x)
    {
        rf_vec2 temp_pos = start_pos;
        start_pos = end_pos;
        end_pos = temp_pos;
    }

    float dx = end_pos.x - start_pos.x;
    float dy = end_pos.y - start_pos.y;

    float d = sqrtf(dx*dx + dy*dy);
    float angle = asinf(dy/d);

    rf_gfx_enable_texture(rf_internal_get_shapes_texture().id);

    rf_gfx_push_matrix();
    rf_gfx_translatef((float)start_pos.x, (float)start_pos.y, 0.0f);
    rf_gfx_rotatef(RF_RAD2DEG * angle, 0.0f, 0.0f, 1.0f);
    rf_gfx_translatef(0, (thick > 1.0f)? -thick/2.0f : -1.0f, 0.0f);

    rf_gfx_begin(RF_QUADS);
    rf_gfx_color4ub(color.r, color.g, color.b, color.a);
    rf_gfx_normal3f(0.0f, 0.0f, 1.0f);

    rf_gfx_tex_coord2f(rf_internal_ctx->rec_tex_shapes.x/rf_internal_ctx->tex_shapes.width, rf_internal_ctx->rec_tex_shapes.y/rf_internal_ctx->tex_shapes.height);
    rf_gfx_vertex2f(0.0f, 0.0f);

    rf_gfx_tex_coord2f(rf_internal_ctx->rec_tex_shapes.x/rf_internal_ctx->tex_shapes.width, (rf_internal_ctx->rec_tex_shapes.y + rf_internal_ctx->rec_tex_shapes.height)/rf_internal_ctx->tex_shapes.height);
    rf_gfx_vertex2f(0.0f, thick);

    rf_gfx_tex_coord2f((rf_internal_ctx->rec_tex_shapes.x + rf_internal_ctx->rec_tex_shapes.width)/rf_internal_ctx->tex_shapes.width, (rf_internal_ctx->rec_tex_shapes.y + rf_internal_ctx->rec_tex_shapes.height)/rf_internal_ctx->tex_shapes.height);
    rf_gfx_vertex2f(d, thick);

    rf_gfx_tex_coord2f((rf_internal_ctx->rec_tex_shapes.x + rf_internal_ctx->rec_tex_shapes.width)/rf_internal_ctx->tex_shapes.width, rf_internal_ctx->rec_tex_shapes.y/rf_internal_ctx->tex_shapes.height);
    rf_gfx_vertex2f(d, 0.0f);
    rf_gfx_end();
    rf_gfx_pop_matrix();

    rf_gfx_disable_texture();
}

// Draw line using cubic-bezier curves in-out
RF_API void rf_draw_line_bezier(rf_vec2 start_pos, rf_vec2 end_pos, float thick, rf_color color)
{
#define RF_LINE_DIVISIONS 24 // Bezier line divisions

    rf_vec2 previous = start_pos;
    rf_vec2 current;

    for (int i = 1; i <= RF_LINE_DIVISIONS; i++)
    {
        // Cubic easing in-out
        // NOTE: Easing is calculated only for y position value
        current.y = rf_internal_shapes_ease_cubic_in_out((float)i, start_pos.y, end_pos.y - start_pos.y, (float)RF_LINE_DIVISIONS);
        current.x = previous.x + (end_pos.x - start_pos.x)/ (float)RF_LINE_DIVISIONS;

        rf_draw_line_ex(previous, current, thick, color);

        previous = current;
    }

#undef RF_LINE_DIVISIONS
}

// Draw lines sequence
RF_API void rf_draw_line_strip(rf_vec2 *points, int points_count, rf_color color)
{
    if (points_count >= 2)
    {
        if (rf_gfx_check_buffer_limit(points_count)) rf_gfx_draw();

        rf_gfx_begin(RF_LINES);
        rf_gfx_color4ub(color.r, color.g, color.b, color.a);

        for (int i = 0; i < points_count - 1; i++)
        {
            rf_gfx_vertex2f(points[i].x, points[i].y);
            rf_gfx_vertex2f(points[i + 1].x, points[i + 1].y);
        }
        rf_gfx_end();
    }
}

// Draw a color-filled circle
RF_API void rf_draw_circle(int center_x, int center_y, float radius, rf_color color)
{
    rf_draw_circle_sector((rf_vec2) {center_x, center_y }, radius, 0, 360, 36, color);
}

// Draw a color-filled circle (Vector version)
RF_API void rf_draw_circle_v(rf_vec2 center, float radius, rf_color color)
{
    rf_draw_circle(center.x, center.y, radius, color);
}

// Draw a piece of a circle
RF_API void rf_draw_circle_sector(rf_vec2 center, float radius, int start_angle, int end_angle, int segments, rf_color color)
{
    if (radius <= 0.0f) radius = 0.1f; // Avoid div by zero

    // Function expects (endAngle > start_angle)
    if (end_angle < start_angle)
    {
        // Swap values
        int tmp = start_angle;
        start_angle = end_angle;
        end_angle = tmp;
    }

    if (segments < 4)
    {
        // Calculate how many segments we need to draw a smooth circle, taken from https://stackoverflow.com/a/2244088
        float CIRCLE_ERROR_RATE = 0.5f;

        // Calculate the maximum angle between segments based on the error rate.
        float th = acosf(2*powf(1 - CIRCLE_ERROR_RATE/radius, 2) - 1);
        segments = (end_angle - start_angle) * ceilf(2 * RF_PI / th) / 360;

        if (segments <= 0) segments = 4;
    }

    float step_length = (float)(end_angle - start_angle)/(float)segments;
    float angle = start_angle;
    if (rf_gfx_check_buffer_limit(3*segments)) rf_gfx_draw();

    rf_gfx_begin(RF_TRIANGLES);
    for (int i = 0; i < segments; i++)
    {
        rf_gfx_color4ub(color.r, color.g, color.b, color.a);

        rf_gfx_vertex2f(center.x, center.y);
        rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*radius, center.y + cosf(RF_DEG2RAD*angle)*radius);
        rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + step_length))*radius, center.y + cosf(RF_DEG2RAD*(angle + step_length))*radius);

        angle += step_length;
    }
    rf_gfx_end();
}

RF_API void rf_draw_circle_sector_lines(rf_vec2 center, float radius, int start_angle, int end_angle, int segments, rf_color color)
{
    if (radius <= 0.0f) radius = 0.1f; // Avoid div by zero issue

    // Function expects (endAngle > start_angle)
    if (end_angle < start_angle)
    {
        // Swap values
        int tmp = start_angle;
        start_angle = end_angle;
        end_angle = tmp;
    }

    if (segments < 4)
    {
        // Calculate how many segments we need to draw a smooth circle, taken from https://stackoverflow.com/a/2244088

        float CIRCLE_ERROR_RATE = 0.5f;


        // Calculate the maximum angle between segments based on the error rate.
        float th = acosf(2*powf(1 - CIRCLE_ERROR_RATE/radius, 2) - 1);
        segments = (end_angle - start_angle) * ceilf(2 * RF_PI / th) / 360;

        if (segments <= 0) segments = 4;
    }

    float step_length = (float)(end_angle - start_angle)/(float)segments;
    float angle = start_angle;

    // Hide the cap lines when the circle is full
    bool show_cap_lines = true;
    int limit = 2*(segments + 2);
    if ((end_angle - start_angle)%360 == 0) { limit = 2*segments; show_cap_lines = false; }

    if (rf_gfx_check_buffer_limit(limit)) rf_gfx_draw();

    rf_gfx_begin(RF_LINES);
    if (show_cap_lines)
    {
        rf_gfx_color4ub(color.r, color.g, color.b, color.a);
        rf_gfx_vertex2f(center.x, center.y);
        rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*radius, center.y + cosf(RF_DEG2RAD*angle)*radius);
    }

    for (int i = 0; i < segments; i++)
    {
        rf_gfx_color4ub(color.r, color.g, color.b, color.a);

        rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*radius, center.y + cosf(RF_DEG2RAD*angle)*radius);
        rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + step_length))*radius, center.y + cosf(RF_DEG2RAD*(angle + step_length))*radius);

        angle += step_length;
    }

    if (show_cap_lines)
    {
        rf_gfx_color4ub(color.r, color.g, color.b, color.a);
        rf_gfx_vertex2f(center.x, center.y);
        rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*radius, center.y + cosf(RF_DEG2RAD*angle)*radius);
    }
    rf_gfx_end();
}

// Draw a gradient-filled circle
// NOTE: Gradient goes from center (color1) to border (color2)
RF_API void rf_draw_circle_gradient(int center_x, int center_y, float radius, rf_color color1, rf_color color2)
{
    if (rf_gfx_check_buffer_limit(3 * 36)) rf_gfx_draw();

    rf_gfx_begin(RF_TRIANGLES);
    for (int i = 0; i < 360; i += 10)
    {
        rf_gfx_color4ub(color1.r, color1.g, color1.b, color1.a);
        rf_gfx_vertex2f(center_x, center_y);
        rf_gfx_color4ub(color2.r, color2.g, color2.b, color2.a);
        rf_gfx_vertex2f(center_x + sinf(RF_DEG2RAD*i)*radius, center_y + cosf(RF_DEG2RAD*i)*radius);
        rf_gfx_color4ub(color2.r, color2.g, color2.b, color2.a);
        rf_gfx_vertex2f(center_x + sinf(RF_DEG2RAD*(i + 10))*radius, center_y + cosf(RF_DEG2RAD*(i + 10))*radius);
    }
    rf_gfx_end();
}

// Draw circle outline
RF_API void rf_draw_circle_lines(int center_x, int center_y, float radius, rf_color color)
{
    if (rf_gfx_check_buffer_limit(2 * 36)) rf_gfx_draw();

    rf_gfx_begin(RF_LINES);
    rf_gfx_color4ub(color.r, color.g, color.b, color.a);

    // NOTE: Circle outline is drawn pixel by pixel every degree (0 to 360)
    for (int i = 0; i < 360; i += 10)
    {
        rf_gfx_vertex2f(center_x + sinf(RF_DEG2RAD*i)*radius, center_y + cosf(RF_DEG2RAD*i)*radius);
        rf_gfx_vertex2f(center_x + sinf(RF_DEG2RAD*(i + 10))*radius, center_y + cosf(RF_DEG2RAD*(i + 10))*radius);
    }
    rf_gfx_end();
}

RF_API void rf_draw_ring(rf_vec2 center, float inner_radius, float outer_radius, int start_angle, int end_angle, int segments, rf_color color)
{
    if (start_angle == end_angle) return;

    // Function expects (outerRadius > innerRadius)
    if (outer_radius < inner_radius)
    {
        float tmp = outer_radius;
        outer_radius = inner_radius;
        inner_radius = tmp;

        if (outer_radius <= 0.0f) outer_radius = 0.1f;
    }

    // Function expects (endAngle > start_angle)
    if (end_angle < start_angle)
    {
        // Swap values
        int tmp = start_angle;
        start_angle = end_angle;
        end_angle = tmp;
    }

    if (segments < 4)
    {
        // Calculate how many segments we need to draw a smooth circle, taken from https://stackoverflow.com/a/2244088

        float CIRCLE_ERROR_RATE = 0.5f;


        // Calculate the maximum angle between segments based on the error rate.
        float th = acosf(2*powf(1 - CIRCLE_ERROR_RATE/outer_radius, 2) - 1);
        segments = (end_angle - start_angle) * ceilf(2 * RF_PI / th) / 360;

        if (segments <= 0) segments = 4;
    }

    // Not a ring
    if (inner_radius <= 0.0f)
    {
        rf_draw_circle_sector(center, outer_radius, start_angle, end_angle, segments, color);
        return;
    }

    float step_length = (float)(end_angle - start_angle)/(float)segments;
    float angle = start_angle;
    if (rf_gfx_check_buffer_limit(6*segments)) rf_gfx_draw();

    rf_gfx_begin(RF_TRIANGLES);
    for (int i = 0; i < segments; i++)
    {
        rf_gfx_color4ub(color.r, color.g, color.b, color.a);

        rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*inner_radius, center.y + cosf(RF_DEG2RAD*angle)*inner_radius);
        rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*outer_radius, center.y + cosf(RF_DEG2RAD*angle)*outer_radius);
        rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + step_length))*inner_radius, center.y + cosf(RF_DEG2RAD*(angle + step_length))*inner_radius);

        rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + step_length))*inner_radius, center.y + cosf(RF_DEG2RAD*(angle + step_length))*inner_radius);
        rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*outer_radius, center.y + cosf(RF_DEG2RAD*angle)*outer_radius);
        rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + step_length))*outer_radius, center.y + cosf(RF_DEG2RAD*(angle + step_length))*outer_radius);

        angle += step_length;
    }
    rf_gfx_end();

}

RF_API void rf_draw_ring_lines(rf_vec2 center, float inner_radius, float outer_radius, int start_angle, int end_angle, int segments, rf_color color)
{
    if (start_angle == end_angle) return;

    // Function expects (outerRadius > innerRadius)
    if (outer_radius < inner_radius)
    {
        float tmp = outer_radius;
        outer_radius = inner_radius;
        inner_radius = tmp;

        if (outer_radius <= 0.0f) outer_radius = 0.1f;
    }

    // Function expects (endAngle > start_angle)
    if (end_angle < start_angle)
    {
        // Swap values
        int tmp = start_angle;
        start_angle = end_angle;
        end_angle = tmp;
    }

    if (segments < 4)
    {
        // Calculate how many segments we need to draw a smooth circle, taken from https://stackoverflow.com/a/2244088

        float CIRCLE_ERROR_RATE = 0.5f;

        // Calculate the maximum angle between segments based on the error rate.
        float th = acosf(2*powf(1 - CIRCLE_ERROR_RATE/outer_radius, 2) - 1);
        segments = (end_angle - start_angle) * ceilf(2 * RF_PI / th) / 360;

        if (segments <= 0) segments = 4;
    }

    if (inner_radius <= 0.0f)
    {
        rf_draw_circle_sector_lines(center, outer_radius, start_angle, end_angle, segments, color);
        return;
    }

    float step_length = (float)(end_angle - start_angle)/(float)segments;
    float angle = start_angle;

    bool show_cap_lines = true;
    int limit = 4 * (segments + 1);
    if ((end_angle - start_angle)%360 == 0) { limit = 4 * segments; show_cap_lines = false; }

    if (rf_gfx_check_buffer_limit(limit)) rf_gfx_draw();

    rf_gfx_begin(RF_LINES);
    if (show_cap_lines)
    {
        rf_gfx_color4ub(color.r, color.g, color.b, color.a);
        rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*outer_radius, center.y + cosf(RF_DEG2RAD*angle)*outer_radius);
        rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*inner_radius, center.y + cosf(RF_DEG2RAD*angle)*inner_radius);
    }

    for (int i = 0; i < segments; i++)
    {
        rf_gfx_color4ub(color.r, color.g, color.b, color.a);

        rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*outer_radius, center.y + cosf(RF_DEG2RAD*angle)*outer_radius);
        rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + step_length))*outer_radius, center.y + cosf(RF_DEG2RAD*(angle + step_length))*outer_radius);

        rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*inner_radius, center.y + cosf(RF_DEG2RAD*angle)*inner_radius);
        rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + step_length))*inner_radius, center.y + cosf(RF_DEG2RAD*(angle + step_length))*inner_radius);

        angle += step_length;
    }

    if (show_cap_lines)
    {
        rf_gfx_color4ub(color.r, color.g, color.b, color.a);
        rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*outer_radius, center.y + cosf(RF_DEG2RAD*angle)*outer_radius);
        rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*inner_radius, center.y + cosf(RF_DEG2RAD*angle)*inner_radius);
    }
    rf_gfx_end();
}

// Draw a color-filled rectangle
RF_API void rf_draw_rectangle(int posX, int posY, int width, int height, rf_color color)
{
    rf_draw_rectangle_v((rf_vec2){ (float)posX, (float)posY }, (rf_vec2){ (float)width, (float)height }, color);
}

// Draw a color-filled rectangle (Vector version)
RF_API void rf_draw_rectangle_v(rf_vec2 position, rf_vec2 size, rf_color color)
{
    rf_draw_rectangle_pro((rf_rec) { position.x, position.y, size.x, size.y }, (rf_vec2){ 0.0f, 0.0f }, 0.0f, color);
}

// Draw a color-filled rectangle
RF_API void rf_draw_rectangle_rec(rf_rec rec, rf_color color)
{
    rf_draw_rectangle_pro(rec, (rf_vec2){ 0.0f, 0.0f }, 0.0f, color);
}

// Draw a color-filled rectangle with pro parameters
RF_API void rf_draw_rectangle_pro(rf_rec rec, rf_vec2 origin, float rotation, rf_color color)
{
    rf_gfx_enable_texture(rf_internal_get_shapes_texture().id);

    rf_gfx_push_matrix();
    rf_gfx_translatef(rec.x, rec.y, 0.0f);
    rf_gfx_rotatef(rotation, 0.0f, 0.0f, 1.0f);
    rf_gfx_translatef(-origin.x, -origin.y, 0.0f);

    rf_gfx_begin(RF_QUADS);
    rf_gfx_normal3f(0.0f, 0.0f, 1.0f);
    rf_gfx_color4ub(color.r, color.g, color.b, color.a);

    rf_gfx_tex_coord2f(rf_internal_ctx->rec_tex_shapes.x/rf_internal_ctx->tex_shapes.width, rf_internal_ctx->rec_tex_shapes.y/rf_internal_ctx->tex_shapes.height);
    rf_gfx_vertex2f(0.0f, 0.0f);

    rf_gfx_tex_coord2f(rf_internal_ctx->rec_tex_shapes.x/rf_internal_ctx->tex_shapes.width, (rf_internal_ctx->rec_tex_shapes.y + rf_internal_ctx->rec_tex_shapes.height)/rf_internal_ctx->tex_shapes.height);
    rf_gfx_vertex2f(0.0f, rec.height);

    rf_gfx_tex_coord2f((rf_internal_ctx->rec_tex_shapes.x + rf_internal_ctx->rec_tex_shapes.width)/rf_internal_ctx->tex_shapes.width, (rf_internal_ctx->rec_tex_shapes.y + rf_internal_ctx->rec_tex_shapes.height)/rf_internal_ctx->tex_shapes.height);
    rf_gfx_vertex2f(rec.width, rec.height);

    rf_gfx_tex_coord2f((rf_internal_ctx->rec_tex_shapes.x + rf_internal_ctx->rec_tex_shapes.width)/rf_internal_ctx->tex_shapes.width, rf_internal_ctx->rec_tex_shapes.y/rf_internal_ctx->tex_shapes.height);
    rf_gfx_vertex2f(rec.width, 0.0f);
    rf_gfx_end();
    rf_gfx_pop_matrix();

    rf_gfx_disable_texture();
}

// Draw a vertical-gradient-filled rectangle
// NOTE: Gradient goes from bottom (color1) to top (color2)
RF_API void rf_draw_rectangle_gradient_v(int pos_x, int pos_y, int width, int height, rf_color color1, rf_color color2)
{
    rf_draw_rectangle_gradient((rf_rec) {(float)pos_x, (float)pos_y, (float)width, (float)height }, color1, color2, color2, color1);
}

// Draw a horizontal-gradient-filled rectangle
// NOTE: Gradient goes from bottom (color1) to top (color2)
RF_API void rf_draw_rectangle_gradient_h(int pos_x, int pos_y, int width, int height, rf_color color1, rf_color color2)
{
    rf_draw_rectangle_gradient((rf_rec) {(float)pos_x, (float)pos_y, (float)width, (float)height }, color1, color1, color2, color2);
}

// Draw a gradient-filled rectangle
// NOTE: Colors refer to corners, starting at top-lef corner and counter-clockwise
RF_API void rf_draw_rectangle_gradient(rf_rec rec, rf_color col1, rf_color col2, rf_color col3, rf_color col4)
{
    rf_gfx_enable_texture(rf_internal_get_shapes_texture().id);

    rf_gfx_push_matrix();
    rf_gfx_begin(RF_QUADS);
    rf_gfx_normal3f(0.0f, 0.0f, 1.0f);

    // NOTE: Default raylib font character 95 is a white square
    rf_gfx_color4ub(col1.r, col1.g, col1.b, col1.a);
    rf_gfx_tex_coord2f(rf_internal_ctx->rec_tex_shapes.x/rf_internal_ctx->tex_shapes.width, rf_internal_ctx->rec_tex_shapes.y/rf_internal_ctx->tex_shapes.height);
    rf_gfx_vertex2f(rec.x, rec.y);

    rf_gfx_color4ub(col2.r, col2.g, col2.b, col2.a);
    rf_gfx_tex_coord2f(rf_internal_ctx->rec_tex_shapes.x/rf_internal_ctx->tex_shapes.width, (rf_internal_ctx->rec_tex_shapes.y + rf_internal_ctx->rec_tex_shapes.height)/rf_internal_ctx->tex_shapes.height);
    rf_gfx_vertex2f(rec.x, rec.y + rec.height);

    rf_gfx_color4ub(col3.r, col3.g, col3.b, col3.a);
    rf_gfx_tex_coord2f((rf_internal_ctx->rec_tex_shapes.x + rf_internal_ctx->rec_tex_shapes.width)/rf_internal_ctx->tex_shapes.width, (rf_internal_ctx->rec_tex_shapes.y + rf_internal_ctx->rec_tex_shapes.height)/rf_internal_ctx->tex_shapes.height);
    rf_gfx_vertex2f(rec.x + rec.width, rec.y + rec.height);

    rf_gfx_color4ub(col4.r, col4.g, col4.b, col4.a);
    rf_gfx_tex_coord2f((rf_internal_ctx->rec_tex_shapes.x + rf_internal_ctx->rec_tex_shapes.width)/rf_internal_ctx->tex_shapes.width, rf_internal_ctx->rec_tex_shapes.y/rf_internal_ctx->tex_shapes.height);
    rf_gfx_vertex2f(rec.x + rec.width, rec.y);
    rf_gfx_end();
    rf_gfx_pop_matrix();

    rf_gfx_disable_texture();
}

// Draw rectangle outline with extended parameters
RF_API void rf_draw_rectangle_outline(rf_rec rec, int line_thick, rf_color color)
{
    if (line_thick > rec.width || line_thick > rec.height)
    {
        if (rec.width > rec.height) line_thick = (int)rec.height/2;
        else if (rec.width < rec.height) line_thick = (int)rec.width/2;
    }

    rf_draw_rectangle_pro((rf_rec) {(int)rec.x, (int)rec.y, (int)rec.width, line_thick }, (rf_vec2){0.0f, 0.0f}, 0.0f, color);
    rf_draw_rectangle_pro((rf_rec) {(int)(rec.x - line_thick + rec.width), (int)(rec.y + line_thick), line_thick, (int)(rec.height - line_thick * 2.0f) }, (rf_vec2){0.0f, 0.0f}, 0.0f, color);
    rf_draw_rectangle_pro((rf_rec) {(int)rec.x, (int)(rec.y + rec.height - line_thick), (int)rec.width, line_thick }, (rf_vec2){0.0f, 0.0f}, 0.0f, color);
    rf_draw_rectangle_pro((rf_rec) {(int)rec.x, (int)(rec.y + line_thick), line_thick, (int)(rec.height - line_thick * 2) }, (rf_vec2) {0.0f, 0.0f }, 0.0f, color);
}

// Draw rectangle with rounded edges
RF_API void rf_draw_rectangle_rounded(rf_rec rec, float roundness, int segments, rf_color color)
{
    // Not a rounded rectangle
    if ((roundness <= 0.0f) || (rec.width < 1) || (rec.height < 1 ))
    {
        rf_draw_rectangle_pro(rec, (rf_vec2){0.0f, 0.0f}, 0.0f, color);
        return;
    }

    if (roundness >= 1.0f) roundness = 1.0f;

    // Calculate corner radius
    float radius = (rec.width > rec.height)? (rec.height*roundness)/2 : (rec.width*roundness)/2;
    if (radius <= 0.0f) return;

    // Calculate number of segments to use for the corners
    if (segments < 4)
    {
        // Calculate how many segments we need to draw a smooth circle, taken from https://stackoverflow.com/a/2244088

        float CIRCLE_ERROR_RATE = 0.5f;

        // Calculate the maximum angle between segments based on the error rate.
        float th = acosf(2*powf(1 - CIRCLE_ERROR_RATE/radius, 2) - 1);
        segments = ceilf(2 * RF_PI / th) / 4;
        if (segments <= 0) segments = 4;
    }

    float step_length = 90.0f/(float)segments;

    /*  Quick sketch to make sense of all of this (there are 9 parts to draw, also mark the 12 points we'll use below)
     *  Not my best attempt at ASCII art, just preted it's rounded rectangle :)
     *     P0                    P1
     *       ____________________
     *     /|                    |\
     *    /1|          2         |3\
     *P7 /__|____________________|__\ P2
     *  /   |                    |  _\ P2
     *  |   |P8                P9|   |
     *  | 8 |          9         | 4 |
     *  | __|____________________|__ |
     *P6 \  |P11              P10|  / P3
     *    \7|          6         |5/
     *     \|____________________|/
     *     P5                    P4
     */

    const rf_vec2 point[12] = { // coordinates of the 12 points that define the rounded rect (the idea here is to make things easier)
            {(float)rec.x + radius, rec.y}, {(float)(rec.x + rec.width) - radius, rec.y}, { rec.x + rec.width, (float)rec.y + radius }, // PO, P1, P2
            {rec.x + rec.width, (float)(rec.y + rec.height) - radius}, {(float)(rec.x + rec.width) - radius, rec.y + rec.height}, // P3, P4
            {(float)rec.x + radius, rec.y + rec.height}, { rec.x, (float)(rec.y + rec.height) - radius}, {rec.x, (float)rec.y + radius}, // P5, P6, P7
            {(float)rec.x + radius, (float)rec.y + radius}, {(float)(rec.x + rec.width) - radius, (float)rec.y + radius}, // P8, P9
            {(float)(rec.x + rec.width) - radius, (float)(rec.y + rec.height) - radius}, {(float)rec.x + radius, (float)(rec.y + rec.height) - radius} // P10, P11
    };

    const rf_vec2 centers[4] = {point[8], point[9], point[10], point[11] };
    const float angles[4] = { 180.0f, 90.0f, 0.0f, 270.0f };
    if (rf_gfx_check_buffer_limit(12*segments + 5*6)) rf_gfx_draw(); // 4 corners with 3 vertices per segment + 5 rectangles with 6 vertices each

    rf_gfx_begin(RF_TRIANGLES);
    // Draw all of the 4 corners: [1] Upper Left Corner, [3] Upper Right Corner, [5] Lower Right Corner, [7] Lower Left Corner
    for (int k = 0; k < 4; ++k) // Hope the compiler is smart enough to unroll this loop
    {
        float angle = angles[k];
        const rf_vec2 center = centers[k];
        for (int i = 0; i < segments; i++)
        {
            rf_gfx_color4ub(color.r, color.g, color.b, color.a);
            rf_gfx_vertex2f(center.x, center.y);
            rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*radius, center.y + cosf(RF_DEG2RAD*angle)*radius);
            rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + step_length))*radius, center.y + cosf(RF_DEG2RAD*(angle + step_length))*radius);
            angle += step_length;
        }
    }

    // [2] Upper rf_rec
    rf_gfx_color4ub(color.r, color.g, color.b, color.a);
    rf_gfx_vertex2f(point[0].x, point[0].y);
    rf_gfx_vertex2f(point[8].x, point[8].y);
    rf_gfx_vertex2f(point[9].x, point[9].y);
    rf_gfx_vertex2f(point[1].x, point[1].y);
    rf_gfx_vertex2f(point[0].x, point[0].y);
    rf_gfx_vertex2f(point[9].x, point[9].y);

    // [4] Right rf_rec
    rf_gfx_color4ub(color.r, color.g, color.b, color.a);
    rf_gfx_vertex2f(point[9].x, point[9].y);
    rf_gfx_vertex2f(point[10].x, point[10].y);
    rf_gfx_vertex2f(point[3].x, point[3].y);
    rf_gfx_vertex2f(point[2].x, point[2].y);
    rf_gfx_vertex2f(point[9].x, point[9].y);
    rf_gfx_vertex2f(point[3].x, point[3].y);

    // [6] Bottom rf_rec
    rf_gfx_color4ub(color.r, color.g, color.b, color.a);
    rf_gfx_vertex2f(point[11].x, point[11].y);
    rf_gfx_vertex2f(point[5].x, point[5].y);
    rf_gfx_vertex2f(point[4].x, point[4].y);
    rf_gfx_vertex2f(point[10].x, point[10].y);
    rf_gfx_vertex2f(point[11].x, point[11].y);
    rf_gfx_vertex2f(point[4].x, point[4].y);

    // [8] Left rf_rec
    rf_gfx_color4ub(color.r, color.g, color.b, color.a);
    rf_gfx_vertex2f(point[7].x, point[7].y);
    rf_gfx_vertex2f(point[6].x, point[6].y);
    rf_gfx_vertex2f(point[11].x, point[11].y);
    rf_gfx_vertex2f(point[8].x, point[8].y);
    rf_gfx_vertex2f(point[7].x, point[7].y);
    rf_gfx_vertex2f(point[11].x, point[11].y);

    // [9] Middle rf_rec
    rf_gfx_color4ub(color.r, color.g, color.b, color.a);
    rf_gfx_vertex2f(point[8].x, point[8].y);
    rf_gfx_vertex2f(point[11].x, point[11].y);
    rf_gfx_vertex2f(point[10].x, point[10].y);
    rf_gfx_vertex2f(point[9].x, point[9].y);
    rf_gfx_vertex2f(point[8].x, point[8].y);
    rf_gfx_vertex2f(point[10].x, point[10].y);
    rf_gfx_end();

}

// Draw rectangle with rounded edges outline
RF_API void rf_draw_rectangle_rounded_lines(rf_rec rec, float roundness, int segments, int line_thick, rf_color color)
{
    if (line_thick < 0) line_thick = 0;

    // Not a rounded rectangle
    if (roundness <= 0.0f)
    {
        rf_draw_rectangle_outline((rf_rec) {rec.x - line_thick, rec.y - line_thick, rec.width + 2 * line_thick, rec.height + 2 * line_thick}, line_thick, color);
        return;
    }

    if (roundness >= 1.0f) roundness = 1.0f;

    // Calculate corner radius
    float radius = (rec.width > rec.height)? (rec.height*roundness)/2 : (rec.width*roundness)/2;
    if (radius <= 0.0f) return;

    // Calculate number of segments to use for the corners
    if (segments < 4)
    {
        // Calculate how many segments we need to draw a smooth circle, taken from https://stackoverflow.com/a/2244088

        float CIRCLE_ERROR_RATE = 0.5f;

        // Calculate the maximum angle between segments based on the error rate.
        float th = acosf(2*powf(1 - CIRCLE_ERROR_RATE/radius, 2) - 1);
        segments = ceilf(2 * RF_PI / th) / 2;
        if (segments <= 0) segments = 4;
    }

    float step_length = 90.0f/(float)segments;
    const float outer_radius = radius + (float)line_thick, inner_radius = radius;

    /*  Quick sketch to make sense of all of this (mark the 16 + 4(corner centers P16-19) points we'll use below)
     *  Not my best attempt at ASCII art, just preted it's rounded rectangle :)
     *     P0                     P1
     *        ====================
     *     // P8                P9 \
     *    //                        \
     *P7 // P15                  P10 \\ P2
        \\ P2
     *  ||   *P16             P17*    ||
     *  ||                            ||
     *  || P14                   P11  ||
     *P6 \\  *P19             P18*   // P3
     *    \\                        //
     *     \\ P13              P12 //
     *        ====================
     *     P5                     P4

     */
    const rf_vec2 point[16] =
            {
                    {(float)rec.x + inner_radius, rec.y - line_thick}, {(float)(rec.x + rec.width) - inner_radius, rec.y - line_thick}, { rec.x + rec.width + line_thick, (float)rec.y + inner_radius }, // PO, P1, P2
                    {rec.x + rec.width + line_thick, (float)(rec.y + rec.height) - inner_radius}, {(float)(rec.x + rec.width) - inner_radius, rec.y + rec.height + line_thick}, // P3, P4
                    {(float)rec.x + inner_radius, rec.y + rec.height + line_thick}, { rec.x - line_thick, (float)(rec.y + rec.height) - inner_radius}, {rec.x - line_thick, (float)rec.y + inner_radius}, // P5, P6, P7
                    {(float)rec.x + inner_radius, rec.y}, {(float)(rec.x + rec.width) - inner_radius, rec.y}, // P8, P9
                    { rec.x + rec.width, (float)rec.y + inner_radius }, {rec.x + rec.width, (float)(rec.y + rec.height) - inner_radius}, // P10, P11
                    {(float)(rec.x + rec.width) - inner_radius, rec.y + rec.height}, {(float)rec.x + inner_radius, rec.y + rec.height}, // P12, P13
                    { rec.x, (float)(rec.y + rec.height) - inner_radius}, {rec.x, (float)rec.y + inner_radius} // P14, P15
            };

    const rf_vec2 centers[4] =
            {
                    {(float)rec.x + inner_radius, (float)rec.y + inner_radius}, {(float)(rec.x + rec.width) - inner_radius, (float)rec.y + inner_radius}, // P16, P17
                    {(float)(rec.x + rec.width) - inner_radius, (float)(rec.y + rec.height) - inner_radius}, {(float)rec.x + inner_radius, (float)(rec.y + rec.height) - inner_radius} // P18, P19
            };

    const float angles[4] = { 180.0f, 90.0f, 0.0f, 270.0f };

    if (line_thick > 1)
    {
        if (rf_gfx_check_buffer_limit(4 * 6*segments + 4 * 6)) rf_gfx_draw(); // 4 corners with 6(2 * 3) vertices for each segment + 4 rectangles with 6 vertices each

        rf_gfx_begin(RF_TRIANGLES);

        // Draw all of the 4 corners first: Upper Left Corner, Upper Right Corner, Lower Right Corner, Lower Left Corner
        for (int k = 0; k < 4; ++k) // Hope the compiler is smart enough to unroll this loop
        {
            float angle = angles[k];
            const rf_vec2 center = centers[k];

            for (int i = 0; i < segments; i++)
            {
                rf_gfx_color4ub(color.r, color.g, color.b, color.a);

                rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*inner_radius, center.y + cosf(RF_DEG2RAD*angle)*inner_radius);
                rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*outer_radius, center.y + cosf(RF_DEG2RAD*angle)*outer_radius);
                rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + step_length))*inner_radius, center.y + cosf(RF_DEG2RAD*(angle + step_length))*inner_radius);

                rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + step_length))*inner_radius, center.y + cosf(RF_DEG2RAD*(angle + step_length))*inner_radius);
                rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*outer_radius, center.y + cosf(RF_DEG2RAD*angle)*outer_radius);
                rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + step_length))*outer_radius, center.y + cosf(RF_DEG2RAD*(angle + step_length))*outer_radius);

                angle += step_length;
            }
        }

        // Upper rectangle
        rf_gfx_color4ub(color.r, color.g, color.b, color.a);
        rf_gfx_vertex2f(point[0].x, point[0].y);
        rf_gfx_vertex2f(point[8].x, point[8].y);
        rf_gfx_vertex2f(point[9].x, point[9].y);
        rf_gfx_vertex2f(point[1].x, point[1].y);
        rf_gfx_vertex2f(point[0].x, point[0].y);
        rf_gfx_vertex2f(point[9].x, point[9].y);

        // Right rectangle
        rf_gfx_color4ub(color.r, color.g, color.b, color.a);
        rf_gfx_vertex2f(point[10].x, point[10].y);
        rf_gfx_vertex2f(point[11].x, point[11].y);
        rf_gfx_vertex2f(point[3].x, point[3].y);
        rf_gfx_vertex2f(point[2].x, point[2].y);
        rf_gfx_vertex2f(point[10].x, point[10].y);
        rf_gfx_vertex2f(point[3].x, point[3].y);

        // Lower rectangle
        rf_gfx_color4ub(color.r, color.g, color.b, color.a);
        rf_gfx_vertex2f(point[13].x, point[13].y);
        rf_gfx_vertex2f(point[5].x, point[5].y);
        rf_gfx_vertex2f(point[4].x, point[4].y);
        rf_gfx_vertex2f(point[12].x, point[12].y);
        rf_gfx_vertex2f(point[13].x, point[13].y);
        rf_gfx_vertex2f(point[4].x, point[4].y);

        // Left rectangle
        rf_gfx_color4ub(color.r, color.g, color.b, color.a);
        rf_gfx_vertex2f(point[7].x, point[7].y);
        rf_gfx_vertex2f(point[6].x, point[6].y);
        rf_gfx_vertex2f(point[14].x, point[14].y);
        rf_gfx_vertex2f(point[15].x, point[15].y);
        rf_gfx_vertex2f(point[7].x, point[7].y);
        rf_gfx_vertex2f(point[14].x, point[14].y);
        rf_gfx_end();

    }
    else
    {
        // Use LINES to draw the outline
        if (rf_gfx_check_buffer_limit(8*segments + 4 * 2)) rf_gfx_draw(); // 4 corners with 2 vertices for each segment + 4 rectangles with 2 vertices each

        rf_gfx_begin(RF_LINES);

        // Draw all of the 4 corners first: Upper Left Corner, Upper Right Corner, Lower Right Corner, Lower Left Corner
        for (int k = 0; k < 4; ++k) // Hope the compiler is smart enough to unroll this loop
        {
            float angle = angles[k];
            const rf_vec2 center = centers[k];

            for (int i = 0; i < segments; i++)
            {
                rf_gfx_color4ub(color.r, color.g, color.b, color.a);
                rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*outer_radius, center.y + cosf(RF_DEG2RAD*angle)*outer_radius);
                rf_gfx_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + step_length))*outer_radius, center.y + cosf(RF_DEG2RAD*(angle + step_length))*outer_radius);
                angle += step_length;
            }
        }
        // And now the remaining 4 lines
        for(int i = 0; i < 8; i += 2)
        {
            rf_gfx_color4ub(color.r, color.g, color.b, color.a);
            rf_gfx_vertex2f(point[i].x, point[i].y);
            rf_gfx_vertex2f(point[i + 1].x, point[i + 1].y);
        }
        rf_gfx_end();
    }
}

// Draw a triangle
// NOTE: Vertex must be provided in counter-clockwise order
RF_API void rf_draw_triangle(rf_vec2 v1, rf_vec2 v2, rf_vec2 v3, rf_color color)
{
    if (rf_gfx_check_buffer_limit(4)) rf_gfx_draw();
    rf_gfx_begin(RF_TRIANGLES);
    rf_gfx_color4ub(color.r, color.g, color.b, color.a);
    rf_gfx_vertex2f(v1.x, v1.y);
    rf_gfx_vertex2f(v2.x, v2.y);
    rf_gfx_vertex2f(v3.x, v3.y);
    rf_gfx_end();

}

// Draw a triangle using lines
// NOTE: Vertex must be provided in counter-clockwise order
RF_API void rf_draw_triangle_lines(rf_vec2 v1, rf_vec2 v2, rf_vec2 v3, rf_color color)
{
    if (rf_gfx_check_buffer_limit(6)) rf_gfx_draw();

    rf_gfx_begin(RF_LINES);
    rf_gfx_color4ub(color.r, color.g, color.b, color.a);
    rf_gfx_vertex2f(v1.x, v1.y);
    rf_gfx_vertex2f(v2.x, v2.y);

    rf_gfx_vertex2f(v2.x, v2.y);
    rf_gfx_vertex2f(v3.x, v3.y);

    rf_gfx_vertex2f(v3.x, v3.y);
    rf_gfx_vertex2f(v1.x, v1.y);
    rf_gfx_end();
}

// Draw a triangle fan defined by points
// NOTE: First vertex provided is the center, shared by all triangles
RF_API void rf_draw_triangle_fan(rf_vec2 *points, int points_count, rf_color color)
{
    if (points_count >= 3)
    {
        if (rf_gfx_check_buffer_limit((points_count - 2) * 4)) rf_gfx_draw();

        rf_gfx_enable_texture(rf_internal_get_shapes_texture().id);
        rf_gfx_begin(RF_QUADS);
        rf_gfx_color4ub(color.r, color.g, color.b, color.a);

        for (int i = 1; i < points_count - 1; i++)
        {
            rf_gfx_tex_coord2f(rf_internal_ctx->rec_tex_shapes.x/rf_internal_ctx->tex_shapes.width, rf_internal_ctx->rec_tex_shapes.y/rf_internal_ctx->tex_shapes.height);
            rf_gfx_vertex2f(points[0].x, points[0].y);

            rf_gfx_tex_coord2f(rf_internal_ctx->rec_tex_shapes.x/rf_internal_ctx->tex_shapes.width, (rf_internal_ctx->rec_tex_shapes.y + rf_internal_ctx->rec_tex_shapes.height)/rf_internal_ctx->tex_shapes.height);
            rf_gfx_vertex2f(points[i].x, points[i].y);

            rf_gfx_tex_coord2f((rf_internal_ctx->rec_tex_shapes.x + rf_internal_ctx->rec_tex_shapes.width)/rf_internal_ctx->tex_shapes.width, (rf_internal_ctx->rec_tex_shapes.y + rf_internal_ctx->rec_tex_shapes.height)/rf_internal_ctx->tex_shapes.height);
            rf_gfx_vertex2f(points[i + 1].x, points[i + 1].y);

            rf_gfx_tex_coord2f((rf_internal_ctx->rec_tex_shapes.x + rf_internal_ctx->rec_tex_shapes.width)/rf_internal_ctx->tex_shapes.width, rf_internal_ctx->rec_tex_shapes.y/rf_internal_ctx->tex_shapes.height);
            rf_gfx_vertex2f(points[i + 1].x, points[i + 1].y);
        }
        rf_gfx_end();
        rf_gfx_disable_texture();
    }
}

// Draw a triangle strip defined by points
// NOTE: Every new vertex connects with previous two
RF_API void rf_draw_triangle_strip(rf_vec2 *points, int points_count, rf_color color)
{
    if (points_count >= 3)
    {
        if (rf_gfx_check_buffer_limit(points_count)) rf_gfx_draw();

        rf_gfx_begin(RF_TRIANGLES);
        rf_gfx_color4ub(color.r, color.g, color.b, color.a);

        for (int i = 2; i < points_count; i++)
        {
            if ((i%2) == 0)
            {
                rf_gfx_vertex2f(points[i].x, points[i].y);
                rf_gfx_vertex2f(points[i - 2].x, points[i - 2].y);
                rf_gfx_vertex2f(points[i - 1].x, points[i - 1].y);
            }
            else
            {
                rf_gfx_vertex2f(points[i].x, points[i].y);
                rf_gfx_vertex2f(points[i - 1].x, points[i - 1].y);
                rf_gfx_vertex2f(points[i - 2].x, points[i - 2].y);
            }
        }
        rf_gfx_end();
    }
}

// Draw a regular polygon of n sides (Vector version)
RF_API void rf_draw_poly(rf_vec2 center, int sides, float radius, float rotation, rf_color color)
{
    if (sides < 3) sides = 3;
    float centralAngle = 0.0f;

    if (rf_gfx_check_buffer_limit(4 * (360/sides))) rf_gfx_draw();

    rf_gfx_push_matrix();
    rf_gfx_translatef(center.x, center.y, 0.0f);
    rf_gfx_rotatef(rotation, 0.0f, 0.0f, 1.0f);
    rf_gfx_begin(RF_TRIANGLES);
    for (int i = 0; i < sides; i++)
    {
        rf_gfx_color4ub(color.r, color.g, color.b, color.a);

        rf_gfx_vertex2f(0, 0);
        rf_gfx_vertex2f(sinf(RF_DEG2RAD*centralAngle)*radius, cosf(RF_DEG2RAD*centralAngle)*radius);

        centralAngle += 360.0f/(float)sides;
        rf_gfx_vertex2f(sinf(RF_DEG2RAD*centralAngle)*radius, cosf(RF_DEG2RAD*centralAngle)*radius);
    }
    rf_gfx_end();

    rf_gfx_pop_matrix();
}

// Draw a rf_texture2d with extended parameters
RF_API void rf_draw_texture(rf_texture2d texture, rf_vec2 position, float rotation, float scale, rf_color tint)
{
    rf_rec source_rec = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
    rf_rec dest_rec = { position.x, position.y, (float)texture.width*scale, (float)texture.height*scale };
    rf_vec2 origin = { 0.0f, 0.0f };

    rf_draw_texture_region(texture, source_rec, dest_rec, origin, rotation, tint);
}

// Draw a part of a texture (defined by a rectangle) with 'pro' parameters. Note: origin is relative to destination rectangle size
RF_API void rf_draw_texture_region(rf_texture2d texture, rf_rec source_rec, rf_rec dest_rec, rf_vec2 origin, float rotation, rf_color tint)
{
    // Check if texture is valid
    if (texture.id > 0)
    {
        float width = (float)texture.width;
        float height = (float)texture.height;

        bool flip_x = false;

        if (source_rec.width < 0) { flip_x = true; source_rec.width *= -1; }
        if (source_rec.height < 0) source_rec.y -= source_rec.height;

        rf_gfx_enable_texture(texture.id);

        rf_gfx_push_matrix();
        rf_gfx_translatef(dest_rec.x, dest_rec.y, 0.0f);
        rf_gfx_rotatef(rotation, 0.0f, 0.0f, 1.0f);
        rf_gfx_translatef(-origin.x, -origin.y, 0.0f);

        rf_gfx_begin(RF_QUADS);
        rf_gfx_color4ub(tint.r, tint.g, tint.b, tint.a);
        rf_gfx_normal3f(0.0f, 0.0f, 1.0f); // Normal vector pointing towards viewer

        // Bottom-left corner for texture and quad
        if (flip_x) rf_gfx_tex_coord2f((source_rec.x + source_rec.width)/width, source_rec.y/height);
        else rf_gfx_tex_coord2f(source_rec.x/width, source_rec.y/height);
        rf_gfx_vertex2f(0.0f, 0.0f);

        // Bottom-right corner for texture and quad
        if (flip_x) rf_gfx_tex_coord2f((source_rec.x + source_rec.width)/width, (source_rec.y + source_rec.height)/height);
        else rf_gfx_tex_coord2f(source_rec.x/width, (source_rec.y + source_rec.height)/height);
        rf_gfx_vertex2f(0.0f, dest_rec.height);

        // Top-right corner for texture and quad
        if (flip_x) rf_gfx_tex_coord2f(source_rec.x/width, (source_rec.y + source_rec.height)/height);
        else rf_gfx_tex_coord2f((source_rec.x + source_rec.width)/width, (source_rec.y + source_rec.height)/height);
        rf_gfx_vertex2f(dest_rec.width, dest_rec.height);

        // Top-left corner for texture and quad
        if (flip_x) rf_gfx_tex_coord2f(source_rec.x/width, source_rec.y/height);
        else rf_gfx_tex_coord2f((source_rec.x + source_rec.width)/width, source_rec.y/height);
        rf_gfx_vertex2f(dest_rec.width, 0.0f);
        rf_gfx_end();
        rf_gfx_pop_matrix();

        rf_gfx_disable_texture();
    }
}

// Draws a texture (or part of it) that stretches or shrinks nicely using n-patch info
RF_API void rf_draw_texture_npatch(rf_texture2d texture, rf_npatch_info n_patch_info, rf_rec dest_rec, rf_vec2 origin, float rotation, rf_color tint)
{
    if (texture.id > 0)
    {
        float width = (float)texture.width;
        float height = (float)texture.height;

        float patch_width = (dest_rec.width <= 0.0f)? 0.0f : dest_rec.width;
        float patch_height = (dest_rec.height <= 0.0f)? 0.0f : dest_rec.height;

        if (n_patch_info.source_rec.width < 0) n_patch_info.source_rec.x -= n_patch_info.source_rec.width;
        if (n_patch_info.source_rec.height < 0) n_patch_info.source_rec.y -= n_patch_info.source_rec.height;
        if (n_patch_info.type == RF_NPT_3PATCH_HORIZONTAL) patch_height = n_patch_info.source_rec.height;
        if (n_patch_info.type == RF_NPT_3PATCH_VERTICAL) patch_width = n_patch_info.source_rec.width;

        bool draw_center = true;
        bool draw_middle = true;
        float left_border = (float)n_patch_info.left;
        float top_border = (float)n_patch_info.top;
        float right_border = (float)n_patch_info.right;
        float bottom_border = (float)n_patch_info.bottom;

        // adjust the lateral (left and right) border widths in case patch_width < texture.width
        if (patch_width <= (left_border + right_border) && n_patch_info.type != RF_NPT_3PATCH_VERTICAL)
        {
            draw_center = false;
            left_border = (left_border / (left_border + right_border)) * patch_width;
            right_border = patch_width - left_border;
        }
        // adjust the lateral (top and bottom) border heights in case patch_height < texture.height
        if (patch_height <= (top_border + bottom_border) && n_patch_info.type != RF_NPT_3PATCH_HORIZONTAL)
        {
            draw_middle = false;
            top_border = (top_border / (top_border + bottom_border)) * patch_height;
            bottom_border = patch_height - top_border;
        }

        rf_vec2 vert_a, vert_b, vert_c, vert_d;
        vert_a.x = 0.0f; // outer left
        vert_a.y = 0.0f; // outer top
        vert_b.x = left_border; // inner left
        vert_b.y = top_border; // inner top
        vert_c.x = patch_width - right_border; // inner right
        vert_c.y = patch_height - bottom_border; // inner bottom
        vert_d.x = patch_width; // outer right
        vert_d.y = patch_height; // outer bottom

        rf_vec2 coord_a, coord_b, coord_c, coord_d;
        coord_a.x = n_patch_info.source_rec.x / width;
        coord_a.y = n_patch_info.source_rec.y / height;
        coord_b.x = (n_patch_info.source_rec.x + left_border) / width;
        coord_b.y = (n_patch_info.source_rec.y + top_border) / height;
        coord_c.x = (n_patch_info.source_rec.x + n_patch_info.source_rec.width - right_border) / width;
        coord_c.y = (n_patch_info.source_rec.y + n_patch_info.source_rec.height - bottom_border) / height;
        coord_d.x = (n_patch_info.source_rec.x + n_patch_info.source_rec.width) / width;
        coord_d.y = (n_patch_info.source_rec.y + n_patch_info.source_rec.height) / height;

        rf_gfx_enable_texture(texture.id);

        rf_gfx_push_matrix();
        rf_gfx_translatef(dest_rec.x, dest_rec.y, 0.0f);
        rf_gfx_rotatef(rotation, 0.0f, 0.0f, 1.0f);
        rf_gfx_translatef(-origin.x, -origin.y, 0.0f);

        rf_gfx_begin(RF_QUADS);
        rf_gfx_color4ub(tint.r, tint.g, tint.b, tint.a);
        rf_gfx_normal3f(0.0f, 0.0f, 1.0f); // Normal vector pointing towards viewer

        if (n_patch_info.type == RF_NPT_9PATCH)
        {
            // ------------------------------------------------------------
            // TOP-LEFT QUAD
            rf_gfx_tex_coord2f(coord_a.x, coord_b.y); rf_gfx_vertex2f(vert_a.x, vert_b.y); // Bottom-left corner for texture and quad
            rf_gfx_tex_coord2f(coord_b.x, coord_b.y); rf_gfx_vertex2f(vert_b.x, vert_b.y); // Bottom-right corner for texture and quad
            rf_gfx_tex_coord2f(coord_b.x, coord_a.y); rf_gfx_vertex2f(vert_b.x, vert_a.y); // Top-right corner for texture and quad
            rf_gfx_tex_coord2f(coord_a.x, coord_a.y); rf_gfx_vertex2f(vert_a.x, vert_a.y); // Top-left corner for texture and quad
            if (draw_center)
            {
                // TOP-CENTER QUAD
                rf_gfx_tex_coord2f(coord_b.x, coord_b.y); rf_gfx_vertex2f(vert_b.x, vert_b.y); // Bottom-left corner for texture and quad
                rf_gfx_tex_coord2f(coord_c.x, coord_b.y); rf_gfx_vertex2f(vert_c.x, vert_b.y); // Bottom-right corner for texture and quad
                rf_gfx_tex_coord2f(coord_c.x, coord_a.y); rf_gfx_vertex2f(vert_c.x, vert_a.y); // Top-right corner for texture and quad
                rf_gfx_tex_coord2f(coord_b.x, coord_a.y); rf_gfx_vertex2f(vert_b.x, vert_a.y); // Top-left corner for texture and quad
            }
            // TOP-RIGHT QUAD
            rf_gfx_tex_coord2f(coord_c.x, coord_b.y); rf_gfx_vertex2f(vert_c.x, vert_b.y); // Bottom-left corner for texture and quad
            rf_gfx_tex_coord2f(coord_d.x, coord_b.y); rf_gfx_vertex2f(vert_d.x, vert_b.y); // Bottom-right corner for texture and quad
            rf_gfx_tex_coord2f(coord_d.x, coord_a.y); rf_gfx_vertex2f(vert_d.x, vert_a.y); // Top-right corner for texture and quad
            rf_gfx_tex_coord2f(coord_c.x, coord_a.y); rf_gfx_vertex2f(vert_c.x, vert_a.y); // Top-left corner for texture and quad
            if (draw_middle)
            {
                // ------------------------------------------------------------
                // MIDDLE-LEFT QUAD
                rf_gfx_tex_coord2f(coord_a.x, coord_c.y); rf_gfx_vertex2f(vert_a.x, vert_c.y); // Bottom-left corner for texture and quad
                rf_gfx_tex_coord2f(coord_b.x, coord_c.y); rf_gfx_vertex2f(vert_b.x, vert_c.y); // Bottom-right corner for texture and quad
                rf_gfx_tex_coord2f(coord_b.x, coord_b.y); rf_gfx_vertex2f(vert_b.x, vert_b.y); // Top-right corner for texture and quad
                rf_gfx_tex_coord2f(coord_a.x, coord_b.y); rf_gfx_vertex2f(vert_a.x, vert_b.y); // Top-left corner for texture and quad
                if (draw_center)
                {
                    // MIDDLE-CENTER QUAD
                    rf_gfx_tex_coord2f(coord_b.x, coord_c.y); rf_gfx_vertex2f(vert_b.x, vert_c.y); // Bottom-left corner for texture and quad
                    rf_gfx_tex_coord2f(coord_c.x, coord_c.y); rf_gfx_vertex2f(vert_c.x, vert_c.y); // Bottom-right corner for texture and quad
                    rf_gfx_tex_coord2f(coord_c.x, coord_b.y); rf_gfx_vertex2f(vert_c.x, vert_b.y); // Top-right corner for texture and quad
                    rf_gfx_tex_coord2f(coord_b.x, coord_b.y); rf_gfx_vertex2f(vert_b.x, vert_b.y); // Top-left corner for texture and quad
                }

                // MIDDLE-RIGHT QUAD
                rf_gfx_tex_coord2f(coord_c.x, coord_c.y); rf_gfx_vertex2f(vert_c.x, vert_c.y); // Bottom-left corner for texture and quad
                rf_gfx_tex_coord2f(coord_d.x, coord_c.y); rf_gfx_vertex2f(vert_d.x, vert_c.y); // Bottom-right corner for texture and quad
                rf_gfx_tex_coord2f(coord_d.x, coord_b.y); rf_gfx_vertex2f(vert_d.x, vert_b.y); // Top-right corner for texture and quad
                rf_gfx_tex_coord2f(coord_c.x, coord_b.y); rf_gfx_vertex2f(vert_c.x, vert_b.y); // Top-left corner for texture and quad
            }

            // ------------------------------------------------------------
            // BOTTOM-LEFT QUAD
            rf_gfx_tex_coord2f(coord_a.x, coord_d.y); rf_gfx_vertex2f(vert_a.x, vert_d.y); // Bottom-left corner for texture and quad
            rf_gfx_tex_coord2f(coord_b.x, coord_d.y); rf_gfx_vertex2f(vert_b.x, vert_d.y); // Bottom-right corner for texture and quad
            rf_gfx_tex_coord2f(coord_b.x, coord_c.y); rf_gfx_vertex2f(vert_b.x, vert_c.y); // Top-right corner for texture and quad
            rf_gfx_tex_coord2f(coord_a.x, coord_c.y); rf_gfx_vertex2f(vert_a.x, vert_c.y); // Top-left corner for texture and quad
            if (draw_center)
            {
                // BOTTOM-CENTER QUAD
                rf_gfx_tex_coord2f(coord_b.x, coord_d.y); rf_gfx_vertex2f(vert_b.x, vert_d.y); // Bottom-left corner for texture and quad
                rf_gfx_tex_coord2f(coord_c.x, coord_d.y); rf_gfx_vertex2f(vert_c.x, vert_d.y); // Bottom-right corner for texture and quad
                rf_gfx_tex_coord2f(coord_c.x, coord_c.y); rf_gfx_vertex2f(vert_c.x, vert_c.y); // Top-right corner for texture and quad
                rf_gfx_tex_coord2f(coord_b.x, coord_c.y); rf_gfx_vertex2f(vert_b.x, vert_c.y); // Top-left corner for texture and quad
            }

            // BOTTOM-RIGHT QUAD
            rf_gfx_tex_coord2f(coord_c.x, coord_d.y); rf_gfx_vertex2f(vert_c.x, vert_d.y); // Bottom-left corner for texture and quad
            rf_gfx_tex_coord2f(coord_d.x, coord_d.y); rf_gfx_vertex2f(vert_d.x, vert_d.y); // Bottom-right corner for texture and quad
            rf_gfx_tex_coord2f(coord_d.x, coord_c.y); rf_gfx_vertex2f(vert_d.x, vert_c.y); // Top-right corner for texture and quad
            rf_gfx_tex_coord2f(coord_c.x, coord_c.y); rf_gfx_vertex2f(vert_c.x, vert_c.y); // Top-left corner for texture and quad
        }
        else if (n_patch_info.type == RF_NPT_3PATCH_VERTICAL)
        {
            // TOP QUAD
            // -----------------------------------------------------------
            // rf_texture coords                 Vertices
            rf_gfx_tex_coord2f(coord_a.x, coord_b.y); rf_gfx_vertex2f(vert_a.x, vert_b.y); // Bottom-left corner for texture and quad
            rf_gfx_tex_coord2f(coord_d.x, coord_b.y); rf_gfx_vertex2f(vert_d.x, vert_b.y); // Bottom-right corner for texture and quad
            rf_gfx_tex_coord2f(coord_d.x, coord_a.y); rf_gfx_vertex2f(vert_d.x, vert_a.y); // Top-right corner for texture and quad
            rf_gfx_tex_coord2f(coord_a.x, coord_a.y); rf_gfx_vertex2f(vert_a.x, vert_a.y); // Top-left corner for texture and quad
            if (draw_center)
            {
                // MIDDLE QUAD
                // -----------------------------------------------------------
                // rf_texture coords                 Vertices
                rf_gfx_tex_coord2f(coord_a.x, coord_c.y); rf_gfx_vertex2f(vert_a.x, vert_c.y); // Bottom-left corner for texture and quad
                rf_gfx_tex_coord2f(coord_d.x, coord_c.y); rf_gfx_vertex2f(vert_d.x, vert_c.y); // Bottom-right corner for texture and quad
                rf_gfx_tex_coord2f(coord_d.x, coord_b.y); rf_gfx_vertex2f(vert_d.x, vert_b.y); // Top-right corner for texture and quad
                rf_gfx_tex_coord2f(coord_a.x, coord_b.y); rf_gfx_vertex2f(vert_a.x, vert_b.y); // Top-left corner for texture and quad
            }
            // BOTTOM QUAD
            // -----------------------------------------------------------
            // rf_texture coords                 Vertices
            rf_gfx_tex_coord2f(coord_a.x, coord_d.y); rf_gfx_vertex2f(vert_a.x, vert_d.y); // Bottom-left corner for texture and quad
            rf_gfx_tex_coord2f(coord_d.x, coord_d.y); rf_gfx_vertex2f(vert_d.x, vert_d.y); // Bottom-right corner for texture and quad
            rf_gfx_tex_coord2f(coord_d.x, coord_c.y); rf_gfx_vertex2f(vert_d.x, vert_c.y); // Top-right corner for texture and quad
            rf_gfx_tex_coord2f(coord_a.x, coord_c.y); rf_gfx_vertex2f(vert_a.x, vert_c.y); // Top-left corner for texture and quad
        }
        else if (n_patch_info.type == RF_NPT_3PATCH_HORIZONTAL)
        {
            // LEFT QUAD
            // -----------------------------------------------------------
            // rf_texture coords                 Vertices
            rf_gfx_tex_coord2f(coord_a.x, coord_d.y); rf_gfx_vertex2f(vert_a.x, vert_d.y); // Bottom-left corner for texture and quad
            rf_gfx_tex_coord2f(coord_b.x, coord_d.y); rf_gfx_vertex2f(vert_b.x, vert_d.y); // Bottom-right corner for texture and quad
            rf_gfx_tex_coord2f(coord_b.x, coord_a.y); rf_gfx_vertex2f(vert_b.x, vert_a.y); // Top-right corner for texture and quad
            rf_gfx_tex_coord2f(coord_a.x, coord_a.y); rf_gfx_vertex2f(vert_a.x, vert_a.y); // Top-left corner for texture and quad
            if (draw_center)
            {
                // CENTER QUAD
                // -----------------------------------------------------------
                // rf_texture coords                 Vertices
                rf_gfx_tex_coord2f(coord_b.x, coord_d.y); rf_gfx_vertex2f(vert_b.x, vert_d.y); // Bottom-left corner for texture and quad
                rf_gfx_tex_coord2f(coord_c.x, coord_d.y); rf_gfx_vertex2f(vert_c.x, vert_d.y); // Bottom-right corner for texture and quad
                rf_gfx_tex_coord2f(coord_c.x, coord_a.y); rf_gfx_vertex2f(vert_c.x, vert_a.y); // Top-right corner for texture and quad
                rf_gfx_tex_coord2f(coord_b.x, coord_a.y); rf_gfx_vertex2f(vert_b.x, vert_a.y); // Top-left corner for texture and quad
            }
            // RIGHT QUAD
            // -----------------------------------------------------------
            // rf_texture coords                 Vertices
            rf_gfx_tex_coord2f(coord_c.x, coord_d.y); rf_gfx_vertex2f(vert_c.x, vert_d.y); // Bottom-left corner for texture and quad
            rf_gfx_tex_coord2f(coord_d.x, coord_d.y); rf_gfx_vertex2f(vert_d.x, vert_d.y); // Bottom-right corner for texture and quad
            rf_gfx_tex_coord2f(coord_d.x, coord_a.y); rf_gfx_vertex2f(vert_d.x, vert_a.y); // Top-right corner for texture and quad
            rf_gfx_tex_coord2f(coord_c.x, coord_a.y); rf_gfx_vertex2f(vert_c.x, vert_a.y); // Top-left corner for texture and quad
        }
        rf_gfx_end();
        rf_gfx_pop_matrix();

        rf_gfx_disable_texture();

    }
}

// Shows current FPS on top-left corner
RF_API void rf_draw_fps(int posX, int posY)
{
    // NOTE: We are rendering fps every second for better viewing on high framerates
    RF_INTERNAL int fps = 0;
    RF_INTERNAL int counter = 0;
    RF_INTERNAL int refreshRate = 20;

    if (counter < refreshRate) counter++;
    else
    {
        fps = rf_get_fps();
        refreshRate = fps;
        counter = 0;
    }

    // NOTE: We have rounding errors every frame, so it oscillates a lot
    char buff[100];
    snprintf(buff, 100, "%2i FPS", fps);
    rf_draw_text(buff, posX, posY, 20, RF_LIME);
}

// Draw text (using default font)
RF_API void rf_draw_text(const char* text, int posX, int posY, int fontSize, rf_color color)
{
    // Check if default font has been loaded
    if (rf_get_default_font().texture.id == 0)
    {
        return;
    }
    
    rf_vec2 position = { (float)posX, (float)posY };

    int defaultFontSize = 10;   // Default Font chars height in pixel
    if (fontSize < defaultFontSize) fontSize = defaultFontSize;
    int spacing = fontSize/defaultFontSize;

    rf_draw_text_ex(rf_get_default_font(), text, strlen(text), position, (float)fontSize, (float)spacing, color);
}

// Draw text with custom font
RF_API void rf_draw_text_ex(rf_font font, const char* text, int text_len, rf_vec2 position, float font_size, float spacing, rf_color tint)
{
    int text_offset_y = 0; // Required for line break!
    float text_offset_x = 0.0f; // Offset between characters
    float scale_factor = 0.0f;

    int letter = 0; // Current character
    int index = 0; // Index position in sprite font

    scale_factor = font_size/font.base_size;

    for (int i = 0; i < text_len; i++)
    {
        rf_utf8_codepoint codepoint = rf_get_next_utf8_codepoint(&text[i], text_len - i);
        letter = codepoint.value;
        index = rf_get_glyph_index(font, letter);

        // NOTE: Normally we exit the decoding sequence as soon as a bad unsigned char is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol so to not skip any we set 'next = 1'
        if (letter == 0x3f) codepoint.bytes_processed = 1;
        i += (codepoint.bytes_processed - 1);

        if (letter == '\n')
        {
            // NOTE: Fixed line spacing of 1.5 lines
            text_offset_y += (int)((font.base_size + font.base_size/2)*scale_factor);
            text_offset_x = 0.0f;
        }
        else
        {
            if (letter != ' ')
            {
                rf_draw_texture_region(font.texture, font.recs[index],
                                       (rf_rec){position.x + text_offset_x + font.chars[index].offset_x * scale_factor,
                                                position.y + text_offset_y + font.chars[index].offset_y*scale_factor,
                                                font.recs[index].width*scale_factor,
                                                font.recs[index].height*scale_factor }, (rf_vec2){0, 0 }, 0.0f, tint);
            }

            if (font.chars[index].advance_x == 0) text_offset_x += ((float)font.recs[index].width*scale_factor + spacing);
            else text_offset_x += ((float)font.chars[index].advance_x*scale_factor + spacing);
        }
    }
}

// Draw text wrapped
RF_API void rf_draw_text_wrap(rf_font font, const char* text, int text_len, rf_vec2 position, float font_size, float spacing, rf_color tint, float wrap_width, rf_text_wrap_mode mode)
{
    rf_rec rec = { 0, 0, wrap_width, FLT_MAX };
    rf_draw_text_rec(font, text, text_len, rec, font_size, spacing, mode, tint);
}

// Draw text using font inside rectangle limits
RF_API void rf_draw_text_rec(rf_font font, const char* text, int text_len, rf_rec rec, float font_size, float spacing, rf_text_wrap_mode wrap, rf_color tint)
{
    int   text_offset_x = 0; // Offset between characters
    int   text_offset_y = 0; // Required for line break!
    float scale_factor  = 0.0f;

    int letter = 0; // Current character
    int index  = 0; // Index position in sprite font

    scale_factor = font_size/font.base_size;

    enum
    {
        MEASURE_WORD_WRAP_STATE = 0,
        MEASURE_RESULT_STATE = 1
    };

    int state = wrap == RF_WORD_WRAP ? MEASURE_WORD_WRAP_STATE : MEASURE_RESULT_STATE;
    int start_line = -1; // Index where to begin drawing (where a line begins)
    int end_line = -1; // Index where to stop drawing (where a line ends)
    int lastk = -1; // Holds last value of the character position

    for (int i = 0, k = 0; i < text_len; i++, k++)
    {
        int glyph_width = 0;

        rf_utf8_codepoint codepoint = rf_get_next_utf8_codepoint(&text[i], text_len - i);
        letter = codepoint.value;
        index = rf_get_glyph_index(font, letter);

        // NOTE: normally we exit the decoding sequence as soon as a bad unsigned char is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol so to not skip any we set next = 1
        if (letter == 0x3f) codepoint.bytes_processed = 1;
        i += codepoint.bytes_processed - 1;

        if (letter != '\n')
        {
            glyph_width = (font.chars[index].advance_x == 0)?
                          (int)(font.recs[index].width*scale_factor + spacing):
                          (int)(font.chars[index].advance_x*scale_factor + spacing);
        }

        // NOTE: When wrap is ON we first measure how much of the text we can draw before going outside of the rec container
        // We store this info in start_line and end_line, then we change states, draw the text between those two variables
        // and change states again and again recursively until the end of the text (or until we get outside of the container).
        // When wrap is OFF we don't need the measure state so we go to the drawing state immediately
        // and begin drawing on the next line before we can get outside the container.
        if (state == MEASURE_WORD_WRAP_STATE)
        {
            // TODO: there are multiple types of spaces in UNICODE, maybe it's a good idea to add support for more
            // See: http://jkorpela.fi/chars/spaces.html
            if ((letter == ' ') || (letter == '\t') || (letter == '\n')) end_line = i;

            if ((text_offset_x + glyph_width + 1) >= rec.width)
            {
                end_line = (end_line < 1)? i : end_line;
                if (i == end_line) end_line -= codepoint.bytes_processed;
                if ((start_line + codepoint.bytes_processed) == end_line) end_line = i - codepoint.bytes_processed;
                state = !state;
            }
            else if ((i + 1) == text_len)
            {
                end_line = i;
                state = !state;
            }
            else if (letter == '\n')
            {
                state = !state;
            }

            if (state == MEASURE_RESULT_STATE)
            {
                text_offset_x = 0;
                i = start_line;
                glyph_width = 0;

                // Save character position when we switch states
                int tmp = lastk;
                lastk = k - 1;
                k = tmp;
            }
        }
        else
        {
            if (letter == '\n')
            {
                if (!wrap)
                {
                    text_offset_y += (int)((font.base_size + font.base_size/2)*scale_factor);
                    text_offset_x = 0;
                }
            }
            else
            {
                if (!wrap && ((text_offset_x + glyph_width + 1) >= rec.width))
                {
                    text_offset_y += (int)((font.base_size + font.base_size/2)*scale_factor);
                    text_offset_x = 0;
                }

                if ((text_offset_y + (int)(font.base_size*scale_factor)) > rec.height) break;

                // Draw glyph
                if ((letter != ' ') && (letter != '\t'))
                {
                    rf_draw_texture_region(font.texture, font.recs[index],
                                           (rf_rec) {
                                               rec.x + text_offset_x + font.chars[index].offset_x * scale_factor,
                                               rec.y + text_offset_y + font.chars[index].offset_y * scale_factor,
                                               font.recs[index].width  * scale_factor,
                                               font.recs[index].height * scale_factor
                                           },
                                           (rf_vec2){ 0, 0 }, 0.0f, tint);
                }
            }

            if (wrap && (i == end_line))
            {
                text_offset_y += (int)((font.base_size + font.base_size/2)*scale_factor);
                text_offset_x = 0;
                start_line = end_line;
                end_line = -1;
                glyph_width = 0;
                k = lastk;
                state = !state;
            }
        }

        text_offset_x += glyph_width;
    }
}

RF_API void rf_draw_line3d(rf_vec3 start_pos, rf_vec3 end_pos, rf_color color)
{
    rf_gfx_begin(RF_LINES);
    rf_gfx_color4ub(color.r, color.g, color.b, color.a);
    rf_gfx_vertex3f(start_pos.x, start_pos.y, start_pos.z);
    rf_gfx_vertex3f(end_pos.x, end_pos.y, end_pos.z);
    rf_gfx_end();
}

// Draw a circle in 3D world space
RF_API void rf_draw_circle3d(rf_vec3 center, float radius, rf_vec3 rotation_axis, float rotationAngle, rf_color color)
{
    if (rf_gfx_check_buffer_limit(2 * 36)) rf_gfx_draw();

    rf_gfx_push_matrix();
    rf_gfx_translatef(center.x, center.y, center.z);
    rf_gfx_rotatef(rotationAngle, rotation_axis.x, rotation_axis.y, rotation_axis.z);

    rf_gfx_begin(RF_LINES);
    for (int i = 0; i < 360; i += 10)
    {
        rf_gfx_color4ub(color.r, color.g, color.b, color.a);

        rf_gfx_vertex3f(sinf(RF_DEG2RAD*i)*radius, cosf(RF_DEG2RAD*i)*radius, 0.0f);
        rf_gfx_vertex3f(sinf(RF_DEG2RAD*(i + 10))*radius, cosf(RF_DEG2RAD*(i + 10))*radius, 0.0f);
    }
    rf_gfx_end();
    rf_gfx_pop_matrix();
}

// Draw cube
// NOTE: Cube position is the center position
RF_API void rf_draw_cube(rf_vec3 position, float width, float height, float length, rf_color color)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    if (rf_gfx_check_buffer_limit(36)) rf_gfx_draw();

    rf_gfx_push_matrix();
    // NOTE: Transformation is applied in inverse order (scale -> rotate -> translate)
    rf_gfx_translatef(position.x, position.y, position.z);
    //rf_gfx_rotatef(45, 0, 1, 0);
    //rf_gfx_scalef(1.0f, 1.0f, 1.0f);   // NOTE: Vertices are directly scaled on definition

    rf_gfx_begin(RF_TRIANGLES);
    rf_gfx_color4ub(color.r, color.g, color.b, color.a);

    // Front face
    rf_gfx_vertex3f(x - width/2, y - height/2, z + length/2); // Bottom Left
    rf_gfx_vertex3f(x + width/2, y - height/2, z + length/2); // Bottom Right
    rf_gfx_vertex3f(x - width/2, y + height/2, z + length/2); // Top Left

    rf_gfx_vertex3f(x + width/2, y + height/2, z + length/2); // Top Right
    rf_gfx_vertex3f(x - width/2, y + height/2, z + length/2); // Top Left
    rf_gfx_vertex3f(x + width/2, y - height/2, z + length/2); // Bottom Right

    // Back face
    rf_gfx_vertex3f(x - width/2, y - height/2, z - length/2); // Bottom Left
    rf_gfx_vertex3f(x - width/2, y + height/2, z - length/2); // Top Left
    rf_gfx_vertex3f(x + width/2, y - height/2, z - length/2); // Bottom Right

    rf_gfx_vertex3f(x + width/2, y + height/2, z - length/2); // Top Right
    rf_gfx_vertex3f(x + width/2, y - height/2, z - length/2); // Bottom Right
    rf_gfx_vertex3f(x - width/2, y + height/2, z - length/2); // Top Left

    // Top face
    rf_gfx_vertex3f(x - width/2, y + height/2, z - length/2); // Top Left
    rf_gfx_vertex3f(x - width/2, y + height/2, z + length/2); // Bottom Left
    rf_gfx_vertex3f(x + width/2, y + height/2, z + length/2); // Bottom Right

    rf_gfx_vertex3f(x + width/2, y + height/2, z - length/2); // Top Right
    rf_gfx_vertex3f(x - width/2, y + height/2, z - length/2); // Top Left
    rf_gfx_vertex3f(x + width/2, y + height/2, z + length/2); // Bottom Right

    // Bottom face
    rf_gfx_vertex3f(x - width/2, y - height/2, z - length/2); // Top Left
    rf_gfx_vertex3f(x + width/2, y - height/2, z + length/2); // Bottom Right
    rf_gfx_vertex3f(x - width/2, y - height/2, z + length/2); // Bottom Left

    rf_gfx_vertex3f(x + width/2, y - height/2, z - length/2); // Top Right
    rf_gfx_vertex3f(x + width/2, y - height/2, z + length/2); // Bottom Right
    rf_gfx_vertex3f(x - width/2, y - height/2, z - length/2); // Top Left

    // Right face
    rf_gfx_vertex3f(x + width/2, y - height/2, z - length/2); // Bottom Right
    rf_gfx_vertex3f(x + width/2, y + height/2, z - length/2); // Top Right
    rf_gfx_vertex3f(x + width/2, y + height/2, z + length/2); // Top Left

    rf_gfx_vertex3f(x + width/2, y - height/2, z + length/2); // Bottom Left
    rf_gfx_vertex3f(x + width/2, y - height/2, z - length/2); // Bottom Right
    rf_gfx_vertex3f(x + width/2, y + height/2, z + length/2); // Top Left

    // Left face
    rf_gfx_vertex3f(x - width/2, y - height/2, z - length/2); // Bottom Right
    rf_gfx_vertex3f(x - width/2, y + height/2, z + length/2); // Top Left
    rf_gfx_vertex3f(x - width/2, y + height/2, z - length/2); // Top Right

    rf_gfx_vertex3f(x - width/2, y - height/2, z + length/2); // Bottom Left
    rf_gfx_vertex3f(x - width/2, y + height/2, z + length/2); // Top Left
    rf_gfx_vertex3f(x - width/2, y - height/2, z - length/2); // Bottom Right
    rf_gfx_end();
    rf_gfx_pop_matrix();
}

// Draw cube wires
RF_API void rf_draw_cube_wires(rf_vec3 position, float width, float height, float length, rf_color color)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    if (rf_gfx_check_buffer_limit(36)) rf_gfx_draw();

    rf_gfx_push_matrix();
    rf_gfx_translatef(position.x, position.y, position.z);

    rf_gfx_begin(RF_LINES);
    rf_gfx_color4ub(color.r, color.g, color.b, color.a);

    // Front Face -----------------------------------------------------
    // Bottom Line
    rf_gfx_vertex3f(x-width/2, y-height/2, z+length/2); // Bottom Left
    rf_gfx_vertex3f(x+width/2, y-height/2, z+length/2); // Bottom Right

    // Left Line
    rf_gfx_vertex3f(x+width/2, y-height/2, z+length/2); // Bottom Right
    rf_gfx_vertex3f(x+width/2, y+height/2, z+length/2); // Top Right

    // Top Line
    rf_gfx_vertex3f(x+width/2, y+height/2, z+length/2); // Top Right
    rf_gfx_vertex3f(x-width/2, y+height/2, z+length/2); // Top Left

    // Right Line
    rf_gfx_vertex3f(x-width/2, y+height/2, z+length/2); // Top Left
    rf_gfx_vertex3f(x-width/2, y-height/2, z+length/2); // Bottom Left

    // Back Face ------------------------------------------------------
    // Bottom Line
    rf_gfx_vertex3f(x-width/2, y-height/2, z-length/2); // Bottom Left
    rf_gfx_vertex3f(x+width/2, y-height/2, z-length/2); // Bottom Right

    // Left Line
    rf_gfx_vertex3f(x+width/2, y-height/2, z-length/2); // Bottom Right
    rf_gfx_vertex3f(x+width/2, y+height/2, z-length/2); // Top Right

    // Top Line
    rf_gfx_vertex3f(x+width/2, y+height/2, z-length/2); // Top Right
    rf_gfx_vertex3f(x-width/2, y+height/2, z-length/2); // Top Left

    // Right Line
    rf_gfx_vertex3f(x-width/2, y+height/2, z-length/2); // Top Left
    rf_gfx_vertex3f(x-width/2, y-height/2, z-length/2); // Bottom Left

    // Top Face -------------------------------------------------------
    // Left Line
    rf_gfx_vertex3f(x-width/2, y+height/2, z+length/2); // Top Left Front
    rf_gfx_vertex3f(x-width/2, y+height/2, z-length/2); // Top Left Back

    // Right Line
    rf_gfx_vertex3f(x+width/2, y+height/2, z+length/2); // Top Right Front
    rf_gfx_vertex3f(x+width/2, y+height/2, z-length/2); // Top Right Back

    // Bottom Face  ---------------------------------------------------
    // Left Line
    rf_gfx_vertex3f(x-width/2, y-height/2, z+length/2); // Top Left Front
    rf_gfx_vertex3f(x-width/2, y-height/2, z-length/2); // Top Left Back

    // Right Line
    rf_gfx_vertex3f(x+width/2, y-height/2, z+length/2); // Top Right Front
    rf_gfx_vertex3f(x+width/2, y-height/2, z-length/2); // Top Right Back
    rf_gfx_end();
    rf_gfx_pop_matrix();
}

// Draw cube
// NOTE: Cube position is the center position
RF_API void rf_draw_cube_texture(rf_texture2d texture, rf_vec3 position, float width, float height, float length, rf_color color)
{
    float x = position.x;
    float y = position.y;
    float z = position.z;

    if (rf_gfx_check_buffer_limit(36)) rf_gfx_draw();

    rf_gfx_enable_texture(texture.id);

    //rf_gfx_push_matrix();
    // NOTE: Transformation is applied in inverse order (scale -> rotate -> translate)
    //rf_gfx_translatef(2.0f, 0.0f, 0.0f);
    //rf_gfx_rotatef(45, 0, 1, 0);
    //rf_gfx_scalef(2.0f, 2.0f, 2.0f);

    rf_gfx_begin(RF_QUADS);
    rf_gfx_color4ub(color.r, color.g, color.b, color.a);
    // Front Face
    rf_gfx_normal3f(0.0f, 0.0f, 1.0f); // Normal Pointing Towards Viewer
    rf_gfx_tex_coord2f(0.0f, 0.0f); rf_gfx_vertex3f(x - width/2, y - height/2, z + length/2); // Bottom Left Of The rf_texture and Quad
    rf_gfx_tex_coord2f(1.0f, 0.0f); rf_gfx_vertex3f(x + width/2, y - height/2, z + length/2); // Bottom Right Of The rf_texture and Quad
    rf_gfx_tex_coord2f(1.0f, 1.0f); rf_gfx_vertex3f(x + width/2, y + height/2, z + length/2); // Top Right Of The rf_texture and Quad
    rf_gfx_tex_coord2f(0.0f, 1.0f); rf_gfx_vertex3f(x - width/2, y + height/2, z + length/2); // Top Left Of The rf_texture and Quad
    // Back Face
    rf_gfx_normal3f(0.0f, 0.0f, - 1.0f); // Normal Pointing Away From Viewer
    rf_gfx_tex_coord2f(1.0f, 0.0f); rf_gfx_vertex3f(x - width/2, y - height/2, z - length/2); // Bottom Right Of The rf_texture and Quad
    rf_gfx_tex_coord2f(1.0f, 1.0f); rf_gfx_vertex3f(x - width/2, y + height/2, z - length/2); // Top Right Of The rf_texture and Quad
    rf_gfx_tex_coord2f(0.0f, 1.0f); rf_gfx_vertex3f(x + width/2, y + height/2, z - length/2); // Top Left Of The rf_texture and Quad
    rf_gfx_tex_coord2f(0.0f, 0.0f); rf_gfx_vertex3f(x + width/2, y - height/2, z - length/2); // Bottom Left Of The rf_texture and Quad
    // Top Face
    rf_gfx_normal3f(0.0f, 1.0f, 0.0f); // Normal Pointing Up
    rf_gfx_tex_coord2f(0.0f, 1.0f); rf_gfx_vertex3f(x - width/2, y + height/2, z - length/2); // Top Left Of The rf_texture and Quad
    rf_gfx_tex_coord2f(0.0f, 0.0f); rf_gfx_vertex3f(x - width/2, y + height/2, z + length/2); // Bottom Left Of The rf_texture and Quad
    rf_gfx_tex_coord2f(1.0f, 0.0f); rf_gfx_vertex3f(x + width/2, y + height/2, z + length/2); // Bottom Right Of The rf_texture and Quad
    rf_gfx_tex_coord2f(1.0f, 1.0f); rf_gfx_vertex3f(x + width/2, y + height/2, z - length/2); // Top Right Of The rf_texture and Quad
    // Bottom Face
    rf_gfx_normal3f(0.0f, - 1.0f, 0.0f); // Normal Pointing Down
    rf_gfx_tex_coord2f(1.0f, 1.0f); rf_gfx_vertex3f(x - width/2, y - height/2, z - length/2); // Top Right Of The rf_texture and Quad
    rf_gfx_tex_coord2f(0.0f, 1.0f); rf_gfx_vertex3f(x + width/2, y - height/2, z - length/2); // Top Left Of The rf_texture and Quad
    rf_gfx_tex_coord2f(0.0f, 0.0f); rf_gfx_vertex3f(x + width/2, y - height/2, z + length/2); // Bottom Left Of The rf_texture and Quad
    rf_gfx_tex_coord2f(1.0f, 0.0f); rf_gfx_vertex3f(x - width/2, y - height/2, z + length/2); // Bottom Right Of The rf_texture and Quad
    // Right face
    rf_gfx_normal3f(1.0f, 0.0f, 0.0f); // Normal Pointing Right
    rf_gfx_tex_coord2f(1.0f, 0.0f); rf_gfx_vertex3f(x + width/2, y - height/2, z - length/2); // Bottom Right Of The rf_texture and Quad
    rf_gfx_tex_coord2f(1.0f, 1.0f); rf_gfx_vertex3f(x + width/2, y + height/2, z - length/2); // Top Right Of The rf_texture and Quad
    rf_gfx_tex_coord2f(0.0f, 1.0f); rf_gfx_vertex3f(x + width/2, y + height/2, z + length/2); // Top Left Of The rf_texture and Quad
    rf_gfx_tex_coord2f(0.0f, 0.0f); rf_gfx_vertex3f(x + width/2, y - height/2, z + length/2); // Bottom Left Of The rf_texture and Quad
    // Left Face
    rf_gfx_normal3f(-1.0f, 0.0f, 0.0f); // Normal Pointing Left
    rf_gfx_tex_coord2f(0.0f, 0.0f); rf_gfx_vertex3f(x - width/2, y - height/2, z - length/2); // Bottom Left Of The rf_texture and Quad
    rf_gfx_tex_coord2f(1.0f, 0.0f); rf_gfx_vertex3f(x - width/2, y - height/2, z + length/2); // Bottom Right Of The rf_texture and Quad
    rf_gfx_tex_coord2f(1.0f, 1.0f); rf_gfx_vertex3f(x - width/2, y + height/2, z + length/2); // Top Right Of The rf_texture and Quad
    rf_gfx_tex_coord2f(0.0f, 1.0f); rf_gfx_vertex3f(x - width/2, y + height/2, z - length/2); // Top Left Of The rf_texture and Quad
    rf_gfx_end();
    //rf_gfx_pop_matrix();

    rf_gfx_disable_texture();
}

// Draw sphere
RF_API void rf_draw_sphere(rf_vec3 center_pos, float radius, rf_color color)
{
    rf_draw_sphere_ex(center_pos, radius, 16, 16, color);
}

// Draw sphere with extended parameters
RF_API void rf_draw_sphere_ex(rf_vec3 center_pos, float radius, int rings, int slices, rf_color color)
{
    int num_vertex = (rings + 2)*slices*6;
    if (rf_gfx_check_buffer_limit(num_vertex)) rf_gfx_draw();

    rf_gfx_push_matrix();
    // NOTE: Transformation is applied in inverse order (scale -> translate)
    rf_gfx_translatef(center_pos.x, center_pos.y, center_pos.z);
    rf_gfx_scalef(radius, radius, radius);

    rf_gfx_begin(RF_TRIANGLES);
    rf_gfx_color4ub(color.r, color.g, color.b, color.a);

    for (int i = 0; i < (rings + 2); i++)
    {
        for (int j = 0; j < slices; j++)
        {
            rf_gfx_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*i))*sinf(RF_DEG2RAD*(j * 360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*i)),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*i))*cosf(RF_DEG2RAD*(j * 360/slices)));
            rf_gfx_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(RF_DEG2RAD*((j+1) * 360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(RF_DEG2RAD*((j+1) * 360/slices)));
            rf_gfx_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(RF_DEG2RAD*(j * 360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(RF_DEG2RAD*(j * 360/slices)));

            rf_gfx_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*i))*sinf(RF_DEG2RAD*(j * 360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*i)),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*i))*cosf(RF_DEG2RAD*(j * 360/slices)));
            rf_gfx_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i)))*sinf(RF_DEG2RAD*((j+1) * 360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*(i))),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i)))*cosf(RF_DEG2RAD*((j+1) * 360/slices)));
            rf_gfx_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(RF_DEG2RAD*((j+1) * 360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(RF_DEG2RAD*((j+1) * 360/slices)));
        }
    }
    rf_gfx_end();
    rf_gfx_pop_matrix();
}

// Draw sphere wires
RF_API void rf_draw_sphere_wires(rf_vec3 center_pos, float radius, int rings, int slices, rf_color color)
{
    int num_vertex = (rings + 2)*slices*6;
    if (rf_gfx_check_buffer_limit(num_vertex)) rf_gfx_draw();

    rf_gfx_push_matrix();
    // NOTE: Transformation is applied in inverse order (scale -> translate)
    rf_gfx_translatef(center_pos.x, center_pos.y, center_pos.z);
    rf_gfx_scalef(radius, radius, radius);

    rf_gfx_begin(RF_LINES);
    rf_gfx_color4ub(color.r, color.g, color.b, color.a);

    for (int i = 0; i < (rings + 2); i++)
    {
        for (int j = 0; j < slices; j++)
        {
            rf_gfx_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*i))*sinf(RF_DEG2RAD*(j * 360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*i)),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*i))*cosf(RF_DEG2RAD*(j * 360/slices)));
            rf_gfx_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(RF_DEG2RAD*((j+1) * 360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(RF_DEG2RAD*((j+1) * 360/slices)));

            rf_gfx_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(RF_DEG2RAD*((j+1) * 360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(RF_DEG2RAD*((j+1) * 360/slices)));
            rf_gfx_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(RF_DEG2RAD*(j * 360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(RF_DEG2RAD*(j * 360/slices)));

            rf_gfx_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(RF_DEG2RAD*(j * 360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(RF_DEG2RAD*(j * 360/slices)));

            rf_gfx_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*i))*sinf(RF_DEG2RAD*(j * 360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*i)),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*i))*cosf(RF_DEG2RAD*(j * 360/slices)));
        }
    }

    rf_gfx_end();
    rf_gfx_pop_matrix();
}

// Draw a cylinder
// NOTE: It could be also used for pyramid and cone
RF_API void rf_draw_cylinder(rf_vec3 position, float radius_top, float radius_bottom, float height, int sides, rf_color color)
{
    if (sides < 3) sides = 3;

    int num_vertex = sides*6;
    if (rf_gfx_check_buffer_limit(num_vertex)) rf_gfx_draw();

    rf_gfx_push_matrix();
    rf_gfx_translatef(position.x, position.y, position.z);

    rf_gfx_begin(RF_TRIANGLES);
    rf_gfx_color4ub(color.r, color.g, color.b, color.a);

    if (radius_top > 0)
    {
        // Draw Body -------------------------------------------------------------------------------------
        for (int i = 0; i < 360; i += 360/sides)
        {
            rf_gfx_vertex3f(sinf(RF_DEG2RAD*i)*radius_bottom, 0, cosf(RF_DEG2RAD*i)*radius_bottom); //Bottom Left
            rf_gfx_vertex3f(sinf(RF_DEG2RAD*(i + 360/sides))*radius_bottom, 0, cosf(RF_DEG2RAD*(i + 360/sides))*radius_bottom); //Bottom Right
            rf_gfx_vertex3f(sinf(RF_DEG2RAD*(i + 360/sides))*radius_top, height, cosf(RF_DEG2RAD*(i + 360/sides))*radius_top); //Top Right

            rf_gfx_vertex3f(sinf(RF_DEG2RAD*i)*radius_top, height, cosf(RF_DEG2RAD*i)*radius_top); //Top Left
            rf_gfx_vertex3f(sinf(RF_DEG2RAD*i)*radius_bottom, 0, cosf(RF_DEG2RAD*i)*radius_bottom); //Bottom Left
            rf_gfx_vertex3f(sinf(RF_DEG2RAD*(i + 360/sides))*radius_top, height, cosf(RF_DEG2RAD*(i + 360/sides))*radius_top); //Top Right
        }

        // Draw Cap --------------------------------------------------------------------------------------
        for (int i = 0; i < 360; i += 360/sides)
        {
            rf_gfx_vertex3f(0, height, 0);
            rf_gfx_vertex3f(sinf(RF_DEG2RAD*i)*radius_top, height, cosf(RF_DEG2RAD*i)*radius_top);
            rf_gfx_vertex3f(sinf(RF_DEG2RAD*(i + 360/sides))*radius_top, height, cosf(RF_DEG2RAD*(i + 360/sides))*radius_top);
        }
    }
    else
    {
        // Draw Cone -------------------------------------------------------------------------------------
        for (int i = 0; i < 360; i += 360/sides)
        {
            rf_gfx_vertex3f(0, height, 0);
            rf_gfx_vertex3f(sinf(RF_DEG2RAD*i)*radius_bottom, 0, cosf(RF_DEG2RAD*i)*radius_bottom);
            rf_gfx_vertex3f(sinf(RF_DEG2RAD*(i + 360/sides))*radius_bottom, 0, cosf(RF_DEG2RAD*(i + 360/sides))*radius_bottom);
        }
    }

    // Draw Base -----------------------------------------------------------------------------------------
    for (int i = 0; i < 360; i += 360/sides)
    {
        rf_gfx_vertex3f(0, 0, 0);
        rf_gfx_vertex3f(sinf(RF_DEG2RAD*(i + 360/sides))*radius_bottom, 0, cosf(RF_DEG2RAD*(i + 360/sides))*radius_bottom);
        rf_gfx_vertex3f(sinf(RF_DEG2RAD*i)*radius_bottom, 0, cosf(RF_DEG2RAD*i)*radius_bottom);
    }

    rf_gfx_end();
    rf_gfx_pop_matrix();
}

// Draw a wired cylinder
// NOTE: It could be also used for pyramid and cone
RF_API void rf_draw_cylinder_wires(rf_vec3 position, float radius_top, float radius_bottom, float height, int sides, rf_color color)
{
    if (sides < 3) sides = 3;

    int num_vertex = sides*8;
    if (rf_gfx_check_buffer_limit(num_vertex)) rf_gfx_draw();

    rf_gfx_push_matrix();
    rf_gfx_translatef(position.x, position.y, position.z);

    rf_gfx_begin(RF_LINES);
    rf_gfx_color4ub(color.r, color.g, color.b, color.a);

    for (int i = 0; i < 360; i += 360/sides)
    {
        rf_gfx_vertex3f(sinf(RF_DEG2RAD*i)*radius_bottom, 0, cosf(RF_DEG2RAD*i)*radius_bottom);
        rf_gfx_vertex3f(sinf(RF_DEG2RAD*(i + 360/sides))*radius_bottom, 0, cosf(RF_DEG2RAD*(i + 360/sides))*radius_bottom);

        rf_gfx_vertex3f(sinf(RF_DEG2RAD*(i + 360/sides))*radius_bottom, 0, cosf(RF_DEG2RAD*(i + 360/sides))*radius_bottom);
        rf_gfx_vertex3f(sinf(RF_DEG2RAD*(i + 360/sides))*radius_top, height, cosf(RF_DEG2RAD*(i + 360/sides))*radius_top);

        rf_gfx_vertex3f(sinf(RF_DEG2RAD*(i + 360/sides))*radius_top, height, cosf(RF_DEG2RAD*(i + 360/sides))*radius_top);
        rf_gfx_vertex3f(sinf(RF_DEG2RAD*i)*radius_top, height, cosf(RF_DEG2RAD*i)*radius_top);

        rf_gfx_vertex3f(sinf(RF_DEG2RAD*i)*radius_top, height, cosf(RF_DEG2RAD*i)*radius_top);
        rf_gfx_vertex3f(sinf(RF_DEG2RAD*i)*radius_bottom, 0, cosf(RF_DEG2RAD*i)*radius_bottom);
    }
    rf_gfx_end();
    rf_gfx_pop_matrix();
}

// Draw a plane
RF_API void rf_draw_plane(rf_vec3 center_pos, rf_vec2 size, rf_color color)
{
    if (rf_gfx_check_buffer_limit(4)) rf_gfx_draw();

    // NOTE: Plane is always created on XZ ground
    rf_gfx_push_matrix();
    rf_gfx_translatef(center_pos.x, center_pos.y, center_pos.z);
    rf_gfx_scalef(size.x, 1.0f, size.y);

    rf_gfx_begin(RF_QUADS);
    rf_gfx_color4ub(color.r, color.g, color.b, color.a);
    rf_gfx_normal3f(0.0f, 1.0f, 0.0f);

    rf_gfx_vertex3f(-0.5f, 0.0f, -0.5f);
    rf_gfx_vertex3f(-0.5f, 0.0f, 0.5f);
    rf_gfx_vertex3f(0.5f, 0.0f, 0.5f);
    rf_gfx_vertex3f(0.5f, 0.0f, -0.5f);
    rf_gfx_end();
    rf_gfx_pop_matrix();
}

// Draw a ray line
RF_API void rf_draw_ray(rf_ray ray, rf_color color)
{
    float scale = 10000;

    rf_gfx_begin(RF_LINES);
    rf_gfx_color4ub(color.r, color.g, color.b, color.a);
    rf_gfx_color4ub(color.r, color.g, color.b, color.a);

    rf_gfx_vertex3f(ray.position.x, ray.position.y, ray.position.z);
    rf_gfx_vertex3f(ray.position.x + ray.direction.x*scale, ray.position.y + ray.direction.y*scale, ray.position.z + ray.direction.z*scale);
    rf_gfx_end();
}

// Draw a grid centered at (0, 0, 0)
RF_API void rf_draw_grid(int slices, float spacing)
{
    int half_slices = slices/2;

    if (rf_gfx_check_buffer_limit(slices * 4)) rf_gfx_draw();

    rf_gfx_begin(RF_LINES);
    for (int i = -half_slices; i <= half_slices; i++)
    {
        if (i == 0)
        {
            rf_gfx_color3f(0.5f, 0.5f, 0.5f);
            rf_gfx_color3f(0.5f, 0.5f, 0.5f);
            rf_gfx_color3f(0.5f, 0.5f, 0.5f);
            rf_gfx_color3f(0.5f, 0.5f, 0.5f);
        }
        else
        {
            rf_gfx_color3f(0.75f, 0.75f, 0.75f);
            rf_gfx_color3f(0.75f, 0.75f, 0.75f);
            rf_gfx_color3f(0.75f, 0.75f, 0.75f);
            rf_gfx_color3f(0.75f, 0.75f, 0.75f);
        }

        rf_gfx_vertex3f((float)i*spacing, 0.0f, (float)-half_slices*spacing);
        rf_gfx_vertex3f((float)i*spacing, 0.0f, (float)half_slices*spacing);

        rf_gfx_vertex3f((float)-half_slices*spacing, 0.0f, (float)i*spacing);
        rf_gfx_vertex3f((float)half_slices*spacing, 0.0f, (float)i*spacing);
    }
    rf_gfx_end();
}

// Draw gizmo
RF_API void rf_draw_gizmo(rf_vec3 position)
{
    // NOTE: RGB = XYZ
    float length = 1.0f;

    rf_gfx_push_matrix();
    rf_gfx_translatef(position.x, position.y, position.z);
    rf_gfx_scalef(length, length, length);

    rf_gfx_begin(RF_LINES);
    rf_gfx_color3f(1.0f, 0.0f, 0.0f); rf_gfx_vertex3f(0.0f, 0.0f, 0.0f);
    rf_gfx_color3f(1.0f, 0.0f, 0.0f); rf_gfx_vertex3f(1.0f, 0.0f, 0.0f);

    rf_gfx_color3f(0.0f, 1.0f, 0.0f); rf_gfx_vertex3f(0.0f, 0.0f, 0.0f);
    rf_gfx_color3f(0.0f, 1.0f, 0.0f); rf_gfx_vertex3f(0.0f, 1.0f, 0.0f);

    rf_gfx_color3f(0.0f, 0.0f, 1.0f); rf_gfx_vertex3f(0.0f, 0.0f, 0.0f);
    rf_gfx_color3f(0.0f, 0.0f, 1.0f); rf_gfx_vertex3f(0.0f, 0.0f, 1.0f);
    rf_gfx_end();
    rf_gfx_pop_matrix();
}

// Draw a model with extended parameters
RF_API void rf_draw_model(rf_model model, rf_vec3 position, rf_vec3 rotation_axis, float rotationAngle, rf_vec3 scale, rf_color tint)
{
    // Calculate transformation matrix from function parameters
    // Get transform matrix (rotation -> scale -> translation)
    rf_mat mat_scale = rf_mat_scale(scale.x, scale.y, scale.z);
    rf_mat mat_rotation = rf_mat_rotate(rotation_axis, rotationAngle * RF_DEG2RAD);
    rf_mat mat_translation = rf_mat_translate(position.x, position.y, position.z);

    rf_mat mat_transform = rf_mat_mul(rf_mat_mul(mat_scale, mat_rotation), mat_translation);

    // Combine model transformation matrix (model.transform) with matrix generated by function parameters (mat_transform)
    model.transform = rf_mat_mul(model.transform, mat_transform);

    for (int i = 0; i < model.mesh_count; i++)
    {
        // TODO: Review color + tint premultiplication mechanism
        rf_color color = model.materials[model.mesh_material[i]].maps[RF_MAP_DIFFUSE].color;

        rf_color color_tint = RF_WHITE;
        color_tint.r = (((float)color.r/255.0)*((float)tint.r/255.0))*255;
        color_tint.g = (((float)color.g/255.0)*((float)tint.g/255.0))*255;
        color_tint.b = (((float)color.b/255.0)*((float)tint.b/255.0))*255;
        color_tint.a = (((float)color.a/255.0)*((float)tint.a/255.0))*255;

        model.materials[model.mesh_material[i]].maps[RF_MAP_DIFFUSE].color = color_tint;
        rf_gfx_draw_mesh(model.meshes[i], model.materials[model.mesh_material[i]], model.transform);
        model.materials[model.mesh_material[i]].maps[RF_MAP_DIFFUSE].color = color;
    }
}

// Draw a model wires (with texture if set) with extended parameters
RF_API void rf_draw_model_wires(rf_model model, rf_vec3 position, rf_vec3 rotation_axis, float rotationAngle, rf_vec3 scale, rf_color tint)
{
    rf_gfx_enable_wire_mode();

    rf_draw_model(model, position, rotation_axis, rotationAngle, scale, tint);

    rf_gfx_disable_wire_mode();
}

// Draw a bounding box with wires
RF_API void rf_draw_bounding_box(rf_bounding_box box, rf_color color)
{
    rf_vec3 size;

    size.x = (float)fabs(box.max.x - box.min.x);
    size.y = (float)fabs(box.max.y - box.min.y);
    size.z = (float)fabs(box.max.z - box.min.z);

    rf_vec3 center = {box.min.x + size.x / 2.0f, box.min.y + size.y / 2.0f, box.min.z + size.z / 2.0f };

    rf_draw_cube_wires(center, size.x, size.y, size.z, color);
}

// Draw a billboard
RF_API void rf_draw_billboard(rf_camera3d camera, rf_texture2d texture, rf_vec3 center, float size, rf_color tint)
{
    rf_rec source_rec = {0.0f, 0.0f, (float)texture.width, (float)texture.height };

    rf_draw_billboard_rec(camera, texture, source_rec, center, size, tint);
}

// Draw a billboard (part of a texture defined by a rectangle)
RF_API void rf_draw_billboard_rec(rf_camera3d camera, rf_texture2d texture, rf_rec source_rec, rf_vec3 center, float size, rf_color tint)
{
    // NOTE: Billboard size will maintain source_rec aspect ratio, size will represent billboard width
    rf_vec2 size_ratio = {size, size * (float)source_rec.height / source_rec.width };

    rf_mat mat_view = rf_mat_look_at(camera.position, camera.target, camera.up);

    rf_vec3 right = {mat_view.m0, mat_view.m4, mat_view.m8 };
    //rf_vec3 up = { mat_view.m1, mat_view.m5, mat_view.m9 };

    // NOTE: Billboard locked on axis-Y
    rf_vec3 up = {0.0f, 1.0f, 0.0f };
    /*
        a-------b
        |       |
        |   *   |
        |       |
        d-------c
    */
    right = rf_vec3_scale(right, size_ratio.x / 2);
    up = rf_vec3_scale(up, size_ratio.y / 2);

    rf_vec3 p1 = rf_vec3_add(right, up);
    rf_vec3 p2 = rf_vec3_sub(right, up);

    rf_vec3 a = rf_vec3_sub(center, p2);
    rf_vec3 b = rf_vec3_add(center, p1);
    rf_vec3 c = rf_vec3_add(center, p2);
    rf_vec3 d = rf_vec3_sub(center, p1);

    if (rf_gfx_check_buffer_limit(4)) rf_gfx_draw();

    rf_gfx_enable_texture(texture.id);

    rf_gfx_begin(RF_QUADS);
    rf_gfx_color4ub(tint.r, tint.g, tint.b, tint.a);

    // Bottom-left corner for texture and quad
    rf_gfx_tex_coord2f((float)source_rec.x/texture.width, (float)source_rec.y/texture.height);
    rf_gfx_vertex3f(a.x, a.y, a.z);

    // Top-left corner for texture and quad
    rf_gfx_tex_coord2f((float)source_rec.x/texture.width, (float)(source_rec.y + source_rec.height)/texture.height);
    rf_gfx_vertex3f(d.x, d.y, d.z);

    // Top-right corner for texture and quad
    rf_gfx_tex_coord2f((float)(source_rec.x + source_rec.width)/texture.width, (float)(source_rec.y + source_rec.height)/texture.height);
    rf_gfx_vertex3f(c.x, c.y, c.z);

    // Bottom-right corner for texture and quad
    rf_gfx_tex_coord2f((float)(source_rec.x + source_rec.width)/texture.width, (float)source_rec.y/texture.height);
    rf_gfx_vertex3f(b.x, b.y, b.z);
    rf_gfx_end();

    rf_gfx_disable_texture();
}
//endregion

//region image
//region extract image data functions

// Returns the size of the image in bytes
RF_API int rf_image_size(rf_image image)
{
    int size = 0;
    int width = image.width;
    int height = image.height;

    for (int i = 0; i < image.mipmaps; i++)
    {
        size += rf_get_buffer_size_for_pixel_format(image.width, image.height, image.format);

        width /= 2;
        height /= 2;

        // Security check for NPOT textures
        if (width < 1) width = 1;
        if (height < 1) height = 1;
    }

    return size;
}

// Get pixel data from image in the form of rf_color struct array
RF_API rf_color* rf_get_image_pixel_data(rf_image image, rf_allocator allocator)
{
    rf_color* pixels = (rf_color*) RF_ALLOC(allocator, image.width * image.height * sizeof(rf_color));

    if (pixels == NULL) return pixels;

    if (image.format >= RF_COMPRESSED_DXT1_RGB) RF_LOG_V(RF_LOG_WARNING, "Pixel data retrieval not supported for compressed image formats");
    else
    {
        if ((image.format == RF_UNCOMPRESSED_R32) ||
            (image.format == RF_UNCOMPRESSED_R32G32B32) ||
            (image.format == RF_UNCOMPRESSED_R32G32B32A32)) RF_LOG_V(RF_LOG_WARNING, "32bit pixel format converted to 8bit per channel");

        for (int i = 0, k = 0; i < image.width*image.height; i++)
        {
            switch (image.format)
            {
                case RF_UNCOMPRESSED_GRAYSCALE:
                {
                    pixels[i].r = ((unsigned char* )image.data)[i];
                    pixels[i].g = ((unsigned char* )image.data)[i];
                    pixels[i].b = ((unsigned char* )image.data)[i];
                    pixels[i].a = 255;

                } break;
                case RF_UNCOMPRESSED_GRAY_ALPHA:
                {
                    pixels[i].r = ((unsigned char* )image.data)[k];
                    pixels[i].g = ((unsigned char* )image.data)[k];
                    pixels[i].b = ((unsigned char* )image.data)[k];
                    pixels[i].a = ((unsigned char* )image.data)[k + 1];

                    k += 2;
                } break;
                case RF_UNCOMPRESSED_R5G5B5A1:
                {
                    unsigned short pixel = ((unsigned short *)image.data)[i];

                    pixels[i].r = (unsigned char)((float)((pixel & 0b1111100000000000) >> 11)*(255/31));
                    pixels[i].g = (unsigned char)((float)((pixel & 0b0000011111000000) >> 6)*(255/31));
                    pixels[i].b = (unsigned char)((float)((pixel & 0b0000000000111110) >> 1)*(255/31));
                    pixels[i].a = (unsigned char)((pixel & 0b0000000000000001)*255);

                } break;
                case RF_UNCOMPRESSED_R5G6B5:
                {
                    unsigned short pixel = ((unsigned short *)image.data)[i];

                    pixels[i].r = (unsigned char)((float)((pixel & 0b1111100000000000) >> 11)*(255/31));
                    pixels[i].g = (unsigned char)((float)((pixel & 0b0000011111100000) >> 5)*(255/63));
                    pixels[i].b = (unsigned char)((float)(pixel & 0b0000000000011111)*(255/31));
                    pixels[i].a = 255;

                } break;
                case RF_UNCOMPRESSED_R4G4B4A4:
                {
                    unsigned short pixel = ((unsigned short *)image.data)[i];

                    pixels[i].r = (unsigned char)((float)((pixel & 0b1111000000000000) >> 12)*(255/15));
                    pixels[i].g = (unsigned char)((float)((pixel & 0b0000111100000000) >> 8)*(255/15));
                    pixels[i].b = (unsigned char)((float)((pixel & 0b0000000011110000) >> 4)*(255/15));
                    pixels[i].a = (unsigned char)((float)(pixel & 0b0000000000001111)*(255/15));

                } break;
                case RF_UNCOMPRESSED_R8G8B8A8:
                {
                    pixels[i].r = ((unsigned char* )image.data)[k];
                    pixels[i].g = ((unsigned char* )image.data)[k + 1];
                    pixels[i].b = ((unsigned char* )image.data)[k + 2];
                    pixels[i].a = ((unsigned char* )image.data)[k + 3];

                    k += 4;
                } break;
                case RF_UNCOMPRESSED_R8G8B8:
                {
                    pixels[i].r = (unsigned char)((unsigned char* )image.data)[k];
                    pixels[i].g = (unsigned char)((unsigned char* )image.data)[k + 1];
                    pixels[i].b = (unsigned char)((unsigned char* )image.data)[k + 2];
                    pixels[i].a = 255;

                    k += 3;
                } break;
                case RF_UNCOMPRESSED_R32:
                {
                    pixels[i].r = (unsigned char)(((float* )image.data)[k]*255.0f);
                    pixels[i].g = 0;
                    pixels[i].b = 0;
                    pixels[i].a = 255;

                } break;
                case RF_UNCOMPRESSED_R32G32B32:
                {
                    pixels[i].r = (unsigned char)(((float* )image.data)[k]*255.0f);
                    pixels[i].g = (unsigned char)(((float* )image.data)[k + 1]*255.0f);
                    pixels[i].b = (unsigned char)(((float* )image.data)[k + 2]*255.0f);
                    pixels[i].a = 255;

                    k += 3;
                } break;
                case RF_UNCOMPRESSED_R32G32B32A32:
                {
                    pixels[i].r = (unsigned char)(((float* )image.data)[k]*255.0f);
                    pixels[i].g = (unsigned char)(((float* )image.data)[k]*255.0f);
                    pixels[i].b = (unsigned char)(((float* )image.data)[k]*255.0f);
                    pixels[i].a = (unsigned char)(((float* )image.data)[k]*255.0f);

                    k += 4;
                } break;
                default: break;
            }
        }
    }

    return pixels;
}

// Get pixel data from image as rf_vec4 array (float normalized)
RF_API rf_vec4* rf_get_image_data_normalized(rf_image image, rf_allocator allocator)
{
    rf_vec4*  pixels = (rf_vec4*) RF_ALLOC(allocator, image.width * image.height * sizeof(rf_vec4));

    if (image.format >= RF_COMPRESSED_DXT1_RGB) RF_LOG_V(RF_LOG_WARNING, "Pixel data retrieval not supported for compressed image formats");
    else
    {
        for (int i = 0, k = 0; i < image.width*image.height; i++)
        {
            switch (image.format)
            {
                case RF_UNCOMPRESSED_GRAYSCALE:
                {
                    pixels[i].x = (float)((unsigned char* )image.data)[i]/255.0f;
                    pixels[i].y = (float)((unsigned char* )image.data)[i]/255.0f;
                    pixels[i].z = (float)((unsigned char* )image.data)[i]/255.0f;
                    pixels[i].w = 1.0f;

                } break;
                case RF_UNCOMPRESSED_GRAY_ALPHA:
                {
                    pixels[i].x = (float)((unsigned char* )image.data)[k]/255.0f;
                    pixels[i].y = (float)((unsigned char* )image.data)[k]/255.0f;
                    pixels[i].z = (float)((unsigned char* )image.data)[k]/255.0f;
                    pixels[i].w = (float)((unsigned char* )image.data)[k + 1]/255.0f;

                    k += 2;
                } break;
                case RF_UNCOMPRESSED_R5G5B5A1:
                {
                    unsigned short pixel = ((unsigned short *)image.data)[i];

                    pixels[i].x = (float)((pixel & 0b1111100000000000) >> 11)*(1.0f/31);
                    pixels[i].y = (float)((pixel & 0b0000011111000000) >> 6)*(1.0f/31);
                    pixels[i].z = (float)((pixel & 0b0000000000111110) >> 1)*(1.0f/31);
                    pixels[i].w = ((pixel & 0b0000000000000001) == 0)? 0.0f : 1.0f;

                } break;
                case RF_UNCOMPRESSED_R5G6B5:
                {
                    unsigned short pixel = ((unsigned short *)image.data)[i];

                    pixels[i].x = (float)((pixel & 0b1111100000000000) >> 11)*(1.0f/31);
                    pixels[i].y = (float)((pixel & 0b0000011111100000) >> 5)*(1.0f/63);
                    pixels[i].z = (float)(pixel & 0b0000000000011111)*(1.0f/31);
                    pixels[i].w = 1.0f;

                } break;
                case RF_UNCOMPRESSED_R4G4B4A4:
                {
                    unsigned short pixel = ((unsigned short *)image.data)[i];

                    pixels[i].x = (float)((pixel & 0b1111000000000000) >> 12)*(1.0f/15);
                    pixels[i].y = (float)((pixel & 0b0000111100000000) >> 8)*(1.0f/15);
                    pixels[i].z = (float)((pixel & 0b0000000011110000) >> 4)*(1.0f/15);
                    pixels[i].w = (float)(pixel & 0b0000000000001111)*(1.0f/15);

                } break;
                case RF_UNCOMPRESSED_R8G8B8A8:
                {
                    pixels[i].x = (float)((unsigned char* )image.data)[k]/255.0f;
                    pixels[i].y = (float)((unsigned char* )image.data)[k + 1]/255.0f;
                    pixels[i].z = (float)((unsigned char* )image.data)[k + 2]/255.0f;
                    pixels[i].w = (float)((unsigned char* )image.data)[k + 3]/255.0f;

                    k += 4;
                } break;
                case RF_UNCOMPRESSED_R8G8B8:
                {
                    pixels[i].x = (float)((unsigned char* )image.data)[k]/255.0f;
                    pixels[i].y = (float)((unsigned char* )image.data)[k + 1]/255.0f;
                    pixels[i].z = (float)((unsigned char* )image.data)[k + 2]/255.0f;
                    pixels[i].w = 1.0f;

                    k += 3;
                } break;
                case RF_UNCOMPRESSED_R32:
                {
                    pixels[i].x = ((float* )image.data)[k];
                    pixels[i].y = 0.0f;
                    pixels[i].z = 0.0f;
                    pixels[i].w = 1.0f;

                } break;
                case RF_UNCOMPRESSED_R32G32B32:
                {
                    pixels[i].x = ((float* )image.data)[k];
                    pixels[i].y = ((float* )image.data)[k + 1];
                    pixels[i].z = ((float* )image.data)[k + 2];
                    pixels[i].w = 1.0f;

                    k += 3;
                } break;
                case RF_UNCOMPRESSED_R32G32B32A32:
                {
                    pixels[i].x = ((float* )image.data)[k];
                    pixels[i].y = ((float* )image.data)[k + 1];
                    pixels[i].z = ((float* )image.data)[k + 2];
                    pixels[i].w = ((float* )image.data)[k + 3];

                    k += 4;
                } break;
                default: break;
            }
        }
    }

    return pixels;
}

// Extract color palette from image to maximum size.
RF_API rf_color* rf_image_extract_palette(rf_image image, int max_palette_size, int* extract_count, rf_allocator allocator, rf_allocator temp_allocator)
{
    rf_color* pixels = rf_get_image_pixel_data(image, temp_allocator);
    rf_color* palette = (rf_color*) RF_ALLOC(allocator, max_palette_size * sizeof(rf_color));

    int pal_count = 0;
    for (int i = 0; i < max_palette_size; i++)
    {
        palette[i] = RF_BLANK; // Set all colors to RF_BLANK
    }

    for (int i = 0; i < image.width*image.height; i++)
    {
        if (pixels[i].a > 0)
        {
            bool color_in_palette = false;

            // Check if the color is already on palette
            for (int j = 0; j < max_palette_size; j++)
            {
                //If the colors are equal
                if (pixels[i].r == palette[j].r &&
                    pixels[i].g == palette[j].g &&
                    pixels[i].b == palette[j].b &&
                    pixels[i].a == palette[j].a)
                {
                    color_in_palette = true;
                    break;
                }
            }

            // Store color if not on the palette
            if (!color_in_palette)
            {
                palette[pal_count] = pixels[i]; // Add pixels[i] to palette
                pal_count++;

                // We reached the limit of colors supported by palette
                if (pal_count >= max_palette_size)
                {
                    i = image.width * image.height; // Finish palette get
                    RF_LOG_V(RF_LOG_WARNING, "rf_image palette is greater than %i colors!", max_palette_size);
                }
            }
        }
    }

    RF_FREE(temp_allocator, pixels);

    *extract_count = pal_count;

    return palette;
}

// Get image alpha border rectangle
RF_API rf_rec rf_get_image_alpha_border(rf_image image, float threshold, rf_allocator temp_allocator)
{
    rf_rec crop = { 0 };

    rf_color* pixels = rf_get_image_pixel_data(image, temp_allocator);

    if (pixels != NULL)
    {
        int x_min = 65536; // Define a big enough number
        int x_max = 0;
        int y_min = 65536;
        int y_max = 0;

        for (int y = 0; y < image.height; y++)
        {
            for (int x = 0; x < image.width; x++)
            {
                if (pixels[y*image.width + x].a > (unsigned char)(threshold * 255.0f))
                {
                    if (x < x_min) x_min = x;
                    if (x > x_max) x_max = x;
                    if (y < y_min) y_min = y;
                    if (y > y_max) y_max = y;
                }
            }
        }

        crop = (rf_rec) { x_min, y_min, (x_max + 1) - x_min, (y_max + 1) - y_min };

        RF_FREE(temp_allocator, pixels);
    }

    return crop;
}
//endregion

//region loading & unloading functions
// Load image from file into CPU memory (RAM)
RF_API rf_image rf_load_image_from_file(const char* filename, rf_allocator allocator, rf_allocator temp_allocator, rf_io_callbacks io)
{
    rf_image image = { 0 };

    if ((   rf_internal_is_file_extension(filename, ".png"))
        || (rf_internal_is_file_extension(filename, ".bmp"))
        || (rf_internal_is_file_extension(filename, ".tga"))
        || (rf_internal_is_file_extension(filename, ".gif"))
        || (rf_internal_is_file_extension(filename, ".pic"))
        || (rf_internal_is_file_extension(filename, ".psd")))
    {

        int img_width = 0;
        int img_height = 0;
        int img_bpp = 0;

        int file_size = io.get_file_size_proc(filename);
        unsigned char* image_file_buffer = (unsigned char*) RF_ALLOC(temp_allocator, file_size);
        io.read_file_into_buffer_proc(filename, image_file_buffer, file_size);

        if (image_file_buffer != NULL) //Todo(LucaSas): Better error handling here, check if the file was read correctly
        {
            // NOTE: Using stb_image to load images (Supports multiple image formats)
            RF_SET_STBI_ALLOCATOR(&allocator);
            {
                image.data = stbi_load_from_memory(image_file_buffer, file_size, &img_width, &img_height, &img_bpp, 0);
            }
            RF_SET_STBI_ALLOCATOR(NULL);

            image.width     = img_width;
            image.height    = img_height;
            image.mipmaps   = 1;
            image.allocator = allocator;

            if (img_bpp == 1)      image.format = RF_UNCOMPRESSED_GRAYSCALE;
            else if (img_bpp == 2) image.format = RF_UNCOMPRESSED_GRAY_ALPHA;
            else if (img_bpp == 3) image.format = RF_UNCOMPRESSED_R8G8B8;
            else if (img_bpp == 4) image.format = RF_UNCOMPRESSED_R8G8B8A8;
        }

        RF_FREE(temp_allocator, image_file_buffer);
    }
    else
    {
        RF_LOG_V(RF_LOG_WARNING, "[%s] rf_image fileformat not supported", filename);
    }

    if (image.data != NULL) RF_LOG_V(RF_LOG_INFO, "[%s] rf_image loaded successfully (%ix%i)", filename, image.width, image.height);
    else RF_LOG_V(RF_LOG_WARNING, "[%s] rf_image could not be loaded", filename);

    return image;
}

// Load image from file data into CPU memory (RAM)
RF_API rf_image rf_load_image_from_data(const void* data, int data_size, rf_allocator allocator)
{
    if (data == NULL || data_size == 0) return (rf_image) { 0 };

    int img_width   = 0;
    int img_height  = 0;
    int img_bpp     = 0;
    rf_image image = { 0 };

    // NOTE: Using stb_image to load images (Supports multiple image formats)
    RF_SET_STBI_ALLOCATOR(&allocator);
    {
        image.data = stbi_load_from_memory(data, data_size, &img_width, &img_height, &img_bpp, 0);
    }
    RF_SET_STBI_ALLOCATOR(NULL);

    image.width     = img_width;
    image.height    = img_height;
    image.mipmaps   = 1;
    image.allocator = allocator;

    if (img_bpp == 1)      image.format = RF_UNCOMPRESSED_GRAYSCALE;
    else if (img_bpp == 2) image.format = RF_UNCOMPRESSED_GRAY_ALPHA;
    else if (img_bpp == 3) image.format = RF_UNCOMPRESSED_R8G8B8;
    else if (img_bpp == 4) image.format = RF_UNCOMPRESSED_R8G8B8A8;

    if (image.data == NULL)
    {
        RF_LOG(RF_LOG_WARNING, "rf_image fileformat not supported or could not be loaded");
    }

    return image;
}

// Load image from rf_color array data (RGBA - 32bit)
RF_API rf_image rf_load_image_from_pixels(rf_color* pixels, int width, int height, rf_allocator allocator)
{
    rf_image image;
    image.data      = NULL;
    image.width     = width;
    image.height    = height;
    image.mipmaps   = 1;
    image.format    = RF_UNCOMPRESSED_R8G8B8A8;
    image.allocator = allocator;

    int k = 0;

    image.data = (unsigned char*) RF_ALLOC(allocator,image.width * image.height * 4 * sizeof(unsigned char));

    for (int i = 0; i < image.width * image.height * 4; i += 4)
    {
        ((unsigned char* )image.data)[i    ] = pixels[k].r;
        ((unsigned char* )image.data)[i + 1] = pixels[k].g;
        ((unsigned char* )image.data)[i + 2] = pixels[k].b;
        ((unsigned char* )image.data)[i + 3] = pixels[k].a;
        k++;
    }

    return image;
}

// Load image from raw data with parameters
RF_API rf_image rf_load_image_from_data_in_format(const void* data, int data_size, int width, int height, int format, rf_allocator allocator)
{
    rf_image src_image = { 0 };

    src_image.data = ((void*)data); //Safe const-cast
    src_image.width = width;
    src_image.height = height;
    src_image.mipmaps = 1;
    src_image.format = format;

    rf_image dst_image = rf_image_copy(src_image, allocator);

    return dst_image;
}

// Load image from .dds
RF_API rf_image rf_load_image_from_dds(const void* data, rf_allocator allocator)
{
    // Required extension:
    // GL_EXT_texture_compression_s3tc

    // Supported tokens (defined by extensions)
    // GL_COMPRESSED_RGB_S3TC_DXT1_EXT      0x83F0
    // GL_COMPRESSED_RGBA_S3TC_DXT1_EXT     0x83F1
    // GL_COMPRESSED_RGBA_S3TC_DXT3_EXT     0x83F2
    // GL_COMPRESSED_RGBA_S3TC_DXT5_EXT     0x83F3

#define FOURCC_DXT1 0x31545844  // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844  // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844  // Equivalent to "DXT5" in ASCII

    // DDS Pixel Format
    typedef struct {
        unsigned int size;
        unsigned int flags;
        unsigned int four_cc;
        unsigned int rgb_bit_count;
        unsigned int r_bit_mask;
        unsigned int g_bit_mask;
        unsigned int b_bit_mask;
        unsigned int a_bit_mask;
    } dds_pixel_format;

    // DDS Header (124 bytes)
    typedef struct {
        unsigned int size;
        unsigned int flags;
        unsigned int height;
        unsigned int width;
        unsigned int pitch_or_linear_size;
        unsigned int depth;
        unsigned int mipmap_count;
        unsigned int reserved_1[11];
        dds_pixel_format ddspf;
        unsigned int caps;
        unsigned int caps_2;
        unsigned int caps_3;
        unsigned int caps_4;
        unsigned int reserved_2;
    } dds_header;

    rf_image image = { 0 };

    // Verify the type of file
    char dds_header_id[4];
    int file_iterator = 0;
//        fread(ddsHeaderId, 4, 1, ddsFile);
    memcpy(dds_header_id, data + file_iterator, sizeof(dds_header_id));
    file_iterator += sizeof(dds_header_id);

    if ((dds_header_id[0] != 'D') || (dds_header_id[1] != 'D') || (dds_header_id[2] != 'S') || (dds_header_id[3] != ' '))
    {
        RF_LOG_V(RF_LOG_WARNING, "[%s] DDS file does not seem to be a valid image", "filename");
    }
    else
    {
        dds_header dds_header;

        // Get the image header
//            fread(&dds_header, sizeof(dds_header), 1, ddsFile);
        memcpy(&dds_header, data + file_iterator, sizeof(dds_header));
        file_iterator += sizeof(dds_header);

        RF_LOG_V(RF_LOG_DEBUG, "[%s] DDS file header size: %i", "fileName", sizeof(dds_header));
        RF_LOG_V(RF_LOG_DEBUG, "[%s] DDS file pixel format size: %i", "fileName", dds_header.ddspf.size);
        RF_LOG_V(RF_LOG_DEBUG, "[%s] DDS file pixel format flags: 0x%x", "fileName", dds_header.ddspf.flags);
        RF_LOG_V(RF_LOG_DEBUG, "[%s] DDS file format: 0x%x", "fileName", dds_header.ddspf.fourCC);
        RF_LOG_V(RF_LOG_DEBUG, "[%s] DDS file bit count: 0x%x", "fileName", dds_header.ddspf.rgbBitCount);

        image.width = dds_header.width;
        image.height = dds_header.height;

        if (dds_header.mipmap_count == 0) image.mipmaps = 1;      // Parameter not used
        else image.mipmaps = dds_header.mipmap_count;

        if (dds_header.ddspf.rgb_bit_count == 16)     // 16bit mode, no compressed
        {
            if (dds_header.ddspf.flags == 0x40)         // no alpha channel
            {
                image.data = (unsigned short *) RF_ALLOC(allocator, image.width * image.height * sizeof(unsigned short));
//                    fread(image.data, image.width*image.height*sizeof(unsigned short), 1, dds_file);
                memcpy(image.data, data + file_iterator, image.width * image.height * sizeof(unsigned short));
                file_iterator += image.width * image.height * sizeof(unsigned short);

                image.format = RF_UNCOMPRESSED_R5G6B5;
            }
            else if (dds_header.ddspf.flags == 0x41)        // with alpha channel
            {
                if (dds_header.ddspf.a_bit_mask == 0x8000)    // 1bit alpha
                {
                    image.data = (unsigned short *) RF_ALLOC(allocator, image.width * image.height * sizeof(unsigned short));
//                        fread(image.data, image.width*image.height*sizeof(unsigned short), 1, dds_file);
                    memcpy(image.data, data + file_iterator, image.width * image.height * sizeof(unsigned short));
                    file_iterator += image.width * image.height * sizeof(unsigned short);

                    unsigned char alpha = 0;

                    // NOTE: Data comes as A1R5G5B5, it must be reordered to R5G5B5A1
                    for (int i = 0; i < image.width * image.height; i++)
                    {
                        alpha = ((unsigned short *)image.data)[i] >> 15;
                        ((unsigned short *)image.data)[i] = ((unsigned short *)image.data)[i] << 1;
                        ((unsigned short *)image.data)[i] += alpha;
                    }

                    image.format = RF_UNCOMPRESSED_R5G5B5A1;
                }
                else if (dds_header.ddspf.a_bit_mask == 0xf000)   // 4bit alpha
                {
                    image.data = (unsigned short *) RF_ALLOC(allocator, image.width * image.height * sizeof(unsigned short));
//                        fread(image.data, image.width*image.height*sizeof(unsigned short), 1, dds_file);
                    memcpy(image.data, data + file_iterator, image.width * image.height * sizeof(unsigned short));
                    file_iterator += image.width * image.height * sizeof(unsigned short);

                    unsigned char alpha = 0;

                    // NOTE: Data comes as A4R4G4B4, it must be reordered R4G4B4A4
                    for (int i = 0; i < image.width * image.height; i++)
                    {
                        alpha = ((unsigned short *)image.data)[i] >> 12;
                        ((unsigned short *)image.data)[i] = ((unsigned short *)image.data)[i] << 4;
                        ((unsigned short *)image.data)[i] += alpha;
                    }

                    image.format = RF_UNCOMPRESSED_R4G4B4A4;
                }
            }
        }

        if (dds_header.ddspf.flags == 0x40 && dds_header.ddspf.rgb_bit_count == 24)   // DDS_RGB, no compressed
        {
            // NOTE: not sure if this case exists...
            image.data = (unsigned char *) RF_ALLOC(allocator, image.width * image.height * 3 * sizeof(unsigned char));
//                fread(image.data, image.width*image.height*3, 1, dds_file);
            memcpy(image.data, data + file_iterator, image.width * image.height * 3 * sizeof(unsigned char));
            file_iterator += image.width * image.height * 3 * sizeof(unsigned char);

            image.format = RF_UNCOMPRESSED_R8G8B8;
        }
        else if (dds_header.ddspf.flags == 0x41 && dds_header.ddspf.rgb_bit_count == 32) // DDS_RGBA, no compressed
        {
            image.data = (unsigned char *) RF_ALLOC(allocator, image.width * image.height * 4 * sizeof(unsigned char));
//                fread(image.data, image.width*image.height*4, 1, dds_file);
            memcpy(image.data, data + file_iterator, image.width * image.height * 3 * sizeof(unsigned char));
            file_iterator += image.width * image.height * 3 * sizeof(unsigned char);

            unsigned char blue = 0;

            // NOTE: Data comes as A8R8G8B8, it must be reordered R8G8B8A8 (view next comment)
            // DirecX understand ARGB as a 32bit DWORD but the actual memory byte alignment is BGRA
            // So, we must realign B8G8R8A8 to R8G8B8A8
            for (int i = 0; i < image.width*image.height*4; i += 4)
            {
                blue = ((unsigned char *)image.data)[i];
                ((unsigned char *)image.data)[i] = ((unsigned char *)image.data)[i + 2];
                ((unsigned char *)image.data)[i + 2] = blue;
            }

            image.format = RF_UNCOMPRESSED_R8G8B8A8;
        }
        else if (((dds_header.ddspf.flags == 0x04) || (dds_header.ddspf.flags == 0x05)) && (dds_header.ddspf.four_cc > 0)) // Compressed
        {
            int size;       // DDS image data size

            // Calculate data size, including all mipmaps
            if (dds_header.mipmap_count > 1) size = dds_header.pitch_or_linear_size * 2;
            else size = dds_header.pitch_or_linear_size;

            RF_LOG_V(RF_LOG_DEBUG, "Pitch or linear size: %i", dds_header.pitch_or_linear_size);

            image.data = (unsigned char *) RF_ALLOC(allocator, size * sizeof(unsigned char));

//                fread(image.data, size, 1, dds_file);
            memcpy(image.data, data + file_iterator, size * sizeof(unsigned char));
            file_iterator += size * sizeof(unsigned char);

            switch (dds_header.ddspf.four_cc)
            {
                case FOURCC_DXT1:
                {
                    if (dds_header.ddspf.flags == 0x04) image.format = RF_COMPRESSED_DXT1_RGB;
                    else image.format = RF_COMPRESSED_DXT1_RGBA;
                } break;
                case FOURCC_DXT3: image.format = RF_COMPRESSED_DXT3_RGBA; break;
                case FOURCC_DXT5: image.format = RF_COMPRESSED_DXT5_RGBA; break;
                default: break;
            }
        }
    }

    return image;
}

// Load image from .pkm
RF_API rf_image rf_load_image_from_pkm(const void* data, rf_allocator allocator)
{
    // Required extensions:
    // GL_OES_compressed_ETC1_RGB8_texture  (ETC1) (OpenGL ES 2.0)
    // GL_ARB_ES3_compatibility  (ETC2/EAC) (OpenGL ES 3.0)

    // Supported tokens (defined by extensions)
    // GL_ETC1_RGB8_OES                 0x8D64
    // GL_COMPRESSED_RGB8_ETC2          0x9274
    // GL_COMPRESSED_RGBA8_ETC2_EAC     0x9278

    // PKM file (ETC1) Header (16 bytes)
    typedef struct {
        char id[4];                 // "PKM "
        char version[2];            // "10" or "20"
        unsigned short format;      // Data format (big-endian) (Check list below)
        unsigned short width;       // Texture width (big-endian) (origWidth rounded to multiple of 4)
        unsigned short height;      // Texture height (big-endian) (origHeight rounded to multiple of 4)
        unsigned short orig_width;   // Original width (big-endian)
        unsigned short orig_height;  // Original height (big-endian)
    } s_pkm_header;

    // Formats list
    // version 10: format: 0=ETC1_RGB, [1=ETC1_RGBA, 2=ETC1_RGB_MIP, 3=ETC1_RGBA_MIP] (not used)
    // version 20: format: 0=ETC1_RGB, 1=ETC2_RGB, 2=ETC2_RGBA_OLD, 3=ETC2_RGBA, 4=ETC2_RGBA1, 5=ETC2_R, 6=ETC2_RG, 7=ETC2_SIGNED_R, 8=ETC2_SIGNED_R

    // NOTE: The extended width and height are the widths rounded up to a multiple of 4.
    // NOTE: ETC is always 4bit per pixel (64 bit for each 4x4 block of pixels)

    rf_image image = { 0 };

//    FILE *pkmFile = fopen(fileName, "rb");

//    if (pkmFile == NULL)
//    {
//        RF_LOG_V(LOG_WARNING, "[%s] PKM file could not be opened", fileName);
//    }
//    else
//    {
    s_pkm_header pkm_header = { 0 };
    int file_iterator = 0;

    // Get the image header
//    fread(&pkm_header, sizeof(pkm_header), 1, pkmFile);
    memcpy(pkm_header.id, data, sizeof(s_pkm_header));
    file_iterator += sizeof(s_pkm_header);

    if ((pkm_header.id[0] != 'P') || (pkm_header.id[1] != 'K') || (pkm_header.id[2] != 'M') || (pkm_header.id[3] != ' '))
    {
        RF_LOG_V(LOG_WARNING, "[%s] PKM file does not seem to be a valid image", "fileName");
    }
    else
    {
        // NOTE: format, width and height come as big-endian, data must be swapped to little-endian
        pkm_header.format = ((pkm_header.format & 0x00FF) << 8) | ((pkm_header.format & 0xFF00) >> 8);
        pkm_header.width = ((pkm_header.width & 0x00FF) << 8) | ((pkm_header.width & 0xFF00) >> 8);
        pkm_header.height = ((pkm_header.height & 0x00FF) << 8) | ((pkm_header.height & 0xFF00) >> 8);

        RF_LOG_V("PKM (ETC) image width: %i", pkm_header.width);
        RF_LOG_V("PKM (ETC) image height: %i", pkm_header.height);
        RF_LOG_V("PKM (ETC) image format: %i", pkm_header.format);

        image.width = pkm_header.width;
        image.height = pkm_header.height;
        image.mipmaps = 1;

        int bpp = 4;
        if (pkm_header.format == 3) bpp = 8;

        int size = image.width * image.height * bpp/8;  // Total data size in bytes

        image.data = (unsigned char *) RF_ALLOC(allocator, size * sizeof(unsigned char));

//        fread(image.data, size, 1, pkmFile);
        memcpy(image.data, data + file_iterator, size);

        if (pkm_header.format == 0) image.format = RF_COMPRESSED_ETC1_RGB;
        else if (pkm_header.format == 1) image.format = RF_COMPRESSED_ETC2_RGB;
        else if (pkm_header.format == 3) image.format = RF_COMPRESSED_ETC2_EAC_RGBA;
    }

//    fclose(pkmFile);    // Close file pointer
//    }

    return image;
}

// Load image from .ktx
RF_API rf_image rf_load_image_from_ktx(const void* data, rf_allocator allocator)
{
    // Required extensions:
    // GL_OES_compressed_ETC1_RGB8_texture  (ETC1)
    // GL_ARB_ES3_compatibility  (ETC2/EAC)

    // Supported tokens (defined by extensions)
    // GL_ETC1_RGB8_OES                 0x8D64
    // GL_COMPRESSED_RGB8_ETC2          0x9274
    // GL_COMPRESSED_RGBA8_ETC2_EAC     0x9278

    // KTX file Header (64 bytes)
    // v1.1 - https://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec/
    // v2.0 - http://github.khronos.org/KTX-Specification/

    // TODO: Support KTX 2.2 specs!

    typedef struct {
        char id[12];                        // Identifier: "«KTX 11»\r\n\x1A\n"
        unsigned int endianness;            // Little endian: 0x01 0x02 0x03 0x04
        unsigned int gl_type;                // For compressed textures, gl_type must equal 0
        unsigned int gl_type_size;            // For compressed texture data, usually 1
        unsigned int gl_format;              // For compressed textures is 0
        unsigned int gl_internal_format;      // Compressed internal format
        unsigned int gl_base_internal_format;  // Same as gl_format (RGB, RGBA, ALPHA...)
        unsigned int width;                 // Texture image width in pixels
        unsigned int height;                // Texture image height in pixels
        unsigned int depth;                 // For 2D textures is 0
        unsigned int elements;              // Number of array elements, usually 0
        unsigned int faces;                 // Cubemap faces, for no-cubemap = 1
        unsigned int mipmap_levels;          // Non-mipmapped textures = 1
        unsigned int key_value_data_size;      // Used to encode any arbitrary data...
    } s_ktx_header;

    // NOTE: Before start of every mipmap data block, we have: unsigned int dataSize

    rf_image image = { 0 };

//    FILE *ktxFile = fopen(fileName, "rb");

//    if (ktxFile == NULL)
//    {
//        TRACELOG(LOG_WARNING, "[%s] KTX image file could not be opened", fileName);
//    }
//    else
//    {
    s_ktx_header ktx_header = { 0 };
    int file_iterator = 0;
        // Get the image header
//    fread(&ktxHeader, sizeof(KTXHeader), 1, ktxFile);
    memcpy(&ktx_header, data, sizeof(s_ktx_header));
    file_iterator += sizeof(s_ktx_header);

    if ((ktx_header.id[1] != 'K') || (ktx_header.id[2] != 'T') || (ktx_header.id[3] != 'X') ||
        (ktx_header.id[4] != ' ') || (ktx_header.id[5] != '1') || (ktx_header.id[6] != '1'))
    {
        RF_LOG_V(LOG_WARNING, "[%s] KTX file does not seem to be a valid file", "fileName");
    }
    else
    {
        image.width = ktx_header.width;
        image.height = ktx_header.height;
        image.mipmaps = ktx_header.mipmap_levels;

        RF_LOG_V("KTX (ETC) image width: %i", ktx_header.width);
        RF_LOG_V("KTX (ETC) image height: %i", ktx_header.height);
        RF_LOG_V("KTX (ETC) image format: 0x%x", ktx_header.gl_internal_format);

        unsigned char unused;

        if (ktx_header.key_value_data_size > 0)
        {
            for (unsigned int i = 0; i < ktx_header.key_value_data_size; i++)
            {
//                fread(&unused, sizeof(unsigned char), 1U, ktxFile);
                memcpy(&unused, data + file_iterator, sizeof(unsigned char));
                file_iterator += sizeof(unsigned char);
            }
        }

        int dataSize;
//        fread(&dataSize, sizeof(unsigned int), 1, ktxFile);
        memcpy(&dataSize, data + file_iterator, sizeof(unsigned int));
        file_iterator += sizeof(unsigned int);

        image.data = (unsigned char *) RF_ALLOC(allocator, dataSize * sizeof(unsigned char));

//        fread(image.data, dataSize, 1, ktxFile);
        memcpy(image.data, data + file_iterator, dataSize);

        if (ktx_header.gl_internal_format == 0x8D64) image.format = RF_COMPRESSED_ETC1_RGB;
        else if (ktx_header.gl_internal_format == 0x9274) image.format = RF_COMPRESSED_ETC2_RGB;
        else if (ktx_header.gl_internal_format == 0x9278) image.format = RF_COMPRESSED_ETC2_EAC_RGBA;
    }

//        fclose(ktxFile);    // Close file pointer
//    }

    return image;
}

RF_API rf_image rf_load_image_from_pvr(const void* data, rf_allocator allocator)
{
    // Required extension:
    // GL_IMG_texture_compression_pvrtc

    // Supported tokens (defined by extensions)
    // GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG       0x8C00
    // GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG      0x8C02

    // PVR file v3 Header (52 bytes)
    // NOTE: After it could be metadata (15 bytes?)
    typedef struct {
        char id[4];
        unsigned int flags;
        unsigned char channels[4];      // pixelFormat high part
        unsigned char channelDepth[4];  // pixelFormat low part
        unsigned int colourSpace;
        unsigned int channelType;
        unsigned int height;
        unsigned int width;
        unsigned int depth;
        unsigned int numSurfaces;
        unsigned int numFaces;
        unsigned int numMipmaps;
        unsigned int metaDataSize;
    } pvr_header_v3;

    rf_image image = { 0 };

//    FILE *pvrFile = fopen(fileName, "rb");

//    if (pvrFile == NULL)
//    {
//        TRACELOG(LOG_WARNING, "[%s] PVR file could not be opened", fileName);
//    }
//    else
//    {
        // Check PVR image version
    unsigned char pvr_version = 0;
    int file_iterator = 0;
//    fread(&pvrVersion, sizeof(unsigned char), 1, pvrFile);
    memcpy(&pvr_version, data, sizeof(unsigned char));
    // The file_iterator should be 0 here
//    fseek(pvrFile, 0, SEEK_SET);

    // Load different PVR data formats
    if (pvr_version == 0x50)
    {
        pvr_header_v3 pvr_header = { 0 };

        // Get PVR image header
//        fread(&pvr_header, sizeof(pvr_header_v3), 1, pvrFile);
        memcpy(&pvr_header, data, sizeof(pvr_header_v3));
        file_iterator += sizeof(pvr_header_v3);

        if ((pvr_header.id[0] != 'P') || (pvr_header.id[1] != 'V') || (pvr_header.id[2] != 'R') || (pvr_header.id[3] != 3))
        {
            RF_LOG_V(LOG_WARNING, "[%s] PVR file does not seem to be a valid image", "fileName");
        }
        else
        {
            image.width = pvr_header.width;
            image.height = pvr_header.height;
            image.mipmaps = pvr_header.numMipmaps;

            // Check data format
            if (((pvr_header.channels[0] == 'l') && (pvr_header.channels[1] == 0)) && (pvr_header.channelDepth[0] == 8))
                image.format = RF_UNCOMPRESSED_GRAYSCALE;
            else if (((pvr_header.channels[0] == 'l') && (pvr_header.channels[1] == 'a')) && ((pvr_header.channelDepth[0] == 8) && (pvr_header.channelDepth[1] == 8)))
                image.format = RF_UNCOMPRESSED_GRAY_ALPHA;
            else if ((pvr_header.channels[0] == 'r') && (pvr_header.channels[1] == 'g') && (pvr_header.channels[2] == 'b'))
            {
                if (pvr_header.channels[3] == 'a')
                {
                    if ((pvr_header.channelDepth[0] == 5) && (pvr_header.channelDepth[1] == 5) && (pvr_header.channelDepth[2] == 5) && (pvr_header.channelDepth[3] == 1))
                        image.format = RF_UNCOMPRESSED_R5G5B5A1;
                    else if ((pvr_header.channelDepth[0] == 4) && (pvr_header.channelDepth[1] == 4) && (pvr_header.channelDepth[2] == 4) && (pvr_header.channelDepth[3] == 4))
                        image.format = RF_UNCOMPRESSED_R4G4B4A4;
                    else if ((pvr_header.channelDepth[0] == 8) && (pvr_header.channelDepth[1] == 8) && (pvr_header.channelDepth[2] == 8) && (pvr_header.channelDepth[3] == 8))
                        image.format = RF_UNCOMPRESSED_R8G8B8A8;
                }
                else if (pvr_header.channels[3] == 0)
                {
                    if ((pvr_header.channelDepth[0] == 5) && (pvr_header.channelDepth[1] == 6) && (pvr_header.channelDepth[2] == 5)) image.format = RF_UNCOMPRESSED_R5G6B5;
                    else if ((pvr_header.channelDepth[0] == 8) && (pvr_header.channelDepth[1] == 8) && (pvr_header.channelDepth[2] == 8)) image.format = RF_UNCOMPRESSED_R8G8B8;
                }
            }
            else if (pvr_header.channels[0] == 2) image.format = RF_COMPRESSED_PVRT_RGB;
            else if (pvr_header.channels[0] == 3) image.format = RF_COMPRESSED_PVRT_RGBA;

            // Skip meta data header
//            unsigned char unused = 0;
//            for (int i = 0; i < pvr_header.metaDataSize; i++)
//            {
////                fread(&unused, sizeof(unsigned char), 1, pvrFile);
//            }
            file_iterator += sizeof(unsigned char) * pvr_header.metaDataSize;

            // Calculate data size (depends on format)
            int bpp = 0;

            switch (image.format)
            {
                case RF_UNCOMPRESSED_GRAYSCALE: bpp = 8; break;
                case RF_UNCOMPRESSED_GRAY_ALPHA:
                case RF_UNCOMPRESSED_R5G5B5A1:
                case RF_UNCOMPRESSED_R5G6B5:
                case RF_UNCOMPRESSED_R4G4B4A4: bpp = 16; break;
                case RF_UNCOMPRESSED_R8G8B8A8: bpp = 32; break;
                case RF_UNCOMPRESSED_R8G8B8: bpp = 24; break;
                case RF_COMPRESSED_PVRT_RGB:
                case RF_COMPRESSED_PVRT_RGBA: bpp = 4; break;
                default: break;
            }

            int dataSize = image.width * image.height * bpp / 8;  // Total data size in bytes
            image.data = (unsigned char *) RF_ALLOC(allocator, dataSize * sizeof(unsigned char));

            // Read data from file
//            fread(image.data, dataSize, 1, pvrFile);
            memcpy(image.data, data + file_iterator, dataSize);
        }
    }
    else if (pvr_version == 52) RF_LOG_V(RF_LOG_INFO, "PVR v2 not supported, update your files to PVR v3");

//        fclose(pvrFile);    // Close file pointer
//    }

    return image;
}

RF_API rf_image rf_load_image_from_astc(const void* data, rf_allocator allocator)
{
    // Required extensions:
    // GL_KHR_texture_compression_astc_hdr
    // GL_KHR_texture_compression_astc_ldr

    // Supported tokens (defined by extensions)
    // GL_COMPRESSED_RGBA_ASTC_4x4_KHR      0x93b0
    // GL_COMPRESSED_RGBA_ASTC_8x8_KHR      0x93b7

    // ASTC file Header (16 bytes)
    typedef struct {
        unsigned char id[4];        // Signature: 0x13 0xAB 0xA1 0x5C
        unsigned char blockX;       // Block X dimensions
        unsigned char blockY;       // Block Y dimensions
        unsigned char blockZ;       // Block Z dimensions (1 for 2D images)
        unsigned char width[3];     // Image width in pixels (24bit value)
        unsigned char height[3];    // Image height in pixels (24bit value)
        unsigned char length[3];    // Image Z-size (1 for 2D images)
    } s_astc_header;

    rf_image image = { 0 };

//    FILE *astcFile = fopen(fileName, "rb");

//    if (astcFile == NULL)
//    {
//        TRACELOG(LOG_WARNING, "[%s] ASTC file could not be opened", fileName);
//    }
//    else
//    {
    s_astc_header astc_header = { 0 };
    int file_iterator = 0;
    // Get ASTC image header
//    fread(&astc_header, sizeof(s_astc_header), 1, astcFile);
    memcpy(&astc_header, data, sizeof(s_astc_header));
    file_iterator += sizeof(s_astc_header);

    if ((astc_header.id[3] != 0x5c) || (astc_header.id[2] != 0xa1) || (astc_header.id[1] != 0xab) || (astc_header.id[0] != 0x13))
    {
        RF_LOG_V(RF_LOG_WARNING, "[%s] ASTC file does not seem to be a valid image", "fileName");
    }
    else
    {
        // NOTE: Assuming Little Endian (could it be wrong?)
        image.width = 0x00000000 | ((int)astc_header.width[2] << 16) | ((int)astc_header.width[1] << 8) | ((int)astc_header.width[0]);
        image.height = 0x00000000 | ((int)astc_header.height[2] << 16) | ((int)astc_header.height[1] << 8) | ((int)astc_header.height[0]);

        RF_LOG_V("ASTC image width: %i", image.width);
        RF_LOG_V("ASTC image height: %i", image.height);
        RF_LOG_V("ASTC image blocks: %ix%i", astc_header.blockX, astc_header.blockY);

        image.mipmaps = 1;      // NOTE: ASTC format only contains one mipmap level

        // NOTE: Each block is always stored in 128bit so we can calculate the bpp
        int bpp = 128/(astc_header.blockX * astc_header.blockY);

        // NOTE: Currently we only support 2 blocks configurations: 4x4 and 8x8
        if ((bpp == 8) || (bpp == 2))
        {
            int dataSize = image.width * image.height * bpp / 8;  // Data size in bytes

            image.data = (unsigned char *) RF_ALLOC(allocator, dataSize * sizeof(unsigned char));
//            fread(image.data, dataSize, 1, astcFile);
            memcpy(image.data, data + file_iterator, dataSize * sizeof(unsigned char));

            if (bpp == 8) image.format = RF_COMPRESSED_ASTC_4x4_RGBA;
            else if (bpp == 2) image.format = RF_COMPRESSED_ASTC_8x8_RGBA;
        }
        else RF_LOG_V(RF_LOG_WARNING, "[%s] ASTC block size configuration not supported", "fileName");
    }

//        fclose(astcFile);
//    }

    return image;
}

// Unloads the image using its allocator
RF_API void rf_unload_image(rf_image image)
{
    RF_FREE(image.allocator, image.data);
}
//endregion

//region gif
// Load animated GIF data
//  - rf_image.data buffer includes all frames: [image#0][image#1][image#2][...]
//  - Number of frames is returned through 'frames' parameter
//  - Frames delay is returned through 'delays' parameter (int array)
//  - All frames are returned in RGBA format
RF_API rf_gif rf_load_animated_gif_file(const char* filename, rf_allocator allocator, rf_allocator temp_allocator, rf_io_callbacks io)
{
    rf_gif result = (rf_gif) { 0 };

    int file_size = io.get_file_size_proc(filename);
    unsigned char* buffer = RF_ALLOC(temp_allocator, file_size);

    if (io.read_file_into_buffer_proc(filename, buffer, file_size))
    {
        result = rf_load_animated_gif(buffer, file_size, allocator, temp_allocator);
    }

    RF_FREE(temp_allocator, buffer);

    return result;
}

RF_API rf_gif rf_load_animated_gif(const void* data, int data_size, rf_allocator allocator, rf_allocator temp_allocator)
{
    rf_gif gif = { 0 };

    int comp = 0;

    RF_SET_STBI_ALLOCATOR(&allocator);
    {
        gif.data      = stbi_load_gif_from_memory(data, data_size, &gif.frame_delays, &gif.width, &gif.height, &gif.frames_count, &comp, 4);
        gif.allocator = allocator;
    }
    RF_SET_STBI_ALLOCATOR(NULL);

    gif.mipmaps = 1;
    gif.format  = RF_UNCOMPRESSED_R8G8B8A8;

    return gif;
}

RF_API rf_sizei rf_gif_frame_size(rf_gif gif)
{
    return (rf_sizei) { gif.width / gif.frames_count, gif.height / gif.frames_count };
}

// Returns an image pointing to the frame in the gif
RF_API rf_image rf_get_frame_from_gif(rf_gif gif, int frame)
{
    rf_sizei size = rf_gif_frame_size(gif);

    return (rf_image)
    {
        .data      = ((unsigned char*)gif.data) + rf_get_buffer_size_for_pixel_format(size.width, size.height, gif.format) * frame,
        .width     = size.width,
        .height    = size.height,
        .mipmaps   = 1,
        .format    = gif.format,
        .allocator = RF_NULL_ALLOCATOR
    };
}

RF_API void rf_unload_gif(rf_gif gif)
{
    RF_FREE(gif.allocator, gif.frame_delays);
    rf_unload_image(gif.image);
}
//endregion

//region image manipulation
// Copy an image to a new image
RF_API rf_image rf_image_copy(rf_image image, rf_allocator allocator)
{
    rf_image new_image = { 0 };
    new_image.allocator = allocator;

    int width = image.width;
    int height = image.height;
    int size = 0;

    for (int i = 0; i < image.mipmaps; i++)
    {
        size += rf_get_buffer_size_for_pixel_format(width, height, image.format);

        width /= 2;
        height /= 2;

        // Security check for NPOT textures
        if (width < 1) width = 1;
        if (height < 1) height = 1;
    }

    new_image.data = RF_ALLOC(allocator, size);

    if (new_image.data != NULL)
    {
        // NOTE: Size must be provided in bytes
        memcpy(new_image.data, image.data, size);

        new_image.width   = image.width;
        new_image.height  = image.height;
        new_image.mipmaps = image.mipmaps;
        new_image.format  = image.format;
    }

    return new_image;
}

// Create an image from another image piece
RF_API rf_image rf_image_from_image(rf_image image, rf_rec rec, rf_allocator allocator, rf_allocator temp_allocator)
{
    rf_image result = rf_image_copy(image, allocator);

    rf_image_crop(&result, rec, temp_allocator);

    return result;
}

// Resize and image to new size. Note: Uses stb default scaling filters (both bicubic): STBIR_DEFAULT_FILTER_UPSAMPLE STBIR_FILTER_CATMULLROM STBIR_DEFAULT_FILTER_DOWNSAMPLE STBIR_FILTER_MITCHELL (high-quality Catmull-Rom)
RF_API void rf_image_resize(rf_image* image, int new_width, int new_height, rf_allocator temp_allocator)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    // Get data as rf_color pixels array to work with it
    rf_color* pixels = rf_get_image_pixel_data(*image, temp_allocator);
    rf_color* output = (rf_color*) RF_ALLOC(temp_allocator, new_width * new_height * sizeof(rf_color));

    // NOTE: rf_color data is casted to (unsigned char* ), there shouldn't been any problem...
    stbir_resize_uint8((unsigned char* )pixels, image->width, image->height, 0, (unsigned char* )output, new_width, new_height, 0, 4);

    int format = image->format;

    RF_FREE(temp_allocator, image->data);

    *image = rf_load_image_from_pixels(output, new_width, new_height, image->allocator);
    rf_image_format(image, format, temp_allocator); // Reformat 32bit RGBA image to original format

    RF_FREE(temp_allocator, output);
    RF_FREE(temp_allocator, pixels);
}

// Resize and image to new size using Nearest-Neighbor scaling algorithm
RF_API void rf_image_resize_nn(rf_image* image, int new_width, int new_height, rf_allocator temp_allocator)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    rf_color* pixels = rf_get_image_pixel_data(*image, temp_allocator);
    rf_color* output = (rf_color*) RF_ALLOC(temp_allocator, new_width*new_height * sizeof(rf_color));

    // EDIT: added +1 to account for an early rounding problem
    int x_ratio = (int)((image->width << 16)/new_width) + 1;
    int y_ratio = (int)((image->height << 16)/new_height) + 1;

    int x2, y2;
    for (int y = 0; y < new_height; y++)
    {
        for (int x = 0; x < new_width; x++)
        {
            x2 = ((x*x_ratio) >> 16);
            y2 = ((y*y_ratio) >> 16);

            output[(y*new_width) + x] = pixels[(y2*image->width) + x2] ;
        }
    }

    int format = image->format;

    RF_FREE(temp_allocator, image->data);

    *image = rf_load_image_from_pixels(output, new_width, new_height, image->allocator);
    rf_image_format(image, format, temp_allocator); // Reformat 32bit RGBA image to original format

    RF_FREE(temp_allocator, output);
    RF_FREE(temp_allocator, pixels);
}

// Resize canvas and fill with color. Note: Resize offset is relative to the top-left corner of the original image
RF_API void rf_image_resize_canvas(rf_image* image, int new_width, int new_height, int offset_x, int offset_y, rf_color color, rf_allocator temp_allocator)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    if ((new_width != image->width) || (new_height != image->height))
    {
        // Support offsets out of canvas new size -> original image is cropped
        if (offset_x < 0)
        {
            rf_image_crop(image, (rf_rec) {-offset_x, 0, image->width + offset_x, image->height }, temp_allocator);
            offset_x = 0;
        }
        else if (offset_x > (new_width - image->width))
        {
            rf_image_crop(image, (rf_rec) {0, 0, image->width - (offset_x - (new_width - image->width)), image->height }, temp_allocator);
            offset_x = new_width - image->width;
        }

        if (offset_y < 0)
        {
            rf_image_crop(image, (rf_rec) {0, -offset_y, image->width, image->height + offset_y }, temp_allocator);
            offset_y = 0;
        }
        else if (offset_y > (new_height - image->height))
        {
            rf_image_crop(image, (rf_rec) {0, 0, image->width, image->height - (offset_y - (new_height - image->height)) }, temp_allocator);
            offset_y = new_height - image->height;
        }

        if ((new_width > image->width) && (new_height > image->height))
        {
            rf_image im_temp = rf_gen_image_color(new_width, new_height, color, image->allocator, temp_allocator);

            rf_rec src_rec = {0.0f, 0.0f, (float)image->width, (float)image->height };
            rf_rec dst_rec = {(float)offset_x, (float)offset_y, src_rec.width, src_rec.height };

            rf_image_draw(&im_temp, *image, src_rec, dst_rec, RF_WHITE, temp_allocator);
            rf_image_format(&im_temp, image->format, temp_allocator);
            rf_unload_image(*image);

            *image = im_temp;
        }
        else if ((new_width < image->width) && (new_height < image->height))
        {
            rf_rec crop = {(float)offset_x, (float)offset_y, (float)new_width, (float)new_height };
            rf_image_crop(image, crop, temp_allocator);
        }
        else // One side is bigger and the other is smaller
        {
            rf_image new_image = rf_gen_image_color(new_width, new_height, color, image->allocator, temp_allocator);

            rf_rec src_rec = {0.0f, 0.0f, (float)image->width, (float)image->height };
            rf_rec dst_rec = {(float)offset_x, (float)offset_y, (float)image->width, (float)image->height };

            if (new_width < image->width)
            {
                src_rec.x = offset_x;
                src_rec.width = new_width;
                dst_rec.x = 0.0f;
            }

            if (new_height < image->height)
            {
                src_rec.y = offset_y;
                src_rec.height = new_height;
                dst_rec.y = 0.0f;
            }

            rf_image_draw(&new_image, *image, src_rec, dst_rec, RF_WHITE, temp_allocator);
            rf_image_format(&new_image, image->format, temp_allocator);
            rf_unload_image(*image);

            *image = new_image;
        }
    }
}

// Generate all mipmap levels for a provided image. image.data is scaled to include mipmap levels. Mipmaps format is the same as base image
RF_API void rf_image_gen_mipmaps(rf_image* image, rf_allocator temp_allocator)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    int mip_count = 1; // Required mipmap levels count (including base level)
    int mip_width = image->width; // Base image width
    int mip_height = image->height; // Base image height
    int mip_size = rf_get_buffer_size_for_pixel_format(mip_width, mip_height, image->format); // rf_image data size (in bytes)

    // Count mipmap levels required
    while ((mip_width != 1) || (mip_height != 1))
    {
        if (mip_width != 1) mip_width /= 2;
        if (mip_height != 1) mip_height /= 2;

        // Security check for NPOT textures
        if (mip_width < 1) mip_width = 1;
        if (mip_height < 1) mip_height = 1;

        RF_LOG_V(RF_LOG_DEBUG, "Next mipmap level: %i x %i - current size %i", mip_width, mip_height, mip_size);

        mip_count++;
        mip_size += rf_get_buffer_size_for_pixel_format(mip_width, mip_height, image->format); // Add mipmap size (in bytes)
    }

    RF_LOG_V(RF_LOG_DEBUG, "Mipmaps available: %i - Mipmaps required: %i", image->mipmaps, mip_count);
    RF_LOG_V(RF_LOG_DEBUG, "Mipmaps total size required: %i", mip_size);
    RF_LOG_V(RF_LOG_DEBUG, "rf_image data memory start address: 0x%x", image->data);

    if (image->mipmaps >= mip_count)
    {
        RF_LOG_V(RF_LOG_WARNING, "rf_image mipmaps already available");
        return;
    }

    void* temp = rf_internal_realloc_wrapper(image->allocator, image->data, rf_image_size(*image), mip_size);

    if (temp != NULL)
    {
        image->data = temp; // Assign new pointer (new size) to store mipmaps data
        RF_LOG_V(RF_LOG_DEBUG, "rf_image data memory point reallocated: 0x%x", temp);
    }
    else RF_LOG_V(RF_LOG_WARNING, "Mipmaps required memory could not be allocated");

    // Pointer to allocated memory point where store next mipmap level data
    unsigned char* nextmip = (unsigned char*) image->data + rf_get_buffer_size_for_pixel_format(image->width, image->height, image->format);

    mip_width  = image->width / 2;
    mip_height = image->height / 2;
    mip_size   = rf_get_buffer_size_for_pixel_format(mip_width, mip_height, image->format);

    //Looks like a good candidate to remove the temporary allocation
    rf_image im_copy = rf_image_copy(*image, temp_allocator);

    for (int i = 1; i < mip_count; i++)
    {
        RF_LOG_V(RF_LOG_DEBUG, "Gen mipmap level: %i (%i x %i) - size: %i - offset: 0x%x", i, mip_width, mip_height, mip_size, nextmip);

        rf_image_resize(&im_copy, mip_width, mip_height, temp_allocator); // Uses internally Mitchell cubic downscale filter

        memcpy(nextmip, im_copy.data, mip_size);
        nextmip += mip_size;
        image->mipmaps++;

        mip_width /= 2;
        mip_height /= 2;

        // Security check for NPOT textures
        if (mip_width < 1) mip_width = 1;
        if (mip_height < 1) mip_height = 1;

        mip_size = rf_get_buffer_size_for_pixel_format(mip_width, mip_height, image->format);
    }

    rf_unload_image(im_copy);
}

RF_API void rf_image_to_pot(rf_image* image, rf_color fill_color, rf_allocator temp_allocator)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    rf_color* pixels = rf_get_image_pixel_data(*image, temp_allocator); // Get pixels data

    // Calculate next power-of-two values
    // NOTE: Just add the required amount of pixels at the right and bottom sides of image...
    int pot_width  = (int) powf(2, ceilf(logf((float)image->width)/logf(2)));
    int pot_height = (int) powf(2, ceilf(logf((float)image->height)/logf(2)));

    // Check if POT image generation is required (if texture is not already POT)
    if ((pot_width == image->width) && (pot_height == image->height)) return;

    rf_color* pixels_pot = NULL;

    // Generate POT array from NPOT data
    pixels_pot = (rf_color*) RF_ALLOC(temp_allocator, pot_width * pot_height * sizeof(rf_color));

    for (int j = 0; j < pot_height; j++)
    {
        for (int i = 0; i < pot_width; i++)
        {
            if ((j < image->height) && (i < image->width)) pixels_pot[j*pot_width + i] = pixels[j*image->width + i];
            else pixels_pot[j*pot_width + i] = fill_color;
        }
    }

    RF_LOG_V(RF_LOG_WARNING, "rf_image converted to POT: (%ix%i) -> (%ix%i)", image->width, image->height, pot_width, pot_height);

    RF_FREE(temp_allocator, pixels); // Free pixels data
    RF_FREE(temp_allocator, image->data); // Free old image data

    int format = image->format; // Store image data format to reconvert later

    // NOTE: rf_image size changes, new width and height
    *image = rf_load_image_from_pixels(pixels_pot, pot_width, pot_height, image->allocator);

    RF_FREE(temp_allocator, pixels_pot); // Free POT pixels data

    rf_image_format(image, format, temp_allocator); // Reconvert image to previous format
}

// Convert image data to desired format
RF_API void rf_image_format(rf_image* image, int new_format, rf_allocator temp_allocator)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    if ((new_format != 0) && (image->format != new_format))
    {
        if ((image->format < RF_COMPRESSED_DXT1_RGB) && (new_format < RF_COMPRESSED_DXT1_RGB))
        {
            rf_vec4* pixels = rf_get_image_data_normalized(*image, temp_allocator); // Supports 8 to 32 bit per channel

            RF_FREE(image->allocator, image->data); // WARNING! We loose mipmaps data --> Regenerated at the end...
            image->data = NULL;
            image->format = new_format;

            int k = 0;

            switch (image->format)
            {
                case RF_UNCOMPRESSED_GRAYSCALE:
                {
                    image->data = (unsigned char*) RF_ALLOC(image->allocator, image->width*image->height * sizeof(unsigned char));

                    for (int i = 0; i < image->width*image->height; i++)
                    {
                        ((unsigned char* )image->data)[i] = (unsigned char)((pixels[i].x*0.299f + pixels[i].y*0.587f + pixels[i].z*0.114f)*255.0f);
                    }

                } break;
                case RF_UNCOMPRESSED_GRAY_ALPHA:
                {
                    image->data = (unsigned char*) RF_ALLOC(image->allocator, image->width*image->height * 2 * sizeof(unsigned char));

                    for (int i = 0; i < image->width*image->height * 2; i += 2, k++)
                    {
                        ((unsigned char* )image->data)[i] = (unsigned char)((pixels[k].x*0.299f + (float)pixels[k].y*0.587f + (float)pixels[k].z*0.114f)*255.0f);
                        ((unsigned char* )image->data)[i + 1] = (unsigned char)(pixels[k].w*255.0f);
                    }

                } break;
                case RF_UNCOMPRESSED_R5G6B5:
                {
                    image->data = (unsigned short*) RF_ALLOC(image->allocator, image->width*image->height * sizeof(unsigned short));

                    unsigned char r = 0;
                    unsigned char g = 0;
                    unsigned char b = 0;

                    for (int i = 0; i < image->width*image->height; i++)
                    {
                        r = (unsigned char)(round(pixels[i].x * 31.0f));
                        g = (unsigned char)(round(pixels[i].y*63.0f));
                        b = (unsigned char)(round(pixels[i].z * 31.0f));

                        ((unsigned short *)image->data)[i] = (unsigned short)r << 11 | (unsigned short)g << 5 | (unsigned short)b;
                    }

                } break;
                case RF_UNCOMPRESSED_R8G8B8:
                {
                    image->data = (unsigned char*) RF_ALLOC(image->allocator, image->width*image->height * 3 * sizeof(unsigned char));

                    for (int i = 0, kk = 0; i < image->width * image->height * 3; i += 3, kk++)
                    {
                        ((unsigned char* )image->data)[i] = (unsigned char)(pixels[kk].x * 255.0f);
                        ((unsigned char* )image->data)[i + 1] = (unsigned char)(pixels[kk].y * 255.0f);
                        ((unsigned char* )image->data)[i + 2] = (unsigned char)(pixels[kk].z * 255.0f);
                    }
                } break;
                case RF_UNCOMPRESSED_R5G5B5A1:
                {
                    int ALPHA_THRESHOLD = 50;

                    image->data = (unsigned short*) RF_ALLOC(image->allocator, image->width*image->height * sizeof(unsigned short));

                    unsigned char r = 0;
                    unsigned char g = 0;
                    unsigned char b = 0;
                    unsigned char a = 0;

                    for (int i = 0; i < image->width*image->height; i++)
                    {
                        r = (unsigned char)(round(pixels[i].x * 31.0f));
                        g = (unsigned char)(round(pixels[i].y * 31.0f));
                        b = (unsigned char)(round(pixels[i].z * 31.0f));
                        a = (pixels[i].w > ((float)ALPHA_THRESHOLD/255.0f))? 1 : 0;

                        ((unsigned short *)image->data)[i] = (unsigned short)r << 11 | (unsigned short)g << 6 | (unsigned short)b << 1 | (unsigned short)a;
                    }

                } break;
                case RF_UNCOMPRESSED_R4G4B4A4:
                {
                    image->data = (unsigned short*) RF_ALLOC(image->allocator, image->width*image->height * sizeof(unsigned short));

                    unsigned char r = 0;
                    unsigned char g = 0;
                    unsigned char b = 0;
                    unsigned char a = 0;

                    for (int i = 0; i < image->width*image->height; i++)
                    {
                        r = (unsigned char)(round(pixels[i].x*15.0f));
                        g = (unsigned char)(round(pixels[i].y*15.0f));
                        b = (unsigned char)(round(pixels[i].z*15.0f));
                        a = (unsigned char)(round(pixels[i].w*15.0f));

                        ((unsigned short *)image->data)[i] = (unsigned short)r << 12 | (unsigned short)g << 8 | (unsigned short)b << 4 | (unsigned short)a;
                    }

                } break;
                case RF_UNCOMPRESSED_R8G8B8A8:
                {
                    image->data = (unsigned char*) RF_ALLOC(image->allocator, image->width*image->height * 4 * sizeof(unsigned char));

                    for (int i = 0, kk = 0; i < image->width * image->height * 4; i += 4, kk++)
                    {
                        ((unsigned char* )image->data)[i] = (unsigned char)(pixels[kk].x * 255.0f);
                        ((unsigned char* )image->data)[i + 1] = (unsigned char)(pixels[kk].y * 255.0f);
                        ((unsigned char* )image->data)[i + 2] = (unsigned char)(pixels[kk].z * 255.0f);
                        ((unsigned char* )image->data)[i + 3] = (unsigned char)(pixels[kk].w * 255.0f);
                    }
                } break;
                case RF_UNCOMPRESSED_R32:
                {
                    // WARNING: rf_image is converted to GRAYSCALE eqeuivalent 32bit

                    image->data = (float*) RF_ALLOC(image->allocator, image->width*image->height * sizeof(float));

                    for (int i = 0; i < image->width*image->height; i++)
                    {
                        ((float* )image->data)[i] = (float)(pixels[i].x*0.299f + pixels[i].y*0.587f + pixels[i].z*0.114f);
                    }
                } break;
                case RF_UNCOMPRESSED_R32G32B32:
                {
                    image->data = (float*) RF_ALLOC(image->allocator, image->width*image->height * 3 * sizeof(float));

                    for (int i = 0, kk = 0; i < image->width * image->height * 3; i += 3, kk++)
                    {
                        ((float* )image->data)[i] = pixels[kk].x;
                        ((float* )image->data)[i + 1] = pixels[kk].y;
                        ((float* )image->data)[i + 2] = pixels[kk].z;
                    }
                } break;
                case RF_UNCOMPRESSED_R32G32B32A32:
                {
                    image->data = (float*) RF_ALLOC(image->allocator, image->width*image->height * 4 * sizeof(float));

                    for (int i = 0, kk = 0; i < image->width * image->height * 4; i += 4, kk++)
                    {
                        ((float* )image->data)[i] = pixels[kk].x;
                        ((float* )image->data)[i + 1] = pixels[kk].y;
                        ((float* )image->data)[i + 2] = pixels[kk].z;
                        ((float* )image->data)[i + 3] = pixels[kk].w;
                    }
                } break;
                default: break;
            }

            RF_FREE(temp_allocator, pixels);
            pixels = NULL;

            // In case original image had mipmaps, generate mipmaps for formated image
            // NOTE: Original mipmaps are replaced by new ones, if custom mipmaps were used, they are lost
            if (image->mipmaps > 1)
            {
                image->mipmaps = 1;

                if (image->data != NULL) rf_image_gen_mipmaps(image, temp_allocator);

            }
        }
        else RF_LOG_V(RF_LOG_WARNING, "rf_image data format is compressed, can not be converted");
    }
}

// Apply alpha mask to image. Note 1: Returned image is GRAY_ALPHA (16bit) or RGBA (32bit). Note 2: alphaMask should be same size as image
RF_API void rf_image_alpha_mask(rf_image* image, rf_image alpha_mask, rf_allocator temp_allocator)
{
    if ((image->width != alpha_mask.width) || (image->height != alpha_mask.height))
    {
        RF_LOG(RF_LOG_WARNING, "Alpha mask must be same size as image");
    }
    else if (image->format >= RF_COMPRESSED_DXT1_RGB)
    {
        RF_LOG(RF_LOG_WARNING, "Alpha mask can not be applied to compressed data formats");
    }
    else
    {
        // Force mask to be Grayscale
        rf_image mask = rf_image_copy(alpha_mask, temp_allocator);
        if (mask.format != RF_UNCOMPRESSED_GRAYSCALE) rf_image_format(&mask, RF_UNCOMPRESSED_GRAYSCALE, temp_allocator);

        // In case image is only grayscale, we just add alpha channel
        if (image->format == RF_UNCOMPRESSED_GRAYSCALE)
        {
            unsigned char* data = (unsigned char*) RF_ALLOC(image->allocator, image->width*image->height * 2);

            // Apply alpha mask to alpha channel
            for (int i = 0, k = 0; (i < mask.width*mask.height) || (i < image->width*image->height); i++, k += 2)
            {
                data[k] = ((unsigned char* )image->data)[i];
                data[k + 1] = ((unsigned char* )mask.data)[i];
            }

            RF_FREE(image->allocator, image->data);
            image->data = data;
            image->format = RF_UNCOMPRESSED_GRAY_ALPHA;
        }
        else
        {
            // Convert image to RGBA
            if (image->format != RF_UNCOMPRESSED_R8G8B8A8) rf_image_format(image, RF_UNCOMPRESSED_R8G8B8A8, temp_allocator);

            // Apply alpha mask to alpha channel
            for (int i = 0, k = 3; (i < mask.width*mask.height) || (i < image->width*image->height); i++, k += 4)
            {
                ((unsigned char* )image->data)[k] = ((unsigned char* )mask.data)[i];
            }
        }

        RF_FREE(temp_allocator, mask.data);
    }
}

// Clear alpha channel to desired color. Note: Threshold defines the alpha limit, 0.0f to 1.0f
RF_API void rf_image_alpha_clear(rf_image* image, rf_color color, float threshold, rf_allocator temp_allocator)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    rf_color* pixels = rf_get_image_pixel_data(*image, temp_allocator);

    for (int i = 0; i < image->width*image->height; i++)
    {
        if (pixels[i].a <= (unsigned char)(threshold*255.0f))
        {
            pixels[i] = color;
        }
    }

    RF_FREE(image->allocator, image->data);

    int prev_format = image->format;
    *image = rf_load_image_from_pixels(pixels, image->width, image->height, image->allocator);

    RF_FREE(temp_allocator, pixels);

    rf_image_format(image, prev_format, temp_allocator);
}

// Premultiply alpha channel
RF_API void rf_image_alpha_premultiply(rf_image* image, rf_allocator temp_allocator)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    float alpha = 0.0f;
    rf_color* pixels = rf_get_image_pixel_data(*image, temp_allocator);

    for (int i = 0; i < image->width*image->height; i++)
    {
        alpha = (float)pixels[i].a / 255.0f;
        pixels[i].r = (unsigned char)((float)pixels[i].r*alpha);
        pixels[i].g = (unsigned char)((float)pixels[i].g*alpha);
        pixels[i].b = (unsigned char)((float)pixels[i].b*alpha);
    }

    RF_FREE(image->allocator, image->data);

    int prev_format = image->format;
    *image = rf_load_image_from_pixels(pixels, image->width, image->height, image->allocator);

    RF_FREE(temp_allocator, pixels);

    rf_image_format(image, prev_format, temp_allocator);
}

// Crop image depending on alpha value
RF_API void rf_image_alpha_crop(rf_image* image, float threshold, rf_allocator temp_allocator)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    rf_color* pixels = rf_get_image_pixel_data(*image, temp_allocator);

    int x_min = 65536; // Define a big enough number
    int x_max = 0;
    int y_min = 65536;
    int y_max = 0;

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            if (pixels[y*image->width + x].a > (unsigned char)(threshold*255.0f))
            {
                if (x < x_min) x_min = x;
                if (x > x_max) x_max = x;
                if (y < y_min) y_min = y;
                if (y > y_max) y_max = y;
            }
        }
    }

    rf_rec crop = { x_min, y_min, (x_max + 1) - x_min, (y_max + 1) - y_min };

    RF_FREE(temp_allocator, pixels);

    // Check for not empty image brefore cropping
    if (!((x_max < x_min) || (y_max < y_min))) rf_image_crop(image, crop, temp_allocator);
}

// Crop an image to area defined by a rectangle
RF_API void rf_image_crop(rf_image* image, rf_rec crop, rf_allocator temp_allocator)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    // Security checks to validate crop rectangle
    if (crop.x < 0) { crop.width += crop.x; crop.x = 0; }
    if (crop.y < 0) { crop.height += crop.y; crop.y = 0; }
    if ((crop.x + crop.width) > image->width) crop.width = image->width - crop.x;
    if ((crop.y + crop.height) > image->height) crop.height = image->height - crop.y;

    if ((crop.x < image->width) && (crop.y < image->height))
    {
        // Start the cropping process
        rf_color* pixels = rf_get_image_pixel_data(*image, temp_allocator); // Get data as rf_color pixels array
        rf_color* crop_pixels = (rf_color*) RF_ALLOC(temp_allocator, (int)crop.width*(int)crop.height * sizeof(rf_color));

        for (int j = (int)crop.y; j < (int)(crop.y + crop.height); j++)
        {
            for (int i = (int)crop.x; i < (int)(crop.x + crop.width); i++)
            {
                crop_pixels[(j - (int)crop.y)*(int)crop.width + (i - (int)crop.x)] = pixels[j*image->width + i];
            }
        }

        RF_FREE(temp_allocator, pixels);

        int format = image->format;

        RF_FREE(image->allocator, image->data);

        *image = rf_load_image_from_pixels(crop_pixels, (int)crop.width, (int)crop.height, image->allocator);

        RF_FREE(temp_allocator, crop_pixels);

        // Reformat 32bit RGBA image to original format
        rf_image_format(image, format, temp_allocator);
    }
    else RF_LOG_V(RF_LOG_WARNING, "rf_image can not be cropped, crop rectangle out of bounds");
}

// Dither image data to 16bpp or lower (Floyd-Steinberg dithering) Note: In case selected bpp do not represent an known 16bit format, dithered data is stored in the LSB part of the unsigned short
RF_API void rf_image_dither(rf_image* image, int r_bpp, int g_bpp, int b_bpp, int a_bpp, rf_allocator temp_allocator)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    if (image->format >= RF_COMPRESSED_DXT1_RGB)
    {
        RF_LOG(RF_LOG_WARNING, "Compressed data formats can not be dithered");
        return;
    }

    if ((r_bpp + g_bpp + b_bpp + a_bpp) > 16)
    {
        RF_LOG_V(RF_LOG_WARNING, "Unsupported dithering bpps (%ibpp), only 16bpp or lower modes supported", (r_bpp + g_bpp + b_bpp + a_bpp));
    }
    else
    {
        rf_color* pixels = rf_get_image_pixel_data(*image, temp_allocator);

        rf_unload_image(*image); // free old image data

        if ((image->format != RF_UNCOMPRESSED_R8G8B8) && (image->format != RF_UNCOMPRESSED_R8G8B8A8))
        {
            RF_LOG(RF_LOG_WARNING, "rf_image format is already 16bpp or lower, dithering could have no effect");
        }

        // Define new image format, check if desired bpp match internal known format
        if ((r_bpp == 5) && (g_bpp == 6) && (b_bpp == 5) && (a_bpp == 0)) image->format = RF_UNCOMPRESSED_R5G6B5;
        else if ((r_bpp == 5) && (g_bpp == 5) && (b_bpp == 5) && (a_bpp == 1)) image->format = RF_UNCOMPRESSED_R5G5B5A1;
        else if ((r_bpp == 4) && (g_bpp == 4) && (b_bpp == 4) && (a_bpp == 4)) image->format = RF_UNCOMPRESSED_R4G4B4A4;
        else
        {
            image->format = 0;
            RF_LOG_V(RF_LOG_WARNING, "Unsupported dithered OpenGL internal format: %ibpp (R%i_g%i_b%i_a%i)", (r_bpp + g_bpp + b_bpp + a_bpp), r_bpp, g_bpp, b_bpp, a_bpp);
        }

        // NOTE: We will store the dithered data as unsigned short (16bpp)
        image->data = (unsigned short*) RF_ALLOC(image->allocator, image->width * image->height * sizeof(unsigned short));

        rf_color old_pixel = RF_WHITE;
        rf_color new_pixel  = RF_WHITE;

        int r_error, g_error, b_error;
        unsigned short r_pixel, g_pixel, b_pixel, a_pixel; // Used for 16bit pixel composition

        for (int y = 0; y < image->height; y++)
        {
            for (int x = 0; x < image->width; x++)
            {
                old_pixel = pixels[y * image->width + x];

                // NOTE: New pixel obtained by bits truncate, it would be better to round values (check rf_image_format())
                new_pixel.r = old_pixel.r >> (8 - r_bpp); // R bits
                new_pixel.g = old_pixel.g >> (8 - g_bpp); // G bits
                new_pixel.b = old_pixel.b >> (8 - b_bpp); // B bits
                new_pixel.a = old_pixel.a >> (8 - a_bpp); // A bits (not used on dithering)

                // NOTE: Error must be computed between new and old pixel but using same number of bits!
                // We want to know how much color precision we have lost...
                r_error = (int)old_pixel.r - (int)(new_pixel.r << (8 - r_bpp));
                g_error = (int)old_pixel.g - (int)(new_pixel.g << (8 - g_bpp));
                b_error = (int)old_pixel.b - (int)(new_pixel.b << (8 - b_bpp));

                pixels[y*image->width + x] = new_pixel;

                // NOTE: Some cases are out of the array and should be ignored
                if (x < (image->width - 1))
                {
                    pixels[y*image->width + x+1].r = RF_MIN((int)pixels[y * image->width + x + 1].r + (int)((float)r_error * 7.0f / 16), 0xff);
                    pixels[y*image->width + x+1].g = RF_MIN((int)pixels[y * image->width + x + 1].g + (int)((float)g_error * 7.0f / 16), 0xff);
                    pixels[y*image->width + x+1].b = RF_MIN((int)pixels[y * image->width + x + 1].b + (int)((float)b_error * 7.0f / 16), 0xff);
                }

                if ((x > 0) && (y < (image->height - 1)))
                {
                    pixels[(y+1)*image->width + x-1].r = RF_MIN((int)pixels[(y + 1) * image->width + x - 1].r + (int)((float)r_error * 3.0f / 16), 0xff);
                    pixels[(y+1)*image->width + x-1].g = RF_MIN((int)pixels[(y + 1) * image->width + x - 1].g + (int)((float)g_error * 3.0f / 16), 0xff);
                    pixels[(y+1)*image->width + x-1].b = RF_MIN((int)pixels[(y + 1) * image->width + x - 1].b + (int)((float)b_error * 3.0f / 16), 0xff);
                }

                if (y < (image->height - 1))
                {
                    pixels[(y+1)*image->width + x].r = RF_MIN((int)pixels[(y+1)*image->width + x].r + (int)((float)r_error*5.0f/16), 0xff);
                    pixels[(y+1)*image->width + x].g = RF_MIN((int)pixels[(y+1)*image->width + x].g + (int)((float)g_error*5.0f/16), 0xff);
                    pixels[(y+1)*image->width + x].b = RF_MIN((int)pixels[(y+1)*image->width + x].b + (int)((float)b_error*5.0f/16), 0xff);
                }

                if ((x < (image->width - 1)) && (y < (image->height - 1)))
                {
                    pixels[(y+1)*image->width + x+1].r = RF_MIN((int)pixels[(y+1)*image->width + x+1].r + (int)((float)r_error*1.0f/16), 0xff);
                    pixels[(y+1)*image->width + x+1].g = RF_MIN((int)pixels[(y+1)*image->width + x+1].g + (int)((float)g_error*1.0f/16), 0xff);
                    pixels[(y+1)*image->width + x+1].b = RF_MIN((int)pixels[(y+1)*image->width + x+1].b + (int)((float)b_error*1.0f/16), 0xff);
                }

                r_pixel = (unsigned short)new_pixel.r;
                g_pixel = (unsigned short)new_pixel.g;
                b_pixel = (unsigned short)new_pixel.b;
                a_pixel = (unsigned short)new_pixel.a;

                ((unsigned short *)image->data)[y*image->width + x] = (r_pixel << (g_bpp + b_bpp + a_bpp)) | (g_pixel << (b_bpp + a_bpp)) | (b_pixel << a_bpp) | a_pixel;
            }
        }

        RF_FREE(temp_allocator, pixels);
    }
}

// Create an image from text (default font)
RF_API rf_image rf_image_text(const char* text, int text_len, int font_size, rf_color color, rf_allocator allocator, rf_allocator temp_allocator)
{
    int size = 10; // Default rf_font chars height in pixel
    if (font_size < size) font_size = size;
    int spacing = font_size / size;

    rf_image im_text = rf_image_text_ex(rf_get_default_font(), text, text_len, (float)font_size, (float)spacing, color, allocator, temp_allocator);

    return im_text;
}

// Create an image from text (custom sprite font)
RF_API rf_image rf_image_text_ex(rf_font font, const char* text, int text_len, float font_size, float spacing, rf_color tint, rf_allocator allocator, rf_allocator temp_allocator)
{
    int length = strlen(text);

    int index; // Index position in sprite font
    int letter = 0; // Current character
    int position_x = 0; // rf_image drawing position

    // NOTE: Text image is generated at font base size, later scaled to desired font size
    rf_sizef im_size = rf_measure_text(font, text, text_len, (float)font.base_size, spacing);

    // Create image to store text
    rf_image im_text = rf_gen_image_color((int)im_size.width, (int)im_size.height, RF_BLANK, allocator, temp_allocator);

    for (int i = 0; i < length; i++)
    {
        rf_utf8_codepoint codepoint = rf_get_next_utf8_codepoint(&text[i], length - i);
        letter = codepoint.value;
        index  = rf_get_glyph_index(font, letter);

        if (letter == 0x3f) codepoint.bytes_processed = 1;
        i += (codepoint.bytes_processed - 1);

        if (letter == '\n')
        {
            // TODO: Support line break
        }
        else
        {
            if (letter != ' ')
            {
                rf_rec src_rec = {0, 0, font.chars[index].image.width, font.chars[index].image.height };
                rf_rec dst_rec = {(float)(position_x + font.chars[index].offset_x), (float)font.chars[index].offset_y,font.chars[index].image.width, font.chars[index].image.height };
                rf_image_draw(&im_text, font.chars[index].image, src_rec,dst_rec, tint, temp_allocator);
            }

            if (font.chars[index].advance_x == 0) position_x += (int)(font.recs[index].width + spacing);
            else position_x += font.chars[index].advance_x + (int)spacing;
        }
    }

    // Scale image depending on text size
    if (font_size > im_size.height)
    {
        float scale_factor = font_size / im_size.height;
        RF_LOG_V(RF_LOG_INFO, "rf_image text scaled by factor: %f", scale_factor);

        // Using nearest-neighbor scaling algorithm for default font
        if (font.texture.id == rf_get_default_font().texture.id) rf_image_resize_nn(&im_text, (int)(im_size.width*scale_factor), (int)(im_size.height*scale_factor), temp_allocator);
        else rf_image_resize(&im_text, (int)(im_size.width*scale_factor), (int)(im_size.height*scale_factor), temp_allocator);
    }

    return im_text;
}

// Flip image vertically
RF_API void rf_image_flip_vertical(rf_image* image, rf_allocator temp_allocator)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    rf_color* src_pixels = rf_get_image_pixel_data(*image, temp_allocator);
    rf_color* dst_pixels = (rf_color*) RF_ALLOC(temp_allocator, image->width * image->height * sizeof(rf_color));

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            dst_pixels[y*image->width + x] = src_pixels[(image->height - 1 - y)*image->width + x];
        }
    }

    rf_image processed = rf_load_image_from_pixels(dst_pixels, image->width, image->height, image->allocator);
    rf_image_format(&processed, image->format, temp_allocator);
    rf_unload_image(*image);

    RF_FREE(temp_allocator, src_pixels);
    RF_FREE(temp_allocator, dst_pixels);

    image->data = processed.data;
}

// Flip image horizontally
RF_API void rf_image_flip_horizontal(rf_image* image, rf_allocator temp_allocator)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    rf_color* src_pixels = rf_get_image_pixel_data(*image, temp_allocator);
    rf_color* dst_pixels = (rf_color*) RF_ALLOC(temp_allocator, image->width * image->height * sizeof(rf_color));

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            dst_pixels[y*image->width + x] = src_pixels[y*image->width + (image->width - 1 - x)];
        }
    }

    rf_image processed = rf_load_image_from_pixels(dst_pixels, image->width, image->height, image->allocator);
    rf_image_format(&processed, image->format, temp_allocator);
    rf_unload_image(*image);

    RF_FREE(temp_allocator, src_pixels);
    RF_FREE(temp_allocator, dst_pixels);

    image->data = processed.data;
}

// Rotate image clockwise 90deg
RF_API void rf_image_rotate_cw(rf_image* image, rf_allocator temp_allocator)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    rf_color* src_pixels = rf_get_image_pixel_data(*image, temp_allocator);
    rf_color* rot_pixels = (rf_color*) RF_ALLOC(temp_allocator, image->width * image->height * sizeof(rf_color));

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            rot_pixels[x * image->height + (image->height - y - 1)] = src_pixels[y * image->width + x];
        }
    }

    rf_image processed = rf_load_image_from_pixels(rot_pixels, image->height, image->width, image->allocator);
    rf_image_format(&processed, image->format, temp_allocator);
    rf_unload_image(*image);
    RF_FREE(temp_allocator, src_pixels);
    RF_FREE(temp_allocator, rot_pixels);

    image->data   = processed.data;
    image->width  = processed.width;
    image->height = processed.height;
}

// Rotate image counter-clockwise 90deg
RF_API void rf_image_rotate_ccw(rf_image* image, rf_allocator temp_allocator)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    rf_color* src_pixels = rf_get_image_pixel_data(*image, temp_allocator);
    rf_color* rot_pixels = (rf_color*) RF_ALLOC(temp_allocator, image->width * image->height * sizeof(rf_color));

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            rot_pixels[x * image->height + y] = src_pixels[y * image->width + (image->width - x - 1)];
        }
    }

    rf_image new_image = rf_load_image_from_pixels(rot_pixels, image->height, image->width, image->allocator);
    rf_image_format(&new_image, image->format, temp_allocator);
    rf_unload_image(*image);

    RF_FREE(temp_allocator, src_pixels);
    RF_FREE(temp_allocator, rot_pixels);

    *image = new_image;
}

// Modify image color: tint
RF_API void rf_image_color_tint(rf_image* image, rf_color color, rf_allocator temp_allocator)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    rf_color* pixels = rf_get_image_pixel_data(*image, temp_allocator);

    float c_r = (float)color.r/255;
    float c_g = (float)color.g/255;
    float c_b = (float)color.b/255;
    float c_a = (float)color.a/255;

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            int index = y * image->width + x;
            unsigned char r = 255*((float)pixels[index].r/255*c_r);
            unsigned char g = 255*((float)pixels[index].g/255*c_g);
            unsigned char b = 255*((float)pixels[index].b/255*c_b);
            unsigned char a = 255*((float)pixels[index].a/255*c_a);

            pixels[y*image->width + x].r = r;
            pixels[y*image->width + x].g = g;
            pixels[y*image->width + x].b = b;
            pixels[y*image->width + x].a = a;
        }
    }

    rf_image processed = rf_load_image_from_pixels(pixels, image->width, image->height, image->allocator);
    rf_image_format(&processed, image->format, temp_allocator);

    rf_unload_image(*image);
    RF_FREE(temp_allocator, pixels);

    image->data = processed.data;
}

// Modify image color: invert
RF_API void rf_image_color_invert(rf_image* image, rf_allocator temp_allocator)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    rf_color* pixels = rf_get_image_pixel_data(*image, temp_allocator);

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            pixels[y*image->width + x].r = 255 - pixels[y*image->width + x].r;
            pixels[y*image->width + x].g = 255 - pixels[y*image->width + x].g;
            pixels[y*image->width + x].b = 255 - pixels[y*image->width + x].b;
        }
    }

    rf_image processed = rf_load_image_from_pixels(pixels, image->width, image->height, image->allocator);
    rf_image_format(&processed, image->format, temp_allocator);

    rf_unload_image(*image);
    RF_FREE(temp_allocator, pixels);

    image->data = processed.data;
}

// Modify image color: grayscale
RF_API void rf_image_color_grayscale(rf_image* image, rf_allocator temp_allocator)
{
    rf_image_format(image, RF_UNCOMPRESSED_GRAYSCALE, temp_allocator);
}

// Modify image color: contrast
// NOTE: Contrast values between -100 and 100
RF_API void rf_image_color_contrast(rf_image* image, float contrast, rf_allocator temp_allocator)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    if (contrast < -100) contrast = -100;
    if (contrast > 100) contrast = 100;

    contrast = (100.0f + contrast)/100.0f;
    contrast *= contrast;

    rf_color* pixels = rf_get_image_pixel_data(*image, temp_allocator);

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            float p_r = (float)pixels[y*image->width + x].r/255.0f;
            p_r -= 0.5;
            p_r *= contrast;
            p_r += 0.5;
            p_r *= 255;
            if (p_r < 0) p_r = 0;
            if (p_r > 255) p_r = 255;

            float p_g = (float)pixels[y*image->width + x].g/255.0f;
            p_g -= 0.5;
            p_g *= contrast;
            p_g += 0.5;
            p_g *= 255;
            if (p_g < 0) p_g = 0;
            if (p_g > 255) p_g = 255;

            float p_b = (float)pixels[y*image->width + x].b/255.0f;
            p_b -= 0.5;
            p_b *= contrast;
            p_b += 0.5;
            p_b *= 255;
            if (p_b < 0) p_b = 0;
            if (p_b > 255) p_b = 255;

            pixels[y*image->width + x].r = (unsigned char)p_r;
            pixels[y*image->width + x].g = (unsigned char)p_g;
            pixels[y*image->width + x].b = (unsigned char)p_b;
        }
    }

    rf_image processed = rf_load_image_from_pixels(pixels, image->width, image->height, image->allocator);
    rf_image_format(&processed, image->format, temp_allocator);
    
    rf_unload_image(*image);
    RF_FREE(temp_allocator, pixels);

    image->data = processed.data;
}

// Modify image color: brightness
// NOTE: Brightness values between -255 and 255
RF_API void rf_image_color_brightness(rf_image* image, int brightness, rf_allocator temp_allocator)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    if (brightness < -255) brightness = -255;
    if (brightness > 255) brightness = 255;

    rf_color* pixels = rf_get_image_pixel_data(*image, temp_allocator);

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            int c_r = pixels[y*image->width + x].r + brightness;
            int c_g = pixels[y*image->width + x].g + brightness;
            int c_b = pixels[y*image->width + x].b + brightness;

            if (c_r < 0) c_r = 1;
            if (c_r > 255) c_r = 255;

            if (c_g < 0) c_g = 1;
            if (c_g > 255) c_g = 255;

            if (c_b < 0) c_b = 1;
            if (c_b > 255) c_b = 255;

            pixels[y*image->width + x].r = (unsigned char)c_r;
            pixels[y*image->width + x].g = (unsigned char)c_g;
            pixels[y*image->width + x].b = (unsigned char)c_b;
        }
    }

    rf_image processed = rf_load_image_from_pixels(pixels, image->width, image->height, image->allocator);
    rf_image_format(&processed, image->format, temp_allocator);
    
    rf_unload_image(*image);
    RF_FREE(temp_allocator, pixels);

    image->data = processed.data;
}

// Modify image color: replace color
RF_API void rf_image_color_replace(rf_image* image, rf_color color, rf_color replace, rf_allocator temp_allocator)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    rf_color* pixels = rf_get_image_pixel_data(*image, temp_allocator);

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            if ((pixels[y*image->width + x].r == color.r) &&
                (pixels[y*image->width + x].g == color.g) &&
                (pixels[y*image->width + x].b == color.b) &&
                (pixels[y*image->width + x].a == color.a))
            {
                pixels[y*image->width + x].r = replace.r;
                pixels[y*image->width + x].g = replace.g;
                pixels[y*image->width + x].b = replace.b;
                pixels[y*image->width + x].a = replace.a;
            }
        }
    }

    rf_image processed = rf_load_image_from_pixels(pixels, image->width, image->height, image->allocator);
    rf_image_format(&processed, image->format, temp_allocator);
    rf_unload_image(*image);
    RF_FREE(temp_allocator, pixels);

    image->data = processed.data;
}

// Generate image: plain color
RF_API rf_image rf_gen_image_color(int width, int height, rf_color color, rf_allocator allocator, rf_allocator temp_allocator)
{
    rf_color* pixels = (rf_color*) RF_ALLOC(temp_allocator, width * height * sizeof(rf_color));
    memset(pixels, 0, width*height * sizeof(rf_color));

    for (int i = 0; i < width*height; i++) pixels[i] = color;

    rf_image image = rf_load_image_from_pixels(pixels, width, height, allocator);

    RF_FREE(temp_allocator, pixels);

    return image;
}

// Generate image: vertical gradient
RF_API rf_image rf_gen_image_gradient_v(int width, int height, rf_color top, rf_color bottom, rf_allocator allocator, rf_allocator temp_allocator)
{
    rf_color* pixels = (rf_color*) RF_ALLOC(temp_allocator, width * height * sizeof(rf_color));

    for (int j = 0; j < height; j++)
    {
        float factor = (float)j/(float)height;
        for (int i = 0; i < width; i++)
        {
            pixels[j*width + i].r = (int)((float)bottom.r*factor + (float)top.r*(1.f - factor));
            pixels[j*width + i].g = (int)((float)bottom.g*factor + (float)top.g*(1.f - factor));
            pixels[j*width + i].b = (int)((float)bottom.b*factor + (float)top.b*(1.f - factor));
            pixels[j*width + i].a = (int)((float)bottom.a*factor + (float)top.a*(1.f - factor));
        }
    }

    rf_image image = rf_load_image_from_pixels(pixels, width, height, allocator);
    RF_FREE(temp_allocator, pixels);

    return image;
}

// Generate image: horizontal gradient
RF_API rf_image rf_gen_image_gradient_h(int width, int height, rf_color left, rf_color right, rf_allocator allocator, rf_allocator temp_allocator)
{
    rf_color* pixels = (rf_color*) RF_ALLOC(temp_allocator, width * height * sizeof(rf_color));

    for (int i = 0; i < width; i++)
    {
        float factor = (float)i/(float)width;
        for (int j = 0; j < height; j++)
        {
            pixels[j*width + i].r = (int)((float)right.r*factor + (float)left.r*(1.f - factor));
            pixels[j*width + i].g = (int)((float)right.g*factor + (float)left.g*(1.f - factor));
            pixels[j*width + i].b = (int)((float)right.b*factor + (float)left.b*(1.f - factor));
            pixels[j*width + i].a = (int)((float)right.a*factor + (float)left.a*(1.f - factor));
        }
    }

    rf_image image = rf_load_image_from_pixels(pixels, width, height, allocator);
    RF_FREE(temp_allocator, pixels);

    return image;
}

// Generate image: radial gradient
RF_API rf_image rf_gen_image_gradient_radial(int width, int height, float density, rf_color inner, rf_color outer, rf_allocator allocator, rf_allocator temp_allocator)
{
    rf_color* pixels = (rf_color*) RF_ALLOC(temp_allocator, width * height * sizeof(rf_color));
    float radius = (width < height)? (float)width/2.0f : (float)height/2.0f;

    float center_x = (float)width/2.0f;
    float center_y = (float)height/2.0f;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float dist = hypotf((float)x - center_x, (float)y - center_y);
            float factor = (dist - radius*density)/(radius*(1.0f - density));

            factor = (float)fmax(factor, 0.f);
            factor = (float)fmin(factor, 1.f); // dist can be bigger than radius so we have to check

            pixels[y*width + x].r = (int)((float)outer.r*factor + (float)inner.r*(1.0f - factor));
            pixels[y*width + x].g = (int)((float)outer.g*factor + (float)inner.g*(1.0f - factor));
            pixels[y*width + x].b = (int)((float)outer.b*factor + (float)inner.b*(1.0f - factor));
            pixels[y*width + x].a = (int)((float)outer.a*factor + (float)inner.a*(1.0f - factor));
        }
    }

    rf_image image = rf_load_image_from_pixels(pixels, width, height, allocator);
    RF_FREE(temp_allocator, pixels);

    return image;
}

// Generate image: checked
RF_API rf_image rf_gen_image_checked(int width, int height, int checks_x, int checks_y, rf_color col1, rf_color col2, rf_allocator allocator, rf_allocator temp_allocator)
{
    rf_color* pixels = (rf_color*) RF_ALLOC(temp_allocator, width * height * sizeof(rf_color));

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if ((x/checks_x + y/checks_y)%2 == 0) pixels[y*width + x] = col1;
            else pixels[y*width + x] = col2;
        }
    }

    rf_image image = rf_load_image_from_pixels(pixels, width, height, allocator);
    RF_FREE(temp_allocator, pixels);

    return image;
}

// Generate image: white noise
RF_API rf_image rf_gen_image_white_noise(int width, int height, float factor, rf_allocator allocator, rf_allocator temp_allocator)
{
    rf_color* pixels = (rf_color*) RF_ALLOC(temp_allocator, width * height * sizeof(rf_color));

    for (int i = 0; i < width*height; i++)
    {
        if (rf_internal_ctx->get_random_value_proc(0, 99) < (int)(factor * 100.0f)) pixels[i] = RF_WHITE;
        else pixels[i] = RF_BLACK;
    }

    rf_image image = rf_load_image_from_pixels(pixels, width, height, allocator);
    RF_FREE(temp_allocator, pixels);

    return image;
}

// Generate image: perlin noise
RF_API rf_image rf_gen_image_perlin_noise(int width, int height, int offset_x, int offset_y, float scale, rf_allocator allocator, rf_allocator temp_allocator)
{
    rf_color* pixels = (rf_color*) RF_ALLOC(temp_allocator, width * height * sizeof(rf_color));

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float nx = (float)(x + offset_x)*scale/(float)width;
            float ny = (float)(y + offset_y)*scale/(float)height;

            // Typical values to start playing with:
            //   lacunarity = ~2.0   -- spacing between successive octaves (use exactly 2.0 for wrapping output)
            //   gain       =  0.5   -- relative weighting applied to each successive octave
            //   octaves    =  6     -- number of "octaves" of noise3() to sum

            // NOTE: We need to translate the data from [-1..1] to [0..1]
            float p = (stb_perlin_fbm_noise3(nx, ny, 1.0f, 2.0f, 0.5f, 6) + 1.0f)/2.0f;

            int intensity = (int)(p*255.0f);
            pixels[y*width + x] = (rf_color){intensity, intensity, intensity, 255};
        }
    }

    rf_image image = rf_load_image_from_pixels(pixels, width, height, allocator);
    RF_FREE(temp_allocator, pixels);

    return image;
}

// Generate image: cellular algorithm. Bigger tileSize means bigger cells
RF_API rf_image rf_gen_image_cellular(int width, int height, int tile_size, rf_allocator allocator, rf_allocator temp_allocator)
{
    rf_color* pixels = (rf_color*) RF_ALLOC(temp_allocator, width * height * sizeof(rf_color));

    int seeds_per_row = width/tile_size;
    int seeds_per_col = height/tile_size;
    int seeds_count = seeds_per_row * seeds_per_col;

    rf_vec2* seeds = (rf_vec2*) RF_ALLOC(temp_allocator, seeds_count * sizeof(rf_vec2));

    for (int i = 0; i < seeds_count; i++)
    {
        int y = (i / seeds_per_row) * tile_size + rf_internal_ctx->get_random_value_proc(0, tile_size - 1);
        int x = (i % seeds_per_row) * tile_size + rf_internal_ctx->get_random_value_proc(0, tile_size - 1);
        seeds[i] = (rf_vec2) { (float) x, (float) y };
    }

    for (int y = 0; y < height; y++)
    {
        int tile_y = y / tile_size;

        for (int x = 0; x < width; x++)
        {
            int tile_x = x / tile_size;

            float min_distance = (float) strtod("Inf", NULL); // Note(LucaSas): I dont think we need this

            // Check all adjacent tiles
            for (int i = -1; i < 2; i++)
            {
                if ((tile_x + i < 0) || (tile_x + i >= seeds_per_row)) continue;

                for (int j = -1; j < 2; j++)
                {
                    if ((tile_y + j < 0) || (tile_y + j >= seeds_per_col)) continue;

                    rf_vec2 neighbor_seed = seeds[(tile_y + j) * seeds_per_row + tile_x + i];

                    float dist = (float)hypot(x - (int)neighbor_seed.x, y - (int)neighbor_seed.y);
                    min_distance = (float)fmin(min_distance, dist);
                }
            }

            // I made this up but it seems to give good results at all tile sizes
            int intensity = (int)(min_distance * 256.0f / tile_size);
            if (intensity > 255) intensity = 255;

            pixels[y * width + x] = (rf_color) { intensity, intensity, intensity, 255 };
        }
    }

    RF_FREE(temp_allocator, seeds);

    rf_image image = rf_load_image_from_pixels(pixels, width, height, allocator);
    RF_FREE(temp_allocator, pixels);

    return image;
}

// Draw an image (source) within an image (destination)
// NOTE: rf_color tint is applied to source image
RF_API void rf_image_draw(rf_image* dst, rf_image src, rf_rec src_rec, rf_rec dst_rec, rf_color tint, rf_allocator temp_allocator)
{
    // Security check to avoid program crash
    if ((dst->data == NULL) || (dst->width == 0) || (dst->height == 0) ||
        (src.data == NULL) || (src.width == 0) || (src.height == 0)) return;

    // Security checks to avoid size and rectangle issues (out of bounds)
    // Check that srcRec is inside src image
    if (src_rec.x < 0) src_rec.x = 0;
    if (src_rec.y < 0) src_rec.y = 0;

    if ((src_rec.x + src_rec.width) > src.width)
    {
        src_rec.width = src.width - src_rec.x;
        RF_LOG_V(RF_LOG_WARNING, "Source rectangle width out of bounds, rescaled width: %i", src_rec.width);
    }

    if ((src_rec.y + src_rec.height) > src.height)
    {
        src_rec.height = src.height - src_rec.y;
        RF_LOG_V(RF_LOG_WARNING, "Source rectangle height out of bounds, rescaled height: %i", src_rec.height);
    }

    rf_image src_copy = rf_image_copy(src, temp_allocator); // Make a copy of source image to work with it

    // Crop source image to desired source rectangle (if required)
    if ((src.width != (int)src_rec.width) && (src.height != (int)src_rec.height)) rf_image_crop(&src_copy, src_rec, temp_allocator);

    // Scale source image in case destination rec size is different than source rec size
    if (((int)dst_rec.width != (int)src_rec.width) || ((int)dst_rec.height != (int)src_rec.height))
    {
        rf_image_resize(&src_copy, (int)dst_rec.width, (int)dst_rec.height, temp_allocator);
    }

    // Check that dstRec is inside dst image
    // Allow negative position within destination with cropping
    if (dst_rec.x < 0)
    {
        rf_image_crop(&src_copy, (rf_rec) {-dst_rec.x, 0, dst_rec.width + dst_rec.x, dst_rec.height }, temp_allocator);
        dst_rec.width = dst_rec.width + dst_rec.x;
        dst_rec.x = 0;
    }

    if ((dst_rec.x + dst_rec.width) > dst->width)
    {
        rf_image_crop(&src_copy, (rf_rec) {0, 0, dst->width - dst_rec.x, dst_rec.height }, temp_allocator);
        dst_rec.width = dst->width - dst_rec.x;
    }

    if (dst_rec.y < 0)
    {
        rf_image_crop(&src_copy, (rf_rec) {0, -dst_rec.y, dst_rec.width, dst_rec.height + dst_rec.y }, temp_allocator);
        dst_rec.height = dst_rec.height + dst_rec.y;
        dst_rec.y = 0;
    }

    if ((dst_rec.y + dst_rec.height) > dst->height)
    {
        rf_image_crop(&src_copy, (rf_rec) {0, 0, dst_rec.width, dst->height - dst_rec.y }, temp_allocator);
        dst_rec.height = dst->height - dst_rec.y;
    }

    // Get image data as rf_color pixels array to work with it
    rf_color* dst_pixels = rf_get_image_pixel_data(*dst, temp_allocator);
    rf_color* src_pixels = rf_get_image_pixel_data(src_copy, temp_allocator);

    rf_unload_image(src_copy); // Source copy not required any more

    rf_vec4 fsrc, fdst, fout; // Normalized pixel data (ready for operation)
    rf_vec4 ftint = rf_color_normalize(tint); // Normalized color tint

    // Blit pixels, copy source image into destination
    // TODO: Maybe out-of-bounds blitting could be considered here instead of so much cropping
    for (int j = (int)dst_rec.y; j < (int)(dst_rec.y + dst_rec.height); j++)
    {
        for (int i = (int)dst_rec.x; i < (int)(dst_rec.x + dst_rec.width); i++)
        {
            // Alpha blending (https://en.wikipedia.org/wiki/Alpha_compositing)

            fdst = rf_color_normalize(dst_pixels[j*(int)dst->width + i]);
            fsrc = rf_color_normalize(src_pixels[(j - (int)dst_rec.y)*(int)dst_rec.width + (i - (int)dst_rec.x)]);

            // Apply color tint to source image
            fsrc.x *= ftint.x; fsrc.y *= ftint.y; fsrc.z *= ftint.z; fsrc.w *= ftint.w;

            fout.w = fsrc.w + fdst.w*(1.0f - fsrc.w);

            if (fout.w <= 0.0f)
            {
                fout.x = 0.0f;
                fout.y = 0.0f;
                fout.z = 0.0f;
            }
            else
            {
                fout.x = (fsrc.x*fsrc.w + fdst.x*fdst.w*(1 - fsrc.w))/fout.w;
                fout.y = (fsrc.y*fsrc.w + fdst.y*fdst.w*(1 - fsrc.w))/fout.w;
                fout.z = (fsrc.z*fsrc.w + fdst.z*fdst.w*(1 - fsrc.w))/fout.w;
            }

            dst_pixels[j*(int)dst->width + i] = (rf_color){ (unsigned char)(fout.x*255.0f),
                                                            (unsigned char)(fout.y*255.0f),
                                                            (unsigned char)(fout.z*255.0f),
                                                            (unsigned char)(fout.w*255.0f) };

            // TODO: Support other blending options
        }
    }

    rf_unload_image(*dst);

    *dst = rf_load_image_from_pixels(dst_pixels, (int)dst->width, (int)dst->height, dst->allocator);
    rf_image_format(dst, dst->format, temp_allocator);

    RF_FREE(temp_allocator, src_pixels);
    RF_FREE(temp_allocator, dst_pixels);
}

// Draw rectangle within an image
RF_API void rf_image_draw_rectangle(rf_image* dst, rf_rec rec, rf_color color, rf_allocator temp_allocator)
{
    // Security check to avoid program crash
    if ((dst->data == NULL) || (dst->width == 0) || (dst->height == 0)) return;

    rf_image im_rec = rf_gen_image_color((int)rec.width, (int)rec.height, color, temp_allocator, temp_allocator);
    rf_image_draw(dst, im_rec, (rf_rec){0, 0, rec.width, rec.height }, rec, RF_WHITE, temp_allocator);
    rf_unload_image(im_rec);
}

// Draw rectangle lines within an image
RF_API void rf_image_draw_rectangle_lines(rf_image* dst, rf_rec rec, int thick, rf_color color, rf_allocator temp_allocator)
{
    rf_image_draw_rectangle(dst, (rf_rec){rec.x, rec.y, rec.width, thick }, color, temp_allocator);
    rf_image_draw_rectangle(dst, (rf_rec){rec.x, rec.y + thick, thick, rec.height - thick * 2 }, color, temp_allocator);
    rf_image_draw_rectangle(dst, (rf_rec){rec.x + rec.width - thick, rec.y + thick, thick, rec.height - thick * 2 }, color, temp_allocator);
    rf_image_draw_rectangle(dst, (rf_rec){rec.x, rec.y + rec.height - thick, rec.width, thick }, color, temp_allocator);
}

// Draw text (default font) within an image (destination)
RF_API void rf_image_draw_text(rf_image* dst, rf_vec2 position, const char* text, int text_len, int font_size, rf_color color, rf_allocator temp_allocator)
{
    // NOTE: For default font, sapcing is set to desired font size / default font size (10)
    rf_image_draw_text_ex(dst, position, rf_get_default_font(), text, text_len, (float)font_size, (float)font_size/10, color, temp_allocator);
}

// Draw text (custom sprite font) within an image (destination)
RF_API void rf_image_draw_text_ex(rf_image* dst, rf_vec2 position, rf_font font, const char* text, int text_len, float font_size, float spacing, rf_color color, rf_allocator temp_allocator)
{
    rf_image im_text = rf_image_text_ex(font, text, text_len, font_size, spacing, color, temp_allocator, temp_allocator);

    rf_rec src_rec = {0.0f, 0.0f, (float)im_text.width, (float)im_text.height };
    rf_rec dst_rec = {position.x, position.y, (float)im_text.width, (float)im_text.height };

    rf_image_draw(dst, im_text, src_rec, dst_rec, RF_WHITE, temp_allocator);

    rf_unload_image(im_text);
}
//endregion
//endregion

//region textures

// Load texture from file into GPU memory (VRAM)
RF_API rf_texture2d rf_load_texture_from_file(const char* filename, rf_allocator temp_allocator, rf_io_callbacks io)
{
    rf_image img = rf_load_image_from_file(filename, temp_allocator, temp_allocator, io);

    rf_texture2d texture = rf_load_texture_from_image(img);

    rf_unload_image(img);

    return texture;
}

// Load texture from an image file data
RF_API rf_texture2d rf_load_texture_from_data(const void* data, int data_len, rf_allocator temp_allocator)
{
    rf_image img = rf_load_image_from_data((void*)data, data_len, temp_allocator);

    rf_texture2d texture = rf_load_texture_from_image(img);

    rf_unload_image(img);

    return texture;
}

// Load texture from image data
RF_API rf_texture2d rf_load_texture_from_image(rf_image image)
{
    rf_texture2d texture = { 0 };

    if ((image.data != NULL) && (image.width != 0) && (image.height != 0))
    {
        texture.id = rf_gfx_load_texture(image.data, image.width, image.height, image.format, image.mipmaps);

        if (texture.id != 0)
        {
            texture.width = image.width;
            texture.height = image.height;
            texture.mipmaps = image.mipmaps;
            texture.format = image.format;
        }
    }
    else RF_LOG(RF_LOG_WARNING, "rf_texture could not be loaded from rf_image");

    return texture;
}

// Load cubemap from image, multiple image cubemap layouts supported
RF_API rf_texture_cubemap rf_load_texture_cubemap_from_image(rf_image image, rf_cubemap_layout_type layout_type, rf_allocator temp_allocator)
{
    rf_texture_cubemap cubemap = { 0 };

    if (layout_type == RF_CUBEMAP_AUTO_DETECT) // Try to automatically guess layout type
    {
        // Check image width/height to determine the type of cubemap provided
        if (image.width > image.height)
        {
            if ((image.width / 6) == image.height) { layout_type = RF_CUBEMAP_LINE_HORIZONTAL; cubemap.width = image.width / 6; }
            else if ((image.width / 4) == (image.height/3)) { layout_type = RF_CUBEMAP_CROSS_FOUR_BY_TREE; cubemap.width = image.width / 4; }
            else if (image.width >= (int)((float)image.height * 1.85f)) { layout_type = RF_CUBEMAP_PANORAMA; cubemap.width = image.width / 4; }
        }
        else if (image.height > image.width)
        {
            if ((image.height / 6) == image.width) { layout_type = RF_CUBEMAP_LINE_VERTICAL; cubemap.width = image.height / 6; }
            else if ((image.width / 3) == (image.height/4)) { layout_type = RF_CUBEMAP_CROSS_THREE_BY_FOUR; cubemap.width = image.width / 3; }
        }

        cubemap.height = cubemap.width;
    }

    if (layout_type != RF_CUBEMAP_AUTO_DETECT)
    {
        int size = cubemap.width;

        rf_image faces = { 0 }; // Vertical column image
        rf_rec face_recs[6] = { 0 }; // Face source rectangles
        for (int i = 0; i < 6; i++) face_recs[i] = (rf_rec) {0, 0, size, size };

        if (layout_type == RF_CUBEMAP_LINE_VERTICAL)
        {
            faces = image;
            for (int i = 0; i < 6; i++) face_recs[i].y = size*i;
        }
        else if (layout_type == RF_CUBEMAP_PANORAMA)
        {
            // TODO: Convert panorama image to square faces...
            // Ref: https://github.com/denivip/panorama/blob/master/panorama.cpp
        }
        else
        {
            if (layout_type == RF_CUBEMAP_LINE_HORIZONTAL) { for (int i = 0; i < 6; i++) { face_recs[i].x = size * i; } }
            else if (layout_type == RF_CUBEMAP_CROSS_THREE_BY_FOUR)
            {
                face_recs[0].x = size; face_recs[0].y = size;
                face_recs[1].x = size; face_recs[1].y = 3*size;
                face_recs[2].x = size; face_recs[2].y = 0;
                face_recs[3].x = size; face_recs[3].y = 2*size;
                face_recs[4].x = 0; face_recs[4].y = size;
                face_recs[5].x = 2*size; face_recs[5].y = size;
            }
            else if (layout_type == RF_CUBEMAP_CROSS_FOUR_BY_TREE)
            {
                face_recs[0].x = 2*size; face_recs[0].y = size;
                face_recs[1].x = 0; face_recs[1].y = size;
                face_recs[2].x = size; face_recs[2].y = 0;
                face_recs[3].x = size; face_recs[3].y = 2*size;
                face_recs[4].x = size; face_recs[4].y = size;
                face_recs[5].x = 3*size; face_recs[5].y = size;
            }

            // Convert image data to 6 faces in a vertical column, that's the optimum layout for loading
            faces = rf_gen_image_color(size, size * 6, RF_MAGENTA, temp_allocator, temp_allocator);
            rf_image_format(&faces, image.format, temp_allocator);

            // TODO: rf_image formating does not work with compressed textures!
        }

        for (int i = 0; i < 6; i++) rf_image_draw(&faces, image, face_recs[i], (rf_rec) {0, size * i, size, size }, RF_WHITE, temp_allocator);

        cubemap.id = rf_gfx_load_texture_cubemap(faces.data, size, faces.format);
        if (cubemap.id == 0) RF_LOG(RF_LOG_WARNING, "Cubemap image could not be loaded.");

        rf_unload_image(faces);
    }
    else RF_LOG(RF_LOG_WARNING, "Cubemap image layout can not be detected.");

    return cubemap;
}

// Load texture for rendering (framebuffer)
RF_API rf_render_texture2d rf_load_render_texture(int width, int height)
{
    rf_render_texture2d target = rf_gfx_load_render_texture(width, height, RF_UNCOMPRESSED_R8G8B8A8, 24, false);

    return target;
}

// Get pixel data from GPU texture and return an rf_image
RF_API rf_image rf_get_texture_data(rf_texture2d texture, rf_allocator allocator)
{
    rf_image image = { 0 };

    if (texture.format < 8)
    {
        image.data = rf_gfx_read_texture_pixels(texture, allocator);

        if (image.data != NULL)
        {
            image.width = texture.width;
            image.height = texture.height;
            image.format = texture.format;
            image.mipmaps = 1;

            // NOTE: Data retrieved on OpenGL ES 2.0 should be RGBA
            // coming from FBO color buffer, but it seems original
            // texture format is retrieved on RPI... weird...
            //image.format = RF_UNCOMPRESSED_R8G8B8A8;

            RF_LOG(RF_LOG_INFO, "rf_texture pixel data obtained successfully");
        }
        else RF_LOG(RF_LOG_WARNING, "rf_texture pixel data could not be obtained");
    }
    else RF_LOG(RF_LOG_WARNING, "Compressed texture data could not be obtained");

    return image;
}

// Update GPU texture with new data. Pixels data must match texture.format
RF_API void rf_update_texture(rf_texture2d texture, const void* pixels)
{
    rf_gfx_update_texture(texture.id, texture.width, texture.height, texture.format, pixels);
}

// Generate GPU mipmaps for a texture
RF_API void rf_gen_texture_mipmaps(rf_texture2d* texture)
{
    // NOTE: NPOT textures support check inside function
    // On WebGL (OpenGL ES 2.0) NPOT textures support is limited
    rf_gfx_generate_mipmaps(texture);
}

// Set texture wrapping mode
RF_API void rf_set_texture_wrap(rf_texture2d texture, rf_texture_wrap_mode wrap_mode)
{
    rf_gfx_set_texture_wrap(texture, wrap_mode);
}

// Set texture scaling filter mode
RF_API void rf_set_texture_filter(rf_texture2d texture, rf_texture_filter_mode filter_mode)
{
    rf_gfx_set_texture_filter(texture, filter_mode);
}

// Unload texture from GPU memory (VRAM)
RF_API void rf_unload_texture(rf_texture2d texture)
{
    if (texture.id > 0)
    {
        rf_gfx_delete_textures(texture.id);

        RF_LOG_V(RF_LOG_INFO, "[TEX ID %i] Unloaded texture data from VRAM (GPU)", texture.id);
    }
}

// Unload render texture from GPU memory (VRAM)
RF_API void rf_unload_render_texture(rf_render_texture2d target)
{
    if (target.id > 0)
    {
        rf_gfx_delete_render_textures(target);

        RF_LOG_V(RF_LOG_INFO, "[TEX ID %i] Unloaded render texture data from VRAM (GPU)", target.id);
    }
}

//endregion

//region font & text
/*
   Returns next codepoint in a UTF8 encoded text, scanning until '\0' is found or the length is exhausted
   When a invalid UTF8 rf_byte is encountered we exit as soon as possible and a '?'(0x3f) codepoint is returned
   Total number of bytes processed are returned as a parameter
   NOTE: the standard says U+FFFD should be returned in case of errors
   but that character is not supported by the default font in raylib
   TODO: optimize this code for speed!!
*/
RF_API rf_utf8_codepoint rf_get_next_utf8_codepoint(const char* text, int len)
{
    /*
    UTF8 specs from https://www.ietf.org/rfc/rfc3629.txt
    Char. number range  |        UTF-8 octet sequence
      (hexadecimal)     |              (binary)
    --------------------+---------------------------------------------
    0000 0000-0000 007F | 0xxxxxxx
    0000 0080-0000 07FF | 110xxxxx 10xxxxxx
    0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
    0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    */

    #define RF_DEFAULT_CODEPOINT (0x3f) // Codepoint defaults to '?' if invalid

    if (len < 1)
    {
        return (rf_utf8_codepoint) { RF_DEFAULT_CODEPOINT };
    }

    const int octet = (unsigned char)(text[0]); // The first UTF8 octet

    if (octet <= 0x7f)
    {
        // Only one octet (ASCII range x00-7F)
        const int code = text[0];

        // Codepoints after U+10ffff are invalid
        return (rf_utf8_codepoint) { code > 0x10ffff ? RF_DEFAULT_CODEPOINT : code, .bytes_processed = 1 };
    }
    else if ((octet & 0xe0) == 0xc0)
    {
        if (len < 2)
        {
            return (rf_utf8_codepoint) { RF_DEFAULT_CODEPOINT, .bytes_processed = 1 };
        }

        // Two octets
        // [0]xC2-DF    [1]UTF8-tail(x80-BF)
        const unsigned char octet1 = text[1];

        // Check for unexpected sequence
        if ((octet1 == '\0') || ((octet1 >> 6) != 2))
        {
            return (rf_utf8_codepoint) { RF_DEFAULT_CODEPOINT, .bytes_processed = 2 };
        }

        if ((octet >= 0xc2) && (octet <= 0xdf))
        {
            const int code = ((octet & 0x1f) << 6) | (octet1 & 0x3f);

            // Codepoints after U+10ffff are invalid
            return (rf_utf8_codepoint) { code > 0x10ffff ? RF_DEFAULT_CODEPOINT : code, .bytes_processed = 2 };
        }
    }
    else if ((octet & 0xf0) == 0xe0)
    {
        if (len < 2)
        {
            return (rf_utf8_codepoint) { RF_DEFAULT_CODEPOINT, .bytes_processed = 1 };
        }

        // Three octets
        const unsigned char octet1 = text[1];

        // Check for unexpected sequence
        if ((octet1 == '\0') || (len < 3) || ((octet1 >> 6) != 2))
        {
            return (rf_utf8_codepoint) { RF_DEFAULT_CODEPOINT, .bytes_processed = 2 };
        }

        const unsigned char octet2 = text[2];

        // Check for unexpected sequence
        if ((octet2 == '\0') || ((octet2 >> 6) != 2))
        {
            return (rf_utf8_codepoint) { RF_DEFAULT_CODEPOINT, .bytes_processed = 3 };
        }

        /*
            [0]xE0    [1]xA0-BF       [2]UTF8-tail(x80-BF)
            [0]xE1-EC [1]UTF8-tail    [2]UTF8-tail(x80-BF)
            [0]xED    [1]x80-9F       [2]UTF8-tail(x80-BF)
            [0]xEE-EF [1]UTF8-tail    [2]UTF8-tail(x80-BF)
        */
        if (((octet == 0xe0) && !((octet1 >= 0xa0) && (octet1 <= 0xbf))) ||
            ((octet == 0xed) && !((octet1 >= 0x80) && (octet1 <= 0x9f))))
        {
            return (rf_utf8_codepoint) { RF_DEFAULT_CODEPOINT, .bytes_processed = 2 };
        }

        if ((octet >= 0xe0) && (0 <= 0xef))
        {
            const int code = ((octet & 0xf) << 12) | ((octet1 & 0x3f) << 6) | (octet2 & 0x3f);

            // Codepoints after U+10ffff are invalid
            return (rf_utf8_codepoint) { code > 0x10ffff ? RF_DEFAULT_CODEPOINT : code, .bytes_processed = 3 };
        }
    }
    else if ((octet & 0xf8) == 0xf0)
    {
        // Four octets
        if (octet > 0xf4 || len < 2)
        {
            return (rf_utf8_codepoint) { RF_DEFAULT_CODEPOINT, .bytes_processed = 1 };
        }

        const unsigned char octet1 = text[1];

        // Check for unexpected sequence
        if ((octet1 == '\0') || (len < 3) || ((octet1 >> 6) != 2))
        {
            return (rf_utf8_codepoint) { RF_DEFAULT_CODEPOINT, .bytes_processed = 2 };
        }

        const unsigned char octet2 = text[2];

        // Check for unexpected sequence
        if ((octet2 == '\0') || (len < 4) || ((octet2 >> 6) != 2))
        {
            return (rf_utf8_codepoint) { RF_DEFAULT_CODEPOINT, .bytes_processed = 3 };
        }

        const unsigned char octet3 = text[3];

        // Check for unexpected sequence
        if ((octet3 == '\0') || ((octet3 >> 6) != 2))
        {
            return (rf_utf8_codepoint) { RF_DEFAULT_CODEPOINT, .bytes_processed = 4 };
        }

        /*
            [0]xF0       [1]x90-BF       [2]UTF8-tail  [3]UTF8-tail
            [0]xF1-F3    [1]UTF8-tail    [2]UTF8-tail  [3]UTF8-tail
            [0]xF4       [1]x80-8F       [2]UTF8-tail  [3]UTF8-tail
        */

        // Check for unexpected sequence
        if (((octet == 0xf0) && !((octet1 >= 0x90) && (octet1 <= 0xbf))) ||
            ((octet == 0xf4) && !((octet1 >= 0x80) && (octet1 <= 0x8f))))
        {
            return (rf_utf8_codepoint) { RF_DEFAULT_CODEPOINT, .bytes_processed = 2 };
        }

        if (octet >= 0xf0)
        {
            const int code = ((octet & 0x7) << 18) | ((octet1 & 0x3f) << 12) | ((octet2 & 0x3f) << 6) | (octet3 & 0x3f);

            // Codepoints after U+10ffff are invalid
            return (rf_utf8_codepoint) { code > 0x10ffff ? RF_DEFAULT_CODEPOINT : code, .bytes_processed = 4 };
        }
    }

    RF_ASSERT(false); // Unreachable
}

// Load rf_font from file into GPU memory (VRAM)
RF_API rf_font rf_load_font_from_file(const char* filename, rf_allocator allocator, rf_allocator temp_allocator, rf_io_callbacks io)
{
    // Default hardcoded values for ttf file loading
    #define RF_DEFAULT_TTF_FONT_SIZE (32) // rf_font first character (32 - space)
    #define RF_DEFAULT_TTF_NUMCHARS  (95) // ASCII 32..126 is 95 glyphs
    #define RF_DEFAULT_FIRST_CHAR    (32) // Expected first char for image sprite font

    rf_font font = { 0 };

    if (rf_internal_is_file_extension(filename, ".ttf") || rf_internal_is_file_extension(filename, ".otf"))
    {
        int file_size = io.get_file_size_proc(filename);
        void* data = RF_ALLOC(temp_allocator, file_size);
        font = rf_load_font(data, file_size, RF_DEFAULT_TTF_FONT_SIZE, NULL, RF_DEFAULT_TTF_NUMCHARS, allocator, temp_allocator);
        RF_FREE(temp_allocator, data);
    }
    else
    {
        rf_image image = rf_load_image_from_file(filename, temp_allocator, temp_allocator, io);
        if (image.data != NULL) font = rf_load_font_from_image(image, RF_MAGENTA, RF_DEFAULT_FIRST_CHAR, allocator, temp_allocator);
        rf_unload_image(image);
    }

    if (font.texture.id == 0)
    {
        RF_LOG_V(RF_LOG_WARNING, "[%s] rf_font could not be loaded, using default font", filename);
        font = rf_get_default_font();
    }
    else rf_set_texture_filter(font.texture, RF_FILTER_POINT); // By default we set point filter (best performance)

    return font;
}

// Load rf_font from TTF font file with generation parameters
// NOTE: You can pass an array with desired characters, those characters should be available in the font
// if array is NULL, default char set is selected 32..126
RF_API rf_font rf_load_font(const void* font_file_data, int font_file_data_size, int fontSize, int* fontChars, int chars_count, rf_allocator allocator, rf_allocator temp_allocator)
{
    rf_font font = { 0 };

    font.allocator = allocator;
    font.base_size = fontSize;
    font.chars_count = (chars_count > 0) ? chars_count : 95;
    font.chars = rf_load_font_data(font_file_data, font_file_data_size, font.base_size, fontChars, font.chars_count, RF_FONT_DEFAULT, allocator, temp_allocator);

    RF_ASSERT(font.chars != NULL);

    rf_image atlas = rf_gen_image_font_atlas(font.chars, &font.recs, font.chars_count, font.base_size, 2, 0, allocator, temp_allocator);
    font.texture = rf_load_texture_from_image(atlas);

    // Update chars[i].image to use alpha, required to be used on rf_image_draw_text()
    for (int i = 0; i < font.chars_count; i++)
    {
        rf_unload_image(font.chars[i].image);
        font.chars[i].image = rf_image_from_image(atlas, font.recs[i], allocator, temp_allocator);
    }

    rf_unload_image(atlas);

    return font;
}

//Note: Must call rf_finish_load_font_thread_safe on the gl thread afterwards to finish loading the font
RF_API rf_load_font_async_result rf_load_font_async(const unsigned char* font_file_data, int font_file_data_size, int fontSize, int* fontChars, int chars_count, rf_allocator allocator, rf_allocator temp_allocator)
{
    rf_font font = { 0 };

    font.allocator = allocator;
    font.base_size = fontSize;
    font.chars_count = (chars_count > 0)? chars_count : 95;
    font.chars = rf_load_font_data(font_file_data, font_file_data_size, font.base_size, fontChars, font.chars_count, RF_FONT_DEFAULT, allocator, temp_allocator);

    RF_ASSERT(font.chars != NULL);

    rf_image atlas = rf_gen_image_font_atlas(font.chars, &font.recs, font.chars_count, font.base_size, 2, 0, allocator, temp_allocator);

    // Update chars[i].image to use alpha, required to be used on rf_image_draw_text()
    for (int i = 0; i < font.chars_count; i++)
    {
        rf_unload_image(font.chars[i].image);
        font.chars[i].image = rf_image_from_image(atlas, font.recs[i], allocator, temp_allocator);
    }

    return (rf_load_font_async_result) { font, atlas };
}

RF_API rf_font rf_finish_load_font_async(rf_load_font_async_result fontJobResult)
{
    fontJobResult.font.texture = rf_load_texture_from_image(fontJobResult.atlas);
    rf_unload_image(fontJobResult.atlas);

    return fontJobResult.font;
}

// Load font data for further use. Note: Requires TTF font and can generate SDF data
RF_API rf_char_info* rf_load_font_data(const void* font_data, int font_data_size, int font_size, int* font_chars, int chars_count, rf_font_type type, rf_allocator allocator, rf_allocator temp_allocator)
{
    // NOTE: Using some SDF generation default values,
    // trades off precision with ability to handle *smaller* sizes
    #define RF_SDF_CHAR_PADDING       (4)
    #define RF_SDF_ON_EDGE_VALUE      (128)
    #define RF_SDF_PIXEL_DIST_SCALE   (64.0f)
    #define RF_BITMAP_ALPHA_THRESHOLD (80)

    rf_char_info* chars = NULL;

    // Load font data (including pixel data) from TTF file
    // NOTE: Loaded information should be enough to generate font image atlas,
    // using any packaging method

    unsigned char* font_buffer = (unsigned char*) RF_ALLOC(temp_allocator, font_data_size);
    memcpy(font_buffer, font_data, font_data_size);

    // Init font for data reading
    stbtt_fontinfo font_info;
    if (!stbtt_InitFont(&font_info, font_buffer, 0)) RF_LOG(RF_LOG_WARNING, "Failed to init font!");

    // Calculate font scale factor
    float scale_factor = stbtt_ScaleForPixelHeight(&font_info, (float)font_size);

    // Calculate font basic metrics
    // NOTE: ascent is equivalent to font baseline
    int ascent, descent, line_gap;
    stbtt_GetFontVMetrics(&font_info, &ascent, &descent, &line_gap);

    // In case no chars count provided, default to 95
    chars_count = (chars_count > 0)? chars_count : 95;

    // Fill font_chars in case not provided externally
    // NOTE: By default we fill chars_count consecutevely, starting at 32 (Space)
    int gen_font_chars = false;
    if (font_chars == NULL)
    {
        font_chars = (int*) RF_ALLOC(temp_allocator, chars_count * sizeof(int));
        for (int i = 0; i < chars_count; i++) font_chars[i] = i + 32;
        gen_font_chars = true;
    }

    chars = (rf_char_info*) RF_ALLOC(allocator, chars_count * sizeof(rf_char_info));

    // NOTE: Using simple packaging, one char after another
    for (int i = 0; i < chars_count; i++)
    {
        int chw = 0, chh = 0; // Character width and height (on generation)
        int ch = font_chars[i]; // Character value to get info for
        chars[i].value = ch;

        //  Render a unicode codepoint to a bitmap
        //      stbtt_GetCodepointBitmap()           -- allocates and returns a bitmap
        //      stbtt_GetCodepointBitmap_box()        -- how big the bitmap must be
        //      stbtt_MakeCodepointBitmap()          -- renders into bitmap you provide

        if (type != RF_FONT_SDF) chars[i].image.data = stbtt_GetCodepointBitmap(&font_info, scale_factor, scale_factor, ch, &chw, &chh, &chars[i].offset_x, &chars[i].offset_y);
        else if (ch != 32) chars[i].image.data = stbtt_GetCodepointSDF(&font_info, scale_factor, ch, RF_SDF_CHAR_PADDING, RF_SDF_ON_EDGE_VALUE, RF_SDF_PIXEL_DIST_SCALE, &chw, &chh, &chars[i].offset_x, &chars[i].offset_y);
        else chars[i].image.data = NULL;

        if (type == RF_FONT_BITMAP)
        {
            // Aliased bitmap (black & white) font generation, avoiding anti-aliasing
            // NOTE: For optimum results, bitmap font should be generated at base pixel size
            for (int p = 0; p < chw*chh; p++)
            {
                if (((unsigned char* )chars[i].image.data)[p] < RF_BITMAP_ALPHA_THRESHOLD) ((unsigned char* )chars[i].image.data)[p] = 0;
                else ((unsigned char* )chars[i].image.data)[p] = 255;
            }
        }

        // Load characters images
        chars[i].image.width = chw;
        chars[i].image.height = chh;
        chars[i].image.mipmaps = 1;
        chars[i].image.format = RF_UNCOMPRESSED_GRAYSCALE;

        chars[i].offset_y += (int)((float)ascent * scale_factor);

        // Get bounding box for character (may be offset to account for chars that dip above or below the line)
        int ch_x1, ch_y1, ch_x2, ch_y2;
        stbtt_GetCodepointBitmapBox(&font_info, ch, scale_factor, scale_factor, &ch_x1, &ch_y1, &ch_x2, &ch_y2);

        RF_LOG_V(RF_LOG_DEBUG, "Character box measures: %i, %i, %i, %i", ch_x1, ch_y1, ch_x2 - ch_x1, ch_y2 - ch_y1);
        RF_LOG_V(RF_LOG_DEBUG, "Character offset_y: %i", (int)((float)ascent*scale_factor) + ch_y1);

        stbtt_GetCodepointHMetrics(&font_info, ch, &chars[i].advance_x, NULL);
        chars[i].advance_x *= scale_factor;
    }

    RF_FREE(temp_allocator, font_buffer);
    if (gen_font_chars) RF_FREE(temp_allocator, font_chars);

    return chars;
}

// Load an rf_image font file (XNA style)
RF_API rf_font rf_load_font_from_image(rf_image image, rf_color key, int firstChar, rf_allocator allocator, rf_allocator temp_allocator)
{
    #define rf_color_equal(col1, col2) ((col1.r == col2.r)&&(col1.g == col2.g)&&(col1.b == col2.b)&&(col1.a == col2.a))

    int charSpacing = 0;
    int lineSpacing = 0;

    int x = 0;
    int y = 0;

    // We allocate a temporal arrays for chars data measures,
    // once we get the actual number of chars, we copy data to a sized arrays
    int tempCharValues[RF_MAX_FONT_CHARS];
    rf_rec tempCharRecs[RF_MAX_FONT_CHARS];

    rf_color* pixels = rf_get_image_pixel_data(image, temp_allocator);

    // Parse image data to get charSpacing and lineSpacing
    for (y = 0; y < image.height; y++)
    {
        for (x = 0; x < image.width; x++)
        {
            if (!rf_color_equal(pixels[y*image.width + x], key)) break;
        }

        if (!rf_color_equal(pixels[y*image.width + x], key)) break;
    }

    charSpacing = x;
    lineSpacing = y;

    int charHeight = 0;
    int j = 0;

    while (!rf_color_equal(pixels[(lineSpacing + j)*image.width + charSpacing], key)) j++;

    charHeight = j;

    // Check array values to get characters: value, x, y, w, h
    int index = 0;
    int lineToRead = 0;
    int xPosToRead = charSpacing;

    // Parse image data to get rectangle sizes
    while ((lineSpacing + lineToRead*(charHeight + lineSpacing)) < image.height)
    {
        while ((xPosToRead < image.width) &&
               !rf_color_equal((pixels[(lineSpacing + (charHeight+lineSpacing)*lineToRead)*image.width + xPosToRead]), key))
        {
            tempCharValues[index] = firstChar + index;

            tempCharRecs[index].x = (float)xPosToRead;
            tempCharRecs[index].y = (float)(lineSpacing + lineToRead*(charHeight + lineSpacing));
            tempCharRecs[index].height = (float)charHeight;

            int charWidth = 0;

            while (!rf_color_equal(pixels[(lineSpacing + (charHeight+lineSpacing)*lineToRead)*image.width + xPosToRead + charWidth], key)) charWidth++;

            tempCharRecs[index].width = (float)charWidth;

            index++;

            xPosToRead += (charWidth + charSpacing);
        }

        lineToRead++;
        xPosToRead = charSpacing;
    }

    RF_LOG(RF_LOG_DEBUG, "rf_font data parsed correctly from image");

    // NOTE: We need to remove key color borders from image to avoid weird
    // artifacts on texture scaling when using rf_filter_bilinear or rf_filter_trilinear
    for (int i = 0; i < image.height*image.width; i++) if (rf_color_equal(pixels[i], key)) pixels[i] = RF_BLANK;

    // Create a new image with the processed color data (key color replaced by RF_BLANK)
    rf_image fontClear = rf_load_image_from_pixels(pixels, image.width, image.height, temp_allocator);

    RF_FREE(temp_allocator, pixels); // Free pixels array memory

    // Create spritefont with all data parsed from image
    rf_font spriteFont = { 0 };
    spriteFont.allocator = allocator;
    spriteFont.texture = rf_load_texture_from_image(fontClear); // Convert processed image to OpenGL texture
    spriteFont.chars_count = index;

    // We got tempCharValues and tempCharsRecs populated with chars data
    // Now we move temp data to sized charValues and charRecs arrays
    spriteFont.chars = (rf_char_info*) RF_ALLOC(allocator, spriteFont.chars_count * sizeof(rf_char_info));
    spriteFont.recs = (rf_rec*) RF_ALLOC(allocator, spriteFont.chars_count * sizeof(rf_rec));

    for (int i = 0; i < spriteFont.chars_count; i++)
    {
        spriteFont.chars[i].value = tempCharValues[i];

        // Get character rectangle in the font atlas texture
        spriteFont.recs[i] = tempCharRecs[i];

        // NOTE: On image based fonts (XNA style), character offsets and xAdvance are not required (set to 0)
        spriteFont.chars[i].offset_x = 0;
        spriteFont.chars[i].offset_y = 0;
        spriteFont.chars[i].advance_x = 0;

        // Fill character image data from fontClear data
        spriteFont.chars[i].image = rf_image_from_image(fontClear, tempCharRecs[i], allocator, temp_allocator);
    }

    rf_unload_image(fontClear); // Unload processed image once converted to texture

    spriteFont.base_size = (int) spriteFont.recs[0].height;

    RF_LOG(RF_LOG_INFO, "rf_image file loaded correctly as rf_font");

    return spriteFont;
}

// Generate image font atlas using chars info. Note: Packing method: 0-Default, 1-Skyline
RF_API rf_image rf_gen_image_font_atlas(const rf_char_info* chars, rf_rec** char_recs, int chars_count, int font_size, int padding, bool use_skyline_rect_packing, rf_allocator allocator, rf_allocator temp_allocator)
{
    //Note: We switch the allocator and the buffer of this image at the end of the function before returning. The code is a bit weird, would be a good candidate for refactoring
    rf_image atlas = { 0 };

    *char_recs = NULL;

    // In case no chars count provided we suppose default of 95
    chars_count = (chars_count > 0) ? chars_count : 95;

    // NOTE: Rectangles memory is loaded here!
    rf_rec* recs = (rf_rec*) RF_ALLOC(allocator, chars_count * sizeof(rf_rec));

    // Calculate image size based on required pixel area
    // NOTE 1: rf_image is forced to be squared and POT... very conservative!
    // NOTE 2: SDF font characters already contain an internal padding,
    // so image size would result bigger than default font type
    float required_area = 0;
    for (int i = 0; i < chars_count; i++) required_area += ((chars[i].image.width + 2*padding)*(chars[i].image.height + 2*padding));
    float guess_size = sqrtf(required_area)*1.3f;
    int image_size = (int)powf(2, ceilf(logf((float)guess_size)/logf(2))); // Calculate next POT

    atlas.width = image_size; // Atlas bitmap width
    atlas.height = image_size; // Atlas bitmap height
    atlas.data = (unsigned char*) RF_ALLOC(temp_allocator, atlas.width * atlas.height); // Create a bitmap to store characters (8 bpp)
    memset(atlas.data, 0, atlas.width * atlas.height);
    atlas.format = RF_UNCOMPRESSED_GRAYSCALE;
    atlas.mipmaps = 1;
    atlas.allocator = temp_allocator; // Note: we switch the allocator later in this function before we return

    // DEBUG: We can see padding in the generated image setting a gray background...
    //for (int i = 0; i < atlas.width*atlas.height; i++) ((unsigned char* )atlas.data)[i] = 100;

    if (!use_skyline_rect_packing) // Use basic packing algorythm
    {
        int offset_x = padding;
        int offset_y = padding;

        // NOTE: Using simple packaging, one char after another
        for (int i = 0; i < chars_count; i++)
        {
            // Copy pixel data from fc.data to atlas
            for (int y = 0; y < chars[i].image.height; y++)
            {
                for (int x = 0; x < chars[i].image.width; x++)
                {
                    ((unsigned char* )atlas.data)[(offset_y + y)*atlas.width + (offset_x + x)] = ((unsigned char* )chars[i].image.data)[y*chars[i].image.width + x];
                }
            }

            // Fill chars rectangles in atlas info
            recs[i].x = (float)offset_x;
            recs[i].y = (float)offset_y;
            recs[i].width = (float)chars[i].image.width;
            recs[i].height = (float)chars[i].image.height;

            // Move atlas position X for next character drawing
            offset_x += (chars[i].image.width + 2*padding);

            if (offset_x >= (atlas.width - chars[i].image.width - padding))
            {
                offset_x = padding;

                // NOTE: Be careful on offset_y for SDF fonts, by default SDF
                // use an internal padding of 4 pixels, it means char rectangle
                // height is bigger than font_size, it could be up to (font_size + 8)
                offset_y += (font_size + 2*padding);

                if (offset_y > (atlas.height - font_size - padding)) break;
            }
        }
    }
    else if (use_skyline_rect_packing) // Use Skyline rect packing algorythm (stb_pack_rect)
    {
        RF_LOG(RF_LOG_DEBUG, "Using Skyline packing algorythm!");

        stbrp_context context = { 0 };
        stbrp_node* nodes = (stbrp_node*) RF_ALLOC(temp_allocator, chars_count * sizeof(*nodes));

        stbrp_init_target(&context, atlas.width, atlas.height, nodes, chars_count);
        stbrp_rect* rects = (stbrp_rect*) RF_ALLOC(temp_allocator, chars_count * sizeof(stbrp_rect));

        // Fill rectangles for packaging
        for (int i = 0; i < chars_count; i++)
        {
            rects[i].id = i;
            rects[i].w = chars[i].image.width + 2 * padding;
            rects[i].h = chars[i].image.height + 2 * padding;
        }

        // Package rectangles into atlas
        stbrp_pack_rects(&context, rects, chars_count);

        for (int i = 0; i < chars_count; i++)
        {
            // It return char rectangles in atlas
            recs[i].x = rects[i].x + (float) padding;
            recs[i].y = rects[i].y + (float) padding;
            recs[i].width = (float) chars[i].image.width;
            recs[i].height = (float) chars[i].image.height;

            if (rects[i].was_packed)
            {
                // Copy pixel data from fc.data to atlas
                for (int y = 0; y < chars[i].image.height; y++)
                {
                    for (int x = 0; x < chars[i].image.width; x++)
                    {
                        ((unsigned char *) atlas.data)[(rects[i].y + padding + y) * atlas.width + (rects[i].x + padding + x)] = ((unsigned char *) chars[i].image.data)[y * chars[i].image.width + x];
                    }
                }
            }
            else RF_LOG_V(RF_LOG_WARNING, "Character could not be packed: %i", i);
        }

        RF_FREE(temp_allocator, recs);
        RF_FREE(temp_allocator, nodes);
    }

    // TODO: Crop image if required for smaller size
    // Convert image data from GRAYSCALE to GRAY_ALPHA
    // WARNING: rf_image_alpha_mask(&atlas, atlas) does not work in this case, requires manual operation
    unsigned char* data_gray_alpha = (unsigned char*) RF_ALLOC(allocator, atlas.width*atlas.height * sizeof(unsigned char) * 2); // Two channels

    for (int i = 0, k = 0; i < atlas.width*atlas.height; i++, k += 2)
    {
        data_gray_alpha[k] = 255;
        data_gray_alpha[k + 1] = ((unsigned char* )atlas.data)[i];
    }

    atlas.data = data_gray_alpha;
    atlas.format = RF_UNCOMPRESSED_GRAY_ALPHA;
    atlas.allocator = allocator;

    rf_unload_image(atlas);

    *char_recs = recs;

    return atlas;
}

// Unload rf_font from GPU memory (VRAM)
RF_API void rf_unload_font(rf_font font)
{
    // NOTE: Make sure spriteFont is not default font (fallback)
    if (font.texture.id != rf_get_default_font().texture.id)
    {
        for (int i = 0; i < font.chars_count; i++) RF_FREE(font.allocator, font.chars[i].image.data);

        rf_unload_texture(font.texture);
        RF_FREE(font.allocator, font.chars);
        RF_FREE(font.allocator, font.recs);

        RF_LOG(RF_LOG_DEBUG, "Unloaded sprite font data");
    }
}

// Returns index position for a unicode character on spritefont
RF_API int rf_get_glyph_index(rf_font font, int character)
{
    return (character - 32);
}

// Measure string size for rf_font
RF_API rf_sizef rf_measure_text(rf_font font, const char* text, int len, float font_size, float spacing)
{
    int temp_len = 0; // Used to count longer text line num chars
    int len_counter = 0;

    float text_width = 0.0f;
    float temp_text_width = 0.0f; // Used to count longer text line width

    float text_height  = (float)font.base_size;
    float scale_factor = font_size/(float)font.base_size;

    int letter = 0; // Current character
    int index  = 0; // Index position in sprite font

    for (int i = 0; i < len; i++)
    {
        len_counter++;

        rf_utf8_codepoint codepoint = rf_get_next_utf8_codepoint(&text[i], len - i);
        index = rf_get_glyph_index(font, codepoint.bytes_processed);

        // NOTE: normally we exit the decoding sequence as soon as a bad unsigned char is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol so to not skip any we set next = 1
        if (letter == 0x3f) { codepoint.bytes_processed = 1; }
        i += codepoint.bytes_processed - 1;

        if (letter != '\n')
        {
            if (font.chars[index].advance_x != 0) { text_width += font.chars[index].advance_x; }
            else { text_width += (font.recs[index].width + font.chars[index].offset_x); }
        }
        else
        {
            if (temp_text_width < text_width) { temp_text_width = text_width; }

            len_counter = 0;
            text_width = 0;
            text_height += ((float)font.base_size*1.5f); // NOTE: Fixed line spacing of 1.5 lines
        }

        if (temp_len < len_counter) { temp_len = len_counter; }
    }

    if (temp_text_width < text_width) temp_text_width = text_width;

    return (rf_sizef) {
        temp_text_width * scale_factor + (float)((temp_len - 1)*spacing), // Adds chars spacing to measure
        text_height * scale_factor,
    };
}

RF_API rf_sizef rf_measure_text_rec(rf_font font, const char* text, int text_len, rf_rec rec, float font_size, float extra_spacing, bool wrap)
{
    rf_sizef result = { 0 };
    int text_offset_x = 0; // Offset between characters
    int text_offset_y = 0; // Required for line break!
    float scale_factor = 0.0f;

    int letter = 0; // Current character
    int index = 0; // Index position in sprite font

    scale_factor = font_size / font.base_size;

    enum
    {
        MEASURE_WRAP_STATE = 0,
        MEASURE_REGULAR_STATE = 1
    };

    int state = wrap ? MEASURE_WRAP_STATE : MEASURE_REGULAR_STATE;
    int start_line = -1; // Index where to begin drawing (where a line begins)
    int end_line = -1; // Index where to stop drawing (where a line ends)
    int lastk = -1; // Holds last value of the character position

    int max_y   = 0;
    int first_y = 0;
    bool first_y_set = false;

    for (int i = 0, k = 0; i < text_len; i++, k++)
    {
        int glyph_width = 0;

        rf_utf8_codepoint codepoint = rf_get_next_utf8_codepoint(&text[i], text_len - i);
        letter = codepoint.value;
        index = rf_get_glyph_index(font, letter);

        // NOTE: normally we exit the decoding sequence as soon as a bad unsigned char is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol so to not skip any we set next = 1
        if (letter == 0x3f) codepoint.bytes_processed = 1;
        i += codepoint.bytes_processed - 1;

        if (letter != '\n')
        {
            glyph_width = (font.chars[index].advance_x == 0) ?
                          (int)(font.recs[index].width * scale_factor + extra_spacing) :
                          (int)(font.chars[index].advance_x * scale_factor + extra_spacing);
        }

        // NOTE: When word_wrap is ON we first measure how much of the text we can draw before going outside of the rec container
        // We store this info in start_line and end_line, then we change states, draw the text between those two variables
        // and change states again and again recursively until the end of the text (or until we get outside of the container).
        // When word_wrap is OFF we don't need the measure state so we go to the drawing state immediately
        // and begin drawing on the next line before we can get outside the container.
        if (state == MEASURE_WRAP_STATE)
        {
            // TODO: there are multiple types of spaces in UNICODE, maybe it's a good idea to add support for more
            // See: http://jkorpela.fi/chars/spaces.html
            if ((letter == ' ') || (letter == '\t') || (letter == '\n')) { end_line = i; }

            if ((text_offset_x + glyph_width + 1) >= rec.width)
            {
                end_line = (end_line < 1) ? i : end_line;
                if (i == end_line) { end_line -= codepoint.bytes_processed; }
                if ((start_line + codepoint.bytes_processed) == end_line) { end_line = i - codepoint.bytes_processed; }
                state = !state;
            }
            else if ((i + 1) == text_len)
            {
                end_line = i;
                state = !state;
            }
            else if (letter == '\n')
            {
                state = !state;
            }

            if (state == MEASURE_REGULAR_STATE)
            {
                text_offset_x = 0;
                i = start_line;
                glyph_width = 0;

                // Save character position when we switch states
                int tmp = lastk;
                lastk = k - 1;
                k = tmp;
            }
        }
        else
        {
            if (letter == '\n')
            {
                if (!wrap)
                {
                    text_offset_y += (int)((font.base_size + font.base_size/2)*scale_factor);
                    text_offset_x = 0;
                }
            }
            else
            {
                if (!wrap && (text_offset_x + glyph_width + 1) >= rec.width)
                {
                    text_offset_y += (int)((font.base_size + font.base_size/2)*scale_factor);
                    text_offset_x = 0;
                }

                if ((text_offset_y + (int)(font.base_size*scale_factor)) > rec.height) break;

                // The right side expression is the offset of the latest character plus its width (so the end of the line)
                // We want the highest value of that expression by the end of the function
                result.width  = RF_MAX(result.width,  rec.x + text_offset_x - 1 + glyph_width);

                if (!first_y_set)
                {
                    first_y = rec.y + text_offset_y;
                    first_y_set = true;
                }

                max_y = RF_MAX(max_y, rec.y + text_offset_y + font.base_size * scale_factor);
            }

            if (wrap && i == end_line)
            {
                text_offset_y += (int)((font.base_size + font.base_size/2)*scale_factor);
                text_offset_x = 0;
                start_line = end_line;
                end_line = -1;
                glyph_width = 0;
                k = lastk;
                state = !state;
            }
        }

        text_offset_x += glyph_width;
    }

    result.height = max_y - first_y;

    return result;
}
//endregion

//region model
RF_INTERNAL rf_model rf_internal_load_meshes_and_materials_for_model(rf_model model, rf_allocator temp_allocator)
{
    // Make sure model transform is set to identity matrix!
    model.transform = rf_mat_identity();

    if (model.mesh_count == 0)
    {
        RF_LOG_V(RF_LOG_WARNING, "[%s] No meshes can be loaded, default to cube mesh", file_name);

        model.mesh_count = 1;
        model.meshes = (rf_mesh *) RF_ALLOC(model.allocator, sizeof(rf_mesh));
        memset(model.meshes, 0, sizeof(rf_mesh));
        model.meshes[0] = rf_gen_mesh_cube(1.0f, 1.0f, 1.0f, model.allocator, temp_allocator);
        model.meshes[0].allocator = model.allocator;
    }
    else
    {
        // Upload vertex data to GPU (static mesh)
        for (int i = 0; i < model.mesh_count; i++)
            rf_gfx_load_mesh(&model.meshes[i], false);
    }

    if (model.material_count == 0)
    {
        RF_LOG_V(RF_LOG_WARNING, "[%s] No materials can be loaded, default to white material", file_name);

        model.material_count = 1;
        model.materials = (rf_material *) RF_ALLOC(model.allocator, sizeof(rf_material));
        memset(model.materials, 0, sizeof(rf_material));
        model.materials[0] = rf_load_default_material(model.allocator);

        if (model.mesh_material == NULL)
        {
            model.mesh_material = (int *) RF_ALLOC(model.allocator, model.mesh_count * sizeof(int));
            memset(model.mesh_material, 0, model.mesh_count * sizeof(int));
        }
    }

    return model;
}

// Compute mesh bounding box limits. Note: min_vertex and max_vertex should be transformed by model transform matrix
RF_API rf_bounding_box rf_mesh_bounding_box(rf_mesh mesh)
{
    // Get min and max vertex to construct bounds (AABB)
    rf_vec3 min_vertex = { 0 };
    rf_vec3 max_vertex = { 0 };

    if (mesh.vertices != NULL)
    {
        min_vertex = (rf_vec3){mesh.vertices[0], mesh.vertices[1], mesh.vertices[2] };
        max_vertex = (rf_vec3){mesh.vertices[0], mesh.vertices[1], mesh.vertices[2] };

        for (int i = 1; i < mesh.vertex_count; i++)
        {
            min_vertex = rf_vec3_RF_MIN(min_vertex, (rf_vec3) {mesh.vertices[i * 3], mesh.vertices[i * 3 + 1],
                                                            mesh.vertices[i * 3 + 2]});
            max_vertex = rf_vec3_max(max_vertex, (rf_vec3) {mesh.vertices[i * 3], mesh.vertices[i * 3 + 1],
                                                            mesh.vertices[i * 3 + 2]});
        }
    }

    // Create the bounding box
    rf_bounding_box box = { 0 };
    box.min = min_vertex;
    box.max = max_vertex;

    return box;
}

// Compute mesh tangents
// NOTE: To calculate mesh tangents and binormals we need mesh vertex positions and texture coordinates
// Implementation base don: https://answers.unity.com/questions/7789/calculating-tangents-vector4.html
RF_API void rf_mesh_compute_tangents(rf_mesh* mesh, rf_allocator temp_allocator)
{
    if (mesh->tangents == NULL) mesh->tangents = (float*) RF_ALLOC(mesh->allocator, mesh->vertex_count * 4 * sizeof(float));
    else RF_LOG(RF_LOG_WARNING, "rf_mesh tangents already exist");

    rf_vec3* tan1 = (rf_vec3*) RF_ALLOC(temp_allocator, mesh->vertex_count * sizeof(rf_vec3));
    rf_vec3* tan2 = (rf_vec3*) RF_ALLOC(temp_allocator, mesh->vertex_count * sizeof(rf_vec3));

    for (int i = 0; i < mesh->vertex_count; i += 3)
    {
        // Get triangle vertices
        rf_vec3 v1 = { mesh->vertices[(i + 0) * 3 + 0], mesh->vertices[(i + 0) * 3 + 1], mesh->vertices[(i + 0) * 3 + 2] };
        rf_vec3 v2 = { mesh->vertices[(i + 1) * 3 + 0], mesh->vertices[(i + 1) * 3 + 1], mesh->vertices[(i + 1) * 3 + 2] };
        rf_vec3 v3 = { mesh->vertices[(i + 2) * 3 + 0], mesh->vertices[(i + 2) * 3 + 1], mesh->vertices[(i + 2) * 3 + 2] };

        // Get triangle texcoords
        rf_vec2 uv1 = { mesh->texcoords[(i + 0) * 2 + 0], mesh->texcoords[(i + 0) * 2 + 1] };
        rf_vec2 uv2 = { mesh->texcoords[(i + 1) * 2 + 0], mesh->texcoords[(i + 1) * 2 + 1] };
        rf_vec2 uv3 = { mesh->texcoords[(i + 2) * 2 + 0], mesh->texcoords[(i + 2) * 2 + 1] };

        float x1 = v2.x - v1.x;
        float y1 = v2.y - v1.y;
        float z1 = v2.z - v1.z;
        float x2 = v3.x - v1.x;
        float y2 = v3.y - v1.y;
        float z2 = v3.z - v1.z;

        float s1 = uv2.x - uv1.x;
        float t1 = uv2.y - uv1.y;
        float s2 = uv3.x - uv1.x;
        float t2 = uv3.y - uv1.y;

        float div = s1 * t2 - s2 * t1;
        float r = (div == 0.0f) ? (0.0f) : (1.0f / div);

        rf_vec3 sdir = {(t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r };
        rf_vec3 tdir = {(s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r };

        tan1[i + 0] = sdir;
        tan1[i + 1] = sdir;
        tan1[i + 2] = sdir;

        tan2[i + 0] = tdir;
        tan2[i + 1] = tdir;
        tan2[i + 2] = tdir;
    }

    // Compute tangents considering normals
    for (int i = 0; i < mesh->vertex_count; ++i)
    {
        rf_vec3 normal = {mesh->normals[i * 3 + 0], mesh->normals[i * 3 + 1], mesh->normals[i * 3 + 2] };
        rf_vec3 tangent = tan1[i];

        // TODO: Review, not sure if tangent computation is right, just used reference proposed maths...
        rf_vec3_ortho_normalize(&normal, &tangent);
        mesh->tangents[i * 4 + 0] = tangent.x;
        mesh->tangents[i * 4 + 1] = tangent.y;
        mesh->tangents[i * 4 + 2] = tangent.z;
        mesh->tangents[i * 4 + 3] = (rf_vec3_dot_product(rf_vec3_cross_product(normal, tangent), tan2[i]) < 0.0f) ? -1.0f : 1.0f;
    }

    RF_FREE(temp_allocator, tan1);
    RF_FREE(temp_allocator, tan2);

    // Load a new tangent attributes buffer
    mesh->vbo_id[RF_LOC_VERTEX_TANGENT] = rf_gfx_load_attrib_buffer(mesh->vao_id, RF_LOC_VERTEX_TANGENT, mesh->tangents, mesh->vertex_count * 4 * sizeof(float), false);

    RF_LOG(RF_LOG_INFO, "Tangents computed for mesh");
}

// Compute mesh binormals (aka bitangent)
RF_API void rf_mesh_compute_binormals(rf_mesh* mesh)
{
    for (int i = 0; i < mesh->vertex_count; i++)
    {
        rf_vec3 normal = {mesh->normals[i * 3 + 0], mesh->normals[i * 3 + 1], mesh->normals[i * 3 + 2] };
        rf_vec3 tangent = {mesh->tangents[i * 4 + 0], mesh->tangents[i * 4 + 1], mesh->tangents[i * 4 + 2] };
        float tangent_w = mesh->tangents[i * 4 + 3];

        // TODO: Register computed binormal in mesh->binormal?
        // rf_vec3 binormal = rf_vec3_mul(rf_vec3_cross_product(normal, tangent), tangent_w);
    }
}

// Unload mesh from memory (RAM and/or VRAM)
RF_API void rf_unload_mesh(rf_mesh mesh)
{
    rf_gfx_unload_mesh(mesh);
    RF_FREE(mesh.allocator, mesh.vbo_id);
}

RF_API rf_model rf_load_model(const char* filename, rf_allocator allocator, rf_allocator temp_allocator, rf_io_callbacks io)
{
    rf_model model = { 0 };

    if (rf_internal_is_file_extension(filename, ".obj"))
    {
        model = rf_load_model_from_obj(filename, allocator, temp_allocator, io);
    }

    if (rf_internal_is_file_extension(filename, ".iqm"))
    {
        model = rf_load_model_from_iqm(filename, allocator, temp_allocator, io);
    }

    if (rf_internal_is_file_extension(filename, ".gltf") || rf_internal_is_file_extension(filename, ".glb"))
    {
        model = rf_load_model_from_gltf(filename, allocator, temp_allocator, io);
    }

    // Make sure model transform is set to identity matrix!
    model.transform = rf_mat_identity();
    model.allocator = allocator;

    if (model.mesh_count == 0)
    {
        RF_LOG_V(RF_LOG_WARNING, "[%s] No meshes can be loaded, default to cube mesh", fileName);

        model.mesh_count = 1;
        model.meshes = (rf_mesh*) RF_ALLOC(allocator, model.mesh_count * sizeof(rf_mesh));
        memset(model.meshes, 0, model.mesh_count * sizeof(rf_mesh));
        model.meshes[0] = rf_gen_mesh_cube(1.0f, 1.0f, 1.0f, allocator, temp_allocator);
    }
    else
    {
        // Upload vertex data to GPU (static mesh)
        for (int i = 0; i < model.mesh_count; i++)
        {
            rf_gfx_load_mesh(&model.meshes[i], false);
        }
    }

    if (model.material_count == 0)
    {
        RF_LOG_V(RF_LOG_WARNING, "[%s] No materials can be loaded, default to white material", filename);

        model.material_count = 1;
        model.materials = (rf_material*) RF_ALLOC(allocator, model.material_count * sizeof(rf_material));
        memset(model.materials, 0, model.material_count * sizeof(rf_material));
        model.materials[0] = rf_load_default_material(allocator);

        if (model.mesh_material == NULL)
        {
            model.mesh_material = (int*) RF_ALLOC(allocator, model.mesh_count * sizeof(int));
        }
    }

    return model;
}

// Load OBJ mesh data. Note: This calls into a library to do io, so we need to ask the user for IO callbacks
RF_API rf_model rf_load_model_from_obj(const char* filename, rf_allocator allocator, rf_allocator temp_allocator, rf_io_callbacks io)
{
    rf_model model  = { 0 };
    model.allocator = allocator;

    tinyobj_attrib_t attrib     = { 0 };
    tinyobj_shape_t* meshes     = NULL;
    size_t           mesh_count = 0;

    tinyobj_material_t* materials      = NULL;
    size_t              material_count = 0;

    size_t data_size = io.get_file_size_proc(filename);
    void*  data      = RF_ALLOC(temp_allocator, data_size);

    if (io.read_file_into_buffer_proc(filename, data, data_size))
    {
        RF_FREE(temp_allocator, data);
        return (rf_model) { 0 };
    }

    RF_SET_TINYOBJ_ALLOCATOR(&temp_allocator); // Set to NULL at the end of the function
    {
        unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;
        int ret            = tinyobj_parse_obj(&attrib, &meshes, (size_t*) &mesh_count, &materials, &material_count, (const char*) data, data_size, flags);

        if (ret != TINYOBJ_SUCCESS)
        {
            RF_LOG_V(RF_LOG_WARNING, "[%s] rf_model data could not be loaded", file_name);
        }
        else
        {
            RF_LOG_V(RF_LOG_INFO, "[%s] rf_model data loaded successfully: %i meshes / %i materials", file_name, mesh_count, material_count);
        }

        // Init model meshes array
        {
            // TODO: Support multiple meshes... in the meantime, only one mesh is returned
            //model.mesh_count = mesh_count;
            model.mesh_count = 1;
            model.meshes     = (rf_mesh*) RF_ALLOC(allocator, model.mesh_count * sizeof(rf_mesh));
            memset(model.meshes, 0, model.mesh_count * sizeof(rf_mesh));
        }

        // Init model materials array
        if (material_count > 0)
        {
            model.material_count = material_count;
            model.materials      = (rf_material*) RF_ALLOC(allocator, model.material_count * sizeof(rf_material));
            memset(model.materials, 0, model.material_count * sizeof(rf_material));
        }

        model.mesh_material = (int*) RF_ALLOC(allocator, model.mesh_count * sizeof(int));
        memset(model.mesh_material, 0, model.mesh_count * sizeof(int));

        // Init model meshes
        for (int m = 0; m < 1; m++)
        {
            rf_mesh mesh = (rf_mesh)
            {
                .allocator      = allocator,
                .vertex_count   = attrib.num_faces * 3,
                .triangle_count = attrib.num_faces,

                .vertices  = (float*)        RF_ALLOC(allocator, (attrib.num_faces * 3) * 3 * sizeof(float)),
                .texcoords = (float*)        RF_ALLOC(allocator, (attrib.num_faces * 3) * 2 * sizeof(float)),
                .normals   = (float*)        RF_ALLOC(allocator, (attrib.num_faces * 3) * 3 * sizeof(float)),
                .vbo_id    = (unsigned int*) RF_ALLOC(allocator, RF_MAX_MESH_VBO            * sizeof(unsigned int)),
            };

            memset(mesh.vertices,  0, mesh.vertex_count * 3 * sizeof(float));
            memset(mesh.texcoords, 0, mesh.vertex_count * 2 * sizeof(float));
            memset(mesh.normals,   0, mesh.vertex_count * 3 * sizeof(float));
            memset(mesh.vbo_id,    0, RF_MAX_MESH_VBO       * sizeof(unsigned int));

            int vCount  = 0;
            int vtCount = 0;
            int vnCount = 0;

            for (int f = 0; f < attrib.num_faces; f++)
            {
                // Get indices for the face
                tinyobj_vertex_index_t idx0 = attrib.faces[3 * f + 0];
                tinyobj_vertex_index_t idx1 = attrib.faces[3 * f + 1];
                tinyobj_vertex_index_t idx2 = attrib.faces[3 * f + 2];

                // RF_LOG(RF_LOG_DEBUG, "Face %i index: v %i/%i/%i . vt %i/%i/%i . vn %i/%i/%i\n", f, idx0.v_idx, idx1.v_idx, idx2.v_idx, idx0.vt_idx, idx1.vt_idx, idx2.vt_idx, idx0.vn_idx, idx1.vn_idx, idx2.vn_idx);

                // Fill vertices buffer (float) using vertex index of the face
                for (int v = 0; v < 3; v++) { mesh.vertices[vCount + v] = attrib.vertices[idx0.v_idx * 3 + v]; }
                vCount +=3;

                for (int v = 0; v < 3; v++) { mesh.vertices[vCount + v] = attrib.vertices[idx1.v_idx * 3 + v]; }
                vCount +=3;

                for (int v = 0; v < 3; v++) { mesh.vertices[vCount + v] = attrib.vertices[idx2.v_idx * 3 + v]; }
                vCount +=3;

                // Fill texcoords buffer (float) using vertex index of the face
                // NOTE: Y-coordinate must be flipped upside-down
                mesh.texcoords[vtCount + 0] = attrib.texcoords[idx0.vt_idx * 2 + 0];
                mesh.texcoords[vtCount + 1] = 1.0f - attrib.texcoords[idx0.vt_idx * 2 + 1]; vtCount += 2;
                mesh.texcoords[vtCount + 0] = attrib.texcoords[idx1.vt_idx * 2 + 0];
                mesh.texcoords[vtCount + 1] = 1.0f - attrib.texcoords[idx1.vt_idx * 2 + 1]; vtCount += 2;
                mesh.texcoords[vtCount + 0] = attrib.texcoords[idx2.vt_idx * 2 + 0];
                mesh.texcoords[vtCount + 1] = 1.0f - attrib.texcoords[idx2.vt_idx * 2 + 1]; vtCount += 2;

                // Fill normals buffer (float) using vertex index of the face
                for (int v = 0; v < 3; v++) { mesh.normals[vnCount + v] = attrib.normals[idx0.vn_idx * 3 + v]; }
                vnCount +=3;

                for (int v = 0; v < 3; v++) { mesh.normals[vnCount + v] = attrib.normals[idx1.vn_idx * 3 + v]; }
                vnCount +=3;

                for (int v = 0; v < 3; v++) { mesh.normals[vnCount + v] = attrib.normals[idx2.vn_idx * 3 + v]; }
                vnCount +=3;
            }

            model.meshes[m] = mesh; // Assign mesh data to model

            // Assign mesh material for current mesh
            model.mesh_material[m] = attrib.material_ids[m];

            // Set unfound materials to default
            if (model.mesh_material[m] == -1) { model.mesh_material[m] = 0; }
        }

        // Init model materials
        for (int m = 0; m < material_count; m++)
        {
            // Init material to default
            // NOTE: Uses default shader, only RF_MAP_DIFFUSE supported
            model.materials[m] = rf_load_default_material(allocator);
            model.materials[m].maps[RF_MAP_DIFFUSE].texture = rf_get_default_texture(); // Get default texture, in case no texture is defined

            if (materials[m].diffuse_texname != NULL)
            {
                model.materials[m].maps[RF_MAP_DIFFUSE].texture = rf_load_texture_from_file(materials[m].diffuse_texname, temp_allocator, io); //char* diffuse_texname; // map_Kd
            }

            model.materials[m].maps[RF_MAP_DIFFUSE].color = (rf_color)
            {
                (float)(materials[m].diffuse[0] * 255.0f),
                (float)(materials[m].diffuse[1] * 255.0f),
                (float)(materials[m].diffuse[2] * 255.0f),
                255
            };

            model.materials[m].maps[RF_MAP_DIFFUSE].value = 0.0f;

            if (materials[m].specular_texname != NULL)
            {
                model.materials[m].maps[RF_MAP_SPECULAR].texture = rf_load_texture_from_file(materials[m].specular_texname, temp_allocator, io); //char* specular_texname; // map_Ks
            }

            model.materials[m].maps[RF_MAP_SPECULAR].color = (rf_color)
            {
                (float)(materials[m].specular[0] * 255.0f),
                (float)(materials[m].specular[1] * 255.0f),
                (float)(materials[m].specular[2] * 255.0f),
                255
            };

            model.materials[m].maps[RF_MAP_SPECULAR].value = 0.0f;

            if (materials[m].bump_texname != NULL)
            {
                model.materials[m].maps[RF_MAP_NORMAL].texture = rf_load_texture_from_file(materials[m].bump_texname, temp_allocator, io); //char* bump_texname; // map_bump, bump
            }

            model.materials[m].maps[RF_MAP_NORMAL].color = RF_WHITE;
            model.materials[m].maps[RF_MAP_NORMAL].value = materials[m].shininess;

            model.materials[m].maps[RF_MAP_EMISSION].color = (rf_color)
            {
                (float)(materials[m].emission[0] * 255.0f),
                (float)(materials[m].emission[1] * 255.0f),
                (float)(materials[m].emission[2] * 255.0f),
                255
            };

            if (materials[m].displacement_texname != NULL)
            {
                model.materials[m].maps[RF_MAP_HEIGHT].texture = rf_load_texture_from_file(materials[m].displacement_texname, temp_allocator, io); //char* displacement_texname; // disp
            }
        }

        tinyobj_attrib_free(&attrib);
        tinyobj_shapes_free(meshes, mesh_count);
        tinyobj_materials_free(materials, material_count);
    }
    RF_SET_TINYOBJ_ALLOCATOR(NULL);

    // NOTE: At this point we have all model data loaded
    RF_LOG_V(RF_LOG_INFO, "[%s] rf_model loaded successfully in RAM (CPU)", file_name);

    return rf_internal_load_meshes_and_materials_for_model(model, temp_allocator);
}

// Load IQM mesh data
RF_API rf_model rf_load_model_from_iqm(const char* filename, rf_allocator allocator, rf_allocator temp_allocator, rf_io_callbacks io)
{
    //region constants
    #define RF_IQM_MAGIC "INTERQUAKEMODEL" // IQM file magic number
    #define RF_IQM_VERSION 2 // only IQM version 2 supported

    #define RF_BONE_NAME_LENGTH 32 // rf_bone_info name string length
    #define RF_MESH_NAME_LENGTH 32 // rf_mesh name string length
    //endregion

    //region IQM file structs
    typedef struct rf_iqm_header rf_iqm_header;
    struct rf_iqm_header
    {
        char         magic[16];
        unsigned int version;
        unsigned int filesize;
        unsigned int flags;
        unsigned int num_text, ofs_text;
        unsigned int num_meshes, ofs_meshes;
        unsigned int num_vertexarrays, num_vertexes, ofs_vertexarrays;
        unsigned int num_triangles, ofs_triangles, ofs_adjacency;
        unsigned int num_joints, ofs_joints;
        unsigned int num_poses, ofs_poses;
        unsigned int num_anims, ofs_anims;
        unsigned int num_frames, num_framechannels, ofs_frames, ofs_bounds;
        unsigned int num_comment, ofs_comment;
        unsigned int num_extensions, ofs_extensions;
    };

    typedef struct rf_iqm_mesh rf_iqm_mesh;
    struct rf_iqm_mesh
    {
        unsigned int name;
        unsigned int material;
        unsigned int first_vertex, num_vertexes;
        unsigned int first_triangle, num_triangles;
    };

    typedef struct rf_iqm_triangle rf_iqm_triangle;
    struct rf_iqm_triangle
    {
        unsigned int vertex[3];
    };

    typedef struct rf_iqm_joint rf_iqm_joint;
    struct rf_iqm_joint
    {
        unsigned int name;
        int          parent;
        float        translate[3], rotate[4], scale[3];
    };

    typedef struct rf_iqm_vertex_array rf_iqm_vertex_array;
    struct rf_iqm_vertex_array
    {
        unsigned int type;
        unsigned int flags;
        unsigned int format;
        unsigned int size;
        unsigned int offset;
    };

    // IQM vertex data types
    typedef enum rf_iqm_vertex_type
    {
        RF_IQM_POSITION     = 0,
        RF_IQM_TEXCOORD     = 1,
        RF_IQM_NORMAL       = 2,
        RF_IQM_TANGENT      = 3,   // Note: Tangents unused by default
        RF_IQM_BLENDINDEXES = 4,
        RF_IQM_BLENDWEIGHTS = 5,
        RF_IQM_COLOR        = 6,   // Note: Vertex colors unused by default
        RF_IQM_CUSTOM       = 0x10 // Note: Custom vertex values unused by default
    }  rf_iqm_vertex_type;
    //endregion

    rf_model model = { .allocator = allocator };

    size_t data_size = io.get_file_size_proc(filename);
    unsigned char* data = (unsigned char*) RF_ALLOC(temp_allocator, data_size);

    if (io.read_file_into_buffer_proc(filename, data, data_size))
    {
        RF_FREE(temp_allocator, data);
    }

    rf_iqm_header iqm = *((rf_iqm_header*)data);

    rf_iqm_mesh*          imesh;
    rf_iqm_triangle*      tri;
    rf_iqm_vertex_array*  va;
    rf_iqm_joint*         ijoint;

    float* vertex         = NULL;
    float* normal         = NULL;
    float* text           = NULL;
    char*  blendi         = NULL;
    unsigned char* blendw = NULL;

    if (strncmp(iqm.magic, RF_IQM_MAGIC, sizeof(RF_IQM_MAGIC)))
    {
        RF_LOG_V(RF_LOG_WARNING, "[%s] IQM file does not seem to be valid", file_name);
        return model;
    }

    if (iqm.version != RF_IQM_VERSION)
    {
        RF_LOG_V(RF_LOG_WARNING, "[%s] IQM file version is not supported (%i).", file_name, iqm.version);
        return model;
    }

    // Meshes data processing
    imesh = (rf_iqm_mesh*) RF_ALLOC(temp_allocator, sizeof(rf_iqm_mesh) * iqm.num_meshes);
    memcpy(imesh, data + iqm.ofs_meshes, sizeof(rf_iqm_mesh) * iqm.num_meshes);

    model.mesh_count = iqm.num_meshes;
    model.meshes = (rf_mesh*) RF_ALLOC(model.allocator, model.mesh_count * sizeof(rf_mesh));

    char name[RF_MESH_NAME_LENGTH] = { 0 };
    for (int i = 0; i < model.mesh_count; i++)
    {
        memcpy(name, data + (iqm.ofs_text + imesh[i].name), RF_MESH_NAME_LENGTH);

        model.meshes[i] = (rf_mesh)
        {
            .allocator = allocator,
            .vertex_count = imesh[i].num_vertexes
        };

        model.meshes[i].vertices = (float*) RF_ALLOC(model.allocator, model.meshes[i].vertex_count * 3 * sizeof(float)); // Default vertex positions
        memset(model.meshes[i].vertices, 0, model.meshes[i].vertex_count * 3 * sizeof(float));

        model.meshes[i].normals = (float*) RF_ALLOC(model.allocator, model.meshes[i].vertex_count * 3 * sizeof(float)); // Default vertex normals
        memset(model.meshes[i].normals, 0, model.meshes[i].vertex_count * 3 * sizeof(float));

        model.meshes[i].texcoords = (float*) RF_ALLOC(model.allocator, model.meshes[i].vertex_count * 2 * sizeof(float)); // Default vertex texcoords
        memset(model.meshes[i].texcoords, 0, model.meshes[i].vertex_count * 2 * sizeof(float));

        model.meshes[i].bone_ids = (int*) RF_ALLOC(model.allocator, model.meshes[i].vertex_count * 4 * sizeof(float)); // Up-to 4 bones supported!
        memset(model.meshes[i].bone_ids, 0, model.meshes[i].vertex_count * 4 * sizeof(float));

        model.meshes[i].bone_weights = (float*) RF_ALLOC(model.allocator, model.meshes[i].vertex_count * 4 * sizeof(float)); // Up-to 4 bones supported!
        memset(model.meshes[i].bone_weights, 0, model.meshes[i].vertex_count * 4 * sizeof(float));

        model.meshes[i].triangle_count = imesh[i].num_triangles;

        model.meshes[i].indices = (unsigned short*) RF_ALLOC(model.allocator, model.meshes[i].triangle_count * 3 * sizeof(unsigned short));
        memset(model.meshes[i].indices, 0, model.meshes[i].triangle_count * 3 * sizeof(unsigned short));

        // Animated verted data, what we actually process for rendering
        // NOTE: Animated vertex should be re-uploaded to GPU (if not using GPU skinning)
        model.meshes[i].anim_vertices = (float*) RF_ALLOC(model.allocator, model.meshes[i].vertex_count * 3 * sizeof(float));
        memset(model.meshes[i].anim_vertices, 0, model.meshes[i].vertex_count * 3 * sizeof(float));

        model.meshes[i].anim_normals = (float*) RF_ALLOC(model.allocator, model.meshes[i].vertex_count * 3 * sizeof(float));
        memset(model.meshes[i].anim_normals, 0, model.meshes[i].vertex_count * 3 * sizeof(float));

        model.meshes[i].vbo_id = (unsigned int*) RF_ALLOC(model.allocator, RF_MAX_MESH_VBO * sizeof(unsigned int));
        memset(model.meshes[i].vbo_id, 0, RF_MAX_MESH_VBO * sizeof(unsigned int));
    }

    // Triangles data processing
    tri = (rf_iqm_triangle*) RF_ALLOC(temp_allocator, iqm.num_triangles * sizeof(rf_iqm_triangle));
    memcpy(tri, data + iqm.ofs_triangles, iqm.num_triangles * sizeof(rf_iqm_triangle));

    for (int m = 0; m < model.mesh_count; m++)
    {
        int tcounter = 0;

        for (int i = imesh[m].first_triangle; i < (imesh[m].first_triangle + imesh[m].num_triangles); i++)
        {
            // IQM triangles are stored counter clockwise, but raylib sets opengl to clockwise drawing, so we swap them around
            model.meshes[m].indices[tcounter + 2] = tri[i].vertex[0] - imesh[m].first_vertex;
            model.meshes[m].indices[tcounter + 1] = tri[i].vertex[1] - imesh[m].first_vertex;
            model.meshes[m].indices[tcounter    ] = tri[i].vertex[2] - imesh[m].first_vertex;
            tcounter += 3;
        }
    }

    // Vertex arrays data processing
    va = (rf_iqm_vertex_array*) RF_ALLOC(temp_allocator, iqm.num_vertexarrays * sizeof(rf_iqm_vertex_array));
    memcpy(va, data + iqm.ofs_vertexarrays, iqm.num_vertexarrays * sizeof(rf_iqm_vertex_array));

    for (int i = 0; i < iqm.num_vertexarrays; i++)
    {
        switch (va[i].type)
        {
            case RF_IQM_POSITION:
            {
                vertex = (float*) RF_ALLOC(temp_allocator, iqm.num_vertexes * 3 * sizeof(float));
                memcpy(vertex, data + va[i].offset, iqm.num_vertexes * 3 * sizeof(float));

                for (int m = 0; m < iqm.num_meshes; m++)
                {
                    int vertex_pos_counter = 0;
                    for (int ii = imesh[m].first_vertex * 3; ii < (imesh[m].first_vertex + imesh[m].num_vertexes) * 3; ii++)
                    {
                        model.meshes[m].vertices     [vertex_pos_counter] = vertex[ii];
                        model.meshes[m].anim_vertices[vertex_pos_counter] = vertex[ii];
                        vertex_pos_counter++;
                    }
                }
            } break;

            case RF_IQM_NORMAL:
            {
                normal = (float*) RF_ALLOC(temp_allocator, iqm.num_vertexes * 3 * sizeof(float));
                memcpy(normal, data + va[i].offset, iqm.num_vertexes * 3 * sizeof(float));

                for (int m = 0; m < iqm.num_meshes; m++)
                {
                    int vertex_pos_counter = 0;
                    for (int ii = imesh[m].first_vertex * 3; ii < (imesh[m].first_vertex + imesh[m].num_vertexes) * 3; ii++)
                    {
                        model.meshes[m].normals     [vertex_pos_counter] = normal[ii];
                        model.meshes[m].anim_normals[vertex_pos_counter] = normal[ii];
                        vertex_pos_counter++;
                    }
                }
            } break;

            case RF_IQM_TEXCOORD:
            {
                text = (float*) RF_ALLOC(temp_allocator, iqm.num_vertexes * 2 * sizeof(float));
                memcpy(text, data + va[i].offset, iqm.num_vertexes * 2 * sizeof(float));

                for (int m = 0; m < iqm.num_meshes; m++)
                {
                    int vertex_pos_counter = 0;
                    for (int ii = imesh[m].first_vertex * 2; ii < (imesh[m].first_vertex + imesh[m].num_vertexes) * 2; ii++)
                    {
                        model.meshes[m].texcoords[vertex_pos_counter] = text[ii];
                        vertex_pos_counter++;
                    }
                }
            } break;

            case RF_IQM_BLENDINDEXES:
            {
                blendi = (char*) RF_ALLOC(temp_allocator, iqm.num_vertexes * 4 * sizeof(char));
                memcpy(blendi, data + va[i].offset, iqm.num_vertexes * 4 * sizeof(char));

                for (int m = 0; m < iqm.num_meshes; m++)
                {
                    int bone_counter = 0;
                    for (int ii = imesh[m].first_vertex * 4; ii < (imesh[m].first_vertex + imesh[m].num_vertexes) * 4; ii++)
                    {
                        model.meshes[m].bone_ids[bone_counter] = blendi[ii];
                        bone_counter++;
                    }
                }
            } break;

            case RF_IQM_BLENDWEIGHTS:
            {
                blendw = (unsigned char*) RF_ALLOC(temp_allocator, iqm.num_vertexes * 4 * sizeof(unsigned char));
                memcpy(blendw, data + va[i].offset, iqm.num_vertexes * 4 * sizeof(unsigned char));

                for (int m = 0; m < iqm.num_meshes; m++)
                {
                    int bone_counter = 0;
                    for (int ii = imesh[m].first_vertex * 4; ii < (imesh[m].first_vertex + imesh[m].num_vertexes) * 4; ii++)
                    {
                        model.meshes[m].bone_weights[bone_counter] = blendw[ii] / 255.0f;
                        bone_counter++;
                    }
                }
            } break;
        }
    }

    // Bones (joints) data processing
    ijoint = (rf_iqm_joint*) RF_ALLOC(temp_allocator, iqm.num_joints * sizeof(rf_iqm_joint));
    memcpy(ijoint, data + iqm.ofs_joints, iqm.num_joints * sizeof(rf_iqm_joint));

    model.bone_count = iqm.num_joints;
    model.bones      = (rf_bone_info*) RF_ALLOC(model.allocator, iqm.num_joints * sizeof(rf_bone_info));
    model.bind_pose  = (rf_transform*) RF_ALLOC(model.allocator, iqm.num_joints * sizeof(rf_transform));

    for (int i = 0; i < iqm.num_joints; i++)
    {
        // Bones
        model.bones[i].parent = ijoint[i].parent;
        memcpy(model.bones[i].name, data + iqm.ofs_text + ijoint[i].name, RF_BONE_NAME_LENGTH * sizeof(char));

        // Bind pose (base pose)
        model.bind_pose[i].translation.x = ijoint[i].translate[0];
        model.bind_pose[i].translation.y = ijoint[i].translate[1];
        model.bind_pose[i].translation.z = ijoint[i].translate[2];

        model.bind_pose[i].rotation.x = ijoint[i].rotate[0];
        model.bind_pose[i].rotation.y = ijoint[i].rotate[1];
        model.bind_pose[i].rotation.z = ijoint[i].rotate[2];
        model.bind_pose[i].rotation.w = ijoint[i].rotate[3];

        model.bind_pose[i].scale.x = ijoint[i].scale[0];
        model.bind_pose[i].scale.y = ijoint[i].scale[1];
        model.bind_pose[i].scale.z = ijoint[i].scale[2];
    }

    // Build bind pose from parent joints
    for (int i = 0; i < model.bone_count; i++)
    {
        if (model.bones[i].parent >= 0)
        {
            model.bind_pose[i].rotation    = rf_quaternion_mul(model.bind_pose[model.bones[i].parent].rotation, model.bind_pose[i].rotation);
            model.bind_pose[i].translation = rf_vec3_rotate_by_quaternion(model.bind_pose[i].translation, model.bind_pose[model.bones[i].parent].rotation);
            model.bind_pose[i].translation = rf_vec3_add(model.bind_pose[i].translation, model.bind_pose[model.bones[i].parent].translation);
            model.bind_pose[i].scale       = rf_vec3_mul_v(model.bind_pose[i].scale, model.bind_pose[model.bones[i].parent].scale);
        }
    }

    RF_FREE(temp_allocator, imesh);
    RF_FREE(temp_allocator, tri);
    RF_FREE(temp_allocator, va);
    RF_FREE(temp_allocator, vertex);
    RF_FREE(temp_allocator, normal);
    RF_FREE(temp_allocator, text);
    RF_FREE(temp_allocator, blendi);
    RF_FREE(temp_allocator, blendw);
    RF_FREE(temp_allocator, ijoint);

    return rf_internal_load_meshes_and_materials_for_model(model, temp_allocator);
}

/***********************************************************************************
    Function based on work by Wilhem Barbier (@wbrbr)

    Features:
      - Supports .gltf and .glb files
      - Supports embedded (base64) or external textures
      - Loads the albedo/diffuse texture (other maps could be added)
      - Supports multiple mesh per model and multiple primitives per model

    Some restrictions (not exhaustive):
      - Triangle-only meshes
      - Not supported node hierarchies or transforms
      - Only loads the diffuse texture... but not too hard to support other maps (normal, roughness/metalness...)
      - Only supports unsigned short indices (no unsigned char/unsigned int)
      - Only supports float for texture coordinates (no unsigned char/unsigned short)
    *************************************************************************************/
// Load texture from cgltf_image
RF_INTERNAL rf_texture2d rf_internal_load_texture_from_cgltf_image(cgltf_image* image, const char* tex_path, rf_color tint, rf_allocator temp_allocator, rf_io_callbacks io)
{
    rf_texture2d texture = { 0 };

    if (image->uri)
    {
        if ((strlen(image->uri) > 5) &&
            (image->uri[0] == 'd') &&
            (image->uri[1] == 'a') &&
            (image->uri[2] == 't') &&
            (image->uri[3] == 'a') &&
            (image->uri[4] == ':'))
        {
            // Data URI
            // Format: data:<mediatype>;base64,<data>

            // Find the comma
            int i = 0;
            while ((image->uri[i] != ',') && (image->uri[i] != 0)) i++;

            if (image->uri[i] == 0) RF_LOG(RF_LOG_WARNING, "CGLTF rf_image: Invalid data URI");
            else
            {
                rf_base64_output data = rf_decode_base64((const unsigned char*)image->uri + i + 1, temp_allocator);

                RF_SET_STBI_ALLOCATOR(&temp_allocator);
                int w, h;
                unsigned char* raw = stbi_load_from_memory(data.buffer, data.size, &w, &h, NULL, 4);
                RF_SET_STBI_ALLOCATOR(NULL);

                rf_image rimage = {
                    .data      = raw,
                    .width     = w,
                    .height    = h,
                    .mipmaps   = 1,
                    .format    = RF_UNCOMPRESSED_R8G8B8A8,
                    .allocator = temp_allocator
                };

                // TODO: Tint shouldn't be applied here!
                rf_image_color_tint(&rimage, tint, temp_allocator);

                texture = rf_load_texture_from_image(rimage);

                rf_unload_image(rimage);
                rf_unload_base64_output(data);
            }
        }
        else
        {
            char buff[1024];
            snprintf(buff, 1024, "%s/%s", tex_path, image->uri);
            rf_image rimage = rf_load_image_from_file(buff, temp_allocator, temp_allocator, io);

            // TODO: Tint shouldn't be applied here!
            rf_image_color_tint(&rimage, tint, temp_allocator);

            texture = rf_load_texture_from_image(rimage);

            rf_unload_image(rimage);
        }
    }
    else if (image->buffer_view)
    {
        unsigned char* data = (unsigned char*) RF_ALLOC(temp_allocator, image->buffer_view->size);
        int n = image->buffer_view->offset;
        int stride = image->buffer_view->stride ? image->buffer_view->stride : 1;

        for (int i = 0; i < image->buffer_view->size; i++)
        {
            data[i] = ((unsigned char* )image->buffer_view->buffer->data)[n];
            n += stride;
        }

        int w, h;
        RF_SET_STBI_ALLOCATOR(&temp_allocator);
        unsigned char* raw = stbi_load_from_memory(data, image->buffer_view->size, &w, &h, NULL, 4);
        RF_SET_STBI_ALLOCATOR(NULL);

        rf_image rimage = {
            .data      = raw,
            .width     = w,
            .height    = h,
            .mipmaps   = 1,
            .format    = RF_UNCOMPRESSED_R8G8B8A8,
            .allocator = temp_allocator
        };

        // TODO: Tint shouldn't be applied here!
        rf_image_color_tint(&rimage, tint, temp_allocator);

        texture = rf_load_texture_from_image(rimage);

        rf_unload_image(rimage);
        RF_FREE(temp_allocator, data);
        RF_FREE(temp_allocator, raw);
    }
    else
    {
        rf_image rimage = rf_load_image_from_pixels(&tint, 1, 1, temp_allocator);
        texture = rf_load_texture_from_image(rimage);
        rf_unload_image(rimage);
    }

    return texture;
}

// Load model from files (meshes and materials)
RF_API rf_model rf_load_model_from_gltf(const char* filename, rf_allocator allocator, rf_allocator temp_allocator, rf_io_callbacks io)
{
    #define rf_load_accessor(type, nbcomp, acc, dst)\
    { \
        int n = 0;\
        type* buf = (type*)acc->buffer_view->buffer->data+acc->buffer_view->offset/sizeof(type)+acc->offset/sizeof(type);\
        for (int k = 0; k < acc->count; k++) {\
            for (int l = 0; l < nbcomp; l++) {\
                dst[nbcomp*k+l] = buf[n+l];\
            }\
            n += acc->stride/sizeof(type);\
        }\
    }

    RF_SET_CGLTF_ALLOCATOR(&temp_allocator);
    rf_model model = { 0 };
    
    cgltf_options options = {
        cgltf_file_type_invalid,
        .file = {
            .read = &rf_internal_cgltf_io_read,
            .release = &rf_internal_cgltf_io_release,
            .user_data = &io
        }
    };
    
    int data_size = io.get_file_size_proc(filename);
    void* data = RF_ALLOC(temp_allocator, data_size);
    if (!io.read_file_into_buffer_proc(filename, data, data_size))
    {
        RF_FREE(temp_allocator, data);
        RF_SET_CGLTF_ALLOCATOR(&temp_allocator);
        return model;    
    }
    
    cgltf_data* cgltf_data = NULL;
    cgltf_result result = cgltf_parse(&options, data, data_size, &cgltf_data);

    if (result == cgltf_result_success)
    {
        RF_LOG_V(RF_LOG_INFO, "[%s][%s] rf_model meshes/materials: %i/%i", filename, (cgltf_data->file_type == 2) ? "glb" : "gltf", cgltf_data->meshes_count, cgltf_data->materials_count);

        // Read cgltf_data buffers
        result = cgltf_load_buffers(&options, cgltf_data, filename);
        if (result != cgltf_result_success) RF_LOG_V(RF_LOG_INFO, "[%s][%s] Error loading mesh/material buffers", file_name, (cgltf_data->file_type == 2) ? "glb" : "gltf");

        int primitivesCount = 0;

        for (int i = 0; i < cgltf_data->meshes_count; i++) primitivesCount += (int)cgltf_data->meshes[i].primitives_count;

        // Process glTF cgltf_data and map to model
        model.allocator = allocator;
        model.mesh_count = primitivesCount;
        model.material_count = cgltf_data->materials_count + 1;
        model.meshes = (rf_mesh*) RF_ALLOC(allocator, model.mesh_count * sizeof(rf_mesh));
        model.materials = (rf_material*) RF_ALLOC(allocator, model.material_count * sizeof(rf_material));
        model.mesh_material = (int*) RF_ALLOC(allocator, model.mesh_count * sizeof(int));

        memset(model.meshes, 0, model.mesh_count * sizeof(rf_mesh));

        for (int i = 0; i < model.mesh_count; i++)
        {
            model.meshes[i].vbo_id = (unsigned int*) RF_ALLOC(allocator, RF_MAX_MESH_VBO * sizeof(unsigned int));
            model.meshes[i].allocator = allocator;
            memset(model.meshes[i].vbo_id, 0, RF_MAX_MESH_VBO * sizeof(unsigned int));
        }

        //For each material
        for (int i = 0; i < model.material_count - 1; i++)
        {
            model.materials[i] = rf_load_default_material(allocator);
            rf_color tint = (rf_color){ 255, 255, 255, 255 };
            const char* tex_path = rf_internal_get_directory_path(filename);

            //Ensure material follows raylib support for PBR (metallic/roughness flow)
            if (cgltf_data->materials[i].has_pbr_metallic_roughness)
            {
                float roughness = cgltf_data->materials[i].pbr_metallic_roughness.roughness_factor;
                float metallic = cgltf_data->materials[i].pbr_metallic_roughness.metallic_factor;

                // NOTE: rf_material name not used for the moment
                //if (model.materials[i].name && cgltf_data->materials[i].name) strcpy(model.materials[i].name, cgltf_data->materials[i].name);

                // TODO: REview: shouldn't these be *255 ???
                tint.r = (unsigned char)(cgltf_data->materials[i].pbr_metallic_roughness.base_color_factor[0] * 255);
                tint.g = (unsigned char)(cgltf_data->materials[i].pbr_metallic_roughness.base_color_factor[1] * 255);
                tint.b = (unsigned char)(cgltf_data->materials[i].pbr_metallic_roughness.base_color_factor[2] * 255);
                tint.a = (unsigned char)(cgltf_data->materials[i].pbr_metallic_roughness.base_color_factor[3] * 255);

                model.materials[i].maps[RF_MAP_ROUGHNESS].color = tint;

                if (cgltf_data->materials[i].pbr_metallic_roughness.base_color_texture.texture)
                {
                    model.materials[i].maps[RF_MAP_ALBEDO].texture = rf_internal_load_texture_from_cgltf_image(cgltf_data->materials[i].pbr_metallic_roughness.base_color_texture.texture->image, tex_path, tint, temp_allocator, io);
                }

                // NOTE: Tint isn't need for other textures.. pass null or clear?
                // Just set as white, multiplying by white has no effect
                tint = RF_WHITE;

                if (cgltf_data->materials[i].pbr_metallic_roughness.metallic_roughness_texture.texture)
                {
                    model.materials[i].maps[RF_MAP_ROUGHNESS].texture = rf_internal_load_texture_from_cgltf_image(cgltf_data->materials[i].pbr_metallic_roughness.metallic_roughness_texture.texture->image, tex_path, tint, temp_allocator, io);
                }
                model.materials[i].maps[RF_MAP_ROUGHNESS].value = roughness;
                model.materials[i].maps[RF_MAP_METALNESS].value = metallic;

                if (cgltf_data->materials[i].normal_texture.texture)
                {
                    model.materials[i].maps[RF_MAP_NORMAL].texture = rf_internal_load_texture_from_cgltf_image(cgltf_data->materials[i].normal_texture.texture->image, tex_path, tint, temp_allocator, io);
                }

                if (cgltf_data->materials[i].occlusion_texture.texture)
                {
                    model.materials[i].maps[RF_MAP_OCCLUSION].texture = rf_internal_load_texture_from_cgltf_image(cgltf_data->materials[i].occlusion_texture.texture->image, tex_path, tint, temp_allocator, io);
                }
            }
        }

        model.materials[model.material_count - 1] = rf_load_default_material(allocator);

        int primitiveIndex = 0;

        for (int i = 0; i < cgltf_data->meshes_count; i++)
        {
            for (int p = 0; p < cgltf_data->meshes[i].primitives_count; p++)
            {
                for (int j = 0; j < cgltf_data->meshes[i].primitives[p].attributes_count; j++)
                {
                    if (cgltf_data->meshes[i].primitives[p].attributes[j].type == cgltf_attribute_type_position)
                    {
                        cgltf_accessor* acc = cgltf_data->meshes[i].primitives[p].attributes[j].data;
                        model.meshes[primitiveIndex].vertex_count = acc->count;
                        model.meshes[primitiveIndex].vertices = (float*) RF_ALLOC(model.meshes[primitiveIndex].allocator, sizeof(float)*model.meshes[primitiveIndex].vertex_count * 3);

                        rf_load_accessor(float, 3, acc, model.meshes[primitiveIndex].vertices)
                    }
                    else if (cgltf_data->meshes[i].primitives[p].attributes[j].type == cgltf_attribute_type_normal)
                    {
                        cgltf_accessor* acc = cgltf_data->meshes[i].primitives[p].attributes[j].data;
                        model.meshes[primitiveIndex].normals = (float*) RF_ALLOC(model.meshes[primitiveIndex].allocator, sizeof(float)*acc->count * 3);

                        rf_load_accessor(float, 3, acc, model.meshes[primitiveIndex].normals)
                    }
                    else if (cgltf_data->meshes[i].primitives[p].attributes[j].type == cgltf_attribute_type_texcoord)
                    {
                        cgltf_accessor* acc = cgltf_data->meshes[i].primitives[p].attributes[j].data;

                        if (acc->component_type == cgltf_component_type_r_32f)
                        {
                            model.meshes[primitiveIndex].texcoords = (float*) RF_ALLOC(model.meshes[primitiveIndex].allocator, sizeof(float)*acc->count * 2);
                            rf_load_accessor(float, 2, acc, model.meshes[primitiveIndex].texcoords)
                        }
                        else
                        {
                            // TODO: Support normalized unsigned unsigned char/unsigned short texture coordinates
                            RF_LOG_V(RF_LOG_WARNING, "[%s] rf_texture coordinates must be float", filename);
                        }
                    }
                }

                cgltf_accessor* acc = cgltf_data->meshes[i].primitives[p].indices;

                if (acc)
                {
                    if (acc->component_type == cgltf_component_type_r_16u)
                    {
                        model.meshes[primitiveIndex].triangle_count = acc->count/3;
                        model.meshes[primitiveIndex].indices = (unsigned short*) RF_ALLOC(model.meshes[primitiveIndex].allocator, sizeof(unsigned short)*model.meshes[primitiveIndex].triangle_count * 3);
                        rf_load_accessor(unsigned short, 1, acc, model.meshes[primitiveIndex].indices)
                    }
                    else
                    {
                        // TODO: Support unsigned unsigned char/unsigned int
                        RF_LOG_V(RF_LOG_WARNING, "[%s] Indices must be unsigned short", filename);
                    }
                }
                else
                {
                    // Unindexed mesh
                    model.meshes[primitiveIndex].triangle_count = model.meshes[primitiveIndex].vertex_count/3;
                }

                if (cgltf_data->meshes[i].primitives[p].material)
                {
                    // Compute the offset
                    model.mesh_material[primitiveIndex] = cgltf_data->meshes[i].primitives[p].material - cgltf_data->materials;
                }
                else
                {
                    model.mesh_material[primitiveIndex] = model.material_count - 1;
                }

                primitiveIndex++;
            }
        }

        cgltf_free(cgltf_data);
    }
    else
    {
        RF_LOG_V(RF_LOG_WARNING, "[%s] glTF cgltf_data could not be loaded", file_name);
    }

    RF_SET_CGLTF_ALLOCATOR(NULL);

    return model;
}

// Load model from generated mesh. Note: The function takes ownership of the mesh in model.meshes[0]
RF_API rf_model rf_load_model_with_mesh(rf_mesh mesh, rf_allocator allocator)
{
    rf_model model = { 0 };

    model.transform = rf_mat_identity();

    model.mesh_count = 1;
    model.meshes = (rf_mesh*) RF_ALLOC(allocator, model.mesh_count * sizeof(rf_mesh));
    memset(model.meshes, 0, model.mesh_count * sizeof(rf_mesh));
    model.meshes[0] = mesh;

    model.material_count = 1;
    model.materials = (rf_material*) RF_ALLOC(allocator, model.material_count * sizeof(rf_material));
    memset(model.materials, 0, model.material_count * sizeof(rf_material));
    model.materials[0] = rf_load_default_material(allocator);

    model.mesh_material = (int*) RF_ALLOC(allocator, model.mesh_count * sizeof(int));
    memset(model.mesh_material, 0, model.mesh_count * sizeof(int));
    model.mesh_material[0] = 0; // First material index

    return model;
}

// Unload model from memory (RAM and/or VRAM)
RF_API void rf_unload_model(rf_model model)
{
    for (int i = 0; i < model.mesh_count; i++) rf_unload_mesh(model.meshes[i]);

    // As the user could be sharing shaders and textures between models,
    // we don't unload the material but just free it's maps, the user
    // is responsible for freeing models shaders and textures
    for (int i = 0; i < model.material_count; i++) RF_FREE(model.allocator, model.materials[i].maps);

    RF_FREE(model.allocator, model.meshes);
    RF_FREE(model.allocator, model.materials);
    RF_FREE(model.allocator, model.mesh_material);

    // Unload animation data
    RF_FREE(model.allocator, model.bones);
    RF_FREE(model.allocator, model.bind_pose);

    RF_LOG(RF_LOG_INFO, "Unloaded model data from RAM and VRAM");
}

// Load materials from model file
RF_API rf_material* rf_load_materials_from_mtl(const char* data, int data_size, int* material_count, rf_allocator allocator)
{
    RF_SET_TINYOBJ_ALLOCATOR(&allocator);

    rf_material* materials = NULL;
    unsigned int count = 0;

    // TODO: Support IQM and GLTF for materials parsing

    tinyobj_material_t* mats;

    if (tinyobj_parse_mtl_file(&mats, (size_t*) &count, data, data_size) != TINYOBJ_SUCCESS)
    {

    }

    // TODO: Process materials to return

    tinyobj_materials_free(mats, count);

    // Set materials shader to default (DIFFUSE, SPECULAR, NORMAL)
    for (int i = 0; i < count; i++)
    {
        materials[i].shader = rf_get_default_shader();
    }

    *material_count = count;
    return materials;
}

RF_API void rf_unload_material(rf_material material)
{
    // Unload material shader (avoid unloading default shader, managed by raylib)
    if (material.shader.id != rf_get_default_shader().id)
    {
        rf_gfx_unload_shader(material.shader);
    }

    // Unload loaded texture maps (avoid unloading default texture, managed by raylib)
    for (int i = 0; i < RF_MAX_MATERIAL_MAPS; i++)
    {
        if (material.maps[i].texture.id != rf_get_default_texture().id)
        {
            rf_gfx_delete_textures(material.maps[i].texture.id);
        }
    }

    RF_FREE(material.allocator, material.maps);
}

RF_API void rf_set_material_texture(rf_material* material, int map_type, rf_texture2d texture); // Set texture for a material map type (rf_map_diffuse, rf_map_specular...)

RF_API void rf_set_model_mesh_material(rf_model* model, int mesh_id, int material_id); // Set material for a mesh

// Generated cuboid mesh

RF_API rf_model_animation_array rf_load_model_animations_from_iqm_file(const char* filename, rf_allocator allocator, rf_allocator temp_allocator, rf_io_callbacks io)
{
    int size = io.get_file_size_proc(filename);
    void* data = RF_ALLOC(temp_allocator, size);
    
    rf_model_animation_array result = rf_load_model_animations_from_iqm(data, size, allocator, temp_allocator);
    
    RF_FREE(temp_allocator, data);
    
    return result;
}

RF_API rf_model_animation_array rf_load_model_animations_from_iqm(const unsigned char* data, int data_size, rf_allocator allocator, rf_allocator temp_allocator)
{
    if (!data || !data_size) return (rf_model_animation_array) { 0 };

    #define RF_IQM_MAGIC "INTERQUAKEMODEL" // IQM file magic number
    #define RF_IQM_VERSION 2 // only IQM version 2 supported

    typedef struct rf_iqm_header rf_iqm_header;
    struct rf_iqm_header
    {
        char magic[16];
        unsigned int version;
        unsigned int filesize;
        unsigned int flags;
        unsigned int num_text, ofs_text;
        unsigned int num_meshes, ofs_meshes;
        unsigned int num_vertexarrays, num_vertexes, ofs_vertexarrays;
        unsigned int num_triangles, ofs_triangles, ofs_adjacency;
        unsigned int num_joints, ofs_joints;
        unsigned int num_poses, ofs_poses;
        unsigned int num_anims, ofs_anims;
        unsigned int num_frames, num_framechannels, ofs_frames, ofs_bounds;
        unsigned int num_comment, ofs_comment;
        unsigned int num_extensions, ofs_extensions;
    };

    typedef struct rf_iqm_pose rf_iqm_pose;
    struct rf_iqm_pose
    {
        int parent;
        unsigned int mask;
        float channeloffset[10];
        float channelscale[10];
    };

    typedef struct rf_iqm_anim rf_iqm_anim;
    struct rf_iqm_anim
    {
        unsigned int name;
        unsigned int first_frame, num_frames;
        float framerate;
        unsigned int flags;
    };

    rf_iqm_header iqm;

    // Read IQM header
    memcpy(&iqm, data, sizeof(rf_iqm_header));

    if (strncmp(iqm.magic, RF_IQM_MAGIC, sizeof(RF_IQM_MAGIC)))
    {
        RF_LOG_V(RF_LOG_ERROR, "Magic Number \"%s\"does not match.", iqm.magic);

        return (rf_model_animation_array){ 0 };
    }

    if (iqm.version != RF_IQM_VERSION)
    {
        RF_LOG_V(RF_LOG_ERROR, "IQM version %i is incorrect.", iqm.version);

        return (rf_model_animation_array){ 0 };
    }

    rf_model_animation_array result = {
        .anims_count = iqm.num_anims,
        .allocator = allocator
    };

    // Get bones data
    rf_iqm_pose* poses = (rf_iqm_pose*) RF_ALLOC(temp_allocator, iqm.num_poses * sizeof(rf_iqm_pose));
    memcpy(poses, data + iqm.ofs_poses, iqm.num_poses * sizeof(rf_iqm_pose));

    // Get animations data
    rf_iqm_anim* anim = (rf_iqm_anim*) RF_ALLOC(temp_allocator, iqm.num_anims * sizeof(rf_iqm_anim));
    memcpy(anim, data + iqm.ofs_anims, iqm.num_anims * sizeof(rf_iqm_anim));

    rf_model_animation* animations = (rf_model_animation*) RF_ALLOC(allocator, iqm.num_anims * sizeof(rf_model_animation));

    result.anims       = animations;
    result.anims_count = iqm.num_anims;

    // frameposes
    unsigned short* framedata = (unsigned short*) RF_ALLOC(temp_allocator, iqm.num_frames * iqm.num_framechannels * sizeof(unsigned short));
    memcpy(framedata, data + iqm.ofs_frames, iqm.num_frames*iqm.num_framechannels * sizeof(unsigned short));

    for (int a = 0; a < iqm.num_anims; a++)
    {
        animations[a].frame_count = anim[a].num_frames;
        animations[a].bone_count  = iqm.num_poses;
        animations[a].bones       = (rf_bone_info*) RF_ALLOC(allocator, iqm.num_poses * sizeof(rf_bone_info));
        animations[a].frame_poses = (rf_transform**) RF_ALLOC(allocator, anim[a].num_frames * sizeof(rf_transform*));
        animations[a].allocator   = allocator;
        //animations[a].framerate = anim.framerate;     // TODO: Use framerate?

        for (int j = 0; j < iqm.num_poses; j++)
        {
            strcpy(animations[a].bones[j].name, "ANIMJOINTNAME");
            animations[a].bones[j].parent = poses[j].parent;
        }

        for (int j = 0; j < anim[a].num_frames; j++)
        {
            animations[a].frame_poses[j] = (rf_transform*) RF_ALLOC(allocator, iqm.num_poses * sizeof(rf_transform));
        }

        int dcounter = anim[a].first_frame*iqm.num_framechannels;

        for (int frame = 0; frame < anim[a].num_frames; frame++)
        {
            for (int i = 0; i < iqm.num_poses; i++)
            {
                animations[a].frame_poses[frame][i].translation.x = poses[i].channeloffset[0];

                if (poses[i].mask & 0x01)
                {
                    animations[a].frame_poses[frame][i].translation.x += framedata[dcounter]*poses[i].channelscale[0];
                    dcounter++;
                }

                animations[a].frame_poses[frame][i].translation.y = poses[i].channeloffset[1];

                if (poses[i].mask & 0x02)
                {
                    animations[a].frame_poses[frame][i].translation.y += framedata[dcounter]*poses[i].channelscale[1];
                    dcounter++;
                }

                animations[a].frame_poses[frame][i].translation.z = poses[i].channeloffset[2];

                if (poses[i].mask & 0x04)
                {
                    animations[a].frame_poses[frame][i].translation.z += framedata[dcounter]*poses[i].channelscale[2];
                    dcounter++;
                }

                animations[a].frame_poses[frame][i].rotation.x = poses[i].channeloffset[3];

                if (poses[i].mask & 0x08)
                {
                    animations[a].frame_poses[frame][i].rotation.x += framedata[dcounter]*poses[i].channelscale[3];
                    dcounter++;
                }

                animations[a].frame_poses[frame][i].rotation.y = poses[i].channeloffset[4];

                if (poses[i].mask & 0x10)
                {
                    animations[a].frame_poses[frame][i].rotation.y += framedata[dcounter]*poses[i].channelscale[4];
                    dcounter++;
                }

                animations[a].frame_poses[frame][i].rotation.z = poses[i].channeloffset[5];

                if (poses[i].mask & 0x20)
                {
                    animations[a].frame_poses[frame][i].rotation.z += framedata[dcounter]*poses[i].channelscale[5];
                    dcounter++;
                }

                animations[a].frame_poses[frame][i].rotation.w = poses[i].channeloffset[6];

                if (poses[i].mask & 0x40)
                {
                    animations[a].frame_poses[frame][i].rotation.w += framedata[dcounter]*poses[i].channelscale[6];
                    dcounter++;
                }

                animations[a].frame_poses[frame][i].scale.x = poses[i].channeloffset[7];

                if (poses[i].mask & 0x80)
                {
                    animations[a].frame_poses[frame][i].scale.x += framedata[dcounter]*poses[i].channelscale[7];
                    dcounter++;
                }

                animations[a].frame_poses[frame][i].scale.y = poses[i].channeloffset[8];

                if (poses[i].mask & 0x100)
                {
                    animations[a].frame_poses[frame][i].scale.y += framedata[dcounter]*poses[i].channelscale[8];
                    dcounter++;
                }

                animations[a].frame_poses[frame][i].scale.z = poses[i].channeloffset[9];

                if (poses[i].mask & 0x200)
                {
                    animations[a].frame_poses[frame][i].scale.z += framedata[dcounter]*poses[i].channelscale[9];
                    dcounter++;
                }

                animations[a].frame_poses[frame][i].rotation = rf_quaternion_normalize(animations[a].frame_poses[frame][i].rotation);
            }
        }

        // Build frameposes
        for (int frame = 0; frame < anim[a].num_frames; frame++)
        {
            for (int i = 0; i < animations[a].bone_count; i++)
            {
                if (animations[a].bones[i].parent >= 0)
                {
                    animations[a].frame_poses[frame][i].rotation    = rf_quaternion_mul(animations[a].frame_poses[frame][animations[a].bones[i].parent].rotation, animations[a].frame_poses[frame][i].rotation);
                    animations[a].frame_poses[frame][i].translation = rf_vec3_rotate_by_quaternion(animations[a].frame_poses[frame][i].translation, animations[a].frame_poses[frame][animations[a].bones[i].parent].rotation);
                    animations[a].frame_poses[frame][i].translation = rf_vec3_add(animations[a].frame_poses[frame][i].translation, animations[a].frame_poses[frame][animations[a].bones[i].parent].translation);
                    animations[a].frame_poses[frame][i].scale       = rf_vec3_mul_v(animations[a].frame_poses[frame][i].scale, animations[a].frame_poses[frame][animations[a].bones[i].parent].scale);
                }
            }
        }
    }

    RF_FREE(temp_allocator, framedata);
    RF_FREE(temp_allocator, poses);
    RF_FREE(temp_allocator, anim);

    return result;
}

// Update model animated vertex data (positions and normals) for a given frame
RF_API void rf_update_model_animation(rf_model model, rf_model_animation anim, int frame)
{
    if ((anim.frame_count > 0) && (anim.bones != NULL) && (anim.frame_poses != NULL))
    {
        return;
    }
    
    if (frame >= anim.frame_count) 
    {
        frame = frame%anim.frame_count;
    }

    for (int m = 0; m < model.mesh_count; m++)
    {
        rf_vec3 anim_vertex = { 0 };
        rf_vec3 anim_normal = { 0 };

        rf_vec3 in_translation = { 0 };
        rf_quaternion in_rotation = { 0 };
        rf_vec3 in_scale = { 0 };

        rf_vec3 out_translation = { 0 };
        rf_quaternion out_rotation = { 0 };
        rf_vec3 out_scale = { 0 };

        int vertex_pos_counter = 0;
        int bone_counter = 0;
        int bone_id = 0;

        for (int i = 0; i < model.meshes[m].vertex_count; i++)
        {
            bone_id = model.meshes[m].bone_ids[bone_counter];
            in_translation = model.bind_pose[bone_id].translation;
            in_rotation = model.bind_pose[bone_id].rotation;
            in_scale = model.bind_pose[bone_id].scale;
            out_translation = anim.frame_poses[frame][bone_id].translation;
            out_rotation = anim.frame_poses[frame][bone_id].rotation;
            out_scale = anim.frame_poses[frame][bone_id].scale;

            // Vertices processing
            // NOTE: We use meshes.vertices (default vertex position) to calculate meshes.anim_vertices (animated vertex position)
            anim_vertex = (rf_vec3){model.meshes[m].vertices[vertex_pos_counter], model.meshes[m].vertices[vertex_pos_counter + 1], model.meshes[m].vertices[vertex_pos_counter + 2] };
            anim_vertex = rf_vec3_mul_v(anim_vertex, out_scale);
            anim_vertex = rf_vec3_sub(anim_vertex, in_translation);
            anim_vertex = rf_vec3_rotate_by_quaternion(anim_vertex, rf_quaternion_mul(out_rotation, rf_quaternion_invert(in_rotation)));
            anim_vertex = rf_vec3_add(anim_vertex, out_translation);
            model.meshes[m].anim_vertices[vertex_pos_counter] = anim_vertex.x;
            model.meshes[m].anim_vertices[vertex_pos_counter + 1] = anim_vertex.y;
            model.meshes[m].anim_vertices[vertex_pos_counter + 2] = anim_vertex.z;

            // Normals processing
            // NOTE: We use meshes.baseNormals (default normal) to calculate meshes.normals (animated normals)
            anim_normal = (rf_vec3){model.meshes[m].normals[vertex_pos_counter], model.meshes[m].normals[vertex_pos_counter + 1], model.meshes[m].normals[vertex_pos_counter + 2] };
            anim_normal = rf_vec3_rotate_by_quaternion(anim_normal, rf_quaternion_mul(out_rotation, rf_quaternion_invert(in_rotation)));
            model.meshes[m].anim_normals[vertex_pos_counter] = anim_normal.x;
            model.meshes[m].anim_normals[vertex_pos_counter + 1] = anim_normal.y;
            model.meshes[m].anim_normals[vertex_pos_counter + 2] = anim_normal.z;
            vertex_pos_counter += 3;

            bone_counter += 4;
        }

        // Upload new vertex data to GPU for model drawing
        rf_gfx_update_buffer(model.meshes[m].vbo_id[0], model.meshes[m].anim_vertices, model.meshes[m].vertex_count * 3 * sizeof(float)); // Update vertex position
        rf_gfx_update_buffer(model.meshes[m].vbo_id[2], model.meshes[m].anim_vertices, model.meshes[m].vertex_count * 3 * sizeof(float)); // Update vertex normals
    }
}

// Check model animation skeleton match. Only number of bones and parent connections are checked
RF_API bool rf_is_model_animation_valid(rf_model model, rf_model_animation anim)
{
    int result = true;

    if (model.bone_count != anim.bone_count) result = false;
    else
    {
        for (int i = 0; i < model.bone_count; i++)
        {
            if (model.bones[i].parent != anim.bones[i].parent) { result = false; break; }
        }
    }

    return result;
}

// Unload animation data
RF_API void rf_unload_model_animation(rf_model_animation anim)
{
    for (int i = 0; i < anim.frame_count; i++) RF_FREE(anim.allocator, anim.frame_poses[i]);

    RF_FREE(anim.allocator, anim.bones);
    RF_FREE(anim.allocator, anim.frame_poses);
}

//region mesh generation
RF_API rf_mesh rf_gen_mesh_cube(float width, float height, float length, rf_allocator allocator, rf_allocator temp_allocator)
{
    rf_mesh mesh = {0};
    mesh.allocator = allocator;
    mesh.vbo_id = (unsigned int *) RF_ALLOC(allocator, RF_MAX_MESH_VBO * sizeof(unsigned int));
    memset(mesh.vbo_id, 0, RF_MAX_MESH_VBO * sizeof(unsigned int));

    #define rf_custom_mesh_gen_cube //Todo: Investigate this macro
    /*
    Platonic solids:
    par_shapes_mesh* par_shapes_create_tetrahedron();       // 4 sides polyhedron (pyramid)
    par_shapes_mesh* par_shapes_create_cube();              // 6 sides polyhedron (cube)
    par_shapes_mesh* par_shapes_create_octahedron();        // 8 sides polyhedron (dyamond)
    par_shapes_mesh* par_shapes_create_dodecahedron();      // 12 sides polyhedron
    par_shapes_mesh* par_shapes_create_icosahedron();       // 20 sides polyhedron
    */

    // Platonic solid generation: cube (6 sides)
    // NOTE: No normals/texcoords generated by default
    //RF_SET_PARSHAPES_ALLOCATOR(temp_allocator);
    {
        par_shapes_mesh *cube = par_shapes_create_cube();
        cube->tcoords = PAR_MALLOC(float, 2 * cube->npoints);

        for (int i = 0; i < 2 * cube->npoints; i++)
        {
            cube->tcoords[i] = 0.0f;
        }

        par_shapes_scale(cube, width, height, length);
        par_shapes_translate(cube, -width / 2, 0.0f, -length / 2);
        par_shapes_compute_normals(cube);

        mesh.vertices = (float *) RF_ALLOC(mesh.allocator, cube->ntriangles * 3 * 3 * sizeof(float));
        mesh.texcoords = (float *) RF_ALLOC(mesh.allocator, cube->ntriangles * 3 * 2 * sizeof(float));
        mesh.normals = (float *) RF_ALLOC(mesh.allocator, cube->ntriangles * 3 * 3 * sizeof(float));

        mesh.vertex_count = cube->ntriangles * 3;
        mesh.triangle_count = cube->ntriangles;

        for (int k = 0; k < mesh.vertex_count; k++)
        {
            mesh.vertices[k * 3] = cube->points[cube->triangles[k] * 3];
            mesh.vertices[k * 3 + 1] = cube->points[cube->triangles[k] * 3 + 1];
            mesh.vertices[k * 3 + 2] = cube->points[cube->triangles[k] * 3 + 2];

            mesh.normals[k * 3] = cube->normals[cube->triangles[k] * 3];
            mesh.normals[k * 3 + 1] = cube->normals[cube->triangles[k] * 3 + 1];
            mesh.normals[k * 3 + 2] = cube->normals[cube->triangles[k] * 3 + 2];

            mesh.texcoords[k * 2] = cube->tcoords[cube->triangles[k] * 2];
            mesh.texcoords[k * 2 + 1] = cube->tcoords[cube->triangles[k] * 2 + 1];
        }

        par_shapes_free_mesh(cube);
    }
    //RF_SET_PARSHAPES_ALLOCATOR(NULL);

    // Upload vertex data to GPU (static mesh)
    rf_gfx_load_mesh(&mesh, false);

    return mesh;
}

// Generate polygonal mesh
RF_API rf_mesh rf_gen_mesh_poly(int sides, float radius, rf_allocator allocator, rf_allocator temp_allocator)
{
    rf_mesh mesh = { 0 };
    mesh.allocator = allocator;
    mesh.vbo_id = (unsigned int*) RF_ALLOC(allocator, RF_MAX_MESH_VBO * sizeof(unsigned int));
    memset(mesh.vbo_id, 0, RF_MAX_MESH_VBO * sizeof(unsigned int));
    int vertex_count = sides * 3;

    // Vertices definition
    rf_vec3* vertices = (rf_vec3*) RF_ALLOC(temp_allocator, vertex_count * sizeof(rf_vec3));
    for (int i = 0, v = 0; i < 360; i += 360/sides, v += 3)
    {
        vertices[v    ] = (rf_vec3){ 0.0f, 0.0f, 0.0f };
        vertices[v + 1] = (rf_vec3) { sinf(RF_DEG2RAD * i) * radius, 0.0f, cosf(RF_DEG2RAD * i) * radius };
        vertices[v + 2] = (rf_vec3) { sinf(RF_DEG2RAD * (i + 360 / sides)) * radius, 0.0f, cosf(RF_DEG2RAD * (i + 360 / sides)) * radius };
    }

    // Normals definition
    rf_vec3* normals = (rf_vec3*) RF_ALLOC(temp_allocator, vertex_count * sizeof(rf_vec3));
    for (int n = 0; n < vertex_count; n++) normals[n] = (rf_vec3){0.0f, 1.0f, 0.0f }; // rf_vec3.up;

    // TexCoords definition
    rf_vec2* texcoords = (rf_vec2*) RF_ALLOC(temp_allocator, vertex_count * sizeof(rf_vec2));
    for (int n = 0; n < vertex_count; n++) texcoords[n] = (rf_vec2) {0.0f, 0.0f };

    mesh.vertex_count = vertex_count;
    mesh.triangle_count = sides;
    mesh.vertices  = (float*) RF_ALLOC(allocator, mesh.vertex_count * 3 * sizeof(float));
    mesh.texcoords = (float*) RF_ALLOC(allocator, mesh.vertex_count * 2 * sizeof(float));
    mesh.normals   = (float*) RF_ALLOC(allocator, mesh.vertex_count * 3 * sizeof(float));

    // rf_mesh vertices position array
    for (int i = 0; i < mesh.vertex_count; i++)
    {
        mesh.vertices[3*i] = vertices[i].x;
        mesh.vertices[3*i + 1] = vertices[i].y;
        mesh.vertices[3*i + 2] = vertices[i].z;
    }

    // rf_mesh texcoords array
    for (int i = 0; i < mesh.vertex_count; i++)
    {
        mesh.texcoords[2*i] = texcoords[i].x;
        mesh.texcoords[2*i + 1] = texcoords[i].y;
    }

    // rf_mesh normals array
    for (int i = 0; i < mesh.vertex_count; i++)
    {
        mesh.normals[3*i] = normals[i].x;
        mesh.normals[3*i + 1] = normals[i].y;
        mesh.normals[3*i + 2] = normals[i].z;
    }

    RF_FREE(temp_allocator, vertices);
    RF_FREE(temp_allocator, normals);
    RF_FREE(temp_allocator, texcoords);

    // Upload vertex data to GPU (static mesh)
    rf_gfx_load_mesh(&mesh, false);

    return mesh;
}

// Generate plane mesh (with subdivisions)
RF_API rf_mesh rf_gen_mesh_plane(float width, float length, int res_x, int res_z, rf_allocator allocator, rf_allocator temp_allocator)
{
    rf_mesh mesh = { 0 };
    mesh.allocator = allocator;
    mesh.vbo_id = (unsigned int*) RF_ALLOC(mesh.allocator, RF_MAX_MESH_VBO * sizeof(unsigned int));
    memset(mesh.vbo_id, 0, RF_MAX_MESH_VBO * sizeof(unsigned int));

    #define rf_custom_mesh_gen_plane //Todo: Investigate this macro

    RF_SET_PARSHAPES_ALLOCATOR(&temp_allocator);
    {
        par_shapes_mesh* plane = par_shapes_create_plane(res_x, res_z); // No normals/texcoords generated!!!
        par_shapes_scale(plane, width, length, 1.0f);

        float axis[] = { 1, 0, 0 };
        par_shapes_rotate(plane, -RF_PI / 2.0f, axis);
        par_shapes_translate(plane, -width / 2, 0.0f, length / 2);

        mesh.vertices   = (float*) RF_ALLOC(mesh.allocator, plane->ntriangles * 3 * 3 * sizeof(float));
        mesh.texcoords  = (float*) RF_ALLOC(mesh.allocator, plane->ntriangles * 3 * 2 * sizeof(float));
        mesh.normals    = (float*) RF_ALLOC(mesh.allocator, plane->ntriangles * 3 * 3 * sizeof(float));
        mesh.vbo_id     = (unsigned int*) RF_ALLOC(mesh.allocator, RF_MAX_MESH_VBO * sizeof(unsigned int));
        memset(mesh.vbo_id, 0, RF_MAX_MESH_VBO * sizeof(unsigned int));

        mesh.vertex_count   = plane->ntriangles * 3;
        mesh.triangle_count = plane->ntriangles;

        for (int k = 0; k < mesh.vertex_count; k++)
        {
            mesh.vertices[k * 3    ] = plane->points[plane->triangles[k] * 3    ];
            mesh.vertices[k * 3 + 1] = plane->points[plane->triangles[k] * 3 + 1];
            mesh.vertices[k * 3 + 2] = plane->points[plane->triangles[k] * 3 + 2];

            mesh.normals[k * 3    ] = plane->normals[plane->triangles[k] * 3    ];
            mesh.normals[k * 3 + 1] = plane->normals[plane->triangles[k] * 3 + 1];
            mesh.normals[k * 3 + 2] = plane->normals[plane->triangles[k] * 3 + 2];

            mesh.texcoords[k * 2    ] = plane->tcoords[plane->triangles[k] * 2    ];
            mesh.texcoords[k * 2 + 1] = plane->tcoords[plane->triangles[k] * 2 + 1];
        }

        par_shapes_free_mesh(plane);
    }
    RF_SET_PARSHAPES_ALLOCATOR(NULL);

    // Upload vertex data to GPU (static mesh)
    rf_gfx_load_mesh(&mesh, false);

    return mesh;
}

// Generate sphere mesh (standard sphere)
RF_API rf_mesh rf_gen_mesh_sphere(float radius, int rings, int slices, rf_allocator allocator, rf_allocator temp_allocator)
{
    rf_mesh mesh = { 0 };
    mesh.allocator = allocator;
    mesh.vbo_id = (unsigned int*) RF_ALLOC(allocator, RF_MAX_MESH_VBO * sizeof(unsigned int));
    memset(mesh.vbo_id, 0, RF_MAX_MESH_VBO * sizeof(unsigned int));

    RF_SET_PARSHAPES_ALLOCATOR(&temp_allocator);
    {
        par_shapes_mesh* sphere = par_shapes_create_parametric_sphere(slices, rings);
        par_shapes_scale(sphere, radius, radius, radius);
        // NOTE: Soft normals are computed internally

        mesh.vertices  = (float*) RF_ALLOC(mesh.allocator, sphere->ntriangles * 3 * 3 * sizeof(float));
        mesh.texcoords = (float*) RF_ALLOC(mesh.allocator, sphere->ntriangles * 3 * 2 * sizeof(float));
        mesh.normals   = (float*) RF_ALLOC(mesh.allocator, sphere->ntriangles * 3 * 3 * sizeof(float));

        mesh.vertex_count = sphere->ntriangles * 3;
        mesh.triangle_count = sphere->ntriangles;

        for (int k = 0; k < mesh.vertex_count; k++)
        {
            mesh.vertices[k * 3    ] = sphere->points[sphere->triangles[k] * 3];
            mesh.vertices[k * 3 + 1] = sphere->points[sphere->triangles[k] * 3 + 1];
            mesh.vertices[k * 3 + 2] = sphere->points[sphere->triangles[k] * 3 + 2];

            mesh.normals[k * 3    ] = sphere->normals[sphere->triangles[k] * 3];
            mesh.normals[k * 3 + 1] = sphere->normals[sphere->triangles[k] * 3 + 1];
            mesh.normals[k * 3 + 2] = sphere->normals[sphere->triangles[k] * 3 + 2];

            mesh.texcoords[k * 2    ] = sphere->tcoords[sphere->triangles[k] * 2];
            mesh.texcoords[k * 2 + 1] = sphere->tcoords[sphere->triangles[k] * 2 + 1];
        }

        par_shapes_free_mesh(sphere);
    }
    RF_SET_PARSHAPES_ALLOCATOR(NULL);

    // Upload vertex data to GPU (static mesh)
    rf_gfx_load_mesh(&mesh, false);

    return mesh;
}

// Generate hemi-sphere mesh (half sphere, no bottom cap)
RF_API rf_mesh rf_gen_mesh_hemi_sphere(float radius, int rings, int slices, rf_allocator allocator, rf_allocator temp_allocator)
{
    rf_mesh mesh = { 0 };
    mesh.allocator = allocator;
    mesh.vbo_id = (unsigned int*) RF_ALLOC(mesh.allocator, RF_MAX_MESH_VBO * sizeof(unsigned int));
    memset(mesh.vbo_id, 0, RF_MAX_MESH_VBO * sizeof(unsigned int));

    RF_SET_PARSHAPES_ALLOCATOR(&temp_allocator);
    {
        par_shapes_mesh* sphere = par_shapes_create_hemisphere(slices, rings);
        par_shapes_scale(sphere, radius, radius, radius);
        // NOTE: Soft normals are computed internally

        mesh.vertices  = (float*) RF_ALLOC(mesh.allocator, sphere->ntriangles * 3 * 3 * sizeof(float));
        mesh.texcoords = (float*) RF_ALLOC(mesh.allocator, sphere->ntriangles * 3 * 2 * sizeof(float));
        mesh.normals   = (float*) RF_ALLOC(mesh.allocator, sphere->ntriangles * 3 * 3 * sizeof(float));

        mesh.vertex_count   = sphere->ntriangles * 3;
        mesh.triangle_count = sphere->ntriangles;

        for (int k = 0; k < mesh.vertex_count; k++)
        {
            mesh.vertices[k * 3    ] = sphere->points[sphere->triangles[k] * 3];
            mesh.vertices[k * 3 + 1] = sphere->points[sphere->triangles[k] * 3 + 1];
            mesh.vertices[k * 3 + 2] = sphere->points[sphere->triangles[k] * 3 + 2];

            mesh.normals[k * 3    ] = sphere->normals[sphere->triangles[k] * 3];
            mesh.normals[k * 3 + 1] = sphere->normals[sphere->triangles[k] * 3 + 1];
            mesh.normals[k * 3 + 2] = sphere->normals[sphere->triangles[k] * 3 + 2];

            mesh.texcoords[k * 2    ] = sphere->tcoords[sphere->triangles[k] * 2];
            mesh.texcoords[k * 2 + 1] = sphere->tcoords[sphere->triangles[k] * 2 + 1];
        }

        par_shapes_free_mesh(sphere);
    }
    RF_SET_PARSHAPES_ALLOCATOR(NULL);

    // Upload vertex data to GPU (static mesh)
    rf_gfx_load_mesh(&mesh, false);

    return mesh;
}

// Generate cylinder mesh
RF_API rf_mesh rf_gen_mesh_cylinder(float radius, float height, int slices, rf_allocator allocator, rf_allocator temp_allocator)
{
    rf_mesh mesh = { 0 };
    mesh.allocator = allocator;
    mesh.vbo_id = (unsigned int*) RF_ALLOC(mesh.allocator, RF_MAX_MESH_VBO * sizeof(unsigned int));
    memset(mesh.vbo_id, 0, RF_MAX_MESH_VBO * sizeof(unsigned int));

    RF_SET_PARSHAPES_ALLOCATOR(&temp_allocator);
    {
        // Instance a cylinder that sits on the Z=0 plane using the given tessellation
        // levels across the UV domain.  Think of "slices" like a number of pizza
        // slices, and "stacks" like a number of stacked rings.
        // Height and radius are both 1.0, but they can easily be changed with par_shapes_scale
        par_shapes_mesh* cylinder = par_shapes_create_cylinder(slices, 8);
        par_shapes_scale(cylinder, radius, radius, height);
        float axis[] = { 1, 0, 0 };
        par_shapes_rotate(cylinder, -RF_PI / 2.0f, axis);

        // Generate an orientable disk shape (top cap)
        float center[] = { 0, 0, 0 };
        float normal[] = { 0, 0, 1 };
        float normal_minus_1[] = { 0, 0, -1 };
        par_shapes_mesh* cap_top = par_shapes_create_disk(radius, slices, center, normal);
        cap_top->tcoords = PAR_MALLOC(float, 2*cap_top->npoints);
        for (int i = 0; i < 2 * cap_top->npoints; i++)
        {
            cap_top->tcoords[i] = 0.0f;
        }

        par_shapes_rotate(cap_top, -RF_PI / 2.0f, axis);
        par_shapes_translate(cap_top, 0, height, 0);

        // Generate an orientable disk shape (bottom cap)
        par_shapes_mesh* cap_bottom = par_shapes_create_disk(radius, slices, center, normal_minus_1);
        cap_bottom->tcoords = PAR_MALLOC(float, 2*cap_bottom->npoints);
        for (int i = 0; i < 2*cap_bottom->npoints; i++) cap_bottom->tcoords[i] = 0.95f;
        par_shapes_rotate(cap_bottom, RF_PI / 2.0f, axis);

        par_shapes_merge_and_free(cylinder, cap_top);
        par_shapes_merge_and_free(cylinder, cap_bottom);

        mesh.vertices  = (float*) RF_ALLOC(allocator, cylinder->ntriangles * 3 * 3 * sizeof(float));
        mesh.texcoords = (float*) RF_ALLOC(allocator, cylinder->ntriangles * 3 * 2 * sizeof(float));
        mesh.normals   = (float*) RF_ALLOC(allocator, cylinder->ntriangles * 3 * 3 * sizeof(float));

        mesh.vertex_count   = cylinder->ntriangles * 3;
        mesh.triangle_count = cylinder->ntriangles;

        for (int k = 0; k < mesh.vertex_count; k++)
        {
            mesh.vertices[k * 3    ] = cylinder->points[cylinder->triangles[k] * 3    ];
            mesh.vertices[k * 3 + 1] = cylinder->points[cylinder->triangles[k] * 3 + 1];
            mesh.vertices[k * 3 + 2] = cylinder->points[cylinder->triangles[k] * 3 + 2];

            mesh.normals[k * 3    ] = cylinder->normals[cylinder->triangles[k] * 3    ];
            mesh.normals[k * 3 + 1] = cylinder->normals[cylinder->triangles[k] * 3 + 1];
            mesh.normals[k * 3 + 2] = cylinder->normals[cylinder->triangles[k] * 3 + 2];

            mesh.texcoords[k * 2    ] = cylinder->tcoords[cylinder->triangles[k] * 2    ];
            mesh.texcoords[k * 2 + 1] = cylinder->tcoords[cylinder->triangles[k] * 2 + 1];
        }

        par_shapes_free_mesh(cylinder);
    }
    RF_SET_PARSHAPES_ALLOCATOR(NULL);

    // Upload vertex data to GPU (static mesh)
    rf_gfx_load_mesh(&mesh, false);

    return mesh;
}

// Generate torus mesh
RF_API rf_mesh rf_gen_mesh_torus(float radius, float size, int rad_seg, int sides, rf_allocator allocator, rf_allocator temp_allocator)
{
    rf_mesh mesh = { 0 };
    mesh.allocator = allocator;
    mesh.vbo_id = (unsigned int*) RF_ALLOC(allocator, RF_MAX_MESH_VBO * sizeof(unsigned int));
    memset(mesh.vbo_id, 0, RF_MAX_MESH_VBO * sizeof(unsigned int));

    if (radius > 1.0f)      radius = 1.0f;
    else if (radius < 0.1f) radius = 0.1f;

    RF_SET_PARSHAPES_ALLOCATOR(&temp_allocator);
    {
        // Create a donut that sits on the Z=0 plane with the specified inner radius
        // The outer radius can be controlled with par_shapes_scale
        par_shapes_mesh* torus = par_shapes_create_torus(rad_seg, sides, radius);
        par_shapes_scale(torus, size/2, size/2, size/2);

        mesh.vertices  = (float*) RF_ALLOC(allocator, torus->ntriangles * 3 * 3 * sizeof(float));
        mesh.texcoords = (float*) RF_ALLOC(allocator, torus->ntriangles * 3 * 2 * sizeof(float));
        mesh.normals   = (float*) RF_ALLOC(allocator, torus->ntriangles * 3 * 3 * sizeof(float));

        mesh.vertex_count   = torus->ntriangles * 3;
        mesh.triangle_count = torus->ntriangles;

        for (int k = 0; k < mesh.vertex_count; k++)
        {
            mesh.vertices[k * 3    ] = torus->points[torus->triangles[k] * 3    ];
            mesh.vertices[k * 3 + 1] = torus->points[torus->triangles[k] * 3 + 1];
            mesh.vertices[k * 3 + 2] = torus->points[torus->triangles[k] * 3 + 2];

            mesh.normals[k * 3    ] = torus->normals[torus->triangles[k] * 3    ];
            mesh.normals[k * 3 + 1] = torus->normals[torus->triangles[k] * 3 + 1];
            mesh.normals[k * 3 + 2] = torus->normals[torus->triangles[k] * 3 + 2];

            mesh.texcoords[k * 2    ] = torus->tcoords[torus->triangles[k] * 2    ];
            mesh.texcoords[k * 2 + 1] = torus->tcoords[torus->triangles[k] * 2 + 1];
        }

        par_shapes_free_mesh(torus);
    }
    RF_SET_PARSHAPES_ALLOCATOR(NULL);

    // Upload vertex data to GPU (static mesh)
    rf_gfx_load_mesh(&mesh, false);

    return mesh;
}

// Generate trefoil knot mesh
RF_API rf_mesh rf_gen_mesh_knot(float radius, float size, int rad_seg, int sides, rf_allocator allocator, rf_allocator temp_allocator)
{
    rf_mesh mesh = { 0 };
    mesh.allocator = allocator;
    mesh.vbo_id = (unsigned int*) RF_ALLOC(allocator, RF_MAX_MESH_VBO * sizeof(unsigned int));
    memset(mesh.vbo_id, 0, RF_MAX_MESH_VBO * sizeof(unsigned int));

    if (radius > 3.0f)      radius = 3.0f;
    else if (radius < 0.5f) radius = 0.5f;

    RF_SET_PARSHAPES_ALLOCATOR(&temp_allocator);
    {
        par_shapes_mesh* knot = par_shapes_create_trefoil_knot(rad_seg, sides, radius);
        par_shapes_scale(knot, size, size, size);

        mesh.vertices  = (float*) RF_ALLOC(allocator, knot->ntriangles * 3 * 3 * sizeof(float));
        mesh.texcoords = (float*) RF_ALLOC(allocator, knot->ntriangles * 3 * 2 * sizeof(float));
        mesh.normals   = (float*) RF_ALLOC(allocator, knot->ntriangles * 3 * 3 * sizeof(float));

        mesh.vertex_count   = knot->ntriangles * 3;
        mesh.triangle_count = knot->ntriangles;

        for (int k = 0; k < mesh.vertex_count; k++)
        {
            mesh.vertices[k * 3    ] = knot->points[knot->triangles[k] * 3    ];
            mesh.vertices[k * 3 + 1] = knot->points[knot->triangles[k] * 3 + 1];
            mesh.vertices[k * 3 + 2] = knot->points[knot->triangles[k] * 3 + 2];

            mesh.normals[k * 3    ] = knot->normals[knot->triangles[k] * 3    ];
            mesh.normals[k * 3 + 1] = knot->normals[knot->triangles[k] * 3 + 1];
            mesh.normals[k * 3 + 2] = knot->normals[knot->triangles[k] * 3 + 2];

            mesh.texcoords[k * 2    ] = knot->tcoords[knot->triangles[k] * 2    ];
            mesh.texcoords[k * 2 + 1] = knot->tcoords[knot->triangles[k] * 2 + 1];
        }

        par_shapes_free_mesh(knot);
    }
    RF_SET_PARSHAPES_ALLOCATOR(NULL);

    // Upload vertex data to GPU (static mesh)
    rf_gfx_load_mesh(&mesh, false);

    return mesh;
}

// Generate a mesh from heightmap
// NOTE: Vertex data is uploaded to GPU
RF_API rf_mesh rf_gen_mesh_heightmap(rf_image heightmap, rf_vec3 size, rf_allocator allocator, rf_allocator temp_allocator)
{
#define RF_GRAY_VALUE(c) ((c.r+c.g+c.b)/3)

    rf_mesh mesh = { 0 };
    mesh.allocator = allocator;
    mesh.vbo_id = (unsigned int*) RF_ALLOC(allocator, RF_MAX_MESH_VBO * sizeof(unsigned int));
    memset(mesh.vbo_id, 0, RF_MAX_MESH_VBO * sizeof(unsigned int));

    int map_x = heightmap.width;
    int map_z = heightmap.height;

    rf_color* pixels = rf_get_image_pixel_data(heightmap, temp_allocator);

    // NOTE: One vertex per pixel
    mesh.triangle_count = (map_x - 1) * (map_z - 1) * 2; // One quad every four pixels

    mesh.vertex_count = mesh.triangle_count * 3;

    mesh.vertices  = (float*) RF_ALLOC(allocator, mesh.vertex_count * 3 * sizeof(float));
    mesh.normals   = (float*) RF_ALLOC(allocator, mesh.vertex_count * 3 * sizeof(float));
    mesh.texcoords = (float*) RF_ALLOC(allocator, mesh.vertex_count * 2 * sizeof(float));
    mesh.colors    = NULL;

    int vertex_pos_counter      = 0; // Used to count vertices float by float
    int vertex_texcoord_counter = 0; // Used to count texcoords float by float
    int n_counter               = 0; // Used to count normals float by float
    int tris_counter            = 0;

    rf_vec3 scale_factor = { size.x / map_x, size.y / 255.0f, size.z / map_z };

    for (int z = 0; z < map_z-1; z++)
    {
        for (int x = 0; x < map_x-1; x++)
        {
            // Fill vertices array with data
            //----------------------------------------------------------

            // one triangle - 3 vertex
            mesh.vertices[vertex_pos_counter    ] = (float) x * scale_factor.x;
            mesh.vertices[vertex_pos_counter + 1] = (float) RF_GRAY_VALUE(pixels[x + z * map_x]) * scale_factor.y;
            mesh.vertices[vertex_pos_counter + 2] = (float) z * scale_factor.z;

            mesh.vertices[vertex_pos_counter + 3] = (float) x * scale_factor.x;
            mesh.vertices[vertex_pos_counter + 4] = (float) RF_GRAY_VALUE(pixels[x + (z + 1) * map_x]) * scale_factor.y;
            mesh.vertices[vertex_pos_counter + 5] = (float) (z + 1) * scale_factor.z;

            mesh.vertices[vertex_pos_counter + 6] = (float)(x + 1) * scale_factor.x;
            mesh.vertices[vertex_pos_counter + 7] = (float)RF_GRAY_VALUE(pixels[(x + 1) + z * map_x]) * scale_factor.y;
            mesh.vertices[vertex_pos_counter + 8] = (float)z * scale_factor.z;

            // another triangle - 3 vertex
            mesh.vertices[vertex_pos_counter + 9 ] = mesh.vertices[vertex_pos_counter + 6];
            mesh.vertices[vertex_pos_counter + 10] = mesh.vertices[vertex_pos_counter + 7];
            mesh.vertices[vertex_pos_counter + 11] = mesh.vertices[vertex_pos_counter + 8];

            mesh.vertices[vertex_pos_counter + 12] = mesh.vertices[vertex_pos_counter + 3];
            mesh.vertices[vertex_pos_counter + 13] = mesh.vertices[vertex_pos_counter + 4];
            mesh.vertices[vertex_pos_counter + 14] = mesh.vertices[vertex_pos_counter + 5];

            mesh.vertices[vertex_pos_counter + 15] = (float)(x + 1) * scale_factor.x;
            mesh.vertices[vertex_pos_counter + 16] = (float)RF_GRAY_VALUE(pixels[(x + 1) + (z + 1) * map_x]) * scale_factor.y;
            mesh.vertices[vertex_pos_counter + 17] = (float)(z + 1) * scale_factor.z;
            vertex_pos_counter += 18; // 6 vertex, 18 floats

            // Fill texcoords array with data
            //--------------------------------------------------------------
            mesh.texcoords[vertex_texcoord_counter    ] = (float)x / (map_x - 1);
            mesh.texcoords[vertex_texcoord_counter + 1] = (float)z / (map_z - 1);

            mesh.texcoords[vertex_texcoord_counter + 2] = (float)x / (map_x - 1);
            mesh.texcoords[vertex_texcoord_counter + 3] = (float)(z + 1) / (map_z - 1);

            mesh.texcoords[vertex_texcoord_counter + 4] = (float)(x + 1) / (map_x - 1);
            mesh.texcoords[vertex_texcoord_counter + 5] = (float)z / (map_z - 1);

            mesh.texcoords[vertex_texcoord_counter + 6] = mesh.texcoords[vertex_texcoord_counter + 4];
            mesh.texcoords[vertex_texcoord_counter + 7] = mesh.texcoords[vertex_texcoord_counter + 5];

            mesh.texcoords[vertex_texcoord_counter + 8] = mesh.texcoords[vertex_texcoord_counter + 2];
            mesh.texcoords[vertex_texcoord_counter + 9] = mesh.texcoords[vertex_texcoord_counter + 3];

            mesh.texcoords[vertex_texcoord_counter + 10] = (float)(x + 1) / (map_x - 1);
            mesh.texcoords[vertex_texcoord_counter + 11] = (float)(z + 1) / (map_z - 1);

            vertex_texcoord_counter += 12; // 6 texcoords, 12 floats

            // Fill normals array with data
            //--------------------------------------------------------------
            for (int i = 0; i < 18; i += 3)
            {
                mesh.normals[n_counter + i    ] = 0.0f;
                mesh.normals[n_counter + i + 1] = 1.0f;
                mesh.normals[n_counter + i + 2] = 0.0f;
            }

            // TODO: Calculate normals in an efficient way

            n_counter    += 18; // 6 vertex, 18 floats
            tris_counter += 2;
        }
    }

    RF_FREE(temp_allocator, pixels);

    // Upload vertex data to GPU (static mesh)
    rf_gfx_load_mesh(&mesh, false);

    return mesh;
}

// Generate a cubes mesh from pixel data
// NOTE: Vertex data is uploaded to GPU
RF_API rf_mesh rf_gen_mesh_cubicmap(rf_image cubicmap, rf_vec3 cube_size, rf_allocator allocator, rf_allocator temp_allocator)
{
    rf_mesh mesh = { 0 };
    mesh.allocator = allocator;
    mesh.vbo_id = (unsigned int*) RF_ALLOC(allocator, RF_MAX_MESH_VBO * sizeof(unsigned int));
    memset(mesh.vbo_id, 0, RF_MAX_MESH_VBO * sizeof(unsigned int));

    rf_color* cubicmap_pixels = rf_get_image_pixel_data(cubicmap, temp_allocator);

    int map_width = cubicmap.width;
    int map_height = cubicmap.height;

    // NOTE: Max possible number of triangles numCubes*(12 triangles by cube)
    int maxTriangles = cubicmap.width*cubicmap.height*12;

    int vertex_pos_counter = 0; // Used to count vertices
    int vertex_texcoord_counter = 0; // Used to count texcoords
    int n_counter = 0; // Used to count normals

    float w = cube_size.x;
    float h = cube_size.z;
    float h2 = cube_size.y;

    rf_vec3* map_vertices  = (rf_vec3*) RF_ALLOC(temp_allocator, maxTriangles * 3 * sizeof(rf_vec3));
    rf_vec2 *map_texcoords = (rf_vec2*) RF_ALLOC(temp_allocator, maxTriangles * 3 * sizeof(rf_vec2));
    rf_vec3* map_normals   = (rf_vec3*) RF_ALLOC(temp_allocator, maxTriangles * 3 * sizeof(rf_vec3));

    // Define the 6 normals of the cube, we will combine them accordingly later...
    rf_vec3 n1 = {  1.0f,  0.0f,  0.0f };
    rf_vec3 n2 = { -1.0f,  0.0f,  0.0f };
    rf_vec3 n3 = {  0.0f,  1.0f,  0.0f };
    rf_vec3 n4 = {  0.0f, -1.0f,  0.0f };
    rf_vec3 n5 = {  0.0f,  0.0f,  1.0f };
    rf_vec3 n6 = {  0.0f,  0.0f, -1.0f };

    // NOTE: We use texture rectangles to define different textures for top-bottom-front-back-right-left (6)
    typedef struct rf_recf rf_recf;
    struct rf_recf
    {
        float x;
        float y;
        float width;
        float height;
    };

    rf_recf right_tex_uv  = { 0.0f, 0.0f, 0.5f, 0.5f };
    rf_recf left_tex_uv   = { 0.5f, 0.0f, 0.5f, 0.5f };
    rf_recf front_tex_uv  = { 0.0f, 0.0f, 0.5f, 0.5f };
    rf_recf back_tex_uv   = { 0.5f, 0.0f, 0.5f, 0.5f };
    rf_recf top_tex_uv    = { 0.0f, 0.5f, 0.5f, 0.5f };
    rf_recf bottom_tex_uv = { 0.5f, 0.5f, 0.5f, 0.5f };

    for (int z = 0; z < map_height; ++z)
    {
        for (int x = 0; x < map_width; ++x)
        {
            // Define the 8 vertex of the cube, we will combine them accordingly later...
            rf_vec3 v1 = {w * (x - 0.5f), h2, h * (z - 0.5f) };
            rf_vec3 v2 = {w * (x - 0.5f), h2, h * (z + 0.5f) };
            rf_vec3 v3 = {w * (x + 0.5f), h2, h * (z + 0.5f) };
            rf_vec3 v4 = {w * (x + 0.5f), h2, h * (z - 0.5f) };
            rf_vec3 v5 = {w * (x + 0.5f), 0, h * (z - 0.5f) };
            rf_vec3 v6 = {w * (x - 0.5f), 0, h * (z - 0.5f) };
            rf_vec3 v7 = {w * (x - 0.5f), 0, h * (z + 0.5f) };
            rf_vec3 v8 = {w * (x + 0.5f), 0, h * (z + 0.5f) };

            // We check pixel color to be RF_WHITE, we will full cubes
            if ((cubicmap_pixels[z*cubicmap.width + x].r == 255) &&
                (cubicmap_pixels[z*cubicmap.width + x].g == 255) &&
                (cubicmap_pixels[z*cubicmap.width + x].b == 255))
            {
                // Define triangles (Checking Collateral Cubes!)
                //----------------------------------------------

                // Define top triangles (2 tris, 6 vertex --> v1-v2-v3, v1-v3-v4)
                map_vertices[vertex_pos_counter] = v1;
                map_vertices[vertex_pos_counter + 1] = v2;
                map_vertices[vertex_pos_counter + 2] = v3;
                map_vertices[vertex_pos_counter + 3] = v1;
                map_vertices[vertex_pos_counter + 4] = v3;
                map_vertices[vertex_pos_counter + 5] = v4;
                vertex_pos_counter += 6;

                map_normals[n_counter] = n3;
                map_normals[n_counter + 1] = n3;
                map_normals[n_counter + 2] = n3;
                map_normals[n_counter + 3] = n3;
                map_normals[n_counter + 4] = n3;
                map_normals[n_counter + 5] = n3;
                n_counter += 6;

                map_texcoords[vertex_texcoord_counter] = (rf_vec2){top_tex_uv.x, top_tex_uv.y };
                map_texcoords[vertex_texcoord_counter + 1] = (rf_vec2){top_tex_uv.x, top_tex_uv.y + top_tex_uv.height };
                map_texcoords[vertex_texcoord_counter + 2] = (rf_vec2){top_tex_uv.x + top_tex_uv.width, top_tex_uv.y + top_tex_uv.height };
                map_texcoords[vertex_texcoord_counter + 3] = (rf_vec2){top_tex_uv.x, top_tex_uv.y };
                map_texcoords[vertex_texcoord_counter + 4] = (rf_vec2){top_tex_uv.x + top_tex_uv.width, top_tex_uv.y + top_tex_uv.height };
                map_texcoords[vertex_texcoord_counter + 5] = (rf_vec2){top_tex_uv.x + top_tex_uv.width, top_tex_uv.y };
                vertex_texcoord_counter += 6;

                // Define bottom triangles (2 tris, 6 vertex --> v6-v8-v7, v6-v5-v8)
                map_vertices[vertex_pos_counter] = v6;
                map_vertices[vertex_pos_counter + 1] = v8;
                map_vertices[vertex_pos_counter + 2] = v7;
                map_vertices[vertex_pos_counter + 3] = v6;
                map_vertices[vertex_pos_counter + 4] = v5;
                map_vertices[vertex_pos_counter + 5] = v8;
                vertex_pos_counter += 6;

                map_normals[n_counter] = n4;
                map_normals[n_counter + 1] = n4;
                map_normals[n_counter + 2] = n4;
                map_normals[n_counter + 3] = n4;
                map_normals[n_counter + 4] = n4;
                map_normals[n_counter + 5] = n4;
                n_counter += 6;

                map_texcoords[vertex_texcoord_counter] = (rf_vec2){bottom_tex_uv.x + bottom_tex_uv.width, bottom_tex_uv.y };
                map_texcoords[vertex_texcoord_counter + 1] = (rf_vec2){bottom_tex_uv.x, bottom_tex_uv.y + bottom_tex_uv.height };
                map_texcoords[vertex_texcoord_counter + 2] = (rf_vec2){bottom_tex_uv.x + bottom_tex_uv.width, bottom_tex_uv.y + bottom_tex_uv.height };
                map_texcoords[vertex_texcoord_counter + 3] = (rf_vec2){bottom_tex_uv.x + bottom_tex_uv.width, bottom_tex_uv.y };
                map_texcoords[vertex_texcoord_counter + 4] = (rf_vec2){bottom_tex_uv.x, bottom_tex_uv.y };
                map_texcoords[vertex_texcoord_counter + 5] = (rf_vec2){bottom_tex_uv.x, bottom_tex_uv.y + bottom_tex_uv.height };
                vertex_texcoord_counter += 6;

                if (((z < cubicmap.height - 1) &&
                     (cubicmap_pixels[(z + 1)*cubicmap.width + x].r == 0) &&
                     (cubicmap_pixels[(z + 1)*cubicmap.width + x].g == 0) &&
                     (cubicmap_pixels[(z + 1)*cubicmap.width + x].b == 0)) || (z == cubicmap.height - 1))
                {
                    // Define front triangles (2 tris, 6 vertex) --> v2 v7 v3, v3 v7 v8
                    // NOTE: Collateral occluded faces are not generated
                    map_vertices[vertex_pos_counter] = v2;
                    map_vertices[vertex_pos_counter + 1] = v7;
                    map_vertices[vertex_pos_counter + 2] = v3;
                    map_vertices[vertex_pos_counter + 3] = v3;
                    map_vertices[vertex_pos_counter + 4] = v7;
                    map_vertices[vertex_pos_counter + 5] = v8;
                    vertex_pos_counter += 6;

                    map_normals[n_counter] = n6;
                    map_normals[n_counter + 1] = n6;
                    map_normals[n_counter + 2] = n6;
                    map_normals[n_counter + 3] = n6;
                    map_normals[n_counter + 4] = n6;
                    map_normals[n_counter + 5] = n6;
                    n_counter += 6;

                    map_texcoords[vertex_texcoord_counter] = (rf_vec2){front_tex_uv.x, front_tex_uv.y };
                    map_texcoords[vertex_texcoord_counter + 1] = (rf_vec2){front_tex_uv.x, front_tex_uv.y + front_tex_uv.height };
                    map_texcoords[vertex_texcoord_counter + 2] = (rf_vec2){front_tex_uv.x + front_tex_uv.width, front_tex_uv.y };
                    map_texcoords[vertex_texcoord_counter + 3] = (rf_vec2){front_tex_uv.x + front_tex_uv.width, front_tex_uv.y };
                    map_texcoords[vertex_texcoord_counter + 4] = (rf_vec2){front_tex_uv.x, front_tex_uv.y + front_tex_uv.height };
                    map_texcoords[vertex_texcoord_counter + 5] = (rf_vec2){front_tex_uv.x + front_tex_uv.width, front_tex_uv.y + front_tex_uv.height };
                    vertex_texcoord_counter += 6;
                }

                if (((z > 0) &&
                     (cubicmap_pixels[(z - 1)*cubicmap.width + x].r == 0) &&
                     (cubicmap_pixels[(z - 1)*cubicmap.width + x].g == 0) &&
                     (cubicmap_pixels[(z - 1)*cubicmap.width + x].b == 0)) || (z == 0))
                {
                    // Define back triangles (2 tris, 6 vertex) --> v1 v5 v6, v1 v4 v5
                    // NOTE: Collateral occluded faces are not generated
                    map_vertices[vertex_pos_counter] = v1;
                    map_vertices[vertex_pos_counter + 1] = v5;
                    map_vertices[vertex_pos_counter + 2] = v6;
                    map_vertices[vertex_pos_counter + 3] = v1;
                    map_vertices[vertex_pos_counter + 4] = v4;
                    map_vertices[vertex_pos_counter + 5] = v5;
                    vertex_pos_counter += 6;

                    map_normals[n_counter] = n5;
                    map_normals[n_counter + 1] = n5;
                    map_normals[n_counter + 2] = n5;
                    map_normals[n_counter + 3] = n5;
                    map_normals[n_counter + 4] = n5;
                    map_normals[n_counter + 5] = n5;
                    n_counter += 6;

                    map_texcoords[vertex_texcoord_counter] = (rf_vec2){back_tex_uv.x + back_tex_uv.width, back_tex_uv.y };
                    map_texcoords[vertex_texcoord_counter + 1] = (rf_vec2){back_tex_uv.x, back_tex_uv.y + back_tex_uv.height };
                    map_texcoords[vertex_texcoord_counter + 2] = (rf_vec2){back_tex_uv.x + back_tex_uv.width, back_tex_uv.y + back_tex_uv.height };
                    map_texcoords[vertex_texcoord_counter + 3] = (rf_vec2){back_tex_uv.x + back_tex_uv.width, back_tex_uv.y };
                    map_texcoords[vertex_texcoord_counter + 4] = (rf_vec2){back_tex_uv.x, back_tex_uv.y };
                    map_texcoords[vertex_texcoord_counter + 5] = (rf_vec2){back_tex_uv.x, back_tex_uv.y + back_tex_uv.height };
                    vertex_texcoord_counter += 6;
                }

                if (((x < cubicmap.width - 1) &&
                     (cubicmap_pixels[z*cubicmap.width + (x + 1)].r == 0) &&
                     (cubicmap_pixels[z*cubicmap.width + (x + 1)].g == 0) &&
                     (cubicmap_pixels[z*cubicmap.width + (x + 1)].b == 0)) || (x == cubicmap.width - 1))
                {
                    // Define right triangles (2 tris, 6 vertex) --> v3 v8 v4, v4 v8 v5
                    // NOTE: Collateral occluded faces are not generated
                    map_vertices[vertex_pos_counter] = v3;
                    map_vertices[vertex_pos_counter + 1] = v8;
                    map_vertices[vertex_pos_counter + 2] = v4;
                    map_vertices[vertex_pos_counter + 3] = v4;
                    map_vertices[vertex_pos_counter + 4] = v8;
                    map_vertices[vertex_pos_counter + 5] = v5;
                    vertex_pos_counter += 6;

                    map_normals[n_counter] = n1;
                    map_normals[n_counter + 1] = n1;
                    map_normals[n_counter + 2] = n1;
                    map_normals[n_counter + 3] = n1;
                    map_normals[n_counter + 4] = n1;
                    map_normals[n_counter + 5] = n1;
                    n_counter += 6;

                    map_texcoords[vertex_texcoord_counter] = (rf_vec2){right_tex_uv.x, right_tex_uv.y };
                    map_texcoords[vertex_texcoord_counter + 1] = (rf_vec2){right_tex_uv.x, right_tex_uv.y + right_tex_uv.height };
                    map_texcoords[vertex_texcoord_counter + 2] = (rf_vec2){right_tex_uv.x + right_tex_uv.width, right_tex_uv.y };
                    map_texcoords[vertex_texcoord_counter + 3] = (rf_vec2){right_tex_uv.x + right_tex_uv.width, right_tex_uv.y };
                    map_texcoords[vertex_texcoord_counter + 4] = (rf_vec2){right_tex_uv.x, right_tex_uv.y + right_tex_uv.height };
                    map_texcoords[vertex_texcoord_counter + 5] = (rf_vec2){right_tex_uv.x + right_tex_uv.width, right_tex_uv.y + right_tex_uv.height };
                    vertex_texcoord_counter += 6;
                }

                if (((x > 0) &&
                     (cubicmap_pixels[z*cubicmap.width + (x - 1)].r == 0) &&
                     (cubicmap_pixels[z*cubicmap.width + (x - 1)].g == 0) &&
                     (cubicmap_pixels[z*cubicmap.width + (x - 1)].b == 0)) || (x == 0))
                {
                    // Define left triangles (2 tris, 6 vertex) --> v1 v7 v2, v1 v6 v7
                    // NOTE: Collateral occluded faces are not generated
                    map_vertices[vertex_pos_counter] = v1;
                    map_vertices[vertex_pos_counter + 1] = v7;
                    map_vertices[vertex_pos_counter + 2] = v2;
                    map_vertices[vertex_pos_counter + 3] = v1;
                    map_vertices[vertex_pos_counter + 4] = v6;
                    map_vertices[vertex_pos_counter + 5] = v7;
                    vertex_pos_counter += 6;

                    map_normals[n_counter] = n2;
                    map_normals[n_counter + 1] = n2;
                    map_normals[n_counter + 2] = n2;
                    map_normals[n_counter + 3] = n2;
                    map_normals[n_counter + 4] = n2;
                    map_normals[n_counter + 5] = n2;
                    n_counter += 6;

                    map_texcoords[vertex_texcoord_counter] = (rf_vec2){left_tex_uv.x, left_tex_uv.y };
                    map_texcoords[vertex_texcoord_counter + 1] = (rf_vec2){left_tex_uv.x + left_tex_uv.width, left_tex_uv.y + left_tex_uv.height };
                    map_texcoords[vertex_texcoord_counter + 2] = (rf_vec2){left_tex_uv.x + left_tex_uv.width, left_tex_uv.y };
                    map_texcoords[vertex_texcoord_counter + 3] = (rf_vec2){left_tex_uv.x, left_tex_uv.y };
                    map_texcoords[vertex_texcoord_counter + 4] = (rf_vec2){left_tex_uv.x, left_tex_uv.y + left_tex_uv.height };
                    map_texcoords[vertex_texcoord_counter + 5] = (rf_vec2){left_tex_uv.x + left_tex_uv.width, left_tex_uv.y + left_tex_uv.height };
                    vertex_texcoord_counter += 6;
                }
            }
                // We check pixel color to be RF_BLACK, we will only draw floor and roof
            else if ((cubicmap_pixels[z*cubicmap.width + x].r == 0) &&
                     (cubicmap_pixels[z*cubicmap.width + x].g == 0) &&
                     (cubicmap_pixels[z*cubicmap.width + x].b == 0))
            {
                // Define top triangles (2 tris, 6 vertex --> v1-v2-v3, v1-v3-v4)
                map_vertices[vertex_pos_counter] = v1;
                map_vertices[vertex_pos_counter + 1] = v3;
                map_vertices[vertex_pos_counter + 2] = v2;
                map_vertices[vertex_pos_counter + 3] = v1;
                map_vertices[vertex_pos_counter + 4] = v4;
                map_vertices[vertex_pos_counter + 5] = v3;
                vertex_pos_counter += 6;

                map_normals[n_counter] = n4;
                map_normals[n_counter + 1] = n4;
                map_normals[n_counter + 2] = n4;
                map_normals[n_counter + 3] = n4;
                map_normals[n_counter + 4] = n4;
                map_normals[n_counter + 5] = n4;
                n_counter += 6;

                map_texcoords[vertex_texcoord_counter] = (rf_vec2){top_tex_uv.x, top_tex_uv.y };
                map_texcoords[vertex_texcoord_counter + 1] = (rf_vec2){top_tex_uv.x + top_tex_uv.width, top_tex_uv.y + top_tex_uv.height };
                map_texcoords[vertex_texcoord_counter + 2] = (rf_vec2){top_tex_uv.x, top_tex_uv.y + top_tex_uv.height };
                map_texcoords[vertex_texcoord_counter + 3] = (rf_vec2){top_tex_uv.x, top_tex_uv.y };
                map_texcoords[vertex_texcoord_counter + 4] = (rf_vec2){top_tex_uv.x + top_tex_uv.width, top_tex_uv.y };
                map_texcoords[vertex_texcoord_counter + 5] = (rf_vec2){top_tex_uv.x + top_tex_uv.width, top_tex_uv.y + top_tex_uv.height };
                vertex_texcoord_counter += 6;

                // Define bottom triangles (2 tris, 6 vertex --> v6-v8-v7, v6-v5-v8)
                map_vertices[vertex_pos_counter] = v6;
                map_vertices[vertex_pos_counter + 1] = v7;
                map_vertices[vertex_pos_counter + 2] = v8;
                map_vertices[vertex_pos_counter + 3] = v6;
                map_vertices[vertex_pos_counter + 4] = v8;
                map_vertices[vertex_pos_counter + 5] = v5;
                vertex_pos_counter += 6;

                map_normals[n_counter] = n3;
                map_normals[n_counter + 1] = n3;
                map_normals[n_counter + 2] = n3;
                map_normals[n_counter + 3] = n3;
                map_normals[n_counter + 4] = n3;
                map_normals[n_counter + 5] = n3;
                n_counter += 6;

                map_texcoords[vertex_texcoord_counter] = (rf_vec2){bottom_tex_uv.x + bottom_tex_uv.width, bottom_tex_uv.y };
                map_texcoords[vertex_texcoord_counter + 1] = (rf_vec2){bottom_tex_uv.x + bottom_tex_uv.width, bottom_tex_uv.y + bottom_tex_uv.height };
                map_texcoords[vertex_texcoord_counter + 2] = (rf_vec2){bottom_tex_uv.x, bottom_tex_uv.y + bottom_tex_uv.height };
                map_texcoords[vertex_texcoord_counter + 3] = (rf_vec2){bottom_tex_uv.x + bottom_tex_uv.width, bottom_tex_uv.y };
                map_texcoords[vertex_texcoord_counter + 4] = (rf_vec2){bottom_tex_uv.x, bottom_tex_uv.y + bottom_tex_uv.height };
                map_texcoords[vertex_texcoord_counter + 5] = (rf_vec2){bottom_tex_uv.x, bottom_tex_uv.y };
                vertex_texcoord_counter += 6;
            }
        }
    }

    // Move data from map_vertices temp arays to vertices float array
    mesh.vertex_count = vertex_pos_counter;
    mesh.triangle_count = vertex_pos_counter/3;

    mesh.vertices  = (float*) RF_ALLOC(allocator, mesh.vertex_count * 3 * sizeof(float));
    mesh.normals   = (float*) RF_ALLOC(allocator, mesh.vertex_count * 3 * sizeof(float));
    mesh.texcoords = (float*) RF_ALLOC(allocator, mesh.vertex_count * 2 * sizeof(float));
    mesh.colors = NULL;

    int f_counter = 0;

    // Move vertices data
    for (int i = 0; i < vertex_pos_counter; i++)
    {
        mesh.vertices[f_counter] = map_vertices[i].x;
        mesh.vertices[f_counter + 1] = map_vertices[i].y;
        mesh.vertices[f_counter + 2] = map_vertices[i].z;
        f_counter += 3;
    }

    f_counter = 0;

    // Move normals data
    for (int i = 0; i < n_counter; i++)
    {
        mesh.normals[f_counter] = map_normals[i].x;
        mesh.normals[f_counter + 1] = map_normals[i].y;
        mesh.normals[f_counter + 2] = map_normals[i].z;
        f_counter += 3;
    }

    f_counter = 0;

    // Move texcoords data
    for (int i = 0; i < vertex_texcoord_counter; i++)
    {
        mesh.texcoords[f_counter] = map_texcoords[i].x;
        mesh.texcoords[f_counter + 1] = map_texcoords[i].y;
        f_counter += 2;
    }

    RF_FREE(temp_allocator, map_vertices);
    RF_FREE(temp_allocator, map_normals);
    RF_FREE(temp_allocator, map_texcoords);

    RF_FREE(temp_allocator, cubicmap_pixels); // Free image pixel data

    // Upload vertex data to GPU (static mesh)
    rf_gfx_load_mesh(&mesh, false);

    return mesh;
}
//endregion
//endregion