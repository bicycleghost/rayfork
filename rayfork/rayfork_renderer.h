/**********************************************************************************************
*
*   rayfork_renderer - A single header library 2D/3D rendering
*   rayfork - a set of header only libraries that are forked from the amazing Raylib (https://www.raylib.com/)
*
*   FEATURES:
*       - Cross-platform with no default platform layer
*       - 2D shapes, textures and ninepatch rendering
*       - 3D rendering
*       - Text rendering
*       - Async asset loading
*
*   DEPENDENCIES:
*       - stb_image
*       - stb_image_write
*       - stb_image_resize
*       - stb_perlin
*       - stb_rect_pack
*       - stb_truetype
*       - cgltf
*       - par_shapes
*       - tinyobj_loader_c
*
***********************************************************************************************/

//region interface

#ifndef RF_RENDERER_H
#define RF_RENDERER_H

#include <stdbool.h>
#include "rayfork_math.h"

//region basic defines
#ifndef RF_API
#define RF_API extern
#endif

#ifndef RF_INTERNAL
#define RF_INTERNAL static
#endif

// NOTE: MSVC C++ compiler does not support compound literals (C99 feature)
// Plain structures in C++ (without constructors) can be initialized from { } initializers.
#ifdef __cplusplus
#define RF_CLITERAL(type) type
#else
#define RF_CLITERAL(type) (type)
#endif

#define rf_max_text_buffer_length 1024 // Size of internal RF_INTERNAL buffers used on  some functions:
#define rf_max_text_unicode_chars 512 // Maximum number of unicode codepoints
#define rf_textsplit_max_text_buffer_length 1024 // Size of RF_INTERNAL buffer: _rf_text_split()
#define rf_textsplit_max_substrings_count 128 // Size of RF_INTERNAL pointers array: _rf_text_split()

// Some Basic Colors
// NOTE: Custom raylib color palette for amazing visuals on rf_white background
#define rf_lightgray RF_CLITERAL(rf_color){ 200, 200, 200, 255 } // Light Gray
#define rf_gray RF_CLITERAL(rf_color){ 130, 130, 130, 255 } // Gray
#define rf_darkgray RF_CLITERAL(rf_color){ 80, 80, 80, 255 } // Dark Gray
#define rf_yellow RF_CLITERAL(rf_color){ 253, 249, 0, 255 } // Yellow
#define rf_gold RF_CLITERAL(rf_color){ 255, 203, 0, 255 } // Gold
#define rf_orange RF_CLITERAL(rf_color){ 255, 161, 0, 255 } // Orange
#define rf_pink RF_CLITERAL(rf_color){ 255, 109, 194, 255 } // Pink
#define rf_red RF_CLITERAL(rf_color){ 230, 41, 55, 255 } // Red
#define rf_maroon RF_CLITERAL(rf_color){ 190, 33, 55, 255 } // Maroon
#define rf_green RF_CLITERAL(rf_color){ 0, 228, 48, 255 } // Green
#define rf_lime RF_CLITERAL(rf_color){ 0, 158, 47, 255 } // Lime
#define rf_darkgreen RF_CLITERAL(rf_color){ 0, 117, 44, 255 } // Dark Green
#define rf_skyblue RF_CLITERAL(rf_color){ 102, 191, 255, 255 } // Sky Blue
#define rf_blue RF_CLITERAL(rf_color){ 0, 121, 241, 255 } // Blue
#define rf_darkblue RF_CLITERAL(rf_color){ 0, 82, 172, 255 } // Dark Blue
#define rf_purple RF_CLITERAL(rf_color){ 200, 122, 255, 255 } // Purple
#define rf_violet RF_CLITERAL(rf_color){ 135, 60, 190, 255 } // Violet
#define rf_darkpurple RF_CLITERAL(rf_color){ 112, 31, 126, 255 } // Dark Purple
#define rf_beige RF_CLITERAL(rf_color){ 211, 176, 131, 255 } // Beige
#define rf_brown RF_CLITERAL(rf_color){ 127, 106, 79, 255 } // Brown
#define rf_darkbrown RF_CLITERAL(rf_color){ 76, 63, 47, 255 } // Dark Brown

#define rf_white RF_CLITERAL(rf_color){ 255, 255, 255, 255 } // White
#define rf_black RF_CLITERAL(rf_color){ 0, 0, 0, 255 } // Black
#define rf_blank RF_CLITERAL(rf_color){ 0, 0, 0, 0 } // Blank (Transparent)
#define rf_magenta RF_CLITERAL(rf_color){ 255, 0, 255, 255 } // Magenta
#define rf_raywhite RF_CLITERAL(rf_color){ 245, 245, 245, 255 } // My own White (raylib logo)
//endregion

//region gl constants

// Security check in case multiple RF_GRAPHICS_API_OPENGL_* defined
// Security check in case no GRAPHICS_API_OPENGL_* defined
#if !defined(RF_GRAPHICS_API_OPENGL_11) && !defined(RF_GRAPHICS_API_OPENGL_21) && !defined(RF_GRAPHICS_API_OPENGL_33) && !defined(RF_GRAPHICS_API_OPENGL_ES2)
#define RF_GRAPHICS_API_OPENGL_33
#endif

// Security checks in case multiple GRAPHICS_API_OPENGL_* defined
#if defined(RF_GRAPHICS_API_OPENGL_11)
#undef RF_GRAPHICS_API_OPENGL_21
    #undef RF_GRAPHICS_API_OPENGL_33
    #undef RF_GRAPHICS_API_OPENGL_ES2
#endif

#if defined(RF_GRAPHICS_API_OPENGL_21)
    #undef RF_GRAPHICS_API_OPENGL_11
    #undef RF_GRAPHICS_API_OPENGL_33
    #undef RF_GRAPHICS_API_OPENGL_ES2
#endif

#if defined(RF_GRAPHICS_API_OPENGL_33)
#undef RF_GRAPHICS_API_OPENGL_11
#undef RF_GRAPHICS_API_OPENGL_21
#undef RF_GRAPHICS_API_OPENGL_ES2
#endif

#if defined(RF_GRAPHICS_API_OPENGL_ES2)
#undef RF_GRAPHICS_API_OPENGL_11
    #undef RF_GRAPHICS_API_OPENGL_21
    #undef RF_GRAPHICS_API_OPENGL_33
#endif

#if defined(RF_GRAPHICS_API_OPENGL_21)
#define RF_GRAPHICS_API_OPENGL_33
#endif

#if defined(RF_GRAPHICS_API_OPENGL_11) || defined(RF_GRAPHICS_API_OPENGL_33)
    // This is the maximum amount of elements (quads) per batch
    // NOTE: Be careful with text, every letter maps to a quad
    #ifndef rf_max_batch_elements
        #define rf_max_batch_elements 8192
    #endif
#elif defined(RF_GRAPHICS_API_OPENGL_ES2)
    // We reduce memory sizes for embedded systems (RPI and HTML5)
    // NOTE: On HTML5 (emscripten) this is allocated on heap, by default it's only 16MB!...just take care...
    #ifndef rf_max_batch_elements
        #define rf_max_batch_elements            2048
    #endif
#endif

#define rf_max_batch_buffering                  1      // Max number of buffers for batching (multi-buffering)
#define rf_max_matrix_stack_size               32      // Max size of rf_matrix _rf_global_context_ptr->gl_ctx.stack
#define rf_max_drawcall_registered            256      // Max _rf_global_context_ptr->gl_ctx.draws by state changes (mode, texture)

// rf_shader and material limits
#define rf_max_shader_locations                32      // Maximum number of predefined locations stored in shader struct
#define rf_max_material_maps                   12      // Maximum number of texture maps stored in shader struct

// @ToRemove @Note(lulu): Im pretty sure we can remove these since its the job of the user to include opengl
// rf_texture parameters (equivalent to OpenGL defines)
#define GL_TEXTURE_WRAP_S               0x2802      // GL_TEXTURE_WRAP_S
#define GL_TEXTURE_WRAP_T               0x2803      // GL_TEXTURE_WRAP_T
#define GL_TEXTURE_MAG_FILTER           0x2800      // GL_TEXTURE_MAG_FILTER
#define GL_TEXTURE_MIN_FILTER           0x2801      // GL_TEXTURE_MIN_FILTER
#define GL_TEXTURE_ANISOTROPIC_FILTER   0x3000      // Anisotropic filter (custom identifier)

#define GL_NEAREST                      0x2600      // GL_NEAREST
#define GL_LINEAR                       0x2601      // GL_LINEAR
#define GL_NEAREST_MIPMAP_NEAREST       0x2700      // GL_NEAREST_MIPMAP_NEAREST
#define GL_NEAREST_MIPMAP_LINEAR        0x2702      // GL_NEAREST_MIPMAP_LINEAR
#define GL_LINEAR_MIPMAP_NEAREST        0x2701      // GL_LINEAR_MIPMAP_NEAREST
#define GL_LINEAR_MIPMAP_LINEAR         0x2703      // GL_LINEAR_MIPMAP_LINEAR

#define GL_REPEAT                       0x2901      // GL_REPEAT
#define GL_CLAMP_TO_EDGE                0x812F      // GL_CLAMP_TO_EDGE
#define GL_MIRRORED_REPEAT              0x8370      // GL_MIRRORED_REPEAT
#define GL_MIRROR_CLAMP_EXT             0x8742      // GL_MIRROR_CLAMP_EXT

// rf_matrix modes (equivalent to OpenGL)
#define GL_MODELVIEW                 0x1700      // GL_MODELVIEW
#define GL_PROJECTION                0x1701      // GL_PROJECTION
#define GL_TEXTURE                   0x1702      // GL_TEXTURE

// Primitive assembly draw modes
#define GL_LINES                        0x0001      // GL_LINES
#define GL_TRIANGLES                    0x0004      // GL_TRIANGLES
#define GL_QUADS                        0x0007      // GL_QUADS

// Default vertex attribute names on shader to set location points
#define DEFAULT_ATTRIB_POSITION_NAME    "vertexPosition"    // shader-location = 0
#define DEFAULT_ATTRIB_TEXCOORD_NAME    "vertexTexCoord"    // shader-location = 1
#define DEFAULT_ATTRIB_NORMAL_NAME      "vertexNormal"      // shader-location = 2
#define DEFAULT_ATTRIB_COLOR_NAME       "vertexColor"       // shader-location = 3
#define DEFAULT_ATTRIB_TANGENT_NAME     "vertexTangent"     // shader-location = 4
#define DEFAULT_ATTRIB_TEXCOORD2_NAME   "vertexTexCoord2"   // shader-location = 5

//endregion

//region structs

typedef struct rf_input_state_for_update_camera rf_input_state_for_update_camera;
struct rf_input_state_for_update_camera
{
    rf_vector2 mouse_position;
    int mouse_wheel_move; //mouse wheel movement Y
    bool is_camera_pan_control_key_down; //MOUSE_MIDDLE_BUTTON
    bool is_camera_alt_control_key_down; //KEY_LEFT_ALT
    bool is_camera_smooth_zoom_control_key; //KEY_LEFT_CONTROL
    bool direction_keys[6]; //'W', 'S', 'D', 'A', 'E', 'Q'
};

//RGBA (32bit)
typedef struct rf_color rf_color;
struct rf_color
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

//rf_image type, bpp always RGBA (32bit)
//NOTE: Data stored in CPU memory (RAM)
typedef struct rf_image rf_image;
struct rf_image
{
    void* data;    //rf_image raw data
    int   width;   //rf_image base width
    int   height;  //rf_image base height
    int   mipmaps; //Mipmap levels, 1 by default
    int   format;  //Data format (rf_pixel_format type)
};

// rf_texture2d type
// NOTE: Data stored in GPU memory
typedef struct rf_texture2d rf_texture2d;
typedef struct rf_texture2d rf_texture;
typedef struct rf_texture2d rf_texture_cubemap;
struct rf_texture2d
{
    unsigned int id; //OpenGL texture id
    int width;       //rf_texture base width
    int height;      //rf_texture base height
    int mipmaps;     //Mipmap levels, 1 by default
    int format;      //Data format (rf_pixel_format type)
};

//rf_render_texture2d type, for texture rendering
typedef struct rf_render_texture2d rf_render_texture2d;
typedef struct rf_render_texture2d rf_render_texture;
struct rf_render_texture2d
{
    unsigned int id;   //OpenGL Framebuffer Object (FBO) id
    rf_texture2d texture; //rf_color buffer attachment texture
    rf_texture2d depth;   //Depth buffer attachment texture
    bool depth_texture; //Track if depth attachment is a texture or renderbuffer
};

//N-Patch layout info
typedef struct rf_npatch_info rf_npatch_info;
struct rf_npatch_info
{
    rf_rectangle source_rec; //Region in the texture
    int left;            //left border offset
    int top;             //top border offset
    int right;           //right border offset
    int bottom;          //bottom border offset
    int type;            //layout of the n-patch: 3x3, 1x3 or 3x1
};

//rf_font character info
typedef struct rf_char_info rf_char_info;
struct rf_char_info
{
    int value;    //Character value (Unicode)
    int offset_x;  //Character offset X when drawing
    int offset_y;  //Character offset Y when drawing
    int advance_x; //Character advance position X
    rf_image image;  //Character image data
};

// rf_font type, includes texture and charSet array data
typedef struct rf_font rf_font;
struct rf_font
{
    int base_size;      // Base size (default chars height)
    int chars_count;    // Number of characters
    rf_texture2d texture; // Characters texture atlas
    rf_rectangle* recs;   // Characters rectangles in texture
    rf_char_info* chars;   // Characters info data
};

typedef struct rf_load_font_async_result rf_load_font_async_result;
struct rf_load_font_async_result
{
    rf_font font;
    rf_image atlas;
};

// Camera type, defines a camera position/orientation in 3d space
typedef struct rf_camera3d rf_camera3d;
struct rf_camera3d
{
    rf_vector3 position; // Camera position
    rf_vector3 target;   // Camera target it looks-at
    rf_vector3 up;       // Camera up vector (rotation over its axis)
    float fovy;       // Camera field-of-view apperture in Y (degrees) in perspective, used as near plane width in orthographic
    int type;         // Camera type, defines GL_PROJECTION type: rf_camera_perspective or rf_camera_orthographic
};

// rf_camera2d type, defines a 2d camera
typedef struct rf_camera2d rf_camera2d;
struct rf_camera2d
{
    rf_vector2 offset; // Camera offset (displacement from target)
    rf_vector2 target; // Camera target (rotation and zoom origin)
    float rotation; // Camera rotation in degrees
    float zoom;     // Camera zoom (scaling), should be 1.0f by default
};

// Vertex data definning a mesh
// NOTE: Data stored in CPU memory (and GPU)
typedef struct rf_mesh rf_mesh;
struct rf_mesh
{
    int vertex_count; // Number of vertices stored in arrays
    int triangle_count; // Number of triangles stored (indexed or not)

    // Default vertex data
    float* vertices;         // Vertex position (XYZ - 3 components per vertex) (shader-location = 0)
    float* texcoords;        // Vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
    float* texcoords2;       // Vertex second texture coordinates (useful for lightmaps) (shader-location = 5)
    float* normals;          // Vertex normals (XYZ - 3 components per vertex) (shader-location = 2)
    float* tangents;         // Vertex tangents (XYZW - 4 components per vertex) (shader-location = 4)
    unsigned char* colors;   // Vertex colors (RGBA - 4 components per vertex) (shader-location = 3)
    unsigned short* indices; // Vertex indices (in case vertex data comes indexed)

    // Animation vertex data
    float* anim_vertices; // Animated vertex positions (after bones transformations)
    float* anim_normals;  // Animated normals (after bones transformations)
    int* bone_ids;        // Vertex bone ids, up to 4 bones influence by vertex (skinning)
    float* bone_weights;  // Vertex bone weight, up to 4 bones influence by vertex (skinning)

    // OpenGL identifiers
    unsigned int vao_id;  // OpenGL Vertex Array Object id
    unsigned int* vbo_id; // OpenGL Vertex Buffer Objects id (default vertex data)
};

// rf_shader type (generic)
typedef struct rf_shader rf_shader;
struct rf_shader
{
    unsigned int id; // rf_shader program id
    int* locs;       // rf_shader locations array (rf_max_shader_locations)
};

// rf_material texture map
typedef struct rf_material_map rf_material_map;
struct rf_material_map
{
    rf_texture2d texture; // rf_material map texture
    rf_color color;       // rf_material map color
    float value;       // rf_material map value
};

// rf_material type (generic)
typedef struct rf_material rf_material;
struct rf_material
{
    rf_shader shader;     // rf_material shader
    rf_material_map* maps; // rf_material maps array (rf_max_material_maps)
    float* params;     // rf_material generic parameters (if required)
};

// Transformation properties
typedef struct rf_transform rf_transform;
struct rf_transform
{
    rf_vector3 translation; // Translation
    rf_quaternion rotation; // Rotation
    rf_vector3 scale;       // Scale
};

// Bone information
typedef struct rf_bone_info rf_bone_info;
struct rf_bone_info
{
    char name[32]; // Bone name
    int  parent;   // Bone parent
};

// rf_model type
typedef struct rf_model rf_model;
struct rf_model
{
    rf_matrix transform; // Local transform matrix
    int mesh_count;    // Number of meshes
    rf_mesh* meshes;     // Meshes array

    int material_count;   // Number of materials
    rf_material* materials; // Materials array
    int* mesh_material;   // rf_mesh material number

    // Animation data
    int bone_count;       // Number of bones
    rf_bone_info* bones;     // Bones information (skeleton)
    rf_transform* bind_pose; // Bones base transformation (pose)
};

// rf_model animation
typedef struct rf_model_animation rf_model_animation;
struct rf_model_animation
{
    int bone_count;          // Number of bones
    rf_bone_info* bones;        // Bones information (skeleton)
    int frame_count;         // Number of animation frames
    rf_transform** frame_poses; // Poses array by frame
};

// rf_ray type (useful for raycast)
typedef struct rf_ray rf_ray;
struct rf_ray
{
    rf_vector3 position;  // rf_ray position (origin)
    rf_vector3 direction; // rf_ray direction
};

// Raycast hit information
typedef struct rf_ray_hit_info rf_ray_hit_info;
struct rf_ray_hit_info
{
    bool hit; // Did the ray hit something?
    float distance; // Distance to nearest hit
    rf_vector3 position; // Position of nearest hit
    rf_vector3 normal; // Surface normal of hit
};

// Bounding box type
typedef struct rf_bounding_box rf_bounding_box;
struct rf_bounding_box
{
    rf_vector3 min; // Minimum vertex box-corner
    rf_vector3 max; // Maximum vertex box-corner
};

// Dynamic vertex buffers (position + texcoords + colors + indices arrays)
typedef struct rf_dynamic_buffer rf_dynamic_buffer;
struct rf_dynamic_buffer
{
    int vCounter;               // vertex position counter to process (and draw) from full buffer
    int tcCounter;              // vertex texcoord counter to process (and draw) from full buffer
    int cCounter;               // vertex color counter to process (and draw) from full buffer
    float* vertices;            // vertex position (XYZ - 3 components per vertex) (shader-location = 0)
    float* texcoords;           // vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
    unsigned char* colors;      // vertex colors (RGBA - 4 components per vertex) (shader-location = 3)

#if defined(RF_GRAPHICS_API_OPENGL_11) || defined(RF_GRAPHICS_API_OPENGL_33)
    unsigned int* indices;      // vertex indices (in case vertex data comes indexed) (6 indices per quad)
#elif defined(RF_GRAPHICS_API_OPENGL_ES2)
    unsigned short *indices;    // vertex indices (in case vertex data comes indexed) (6 indices per quad)
    // NOTE: 6*2 rf_byte = 12 rf_byte, not alignment problem!
#endif

    unsigned int vao_id;         // OpenGL Vertex Array Object id
    unsigned int vbo_id[4];      // OpenGL Vertex Buffer Objects id (4 types of vertex data)
};

// Draw call type
typedef struct rf_draw_call rf_draw_call;
struct rf_draw_call
{
    int mode;                   // Drawing mode: LINES, TRIANGLES, QUADS
    int vertex_count;            // Number of vertex of the draw
    int vertexAlignment;        // Number of vertex required for index alignment (LINES, TRIANGLES)
    //unsigned int vao_id;         // Vertex array id to be used on the draw
    //unsigned int shaderId;      // rf_shader id to be used on the draw
    unsigned int textureId;     // rf_texture id to be used on the draw
    // TODO: Support additional texture units?

    //rf_matrix _rf_global_context_ptr->gl_ctx.projection;        // Projection matrix for this draw
    //rf_matrix _rf_global_context_ptr->gl_ctx.modelview;         // Modelview matrix for this draw
};

typedef struct rf_gl_context rf_gl_context;
struct rf_gl_context
{
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    rf_matrix stack[rf_max_matrix_stack_size];
    int stack_counter;

    rf_matrix modelview;
    rf_matrix projection;
    rf_matrix* current_matrix;
    int current_matrix_mode;
    float current_depth;

    // Default dynamic buffer for elements data
    // NOTE: A multi-buffering system is supported
    rf_dynamic_buffer vertex_data[rf_max_batch_buffering];
    int current_buffer;

    // transform matrix to be used with rlTranslate, rlRotate, rlScale
    rf_matrix transform_matrix;
    bool use_transform_matrix;

    // Default buffers draw calls
    rf_draw_call *draws;
    int draws_counter;

    // Default texture (1px white) useful for plain color polys (required by shader)
    unsigned int default_texture_id;

    // Default shaders
    unsigned int default_vertex_shader_id;   // Default vertex shader id (used by default shader program)
    unsigned int default_frag_shader_id;   // Default fragment shader Id (used by default shader program)

    rf_shader default_shader;        // Basic shader, support vertex color and diffuse texture
    rf_shader current_shader;        // rf_shader to be used on rendering (by default, default_shader)

    // Extension supported flag: VAO
    bool vao_supported;           // VAO support (OpenGL ES2 could not support VAO extension)

    // Extension supported flag: Compressed textures
    bool tex_comp_dxt_supported;    // DDS texture compression support
    bool tex_comp_etc1_supported;   // ETC1 texture compression support
    bool tex_comp_etc2_supported;   // ETC2/EAC texture compression support
    bool tex_comp_pvrt_supported;   // PVR texture compression support
    bool tex_comp_astc_supported;   // ASTC texture compression support

    // Extension supported flag: Textures format
    bool tex_npot_supported;       // NPOT textures full support
    bool tex_float_supported;      // float textures support (32 bit per channel)
    bool tex_depth_supported;      // Depth textures supported
    int max_depth_bits;        // Maximum bits for depth component

    // Extension supported flag: Clamp mirror wrap mode
    bool tex_mirror_clamp_supported;        // Clamp mirror wrap mode supported

    // Extension supported flag: Anisotropic filtering
    bool tex_anisotropic_filter_supported;  // Anisotropic texture filtering support
    float max_anisotropic_level;            // Maximum anisotropy level supported (minimum is 2.0f)

    bool debug_marker_supported;   // Debug marker support
#endif // RF_GRAPHICS_API_OPENGL_33 || RF_GRAPHICS_API_OPENGL_ES2

    int blend_mode;                   // Track current blending mode

    // Default framebuffer size
    int framebuffer_width;            // Default framebuffer width
    int framebuffer_height;           // Default framebuffer height

    //@Note(lulu): Camera 3d stuff, might extract into another struct
    rf_vector2 camera_angle; // rf_camera3d angle in plane XZ
    float camera_target_distance; // rf_camera3d distance from position to target
    float player_eyes_position; // Default player eyes position from ground (in meters)

    int camera_move_control[6];
    int camera_pan_control_key; // raylib: MOUSE_MIDDLE_BUTTON
    int camera_alt_control_key; // raylib: KEY_LEFT_ALT
    int camera_smooth_zoom_control_key; // raylib: KEY_LEFT_CONTROL

    int camera_mode; // Current camera mode

    //@Note(lulu): shapes global data
    rf_texture2d tex_shapes;
    rf_rectangle rec_tex_shapes;
};

typedef unsigned char rf_byte;

typedef struct rf_context rf_context;
struct rf_context
{
    //Display size
    union
    {
        rf_sizei display_size;
        struct
        {
            int display_width;
            int display_height;
        };
    };

    // Screen width and height (used render area)
    union
    {
        rf_sizei screen_size;
        struct
        {
            int screen_width;
            int screen_height;
        };
    };

    // Framebuffer width and height (render area, including black bars if required)
    union
    {
        rf_sizei render_size;
        struct
        {
            int render_width;
            int render_height;
        };
    };

    // Current render width and height, it could change on rf_begin_texture_mode()
    union
    {
        rf_sizei current_size;
        struct
        {
            int current_width;
            int current_height;
        };
    };

    int render_offset_x; // Offset X from render area (must be divided by 2)
    int render_offset_y; // Offset Y from render area (must be divided by 2)
    rf_matrix screen_scaling; // rf_matrix to scale screen (fr
    unsigned long long base_time; // Base time measure for hi-res timer

    double current_time; // Current time measure
    double previous_time; // Previous time measure
    double update_time; // Time measure for frame update
    double draw_time; // Time measure for frame draw
    double frame_time; // Time measure for one frame
    double target_time; // Desired time for one frame, if 0 not applied

    rf_font default_font; // Default font provided by raylib

    rf_gl_context gl_ctx;
};

//endregion

//region enums

// rf_shader location point type
typedef enum rf_shader_location_index
{
    rf_loc_vertex_position = 0,
    rf_loc_vertex_texcoord01,
    rf_loc_vertex_texcoord02,
    rf_loc_vertex_normal,
    rf_loc_vertex_tangent,
    rf_loc_vertex_color,
    rf_loc_matrix_mvp,
    rf_loc_matrix_model,
    rf_loc_matrix_view,
    rf_loc_matrix_projection,
    rf_loc_vector_view,
    rf_loc_color_diffuse,
    rf_loc_color_specular,
    rf_loc_color_ambient,
    rf_loc_map_albedo, // rf_loc_map_diffuse
    rf_loc_map_metalness, // rf_loc_map_specular
    rf_loc_map_normal,
    rf_loc_map_roughness,
    rf_loc_map_occlusion,
    rf_loc_map_emission,
    rf_loc_map_height,
    rf_loc_map_cubemap,
    rf_loc_map_irradiance,
    rf_loc_map_prefilter,
    rf_loc_map_brdf
} rf_shader_location_index;

#define rf_loc_map_diffuse rf_loc_map_albedo
#define rf_loc_map_specular rf_loc_map_metalness

// rf_shader uniform data types
typedef enum rf_shader_uniform_data_type
{
    rf_uniform_float = 0,
    rf_uniform_vec2,
    rf_uniform_vec3,
    rf_uniform_vec4,
    rf_uniform_int,
    rf_uniform_ivec2,
    rf_uniform_ivec3,
    rf_uniform_ivec4,
    rf_uniform_sampler2d
} rf_shader_uniform_data_type;

// rf_material map type
typedef enum rf_material_map_type
{
    rf_map_albedo = 0, // rf_map_diffuse
    rf_map_metalness = 1, // rf_map_specular
    rf_map_normal = 2,
    rf_map_roughness = 3,
    rf_map_occlusion,
    rf_map_emission,
    rf_map_height,
    rf_map_cubemap, // NOTE: Uses GL_TEXTURE_CUBE_MAP
    rf_map_irradiance, // NOTE: Uses GL_TEXTURE_CUBE_MAP
    rf_map_prefilter, // NOTE: Uses GL_TEXTURE_CUBE_MAP
    rf_map_brdf
} rf_material_map_type;

#define rf_map_diffuse rf_map_albedo
#define rf_map_specular rf_map_metalness

// Pixel formats
// NOTE: Support depends on OpenGL version and platform
typedef enum rf_pixel_format
{
    rf_uncompressed_grayscale = 1, // 8 bit per pixel (no alpha)
    rf_uncompressed_gray_alpha, // 8*2 bpp (2 channels)
    rf_uncompressed_r5g6b5, // 16 bpp
    rf_uncompressed_r8g8b8, // 24 bpp
    rf_uncompressed_r5g5b5a1, // 16 bpp (1 bit alpha)
    rf_uncompressed_r4g4b4a4, // 16 bpp (4 bit alpha)
    rf_uncompressed_r8g8b8a8, // 32 bpp
    rf_uncompressed_r32, // 32 bpp (1 channel - float)
    rf_uncompressed_r32g32b32, // 32*3 bpp (3 channels - float)
    rf_uncompressed_r32g32b32a32, // 32*4 bpp (4 channels - float)
    rf_compressed_dxt1_rgb, // 4 bpp (no alpha)
    rf_compressed_dxt1_rgba, // 4 bpp (1 bit alpha)
    rf_compressed_dxt3_rgba, // 8 bpp
    rf_compressed_dxt5_rgba, // 8 bpp
    rf_compressed_etc1_rgb, // 4 bpp
    rf_compressed_etc2_rgb, // 4 bpp
    rf_compressed_etc2_eac_rgba, // 8 bpp
    rf_compressed_pvrt_rgb, // 4 bpp
    rf_compressed_pvrt_rgba, // 4 bpp
    rf_compressed_astc_4x4_rgba, // 8 bpp
    rf_compressed_astc_8x8_rgba // 2 bpp
} rf_pixel_format;

// rf_texture parameters: filter mode
// NOTE 1: Filtering considers mipmaps if available in the texture
// NOTE 2: Filter is accordingly set for minification and magnification
typedef enum rf_texture_filter_mode
{
    rf_filter_point = 0, // No filter, just pixel aproximation
    rf_filter_bilinear, // Linear filtering
    rf_filter_trilinear, // Trilinear filtering (linear with mipmaps)
    rf_filter_anisotropic_4x, // Anisotropic filtering 4x
    rf_filter_anisotropic_8x, // Anisotropic filtering 8x
    rf_filter_anisotropic_16x, // Anisotropic filtering 16x
} rf_texture_filter_mode;

// Cubemap layout type
typedef enum rf_cubemap_layout_type
{
    rf_cubemap_auto_detect = 0, // Automatically detect layout type
    rf_cubemap_line_vertical, // Layout is defined by a vertical line with faces
    rf_cubemap_line_horizontal, // Layout is defined by an horizontal line with faces
    rf_cubemap_cross_three_by_four, // Layout is defined by a 3x4 cross with cubemap faces
    rf_cubemap_cross_four_by_three, // Layout is defined by a 4x3 cross with cubemap faces
    rf_cubemap_panorama // Layout is defined by a panorama image (equirectangular map)
} rf_cubemap_layout_type;

// rf_texture parameters: wrap mode
typedef enum rf_texture_wrap_mode
{
    rf_wrap_repeat = 0, // Repeats texture in tiled mode
    rf_wrap_clamp, // Clamps texture to edge pixel in tiled mode
    rf_wrap_mirror_repeat, // Mirrors and repeats the texture in tiled mode
    rf_wrap_mirror_clamp // Mirrors and clamps to border the texture in tiled mode
} rf_texture_wrap_mode;

// rf_font type, defines generation method
typedef enum rf_font_type
{
    rf_font_default = 0, // Default font generation, anti-aliased
    rf_font_bitmap, // Bitmap font generation, no anti-aliasing
    rf_font_sdf // SDF font generation, requires external shader
} rf_font_type;

// rf_color blending modes (pre-defined)
typedef enum rf_blend_mode
{
    rf_blend_alpha = 0, // Blend textures considering alpha (default)
    rf_blend_additive, // Blend textures adding colors
    rf_blend_multiplied // Blend textures multiplying colors
} rf_blend_mode;

// Camera system modes
typedef enum rf_camera_mode
{
    rf_camera_custom = 0,
    rf_camera_free,
    rf_camera_orbital,
    rf_camera_first_person,
    rf_camera_third_person
} rf_camera_mode;

// Camera GL_PROJECTION modes
typedef enum rf_camera_type
{
    rf_camera_perspective = 0,
    rf_camera_orthographic
} rf_camera_type;

// Type of n-patch
typedef enum rf_ninepatch_type
{
    rf_npt_9patch = 0, // Npatch defined by 3x3 tiles
    rf_npt_3patch_vertical, // Npatch defined by 1x3 tiles
    rf_npt_3patch_horizontal // Npatch defined by 3x1 tiles
} rf_ninepatch_type;

//endregion

//region platform layer

RF_API void rf_wait(float); // Wait for some milliseconds (pauses program execution)
RF_API double rf_get_time(void); // Returns elapsed time in seconds since rf_context_init

// Files management functions
RF_API int rf_get_file_size(const char* filename);
RF_API void rf_load_file_into_buffer(const char* filename, rf_byte* buffer, int buffer_size);

//endregion

//region basic

// Initialisation functions
RF_API void rf_context_init(rf_context* rf_ctx, int width, int height);
RF_API void rf_set_global_context_ptr(rf_context* rf_ctx);
RF_API void rf_load_font_default();

// Drawing-related functions
RF_API void rf_clear_background(rf_color color); // Set background color (framebuffer clear color)
RF_API void rf_begin_drawing(); // Setup canvas (framebuffer) to start drawing
RF_API void rf_end_drawing(); // End canvas drawing and swap buffers (double buffering)
RF_API void rf_begin_mode2d(rf_camera2d camera); // Initialize 2D mode with custom camera (2D)
RF_API void rf_end_mode2d(); // Ends 2D mode with custom camera
RF_API void rf_begin_mode3d(rf_camera3d camera); // Initializes 3D mode with custom camera (3D)
RF_API void rf_end_mode3d(); // Ends 3D mode and returns to default 2D orthographic mode
RF_API void rf_begin_texture_mode(rf_render_texture2d target); // Initializes render texture for drawing
RF_API void rf_end_texture_mode(); // Ends drawing to render texture
RF_API void rf_begin_scissor_mode(int x, int y, int width, int height); // Begin scissor mode (define screen area for following drawing)
RF_API void rf_end_scissor_mode(); // End scissor mode

// Screen-space-related functions
RF_API rf_ray rf_get_mouse_ray(rf_sizei screen_size, rf_vector2 mouse_position, rf_camera3d camera); // Returns a ray trace from mouse position
RF_API rf_matrix rf_get_camera_matrix(rf_camera3d camera); // Returns camera transform matrix (view matrix)
RF_API rf_matrix rf_get_camera_matrix2d(rf_camera2d camera); // Returns camera 2d transform matrix
RF_API rf_vector2 rf_get_world_to_screen(rf_sizei screen_size, rf_vector3 position, rf_camera3d camera); // Returns the screen space position for a 3d world space position
RF_API rf_vector2 rf_get_world_to_screen2d(rf_vector2 position, rf_camera2d camera); // Returns the screen space position for a 2d camera world space position
RF_API rf_vector2 rf_get_screen_to_world2d(rf_vector2 position, rf_camera2d camera); // Returns the world space position for a 2d camera screen space position

// Timing-related functions
RF_API void rf_set_target_fps(int fps); // Set target FPS (maximum)
RF_API int rf_get_fps();           // Returns current FPS
RF_API float rf_get_frame_time();   // Returns time in seconds for last frame drawn

// rf_color-related functions
RF_API int rf_color_to_int(rf_color color); // Returns hexadecimal value for a rf_color
RF_API rf_vector4 rf_color_normalize(rf_color color); // Returns color normalized as float [0..1]
RF_API rf_color rf_color_from_normalized(rf_vector4 normalized); // Returns color from normalized values [0..1]
RF_API rf_vector3 rf_color_to_hsv(rf_color color); // Returns HSV values for a rf_color
RF_API rf_color rf_color_from_hsv(rf_vector3 hsv); // Returns a rf_color from HSV values
RF_API rf_color rf_color_from_int(int hexValue); // Returns a rf_color struct from hexadecimal value
RF_API rf_color rf_fade(rf_color color, float alpha); // rf_color fade-in or fade-out, alpha goes from 0.0f to 1.0f

// Camera System Functions (Module: camera)
RF_API void rf_set_camera_mode(rf_camera3d camera, int mode); // Set camera mode (multiple camera modes available)
RF_API void rf_update_camera3d(rf_camera3d* camera, rf_input_state_for_update_camera inputState); // Update camera position for selected mode

RF_API void rf_set_camera_pan_control(int panKey); // Set camera pan key to combine with mouse movement (free camera)
RF_API void rf_set_camera_alt_control(int altKey); // Set camera alt key to combine with mouse movement (free camera)
RF_API void rf_set_camera_smooth_zoom_control(int szKey); // Set camera smooth zoom key to combine with mouse (free camera)
RF_API void rf_set_camera_move_controls(int frontKey, int backKey, int rightKey, int leftKey, int upKey, int downKey); // Set camera move controls (1st person and 3rd person cameras)

//------------------------------------------------------------------------------------
// Basic Shapes Drawing Functions (Module: shapes)
//------------------------------------------------------------------------------------

// Basic shapes drawing functions
RF_API void rf_draw_pixel(int posX, int posY, rf_color color); // Draw a pixel
RF_API void rf_draw_pixel_v(rf_vector2 position, rf_color color); // Draw a pixel (Vector version)
RF_API void rf_draw_line(int startPosX, int startPosY, int endPosX, int endPosY, rf_color color); // Draw a line
RF_API void rf_draw_line_v(rf_vector2 startPos, rf_vector2 endPos, rf_color color); // Draw a line (Vector version)
RF_API void rf_draw_line_ex(rf_vector2 startPos, rf_vector2 endPos, float thick, rf_color color); // Draw a line defining thickness
RF_API void rf_draw_line_bezier(rf_vector2 startPos, rf_vector2 endPos, float thick, rf_color color); // Draw a line using cubic-bezier curves in-out
RF_API void rf_draw_line_strip(rf_vector2* points, int numPoints, rf_color color); // Draw lines sequence
RF_API void rf_draw_circle(int centerX, int centerY, float radius, rf_color color); // Draw a color-filled circle
RF_API void rf_draw_circle_sector(rf_vector2 center, float radius, int startAngle, int endAngle, int segments, rf_color color); // Draw a piece of a circle
RF_API void rf_draw_circle_sector_lines(rf_vector2 center, float radius, int startAngle, int endAngle, int segments, rf_color color); // Draw circle sector outline
RF_API void rf_draw_circle_gradient(int centerX, int centerY, float radius, rf_color color1, rf_color color2); // Draw a gradient-filled circle
RF_API void rf_draw_circle_v(rf_vector2 center, float radius, rf_color color); // Draw a color-filled circle (Vector version)
RF_API void rf_draw_circle_lines(int centerX, int centerY, float radius, rf_color color); // Draw circle outline
RF_API void rf_draw_ring(rf_vector2 center, float innerRadius, float outerRadius, int startAngle, int endAngle, int segments, rf_color color); // Draw ring
RF_API void rf_draw_ring_lines(rf_vector2 center, float innerRadius, float outerRadius, int startAngle, int endAngle, int segments, rf_color color); // Draw ring outline
RF_API void rf_draw_rectangle(int posX, int posY, int width, int height, rf_color color); // Draw a color-filled rectangle
RF_API void rf_draw_rectangle_v(rf_vector2 position, rf_vector2 size, rf_color color); // Draw a color-filled rectangle (Vector version)
RF_API void rf_draw_rectangle_rec(rf_rectangle rec, rf_color color); // Draw a color-filled rectangle
RF_API void rf_draw_rectangle_pro(rf_rectangle rec, rf_vector2 origin, float rotation, rf_color color); // Draw a color-filled rectangle with pro parameters
RF_API void rf_draw_rectangle_gradient_v(int posX, int posY, int width, int height, rf_color color1, rf_color color2);// Draw a vertical-gradient-filled rectangle
RF_API void rf_draw_rectangle_gradient_h(int posX, int posY, int width, int height, rf_color color1, rf_color color2);// Draw a horizontal-gradient-filled rectangle
RF_API void rf_draw_rectangle_gradient_ex(rf_rectangle rec, rf_color col1, rf_color col2, rf_color col3, rf_color col4); // Draw a gradient-filled rectangle with custom vertex colors
RF_API void rf_draw_rectangle_lines(int posX, int posY, int width, int height, rf_color color); // Draw rectangle outline
RF_API void rf_draw_rectangle_lines_ex(rf_rectangle rec, int lineThick, rf_color color); // Draw rectangle outline with extended parameters
RF_API void rf_draw_rectangle_rounded(rf_rectangle rec, float roundness, int segments, rf_color color); // Draw rectangle with rounded edges
RF_API void rf_draw_rectangle_rounded_lines(rf_rectangle rec, float roundness, int segments, int lineThick, rf_color color); // Draw rectangle with rounded edges outline
RF_API void rf_draw_triangle(rf_vector2 v1, rf_vector2 v2, rf_vector2 v3, rf_color color); // Draw a color-filled triangle (vertex in counter-clockwise order!)
RF_API void rf_draw_triangle_lines(rf_vector2 v1, rf_vector2 v2, rf_vector2 v3, rf_color color); // Draw triangle outline (vertex in counter-clockwise order!)
RF_API void rf_draw_triangle_fan(rf_vector2* points, int numPoints, rf_color color); // Draw a triangle fan defined by points (first vertex is the center)
RF_API void rf_draw_triangle_strip(rf_vector2* points, int pointsCount, rf_color color); // Draw a triangle strip defined by points
RF_API void rf_draw_poly(rf_vector2 center, int sides, float radius, float rotation, rf_color color); // Draw a regular polygon (Vector version)

RF_API void rf_set_shapes_texture(rf_texture2d texture, rf_rectangle source); // Define default texture used to draw shapes

// rf_image/rf_texture2d data loading/unloading/saving functions
RF_API rf_image rf_load_image(const char* fileName); // Load image from file into CPU memory (RAM)
RF_API rf_image rf_load_image_ex(rf_color* pixels, int width, int height); // Load image from rf_color array data (RGBA - 32bit)
RF_API rf_image rf_load_image_pro(void* data, int width, int height, int format); // Load image from raw data with parameters
RF_API rf_image rf_load_image_raw(const char* fileName, int width, int height, int format, int headerSize); // Load image from RAW file data
RF_API void rf_export_image(rf_image image, const char* fileName); // Export image data to file
RF_API rf_texture2d rf_load_texture(const char* fileName); // Load texture from file into GPU memory (VRAM)
RF_API rf_texture2d rf_load_texture_from_image(rf_image image); // Load texture from image data
RF_API rf_texture_cubemap rf_load_texture_cubemap(rf_image image, int layoutType); // Load cubemap from image, multiple image cubemap layouts supported
RF_API rf_render_texture2d rf_load_render_texture(int width, int height); // Load texture for rendering (framebuffer)
RF_API void rf_unload_image(rf_image image); // Unload image from CPU memory (RAM)
RF_API void rf_unload_texture(rf_texture2d texture); // Unload texture from GPU memory (VRAM)
RF_API void rf_unload_render_texture(rf_render_texture2d target); // Unload render texture from GPU memory (VRAM)
RF_API rf_color* rf_get_image_data(rf_image image); // Get pixel data from image as a rf_color struct array
RF_API rf_vector4* rf_get_image_data_normalized(rf_image image); // Get pixel data from image as rf_vector4 array (float normalized)
RF_API rf_rectangle rf_get_image_alpha_border(rf_image image, float threshold); // Get image alpha border rectangle
RF_API int rf_get_pixel_data_size(int width, int height, int format); // Get pixel data size in bytes (image or texture)
RF_API rf_image rf_get_texture_data(rf_texture2d texture); // Get pixel data from GPU texture and return an rf_image
RF_API rf_image rf_get_screen_data(); // Get pixel data from screen buffer and return an rf_image (screenshot)
RF_API void rf_update_texture(rf_texture2d texture, const void* pixels); // Update GPU texture with new data

// rf_image manipulation functions
RF_API rf_image rf_image_copy(rf_image image); // Create an image duplicate (useful for transformations)
RF_API rf_image rf_image_from_image(rf_image image, rf_rectangle rec); // Create an image from another image piece
RF_API void rf_image_to_pot(rf_image* image, rf_color fillColor); // Convert image to POT (power-of-two)
RF_API void rf_image_format(rf_image* image, int newFormat); // Convert image data to desired format
RF_API void rf_image_alpha_mask(rf_image* image, rf_image alphaMask); // Apply alpha mask to image
RF_API void rf_image_alpha_clear(rf_image* image, rf_color color, float threshold); // Clear alpha channel to desired color
RF_API void rf_image_alpha_crop(rf_image* image, float threshold); // Crop image depending on alpha value
RF_API void rf_image_alpha_premultiply(rf_image* image); // Premultiply alpha channel
RF_API void rf_image_crop(rf_image* image, rf_rectangle crop); // Crop an image to a defined rectangle
RF_API void rf_image_resize(rf_image* image, int newWidth, int newHeight); // Resize image (Bicubic scaling algorithm)
RF_API void rf_image_resize_nn(rf_image* image, int newWidth,int newHeight); // Resize image (Nearest-Neighbor scaling algorithm)
RF_API void rf_image_resize_canvas(rf_image* image, int newWidth, int newHeight, int offset_x, int offset_y, rf_color color); // Resize canvas and fill with color
RF_API void rf_image_mipmaps(rf_image* image); // Generate all mipmap levels for a provided image
RF_API void rf_image_dither(rf_image* image, int rBpp, int gBpp, int bBpp, int aBpp); // Dither image data to 16bpp or lower (Floyd-Steinberg dithering)
RF_API rf_color* rf_image_extract_palette(rf_image image, int maxPaletteSize, int* extractCount); // Extract color palette from image to maximum size (memory should be freed)
RF_API rf_image rf_image_text(const char* text, int fontSize, rf_color color); // Create an image from text (default font)
RF_API rf_image rf_image_text_ex(rf_font font, const char* text, float fontSize, float spacing, rf_color tint); // Create an image from text (custom sprite font)
RF_API void rf_image_draw(rf_image* dst, rf_image src, rf_rectangle srcRec, rf_rectangle dstRec, rf_color tint); // Draw a source image within a destination image (tint applied to source)
RF_API void rf_image_draw_rectangle(rf_image* dst, rf_rectangle rec, rf_color color); // Draw rectangle within an image
RF_API void rf_image_draw_rectangle_lines(rf_image* dst, rf_rectangle rec, int thick, rf_color color); // Draw rectangle lines within an image
RF_API void rf_image_draw_text(rf_image* dst, rf_vector2 position, const char* text, int fontSize, rf_color color); // Draw text (default font) within an image (destination)
RF_API void rf_image_draw_text_ex(rf_image* dst, rf_vector2 position, rf_font font, const char* text, float fontSize, float spacing, rf_color color); // Draw text (custom sprite font) within an image (destination)
RF_API void rf_image_flip_vertical(rf_image* image); // Flip image vertically
RF_API void rf_image_flip_horizontal(rf_image* image); // Flip image horizontally
RF_API void rf_image_rotate_cw(rf_image* image); // Rotate image clockwise 90deg
RF_API void rf_image_rotate_ccw(rf_image* image); // Rotate image counter-clockwise 90deg
RF_API void rf_image_color_tint(rf_image* image, rf_color color); // Modify image color: tint
RF_API void rf_image_color_invert(rf_image* image); // Modify image color: invert
RF_API void rf_image_color_grayscale(rf_image* image); // Modify image color: grayscale
RF_API void rf_image_color_contrast(rf_image* image, float contrast); // Modify image color: contrast (-100 to 100)
RF_API void rf_image_color_brightness(rf_image* image, int brightness); // Modify image color: brightness (-255 to 255)
RF_API void rf_image_color_replace(rf_image* image, rf_color color, rf_color replace); // Modify image color: replace color

// rf_image generation functions
RF_API rf_image rf_gen_image_color(int width, int height, rf_color color); // Generate image: plain color
RF_API rf_image rf_gen_image_gradient_v(int width, int height, rf_color top, rf_color bottom); // Generate image: vertical gradient
RF_API rf_image rf_gen_image_gradient_h(int width, int height, rf_color left, rf_color right); // Generate image: horizontal gradient
RF_API rf_image rf_gen_image_gradient_radial(int width, int height, float density, rf_color inner, rf_color outer); // Generate image: radial gradient
RF_API rf_image rf_gen_image_checked(int width, int height, int checksX, int checksY, rf_color col1, rf_color col2); // Generate image: checked
RF_API rf_image rf_gen_image_white_noise(int width, int height, float factor); // Generate image: white noise
RF_API rf_image rf_gen_image_perlin_noise(int width, int height, int offset_x, int offset_y, float scale); // Generate image: perlin noise
RF_API rf_image rf_gen_image_cellular(int width, int height, int tileSize); // Generate image: cellular algorithm. Bigger tileSize means bigger cells

// rf_texture2d configuration functions
RF_API void rf_gen_texture_mipmaps(rf_texture2d* texture); // Generate GPU mipmaps for a texture
RF_API void rf_set_texture_filter(rf_texture2d texture, int filterMode); // Set texture scaling filter mode
RF_API void rf_set_texture_wrap(rf_texture2d texture, int wrapMode); // Set texture wrapping mode

// rf_texture2d drawing functions
RF_API void rf_draw_texture(rf_texture2d texture, int posX, int posY, rf_color tint); // Draw a rf_texture2d
RF_API void rf_draw_texture_v(rf_texture2d texture, rf_vector2 position, rf_color tint); // Draw a rf_texture2d with position defined as rf_vector2
RF_API void rf_draw_texture_ex(rf_texture2d texture, rf_vector2 position, float rotation, float scale, rf_color tint); // Draw a rf_texture2d with extended parameters
RF_API void rf_draw_texture_rec(rf_texture2d texture, rf_rectangle source_rec, rf_vector2 position, rf_color tint); // Draw a part of a texture defined by a rectangle
RF_API void rf_draw_texture_quad(rf_texture2d texture, rf_vector2 tiling, rf_vector2 offset, rf_rectangle quad, rf_color tint); // Draw texture quad with tiling and offset parameters
RF_API void rf_draw_texture_pro(rf_texture2d texture, rf_rectangle source_rec, rf_rectangle destRec, rf_vector2 origin, float rotation, rf_color tint); // Draw a part of a texture defined by a rectangle with 'pro' parameters
RF_API void rf_draw_texture_npatch(rf_texture2d texture, rf_npatch_info nPatchInfo, rf_rectangle destRec, rf_vector2 origin, float rotation, rf_color tint); // Draws a texture (or part of it) that stretches or shrinks nicely

// rf_font loading/unloading functions
RF_API rf_font rf_get_font_default(); // Get the default rf_font
RF_API rf_font rf_load_font(const char* fileName); // Load font from file into GPU memory (VRAM)
RF_API rf_font rf_load_font_ex(const char* fileName, int fontSize, int* fontChars, int chars_count); // Load font from file with extended parameters
RF_API rf_load_font_async_result rf_load_font_async(const char* fileName, int fontSize, int* fontChars, int chars_count);
RF_API rf_font rf_finish_load_font_async(rf_load_font_async_result fontJobResult);
RF_API rf_font rf_load_font_from_image(rf_image image, rf_color key, int firstChar); // Load font from rf_image (XNA style)

#ifndef RF_NO_STB_TRUETYPE
RF_API rf_char_info* rf_load_font_data(const char* fileName, int fontSize, int* fontChars, int chars_count, int type); // Load font data for further use
#endif

RF_API rf_image rf_gen_image_font_atlas(const rf_char_info* chars, rf_rectangle** recs, int chars_count, int fontSize, int padding, int packMethod); // Generate image font atlas using chars info
RF_API void rf_unload_font(rf_font font); // Unload rf_font from GPU memory (VRAM)
RF_API void rf_unload_font_default();

// Text drawing functions
RF_API void rf_draw_fps(int posX, int posY); // Shows current FPS
RF_API void rf_draw_text(const char* text, int posX, int posY, int fontSize, rf_color color); // Draw text (using default font)
RF_API void rf_draw_text_ex(rf_font font, const char* text, rf_vector2 position, float fontSize, float spacing, rf_color tint); // Draw text using font and additional parameters
RF_API void rf_draw_text_from_buffer(rf_font font, const char* text, int length, rf_vector2 position, float fontSize, float spacing, rf_color tint); //Draw text using font from text buffer
RF_API void rf_draw_text_rec(rf_font font, const char* text, rf_rectangle rec, float fontSize, float spacing, bool wordWrap, rf_color tint); // Draw text using font inside rectangle limits
RF_API void rf_draw_text_rec_ex(rf_font font, const char* text, rf_rectangle rec, float fontSize, float spacing, bool wordWrap, rf_color tint,
                         int selectStart, int selectLength, rf_color selectText, rf_color selectBack); // Draw text using font inside rectangle limits with support for text selection

// Text misc. functions
RF_API int rf_measure_text(const char* text, int fontSize); // Measure string width for default font
RF_API rf_vector2 rf_measure_text_ex(rf_font font, const char* text, float fontSize, float spacing); // Measure string size for rf_font
RF_API rf_vector2 rf_measure_text_from_buffer(rf_font font, const char* text, int len, float fontSize, float spacing);
RF_API int rf_get_glyph_index(rf_font font, int character); // Get index position for a unicode character on font
RF_API float rf_measure_height_of_text_in_container(rf_font font, float fontSize, const char* text, int length, float container_width);

// Basic geometric 3D shapes drawing functions
RF_API void rf_draw_line3d(rf_vector3 startPos, rf_vector3 endPos, rf_color color); // Draw a line in 3D world space
RF_API void rf_draw_circle3d(rf_vector3 center, float radius, rf_vector3 rotationAxis, float rotationAngle, rf_color color); // Draw a circle in 3D world space
RF_API void rf_draw_cube(rf_vector3 position, float width, float height, float length, rf_color color); // Draw cube
RF_API void rf_draw_cube_v(rf_vector3 position, rf_vector3 size, rf_color color); // Draw cube (Vector version)
RF_API void rf_draw_cube_wires(rf_vector3 position, float width, float height, float length, rf_color color); // Draw cube wires
RF_API void rf_draw_cube_wires_v(rf_vector3 position, rf_vector3 size, rf_color color); // Draw cube wires (Vector version)
RF_API void rf_draw_cube_texture(rf_texture2d texture, rf_vector3 position, float width, float height, float length, rf_color color); // Draw cube textured
RF_API void rf_draw_sphere(rf_vector3 centerPos, float radius, rf_color color); // Draw sphere
RF_API void rf_draw_sphere_ex(rf_vector3 centerPos, float radius, int rings, int slices, rf_color color); // Draw sphere with extended parameters
RF_API void rf_draw_sphere_wires(rf_vector3 centerPos, float radius, int rings, int slices, rf_color color); // Draw sphere wires
RF_API void rf_draw_cylinder(rf_vector3 position, float radiusTop, float radiusBottom, float height, int slices, rf_color color); // Draw a cylinder/cone
RF_API void rf_draw_cylinder_wires(rf_vector3 position, float radiusTop, float radiusBottom, float height, int slices, rf_color color); // Draw a cylinder/cone wires
RF_API void rf_draw_plane(rf_vector3 centerPos, rf_vector2 size, rf_color color); // Draw a plane XZ
RF_API void rf_draw_ray(rf_ray ray, rf_color color); // Draw a ray line
RF_API void rf_draw_grid(int slices, float spacing); // Draw a grid (centered at (0, 0, 0))
RF_API void rf_draw_gizmo( rf_vector3 position); // Draw simple gizmo

// rf_model loading/unloading functions
RF_API rf_model rf_load_model(const char* fileName); // Load model from files (meshes and materials)
RF_API rf_model rf_load_model_from_mesh(rf_mesh mesh); // Load model from generated mesh (default material)
RF_API void rf_unload_model(rf_model model); // Unload model from memory (RAM and/or VRAM)

// rf_mesh loading/unloading functions
RF_API rf_mesh* rf_load_meshes(const char* fileName, int* mesh_count); // Load meshes from model file
RF_API void rf_export_mesh(rf_mesh mesh, const char* fileName); // Export mesh data to file
RF_API void rf_unload_mesh(rf_mesh mesh); // Unload mesh from memory (RAM and/or VRAM)

// rf_material loading/unloading functions
RF_API rf_material* rf_load_materials(const char* fileName, int* material_count); // Load materials from model file
RF_API rf_material rf_load_material_default() ; // Load default material (Supports: DIFFUSE, SPECULAR, NORMAL maps)
RF_API void rf_unload_material(rf_material material); // Unload material from GPU memory (VRAM)
RF_API void rf_set_material_texture(rf_material* material, int mapType, rf_texture2d texture); // Set texture for a material map type (rf_map_diffuse, rf_map_specular...)
RF_API void rf_set_model_mesh_material(rf_model* model, int meshId, int materialId); // Set material for a mesh

// rf_model animations loading/unloading functions
RF_API rf_model_animation* rf_load_model_animations(const char* fileName, int* animsCount); // Load model animations from file
RF_API void rf_update_model_animation(rf_model model, rf_model_animation anim, int frame); // Update model animation pose
RF_API void rf_unload_model_animation(rf_model_animation anim); // Unload animation data
RF_API bool rf_is_model_animation_valid(rf_model model, rf_model_animation anim); // Check model animation skeleton match

// rf_mesh generation functions
RF_API rf_mesh rf_gen_mesh_poly(int sides, float radius); // Generate polygonal mesh
RF_API rf_mesh rf_gen_mesh_plane(float width, float length, int resX, int resZ); // Generate plane mesh (with subdivisions)
RF_API rf_mesh rf_gen_mesh_cube(float width, float height, float length); // Generate cuboid mesh
RF_API rf_mesh rf_gen_mesh_sphere(float radius, int rings, int slices); // Generate sphere mesh (standard sphere)
RF_API rf_mesh rf_gen_mesh_hemi_sphere(float radius, int rings, int slices); // Generate half-sphere mesh (no bottom cap)
RF_API rf_mesh rf_gen_mesh_cylinder(float radius, float height, int slices); // Generate cylinder mesh
RF_API rf_mesh rf_gen_mesh_torus(float radius, float size, int radSeg, int sides); // Generate torus mesh
RF_API rf_mesh rf_gen_mesh_knot(float radius, float size, int radSeg, int sides); // Generate trefoil knot mesh
RF_API rf_mesh rf_gen_mesh_heightmap(rf_image heightmap, rf_vector3 size); // Generate heightmap mesh from image data
RF_API rf_mesh rf_gen_mesh_cubicmap(rf_image cubicmap, rf_vector3 cubeSize); // Generate cubes-based map mesh from image data

// rf_mesh manipulation functions
RF_API rf_bounding_box rf_mesh_bounding_box(rf_mesh mesh); // Compute mesh bounding box limits
RF_API void rf_mesh_tangents(rf_mesh* mesh); // Compute mesh tangents
RF_API void rf_mesh_binormals(rf_mesh* mesh); // Compute mesh binormals

// rf_model drawing functions
RF_API void rf_draw_model(rf_model model, rf_vector3 position, float scale, rf_color tint); // Draw a model (with texture if set)
RF_API void rf_draw_model_ex(rf_model model, rf_vector3 position, rf_vector3 rotationAxis, float rotationAngle, rf_vector3 scale, rf_color tint); // Draw a model with extended parameters
RF_API void rf_draw_model_wires(rf_model model, rf_vector3 position, float scale, rf_color tint); // Draw a model wires (with texture if set)
RF_API void rf_draw_model_wires_ex(rf_model model, rf_vector3 position, rf_vector3 rotationAxis, float rotationAngle, rf_vector3 scale, rf_color tint); // Draw a model wires (with texture if set) with extended parameters
RF_API void rf_draw_bounding_box(rf_bounding_box box, rf_color color); // Draw bounding box (wires)
RF_API void rf_draw_billboard(rf_camera3d camera, rf_texture2d texture, rf_vector3 center, float size, rf_color tint); // Draw a billboard texture
RF_API void rf_draw_billboard_rec(rf_camera3d camera, rf_texture2d texture, rf_rectangle source_rec, rf_vector3 center, float size, rf_color tint); // Draw a billboard texture defined by source_rec

// Collision detection functions
RF_API bool rf_check_collision_spheres(rf_vector3 centerA, float radiusA, rf_vector3 centerB, float radiusB); // Detect collision between two spheres
RF_API bool rf_check_collision_boxes(rf_bounding_box box1, rf_bounding_box box2); // Detect collision between two bounding boxes
RF_API bool rf_check_collision_box_sphere(rf_bounding_box box, rf_vector3 center, float radius); // Detect collision between box and sphere
RF_API bool rf_check_collision_ray_sphere(rf_ray ray, rf_vector3 center, float radius); // Detect collision between ray and sphere
RF_API bool rf_check_collision_ray_sphere_ex(rf_ray ray, rf_vector3 center, float radius, rf_vector3* collisionPoint); // Detect collision between ray and sphere, returns collision point
RF_API bool rf_check_collision_ray_box(rf_ray ray, rf_bounding_box box); // Detect collision between ray and box
RF_API rf_ray_hit_info rf_get_collision_ray_model(rf_ray ray, rf_model model); // Get collision info between ray and model
RF_API rf_ray_hit_info rf_get_collision_ray_triangle(rf_ray ray, rf_vector3 p1, rf_vector3 p2, rf_vector3 p3); // Get collision info between ray and triangle
RF_API rf_ray_hit_info rf_get_collision_ray_ground(rf_ray ray, float groundHeight); // Get collision info between ray and ground plane (Y-normal plane)
//endregion

//NOTE: This functions are useless when using OpenGL 1.1
//region rf_shader

// rf_shader loading/unloading functions
RF_API rf_shader rf_load_shader(const char* vsFileName, const char* fsFileName); // Load shader from files and bind default locations
RF_API rf_shader rf_load_shader_code(const char* vsCode, const char* fsCode); // Load shader from code strings and bind default locations
RF_API void rf_unload_shader(rf_shader shader); // Unload shader from GPU memory (VRAM)

RF_API rf_shader rf_get_shader_default() ; // Get default shader
RF_API rf_texture2d rf_get_texture_default() ; // Get default texture

// rf_shader configuration functions
RF_API int rf_get_shader_location(rf_shader shader, const char* uniformName); // Get shader uniform location
RF_API void rf_set_shader_value(rf_shader shader, int uniformLoc, const void* value, int uniformType); // Set shader uniform value
RF_API void rf_set_shader_value_v(rf_shader shader, int uniformLoc, const void* value, int uniformType, int count); // Set shader uniform value vector
RF_API void rf_set_shader_value_matrix(rf_shader shader, int uniformLoc, rf_matrix mat); // Set shader uniform value (matrix 4x4)
RF_API void rf_set_shader_value_texture(rf_shader shader, int uniformLoc, rf_texture2d texture); // Set shader uniform value for texture
RF_API void rf_set_matrix_projection(rf_matrix proj); // Set a custom GL_PROJECTION matrix (replaces internal GL_PROJECTION matrix)
RF_API void rf_set_matrix_modelview(rf_matrix view); // Set a custom rf_global_model_view matrix (replaces internal rf_global_model_view matrix)
RF_API rf_matrix rf_get_matrix_modelview(); // Get internal rf_global_model_view matrix
RF_API rf_matrix rf_get_matrix_projection(); // Get internal GL_PROJECTION matrix

// rf_texture maps generation (PBR)
// NOTE: Required shaders should be provided
RF_API rf_texture2d rf_gen_texture_cubemap(rf_shader shader, rf_texture2d skyHDR, int size); // Generate cubemap texture from HDR texture
RF_API rf_texture2d rf_gen_texture_irradiance(rf_shader shader, rf_texture2d cubemap, int size); // Generate irradiance texture using cubemap data
RF_API rf_texture2d rf_gen_texture_prefilter(rf_shader shader, rf_texture2d cubemap, int size); // Generate prefilter texture using cubemap data
RF_API rf_texture2d rf_gen_texture_brdf(rf_shader shader, int size); // Generate BRDF texture

// Shading begin/end functions
RF_API void rf_begin_shader_mode(rf_shader shader); // Begin custom shader drawing
RF_API void rf_end_shader_mode(); // End custom shader drawing (use default shader)
RF_API void rf_begin_blend_mode(int mode); // Begin blending mode (alpha, additive, multiplied)
RF_API void rf_end_blend_mode() ; // End blending mode (reset to default: alpha blending)
//endregion

//region rlgl

//------------------------------------------------------------------------------------
// Functions Declaration - rf_matrix operations
//------------------------------------------------------------------------------------
RF_API void rf_matrix_mode(int mode); // Choose the current matrix to be transformed
RF_API void rf_push_matrix(); // Push the current matrix to rf_global_gl_stack
RF_API void rf_pop_matrix(); // Pop lattest inserted matrix from rf_global_gl_stack
RF_API void rf_load_identity(); // Reset current matrix to identity matrix
RF_API void rf_translatef(float x, float y, float z); // Multiply the current matrix by a translation matrix
RF_API void rf_rotatef(float angleDeg, float x, float y, float z); // Multiply the current matrix by a rotation matrix
RF_API void rf_scalef(float x, float y, float z); // Multiply the current matrix by a scaling matrix
RF_API void rf_mult_matrixf(float* matf); // Multiply the current matrix by another matrix
RF_API void rf_frustum(double left, double right, double bottom, double top, double znear, double zfar);
RF_API void rf_ortho(double left, double right, double bottom, double top, double znear, double zfar);
RF_API void rf_gl_viewport(int x, int y, int width, int height); // Set the viewport area

//------------------------------------------------------------------------------------
// Functions Declaration - Vertex level operations
//------------------------------------------------------------------------------------
RF_API void rf_gl_begin(int mode); // Initialize drawing mode (how to organize vertex)
RF_API void rf_gl_end(); // Finish vertex providing
RF_API void rf_gl_vertex2i(int x, int y); // Define one vertex (position) - 2 int
RF_API void rf_gl_vertex2f(float x, float y); // Define one vertex (position) - 2 float
RF_API void rf_gl_vertex3f(float x, float y, float z); // Define one vertex (position) - 3 float
RF_API void rf_gl_tex_coord2f(float x, float y); // Define one vertex (texture coordinate) - 2 float
RF_API void rf_gl_normal3f(float x, float y, float z); // Define one vertex (normal) - 3 float
RF_API void rf_gl_color4ub(rf_byte r, rf_byte g, rf_byte b, rf_byte a); // Define one vertex (color) - 4 rf_byte
RF_API void rf_gl_color3f(float x, float y, float z); // Define one vertex (color) - 3 float
RF_API void rf_gl_color4f(float x, float y, float z, float w); // Define one vertex (color) - 4 float

//------------------------------------------------------------------------------------
// Functions Declaration - OpenGL equivalent functions (common to 1.1, 3.3+, ES2)
// NOTE: This functions are used to completely abstract raylib code from OpenGL layer
//------------------------------------------------------------------------------------
RF_API void rf_gl_enable_texture(unsigned int id); // Enable texture usage
RF_API void rf_gl_disable_texture(); // Disable texture usage
RF_API void rf_gl_texture_parameters(unsigned int id, int param, int value); // Set texture parameters (filter, wrap)
RF_API void rf_gl_enable_render_texture(unsigned int id); // Enable render texture (fbo)
RF_API void rf_gl_disable_render_texture(void); // Disable render texture (fbo), return to default framebuffer
RF_API void rf_gl_enable_depth_test(void); // Enable depth test
RF_API void rf_gl_disable_depth_test(void); // Disable depth test
RF_API void rf_gl_enable_backface_culling(void); // Enable backface culling
RF_API void rf_gl_disable_backface_culling(void); // Disable backface culling
RF_API void rf_gl_enable_scissor_test(void); // Enable scissor test
RF_API void rf_gl_disable_scissor_test(void); // Disable scissor test
RF_API void rf_gl_scissor(int x, int y, int width, int height); // Scissor test
RF_API void rf_gl_enable_wire_mode(void); // Enable wire mode
RF_API void rf_gl_disable_wire_mode(void); // Disable wire mode
RF_API void rf_gl_delete_textures(unsigned int id); // Delete OpenGL texture from GPU
RF_API void rf_gl_delete_render_textures(rf_render_texture2d target); // Delete render textures (fbo) from GPU
RF_API void rf_gl_delete_shader(unsigned int id); // Delete OpenGL shader program from GPU
RF_API void rf_gl_delete_vertex_arrays(unsigned int id); // Unload vertex data (VAO) from GPU memory
RF_API void rf_gl_delete_buffers(unsigned int id); // Unload vertex data (VBO) from GPU memory
RF_API void rf_gl_clear_color(rf_byte r, rf_byte g, rf_byte b, rf_byte a); // Clear color buffer with color
RF_API void rf_gl_clear_screen_buffers(void); // Clear used screen buffers (color and depth)
RF_API void rf_gl_update_buffer(int bufferId, void* data, int dataSize); // Update GPU buffer with new data
RF_API unsigned int rf_gl_load_attrib_buffer(unsigned int vao_id, int shaderLoc, void* buffer, int size, bool dynamic); // Load a new attributes buffer

//------------------------------------------------------------------------------------
// Functions Declaration - rlgl functionality
//------------------------------------------------------------------------------------
RF_API void rf_gl_close(); // De-inititialize rlgl (buffers, shaders, textures)
RF_API void rf_gl_draw(); // Update and draw default internal buffers

RF_API bool rf_gl_check_buffer_limit(int vCount); // Check internal buffer overflow for a given number of vertex
RF_API void rf_gl_set_debug_marker(const char* text); // Set debug marker for analysis
RF_API void rf_gl_load_extensions(void* loader); // Load OpenGL extensions
RF_API rf_vector3 rf_gl_unproject(rf_vector3 source, rf_matrix proj, rf_matrix view); // Get world coordinates from screen coordinates

// Textures data management
RF_API unsigned int rf_gl_load_texture(void* data, int width, int height, int format, int mipmapCount); // Load texture in GPU
RF_API unsigned int rf_gl_load_texture_depth(int width, int height, int bits, bool useRenderBuffer); // Load depth texture/renderbuffer (to be attached to fbo)
RF_API unsigned int rf_gl_load_texture_cubemap(void* data, int size, int format); // Load texture cubemap
RF_API void rf_gl_update_texture(unsigned int id, int width, int height, int format, const void* data); // Update GPU texture with new data
RF_API void rf_gl_get_gl_texture_formats(int format, unsigned int* glInternalFormat, unsigned int* glFormat, unsigned int* glType); // Get OpenGL internal formats
RF_API void rf_gl_unload_texture(unsigned int id); // Unload texture from GPU memory

RF_API void rf_gl_generate_mipmaps(rf_texture2d* texture); // Generate mipmap data for selected texture
RF_API void* rf_gl_read_texture_pixels(rf_texture2d texture); // Read texture pixel data
RF_API unsigned char* rf_gl_read_screen_pixels(int width, int height); // Read screen pixel data (color buffer)

// Render texture management (fbo)
RF_API rf_render_texture2d rf_gl_load_render_texture(int width, int height, int format, int depthBits, bool useDepthTexture); // Load a render texture (with color and depth attachments)
RF_API void rf_gl_render_texture_attach(rf_render_texture target, unsigned int id, int attachType); // Attach texture/renderbuffer to an fbo
RF_API bool rf_gl_render_texture_complete(rf_render_texture target); // Verify render texture is complete

// Vertex data management
RF_API void rf_gl_load_mesh(rf_mesh* mesh, bool dynamic); // Upload vertex data into GPU and provided VAO/VBO ids
RF_API void rf_gl_update_mesh(rf_mesh mesh, int buffer, int num); // Update vertex or index data on GPU (upload new data to one buffer)
RF_API void rf_gl_update_mesh_at(rf_mesh mesh, int buffer, int num, int index); // Update vertex or index data on GPU, at index
RF_API void rf_gl_draw_mesh(rf_mesh mesh, rf_material material, rf_matrix transform); // Draw a 3d mesh with material and transform
RF_API void rf_gl_unload_mesh(rf_mesh mesh); // Unload mesh data from CPU and GPU
//endregion
#endif
//endregion

//region implementation

#if defined(RF_RENDERER_IMPL) && !defined(RF_RENDERER_IMPL_DEFINED)
#define RF_RENDERER_IMPL_DEFINED

#ifndef RF_ASSERT
    #define RF_ASSERT(condition) assert(condition)
#endif

#ifndef _rf_is_file_extension
    #define _rf_is_file_extension(filename, ext) ((strrchr(filename, '.') != NULL) && (strcmp(strrchr(filename, '.'), ext) == 0))
#endif

// Trace log type
#define RF_LOG_TRACE 0
#define RF_LOG_DEBUG 1
#define RF_LOG_INFO 2
#define RF_LOG_WARNING 3
#define RF_LOG_ERROR 4
#define RF_LOG_FATAL 5

#ifndef RF_LOG
#define RF_LOG(log_type, msg, ...)
#endif

// Allow custom memory allocators
#ifndef RF_MALLOC
#define RF_MALLOC(sz) malloc(sz)
#endif

#ifndef RF_FREE
#define RF_FREE(p) free(p)
#endif

rf_context* _rf_global_context_ptr;

//region implementation includes

#define RF_MATH_IMPL
#include "rayfork_math.h"

//For models
#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobj_loader_c.h" // OBJ/MTL file formats loading
#define CGLTF_IMPLEMENTATION
#include "cgltf.h" // glTF file format loading
#define PAR_SHAPES_IMPLEMENTATION
#include "par_shapes.h" // Shapes 3d parametric generation

//For textures
#define STBI_MALLOC RF_MALLOC
#define STBI_FREE RF_FREE
#define STBI_REALLOC(p,newsz) RF_MALLOC(newsz)

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // Required for: stbi_load_from_file()
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h" // Required for: ttf font rectangles packaging

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h" // Required for: ttf font data reading

//endregion

//region default time implementation

#ifndef RF_CUSTOM_TIME

//Windows only
#ifdef _WIN32

static long long int _rf_global_performance_counter_frequency;
static bool _rf_global_performance_counter_frequency_initialised;

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

// Returns elapsed time in seconds since InitWindow()
RF_API double rf_get_time(void)
{
    if (!_rf_global_performance_counter_frequency_initialised)
    {
        #ifdef _WINDOWS_
        RF_ASSERT(QueryPerformanceFrequency((LARGE_INTEGER*)&_rf_global_performance_counter_frequency) != false);
        #else
        RF_ASSERT(QueryPerformanceFrequency(&_rf_global_performance_counter_frequency) != false);
        #endif
        _rf_global_performance_counter_frequency_initialised = true;
    }

    long long int qpc_result = {0};
    #ifdef _WINDOWS_
    RF_ASSERT(QueryPerformanceCounter((LARGE_INTEGER*)&qpc_result) != false);
    #else
    RF_ASSERT(QueryPerformanceCounter(&qpc_result) != false);
    #endif
    return (double) qpc_result / (double) _rf_global_performance_counter_frequency;
}

RF_API void rf_wait(float duration)
{
    Sleep((int) duration);
}

#elif defined(__linux__)

#include <time.h>

//Source: http://man7.org/linux/man-pages/man2/clock_gettime.2.html
RF_API double rf_get_time(void)
{
    struct timespec result;

    RF_ASSERT(clock_gettime(CLOCK_MONOTONIC_RAW, &result) == 0);

    return (double) result.tv_sec;
}

RF_API void rf_wait(float duration)
{
    long milliseconds = (long) duration;
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

#elif defined(__MACH__)

#include <mach/mach_time.h>
#include <unistd.h>
static bool _rf_global_mach_time_initialized;
static uint64_t _rf_global_mach_time_start;
static double _rf_global_mach_time_seconds_factor;

RF_API double rf_get_time(void)
{
    uint64_t time;
    if (!_rf_global_mach_time_initialized)
    {
        mach_timebase_info_data_t timebase;
        mach_timebase_info(&timebase);
        _rf_global_mach_time_seconds_factor = 1e-9 * (double)timebase.numer / (double)timebase.denom;
        _rf_global_mach_time_start = mach_absolute_time();
        _rf_global_mach_time_initialized = true;
    }
    time = mach_absolute_time();
    return (double)(time - _rf_global_mach_time_start) * _rf_global_mach_time_seconds_factor;
}

RF_API void rf_wait(float duration)
{
    usleep(duration * 1000);
}

#endif

#endif
//endregion

//region default io implementation

#ifndef RF_CUSTOM_IO
// Files management functions
RF_API int rf_get_file_size(const char* filename)
{
    FILE* file = fopen(filename, "rb");

    fseek(file, 0L, SEEK_END);
    const int size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    fclose(file);

    return size;
}

RF_API void rf_load_file_into_buffer(const char* filename, uint8_t* buffer, int buffer_size)
{
    FILE* file = fopen(filename, "rb");
    RF_ASSERT(file != NULL);

    fseek(file, 0L, SEEK_END);
    const int file_size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    RF_ASSERT(file_size <= buffer_size);

    const size_t read_size = fread(buffer, sizeof(char), buffer_size, file);

    RF_ASSERT(ferror(file) == 0);
    RF_ASSERT(read_size == file_size);

    fclose(file);
}
#endif
//endregion

//region text

// REQUIRES: strcmp()
// Get pointer to extension for a filename string
RF_INTERNAL const char* _rf_get_file_extension(const char* fileName)
{
    const char* dot = strrchr(fileName, '.');

    if (!dot || dot == fileName) return NULL;

    return (dot + 1);
}

// String pointer reverse break: returns right-most occurrence of charset in s
RF_INTERNAL const char* _rf_strprbrk(const char* s, const char* charset)
{
    const char* latestMatch = NULL;
    for (; s = strpbrk(s, charset), s != NULL; latestMatch = s++) { }
    return latestMatch;
}

// Get directory for a given filePath
RF_INTERNAL const char* _rf_get_directory_path(const char* filePath)
{
    #define rf_max_filepath_length 512 // Use common value
    const char* lastSlash = NULL;
    RF_INTERNAL char dirPath[rf_max_filepath_length];
    memset(dirPath, 0, rf_max_filepath_length);

    lastSlash = _rf_strprbrk(filePath, "\\/");
    if (!lastSlash) return NULL;

    // NOTE: Be careful, strncpy() is not safe, it does not care about '\0'
    strncpy(dirPath, filePath, strlen(filePath) - (strlen(lastSlash) - 1));
    dirPath[strlen(filePath) - strlen(lastSlash)] = '\0'; // Add '\0' manually

    return dirPath;
}

// Get pointer to filename for a path string
RF_INTERNAL const char* _rf_get_file_name(const char* filePath)
{
    const char* fileName = NULL;
    if (filePath != NULL) fileName = _rf_strprbrk(filePath, "\\/");

    if (!fileName || (fileName == filePath)) return filePath;

    return fileName + 1;
}

// Get filename string without extension (uses static string)
RF_INTERNAL const char* _rf_get_file_name_without_ext(const char* filePath)
{
    #define rf_max_filenamewithoutext_length 128

    RF_INTERNAL char fileName[rf_max_filenamewithoutext_length];
    memset(fileName, 0, rf_max_filenamewithoutext_length);

    if (filePath != NULL) strcpy(fileName, _rf_get_file_name(filePath)); // Get filename with extension

    int len = strlen(fileName);

    for (int i = 0; (i < len) && (i < rf_max_filenamewithoutext_length); i++)
    {
        if (fileName[i] == '.')
        {
            // NOTE: We break on first '.' found
            fileName[i] = '\0';
            break;
        }
    }

    return fileName;
}

// Split string into multiple strings
RF_INTERNAL const char** _rf_text_split(const char* text, char delimiter, int* count)
{
    #define rf_textsplit_max_substrings_count 128
    #define rf_textsplit_max_text_buffer_length 1024
    #define rf_max_text_buffer_length 1024
    // NOTE: Current implementation returns a copy of the provided string with '\0' (string end delimiter)
    // inserted between strings defined by "delimiter" parameter. No memory is dynamically allocated,
    // all used memory is static... it has some limitations:
    //      1. Maximum number of possible split strings is set by rf_textsplit_max_substrings_count
    //      2. Maximum size of text to split is rf_textsplit_max_text_buffer_length

    RF_INTERNAL const char* result[rf_textsplit_max_substrings_count] = { NULL };
    RF_INTERNAL char buffer[rf_textsplit_max_text_buffer_length] = { 0 };
    memset(buffer, 0, rf_textsplit_max_text_buffer_length);

    result[0] = buffer;
    int counter = 0;

    if (text != NULL)
    {
        counter = 1;

        // Count how many substrings we have on text and point to every one
        for (int i = 0; i < rf_max_text_buffer_length; i++)
        {
            buffer[i] = text[i];
            if (buffer[i] == '\0') break;
            else if (buffer[i] == delimiter)
            {
                buffer[i] = '\0'; // Set an end of string at this point
                result[counter] = buffer + i + 1;
                counter++;

                if (counter == rf_textsplit_max_substrings_count) break;
            }
        }
    }

    *count = counter;
    return result;
}

// Returns next codepoint in a UTF8 encoded text, scanning until '\0' is found
// When a invalid UTF8 rf_byte is encountered we exit as soon as possible and a '?'(0x3f) codepoint is returned
// Total number of bytes processed are returned as a parameter
// NOTE: the standard says U+FFFD should be returned in case of errors
// but that character is not supported by the default font in raylib
// TODO: optimize this code for speed!!
RF_INTERNAL int _rf_get_next_utf8_codepoint(const char* text, int* bytesProcessed)
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

    // NOTE: on decode errors we return as soon as possible
    int code = 0x3f; // Codepoint (defaults to '?')
    int octet = (unsigned char)(text[0]); // The first UTF8 octet
    *bytesProcessed = 1;

    if (octet <= 0x7f)
    {
        // Only one octet (ASCII range x00-7F)
        code = text[0];
    }
    else if ((octet & 0xe0) == 0xc0)
    {
        // Two octets
        // [0]xC2-DF    [1]UTF8-tail(x80-BF)
        unsigned char octet1 = text[1];

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *bytesProcessed = 2; return code; } // Unexpected sequence

        if ((octet >= 0xc2) && (octet <= 0xdf))
        {
            code = ((octet & 0x1f) << 6) | (octet1 & 0x3f);
            *bytesProcessed = 2;
        }
    }
    else if ((octet & 0xf0) == 0xe0)
    {
        // Three octets
        unsigned char octet1 = text[1];
        unsigned char octet2 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *bytesProcessed = 2; return code; } // Unexpected sequence

        octet2 = text[2];

        if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { *bytesProcessed = 3; return code; } // Unexpected sequence

        /*

            [0]xE0    [1]xA0-BF       [2]UTF8-tail(x80-BF)

            [0]xE1-EC [1]UTF8-tail    [2]UTF8-tail(x80-BF)

            [0]xED    [1]x80-9F       [2]UTF8-tail(x80-BF)

            [0]xEE-EF [1]UTF8-tail    [2]UTF8-tail(x80-BF)

        */
        if (((octet == 0xe0) && !((octet1 >= 0xa0) && (octet1 <= 0xbf))) ||
            ((octet == 0xed) && !((octet1 >= 0x80) && (octet1 <= 0x9f)))) { *bytesProcessed = 2; return code; }

        if ((octet >= 0xe0) && (0 <= 0xef))
        {
            code = ((octet & 0xf) << 12) | ((octet1 & 0x3f) << 6) | (octet2 & 0x3f);
            *bytesProcessed = 3;
        }
    }
    else if ((octet & 0xf8) == 0xf0)
    {
        // Four octets
        if (octet > 0xf4) return code;

        unsigned char octet1 = text[1];
        unsigned char octet2 = '\0';
        unsigned char octet3 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *bytesProcessed = 2; return code; } // Unexpected sequence

        octet2 = text[2];

        if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { *bytesProcessed = 3; return code; } // Unexpected sequence

        octet3 = text[3];

        if ((octet3 == '\0') || ((octet3 >> 6) != 2)) { *bytesProcessed = 4; return code; } // Unexpected sequence

        /*

            [0]xF0       [1]x90-BF       [2]UTF8-tail  [3]UTF8-tail

            [0]xF1-F3    [1]UTF8-tail    [2]UTF8-tail  [3]UTF8-tail

            [0]xF4       [1]x80-8F       [2]UTF8-tail  [3]UTF8-tail

        */
        if (((octet == 0xf0) && !((octet1 >= 0x90) && (octet1 <= 0xbf))) ||
            ((octet == 0xf4) && !((octet1 >= 0x80) && (octet1 <= 0x8f)))) { *bytesProcessed = 2; return code; } // Unexpected sequence

        if (octet >= 0xf0)
        {
            code = ((octet & 0x7) << 18) | ((octet1 & 0x3f) << 12) | ((octet2 & 0x3f) << 6) | (octet3 & 0x3f);
            *bytesProcessed = 4;
        }
    }

    if (code > 0x10ffff) code = 0x3f; // Codepoints after U+10ffff are invalid

    return code;
}
//endregion

//region font

// Load raylib default font
RF_API void rf_load_font_default()
{
    #define rf_bit_check(a,b) ((a) & (1u << (b)))

    // NOTE: Using UTF8 encoding table for Unicode U+0000..U+00FF Basic Latin + Latin-1 Supplement
    // http://www.utf8-chartable.de/unicode-utf8-table.pl

    _rf_global_context_ptr->default_font.chars_count = 224; // Number of chars included in our default font

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

    int charsHeight = 10;
    int charsDivisor = 1; // Every char is separated from the consecutive by a 1 pixel divisor, horizontally and vertically

    int charsWidth[224] = { 3, 1, 4, 6, 5, 7, 6, 2, 3, 3, 5, 5, 2, 4, 1, 7, 5, 2, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1, 3, 4, 3, 6,
            7, 6, 6, 6, 6, 6, 6, 6, 6, 3, 5, 6, 5, 7, 6, 6, 6, 6, 6, 6, 7, 6, 7, 7, 6, 6, 6, 2, 7, 2, 3, 5,
            2, 5, 5, 5, 5, 5, 4, 5, 5, 1, 2, 5, 2, 5, 5, 5, 5, 5, 5, 5, 4, 5, 5, 5, 5, 5, 5, 3, 1, 3, 4, 4,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 5, 5, 5, 7, 1, 5, 3, 7, 3, 5, 4, 1, 7, 4, 3, 5, 3, 3, 2, 5, 6, 1, 2, 2, 3, 5, 6, 6, 6, 6,
            6, 6, 6, 6, 6, 6, 7, 6, 6, 6, 6, 6, 3, 3, 3, 3, 7, 6, 6, 6, 6, 6, 6, 5, 6, 6, 6, 6, 6, 6, 4, 6,
            5, 5, 5, 5, 5, 5, 9, 5, 5, 5, 5, 5, 2, 2, 3, 3, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 3, 5 };

    // Re-construct image from _rf_global_context_ptr->default_font_data and generate OpenGL texture
    //----------------------------------------------------------------------
    int imWidth = 128;
    int imHeight = 128;

    rf_color* imagePixels = (rf_color*) RF_MALLOC(imWidth * imHeight * sizeof(rf_color));

    for (int i = 0; i < imWidth*imHeight; i++) imagePixels[i] = rf_blank; // Initialize array

    int counter = 0; // rf_font data elements counter

    // Fill imgData with _rf_global_context_ptr->default_font_data (convert from bit to pixel!)
    for (int i = 0; i < imWidth*imHeight; i += 32)
    {
        for (int j = 31; j >= 0; j--)
        {
            if (rf_bit_check(default_font_data[counter], j)) imagePixels[i+j] = rf_white;
        }

        counter++;

        if (counter > 512) counter = 0; // Security check...
    }

    rf_image imFont = rf_load_image_ex(imagePixels, imWidth, imHeight);
    rf_image_format(&imFont, rf_uncompressed_gray_alpha);

    RF_FREE(imagePixels);

    _rf_global_context_ptr->default_font.texture = rf_load_texture_from_image(imFont);

    // Reconstruct charSet using charsWidth[], charsHeight, charsDivisor, chars_count
    //------------------------------------------------------------------------------

    // Allocate space for our characters info data
    // NOTE: This memory should be freed at end! --> CloseWindow()
    _rf_global_context_ptr->default_font.chars = (rf_char_info* )RF_MALLOC(_rf_global_context_ptr->default_font.chars_count*sizeof(rf_char_info));
    _rf_global_context_ptr->default_font.recs = (rf_rectangle *)RF_MALLOC(_rf_global_context_ptr->default_font.chars_count*sizeof(rf_rectangle));

    int currentLine = 0;
    int currentPosX = charsDivisor;
    int testPosX = charsDivisor;

    for (int i = 0; i < _rf_global_context_ptr->default_font.chars_count; i++)
    {
        _rf_global_context_ptr->default_font.chars[i].value = 32 + i; // First char is 32

        _rf_global_context_ptr->default_font.recs[i].x = (float)currentPosX;
        _rf_global_context_ptr->default_font.recs[i].y = (float)(charsDivisor + currentLine*(charsHeight + charsDivisor));
        _rf_global_context_ptr->default_font.recs[i].width = (float)charsWidth[i];
        _rf_global_context_ptr->default_font.recs[i].height = (float)charsHeight;

        testPosX += (int)(_rf_global_context_ptr->default_font.recs[i].width + (float)charsDivisor);

        if (testPosX >= _rf_global_context_ptr->default_font.texture.width)
        {
            currentLine++;
            currentPosX = 2*charsDivisor + charsWidth[i];
            testPosX = currentPosX;

            _rf_global_context_ptr->default_font.recs[i].x = (float)charsDivisor;
            _rf_global_context_ptr->default_font.recs[i].y = (float)(charsDivisor + currentLine*(charsHeight + charsDivisor));
        }
        else currentPosX = testPosX;

        // NOTE: On default font character offsets and xAdvance are not required
        _rf_global_context_ptr->default_font.chars[i].offset_x = 0;
        _rf_global_context_ptr->default_font.chars[i].offset_y = 0;
        _rf_global_context_ptr->default_font.chars[i].advance_x = 0;

        // Fill character image data from fontClear data
        _rf_global_context_ptr->default_font.chars[i].image = rf_image_from_image(imFont, _rf_global_context_ptr->default_font.recs[i]);
    }

    rf_unload_image(imFont);

    _rf_global_context_ptr->default_font.base_size = (int)_rf_global_context_ptr->default_font.recs[0].height;

    RF_LOG(RF_LOG_INFO, "[TEX ID %i] Default font loaded successfully", _rf_global_context_ptr->default_font.texture.id);
}

// Unload raylib default font
RF_API void rf_unload_font_default()
{
    for (int i = 0; i < _rf_global_context_ptr->default_font.chars_count; i++) rf_unload_image(_rf_global_context_ptr->default_font.chars[i].image);
    rf_unload_texture(_rf_global_context_ptr->default_font.texture);
    RF_FREE(_rf_global_context_ptr->default_font.chars);
    RF_FREE(_rf_global_context_ptr->default_font.recs);
}


// Get the default font, useful to be used with extended parameters
RF_API rf_font rf_get_font_default()
{
    return _rf_global_context_ptr->default_font;
}

// Load rf_font from file into GPU memory (VRAM)
RF_API rf_font rf_load_font(const char* fileName)
{
    // Default hardcoded values for ttf file loading
#define rf_default_ttf_fontsize 32 // rf_font first character (32 - space)
#define rf_default_ttf_numchars 95 // ASCII 32..126 is 95 glyphs
#define rf_default_first_char 32 // Expected first char for image sprite font

    rf_font font = { 0 };

    if (_rf_is_file_extension(fileName, ".ttf") || _rf_is_file_extension(fileName, ".otf")) font = rf_load_font_ex(fileName, rf_default_ttf_fontsize, NULL, rf_default_ttf_numchars);
    else
    {
        rf_image image = rf_load_image(fileName);
        if (image.data != NULL) font = rf_load_font_from_image(image, rf_magenta, rf_default_first_char);
        rf_unload_image(image);
    }

    if (font.texture.id == 0)
    {
        RF_LOG(RF_LOG_WARNING, "[%s] rf_font could not be loaded, using default font", fileName);
        font = rf_get_font_default();
    }
    else rf_set_texture_filter(font.texture, rf_filter_point); // By default we set point filter (best performance)

    return font;
}

// Load rf_font from TTF font file with generation parameters
// NOTE: You can pass an array with desired characters, those characters should be available in the font
// if array is NULL, default char set is selected 32..126
RF_API rf_font rf_load_font_ex(const char* fileName, int fontSize, int* fontChars, int chars_count)
{
    rf_font font = { 0 };

    font.base_size = fontSize;
    font.chars_count = (chars_count > 0)? chars_count : 95;
    font.chars = rf_load_font_data(fileName, font.base_size, fontChars, font.chars_count, rf_font_default);

    RF_ASSERT(font.chars != NULL);

    rf_image atlas = rf_gen_image_font_atlas(font.chars, &font.recs, font.chars_count, font.base_size, 2, 0);
    font.texture = rf_load_texture_from_image(atlas);

    // Update chars[i].image to use alpha, required to be used on rf_image_draw_text()
    for (int i = 0; i < font.chars_count; i++)
    {
        rf_unload_image(font.chars[i].image);
        font.chars[i].image = rf_image_from_image(atlas, font.recs[i]);
    }

    rf_unload_image(atlas);

    return font;
}

//Note: Must call rf_finish_load_font_thread_safe on the gl thread afterwards to finish loading the font
RF_API rf_load_font_async_result rf_load_font_async(const char* fileName, int fontSize, int* fontChars, int chars_count)
{
    rf_font font = { 0 };

    font.base_size = fontSize;
    font.chars_count = (chars_count > 0)? chars_count : 95;
    font.chars = rf_load_font_data(fileName, font.base_size, fontChars, font.chars_count, rf_font_default);

    RF_ASSERT(font.chars != NULL);

    rf_image atlas = rf_gen_image_font_atlas(font.chars, &font.recs, font.chars_count, font.base_size, 2, 0);

    // Update chars[i].image to use alpha, required to be used on rf_image_draw_text()
    for (int i = 0; i < font.chars_count; i++)
    {
        rf_unload_image(font.chars[i].image);
        font.chars[i].image = rf_image_from_image(atlas, font.recs[i]);
    }

    return RF_CLITERAL(rf_load_font_async_result) { font, atlas };
}

RF_API rf_font rf_finish_load_font_async(rf_load_font_async_result fontJobResult)
{
    fontJobResult.font.texture = rf_load_texture_from_image(fontJobResult.atlas);
    rf_unload_image(fontJobResult.atlas);

    return fontJobResult.font;
}

// Load an rf_image font file (XNA style)
RF_API rf_font rf_load_font_from_image(rf_image image, rf_color key, int firstChar)
{
#define rf_color_equal(col1, col2) ((col1.r == col2.r)&&(col1.g == col2.g)&&(col1.b == col2.b)&&(col1.a == col2.a))

    int charSpacing = 0;
    int lineSpacing = 0;

    int x = 0;
    int y = 0;

    // Default number of characters supported
    #define rf_max_fontchars 256

    // We allocate a temporal arrays for chars data measures,
    // once we get the actual number of chars, we copy data to a sized arrays
    int tempCharValues[rf_max_fontchars];
    rf_rectangle tempCharRecs[rf_max_fontchars];

    rf_color* pixels = rf_get_image_data(image);

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
    for (int i = 0; i < image.height*image.width; i++) if (rf_color_equal(pixels[i], key)) pixels[i] = rf_blank;

    // Create a new image with the processed color data (key color replaced by rf_blank)
    rf_image fontClear = rf_load_image_ex(pixels, image.width, image.height);

    RF_FREE(pixels); // Free pixels array memory

    // Create spritefont with all data parsed from image
    rf_font spriteFont = { 0 };

    spriteFont.texture = rf_load_texture_from_image(fontClear); // Convert processed image to OpenGL texture
    spriteFont.chars_count = index;

    // We got tempCharValues and tempCharsRecs populated with chars data
    // Now we move temp data to sized charValues and charRecs arrays
    spriteFont.chars = (rf_char_info* )RF_MALLOC(spriteFont.chars_count*sizeof(rf_char_info));
    spriteFont.recs = (rf_rectangle *)RF_MALLOC(spriteFont.chars_count*sizeof(rf_rectangle));

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
        spriteFont.chars[i].image = rf_image_from_image(fontClear, tempCharRecs[i]);
    }

    rf_unload_image(fontClear); // Unload processed image once converted to texture

    spriteFont.base_size = (int)spriteFont.recs[0].height;

    RF_LOG(RF_LOG_INFO, "rf_image file loaded correctly as rf_font");

    return spriteFont;
}

#ifndef RF_NO_STB_TRUETYPE

// Load font data for further use
// NOTE: Requires TTF font and can generate SDF data
RF_API rf_char_info* rf_load_font_data(const char* fileName, int fontSize, int* fontChars, int chars_count, int type)
{
    // NOTE: Using some SDF generation default values,
    // trades off precision with ability to handle *smaller* sizes
    #define rf_sdf_char_padding 4
    #define rf_sdf_on_edge_value 128
    #define rf_sdf_pixel_dist_scale 64.0f
    #define rf_bitmap_alpha_threshold 80

    rf_char_info* chars = NULL;

    // Load font data (including pixel data) from TTF file
    // NOTE: Loaded information should be enough to generate font image atlas,
    // using any packaging method
    int size = rf_get_file_size(fileName); // Get file size

    unsigned char* fontBuffer = (unsigned char*) RF_MALLOC(size);

    rf_load_file_into_buffer(fileName, fontBuffer, size);

    // Init font for data reading
    stbtt_fontinfo fontInfo;
    if (!stbtt_InitFont(&fontInfo, fontBuffer, 0)) RF_LOG(RF_LOG_WARNING, "Failed to init font!");

    // Calculate font scale factor
    float scaleFactor = stbtt_ScaleForPixelHeight(&fontInfo, (float)fontSize);

    // Calculate font basic metrics
    // NOTE: ascent is equivalent to font baseline
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);

    // In case no chars count provided, default to 95
    chars_count = (chars_count > 0)? chars_count : 95;

    // Fill fontChars in case not provided externally
    // NOTE: By default we fill chars_count consecutevely, starting at 32 (Space)
    int genFontChars = false;
    if (fontChars == NULL)
    {
        fontChars = (int* )RF_MALLOC(chars_count*sizeof(int));
        for (int i = 0; i < chars_count; i++) fontChars[i] = i + 32;
        genFontChars = true;
    }

    chars = (rf_char_info* )RF_MALLOC(chars_count*sizeof(rf_char_info));

    // NOTE: Using simple packaging, one char after another
    for (int i = 0; i < chars_count; i++)
    {
        int chw = 0, chh = 0; // Character width and height (on generation)
        int ch = fontChars[i]; // Character value to get info for
        chars[i].value = ch;

        //  Render a unicode codepoint to a bitmap
        //      stbtt_GetCodepointBitmap()           -- allocates and returns a bitmap
        //      stbtt_GetCodepointBitmapBox()        -- how big the bitmap must be
        //      stbtt_MakeCodepointBitmap()          -- renders into bitmap you provide

        if (type != rf_font_sdf) chars[i].image.data = stbtt_GetCodepointBitmap(&fontInfo, scaleFactor, scaleFactor, ch, &chw, &chh, &chars[i].offset_x, &chars[i].offset_y);
        else if (ch != 32) chars[i].image.data = stbtt_GetCodepointSDF(&fontInfo, scaleFactor, ch, rf_sdf_char_padding, rf_sdf_on_edge_value, rf_sdf_pixel_dist_scale, &chw, &chh, &chars[i].offset_x, &chars[i].offset_y);
        else chars[i].image.data = NULL;

        if (type == rf_font_bitmap)
        {
            // Aliased bitmap (black & white) font generation, avoiding anti-aliasing
            // NOTE: For optimum results, bitmap font should be generated at base pixel size
            for (int p = 0; p < chw*chh; p++)
            {
                if (((unsigned char* )chars[i].image.data)[p] < rf_bitmap_alpha_threshold) ((unsigned char* )chars[i].image.data)[p] = 0;
                else ((unsigned char* )chars[i].image.data)[p] = 255;
            }
        }

        // Load characters images
        chars[i].image.width = chw;
        chars[i].image.height = chh;
        chars[i].image.mipmaps = 1;
        chars[i].image.format = rf_uncompressed_grayscale;

        chars[i].offset_y += (int)((float)ascent*scaleFactor);

        // Get bounding box for character (may be offset to account for chars that dip above or below the line)
        int chX1, chY1, chX2, chY2;
        stbtt_GetCodepointBitmapBox(&fontInfo, ch, scaleFactor, scaleFactor, &chX1, &chY1, &chX2, &chY2);

        RF_LOG(RF_LOG_DEBUG, "Character box measures: %i, %i, %i, %i", chX1, chY1, chX2 - chX1, chY2 - chY1);
        RF_LOG(RF_LOG_DEBUG, "Character offset_y: %i", (int)((float)ascent*scaleFactor) + chY1);

        stbtt_GetCodepointHMetrics(&fontInfo, ch, &chars[i].advance_x, NULL);
        chars[i].advance_x *= scaleFactor;
    }

    RF_FREE(fontBuffer);
    if (genFontChars) RF_FREE(fontChars);

    return chars;
}

#endif

// Generate image font atlas using chars info
// NOTE: Packing method: 0-Default, 1-Skyline
RF_API rf_image rf_gen_image_font_atlas(const rf_char_info* chars, rf_rectangle** charRecs, int chars_count, int fontSize, int padding, int packMethod)
{
    rf_image atlas = { 0 };

    *charRecs = NULL;

    // In case no chars count provided we suppose default of 95
    chars_count = (chars_count > 0)? chars_count : 95;

    // NOTE: Rectangles memory is loaded here!
    rf_rectangle *recs = (rf_rectangle *)RF_MALLOC(chars_count*sizeof(rf_rectangle));

    // Calculate image size based on required pixel area
    // NOTE 1: rf_image is forced to be squared and POT... very conservative!
    // NOTE 2: SDF font characters already contain an internal padding,
    // so image size would result bigger than default font type
    float requiredArea = 0;
    for (int i = 0; i < chars_count; i++) requiredArea += ((chars[i].image.width + 2*padding)*(chars[i].image.height + 2*padding));
    float guessSize = sqrtf(requiredArea)*1.3f;
    int imageSize = (int)powf(2, ceilf(logf((float)guessSize)/logf(2))); // Calculate next POT

    atlas.width = imageSize; // Atlas bitmap width
    atlas.height = imageSize; // Atlas bitmap height
    atlas.data = (unsigned char*) RF_MALLOC(atlas.width * atlas.height); // Create a bitmap to store characters (8 bpp)
    memset(atlas.data, 0, atlas.width * atlas.height);
    atlas.format = rf_uncompressed_grayscale;
    atlas.mipmaps = 1;

    // DEBUG: We can see padding in the generated image setting a gray background...
    //for (int i = 0; i < atlas.width*atlas.height; i++) ((unsigned char* )atlas.data)[i] = 100;

    if (packMethod == 0) // Use basic packing algorythm
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
                // height is bigger than fontSize, it could be up to (fontSize + 8)
                offset_y += (fontSize + 2*padding);

                if (offset_y > (atlas.height - fontSize - padding)) break;
            }
        }
    }
    else if (packMethod == 1) // Use Skyline rect packing algorythm (stb_pack_rect)
    {
        RF_LOG(RF_LOG_DEBUG, "Using Skyline packing algorythm!");

        stbrp_context *context = (stbrp_context *)RF_MALLOC(sizeof(*context));
        stbrp_node *nodes = (stbrp_node *)RF_MALLOC(chars_count*sizeof(*nodes));

        stbrp_init_target(context, atlas.width, atlas.height, nodes, chars_count);
        stbrp_rect *rects = (stbrp_rect *)RF_MALLOC(chars_count*sizeof(stbrp_rect));

        // Fill rectangles for packaging
        for (int i = 0; i < chars_count; i++)
        {
            rects[i].id = i;
            rects[i].w = chars[i].image.width + 2*padding;
            rects[i].h = chars[i].image.height + 2*padding;
        }

        // Package rectangles into atlas
        stbrp_pack_rects(context, rects, chars_count);

        for (int i = 0; i < chars_count; i++)
        {
            // It return char rectangles in atlas
            recs[i].x = rects[i].x + (float)padding;
            recs[i].y = rects[i].y + (float)padding;
            recs[i].width = (float)chars[i].image.width;
            recs[i].height = (float)chars[i].image.height;

            if (rects[i].was_packed)
            {
                // Copy pixel data from fc.data to atlas
                for (int y = 0; y < chars[i].image.height; y++)
                {
                    for (int x = 0; x < chars[i].image.width; x++)
                    {
                        ((unsigned char* )atlas.data)[(rects[i].y + padding + y)*atlas.width + (rects[i].x + padding + x)] = ((unsigned char* )chars[i].image.data)[y*chars[i].image.width + x];
                    }
                }
            }
            else RF_LOG(RF_LOG_WARNING, "Character could not be packed: %i", i);
        }

        RF_FREE(rects);
        RF_FREE(nodes);
        RF_FREE(context);
    }

    // TODO: Crop image if required for smaller size

    // Convert image data from GRAYSCALE to GRAY_ALPHA
    // WARNING: rf_image_alpha_mask(&atlas, atlas) does not work in this case, requires manual operation
    unsigned char* dataGrayAlpha = (unsigned char* )RF_MALLOC(atlas.width*atlas.height*sizeof(unsigned char)*2); // Two channels

    for (int i = 0, k = 0; i < atlas.width*atlas.height; i++, k += 2)
    {
        dataGrayAlpha[k] = 255;
        dataGrayAlpha[k + 1] = ((unsigned char* )atlas.data)[i];
    }

    RF_FREE(atlas.data);
    atlas.data = dataGrayAlpha;
    atlas.format = rf_uncompressed_gray_alpha;

    *charRecs = recs;

    return atlas;
}


// Unload rf_font from GPU memory (VRAM)
RF_API void rf_unload_font(rf_font font)
{
    // NOTE: Make sure spriteFont is not default font (fallback)
    if (font.texture.id != rf_get_font_default().texture.id)
    {
        for (int i = 0; i < font.chars_count; i++) rf_unload_image(font.chars[i].image);

        rf_unload_texture(font.texture);
        RF_FREE(font.chars);
        RF_FREE(font.recs);

        RF_LOG(RF_LOG_DEBUG, "Unloaded sprite font data");
    }
}

// Shows current FPS on top-left corner
// NOTE: Uses default font
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
    rf_draw_text(buff, posX, posY, 20, rf_lime);
}

// Draw text (using default font)
// NOTE: fontSize work like in any drawing program but if fontSize is lower than font-base-size, then font-base-size is used
// NOTE: chars spacing is proportional to fontSize
RF_API void rf_draw_text(const char* text, int posX, int posY, int fontSize, rf_color color)
{
    // Check if default font has been loaded
    if (_rf_global_context_ptr->default_font.texture.id != 0)
    {
        rf_vector2 position = { (float)posX, (float)posY };

        int size = 10; // Default rf_font chars height in pixel
        if (fontSize < size) fontSize = size;
        int spacing = fontSize/size;

        rf_draw_text_ex(_rf_global_context_ptr->default_font, text, position, (float)fontSize, (float)spacing, color);
    }
}

// Draw text using rf_font
// NOTE: chars spacing is NOT proportional to fontSize
RF_API void rf_draw_text_ex(rf_font font, const char* text, rf_vector2 position, float fontSize, float spacing, rf_color tint)
{
    int length = strlen(text);
    int textOffsetY = 0; // Required for line break!
    float textOffsetX = 0.0f; // Offset between characters
    float scaleFactor = 0.0f;

    int letter = 0; // Current character
    int index = 0; // Index position in sprite font

    scaleFactor = fontSize/font.base_size;

    for (int i = 0; i < length; i++)
    {
        int next = 0;
        letter = _rf_get_next_utf8_codepoint(&text[i], &next);
        index = rf_get_glyph_index(font, letter);

        // NOTE: Normally we exit the decoding sequence as soon as a bad rf_byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol so to not skip any we set 'next = 1'
        if (letter == 0x3f) next = 1;
        i += (next - 1);

        if (letter == '\n')
        {
            // NOTE: Fixed line spacing of 1.5 lines
            textOffsetY += (int)((font.base_size + font.base_size/2)*scaleFactor);
            textOffsetX = 0.0f;
        }
        else
        {
            if (letter != ' ')
            {
                rf_draw_texture_pro(font.texture, font.recs[index],
                                    RF_CLITERAL(rf_rectangle){ position.x + textOffsetX + font.chars[index].offset_x*scaleFactor,
                                            position.y + textOffsetY + font.chars[index].offset_y*scaleFactor,
                                            font.recs[index].width*scaleFactor,
                                            font.recs[index].height*scaleFactor }, RF_CLITERAL(rf_vector2){ 0, 0 }, 0.0f, tint);
            }

            if (font.chars[index].advance_x == 0) textOffsetX += ((float)font.recs[index].width*scaleFactor + spacing);
            else textOffsetX += ((float)font.chars[index].advance_x*scaleFactor + spacing);
        }
    }
}

RF_API void rf_draw_text_from_buffer(rf_font font, const char* text, int length, rf_vector2 position, float fontSize, float spacing, rf_color tint)
{
    int textOffsetY = 0; // Required for line break!
    float textOffsetX = 0.0f; // Offset between characters
    float scaleFactor = 0.0f;

    int letter = 0; // Current character
    int index = 0; // Index position in sprite font

    scaleFactor = fontSize/font.base_size;

    for (int i = 0; i < length; i++)
    {
        int next = 0;
        letter = _rf_get_next_utf8_codepoint(&text[i], &next);
        index = rf_get_glyph_index(font, letter);

        // NOTE: Normally we exit the decoding sequence as soon as a bad rf_byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol so to not skip any we set 'next = 1'
        if (letter == 0x3f) next = 1;
        i += (next - 1);

        if (letter == '\n')
        {
            // NOTE: Fixed line spacing of 1.5 lines
            textOffsetY += (int)((font.base_size + font.base_size/2)*scaleFactor);
            textOffsetX = 0.0f;
        }
        else
        {
            if (letter != ' ')
            {
                rf_draw_texture_pro(font.texture, font.recs[index],
                                    RF_CLITERAL(rf_rectangle){ position.x + textOffsetX + font.chars[index].offset_x*scaleFactor,
                                            position.y + textOffsetY + font.chars[index].offset_y*scaleFactor,
                                            font.recs[index].width*scaleFactor,
                                            font.recs[index].height*scaleFactor }, RF_CLITERAL(rf_vector2){ 0, 0 }, 0.0f, tint);
            }

            if (font.chars[index].advance_x == 0) textOffsetX += ((float)font.recs[index].width*scaleFactor + spacing);
            else textOffsetX += ((float)font.chars[index].advance_x*scaleFactor + spacing);
        }
    }
}

// Draw text using font inside rectangle limits
RF_API void rf_draw_text_rec(rf_font font, const char* text, rf_rectangle rec, float fontSize, float spacing, bool wordWrap, rf_color tint)
{
    rf_draw_text_rec_ex(font, text, rec, fontSize, spacing, wordWrap, tint, 0, 0, rf_white, rf_white);
}

// Draw text using font inside rectangle limits with support for text selection
RF_API void rf_draw_text_rec_ex(rf_font font, const char* text, rf_rectangle rec, float fontSize, float spacing, bool wordWrap, rf_color tint, int selectStart, int selectLength, rf_color selectText, rf_color selectBack)
{
    int length = strlen(text);
    int textOffsetX = 0; // Offset between characters
    int textOffsetY = 0; // Required for line break!
    float scaleFactor = 0.0f;

    int letter = 0; // Current character
    int index = 0; // Index position in sprite font

    scaleFactor = fontSize/font.base_size;

    enum
    {
        MEASURE_STATE = 0,
        DRAW_STATE = 1
    };
    int state = wordWrap? MEASURE_STATE : DRAW_STATE;
    int startLine = -1; // Index where to begin drawing (where a line begins)
    int endLine = -1; // Index where to stop drawing (where a line ends)
    int lastk = -1; // Holds last value of the character position

    for (int i = 0, k = 0; i < length; i++, k++)
    {
        int glyphWidth = 0;
        int next = 0;
        letter = _rf_get_next_utf8_codepoint(&text[i], &next);
        index = rf_get_glyph_index(font, letter);

        // NOTE: normally we exit the decoding sequence as soon as a bad rf_byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol so to not skip any we set next = 1
        if (letter == 0x3f) next = 1;
        i += next - 1;

        if (letter != '\n')
        {
            glyphWidth = (font.chars[index].advance_x == 0)?
                         (int)(font.recs[index].width*scaleFactor + spacing):
                         (int)(font.chars[index].advance_x*scaleFactor + spacing);
        }

        // NOTE: When wordWrap is ON we first measure how much of the text we can draw before going outside of the rec container
        // We store this info in startLine and endLine, then we change states, draw the text between those two variables
        // and change states again and again recursively until the end of the text (or until we get outside of the container).
        // When wordWrap is OFF we don't need the measure state so we go to the drawing state immediately
        // and begin drawing on the next line before we can get outside the container.
        if (state == MEASURE_STATE)
        {
            // TODO: there are multiple types of spaces in UNICODE, maybe it's a good idea to add support for more
            // See: http://jkorpela.fi/chars/spaces.html
            if ((letter == ' ') || (letter == '\t') || (letter == '\n')) endLine = i;

            if ((textOffsetX + glyphWidth + 1) >= rec.width)
            {
                endLine = (endLine < 1)? i : endLine;
                if (i == endLine) endLine -= next;
                if ((startLine + next) == endLine) endLine = i - next;
                state = !state;
            }
            else if ((i + 1) == length)
            {
                endLine = i;
                state = !state;
            }
            else if (letter == '\n')
            {
                state = !state;
            }

            if (state == DRAW_STATE)
            {
                textOffsetX = 0;
                i = startLine;
                glyphWidth = 0;

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
                if (!wordWrap)
                {
                    textOffsetY += (int)((font.base_size + font.base_size/2)*scaleFactor);
                    textOffsetX = 0;
                }
            }
            else
            {
                if (!wordWrap && ((textOffsetX + glyphWidth + 1) >= rec.width))
                {
                    textOffsetY += (int)((font.base_size + font.base_size/2)*scaleFactor);
                    textOffsetX = 0;
                }

                if ((textOffsetY + (int)(font.base_size*scaleFactor)) > rec.height) break;

                // Draw selected
                bool isGlyphSelected = false;
                if ((selectStart >= 0) && (k >= selectStart) && (k < (selectStart + selectLength)))
                {
                    rf_rectangle strec = {rec.x + textOffsetX-1, rec.y + textOffsetY, glyphWidth, font.base_size*scaleFactor };
                    rf_draw_rectangle_rec(strec, selectBack);
                    isGlyphSelected = true;
                }

                // Draw glyph
                if ((letter != ' ') && (letter != '\t'))
                {
                    rf_draw_texture_pro(font.texture, font.recs[index],
                                        RF_CLITERAL(rf_rectangle){ rec.x + textOffsetX + font.chars[index].offset_x*scaleFactor,
                                                rec.y + textOffsetY + font.chars[index].offset_y*scaleFactor,
                                                font.recs[index].width*scaleFactor,
                                                font.recs[index].height*scaleFactor }, RF_CLITERAL(rf_vector2){ 0, 0 }, 0.0f,
                                        (!isGlyphSelected)? tint : selectText);
                }
            }

            if (wordWrap && (i == endLine))
            {
                textOffsetY += (int)((font.base_size + font.base_size/2)*scaleFactor);
                textOffsetX = 0;
                startLine = endLine;
                endLine = -1;
                glyphWidth = 0;
                k = lastk;
                state = !state;
            }
        }

        textOffsetX += glyphWidth;
    }
}

RF_API float rf_measure_height_of_text_in_container(rf_font font, float fontSize, const char* text, int length, float container_width)
{
    rf_vector2 unwrappedStringSize = rf_measure_text_from_buffer(font, text, length, fontSize, 1.0f);
    if (unwrappedStringSize.x <= container_width)
    {
        return unwrappedStringSize.y;
    }

    rf_rectangle rec = { 0, 0, container_width, FLT_MAX };
    int textOffsetX = 0; // Offset between characters
    int textOffsetY = 0; // Required for line break!
    float scaleFactor = 0.0f;
    float spacing = 1.0f;

    int letter = 0; // Current character
    int index = 0; // Index position in sprite font

    scaleFactor = fontSize/font.base_size;

    enum
    {
        MEASURE_STATE = 0,
        DRAW_STATE = 1
    };

    int state = MEASURE_STATE;
    int startLine = -1; // Index where to begin drawing (where a line begins)
    int endLine = -1; // Index where to stop drawing (where a line ends)
    int lastk = -1; // Holds last value of the character position

    for (int i = 0, k = 0; i < length; i++, k++)
    {
        int glyphWidth = 0;
        int next = 0;
        letter = _rf_get_next_utf8_codepoint(&text[i], &next);
        index = rf_get_glyph_index(font, letter);

        // NOTE: normally we exit the decoding sequence as soon as a bad rf_byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol so to not skip any we set next = 1
        if (letter == 0x3f) next = 1;
        i += next - 1;

        if (letter != '\n')
        {
            glyphWidth = (font.chars[index].advance_x == 0)?
                         (int)(font.recs[index].width*scaleFactor + spacing):
                         (int)(font.chars[index].advance_x*scaleFactor + spacing);
        }

        // NOTE: When wordWrap is ON we first measure how much of the text we can draw before going outside of the rec container
        // We store this info in startLine and endLine, then we change states, draw the text between those two variables
        // and change states again and again recursively until the end of the text (or until we get outside of the container).
        // When wordWrap is OFF we don't need the measure state so we go to the drawing state immediately
        // and begin drawing on the next line before we can get outside the container.
        if (state == MEASURE_STATE)
        {
            // TODO: there are multiple types of spaces in UNICODE, maybe it's a good idea to add support for more
            // See: http://jkorpela.fi/chars/spaces.html
            if ((letter == ' ') || (letter == '\t') || (letter == '\n')) endLine = i;

            if ((textOffsetX + glyphWidth + 1) >= rec.width)
            {
                endLine = (endLine < 1)? i : endLine;
                if (i == endLine) endLine -= next;
                if ((startLine + next) == endLine) endLine = i - next;
                state = !state;
            }
            else if ((i + 1) == length)
            {
                endLine = i;
                state = !state;
            }
            else if (letter == '\n')
            {
                state = !state;
            }

            if (state == DRAW_STATE)
            {
                textOffsetX = 0;
                i = startLine;
                glyphWidth = 0;

                // Save character position when we switch states
                int tmp = lastk;
                lastk = k - 1;
                k = tmp;
            }
        }
        else
        {
            if ((textOffsetY + (int)(font.base_size*scaleFactor)) > rec.height) break;

            if (i == endLine)
            {
                textOffsetY += (int)((font.base_size + font.base_size/2)*scaleFactor);
                textOffsetX = 0;
                startLine = endLine;
                endLine = -1;
                glyphWidth = 0;
                k = lastk;
                state = !state;
            }
        }

        textOffsetX += glyphWidth;
    }

    return textOffsetY;
}

// Measure string width for default font
RF_API int rf_measure_text(const char* text, int fontSize)
{
    rf_vector2 vec = { 0.0f, 0.0f };

    // Check if default font has been loaded
    if (_rf_global_context_ptr->default_font.texture.id != 0)
    {
        int size = 10; // Default rf_font chars height in pixel
        if (fontSize < size) fontSize = size;
        int spacing = fontSize/size;

        vec = rf_measure_text_ex(_rf_global_context_ptr->default_font, text, (float)fontSize, (float)spacing);
    }

    return (int)vec.x;
}

// Measure string size for rf_font
RF_API rf_vector2 rf_measure_text_ex(rf_font font, const char* text, float fontSize, float spacing)
{
    int len = strlen(text);
    int tempLen = 0; // Used to count longer text line num chars
    int lenCounter = 0;

    float textWidth = 0.0f;
    float tempTextWidth = 0.0f; // Used to count longer text line width

    float textHeight = (float)font.base_size;
    float scaleFactor = fontSize/(float)font.base_size;

    int letter = 0; // Current character
    int index = 0; // Index position in sprite font

    for (int i = 0; i < len; i++)
    {
        lenCounter++;

        int next = 0;
        letter = _rf_get_next_utf8_codepoint(&text[i], &next);
        index = rf_get_glyph_index(font, letter);

        // NOTE: normally we exit the decoding sequence as soon as a bad rf_byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol so to not skip any we set next = 1
        if (letter == 0x3f) next = 1;
        i += next - 1;

        if (letter != '\n')
        {
            if (font.chars[index].advance_x != 0) textWidth += font.chars[index].advance_x;
            else textWidth += (font.recs[index].width + font.chars[index].offset_x);
        }
        else
        {
            if (tempTextWidth < textWidth) tempTextWidth = textWidth;
            lenCounter = 0;
            textWidth = 0;
            textHeight += ((float)font.base_size*1.5f); // NOTE: Fixed line spacing of 1.5 lines
        }

        if (tempLen < lenCounter) tempLen = lenCounter;
    }

    if (tempTextWidth < textWidth) tempTextWidth = textWidth;

    rf_vector2 vec = { 0 };
    vec.x = tempTextWidth*scaleFactor + (float)((tempLen - 1)*spacing); // Adds chars spacing to measure
    vec.y = textHeight*scaleFactor;

    return vec;
}

// Returns index position for a unicode character on spritefont
RF_API int rf_get_glyph_index(rf_font font, int character)
{
    return (character - 32);
}

// Measure string size for rf_font
RF_API rf_vector2 rf_measure_text_from_buffer(rf_font font, const char* text, int len, float fontSize, float spacing)
{
    int tempLen = 0; // Used to count longer text line num chars
    int lenCounter = 0;

    float textWidth = 0.0f;
    float tempTextWidth = 0.0f; // Used to count longer text line width

    float textHeight = (float)font.base_size;
    float scaleFactor = fontSize/(float)font.base_size;

    int letter = 0; // Current character
    int index = 0; // Index position in sprite font

    for (int i = 0; i < len; i++)
    {
        lenCounter++;

        int next = 0;
        letter = _rf_get_next_utf8_codepoint(&text[i], &next);
        index = rf_get_glyph_index(font, letter);

        // NOTE: normally we exit the decoding sequence as soon as a bad rf_byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol so to not skip any we set next = 1
        if (letter == 0x3f) next = 1;
        i += next - 1;

        if (letter != '\n')
        {
            if (font.chars[index].advance_x != 0) textWidth += font.chars[index].advance_x;
            else textWidth += (font.recs[index].width + font.chars[index].offset_x);
        }
        else
        {
            if (tempTextWidth < textWidth) tempTextWidth = textWidth;
            lenCounter = 0;
            textWidth = 0;
            textHeight += ((float)font.base_size*1.5f); // NOTE: Fixed line spacing of 1.5 lines
        }

        if (tempLen < lenCounter) tempLen = lenCounter;
    }

    if (tempTextWidth < textWidth) tempTextWidth = textWidth;

    rf_vector2 vec = { 0 };
    vec.x = tempTextWidth*scaleFactor + (float)((tempLen - 1)*spacing); // Adds chars spacing to measure
    vec.y = textHeight*scaleFactor;

    return vec;
}
//endregion

//region core

RF_API void rf_set_global_context_ptr(rf_context* rf_ctx)
{
    _rf_global_context_ptr = rf_ctx;
}

// Set viewport for a provided width and height
RF_INTERNAL void _rf_setup_viewport(int width, int height)
{
    _rf_global_context_ptr->render_width = width;
    _rf_global_context_ptr->render_height = height;

    // Set viewport width and height
    // NOTE: We consider render size and offset in case black bars are required and
    // render area does not match full global_display area (this situation is only applicable on fullscreen mode)
    rf_gl_viewport(_rf_global_context_ptr->render_offset_x/2, _rf_global_context_ptr->render_offset_y/2, _rf_global_context_ptr->render_width - _rf_global_context_ptr->render_offset_x, _rf_global_context_ptr->render_height - _rf_global_context_ptr->render_offset_y);

    rf_matrix_mode(GL_PROJECTION); // Switch to PROJECTION matrix
    rf_load_identity(); // Reset current matrix (PROJECTION)

    // Set orthographic GL_PROJECTION to current framebuffer size
    // NOTE: Confirf_gl_projectiongured top-left corner as (0, 0)
    rf_ortho(0, _rf_global_context_ptr->render_width, _rf_global_context_ptr->render_height, 0, 0.0f, 1.0f);

    rf_matrix_mode(GL_MODELVIEW); // Switch back to MODELVIEW matrix
    rf_load_identity(); // Reset current matrix (MODELVIEW)
}

// Compute framebuffer size relative to screen size and global_display size
// NOTE: Global variables _rf_global_context_ptr->render_width/_rf_global_context_ptr->render_height and _rf_global_context_ptr->render_offset_x/_rf_global_context_ptr->render_offset_y can be modified
RF_INTERNAL void _rf_setup_frame_buffer(int width, int height)
{
    // Calculate _rf_global_context_ptr->render_width and _rf_global_context_ptr->render_height, we have the global_display size (input params) and the desired screen size (global var)
    if ((_rf_global_context_ptr->screen_width > _rf_global_context_ptr->display_width) || (_rf_global_context_ptr->screen_height > _rf_global_context_ptr->display_height))
    {
        RF_LOG(RF_LOG_WARNING, "DOWNSCALING: Required screen size (%ix%i) is bigger than global_display size (%ix%i)", _rf_global_context_ptr->screen_width, _rf_global_context_ptr->screen_height, _rf_global_context_ptr->display_width, _rf_global_context_ptr->display_height);

        // Downscaling to fit global_display with border-bars
        float widthRatio = (float)_rf_global_context_ptr->display_width/(float)_rf_global_context_ptr->screen_width;
        float heightRatio = (float)_rf_global_context_ptr->display_height/(float)_rf_global_context_ptr->screen_height;

        if (widthRatio <= heightRatio)
        {
            _rf_global_context_ptr->render_width = _rf_global_context_ptr->display_width;
            _rf_global_context_ptr->render_height = (int)round((float)_rf_global_context_ptr->screen_height*widthRatio);
            _rf_global_context_ptr->render_offset_x = 0;
            _rf_global_context_ptr->render_offset_y = (_rf_global_context_ptr->display_height - _rf_global_context_ptr->render_height);
        }
        else
        {
            _rf_global_context_ptr->render_width = (int)round((float)_rf_global_context_ptr->screen_width*heightRatio);
            _rf_global_context_ptr->render_height = _rf_global_context_ptr->display_height;
            _rf_global_context_ptr->render_offset_x = (_rf_global_context_ptr->display_width - _rf_global_context_ptr->render_width);
            _rf_global_context_ptr->render_offset_y = 0;
        }

        // Screen scaling required
        float scaleRatio = (float)_rf_global_context_ptr->render_width/(float)_rf_global_context_ptr->screen_width;
        _rf_global_context_ptr->screen_scaling = rf_matrix_scale(scaleRatio, scaleRatio, 1.0f);

        // NOTE: We render to full global_display resolution!
        // We just need to calculate above parameters for downscale matrix and offsets
        _rf_global_context_ptr->render_width = _rf_global_context_ptr->display_width;
        _rf_global_context_ptr->render_height = _rf_global_context_ptr->display_height;

        RF_LOG(RF_LOG_WARNING, "Downscale matrix generated, content will be rendered at: %i x %i", _rf_global_context_ptr->render_width, _rf_global_context_ptr->render_height);
    }
    else if ((_rf_global_context_ptr->screen_width < _rf_global_context_ptr->display_width) || (_rf_global_context_ptr->screen_height < _rf_global_context_ptr->display_height))
    {
        // Required screen size is smaller than global_display size
        RF_LOG(RF_LOG_INFO, "UPSCALING: Required screen size: %i x %i -> Display size: %i x %i", _rf_global_context_ptr->screen_width, _rf_global_context_ptr->screen_height, _rf_global_context_ptr->display_width, _rf_global_context_ptr->display_height);

        // Upscaling to fit global_display with border-bars
        float displayRatio = (float)_rf_global_context_ptr->display_width/(float)_rf_global_context_ptr->display_height;
        float screenRatio = (float)_rf_global_context_ptr->screen_width/(float)_rf_global_context_ptr->screen_height;

        if (displayRatio <= screenRatio)
        {
            _rf_global_context_ptr->render_width = _rf_global_context_ptr->screen_width;
            _rf_global_context_ptr->render_height = (int)round((float)_rf_global_context_ptr->screen_width/displayRatio);
            _rf_global_context_ptr->render_offset_x = 0;
            _rf_global_context_ptr->render_offset_y = (_rf_global_context_ptr->render_height - _rf_global_context_ptr->screen_height);
        }
        else
        {
            _rf_global_context_ptr->render_width = (int)round((float)_rf_global_context_ptr->screen_height*displayRatio);
            _rf_global_context_ptr->render_height = _rf_global_context_ptr->screen_height;
            _rf_global_context_ptr->render_offset_x = (_rf_global_context_ptr->render_width - _rf_global_context_ptr->screen_width);
            _rf_global_context_ptr->render_offset_y = 0;
        }
    }
    else // screen == global_display
    {
        _rf_global_context_ptr->render_width = _rf_global_context_ptr->screen_width;
        _rf_global_context_ptr->render_height = _rf_global_context_ptr->screen_height;
        _rf_global_context_ptr->render_offset_x = 0;
        _rf_global_context_ptr->render_offset_y = 0;
    }
}

// Set background color (framebuffer clear color)
RF_API void rf_clear_background(rf_color color)
{
    rf_gl_clear_color(color.r, color.g, color.b, color.a); // Set clear color
    rf_gl_clear_screen_buffers(); // Clear current framebuffers
}

// Setup canvas (framebuffer) to start drawing
RF_API void rf_begin_drawing()
{
    _rf_global_context_ptr->current_time = rf_get_time(); // Number of elapsed seconds since InitTimer()
    _rf_global_context_ptr->update_time = _rf_global_context_ptr->current_time - _rf_global_context_ptr->previous_time;
    _rf_global_context_ptr->previous_time = _rf_global_context_ptr->current_time;

    rf_load_identity(); // Reset current matrix (MODELVIEW)
    rf_mult_matrixf(rf_matrix_to_float(_rf_global_context_ptr->screen_scaling)); // Apply screen scaling

    //rf_translatef(0.375, 0.375, 0);    // HACK to have 2D pixel-perfect drawing on OpenGL 1.1
    // NOTE: Not required with OpenGL 3.3+
}

// End canvas drawing and swap buffers (double buffering)
RF_API void rf_end_drawing()
{
    rf_gl_draw(); // Draw Buffers (Only OpenGL 3+ and ES2)

    // Frame time control system
    _rf_global_context_ptr->current_time = rf_get_time();
    _rf_global_context_ptr->draw_time = _rf_global_context_ptr->current_time - _rf_global_context_ptr->previous_time;
    _rf_global_context_ptr->previous_time = _rf_global_context_ptr->current_time;

    _rf_global_context_ptr->frame_time = _rf_global_context_ptr->update_time + _rf_global_context_ptr->draw_time;

    // rf_wait for some milliseconds...
    if (_rf_global_context_ptr->frame_time < _rf_global_context_ptr->target_time)
    {
        rf_wait((float)(_rf_global_context_ptr->target_time - _rf_global_context_ptr->frame_time)*1000.0f);

        _rf_global_context_ptr->current_time = rf_get_time();
        double extraTime = _rf_global_context_ptr->current_time - _rf_global_context_ptr->previous_time;
        _rf_global_context_ptr->previous_time = _rf_global_context_ptr->current_time;

        _rf_global_context_ptr->frame_time += extraTime;
    }

    return;
}

// Initialize 2D mode with custom camera (2D)
RF_API void rf_begin_mode2d(rf_camera2d camera)
{
    rf_gl_draw(); // Draw Buffers (Only OpenGL 3+ and ES2)

    rf_load_identity(); // Reset current matrix (MODELVIEW)

    // Apply screen scaling if required
    rf_mult_matrixf(rf_matrix_to_float(_rf_global_context_ptr->screen_scaling));

    // Apply 2d camera transformation to rf_global_model_view
    rf_mult_matrixf(rf_matrix_to_float(rf_get_camera_matrix2d(camera)));
}

// Ends 2D mode with custom camera
RF_API void rf_end_mode2d()
{
    rf_gl_draw(); // Draw Buffers (Only OpenGL 3+ and ES2)

    rf_load_identity(); // Reset current matrix (MODELVIEW)
    rf_mult_matrixf(rf_matrix_to_float(_rf_global_context_ptr->screen_scaling)); // Apply screen scaling if required
}

// Initializes 3D mode with custom camera (3D)
RF_API void rf_begin_mode3d(rf_camera3d camera)
{
    rf_gl_draw(); // Draw Buffers (Only OpenGL 3+ and ES2)

    rf_matrix_mode(GL_PROJECTION); // Switch to GL_PROJECTION matrix
    rf_push_matrix(); // Save previous matrix, which contains the settings for the 2d ortho GL_PROJECTION
    rf_load_identity(); // Reset current matrix (PROJECTION)

    float aspect = (float)_rf_global_context_ptr->current_width/(float)_rf_global_context_ptr->current_height;

    if (camera.type == rf_camera_perspective)
    {
        // Setup perspective GL_PROJECTION
        double top = 0.01*tan(camera.fovy*0.5*RF_DEG2RAD);
        double right = top*aspect;

        rf_frustum(-right, right, -top, top, 0.01, 1000.0);
    }
    else if (camera.type == rf_camera_orthographic)
    {
        // Setup orthographic GL_PROJECTION
        double top = camera.fovy/2.0;
        double right = top*aspect;

        rf_ortho(-right,right,-top,top, 0.01, 1000.0);
    }

    // NOTE: zNear and zFar values are important when computing depth buffer values

    rf_matrix_mode(GL_MODELVIEW); // Switch back to rf_global_model_view matrix
    rf_load_identity(); // Reset current matrix (MODELVIEW)

    // Setup rf_camera3d view
    rf_matrix matView = rf_matrix_look_at(camera.position, camera.target, camera.up);
    rf_mult_matrixf(rf_matrix_to_float(matView)); // Multiply MODELVIEW matrix by view matrix (camera)

    rf_gl_enable_depth_test(); // Enable DEPTH_TEST for 3D
}

// Ends 3D mode and returns to default 2D orthographic mode
RF_API void rf_end_mode3d()
{
    rf_gl_draw(); // Process internal buffers (update + draw)

    rf_matrix_mode(GL_PROJECTION); // Switch to GL_PROJECTION matrix
    rf_pop_matrix(); // Restore previous matrix (PROJECTION) from matrix rf_global_gl_stack

    rf_matrix_mode(GL_MODELVIEW); // Get back to rf_global_model_view matrix
    rf_load_identity(); // Reset current matrix (MODELVIEW)

    rf_mult_matrixf(rf_matrix_to_float(_rf_global_context_ptr->screen_scaling)); // Apply screen scaling if required

    rf_gl_disable_depth_test(); // Disable DEPTH_TEST for 2D
}

// Initializes render texture for drawing
RF_API void rf_begin_texture_mode(rf_render_texture2d target)
{
    rf_gl_draw(); // Draw Buffers (Only OpenGL 3+ and ES2)

    rf_gl_enable_render_texture(target.id); // Enable render target

    // Set viewport to framebuffer size
    rf_gl_viewport(0, 0, target.texture.width, target.texture.height);

    rf_matrix_mode(GL_PROJECTION); // Switch to PROJECTION matrix
    rf_load_identity(); // Reset current matrix (PROJECTION)

    // Set orthographic GL_PROJECTION to current framebuffer size
    // NOTE: Configured top-left corner as (0, 0)
    rf_ortho(0, target.texture.width, target.texture.height, 0, 0.0f, 1.0f);

    rf_matrix_mode(GL_MODELVIEW); // Switch back to MODELVIEW matrix
    rf_load_identity(); // Reset current matrix (MODELVIEW)

    //rf_scalef(0.0f, -1.0f, 0.0f);      // Flip Y-drawing (?)

    // Setup current width/height for proper aspect ratio
    // calculation when using rf_begin_mode3d()
    _rf_global_context_ptr->current_width = target.texture.width;
    _rf_global_context_ptr->current_height = target.texture.height;
}

// Ends drawing to render texture
RF_API void rf_end_texture_mode()
{
    rf_gl_draw(); // Draw Buffers (Only OpenGL 3+ and ES2)

    rf_gl_disable_render_texture(); // Disable render target

    // Set viewport to default framebuffer size
    _rf_setup_viewport(_rf_global_context_ptr->render_width, _rf_global_context_ptr->render_height);

    // Reset current screen size
    _rf_global_context_ptr->current_width = _rf_global_context_ptr->screen_width;
    _rf_global_context_ptr->current_height = _rf_global_context_ptr->screen_height;
}

// Begin scissor mode (define screen area for following drawing)
// NOTE: Scissor rec refers to bottom-left corner, we change it to upper-left
RF_API void rf_begin_scissor_mode(int x, int y, int width, int height)
{
    rf_gl_draw(); // Force drawing elements

    rf_gl_enable_scissor_test();
    rf_gl_scissor(x, _rf_global_context_ptr->screen_height - (y + height), width, height);
}

// End scissor mode
RF_API void rf_end_scissor_mode()
{
    rf_gl_draw(); // Force drawing elements
    rf_gl_disable_scissor_test();
}

// Returns a ray trace from mouse position
RF_API rf_ray rf_get_mouse_ray(rf_sizei screen_size, rf_vector2 mouse_position, rf_camera3d camera)
{
    rf_ray ray;

    // Calculate normalized device coordinates
    // NOTE: y value is negative
    float x = (2.0f*mouse_position.x)/(float)screen_size.width - 1.0f;
    float y = 1.0f - (2.0f*mouse_position.y)/(float)screen_size.height;
    float z = 1.0f;

    // Store values in a vector
    rf_vector3 deviceCoords = { x, y, z };

    // Calculate view matrix from camera look at
    rf_matrix matView = rf_matrix_look_at(camera.position, camera.target, camera.up);

    rf_matrix matProj = rf_matrix_identity();

    if (camera.type == rf_camera_perspective)
    {
        // Calculate GL_PROJECTION matrix from perspective
        matProj = rf_matrix_perspective(camera.fovy*RF_DEG2RAD, ((double)screen_size.width/(double)screen_size.height), 0.01, 1000.0);
    }
    else if (camera.type == rf_camera_orthographic)
    {
        float aspect = (float)screen_size.width/(float)screen_size.height;
        double top = camera.fovy/2.0;
        double right = top*aspect;

        // Calculate GL_PROJECTION matrix from orthographic
        matProj = rf_matrix_ortho(-right, right, -top, top, 0.01, 1000.0);
    }

    // Unproject far/near points
    rf_vector3 nearPoint = rf_gl_unproject((rf_vector3){ deviceCoords.x, deviceCoords.y, 0.0f }, matProj, matView);
    rf_vector3 farPoint = rf_gl_unproject((rf_vector3){ deviceCoords.x, deviceCoords.y, 1.0f }, matProj, matView);

    // Unproject the mouse cursor in the near plane.
    // We need this as the source position because orthographic projects, compared to perspect doesn't have a
    // convergence point, meaning that the "eye" of the camera is more like a plane than a point.
    rf_vector3 cameraPlanePointerPos = rf_gl_unproject((rf_vector3){ deviceCoords.x, deviceCoords.y, -1.0f }, matProj, matView);

    // Calculate normalized direction vector
    rf_vector3 direction = rf_vector3_normalize(rf_vector3_substract(farPoint, nearPoint));

    if (camera.type == rf_camera_perspective) ray.position = camera.position;
    else if (camera.type == rf_camera_orthographic) ray.position = cameraPlanePointerPos;

    // Apply calculated vectors to ray
    ray.direction = direction;

    return ray;
}

// Get transform matrix for camera
RF_API rf_matrix rf_get_camera_matrix(rf_camera3d camera)
{
    return rf_matrix_look_at(camera.position, camera.target, camera.up);
}

// Returns camera 2d transform matrix
RF_API rf_matrix rf_get_camera_matrix2d(rf_camera2d camera)
{
    rf_matrix matTransform = { 0 };
    // The camera in world-space is set by
    //   1. Move it to target
    //   2. Rotate by -rotation and scale by (1/zoom)
    //      When setting higher scale, it's more intuitive for the world to become bigger (= camera become smaller),
    //      not for the camera getting bigger, hence the invert. Same deal with rotation.
    //   3. Move it by (-offset);
    //      Offset defines target transform relative to screen, but since we're effectively "moving" screen (camera)
    //      we need to do it into opposite direction (inverse transform)

    // Having camera transform in world-space, inverse of it gives the rf_global_model_view transform.
    // Since (A*B*C)' = C'*B'*A', the rf_global_model_view is
    //   1. Move to offset
    //   2. Rotate and Scale
    //   3. Move by -target
    rf_matrix matOrigin = rf_matrix_translate(-camera.target.x, -camera.target.y, 0.0f);
    rf_matrix matRotation = rf_matrix_rotate((rf_vector3){ 0.0f, 0.0f, 1.0f }, camera.rotation*RF_DEG2RAD);
    rf_matrix matScale = rf_matrix_scale(camera.zoom, camera.zoom, 1.0f);
    rf_matrix matTranslation = rf_matrix_translate(camera.offset.x, camera.offset.y, 0.0f);

    matTransform = rf_matrix_multiply(rf_matrix_multiply(matOrigin, rf_matrix_multiply(matScale, matRotation)), matTranslation);

    return matTransform;
}

// Returns the screen space position from a 3d world space position
RF_API rf_vector2 rf_get_world_to_screen(rf_sizei screen_size, rf_vector3 position, rf_camera3d camera)
{
    // Calculate GL_PROJECTION matrix (from perspective instead of frustum
    rf_matrix matProj = rf_matrix_identity();

    if (camera.type == rf_camera_perspective)
    {
        // Calculate GL_PROJECTION matrix from perspective
        matProj = rf_matrix_perspective(camera.fovy*RF_DEG2RAD, ((double)screen_size.width/(double)screen_size.height), 0.01, 1000.0);
    }
    else if (camera.type == rf_camera_orthographic)
    {
        float aspect = (float)screen_size.width/(float)screen_size.height;
        double top = camera.fovy/2.0;
        double right = top*aspect;

        // Calculate GL_PROJECTION matrix from orthographic
        matProj = rf_matrix_ortho(-right, right, -top, top, 0.01, 1000.0);
    }

    // Calculate view matrix from camera look at (and transpose it)
    rf_matrix matView = rf_matrix_look_at(camera.position, camera.target, camera.up);

    // Convert world position vector to quaternion
    rf_quaternion worldPos = { position.x, position.y, position.z, 1.0f };

    // rf_transform world position to view
    worldPos = rf_quaternion_transform(worldPos, matView);

    // rf_transform result to GL_PROJECTION (clip space position)
    worldPos = rf_quaternion_transform(worldPos, matProj);

    // Calculate normalized device coordinates (inverted y)
    rf_vector3 ndcPos = { worldPos.x/worldPos.w, -worldPos.y/worldPos.w, worldPos.z/worldPos.w };

    // Calculate 2d screen position vector
    rf_vector2 screenPosition = { (ndcPos.x + 1.0f)/2.0f*(float)screen_size.width, (ndcPos.y + 1.0f)/2.0f*(float)screen_size.height };

    return screenPosition;
}

// Returns the screen space position for a 2d camera world space position
RF_API rf_vector2 rf_get_world_to_screen2d(rf_vector2 position, rf_camera2d camera)
{
    rf_matrix matCamera = rf_get_camera_matrix2d(camera);
    rf_vector3 transform = rf_vector3_transform((rf_vector3){ position.x, position.y, 0 }, matCamera);

    return RF_CLITERAL(rf_vector2){ transform.x, transform.y };
}

// Returns the world space position for a 2d camera screen space position
RF_API rf_vector2 rf_get_screen_to_world2d(rf_vector2 position, rf_camera2d camera)
{
    rf_matrix invMatCamera = rf_matrix_invert(rf_get_camera_matrix2d(camera));
    rf_vector3 transform = rf_vector3_transform((rf_vector3){ position.x, position.y, 0 }, invMatCamera);

    return RF_CLITERAL(rf_vector2){ transform.x, transform.y };
}

// Set target FPS (maximum)
RF_API void rf_set_target_fps(int fps)
{
    if (fps < 1) _rf_global_context_ptr->target_time = 0.0;
    else _rf_global_context_ptr->target_time = 1.0/(double)fps;

    RF_LOG(RF_LOG_INFO, "Target time per frame: %02.03f milliseconds", (float)_rf_global_context_ptr->target_time*1000);
}

// Returns current FPS
RF_API int rf_get_fps()
{
    return (int)(1.0f/_rf_global_context_ptr->frame_time);
}

// Returns hexadecimal value for a rf_color
RF_API int rf_color_to_int(rf_color color)
{
    return (((int)color.r << 24) | ((int)color.g << 16) | ((int)color.b << 8) | (int)color.a);
}

// Returns color normalized as float [0..1]
RF_API rf_vector4 rf_color_normalize(rf_color color)
{
    rf_vector4 result;

    result.x = (float)color.r/255.0f;
    result.y = (float)color.g/255.0f;
    result.z = (float)color.b/255.0f;
    result.w = (float)color.a/255.0f;

    return result;
}

// Returns color from normalized values [0..1]
RF_API rf_color rf_color_from_normalized(rf_vector4 normalized)
{
    rf_color result;

    result.r = normalized.x*255.0f;
    result.g = normalized.y*255.0f;
    result.b = normalized.z*255.0f;
    result.a = normalized.w*255.0f;

    return result;
}

// Returns HSV values for a rf_color
// NOTE: Hue is returned as degrees [0..360]
RF_API rf_vector3 rf_color_to_hsv(rf_color color)
{
    rf_vector3 rgb = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f };
    rf_vector3 hsv = { 0.0f, 0.0f, 0.0f };
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

// Returns a rf_color from HSV values
// Implementation reference: https://en.wikipedia.org/wiki/HSL_and_HSV#Alternative_HSV_conversion
// NOTE: rf_color->HSV->rf_color conversion will not yield exactly the same color due to rounding errors
RF_API rf_color rf_color_from_hsv(rf_vector3 hsv)
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
RF_API rf_color rf_color_from_int(int hexValue)
{
    rf_color color;

    color.r = (unsigned char)(hexValue >> 24) & 0xFF;
    color.g = (unsigned char)(hexValue >> 16) & 0xFF;
    color.b = (unsigned char)(hexValue >> 8) & 0xFF;
    color.a = (unsigned char)hexValue & 0xFF;

    return color;
}

// rf_color fade-in or fade-out, alpha goes from 0.0f to 1.0f
RF_API rf_color rf_fade(rf_color color, float alpha)
{
    if (alpha < 0.0f) alpha = 0.0f;
    else if (alpha > 1.0f) alpha = 1.0f;

    return RF_CLITERAL(rf_color){color.r, color.g, color.b, (unsigned char)(255.0f*alpha)};
}

//endregion

//region rlgl

#define GL_SHADING_LANGUAGE_VERSION         0x8B8C
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT     0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT    0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT    0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT    0x83F3
#define GL_ETC1_RGB8_OES                    0x8D64
#define GL_COMPRESSED_RGB8_ETC2             0x9274
#define GL_COMPRESSED_RGBA8_ETC2_EAC        0x9278
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG  0x8C00
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0x8C02
#define GL_COMPRESSED_RGBA_ASTC_4x4_KHR     0x93b0
#define GL_COMPRESSED_RGBA_ASTC_8x8_KHR     0x93b7
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT   0x84FF
#define GL_TEXTURE_MAX_ANISOTROPY_EXT       0x84FE

#if defined(RF_GRAPHICS_API_OPENGL_11)
    #define GL_UNSIGNED_SHORT_5_6_5             0x8363
    #define GL_UNSIGNED_SHORT_5_5_5_1           0x8034
    #define GL_UNSIGNED_SHORT_4_4_4_4           0x8033
#endif

#if defined(RF_GRAPHICS_API_OPENGL_21)
    #define GL_LUMINANCE                        0x1909
    #define GL_LUMINANCE_ALPHA                  0x190A
#endif

#if defined(RF_GRAPHICS_API_OPENGL_ES2)
    #define glClearDepth                glClearDepthf
    #define GL_READ_FRAMEBUFFER         GL_FRAMEBUFFER
    #define GL_DRAW_FRAMEBUFFER         GL_FRAMEBUFFER
#endif

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
RF_INTERNAL unsigned int _rf_compile_shader(const char* shaderStr, int type);     // Compile custom shader and return shader id
RF_INTERNAL unsigned int _rf_load_shader_program(unsigned int vShaderId, unsigned int fShaderId);  // Load custom shader program

RF_INTERNAL rf_shader _rf_load_shader_default() ;      // Load default shader (just vertex positioning and texture coloring)
RF_INTERNAL void _rf_set_shader_default_locations(rf_shader* shader); // Bind default shader locations (attributes and uniforms)
RF_INTERNAL void _rf_unlock_shader_default() ;      // Unload default shader

RF_INTERNAL void _rf_load_buffers_default() ;       // Load default internal buffers
RF_INTERNAL void _rf_update_buffers_default() ;     // Update default internal buffers (VAOs/VBOs) with vertex data
RF_INTERNAL void _rf_draw_buffers_default() ;       // Draw default internal buffers vertex data
RF_INTERNAL void _rf_unload_buffers_default() ;     // Unload default internal buffers vertex data from CPU and GPU

RF_INTERNAL void _rf_gen_draw_cube(void);              // Generate and draw cube
RF_INTERNAL void _rf_gen_draw_quad(void);              // Generate and draw quad

#endif  // RF_GRAPHICS_API_OPENGL_33 || RF_GRAPHICS_API_OPENGL_ES2

#if defined(RF_GRAPHICS_API_OPENGL_11)
RF_INTERNAL int _rf_generate_mipmaps( unsigned char* data, int baseWidth, int baseHeight);
RF_INTERNAL rf_color* _rf_gen_next_mipmap( rf_color* srcData, int srcWidth, int srcHeight);
#endif

// Initialize rlgl: OpenGL extensions, default buffers/shaders/textures, OpenGL states
RF_API void rf_context_init(rf_context* rf_ctx, int width, int height)
{
    RF_ASSERT(width != 0 && height != 0);
    _rf_global_context_ptr = rf_ctx;

    *_rf_global_context_ptr = RF_CLITERAL(rf_context) {0};
    _rf_global_context_ptr->gl_ctx = RF_CLITERAL(rf_gl_context) {0};

    #if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
        _rf_global_context_ptr->gl_ctx.current_matrix_mode = -1;
        _rf_global_context_ptr->gl_ctx.current_depth = -1.0f;
        _rf_global_context_ptr->gl_ctx.max_depth_bits = 16;
    #endif

    _rf_global_context_ptr->gl_ctx.player_eyes_position = 1.85f;
    _rf_global_context_ptr->gl_ctx.camera_move_control[0] = 'W';
    _rf_global_context_ptr->gl_ctx.camera_move_control[1] = 'S';
    _rf_global_context_ptr->gl_ctx.camera_move_control[2] = 'D';
    _rf_global_context_ptr->gl_ctx.camera_move_control[3] = 'A';
    _rf_global_context_ptr->gl_ctx.camera_move_control[4] = 'E';
    _rf_global_context_ptr->gl_ctx.camera_move_control[5] = 'Q';
    _rf_global_context_ptr->gl_ctx.camera_pan_control_key = 2;
    _rf_global_context_ptr->gl_ctx.camera_alt_control_key = 342;
    _rf_global_context_ptr->gl_ctx.camera_smooth_zoom_control_key = 341;
    _rf_global_context_ptr->gl_ctx.camera_mode = rf_camera_custom;

    _rf_global_context_ptr->screen_scaling = rf_matrix_identity(),
    _rf_global_context_ptr->current_width = width,
    _rf_global_context_ptr->current_height = height,

    _rf_setup_frame_buffer(width, height);

    // Check OpenGL information and capabilities
    //------------------------------------------------------------------------------

    // Print current OpenGL and GLSL version
    RF_LOG(RF_LOG_INFO, "GPU: Vendor:   %s", glGetString(GL_VENDOR));
    RF_LOG(RF_LOG_INFO, "GPU: Renderer: %s", glGetString(GL_RENDERER));
    RF_LOG(RF_LOG_INFO, "GPU: Version:  %s", glGetString(GL_VERSION));
    RF_LOG(RF_LOG_INFO, "GPU: GLSL:     %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // NOTE: We can get a bunch of extra information about GPU capabilities (glGet*)
    //int maxTexSize;
    //glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
    //RF_LOG(RF_LOG_INFO, "GL_MAX_TEXTURE_SIZE: %i", maxTexSize);

    //GL_MAX_TEXTURE_IMAGE_UNITS
    //GL_MAX_VIEWPORT_DIMS

    //int numAuxBuffers;
    //glGetIntegerv(GL_AUX_BUFFERS, &numAuxBuffers);
    //RF_LOG(RF_LOG_INFO, "GL_AUX_BUFFERS: %i", numAuxBuffers);

    //GLint numComp = 0;
    //GLint format[32] = { 0 };
    //glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &numComp);
    //glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, format);
    //for (int i = 0; i < numComp; i++) RF_LOG(RF_LOG_INFO, "Supported compressed format: 0x%x", format[i]);

    // NOTE: We don't need that much data on screen... right now...

    // TODO: Automatize extensions loading using rf_gl_load_extensions() and GLAD
    // Actually, when rf_context_create() is called in InitWindow() in core.c,
    // OpenGL required extensions have already been loaded (PLATFORM_DESKTOP)

#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    // Get supported extensions list
    GLint numExt = 0;

#if defined(RF_GRAPHICS_API_OPENGL_33)
    // NOTE: On OpenGL 3.3 VAO and NPOT are supported by default
    _rf_global_context_ptr->gl_ctx.vao_supported = true;

    // Multiple texture extensions supported by default
    _rf_global_context_ptr->gl_ctx.tex_npot_supported = true;
    _rf_global_context_ptr->gl_ctx.tex_float_supported = true;
    _rf_global_context_ptr->gl_ctx.tex_depth_supported = true;

    // We get a list of available extensions and we check for some of them (compressed textures)
    // NOTE: We don't need to check again supported extensions but we do (GLAD already dealt with that)
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);

    // Allocate numExt strings pointers
    const char** extList = RF_MALLOC(sizeof(const char* )*numExt);

    // Get extensions strings
    for (int i = 0; i < numExt; i++) extList[i] = (const char* )glGetStringi(GL_EXTENSIONS, i);

#elif defined(RF_GRAPHICS_API_OPENGL_ES2)
    // Allocate 512 strings pointers (2 KB)
    const char** extList = RF_MALLOC(sizeof(const char* )*512);

    const char* extensions = (const char* )glGetString(GL_EXTENSIONS);  // One big const string

    // NOTE: We have to duplicate string because glGetString() returns a const string
    int len = strlen(extensions) + 1;
    char* extensionsDup = (char*)RF_MALLOC(len);
    memset(extensionsDup, 0, len);
    strcpy(extensionsDup, extensions);

    extList[numExt] = extensionsDup;

    for (int i = 0; i < len; i++)
    {
        if (extensionsDup[i] == ' ')
        {
            extensionsDup[i] = '\0';

            numExt++;
            extList[numExt] = &extensionsDup[i + 1];
        }
    }

    // NOTE: Duplicated string (extensionsDup) must be deallocated
#endif

    RF_LOG(RF_LOG_INFO, "Number of supported extensions: %i", numExt);

    // Show supported extensions
    //for (int i = 0; i < numExt; i++)  RF_LOG(RF_LOG_INFO, "Supported extension: %s", extList[i]);

    // Check required extensions
    for (int i = 0; i < numExt; i++)
    {
#if defined(RF_GRAPHICS_API_OPENGL_ES2)
        // Check VAO support
        // NOTE: Only check on OpenGL ES, OpenGL 3.3 has VAO support as core feature
        // @Note @PossibleBug (LucaSas): The code here to check for VAO support tried to load the functions and checked if the result was NULL.
        //                               Our goal is to be platform independent so we dont want to load opengl for the user, therefore I removed that code.
        //                               However, I dont know if this code is enough to check for vao support. So this is a possible bug.
        if (strcmp(extList[i], (const char* )"GL_OES_vertex_array_object") == 0) _rf_global_context_ptr->gl_ctx.vao_supported = true;

        // Check NPOT textures support
        // NOTE: Only check on OpenGL ES, OpenGL 3.3 has NPOT textures full support as core feature
        if (strcmp(extList[i], (const char* )"GL_OES_texture_npot") == 0) _rf_global_context_ptr->gl_ctx.tex_npot_supported = true;

        // Check texture float support
        if (strcmp(extList[i], (const char* )"GL_OES_texture_float") == 0) _rf_global_context_ptr->gl_ctx.tex_float_supported = true;

        // Check depth texture support
        if ((strcmp(extList[i], (const char* )"GL_OES_depth_texture") == 0) ||
            (strcmp(extList[i], (const char* )"GL_WEBGL_depth_texture") == 0)) _rf_global_context_ptr->gl_ctx.tex_depth_supported = true;

        if (strcmp(extList[i], (const char* )"GL_OES_depth24") == 0) _rf_global_context_ptr->gl_ctx.max_depth_bits = 24;
        if (strcmp(extList[i], (const char* )"GL_OES_depth32") == 0) _rf_global_context_ptr->gl_ctx.max_depth_bits = 32;
#endif
        // DDS texture compression support
        if ((strcmp(extList[i], (const char* )"GL_EXT_texture_compression_s3tc") == 0) ||
            (strcmp(extList[i], (const char* )"GL_WEBGL_compressed_texture_s3tc") == 0) ||
            (strcmp(extList[i], (const char* )"GL_WEBKIT_WEBGL_compressed_texture_s3tc") == 0)) _rf_global_context_ptr->gl_ctx.tex_comp_dxt_supported = true;

        // ETC1 texture compression support
        if ((strcmp(extList[i], (const char* )"GL_OES_compressed_ETC1_RGB8_texture") == 0) ||
            (strcmp(extList[i], (const char* )"GL_WEBGL_compressed_texture_etc1") == 0)) _rf_global_context_ptr->gl_ctx.tex_comp_etc1_supported = true;

        // ETC2/EAC texture compression support
        if (strcmp(extList[i], (const char* )"GL_ARB_ES3_compatibility") == 0) _rf_global_context_ptr->gl_ctx.tex_comp_etc2_supported = true;

        // PVR texture compression support
        if (strcmp(extList[i], (const char* )"GL_IMG_texture_compression_pvrtc") == 0) _rf_global_context_ptr->gl_ctx.tex_comp_pvrt_supported = true;

        // ASTC texture compression support
        if (strcmp(extList[i], (const char* )"GL_KHR_texture_compression_astc_hdr") == 0) _rf_global_context_ptr->gl_ctx.tex_comp_astc_supported = true;

        // Anisotropic texture filter support
        if (strcmp(extList[i], (const char* )"GL_EXT_texture_filter_anisotropic") == 0)
        {
            _rf_global_context_ptr->gl_ctx.tex_anisotropic_filter_supported = true;
            glGetFloatv(0x84FF, &_rf_global_context_ptr->gl_ctx.max_anisotropic_level);   // GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
        }

        // Clamp mirror wrap mode supported
        if (strcmp(extList[i], (const char* )"GL_EXT_texture_mirror_clamp") == 0) _rf_global_context_ptr->gl_ctx.tex_mirror_clamp_supported = true;

        // Debug marker support
        if (strcmp(extList[i], (const char* )"GL_EXT_debug_marker") == 0) _rf_global_context_ptr->gl_ctx.debug_marker_supported = true;
    }

    // Free extensions pointers
    RF_FREE((char**)extList);

#if defined(RF_GRAPHICS_API_OPENGL_ES2)
    RF_FREE(extensionsDup);    // Duplicated string must be deallocated

    if (_rf_global_context_ptr->gl_ctx.vao_supported) RF_LOG(RF_LOG_INFO, "[EXTENSION] VAO extension detected, VAO functions initialized successfully");
    else RF_LOG(RF_LOG_WARNING, "[EXTENSION] VAO extension not found, VAO usage not supported");

    if (_rf_global_context_ptr->gl_ctx.tex_npot_supported) RF_LOG(RF_LOG_INFO, "[EXTENSION] NPOT textures extension detected, full NPOT textures supported");
    else RF_LOG(RF_LOG_WARNING, "[EXTENSION] NPOT textures extension not found, limited NPOT support (no-mipmaps, no-repeat)");
#endif

    if (_rf_global_context_ptr->gl_ctx.tex_comp_dxt_supported) RF_LOG(RF_LOG_INFO, "[EXTENSION] DXT compressed textures supported");
    if (_rf_global_context_ptr->gl_ctx.tex_comp_etc1_supported) RF_LOG(RF_LOG_INFO, "[EXTENSION] ETC1 compressed textures supported");
    if (_rf_global_context_ptr->gl_ctx.tex_comp_etc2_supported) RF_LOG(RF_LOG_INFO, "[EXTENSION] ETC2/EAC compressed textures supported");
    if (_rf_global_context_ptr->gl_ctx.tex_comp_pvrt_supported) RF_LOG(RF_LOG_INFO, "[EXTENSION] PVRT compressed textures supported");
    if (_rf_global_context_ptr->gl_ctx.tex_comp_astc_supported) RF_LOG(RF_LOG_INFO, "[EXTENSION] ASTC compressed textures supported");

    if (_rf_global_context_ptr->gl_ctx.tex_anisotropic_filter_supported) RF_LOG(RF_LOG_INFO, "[EXTENSION] Anisotropic textures filtering supported (max: %.0fX)", _rf_global_context_ptr->gl_ctx.max_anisotropic_level);
    if (_rf_global_context_ptr->gl_ctx.tex_mirror_clamp_supported) RF_LOG(RF_LOG_INFO, "[EXTENSION] Mirror clamp wrap texture mode supported");

    if (_rf_global_context_ptr->gl_ctx.debug_marker_supported) RF_LOG(RF_LOG_INFO, "[EXTENSION] Debug Marker supported");

    // Initialize buffers, default shaders and default textures
    //----------------------------------------------------------
    // Init default white texture
    unsigned char pixels[4] = { 255, 255, 255, 255 };   // 1 pixel RGBA (4 bytes)
    _rf_global_context_ptr->gl_ctx.default_texture_id = rf_gl_load_texture(pixels, 1, 1, rf_uncompressed_r8g8b8a8, 1);

    if (_rf_global_context_ptr->gl_ctx.default_texture_id != 0) RF_LOG(RF_LOG_INFO, "[TEX ID %i] Base white texture loaded successfully", _rf_global_context_ptr->gl_ctx.default_texture_id);
    else RF_LOG(RF_LOG_WARNING, "Base white texture could not be loaded");

    // Init default rf_shader (customized for GL 3.3 and ES2)
    _rf_global_context_ptr->gl_ctx.default_shader = _rf_load_shader_default();
    _rf_global_context_ptr->gl_ctx.current_shader = _rf_global_context_ptr->gl_ctx.default_shader;

    // Init default vertex arrays buffers
    _rf_load_buffers_default();

    // Init transformations matrix accumulator
    _rf_global_context_ptr->gl_ctx.transform_matrix = rf_matrix_identity();

    // Init draw calls tracking system
    _rf_global_context_ptr->gl_ctx.draws = (rf_draw_call *)RF_MALLOC(sizeof(rf_draw_call)*rf_max_drawcall_registered);

    for (int i = 0; i < rf_max_drawcall_registered; i++)
    {
        _rf_global_context_ptr->gl_ctx.draws[i].mode = GL_QUADS;
        _rf_global_context_ptr->gl_ctx.draws[i].vertex_count = 0;
        _rf_global_context_ptr->gl_ctx.draws[i].vertexAlignment = 0;
        //_rf_global_context_ptr->gl_ctx.draws[i].vao_id = 0;
        //_rf_global_context_ptr->gl_ctx.draws[i].shaderId = 0;
        _rf_global_context_ptr->gl_ctx.draws[i].textureId = _rf_global_context_ptr->gl_ctx.default_texture_id;
        //_rf_global_context_ptr->gl_ctx.draws[i].projection = rf_matrix_identity();
        //_rf_global_context_ptr->gl_ctx.draws[i].modelview = rf_matrix_identity();
    }

    _rf_global_context_ptr->gl_ctx.draws_counter = 1;

    // Init internal matrix _rf_global_context_ptr.gl_ctx.stack (emulating OpenGL 1.1)
    for (int i = 0; i < rf_max_matrix_stack_size; i++) _rf_global_context_ptr->gl_ctx.stack[i] = rf_matrix_identity();

    // Init internal _rf_global_context_ptr.gl_ctx.projection and _rf_global_context_ptr.gl_ctx.modelview matrices
    _rf_global_context_ptr->gl_ctx.projection = rf_matrix_identity();
    _rf_global_context_ptr->gl_ctx.modelview = rf_matrix_identity();
    _rf_global_context_ptr->gl_ctx.current_matrix = &_rf_global_context_ptr->gl_ctx.modelview;
#endif      // RF_GRAPHICS_API_OPENGL_33 || RF_GRAPHICS_API_OPENGL_ES2

    // Initialize OpenGL default states
    //----------------------------------------------------------
    // Init state: Depth test
    glDepthFunc(GL_LEQUAL);                                 // Type of depth testing to apply
    glDisable(GL_DEPTH_TEST);                               // Disable depth testing for 2D (only used for 3D)

    // Init state: Blending mode
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);      // rf_color blending function (how colors are mixed)
    glEnable(GL_BLEND);                                     // Enable color blending (required to work with transparencies)

    // Init state: Culling
    // NOTE: All shapes/models triangles are drawn CCW
    glCullFace(GL_BACK);                                    // Cull the back face (default)
    glFrontFace(GL_CCW);                                    // Front face are defined counter clockwise (default)
    glEnable(GL_CULL_FACE);                                 // Enable backface culling

#if defined(RF_GRAPHICS_API_OPENGL_11)
    // Init state: rf_color hints (deprecated in OpenGL 3.0+)
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);      // Improve quality of color and texture coordinate interpolation
    glShadeModel(GL_SMOOTH);                                // Smooth shading between vertex (vertex colors interpolation)
#endif

    // Init state: rf_color/Depth buffers clear
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);                   // Set clear color (black)
    glClearDepth(1.0f);                                     // Set clear depth value (default)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear color and depth buffers (depth buffer required for 3D)

    // Store screen size into global variables
    _rf_global_context_ptr->gl_ctx.framebuffer_width = width;
    _rf_global_context_ptr->gl_ctx.framebuffer_height = height;

    RF_LOG(RF_LOG_INFO, "OpenGL default states initialized successfully");

    // Setup default viewport
    _rf_setup_viewport(width, height);
}

//----------------------------------------------------------------------------------
// Module Functions Definition - rf_matrix operations
//----------------------------------------------------------------------------------

#if defined(RF_GRAPHICS_API_OPENGL_11)

// Fallback to OpenGL 1.1 function calls
//---------------------------------------
RF_API void rf_matrix_mode(int mode)
{
    ((void)_rf_global_context_ptr); //unused param
    switch (mode)
    {
        case GL_PROJECTION: glMatrixMode(GL_PROJECTION); break;
        case GL_MODELVIEW: glMatrixMode(GL_MODELVIEW); break;
        case GL_TEXTURE: glMatrixMode(GL_TEXTURE); break;
        default: break;
    }
}

RF_API void rf_frustum(double left, double right, double bottom, double top, double znear, double zfar)
{
    ((void)_rf_global_context_ptr);
    glFrustum(left, right, bottom, top, znear, zfar);
}

RF_API void rf_ortho(double left, double right, double bottom, double top, double znear, double zfar)
{
    ((void)_rf_global_context_ptr);
    glOrtho(left, right, bottom, top, znear, zfar);
}

RF_API void rf_push_matrix() { ((void)_rf_global_context_ptr); glPushMatrix(); }
RF_API void rf_pop_matrix() { ((void)_rf_global_context_ptr); glPopMatrix(); }
RF_API void rf_load_identity() { ((void)_rf_global_context_ptr); glLoadIdentity(); }
RF_API void rf_translatef(float x, float y, float z) { ((void)_rf_global_context_ptr); glTranslatef(x, y, z); }
RF_API void rf_rotatef(float angleDeg, float x, float y, float z) { ((void)_rf_global_context_ptr); glRotatef(angleDeg, x, y, z); }
RF_API void rf_scalef(float x, float y, float z) { ((void)_rf_global_context_ptr); glScalef(x, y, z); }
RF_API void rf_mult_matrixf(float* matf) { ((void)_rf_global_context_ptr); glMultMatrixf(matf); }

#elif defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)

// Choose the current matrix to be transformed
RF_API void rf_matrix_mode(int mode)
{
    if (mode == GL_PROJECTION) _rf_global_context_ptr->gl_ctx.current_matrix = &_rf_global_context_ptr->gl_ctx.projection;
    else if (mode == GL_MODELVIEW) _rf_global_context_ptr->gl_ctx.current_matrix = &_rf_global_context_ptr->gl_ctx.modelview;
    //else if (mode == GL_TEXTURE) // Not supported

    _rf_global_context_ptr->gl_ctx.current_matrix_mode = mode;
}

// Push the current matrix into _rf_global_context_ptr->gl_ctx.stack
RF_API void rf_push_matrix()
{
    if (_rf_global_context_ptr->gl_ctx.stack_counter >= rf_max_matrix_stack_size) RF_LOG(RF_LOG_ERROR, "rf_matrix _rf_global_context_ptr->gl_ctx.stack overflow");

    if (_rf_global_context_ptr->gl_ctx.current_matrix_mode == GL_MODELVIEW)
    {
        _rf_global_context_ptr->gl_ctx.use_transform_matrix = true;
        _rf_global_context_ptr->gl_ctx.current_matrix = &_rf_global_context_ptr->gl_ctx.transform_matrix;
    }

    _rf_global_context_ptr->gl_ctx.stack[_rf_global_context_ptr->gl_ctx.stack_counter] = *_rf_global_context_ptr->gl_ctx.current_matrix;
    _rf_global_context_ptr->gl_ctx.stack_counter++;
}

// Pop lattest inserted matrix from _rf_global_context_ptr->gl_ctx.stack
RF_API void rf_pop_matrix()
{
    if (_rf_global_context_ptr->gl_ctx.stack_counter > 0)
    {
        rf_matrix mat = _rf_global_context_ptr->gl_ctx.stack[_rf_global_context_ptr->gl_ctx.stack_counter - 1];
        *_rf_global_context_ptr->gl_ctx.current_matrix = mat;
        _rf_global_context_ptr->gl_ctx.stack_counter--;
    }

    if ((_rf_global_context_ptr->gl_ctx.stack_counter == 0) && (_rf_global_context_ptr->gl_ctx.current_matrix_mode == GL_MODELVIEW))
    {
        _rf_global_context_ptr->gl_ctx.current_matrix = &_rf_global_context_ptr->gl_ctx.modelview;
        _rf_global_context_ptr->gl_ctx.use_transform_matrix = false;
    }
}

// Reset current matrix to identity matrix
RF_API void rf_load_identity()
{
    *_rf_global_context_ptr->gl_ctx.current_matrix = rf_matrix_identity();
}

// Multiply the current matrix by a translation matrix
RF_API void rf_translatef(float x, float y, float z)
{
    rf_matrix matTranslation = rf_matrix_translate(x, y, z);

    // NOTE: We transpose matrix with multiplication order
    *_rf_global_context_ptr->gl_ctx.current_matrix = rf_matrix_multiply(matTranslation, *_rf_global_context_ptr->gl_ctx.current_matrix);
}

// Multiply the current matrix by a rotation matrix
RF_API void rf_rotatef(float angleDeg, float x, float y, float z)
{
    rf_matrix matRotation = rf_matrix_identity();

    rf_vector3 axis = (rf_vector3){ x, y, z };
    matRotation = rf_matrix_rotate(rf_vector3_normalize(axis), angleDeg*RF_DEG2RAD);

    // NOTE: We transpose matrix with multiplication order
    *_rf_global_context_ptr->gl_ctx.current_matrix = rf_matrix_multiply(matRotation, *_rf_global_context_ptr->gl_ctx.current_matrix);
}

// Multiply the current matrix by a scaling matrix
RF_API void rf_scalef(float x, float y, float z)
{
    rf_matrix matScale = rf_matrix_scale(x, y, z);

    // NOTE: We transpose matrix with multiplication order
    *_rf_global_context_ptr->gl_ctx.current_matrix = rf_matrix_multiply(matScale, *_rf_global_context_ptr->gl_ctx.current_matrix);
}

// Multiply the current matrix by another matrix
RF_API void rf_mult_matrixf(float* matf)
{
    // rf_matrix creation from array
    rf_matrix mat = { matf[0], matf[4], matf[8], matf[12],
            matf[1], matf[5], matf[9], matf[13],
            matf[2], matf[6], matf[10], matf[14],
            matf[3], matf[7], matf[11], matf[15] };

    *_rf_global_context_ptr->gl_ctx.current_matrix = rf_matrix_multiply(*_rf_global_context_ptr->gl_ctx.current_matrix, mat);
}

// Multiply the current matrix by a perspective matrix generated by parameters
RF_API void rf_frustum(double left, double right, double bottom, double top, double znear, double zfar)
{
    rf_matrix matPerps = rf_matrix_frustum(left, right, bottom, top, znear, zfar);

    *_rf_global_context_ptr->gl_ctx.current_matrix = rf_matrix_multiply(*_rf_global_context_ptr->gl_ctx.current_matrix, matPerps);
}

// Multiply the current matrix by an orthographic matrix generated by parameters
RF_API void rf_ortho(double left, double right, double bottom, double top, double znear, double zfar)
{
    rf_matrix matOrtho = rf_matrix_ortho(left, right, bottom, top, znear, zfar);

    *_rf_global_context_ptr->gl_ctx.current_matrix = rf_matrix_multiply(*_rf_global_context_ptr->gl_ctx.current_matrix, matOrtho);
}

#endif

// Set the viewport area (transformation from normalized device coordinates to window coordinates)
// NOTE: Updates global variables: _rf_global_context_ptr->gl_ctx.framebuffer_width, _rf_global_context_ptr->gl_ctx.framebuffer_height
void rf_gl_viewport(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Vertex level operations
//----------------------------------------------------------------------------------
#if defined(RF_GRAPHICS_API_OPENGL_11)

// Fallback to OpenGL 1.1 function calls
//---------------------------------------
RF_API void rf_gl_begin(int mode)
{
    ((void)_rf_global_context_ptr);
    switch (mode)
    {
        case GL_LINES: glBegin(GL_LINES); break;
        case GL_TRIANGLES: glBegin(GL_TRIANGLES); break;
        case GL_QUADS: glBegin(GL_QUADS); break;
        default: break;
    }
}

RF_API void rf_gl_end() { ((void)_rf_global_context_ptr); glEnd(); }
RF_API void rf_gl_vertex2i(int x, int y) { ((void)_rf_global_context_ptr); glVertex2i(x, y); }
RF_API void rf_gl_vertex2f(float x, float y) { ((void)_rf_global_context_ptr); glVertex2f(x, y); }
RF_API void rf_gl_vertex3f(float x, float y, float z) { ((void)_rf_global_context_ptr); glVertex3f(x, y, z); }
RF_API void rf_gl_tex_coord2f(float x, float y) { ((void)_rf_global_context_ptr); glTexCoord2f(x, y); }
RF_API void rf_gl_normal3f(float x, float y, float z) { ((void)_rf_global_context_ptr); glNormal3f(x, y, z); }
RF_API void rf_gl_color4ub(rf_byte r, rf_byte g, rf_byte b, rf_byte a) { ((void)_rf_global_context_ptr); glColor4ub(r, g, b, a); }
RF_API void rf_gl_color3f(float x, float y, float z) { ((void)_rf_global_context_ptr); glColor3f(x, y, z); }
RF_API void rf_gl_color4f(float x, float y, float z, float w) { ((void)_rf_global_context_ptr); glColor4f(x, y, z, w); }

#elif defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)

// Initialize drawing mode (how to organize vertex)
RF_API void rf_gl_begin(int mode)
{
    // Draw mode can be GL_LINES, GL_TRIANGLES and GL_QUADS
    // NOTE: In all three cases, vertex are accumulated over default internal vertex buffer
    if (_rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].mode != mode)
    {
        if (_rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertex_count > 0)
        {
            // Make sure current _rf_global_context_ptr->gl_ctx.draws[i].vertex_count is aligned a multiple of 4,
            // that way, following QUADS drawing will keep aligned with index processing
            // It implies adding some extra alignment vertex at the end of the draw,
            // those vertex are not processed but they are considered as an additional offset
            // for the next set of vertex to be drawn
            if (_rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].mode == GL_LINES) _rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertexAlignment = ((_rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertex_count < 4)? _rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertex_count : _rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertex_count%4);
            else if (_rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].mode == GL_TRIANGLES) _rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertexAlignment = ((_rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertex_count < 4)? 1 : (4 - (_rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertex_count%4)));

            else _rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertexAlignment = 0;

            if (rf_gl_check_buffer_limit(_rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertexAlignment)) rf_gl_draw();
            else
            {
                _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vCounter += _rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertexAlignment;
                _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].cCounter += _rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertexAlignment;
                _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].tcCounter += _rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertexAlignment;

                _rf_global_context_ptr->gl_ctx.draws_counter++;
            }
        }

        if (_rf_global_context_ptr->gl_ctx.draws_counter >= rf_max_drawcall_registered) rf_gl_draw();

        _rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].mode = mode;
        _rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertex_count = 0;
        _rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].textureId = _rf_global_context_ptr->gl_ctx.default_texture_id;
    }
}

// Finish vertex providing
RF_API void rf_gl_end()
{
    // Make sure vertex_count is the same for vertices, texcoords, colors and normals
    // NOTE: In OpenGL 1.1, one glColor call can be made for all the subsequent glVertex calls

    // Make sure colors count match vertex count
    if (_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vCounter != _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].cCounter)
    {
        int addColors = _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vCounter - _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].cCounter;

        for (int i = 0; i < addColors; i++)
        {
            _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].colors[4*_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].cCounter] = _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].colors[4*_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].cCounter - 4];
            _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].colors[4*_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].cCounter + 1] = _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].colors[4*_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].cCounter - 3];
            _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].colors[4*_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].cCounter + 2] = _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].colors[4*_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].cCounter - 2];
            _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].colors[4*_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].cCounter + 3] = _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].colors[4*_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].cCounter - 1];
            _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].cCounter++;
        }
    }

    // Make sure texcoords count match vertex count
    if (_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vCounter != _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].tcCounter)
    {
        int addTexCoords = _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vCounter - _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].tcCounter;

        for (int i = 0; i < addTexCoords; i++)
        {
            _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].texcoords[2*_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].tcCounter] = 0.0f;
            _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].texcoords[2*_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].tcCounter + 1] = 0.0f;
            _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].tcCounter++;
        }
    }

    // TODO: Make sure normals count match vertex count... if normals support is added in a future... :P

    // NOTE: Depth increment is dependant on rf_ortho(): z-near and z-far values,
    // as well as depth buffer bit-depth (16bit or 24bit or 32bit)
    // Correct increment formula would be: depthInc = (zfar - znear)/pow(2, bits)
    _rf_global_context_ptr->gl_ctx.current_depth += (1.0f/20000.0f);

    // Verify internal buffers limits
    // NOTE: This check is combined with usage of rf_gl_check_buffer_limit()
    if ((_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vCounter) >= (rf_max_batch_elements*4 - 4))
    {
        // WARNING: If we are between rf_push_matrix() and rf_pop_matrix() and we need to force a rf_gl_draw(),
        // we need to call rf_pop_matrix() before to recover *_rf_global_context_ptr->gl_ctx.current_matrix (_rf_global_context_ptr->gl_ctx.modelview) for the next forced draw call!
        // If we have multiple matrix pushed, it will require "_rf_global_context_ptr->gl_ctx.stack_counter" pops before launching the draw
        for (int i = _rf_global_context_ptr->gl_ctx.stack_counter; i >= 0; i--) rf_pop_matrix();
        rf_gl_draw();
    }
}

// Define one vertex (position)
// NOTE: Vertex position data is the basic information required for drawing
RF_API void rf_gl_vertex3f(float x, float y, float z)
{
    rf_vector3 vec = { x, y, z };

    // rf_transform provided vector if required
    if (_rf_global_context_ptr->gl_ctx.use_transform_matrix) vec = rf_vector3_transform(vec, _rf_global_context_ptr->gl_ctx.transform_matrix);

    // Verify that rf_max_batch_elements limit not reached
    if (_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vCounter < (rf_max_batch_elements*4))
    {
        _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vertices[3*_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vCounter] = vec.x;
        _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vertices[3*_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vCounter + 1] = vec.y;
        _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vertices[3*_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vCounter + 2] = vec.z;
        _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vCounter++;

        _rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertex_count++;
    }
    else RF_LOG(RF_LOG_ERROR, "rf_max_batch_elements overflow");
}

// Define one vertex (position)
RF_API void rf_gl_vertex2f(float x, float y)
{
    rf_gl_vertex3f(x, y, _rf_global_context_ptr->gl_ctx.current_depth);
}

// Define one vertex (position)
RF_API void rf_gl_vertex2i(int x, int y)
{
    rf_gl_vertex3f((float)x, (float)y, _rf_global_context_ptr->gl_ctx.current_depth);
}

// Define one vertex (texture coordinate)
// NOTE: rf_texture coordinates are limited to QUADS only
RF_API void rf_gl_tex_coord2f(float x, float y)
{
    _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].texcoords[2*_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].tcCounter] = x;
    _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].texcoords[2*_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].tcCounter + 1] = y;
    _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].tcCounter++;
}

// Define one vertex (normal)
// NOTE: Normals limited to TRIANGLES only?
RF_API void rf_gl_normal3f(float x, float y, float z)
{
    // TODO: Normals usage...
}

// Define one vertex (color)
RF_API void rf_gl_color4ub(rf_byte x, rf_byte y, rf_byte z, rf_byte w)
{
    _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].colors[4*_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].cCounter] = x;
    _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].colors[4*_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].cCounter + 1] = y;
    _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].colors[4*_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].cCounter + 2] = z;
    _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].colors[4*_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].cCounter + 3] = w;
    _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].cCounter++;
}

// Define one vertex (color)
RF_API void rf_gl_color4f(float r, float g, float b, float a)
{
    rf_gl_color4ub((rf_byte)(r*255), (rf_byte)(g*255), (rf_byte)(b*255), (rf_byte)(a*255));
}

// Define one vertex (color)
RF_API void rf_gl_color3f(float x, float y, float z)
{
    rf_gl_color4ub((rf_byte)(x*255), (rf_byte)(y*255), (rf_byte)(z*255), 255);
}

#endif

//----------------------------------------------------------------------------------
// Module Functions Definition - OpenGL equivalent functions (common to 1.1, 3.3+, ES2)
//----------------------------------------------------------------------------------

// Enable texture usage
RF_API void rf_gl_enable_texture(unsigned int id)
{
#if defined(RF_GRAPHICS_API_OPENGL_11)
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, id);
#endif

#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    if (_rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].textureId != id)
    {
        if (_rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertex_count > 0)
        {
            // Make sure current _rf_global_context_ptr->gl_ctx.draws[i].vertex_count is aligned a multiple of 4,
            // that way, following QUADS drawing will keep aligned with index processing
            // It implies adding some extra alignment vertex at the end of the draw,
            // those vertex are not processed but they are considered as an additional offset
            // for the next set of vertex to be drawn
            if (_rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].mode == GL_LINES) _rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertexAlignment = ((_rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertex_count < 4)? _rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertex_count : _rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertex_count%4);
            else if (_rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].mode == GL_TRIANGLES) _rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertexAlignment = ((_rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertex_count < 4)? 1 : (4 - (_rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertex_count%4)));

            else _rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertexAlignment = 0;

            if (rf_gl_check_buffer_limit(_rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertexAlignment)) rf_gl_draw();
            else
            {
                _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vCounter += _rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertexAlignment;
                _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].cCounter += _rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertexAlignment;
                _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].tcCounter += _rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertexAlignment;

                _rf_global_context_ptr->gl_ctx.draws_counter++;
            }
        }

        if (_rf_global_context_ptr->gl_ctx.draws_counter >= rf_max_drawcall_registered) rf_gl_draw();

        _rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].textureId = id;
        _rf_global_context_ptr->gl_ctx.draws[_rf_global_context_ptr->gl_ctx.draws_counter - 1].vertex_count = 0;
    }
#endif
}

// Disable texture usage
RF_API void rf_gl_disable_texture()
{
#if defined(RF_GRAPHICS_API_OPENGL_11)
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
#else
    // NOTE: If quads batch limit is reached,
    // we force a draw call and next batch starts
    if (_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vCounter >= (rf_max_batch_elements*4)) rf_gl_draw();
#endif
}

// Set texture parameters (wrap mode/filter mode)
RF_API void rf_gl_texture_parameters(unsigned int id, int param, int value)
{
    glBindTexture(GL_TEXTURE_2D, id);

    switch (param)
    {
        case GL_TEXTURE_WRAP_S:
        case GL_TEXTURE_WRAP_T:
        {
            if (value == GL_MIRROR_CLAMP_EXT)
            {
#if !defined(RF_GRAPHICS_API_OPENGL_11)
                if (_rf_global_context_ptr->gl_ctx.tex_mirror_clamp_supported) glTexParameteri(GL_TEXTURE_2D, param, value);
                else RF_LOG(RF_LOG_WARNING, "Clamp mirror wrap mode not supported");
#endif
            }
            else glTexParameteri(GL_TEXTURE_2D, param, value);

        }
            break;

        case GL_TEXTURE_MAG_FILTER:
        case GL_TEXTURE_MIN_FILTER: glTexParameteri(GL_TEXTURE_2D, param, value); break;
        case GL_TEXTURE_ANISOTROPIC_FILTER:
        {
#if !defined(RF_GRAPHICS_API_OPENGL_11)
            if (value <= _rf_global_context_ptr->gl_ctx.max_anisotropic_level) glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)value);
            else if (_rf_global_context_ptr->gl_ctx.max_anisotropic_level > 0.0f)
            {
                RF_LOG(RF_LOG_WARNING, "[TEX ID %i] Maximum anisotropic filter level supported is %iX", id, _rf_global_context_ptr->gl_ctx.max_anisotropic_level);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)value);
            }
            else RF_LOG(RF_LOG_WARNING, "Anisotropic filtering not supported");
#endif
        }
            break;

        default: break;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

// Enable rendering to texture (fbo)
RF_API void rf_gl_enable_render_texture(unsigned int id)
{
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    //glDisable(GL_CULL_FACE);    // Allow double side drawing for texture flipping
    //glCullFace(GL_FRONT);
#endif
}

// Disable rendering to texture
RF_API void rf_gl_disable_render_texture(void)
{
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
#endif
}

// Enable depth test
RF_API void rf_gl_enable_depth_test(void) { glEnable(GL_DEPTH_TEST); }

// Disable depth test
RF_API void rf_gl_disable_depth_test(void) { glDisable(GL_DEPTH_TEST); }

// Enable backface culling
RF_API void rf_gl_enable_backface_culling(void) { glEnable(GL_CULL_FACE); }

// Disable backface culling
RF_API void rf_gl_disable_backface_culling(void) { glDisable(GL_CULL_FACE); }

// Enable scissor test
RF_API void rf_gl_enable_scissor_test(void) { glEnable(GL_SCISSOR_TEST); }

// Disable scissor test
RF_API void rf_gl_disable_scissor_test(void) { glDisable(GL_SCISSOR_TEST); }

// Scissor test
RF_API void rf_gl_scissor(int x, int y, int width, int height) { glScissor(x, y, width, height); }

// Enable wire mode
RF_API void rf_gl_enable_wire_mode(void)
{
#if defined (RF_GRAPHICS_API_OPENGL_11) || defined(RF_GRAPHICS_API_OPENGL_33)
    // NOTE: glPolygonMode() not available on OpenGL ES
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif
}

// Disable wire mode
RF_API void rf_gl_disable_wire_mode(void)
{
#if defined (RF_GRAPHICS_API_OPENGL_11) || defined(RF_GRAPHICS_API_OPENGL_33)
    // NOTE: glPolygonMode() not available on OpenGL ES
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
}

// Unload texture from GPU memory
RF_API void rf_gl_delete_textures(unsigned int id)
{
    if (id > 0) glDeleteTextures(1, &id);
}

// Unload render texture from GPU memory
RF_API void rf_gl_delete_render_textures(rf_render_texture2d target)
{
    #if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    if (target.texture.id > 0) glDeleteTextures(1, &target.texture.id);
    if (target.depth.id > 0)
    {
        if (target.depth_texture) glDeleteTextures(1, &target.depth.id);
        else glDeleteRenderbuffers(1, &target.depth.id);
    }

    if (target.id > 0) glDeleteFramebuffers(1, &target.id);

    RF_LOG(RF_LOG_INFO, "[FBO ID %i] Unloaded render texture data from VRAM (GPU)", target.id);
    #endif
}

// Unload shader from GPU memory
RF_API void rf_gl_delete_shader(unsigned int id)
{
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    if (id != 0) glDeleteProgram(id);
#endif
}

// Unload vertex data (VAO) from GPU memory
RF_API void rf_gl_delete_vertex_arrays(unsigned int id)
{
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    if (_rf_global_context_ptr->gl_ctx.vao_supported)
    {
        if (id != 0) glDeleteVertexArrays(1, &id);
        RF_LOG(RF_LOG_INFO, "[VAO ID %i] Unloaded model data from VRAM (GPU)", id);
    }
#endif
}

// Unload vertex data (VBO) from GPU memory
RF_API void rf_gl_delete_buffers(unsigned int id)
{
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    if (id != 0)
    {
        glDeleteBuffers(1, &id);
        if (!_rf_global_context_ptr->gl_ctx.vao_supported) RF_LOG(RF_LOG_INFO, "[VBO ID %i] Unloaded model vertex data from VRAM (GPU)", id);
    }
#endif
}

// Clear color buffer with color
RF_API void rf_gl_clear_color(rf_byte r, rf_byte g, rf_byte b, rf_byte a)
{
    // rf_color values clamp to 0.0f(0) and 1.0f(255)
    float cr = (float)r/255;
    float cg = (float)g/255;
    float cb = (float)b/255;
    float ca = (float)a/255;

    glClearColor(cr, cg, cb, ca);
}

// Clear used screen buffers (color and depth)
RF_API void rf_gl_clear_screen_buffers(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear used buffers: rf_color and Depth (Depth is used for 3D)
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);     // Stencil buffer not used...
}

// Update GPU buffer with new data
RF_API void rf_gl_update_buffer(int bufferId, void* data, int dataSize)
{
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    glBindBuffer(GL_ARRAY_BUFFER, bufferId);
    glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, data);
#endif
}

//----------------------------------------------------------------------------------
// Module Functions Definition - rlgl Functions
//----------------------------------------------------------------------------------

// Vertex Buffer Object deinitialization (memory free)
RF_API void rf_gl_close()
{
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    _rf_unlock_shader_default();              // Unload default shader
    _rf_unload_buffers_default();             // Unload default buffers
    glDeleteTextures(1, &_rf_global_context_ptr->gl_ctx.default_texture_id); // Unload default texture

    RF_LOG(RF_LOG_INFO, "[TEX ID %i] Unloaded texture data (base white texture) from VRAM", _rf_global_context_ptr->gl_ctx.default_texture_id);

    RF_FREE(_rf_global_context_ptr->gl_ctx.draws);
#endif
}

// Update and draw internal buffers
RF_API void rf_gl_draw()
{
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    // Only process data if we have data to process
    if (_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vCounter > 0)
    {
        _rf_update_buffers_default();
        _rf_draw_buffers_default();       // NOTE: Stereo rendering is checked inside
    }
#endif
}

// Check internal buffer overflow for a given number of vertex
RF_API bool rf_gl_check_buffer_limit(int vCount)
{
    bool overflow = false;
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    if ((_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vCounter + vCount) >= (rf_max_batch_elements*4)) overflow = true;
#endif
    return overflow;
}

// Set debug marker
RF_API void rf_gl_set_debug_marker(const char* text)
{
#if defined(RF_GRAPHICS_API_OPENGL_33)
    //if (_rf_global_context_ptr->gl_ctx.debug_marker_supported) glInsertEventMarkerEXT(0, text);
#endif
}

// Get world coordinates from screen coordinates
RF_API rf_vector3 rf_gl_unproject(rf_vector3 source, rf_matrix proj, rf_matrix view)
{
    rf_vector3 result = { 0.0f, 0.0f, 0.0f };

    // Calculate unproject matrix (multiply view patrix by _rf_global_context_ptr->gl_ctx.projection matrix) and invert it
    rf_matrix matViewProj = rf_matrix_multiply(view, proj);
    matViewProj = rf_matrix_invert(matViewProj);

    // Create quaternion from source point
    rf_quaternion quat = { source.x, source.y, source.z, 1.0f };

    // Multiply quat point by unproject matrix
    quat = rf_quaternion_transform(quat, matViewProj);

    // Normalized world points in vectors
    result.x = quat.x/quat.w;
    result.y = quat.y/quat.w;
    result.z = quat.z/quat.w;

    return result;
}

// Convert image data to OpenGL texture (returns OpenGL valid Id)
RF_API unsigned int rf_gl_load_texture(void* data, int width, int height, int format, int mipmapCount)
{
    glBindTexture(GL_TEXTURE_2D, 0);    // Free any old binding

    unsigned int id = 0;

    // Check texture format support by OpenGL 1.1 (compressed textures not supported)
#if defined(RF_GRAPHICS_API_OPENGL_11)
    if (format >= rf_compressed_dxt1_rgb)
    {
        RF_LOG(RF_LOG_WARNING, "OpenGL 1.1 does not support GPU compressed texture formats");
        return id;
    }
#else
    if ((!_rf_global_context_ptr->gl_ctx.tex_comp_dxt_supported) && ((format == rf_compressed_dxt1_rgb) || (format == rf_compressed_dxt1_rgba) ||
                                                     (format == rf_compressed_dxt3_rgba) || (format == rf_compressed_dxt5_rgba)))
    {
        RF_LOG(RF_LOG_WARNING, "DXT compressed texture format not supported");
        return id;
    }
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    if ((!_rf_global_context_ptr->gl_ctx.tex_comp_etc1_supported) && (format == rf_compressed_etc1_rgb))
    {
        RF_LOG(RF_LOG_WARNING, "ETC1 compressed texture format not supported");
        return id;
    }

    if ((!_rf_global_context_ptr->gl_ctx.tex_comp_etc2_supported) && ((format == rf_compressed_etc2_rgb) || (format == rf_compressed_etc2_eac_rgba)))
    {
        RF_LOG(RF_LOG_WARNING, "ETC2 compressed texture format not supported");
        return id;
    }

    if ((!_rf_global_context_ptr->gl_ctx.tex_comp_pvrt_supported) && ((format == rf_compressed_pvrt_rgb) || (format == rf_compressed_pvrt_rgba)))
    {
        RF_LOG(RF_LOG_WARNING, "PVRT compressed texture format not supported");
        return id;
    }

    if ((!_rf_global_context_ptr->gl_ctx.tex_comp_astc_supported) && ((format == rf_compressed_astc_4x4_rgba) || (format == rf_compressed_astc_8x8_rgba)))
    {
        RF_LOG(RF_LOG_WARNING, "ASTC compressed texture format not supported");
        return id;
    }
#endif
#endif      // RF_GRAPHICS_API_OPENGL_11

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &id);              // Generate texture id

#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    //glActiveTexture(GL_TEXTURE0);     // If not defined, using GL_TEXTURE0 by default (shader texture)
#endif

    glBindTexture(GL_TEXTURE_2D, id);

    int mipWidth = width;
    int mipHeight = height;
    int mipOffset = 0;          // Mipmap data offset

    RF_LOG(RF_LOG_DEBUG, "Load texture from data memory address: 0x%x", data);

    // Load the different mipmap levels
    for (int i = 0; i < mipmapCount; i++)
    {
        unsigned int mipSize = rf_get_pixel_data_size(mipWidth, mipHeight, format);

        unsigned int glInternalFormat, glFormat, glType;
        rf_gl_get_gl_texture_formats(format, &glInternalFormat, &glFormat, &glType);

        RF_LOG(RF_LOG_DEBUG, "Load mipmap level %i (%i x %i), size: %i, offset: %i", i, mipWidth, mipHeight, mipSize, mipOffset);

        if (glInternalFormat != -1)
        {
            if (format < rf_compressed_dxt1_rgb) glTexImage2D(GL_TEXTURE_2D, i, glInternalFormat, mipWidth, mipHeight, 0, glFormat, glType, (unsigned char* )data + mipOffset);
#if !defined(RF_GRAPHICS_API_OPENGL_11)
            else glCompressedTexImage2D(GL_TEXTURE_2D, i, glInternalFormat, mipWidth, mipHeight, 0, mipSize, (unsigned char* )data + mipOffset);
#endif

#if defined(RF_GRAPHICS_API_OPENGL_33)
            if (format == rf_uncompressed_grayscale)
            {
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
                glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
            else if (format == rf_uncompressed_gray_alpha)
            {
#if defined(RF_GRAPHICS_API_OPENGL_21)
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ALPHA };
#elif defined(RF_GRAPHICS_API_OPENGL_33)
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_GREEN };
#endif
                glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
#endif
        }

        mipWidth /= 2;
        mipHeight /= 2;
        mipOffset += mipSize;

        // Security check for NPOT textures
        if (mipWidth < 1) mipWidth = 1;
        if (mipHeight < 1) mipHeight = 1;
    }

    // rf_texture parameters configuration
    // NOTE: glTexParameteri does NOT affect texture uploading, just the way it's used
#if defined(RF_GRAPHICS_API_OPENGL_ES2)
    // NOTE: OpenGL ES 2.0 with no GL_OES_texture_npot support (i.e. WebGL) has limited NPOT support, so CLAMP_TO_EDGE must be used
    if (_rf_global_context_ptr->gl_ctx.tex_npot_supported)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // Set texture to repeat on x-axis
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);       // Set texture to repeat on y-axis
    }
    else
    {
        // NOTE: If using negative texture coordinates (LoadOBJ()), it does not work!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);       // Set texture to clamp on x-axis
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);       // Set texture to clamp on y-axis
    }
#else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // Set texture to repeat on x-axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);       // Set texture to repeat on y-axis
#endif

    // Magnification and minification filters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // Alternative: GL_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // Alternative: GL_LINEAR

#if defined(RF_GRAPHICS_API_OPENGL_33)
    if (mipmapCount > 1)
    {
        // Activate Trilinear filtering if mipmaps are available
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
#endif

    // At this point we have the texture loaded in GPU and texture parameters configured

    // NOTE: If mipmaps were not in data, they are not generated automatically

    // Unbind current texture
    glBindTexture(GL_TEXTURE_2D, 0);

    if (id > 0) RF_LOG(RF_LOG_INFO, "[TEX ID %i] rf_texture created successfully (%ix%i - %i mipmaps)", id, width, height, mipmapCount);
    else RF_LOG(RF_LOG_WARNING, "rf_texture could not be created");

    return id;
}

// Load depth texture/renderbuffer (to be attached to fbo)
// WARNING: OpenGL ES 2.0 requires GL_OES_depth_texture/WEBGL_depth_texture extensions
RF_API unsigned int rf_gl_load_texture_depth(int width, int height, int bits, bool useRenderBuffer)
{
    unsigned int id = 0;

#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    unsigned int glInternalFormat = GL_DEPTH_COMPONENT16;

    if ((bits != 16) && (bits != 24) && (bits != 32)) bits = 16;

    if (bits == 24)
    {
#if defined(RF_GRAPHICS_API_OPENGL_33)
        glInternalFormat = GL_DEPTH_COMPONENT24;
#elif defined(RF_GRAPHICS_API_OPENGL_ES2)
        if (_rf_global_context_ptr->gl_ctx.max_depth_bits >= 24) glInternalFormat = GL_DEPTH_COMPONENT24_OES;
#endif
    }

    if (bits == 32)
    {
#if defined(RF_GRAPHICS_API_OPENGL_33)
        glInternalFormat = GL_DEPTH_COMPONENT32;
#elif defined(RF_GRAPHICS_API_OPENGL_ES2)
        if (_rf_global_context_ptr->gl_ctx.max_depth_bits == 32) glInternalFormat = GL_DEPTH_COMPONENT32_OES;
#endif
    }

    if (!useRenderBuffer && _rf_global_context_ptr->gl_ctx.tex_depth_supported)
    {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else
    {
        // Create the renderbuffer that will serve as the depth attachment for the framebuffer
        // NOTE: A renderbuffer is simpler than a texture and could offer better performance on embedded devices
        glGenRenderbuffers(1, &id);
        glBindRenderbuffer(GL_RENDERBUFFER, id);
        glRenderbufferStorage(GL_RENDERBUFFER, glInternalFormat, width, height);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
#endif

    return id;
}

// Load texture cubemap
// NOTE: Cubemap data is expected to be 6 images in a single column,
// expected the following convention: +X, -X, +Y, -Y, +Z, -Z
RF_API unsigned int rf_gl_load_texture_cubemap(void* data, int size, int format)
{
    unsigned int cubemapId = 0;
    unsigned int dataSize = rf_get_pixel_data_size(size, size, format);

#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    glGenTextures(1, &cubemapId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapId);

    unsigned int glInternalFormat, glFormat, glType;
    rf_gl_get_gl_texture_formats(format, &glInternalFormat, &glFormat, &glType);

    if (glInternalFormat != -1)
    {
        // Load cubemap faces
        for (unsigned int i = 0; i < 6; i++)
        {
            if (format < rf_compressed_dxt1_rgb) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glInternalFormat, size, size, 0, glFormat, glType, (unsigned char* )data + i*dataSize);
#if !defined(RF_GRAPHICS_API_OPENGL_11)
            else glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glInternalFormat, size, size, 0, dataSize, (unsigned char* )data + i*dataSize);
#endif
#if defined(RF_GRAPHICS_API_OPENGL_33)
            if (format == rf_uncompressed_grayscale)
            {
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
                glTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
            else if (format == rf_uncompressed_gray_alpha)
            {
#if defined(RF_GRAPHICS_API_OPENGL_21)
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ALPHA };
#elif defined(RF_GRAPHICS_API_OPENGL_33)
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_GREEN };
#endif
                glTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
#endif
        }
    }

    // Set cubemap texture sampling parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if defined(RF_GRAPHICS_API_OPENGL_33)
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  // Flag not supported on OpenGL ES 2.0
#endif

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
#endif

    return cubemapId;
}

// Update already loaded texture in GPU with new data
// NOTE: We don't know safely if internal texture format is the expected one...
RF_API void rf_gl_update_texture(unsigned int id, int width, int height, int format, const void* data)
{
    glBindTexture(GL_TEXTURE_2D, id);

    unsigned int glInternalFormat, glFormat, glType;
    rf_gl_get_gl_texture_formats(format, &glInternalFormat, &glFormat, &glType);

    if ((glInternalFormat != -1) && (format < rf_compressed_dxt1_rgb))
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, glFormat, glType, (unsigned char* )data);
    }
    else RF_LOG(RF_LOG_WARNING, "rf_texture format updating not supported");
}

// Get OpenGL internal formats and data type from raylib rf_pixel_format
RF_API void rf_gl_get_gl_texture_formats(int format, unsigned int* glInternalFormat, unsigned int* glFormat, unsigned int* glType)
{
    *glInternalFormat = -1;
    *glFormat = -1;
    *glType = -1;

    switch (format)
    {
#if defined(RF_GRAPHICS_API_OPENGL_11) || defined(RF_GRAPHICS_API_OPENGL_21) || defined(RF_GRAPHICS_API_OPENGL_ES2)
        // NOTE: on OpenGL ES 2.0 (WebGL), internalFormat must match format and options allowed are: GL_LUMINANCE, GL_RGB, GL_RGBA
        case rf_uncompressed_grayscale: *glInternalFormat = GL_LUMINANCE; *glFormat = GL_LUMINANCE; *glType = GL_UNSIGNED_BYTE; break;
        case rf_uncompressed_gray_alpha: *glInternalFormat = GL_LUMINANCE_ALPHA; *glFormat = GL_LUMINANCE_ALPHA; *glType = GL_UNSIGNED_BYTE; break;
        case rf_uncompressed_r5g6b5: *glInternalFormat = GL_RGB; *glFormat = GL_RGB; *glType = GL_UNSIGNED_SHORT_5_6_5; break;
        case rf_uncompressed_r8g8b8: *glInternalFormat = GL_RGB; *glFormat = GL_RGB; *glType = GL_UNSIGNED_BYTE; break;
        case rf_uncompressed_r5g5b5a1: *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_5_5_5_1; break;
        case rf_uncompressed_r4g4b4a4: *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_4_4_4_4; break;
        case rf_uncompressed_r8g8b8a8: *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_BYTE; break;
#if !defined(RF_GRAPHICS_API_OPENGL_11)
        case rf_uncompressed_r32: if (_rf_global_context_ptr->gl_ctx.tex_float_supported) *glInternalFormat = GL_LUMINANCE; *glFormat = GL_LUMINANCE; *glType = GL_FLOAT; break;   // NOTE: Requires extension OES_texture_float
        case rf_uncompressed_r32g32b32: if (_rf_global_context_ptr->gl_ctx.tex_float_supported) *glInternalFormat = GL_RGB; *glFormat = GL_RGB; *glType = GL_FLOAT; break;         // NOTE: Requires extension OES_texture_float
        case rf_uncompressed_r32g32b32a32: if (_rf_global_context_ptr->gl_ctx.tex_float_supported) *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_FLOAT; break;    // NOTE: Requires extension OES_texture_float
#endif
#elif defined(RF_GRAPHICS_API_OPENGL_33)
        case rf_uncompressed_grayscale: *glInternalFormat = GL_R8; *glFormat = GL_RED; *glType = GL_UNSIGNED_BYTE; break;
        case rf_uncompressed_gray_alpha: *glInternalFormat = GL_RG8; *glFormat = GL_RG; *glType = GL_UNSIGNED_BYTE; break;
            //case rf_uncompressed_r5g6b5: *glInternalFormat = GL_RGB565; *glFormat = GL_RGB; *glType = GL_UNSIGNED_SHORT_5_6_5; break;
        case rf_uncompressed_r8g8b8: *glInternalFormat = GL_RGB8; *glFormat = GL_RGB; *glType = GL_UNSIGNED_BYTE; break;
        case rf_uncompressed_r5g5b5a1: *glInternalFormat = GL_RGB5_A1; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_5_5_5_1; break;
        case rf_uncompressed_r4g4b4a4: *glInternalFormat = GL_RGBA4; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_4_4_4_4; break;
        case rf_uncompressed_r8g8b8a8: *glInternalFormat = GL_RGBA8; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_BYTE; break;
        case rf_uncompressed_r32: if (_rf_global_context_ptr->gl_ctx.tex_float_supported) *glInternalFormat = GL_R32F; *glFormat = GL_RED; *glType = GL_FLOAT; break;
        case rf_uncompressed_r32g32b32: if (_rf_global_context_ptr->gl_ctx.tex_float_supported) *glInternalFormat = GL_RGB32F; *glFormat = GL_RGB; *glType = GL_FLOAT; break;
        case rf_uncompressed_r32g32b32a32: if (_rf_global_context_ptr->gl_ctx.tex_float_supported) *glInternalFormat = GL_RGBA32F; *glFormat = GL_RGBA; *glType = GL_FLOAT; break;
#endif
#if !defined(RF_GRAPHICS_API_OPENGL_11)
        case rf_compressed_dxt1_rgb: if (_rf_global_context_ptr->gl_ctx.tex_comp_dxt_supported) *glInternalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
        case rf_compressed_dxt1_rgba: if (_rf_global_context_ptr->gl_ctx.tex_comp_dxt_supported) *glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
        case rf_compressed_dxt3_rgba: if (_rf_global_context_ptr->gl_ctx.tex_comp_dxt_supported) *glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break;
        case rf_compressed_dxt5_rgba: if (_rf_global_context_ptr->gl_ctx.tex_comp_dxt_supported) *glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
        case rf_compressed_etc1_rgb: if (_rf_global_context_ptr->gl_ctx.tex_comp_etc1_supported) *glInternalFormat = GL_ETC1_RGB8_OES; break;                      // NOTE: Requires OpenGL ES 2.0 or OpenGL 4.3
        case rf_compressed_etc2_rgb: if (_rf_global_context_ptr->gl_ctx.tex_comp_etc2_supported) *glInternalFormat = GL_COMPRESSED_RGB8_ETC2; break;               // NOTE: Requires OpenGL ES 3.0 or OpenGL 4.3
        case rf_compressed_etc2_eac_rgba: if (_rf_global_context_ptr->gl_ctx.tex_comp_etc2_supported) *glInternalFormat = GL_COMPRESSED_RGBA8_ETC2_EAC; break;     // NOTE: Requires OpenGL ES 3.0 or OpenGL 4.3
        case rf_compressed_pvrt_rgb: if (_rf_global_context_ptr->gl_ctx.tex_comp_pvrt_supported) *glInternalFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG; break;    // NOTE: Requires PowerVR GPU
        case rf_compressed_pvrt_rgba: if (_rf_global_context_ptr->gl_ctx.tex_comp_pvrt_supported) *glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG; break;  // NOTE: Requires PowerVR GPU
        case rf_compressed_astc_4x4_rgba: if (_rf_global_context_ptr->gl_ctx.tex_comp_astc_supported) *glInternalFormat = GL_COMPRESSED_RGBA_ASTC_4x4_KHR; break;  // NOTE: Requires OpenGL ES 3.1 or OpenGL 4.3
        case rf_compressed_astc_8x8_rgba: if (_rf_global_context_ptr->gl_ctx.tex_comp_astc_supported) *glInternalFormat = GL_COMPRESSED_RGBA_ASTC_8x8_KHR; break;  // NOTE: Requires OpenGL ES 3.1 or OpenGL 4.3
#endif
        default: RF_LOG(RF_LOG_WARNING, "rf_texture format not supported"); break;
    }
}

// Unload texture from GPU memory
RF_API void rf_gl_unload_texture(unsigned int id)
{
    if (id > 0) glDeleteTextures(1, &id);
}

// Load a texture to be used for rendering (fbo with default color and depth attachments)
// NOTE: If colorFormat or depthBits are no supported, no attachment is done
RF_API rf_render_texture2d rf_gl_load_render_texture(int width, int height, int format, int depthBits, bool useDepthTexture)
{
    rf_render_texture2d target = { 0 };

#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    if (useDepthTexture && _rf_global_context_ptr->gl_ctx.tex_depth_supported) target.depth_texture = true;

    // Create the framebuffer object
    glGenFramebuffers(1, &target.id);
    glBindFramebuffer(GL_FRAMEBUFFER, target.id);

    // Create fbo color texture attachment
    //-----------------------------------------------------------------------------------------------------
    if ((format != -1) && (format < rf_compressed_dxt1_rgb))
    {
        // WARNING: Some texture formats are not supported for fbo color attachment
        target.texture.id = rf_gl_load_texture(NULL, width, height, format, 1);
        target.texture.width = width;
        target.texture.height = height;
        target.texture.format = format;
        target.texture.mipmaps = 1;
    }
    //-----------------------------------------------------------------------------------------------------

    // Create fbo depth renderbuffer/texture
    //-----------------------------------------------------------------------------------------------------
    if (depthBits > 0)
    {
        target.depth.id = rf_gl_load_texture_depth(width, height, depthBits, !useDepthTexture);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19;       //DEPTH_COMPONENT_24BIT?
        target.depth.mipmaps = 1;
    }
    //-----------------------------------------------------------------------------------------------------

    // Attach color texture and depth renderbuffer to FBO
    //-----------------------------------------------------------------------------------------------------
    rf_gl_render_texture_attach(target, target.texture.id, 0);    // COLOR attachment
    rf_gl_render_texture_attach(target, target.depth.id, 1);      // DEPTH attachment
    //-----------------------------------------------------------------------------------------------------

    // Check if fbo is complete with attachments (valid)
    //-----------------------------------------------------------------------------------------------------
    if (rf_gl_render_texture_complete(target)) RF_LOG(RF_LOG_INFO, "[FBO ID %i] Framebuffer object created successfully", target.id);
    //-----------------------------------------------------------------------------------------------------

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

    return target;
}

// Attach color buffer texture to an fbo (unloads previous attachment)
// NOTE: Attach type: 0-rf_color, 1-Depth renderbuffer, 2-Depth texture
RF_API void rf_gl_render_texture_attach(rf_render_texture2d target, unsigned int id, int attachType)
{
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    glBindFramebuffer(GL_FRAMEBUFFER, target.id);

    if (attachType == 0) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0);
    else if (attachType == 1)
    {
        if (target.depth_texture) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, id, 0);
        else glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, id);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}

// Verify render texture is complete
RF_API bool rf_gl_render_texture_complete(rf_render_texture target)
{
    bool result = false;

#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    glBindFramebuffer(GL_FRAMEBUFFER, target.id);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        switch (status)
        {
            case GL_FRAMEBUFFER_UNSUPPORTED: RF_LOG(RF_LOG_WARNING, "Framebuffer is unsupported"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: RF_LOG(RF_LOG_WARNING, "Framebuffer has incomplete attachment"); break;
#if defined(RF_GRAPHICS_API_OPENGL_ES2)
                case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS: RF_LOG(RF_LOG_WARNING, "Framebuffer has incomplete dimensions"); break;
#endif
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: RF_LOG(RF_LOG_WARNING, "Framebuffer has a missing attachment"); break;
            default: break;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    result = (status == GL_FRAMEBUFFER_COMPLETE);
#endif

    return result;
}

// Generate mipmap data for selected texture
RF_API void rf_gl_generate_mipmaps(rf_texture2d* texture)
{
    glBindTexture(GL_TEXTURE_2D, texture->id);

    // Check if texture is power-of-two (POT)
    bool texIsPOT = false;

    if (((texture->width > 0) && ((texture->width & (texture->width - 1)) == 0)) &&
        ((texture->height > 0) && ((texture->height & (texture->height - 1)) == 0))) texIsPOT = true;

#if defined(RF_GRAPHICS_API_OPENGL_11)
    if (texIsPOT)
    {
        // WARNING: Manual mipmap generation only works for RGBA 32bit textures!
        if (texture->format == rf_uncompressed_r8g8b8a8)
        {
            // Retrieve texture data from VRAM
            void* data = rf_gl_read_texture_pixels(*texture);

            // NOTE: data size is reallocated to fit mipmaps data
            // NOTE: CPU mipmap generation only supports RGBA 32bit data
            int mipmapCount = _rf_generate_mipmaps(data, texture->width, texture->height);

            int size = texture->width*texture->height*4;
            int offset = size;

            int mipWidth = texture->width/2;
            int mipHeight = texture->height/2;

            // Load the mipmaps
            for (int level = 1; level < mipmapCount; level++)
            {
                glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA8, mipWidth, mipHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char* )data + offset);

                size = mipWidth*mipHeight*4;
                offset += size;

                mipWidth /= 2;
                mipHeight /= 2;
            }

            texture->mipmaps = mipmapCount + 1;
            RF_FREE(data); // Once mipmaps have been generated and data has been uploaded to GPU VRAM, we can discard RAM data

            RF_LOG(RF_LOG_WARNING, "[TEX ID %i] Mipmaps [%i] generated manually on CPU side", texture->id, texture->mipmaps);
        }
        else RF_LOG(RF_LOG_WARNING, "[TEX ID %i] Mipmaps could not be generated for texture format", texture->id);
    }
#elif defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    if ((texIsPOT) || (_rf_global_context_ptr->gl_ctx.tex_npot_supported))
    {
        //glHint(GL_GENERATE_MIPMAP_HINT, GL_DONT_CARE);   // Hint for mipmaps generation algorythm: GL_FASTEST, GL_NICEST, GL_DONT_CARE
        glGenerateMipmap(GL_TEXTURE_2D);    // Generate mipmaps automatically
        RF_LOG(RF_LOG_INFO, "[TEX ID %i] Mipmaps generated automatically", texture->id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);   // Activate Trilinear filtering for mipmaps

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

        texture->mipmaps =  1 + (int)floor(log(MAX(texture->width, texture->height))/log(2));
    }
#endif
    else RF_LOG(RF_LOG_WARNING, "[TEX ID %i] Mipmaps can not be generated", texture->id);

    glBindTexture(GL_TEXTURE_2D, 0);
}

// Upload vertex data into a VAO (if supported) and VBO
RF_API void rf_gl_load_mesh(rf_mesh* mesh, bool dynamic)
{
    if (mesh->vao_id > 0)
    {
        // Check if mesh has already been loaded in GPU
        RF_LOG(RF_LOG_WARNING, "Trying to re-load an already loaded mesh");
        return;
    }

    mesh->vao_id = 0;        // Vertex Array Object
    mesh->vbo_id[0] = 0;     // Vertex positions VBO
    mesh->vbo_id[1] = 0;     // Vertex texcoords VBO
    mesh->vbo_id[2] = 0;     // Vertex normals VBO
    mesh->vbo_id[3] = 0;     // Vertex colors VBO
    mesh->vbo_id[4] = 0;     // Vertex tangents VBO
    mesh->vbo_id[5] = 0;     // Vertex texcoords2 VBO
    mesh->vbo_id[6] = 0;     // Vertex indices VBO

#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    int drawHint = GL_STATIC_DRAW;
    if (dynamic) drawHint = GL_DYNAMIC_DRAW;

    if (_rf_global_context_ptr->gl_ctx.vao_supported)
    {
        // Initialize Quads VAO (Buffer A)
        glGenVertexArrays(1, &mesh->vao_id);
        glBindVertexArray(mesh->vao_id);
    }

    // NOTE: Attributes must be uploaded considering default locations points

    // Enable vertex attributes: position (shader-location = 0)
    glGenBuffers(1, &mesh->vbo_id[0]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_id[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*mesh->vertex_count, mesh->vertices, drawHint);
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(0);

    // Enable vertex attributes: texcoords (shader-location = 1)
    glGenBuffers(1, &mesh->vbo_id[1]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_id[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*mesh->vertex_count, mesh->texcoords, drawHint);
    glVertexAttribPointer(1, 2, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(1);

    // Enable vertex attributes: normals (shader-location = 2)
    if (mesh->normals != NULL)
    {
        glGenBuffers(1, &mesh->vbo_id[2]);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_id[2]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*mesh->vertex_count, mesh->normals, drawHint);
        glVertexAttribPointer(2, 3, GL_FLOAT, 0, 0, 0);
        glEnableVertexAttribArray(2);
    }
    else
    {
        // Default color vertex attribute set to rf_white
        glVertexAttrib3f(2, 1.0f, 1.0f, 1.0f);
        glDisableVertexAttribArray(2);
    }

    // Default color vertex attribute (shader-location = 3)
    if (mesh->colors != NULL)
    {
        glGenBuffers(1, &mesh->vbo_id[3]);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_id[3]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned char)*4*mesh->vertex_count, mesh->colors, drawHint);
        glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
        glEnableVertexAttribArray(3);
    }
    else
    {
        // Default color vertex attribute set to rf_white
        glVertexAttrib4f(3, 1.0f, 1.0f, 1.0f, 1.0f);
        glDisableVertexAttribArray(3);
    }

    // Default tangent vertex attribute (shader-location = 4)
    if (mesh->tangents != NULL)
    {
        glGenBuffers(1, &mesh->vbo_id[4]);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_id[4]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*mesh->vertex_count, mesh->tangents, drawHint);
        glVertexAttribPointer(4, 4, GL_FLOAT, 0, 0, 0);
        glEnableVertexAttribArray(4);
    }
    else
    {
        // Default tangents vertex attribute
        glVertexAttrib4f(4, 0.0f, 0.0f, 0.0f, 0.0f);
        glDisableVertexAttribArray(4);
    }

    // Default texcoord2 vertex attribute (shader-location = 5)
    if (mesh->texcoords2 != NULL)
    {
        glGenBuffers(1, &mesh->vbo_id[5]);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_id[5]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*mesh->vertex_count, mesh->texcoords2, drawHint);
        glVertexAttribPointer(5, 2, GL_FLOAT, 0, 0, 0);
        glEnableVertexAttribArray(5);
    }
    else
    {
        // Default texcoord2 vertex attribute
        glVertexAttrib2f(5, 0.0f, 0.0f);
        glDisableVertexAttribArray(5);
    }

    if (mesh->indices != NULL)
    {
        glGenBuffers(1, &mesh->vbo_id[6]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->vbo_id[6]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*mesh->triangle_count*3, mesh->indices, drawHint);
    }

    if (_rf_global_context_ptr->gl_ctx.vao_supported)
    {
        if (mesh->vao_id > 0) RF_LOG(RF_LOG_INFO, "[VAO ID %i] rf_mesh uploaded successfully to VRAM (GPU)", mesh->vao_id);
        else RF_LOG(RF_LOG_WARNING, "rf_mesh could not be uploaded to VRAM (GPU)");
    }
    else
    {
        RF_LOG(RF_LOG_INFO, "[VBOs] rf_mesh uploaded successfully to VRAM (GPU)");
    }
#endif
}

// Load a new attributes buffer
RF_API unsigned int rf_gl_load_attrib_buffer(unsigned int vao_id, int shaderLoc, void* buffer, int size, bool dynamic)
{
    unsigned int id = 0;

#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    int drawHint = GL_STATIC_DRAW;
    if (dynamic) drawHint = GL_DYNAMIC_DRAW;

    if (_rf_global_context_ptr->gl_ctx.vao_supported) glBindVertexArray(vao_id);

    glGenBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, size, buffer, drawHint);
    glVertexAttribPointer(shaderLoc, 2, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(shaderLoc);

    if (_rf_global_context_ptr->gl_ctx.vao_supported) glBindVertexArray(0);
#endif

    return id;
}

// Update vertex or index data on GPU (upload new data to one buffer)
RF_API void rf_gl_update_mesh(rf_mesh mesh, int buffer, int num)
{
    rf_gl_update_mesh_at(mesh, buffer, num, 0);
}

// Update vertex or index data on GPU, at index
// WARNING: error checking is in place that will cause the data to not be
//          updated if offset + size exceeds what the buffer can hold
RF_API void rf_gl_update_mesh_at(rf_mesh mesh, int buffer, int num, int index)
{
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    // Activate mesh VAO
    if (_rf_global_context_ptr->gl_ctx.vao_supported) glBindVertexArray(mesh.vao_id);

    switch (buffer)
    {
        case 0:     // Update vertices (vertex position)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_id[0]);
            if (index == 0 && num >= mesh.vertex_count) glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*num, mesh.vertices, GL_DYNAMIC_DRAW);
            else if (index + num >= mesh.vertex_count) break;
            else glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*3*index, sizeof(float)*3*num, mesh.vertices);

        } break;
        case 1:     // Update texcoords (vertex texture coordinates)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_id[1]);
            if (index == 0 && num >= mesh.vertex_count) glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*num, mesh.texcoords, GL_DYNAMIC_DRAW);
            else if (index + num >= mesh.vertex_count) break;
            else glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*2*index, sizeof(float)*2*num, mesh.texcoords);

        } break;
        case 2:     // Update normals (vertex normals)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_id[2]);
            if (index == 0 && num >= mesh.vertex_count) glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*num, mesh.normals, GL_DYNAMIC_DRAW);
            else if (index + num >= mesh.vertex_count) break;
            else glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*3*index, sizeof(float)*3*num, mesh.normals);

        } break;
        case 3:     // Update colors (vertex colors)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_id[3]);
            if (index == 0 && num >= mesh.vertex_count) glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*num, mesh.colors, GL_DYNAMIC_DRAW);
            else if (index + num >= mesh.vertex_count) break;
            else glBufferSubData(GL_ARRAY_BUFFER, sizeof(unsigned char)*4*index, sizeof(unsigned char)*4*num, mesh.colors);

        } break;
        case 4:     // Update tangents (vertex tangents)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_id[4]);
            if (index == 0 && num >= mesh.vertex_count) glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*num, mesh.tangents, GL_DYNAMIC_DRAW);
            else if (index + num >= mesh.vertex_count) break;
            else glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*4*index, sizeof(float)*4*num, mesh.tangents);
        } break;
        case 5:     // Update texcoords2 (vertex second texture coordinates)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_id[5]);
            if (index == 0 && num >= mesh.vertex_count) glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*num, mesh.texcoords2, GL_DYNAMIC_DRAW);
            else if (index + num >= mesh.vertex_count) break;
            else glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*2*index, sizeof(float)*2*num, mesh.texcoords2);
        } break;
        case 6:     // Update indices (triangle index buffer)
        {
            // the * 3 is because each triangle has 3 indices
            unsigned short *indices = mesh.indices;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbo_id[6]);
            if (index == 0 && num >= mesh.triangle_count)
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*indices)*num*3, indices, GL_DYNAMIC_DRAW);
            else if (index + num >= mesh.triangle_count)
                break;
            else
                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*indices)*index*3, sizeof(*indices)*num*3, indices);
        } break;
        default: break;
    }

    // Unbind the current VAO
    if (_rf_global_context_ptr->gl_ctx.vao_supported) glBindVertexArray(0);

    // Another option would be using buffer mapping...
    //mesh.vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
    // Now we can modify vertices
    //glUnmapBuffer(GL_ARRAY_BUFFER);
#endif
}

// Draw a 3d mesh with material and transform
RF_API void rf_gl_draw_mesh(rf_mesh mesh, rf_material material, rf_matrix transform)
{
#if defined(RF_GRAPHICS_API_OPENGL_11)
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, material.maps[rf_map_diffuse].texture.id);

    // NOTE: On OpenGL 1.1 we use Vertex Arrays to draw model
    glEnableClientState(GL_VERTEX_ARRAY);                   // Enable vertex array
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);            // Enable texture coords array
    if (mesh.normals != NULL) glEnableClientState(GL_NORMAL_ARRAY);     // Enable normals array
    if (mesh.colors != NULL) glEnableClientState(GL_COLOR_ARRAY);       // Enable colors array

    glVertexPointer(3, GL_FLOAT, 0, mesh.vertices);         // Pointer to vertex coords array
    glTexCoordPointer(2, GL_FLOAT, 0, mesh.texcoords);      // Pointer to texture coords array
    if (mesh.normals != NULL) glNormalPointer(GL_FLOAT, 0, mesh.normals);           // Pointer to normals array
    if (mesh.colors != NULL) glColorPointer(4, GL_UNSIGNED_BYTE, 0, mesh.colors);   // Pointer to colors array

    rf_push_matrix();
        rf_mult_matrixf(rf_matrix_to_floatv(transform).v);
        rf_gl_color4ub(material.maps[rf_map_diffuse].color.r, material.maps[rf_map_diffuse].color.g, material.maps[rf_map_diffuse].color.b, material.maps[rf_map_diffuse].color.a);

        if (mesh.indices != NULL) glDrawElements(GL_TRIANGLES, mesh.triangle_count*3, GL_UNSIGNED_SHORT, mesh.indices);
        else glDrawArrays(GL_TRIANGLES, 0, mesh.vertex_count);
    rf_pop_matrix();

    glDisableClientState(GL_VERTEX_ARRAY);                  // Disable vertex array
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);           // Disable texture coords array
    if (mesh.normals != NULL) glDisableClientState(GL_NORMAL_ARRAY);    // Disable normals array
    if (mesh.colors != NULL) glDisableClientState(GL_NORMAL_ARRAY);     // Disable colors array

    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
#endif

#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    // Bind shader program
    glUseProgram(material.shader.id);

    // Matrices and other values required by shader
    //-----------------------------------------------------
    // Calculate and send to shader model matrix (used by PBR shader)
    if (material.shader.locs[rf_loc_matrix_model] != -1) rf_set_shader_value_matrix(material.shader, material.shader.locs[rf_loc_matrix_model], transform);

    // Upload to shader material.colDiffuse
    if (material.shader.locs[rf_loc_color_diffuse] != -1)
        glUniform4f(material.shader.locs[rf_loc_color_diffuse], (float)material.maps[rf_map_diffuse].color.r/255.0f,
                    (float)material.maps[rf_map_diffuse].color.g/255.0f,
                    (float)material.maps[rf_map_diffuse].color.b/255.0f,
                    (float)material.maps[rf_map_diffuse].color.a/255.0f);

    // Upload to shader material.colSpecular (if available)
    if (material.shader.locs[rf_loc_color_specular] != -1)
        glUniform4f(material.shader.locs[rf_loc_color_specular], (float)material.maps[rf_map_specular].color.r/255.0f,
                    (float)material.maps[rf_map_specular].color.g/255.0f,
                    (float)material.maps[rf_map_specular].color.b/255.0f,
                    (float)material.maps[rf_map_specular].color.a/255.0f);

    if (material.shader.locs[rf_loc_matrix_view] != -1) rf_set_shader_value_matrix(material.shader, material.shader.locs[rf_loc_matrix_view], _rf_global_context_ptr->gl_ctx.modelview);
    if (material.shader.locs[rf_loc_matrix_projection] != -1) rf_set_shader_value_matrix(material.shader, material.shader.locs[rf_loc_matrix_projection], _rf_global_context_ptr->gl_ctx.projection);

    // At this point the _rf_global_context_ptr->gl_ctx.modelview matrix just contains the view matrix (camera)
    // That's because rf_begin_mode3d() sets it an no model-drawing function modifies it, all use rf_push_matrix() and rf_pop_matrix()
    rf_matrix matView = _rf_global_context_ptr->gl_ctx.modelview;         // View matrix (camera)
    rf_matrix matProjection = _rf_global_context_ptr->gl_ctx.projection;  // Projection matrix (perspective)

    // TODO: Consider possible transform matrices in the _rf_global_context_ptr->gl_ctx.stack
    // Is this the right order? or should we start with the first stored matrix instead of the last one?
    //rf_matrix matStackTransform = rf_matrix_identity();
    //for (int i = _rf_global_context_ptr->gl_ctx.stack_counter; i > 0; i--) matStackTransform = rf_matrix_multiply(_rf_global_context_ptr->gl_ctx.stack[i], matStackTransform);

    // rf_transform to camera-space coordinates
    rf_matrix matModelView = rf_matrix_multiply(transform, rf_matrix_multiply(_rf_global_context_ptr->gl_ctx.transform_matrix, matView));
    //-----------------------------------------------------

    // Bind active texture maps (if available)
    for (int i = 0; i < rf_max_material_maps; i++)
    {
        if (material.maps[i].texture.id > 0)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            if ((i == rf_map_irradiance) || (i == rf_map_prefilter) || (i == rf_map_cubemap)) glBindTexture(GL_TEXTURE_CUBE_MAP, material.maps[i].texture.id);
            else glBindTexture(GL_TEXTURE_2D, material.maps[i].texture.id);

            glUniform1i(material.shader.locs[rf_loc_map_diffuse + i], i);
        }
    }

    // Bind vertex array objects (or VBOs)
    if (_rf_global_context_ptr->gl_ctx.vao_supported) glBindVertexArray(mesh.vao_id);
    else
    {
        // Bind mesh VBO data: vertex position (shader-location = 0)
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_id[0]);
        glVertexAttribPointer(material.shader.locs[rf_loc_vertex_position], 3, GL_FLOAT, 0, 0, 0);
        glEnableVertexAttribArray(material.shader.locs[rf_loc_vertex_position]);

        // Bind mesh VBO data: vertex texcoords (shader-location = 1)
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_id[1]);
        glVertexAttribPointer(material.shader.locs[rf_loc_vertex_texcoord01], 2, GL_FLOAT, 0, 0, 0);
        glEnableVertexAttribArray(material.shader.locs[rf_loc_vertex_texcoord01]);

        // Bind mesh VBO data: vertex normals (shader-location = 2, if available)
        if (material.shader.locs[rf_loc_vertex_normal] != -1)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_id[2]);
            glVertexAttribPointer(material.shader.locs[rf_loc_vertex_normal], 3, GL_FLOAT, 0, 0, 0);
            glEnableVertexAttribArray(material.shader.locs[rf_loc_vertex_normal]);
        }

        // Bind mesh VBO data: vertex colors (shader-location = 3, if available)
        if (material.shader.locs[rf_loc_vertex_color] != -1)
        {
            if (mesh.vbo_id[3] != 0)
            {
                glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_id[3]);
                glVertexAttribPointer(material.shader.locs[rf_loc_vertex_color], 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
                glEnableVertexAttribArray(material.shader.locs[rf_loc_vertex_color]);
            }
            else
            {
                // Set default value for unused attribute
                // NOTE: Required when using default shader and no VAO support
                glVertexAttrib4f(material.shader.locs[rf_loc_vertex_color], 1.0f, 1.0f, 1.0f, 1.0f);
                glDisableVertexAttribArray(material.shader.locs[rf_loc_vertex_color]);
            }
        }

        // Bind mesh VBO data: vertex tangents (shader-location = 4, if available)
        if (material.shader.locs[rf_loc_vertex_tangent] != -1)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_id[4]);
            glVertexAttribPointer(material.shader.locs[rf_loc_vertex_tangent], 4, GL_FLOAT, 0, 0, 0);
            glEnableVertexAttribArray(material.shader.locs[rf_loc_vertex_tangent]);
        }

        // Bind mesh VBO data: vertex texcoords2 (shader-location = 5, if available)
        if (material.shader.locs[rf_loc_vertex_texcoord02] != -1)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_id[5]);
            glVertexAttribPointer(material.shader.locs[rf_loc_vertex_texcoord02], 2, GL_FLOAT, 0, 0, 0);
            glEnableVertexAttribArray(material.shader.locs[rf_loc_vertex_texcoord02]);
        }

        if (mesh.indices != NULL) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbo_id[6]);
    }

    int eyesCount = 1;

    for (int eye = 0; eye < eyesCount; eye++)
    {
        if (eyesCount == 1) _rf_global_context_ptr->gl_ctx.modelview = matModelView;

        // Calculate model-view-_rf_global_context_ptr->gl_ctx.projection matrix (MVP)
        rf_matrix matMVP = rf_matrix_multiply(_rf_global_context_ptr->gl_ctx.modelview, _rf_global_context_ptr->gl_ctx.projection); // rf_transform to screen-space coordinates

        // Send combined model-view-_rf_global_context_ptr->gl_ctx.projection matrix to shader
        glUniformMatrix4fv(material.shader.locs[rf_loc_matrix_mvp], 1, false, rf_matrix_to_floatv(matMVP).v);

        // Draw call!
        if (mesh.indices != NULL) glDrawElements(GL_TRIANGLES, mesh.triangle_count*3, GL_UNSIGNED_SHORT, 0); // Indexed vertices draw
        else glDrawArrays(GL_TRIANGLES, 0, mesh.vertex_count);
    }

    // Unbind all binded texture maps
    for (int i = 0; i < rf_max_material_maps; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);       // Set shader active texture
        if ((i == rf_map_irradiance) || (i == rf_map_prefilter) || (i == rf_map_cubemap)) glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        else glBindTexture(GL_TEXTURE_2D, 0);   // Unbind current active texture
    }

    // Unind vertex array objects (or VBOs)
    if (_rf_global_context_ptr->gl_ctx.vao_supported) glBindVertexArray(0);
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        if (mesh.indices != NULL) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    // Unbind shader program
    glUseProgram(0);

    // Restore _rf_global_context_ptr->gl_ctx.projection/_rf_global_context_ptr->gl_ctx.modelview matrices
    // NOTE: In stereo rendering matrices are being modified to fit every eye
    _rf_global_context_ptr->gl_ctx.projection = matProjection;
    _rf_global_context_ptr->gl_ctx.modelview = matView;
#endif
}

// Unload mesh data from CPU and GPU
RF_API void rf_gl_unload_mesh(rf_mesh mesh)
{
    RF_FREE(mesh.vertices);
    RF_FREE(mesh.texcoords);
    RF_FREE(mesh.normals);
    RF_FREE(mesh.colors);
    RF_FREE(mesh.tangents);
    RF_FREE(mesh.texcoords2);
    RF_FREE(mesh.indices);

    RF_FREE(mesh.anim_vertices);
    RF_FREE(mesh.anim_normals);
    RF_FREE(mesh.bone_weights);
    RF_FREE(mesh.bone_ids);

    rf_gl_delete_buffers(mesh.vbo_id[0]);   // vertex
    rf_gl_delete_buffers(mesh.vbo_id[1]);   // texcoords
    rf_gl_delete_buffers(mesh.vbo_id[2]);   // normals
    rf_gl_delete_buffers(mesh.vbo_id[3]);   // colors
    rf_gl_delete_buffers(mesh.vbo_id[4]);   // tangents
    rf_gl_delete_buffers(mesh.vbo_id[5]);   // texcoords2
    rf_gl_delete_buffers(mesh.vbo_id[6]);   // indices

    rf_gl_delete_vertex_arrays(mesh.vao_id);
}

// Read screen pixel data (color buffer)
RF_API unsigned char* rf_gl_read_screen_pixels(int width, int height)
{
    unsigned char* screenData = (unsigned char* )RF_MALLOC(width*height*4 *  sizeof(unsigned char));
    memset(screenData, 0, width * height * 4 * sizeof(unsigned char));

    // NOTE 1: glReadPixels returns image flipped vertically -> (0,0) is the bottom left corner of the framebuffer
    // NOTE 2: We are getting alpha channel! Be careful, it can be transparent if not cleared properly!
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, screenData);

    // Flip image vertically!
    unsigned char* imgData = (unsigned char* )RF_MALLOC(width*height*sizeof(unsigned char)*4);

    for (int y = height - 1; y >= 0; y--)
    {
        for (int x = 0; x < (width*4); x++)
        {
            imgData[((height - 1) - y)*width*4 + x] = screenData[(y*width*4) + x];  // Flip line

            // Set alpha component value to 255 (no trasparent image retrieval)
            // NOTE: Alpha value has already been applied to RGB in framebuffer, we don't need it!
            if (((x + 1)%4) == 0) imgData[((height - 1) - y)*width*4 + x] = 255;
        }
    }

    RF_FREE(screenData);

    return imgData;     // NOTE: image data should be freed
}

// Read texture pixel data
RF_API void* rf_gl_read_texture_pixels(rf_texture2d texture)
{
    void* pixels = NULL;

#if defined(RF_GRAPHICS_API_OPENGL_11) || defined(RF_GRAPHICS_API_OPENGL_33)
    glBindTexture(GL_TEXTURE_2D, texture.id);

    // NOTE: Using texture.id, we can retrieve some texture info (but not on OpenGL ES 2.0)
    // Possible texture info: GL_TEXTURE_RED_SIZE, GL_TEXTURE_GREEN_SIZE, GL_TEXTURE_BLUE_SIZE, GL_TEXTURE_ALPHA_SIZE
    //int width, height, format;
    //glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    //glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    //glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);

    // NOTE: Each row written to or read from by OpenGL pixel operations like glGetTexImage are aligned to a 4 rf_byte boundary by default, which may add some padding.
    // Use glPixelStorei to modify padding with the GL_[UN]PACK_ALIGNMENT setting.
    // GL_PACK_ALIGNMENT affects operations that read from OpenGL memory (glReadPixels, glGetTexImage, etc.)
    // GL_UNPACK_ALIGNMENT affects operations that write to OpenGL memory (glTexImage, etc.)
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    unsigned int glInternalFormat, glFormat, glType;
    rf_gl_get_gl_texture_formats(texture.format, &glInternalFormat, &glFormat, &glType);
    unsigned int size = rf_get_pixel_data_size(texture.width, texture.height, texture.format);

    if ((glInternalFormat != -1) && (texture.format < rf_compressed_dxt1_rgb))
    {
        pixels = (unsigned char* )RF_MALLOC(size);
        glGetTexImage(GL_TEXTURE_2D, 0, glFormat, glType, pixels);
    }
    else RF_LOG(RF_LOG_WARNING, "rf_texture data retrieval not suported for pixel format");

    glBindTexture(GL_TEXTURE_2D, 0);
#endif

#if defined(RF_GRAPHICS_API_OPENGL_ES2)
    // glGetTexImage() is not available on OpenGL ES 2.0
    // rf_texture2d width and height are required on OpenGL ES 2.0. There is no way to get it from texture id.
    // Two possible Options:
    // 1 - Bind texture to color fbo attachment and glReadPixels()
    // 2 - Create an fbo, activate it, render quad with texture, glReadPixels()
    // We are using Option 1, just need to care for texture format on retrieval
    // NOTE: This behaviour could be conditioned by graphic driver...
    rf_render_texture2d fbo = rf_gl_load_render_texture(texture.width, texture.height, rf_uncompressed_r8g8b8a8, 16, false);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo.id);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach our texture to FBO
    // NOTE: Previoust attached texture is automatically detached
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.id, 0);

    // We read data as RGBA because FBO texture is configured as RGBA, despite binding another texture format
    pixels = (unsigned char* )RF_MALLOC(rf_get_pixel_data_size(texture.width, texture.height, rf_uncompressed_r8g8b8a8));
    glReadPixels(0, 0, texture.width, texture.height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Re-attach internal FBO color texture before deleting it
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo.texture.id, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Clean up temporal fbo
    rf_gl_delete_render_textures(fbo);
#endif

    return pixels;
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Shaders Functions
// NOTE: Those functions are exposed directly to the user in raylib.h
//----------------------------------------------------------------------------------

// Get default internal texture (white texture)
RF_API rf_texture2d rf_get_texture_default()
{
    rf_texture2d texture = { 0 };
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    texture.id = _rf_global_context_ptr->gl_ctx.default_texture_id;
    texture.width = 1;
    texture.height = 1;
    texture.mipmaps = 1;
    texture.format = rf_uncompressed_r8g8b8a8;
#endif
    return texture;
}

// Get default shader
RF_API rf_shader rf_get_shader_default()
{
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    return _rf_global_context_ptr->gl_ctx.default_shader;
#else
    rf_shader shader = { 0 };
    return shader;
#endif
}

// Load text data from file
// NOTE: text chars array should be freed manually
RF_INTERNAL char* _rf_load_text_from_file(const char* fileName)
{
    FILE* textFile = NULL;
    char* text = NULL;

    if (fileName != NULL)
    {
        textFile = fopen(fileName,"rt");

        if (textFile != NULL)
        {
            fseek(textFile, 0, SEEK_END);
            int size = ftell(textFile);
            fseek(textFile, 0, SEEK_SET);

            if (size > 0)
            {
                text = (char* )RF_MALLOC(sizeof(char)*(size + 1));
                int count = fread(text, sizeof(char), size, textFile);
                text[count] = '\0';
            }

            fclose(textFile);
        }
        else RF_LOG(RF_LOG_WARNING, "[%s] Text file could not be opened", fileName);
    }

    return text;
}

// Load shader from files and bind default locations
// NOTE: If shader string is NULL, using default vertex/fragment shaders
//  @ToRemove @Note(Lulu): Remove this since it does fileio, rf_load_shader_code should be used everywhere instead
RF_API rf_shader rf_load_shader(const char* vsFileName, const char* fsFileName)
{
    rf_shader shader = { 0 };

    // NOTE: rf_shader.locs is allocated by rf_load_shader_code()

    char* vShaderStr = NULL;
    char* fShaderStr = NULL;

    if (vsFileName != NULL) vShaderStr = _rf_load_text_from_file(vsFileName);
    if (fsFileName != NULL) fShaderStr = _rf_load_text_from_file(fsFileName);

    shader = rf_load_shader_code(vShaderStr, fShaderStr);

    if (vShaderStr != NULL) RF_FREE(vShaderStr);
    if (fShaderStr != NULL) RF_FREE(fShaderStr);

    return shader;
}

// Load shader from code strings
// NOTE: If shader string is NULL, using default vertex/fragment shaders
RF_API rf_shader rf_load_shader_code(const char* vsCode, const char* fsCode)
{
    rf_shader shader = { 0 };
    shader.locs = (int*)RF_MALLOC(rf_max_shader_locations * sizeof(int));
    memset(shader.locs, 0, rf_max_shader_locations * sizeof(int));

    // NOTE: All locations must be reseted to -1 (no location)
    for (int i = 0; i < rf_max_shader_locations; i++) shader.locs[i] = -1;

#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    unsigned int vertexShaderId = _rf_global_context_ptr->gl_ctx.default_vertex_shader_id;
    unsigned int fragmentShaderId = _rf_global_context_ptr->gl_ctx.default_frag_shader_id;

    if (vsCode != NULL) vertexShaderId = _rf_compile_shader(vsCode, GL_VERTEX_SHADER);
    if (fsCode != NULL) fragmentShaderId = _rf_compile_shader(fsCode, GL_FRAGMENT_SHADER);

    if ((vertexShaderId == _rf_global_context_ptr->gl_ctx.default_vertex_shader_id) && (fragmentShaderId == _rf_global_context_ptr->gl_ctx.default_frag_shader_id)) shader = _rf_global_context_ptr->gl_ctx.default_shader;
    else
    {
        shader.id = _rf_load_shader_program(vertexShaderId, fragmentShaderId);

        if (vertexShaderId != _rf_global_context_ptr->gl_ctx.default_vertex_shader_id) glDeleteShader(vertexShaderId);
        if (fragmentShaderId != _rf_global_context_ptr->gl_ctx.default_frag_shader_id) glDeleteShader(fragmentShaderId);

        if (shader.id == 0)
        {
            RF_LOG(RF_LOG_WARNING, "Custom shader could not be loaded");
            shader = _rf_global_context_ptr->gl_ctx.default_shader;
        }

        // After shader loading, we TRY to set default location names
        if (shader.id > 0) _rf_set_shader_default_locations(&shader);
    }

    // Get available shader uniforms
    // NOTE: This information is useful for debug...
    int uniformCount = -1;

    glGetProgramiv(shader.id, GL_ACTIVE_UNIFORMS, &uniformCount);

    for (int i = 0; i < uniformCount; i++)
    {
        int namelen = -1;
        int num = -1;
        char name[256]; // Assume no variable names longer than 256
        GLenum type = GL_ZERO;

        // Get the name of the uniforms
        glGetActiveUniform(shader.id, i,sizeof(name) - 1, &namelen, &num, &type, name);

        name[namelen] = 0;

        // Get the location of the named uniform
        unsigned int location = glGetUniformLocation(shader.id, name);

        RF_LOG(RF_LOG_DEBUG, "[SHDR ID %i] Active uniform [%s] set at location: %i", shader.id, name, location);
    }
#endif

    return shader;
}

// Unload shader from GPU memory (VRAM)
RF_API void rf_unload_shader(rf_shader shader)
{
    if (shader.id > 0)
    {
        rf_gl_delete_shader(shader.id);
        RF_LOG(RF_LOG_INFO, "[SHDR ID %i] Unloaded shader program data", shader.id);
    }

    RF_FREE(shader.locs);
}

// Begin custom shader mode
RF_API void rf_begin_shader_mode(rf_shader shader)
{
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    if (_rf_global_context_ptr->gl_ctx.current_shader.id != shader.id)
    {
        rf_gl_draw();
        _rf_global_context_ptr->gl_ctx.current_shader = shader;
    }
#endif
}

// End custom shader mode (returns to default shader)
RF_API void rf_end_shader_mode()
{
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    rf_begin_shader_mode(_rf_global_context_ptr->gl_ctx.default_shader);
#endif
}

// Get shader uniform location
RF_API int rf_get_shader_location(rf_shader shader, const char* uniformName)
{
    int location = -1;
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    location = glGetUniformLocation(shader.id, uniformName);

    if (location == -1) RF_LOG(RF_LOG_WARNING, "[SHDR ID %i][%s] rf_shader uniform could not be found", shader.id, uniformName);
    else RF_LOG(RF_LOG_INFO, "[SHDR ID %i][%s] rf_shader uniform set at location: %i", shader.id, uniformName, location);
#endif
    return location;
}

// Set shader uniform value
RF_API void rf_set_shader_value(rf_shader shader, int uniformLoc, const void* value, int uniformType)
{
    rf_set_shader_value_v(shader, uniformLoc, value, uniformType, 1);
}

// Set shader uniform value vector
RF_API void rf_set_shader_value_v(rf_shader shader, int uniformLoc, const void* value, int uniformType, int count)
{
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    glUseProgram(shader.id);

    switch (uniformType)
    {
        case rf_uniform_float: glUniform1fv(uniformLoc, count, (float* )value); break;
        case rf_uniform_vec2: glUniform2fv(uniformLoc, count, (float* )value); break;
        case rf_uniform_vec3: glUniform3fv(uniformLoc, count, (float* )value); break;
        case rf_uniform_vec4: glUniform4fv(uniformLoc, count, (float* )value); break;
        case rf_uniform_int: glUniform1iv(uniformLoc, count, (int* )value); break;
        case rf_uniform_ivec2: glUniform2iv(uniformLoc, count, (int* )value); break;
        case rf_uniform_ivec3: glUniform3iv(uniformLoc, count, (int* )value); break;
        case rf_uniform_ivec4: glUniform4iv(uniformLoc, count, (int* )value); break;
        case rf_uniform_sampler2d: glUniform1iv(uniformLoc, count, (int* )value); break;
        default: RF_LOG(RF_LOG_WARNING, "rf_shader uniform could not be set data type not recognized");
    }

    //glUseProgram(0);      // Avoid reseting current shader program, in case other uniforms are set
#endif
}

// Set shader uniform value (matrix 4x4)
RF_API void rf_set_shader_value_matrix(rf_shader shader, int uniformLoc, rf_matrix mat)
{
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    glUseProgram(shader.id);

    glUniformMatrix4fv(uniformLoc, 1, false, rf_matrix_to_floatv(mat).v);

    //glUseProgram(0);
#endif
}

// Set shader uniform value for texture
RF_API void rf_set_shader_value_texture(rf_shader shader, int uniformLoc, rf_texture2d texture)
{
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    glUseProgram(shader.id);

    glUniform1i(uniformLoc, texture.id);

    //glUseProgram(0);
#endif
}

// Set a custom projection matrix (replaces internal _rf_global_context_ptr->gl_ctx.projection matrix)
RF_API void rf_set_matrix_projection(rf_matrix proj)
{
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    _rf_global_context_ptr->gl_ctx.projection = proj;
#endif
}

// Return internal _rf_global_context_ptr->gl_ctx.projection matrix
RF_API rf_matrix rf_get_matrix_projection() {
#if defined(RF_GRAPHICS_API_OPENGL_11)
    float mat[16];
    glGetFloatv(GL_PROJECTION_MATRIX,mat);
    rf_matrix m;
    m.m0  = mat[0];     m.m1  = mat[1];     m.m2  = mat[2];     m.m3  = mat[3];
    m.m4  = mat[4];     m.m5  = mat[5];     m.m6  = mat[6];     m.m7  = mat[7];
    m.m8  = mat[8];     m.m9  = mat[9];     m.m10 = mat[10];    m.m11 = mat[11];
    m.m12 = mat[12];    m.m13 = mat[13];    m.m14 = mat[14];    m.m15 = mat[15];
    return m;
#else
    return _rf_global_context_ptr->gl_ctx.projection;
#endif
}

// Set a custom _rf_global_context_ptr->gl_ctx.modelview matrix (replaces internal _rf_global_context_ptr->gl_ctx.modelview matrix)
RF_API void rf_set_matrix_modelview(rf_matrix view)
{
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    _rf_global_context_ptr->gl_ctx.modelview = view;
#endif
}

// Return internal _rf_global_context_ptr->gl_ctx.modelview matrix
RF_API rf_matrix rf_get_matrix_modelview()
{
    rf_matrix matrix = rf_matrix_identity();
#if defined(RF_GRAPHICS_API_OPENGL_11)
    float mat[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mat);
    matrix.m0  = mat[0];     matrix.m1  = mat[1];     matrix.m2  = mat[2];     matrix.m3  = mat[3];
    matrix.m4  = mat[4];     matrix.m5  = mat[5];     matrix.m6  = mat[6];     matrix.m7  = mat[7];
    matrix.m8  = mat[8];     matrix.m9  = mat[9];     matrix.m10 = mat[10];    matrix.m11 = mat[11];
    matrix.m12 = mat[12];    matrix.m13 = mat[13];    matrix.m14 = mat[14];    matrix.m15 = mat[15];
#else
    matrix = _rf_global_context_ptr->gl_ctx.modelview;
#endif
    return matrix;
}

// Generate cubemap texture from HDR texture
// TODO: OpenGL ES 2.0 does not support GL_RGB16F texture format, neither GL_DEPTH_COMPONENT24
RF_API rf_texture2d rf_gen_texture_cubemap(rf_shader shader, rf_texture2d skyHDR, int size)
{
    rf_texture2d cubemap = { 0 };
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    // NOTE: _rf_set_shader_default_locations() already setups locations for _rf_global_context_ptr->gl_ctx.projection and view rf_matrix in shader
    // Other locations should be setup externally in shader before calling the function

    // Set up depth face culling and cubemap seamless
    glDisable(GL_CULL_FACE);
#if defined(RF_GRAPHICS_API_OPENGL_33)
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);     // Flag not supported on OpenGL ES 2.0
#endif

    // Setup framebuffer
    unsigned int fbo, rbo;
    glGenFramebuffers(1, &fbo);
    glGenRenderbuffers(1, &rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
#if defined(RF_GRAPHICS_API_OPENGL_33)
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
#elif defined(RF_GRAPHICS_API_OPENGL_ES2)
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, size, size);
#endif
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // Set up cubemap to render and attach to framebuffer
    // NOTE: Faces are stored as 32 bit floating point values
    glGenTextures(1, &cubemap.id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.id);
    for (unsigned int i = 0; i < 6; i++)
    {
#if defined(RF_GRAPHICS_API_OPENGL_33)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, size, size, 0, GL_RGB, GL_FLOAT, NULL);
#elif defined(RF_GRAPHICS_API_OPENGL_ES2)
        if (_rf_global_context_ptr->gl_ctx.tex_float_supported) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, size, size, 0, GL_RGB, GL_FLOAT, NULL);
#endif
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if defined(RF_GRAPHICS_API_OPENGL_33)
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  // Flag not supported on OpenGL ES 2.0
#endif
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Create _rf_global_context_ptr->gl_ctx.projection and different views for each face
    rf_matrix fboProjection = rf_matrix_perspective(90.0*RF_DEG2RAD, 1.0, 0.01, 1000.0);
    rf_matrix fboViews[6] = {
            rf_matrix_look_at((rf_vector3){ 0.0f, 0.0f, 0.0f }, (rf_vector3){ 1.0f, 0.0f, 0.0f }, (rf_vector3){ 0.0f, -1.0f, 0.0f }),
            rf_matrix_look_at((rf_vector3){ 0.0f, 0.0f, 0.0f }, (rf_vector3){ -1.0f, 0.0f, 0.0f }, (rf_vector3){ 0.0f, -1.0f, 0.0f }),
            rf_matrix_look_at((rf_vector3){ 0.0f, 0.0f, 0.0f }, (rf_vector3){ 0.0f, 1.0f, 0.0f }, (rf_vector3){ 0.0f, 0.0f, 1.0f }),
            rf_matrix_look_at((rf_vector3){ 0.0f, 0.0f, 0.0f }, (rf_vector3){ 0.0f, -1.0f, 0.0f }, (rf_vector3){ 0.0f, 0.0f, -1.0f }),
            rf_matrix_look_at((rf_vector3){ 0.0f, 0.0f, 0.0f }, (rf_vector3){ 0.0f, 0.0f, 1.0f }, (rf_vector3){ 0.0f, -1.0f, 0.0f }),
            rf_matrix_look_at((rf_vector3){ 0.0f, 0.0f, 0.0f }, (rf_vector3){ 0.0f, 0.0f, -1.0f }, (rf_vector3){ 0.0f, -1.0f, 0.0f })
    };

    // Convert HDR equirectangular environment map to cubemap equivalent
    glUseProgram(shader.id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, skyHDR.id);
    rf_set_shader_value_matrix(shader, shader.locs[rf_loc_matrix_projection], fboProjection);

    // Note: don't forget to configure the viewport to the capture dimensions
    glViewport(0, 0, size, size);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    for (int i = 0; i < 6; i++)
    {
        rf_set_shader_value_matrix(shader, shader.locs[rf_loc_matrix_view], fboViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap.id, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        _rf_gen_draw_cube();
    }

    // Unbind framebuffer and textures
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Reset viewport dimensions to default
    glViewport(0, 0, _rf_global_context_ptr->gl_ctx.framebuffer_width, _rf_global_context_ptr->gl_ctx.framebuffer_height);
    //glEnable(GL_CULL_FACE);

    // NOTE: rf_texture2d is a GL_TEXTURE_CUBE_MAP, not a GL_TEXTURE_2D!
    cubemap.width = size;
    cubemap.height = size;
    cubemap.mipmaps = 1;
    cubemap.format = rf_uncompressed_r32g32b32;
#endif
    return cubemap;
}

// Generate irradiance texture using cubemap data
// TODO: OpenGL ES 2.0 does not support GL_RGB16F texture format, neither GL_DEPTH_COMPONENT24
RF_API rf_texture2d rf_gen_texture_irradiance(rf_shader shader, rf_texture2d cubemap, int size)
{
    rf_texture2d irradiance = { 0 };

#if defined(RF_GRAPHICS_API_OPENGL_33) // || defined(RF_GRAPHICS_API_OPENGL_ES2)
    // NOTE: _rf_set_shader_default_locations() already setups locations for _rf_global_context_ptr->gl_ctx.projection and view rf_matrix in shader
    // Other locations should be setup externally in shader before calling the function

    // Setup framebuffer
    unsigned int fbo, rbo;
    glGenFramebuffers(1, &fbo);
    glGenRenderbuffers(1, &rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // Create an irradiance cubemap, and re-scale capture FBO to irradiance scale
    glGenTextures(1, &irradiance.id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance.id);
    for (unsigned int i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, NULL);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Create _rf_global_context_ptr->gl_ctx.projection (transposed) and different views for each face
    rf_matrix fboProjection = rf_matrix_perspective(90.0*RF_DEG2RAD, 1.0, 0.01, 1000.0);
    rf_matrix fboViews[6] = {
            rf_matrix_look_at((rf_vector3){ 0.0f, 0.0f, 0.0f }, (rf_vector3){ 1.0f, 0.0f, 0.0f }, (rf_vector3){ 0.0f, -1.0f, 0.0f }),
            rf_matrix_look_at((rf_vector3){ 0.0f, 0.0f, 0.0f }, (rf_vector3){ -1.0f, 0.0f, 0.0f }, (rf_vector3){ 0.0f, -1.0f, 0.0f }),
            rf_matrix_look_at((rf_vector3){ 0.0f, 0.0f, 0.0f }, (rf_vector3){ 0.0f, 1.0f, 0.0f }, (rf_vector3){ 0.0f, 0.0f, 1.0f }),
            rf_matrix_look_at((rf_vector3){ 0.0f, 0.0f, 0.0f }, (rf_vector3){ 0.0f, -1.0f, 0.0f }, (rf_vector3){ 0.0f, 0.0f, -1.0f }),
            rf_matrix_look_at((rf_vector3){ 0.0f, 0.0f, 0.0f }, (rf_vector3){ 0.0f, 0.0f, 1.0f }, (rf_vector3){ 0.0f, -1.0f, 0.0f }),
            rf_matrix_look_at((rf_vector3){ 0.0f, 0.0f, 0.0f }, (rf_vector3){ 0.0f, 0.0f, -1.0f }, (rf_vector3){ 0.0f, -1.0f, 0.0f })
    };

    // Solve diffuse integral by convolution to create an irradiance cubemap
    glUseProgram(shader.id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.id);
    rf_set_shader_value_matrix(shader, shader.locs[rf_loc_matrix_projection], fboProjection);

    // Note: don't forget to configure the viewport to the capture dimensions
    glViewport(0, 0, size, size);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    for (int i = 0; i < 6; i++)
    {
        rf_set_shader_value_matrix(shader, shader.locs[rf_loc_matrix_view], fboViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance.id, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        _rf_gen_draw_cube();
    }

    // Unbind framebuffer and textures
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Reset viewport dimensions to default
    glViewport(0, 0, _rf_global_context_ptr->gl_ctx.framebuffer_width, _rf_global_context_ptr->gl_ctx.framebuffer_height);

    irradiance.width = size;
    irradiance.height = size;
    irradiance.mipmaps = 1;
    //irradiance.format = UNCOMPRESSED_R16G16B16;
#endif
    return irradiance;
}

// Generate prefilter texture using cubemap data
// TODO: OpenGL ES 2.0 does not support GL_RGB16F texture format, neither GL_DEPTH_COMPONENT24
RF_API rf_texture2d rf_gen_texture_prefilter(rf_shader shader, rf_texture2d cubemap, int size)
{
    rf_texture2d prefilter = { 0 };

#if defined(RF_GRAPHICS_API_OPENGL_33) // || defined(RF_GRAPHICS_API_OPENGL_ES2)
    // NOTE: _rf_set_shader_default_locations() already setups locations for _rf_global_context_ptr->gl_ctx.projection and view rf_matrix in shader
    // Other locations should be setup externally in shader before calling the function
    // TODO: Locations should be taken out of this function... too shader dependant...
    int roughnessLoc = rf_get_shader_location(shader, "roughness");

    // Setup framebuffer
    unsigned int fbo, rbo;
    glGenFramebuffers(1, &fbo);
    glGenRenderbuffers(1, &rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // Create a prefiltered HDR environment map
    glGenTextures(1, &prefilter.id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter.id);
    for (unsigned int i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, NULL);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Generate mipmaps for the prefiltered HDR texture
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // Create _rf_global_context_ptr->gl_ctx.projection (transposed) and different views for each face
    rf_matrix fboProjection = rf_matrix_perspective(90.0*RF_DEG2RAD, 1.0, 0.01, 1000.0);
    rf_matrix fboViews[6] = {
            rf_matrix_look_at((rf_vector3){ 0.0f, 0.0f, 0.0f }, (rf_vector3){ 1.0f, 0.0f, 0.0f }, (rf_vector3){ 0.0f, -1.0f, 0.0f }),
            rf_matrix_look_at((rf_vector3){ 0.0f, 0.0f, 0.0f }, (rf_vector3){ -1.0f, 0.0f, 0.0f }, (rf_vector3){ 0.0f, -1.0f, 0.0f }),
            rf_matrix_look_at((rf_vector3){ 0.0f, 0.0f, 0.0f }, (rf_vector3){ 0.0f, 1.0f, 0.0f }, (rf_vector3){ 0.0f, 0.0f, 1.0f }),
            rf_matrix_look_at((rf_vector3){ 0.0f, 0.0f, 0.0f }, (rf_vector3){ 0.0f, -1.0f, 0.0f }, (rf_vector3){ 0.0f, 0.0f, -1.0f }),
            rf_matrix_look_at((rf_vector3){ 0.0f, 0.0f, 0.0f }, (rf_vector3){ 0.0f, 0.0f, 1.0f }, (rf_vector3){ 0.0f, -1.0f, 0.0f }),
            rf_matrix_look_at((rf_vector3){ 0.0f, 0.0f, 0.0f }, (rf_vector3){ 0.0f, 0.0f, -1.0f }, (rf_vector3){ 0.0f, -1.0f, 0.0f })
    };

    // Prefilter HDR and store data into mipmap levels
    glUseProgram(shader.id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.id);
    rf_set_shader_value_matrix(shader, shader.locs[rf_loc_matrix_projection], fboProjection);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

#define MAX_MIPMAP_LEVELS   5   // Max number of prefilter texture mipmaps

    for (int mip = 0; mip < MAX_MIPMAP_LEVELS; mip++)
    {
        // Resize framebuffer according to mip-level size.
        unsigned int mipWidth  = size*(int)powf(0.5f, (float)mip);
        unsigned int mipHeight = size*(int)powf(0.5f, (float)mip);

        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip/(float)(MAX_MIPMAP_LEVELS - 1);
        glUniform1f(roughnessLoc, roughness);

        for (int i = 0; i < 6; i++)
        {
            rf_set_shader_value_matrix(shader, shader.locs[rf_loc_matrix_view], fboViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilter.id, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            _rf_gen_draw_cube();
        }
    }

    // Unbind framebuffer and textures
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Reset viewport dimensions to default
    glViewport(0, 0, _rf_global_context_ptr->gl_ctx.framebuffer_width, _rf_global_context_ptr->gl_ctx.framebuffer_height);

    prefilter.width = size;
    prefilter.height = size;
    //prefilter.mipmaps = 1 + (int)floor(log(size)/log(2));
    //prefilter.format = UNCOMPRESSED_R16G16B16;
#endif
    return prefilter;
}

// Generate BRDF texture using cubemap data
// NOTE: OpenGL ES 2.0 does not support GL_RGB16F texture format, neither GL_DEPTH_COMPONENT24
// TODO: Review implementation: https://github.com/HectorMF/BRDFGenerator
RF_API rf_texture2d rf_gen_texture_brdf(rf_shader shader, int size)
{
    rf_texture2d brdf = { 0 };
#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
    // Generate BRDF convolution texture
    glGenTextures(1, &brdf.id);
    glBindTexture(GL_TEXTURE_2D, brdf.id);
#if defined(RF_GRAPHICS_API_OPENGL_33)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, size, size, 0, GL_RGB, GL_FLOAT, NULL);
#elif defined(RF_GRAPHICS_API_OPENGL_ES2)
    if (_rf_global_context_ptr->gl_ctx.tex_float_supported) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_FLOAT, NULL);
#endif

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Render BRDF LUT into a quad using FBO
    unsigned int fbo, rbo;
    glGenFramebuffers(1, &fbo);
    glGenRenderbuffers(1, &rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
#if defined(RF_GRAPHICS_API_OPENGL_33)
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
#elif defined(RF_GRAPHICS_API_OPENGL_ES2)
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, size, size);
#endif
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdf.id, 0);

    glViewport(0, 0, size, size);
    glUseProgram(shader.id);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _rf_gen_draw_quad();

    // Unbind framebuffer and textures
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Unload framebuffer but keep color texture
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);

    // Reset viewport dimensions to default
    glViewport(0, 0, _rf_global_context_ptr->gl_ctx.framebuffer_width, _rf_global_context_ptr->gl_ctx.framebuffer_height);

    brdf.width = size;
    brdf.height = size;
    brdf.mipmaps = 1;
    brdf.format = rf_uncompressed_r32g32b32;
#endif
    return brdf;
}

// Begin blending mode (alpha, additive, multiplied)
// NOTE: Only 3 blending modes supported, default blend mode is alpha
RF_API void rf_begin_blend_mode(int mode)
{
    if ((_rf_global_context_ptr->gl_ctx.blend_mode != mode) && (mode < 3))
    {
        rf_gl_draw();

        switch (mode)
        {
            case rf_blend_alpha: glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); break;
            case rf_blend_additive: glBlendFunc(GL_SRC_ALPHA, GL_ONE); break; // Alternative: glBlendFunc(GL_ONE, GL_ONE);
            case rf_blend_multiplied: glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA); break;
            default: break;
        }

        _rf_global_context_ptr->gl_ctx.blend_mode = mode;
    }
}

// End blending mode (reset to default: alpha blending)
RF_API void rf_end_blend_mode()
{
    rf_begin_blend_mode(rf_blend_alpha);
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)
// Compile custom shader and return shader id
RF_INTERNAL unsigned int _rf_compile_shader(const char* shaderStr, int type)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &shaderStr, NULL);

    GLint success = 0;
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success != GL_TRUE)
    {
        RF_LOG(RF_LOG_WARNING, "[SHDR ID %i] Failed to compile shader...", shader);
        int maxLength = 0;
        int length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

#if defined(_MSC_VER)
        char* log = RF_MALLOC(maxLength);
#else
        char log[maxLength];
#endif
        glGetShaderInfoLog(shader, maxLength, &length, log);

        RF_LOG(RF_LOG_INFO, "%s", log);

#if defined(_MSC_VER)
        RF_FREE(log);
#endif
    }
    else RF_LOG(RF_LOG_INFO, "[SHDR ID %i] rf_shader compiled successfully", shader);

    return shader;
}

// Load custom shader strings and return program id
RF_INTERNAL unsigned int _rf_load_shader_program(unsigned int vShaderId, unsigned int fShaderId)
{
    unsigned int program = 0;

#if defined(RF_GRAPHICS_API_OPENGL_33) || defined(RF_GRAPHICS_API_OPENGL_ES2)

    GLint success = 0;
    program = glCreateProgram();

    glAttachShader(program, vShaderId);
    glAttachShader(program, fShaderId);

    // NOTE: Default attribute shader locations must be binded before linking
    glBindAttribLocation(program, 0, DEFAULT_ATTRIB_POSITION_NAME);
    glBindAttribLocation(program, 1, DEFAULT_ATTRIB_TEXCOORD_NAME);
    glBindAttribLocation(program, 2, DEFAULT_ATTRIB_NORMAL_NAME);
    glBindAttribLocation(program, 3, DEFAULT_ATTRIB_COLOR_NAME);
    glBindAttribLocation(program, 4, DEFAULT_ATTRIB_TANGENT_NAME);
    glBindAttribLocation(program, 5, DEFAULT_ATTRIB_TEXCOORD2_NAME);

    // NOTE: If some attrib name is no found on the shader, it locations becomes -1

    glLinkProgram(program);

    // NOTE: All uniform variables are intitialised to 0 when a program links

    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success == GL_FALSE)
    {
        RF_LOG(RF_LOG_WARNING, "[SHDR ID %i] Failed to link shader program...", program);

        int maxLength = 0;
        int length;

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

#if defined(_MSC_VER)
        char* log = RF_MALLOC(maxLength);
#else
        char log[maxLength];
#endif
        glGetProgramInfoLog(program, maxLength, &length, log);

        RF_LOG(RF_LOG_INFO, "%s", log);

#if defined(_MSC_VER)
        RF_FREE(log);
#endif
        glDeleteProgram(program);

        program = 0;
    }
    else RF_LOG(RF_LOG_INFO, "[SHDR ID %i] rf_shader program loaded successfully", program);
#endif
    return program;
}


// Load default shader (just vertex positioning and texture coloring)
// NOTE: This shader program is used for internal buffers
RF_INTERNAL rf_shader _rf_load_shader_default()
{
    rf_shader shader = { 0 };
    shader.locs = (int*)RF_MALLOC(rf_max_shader_locations * sizeof(int));
    memset(shader.locs, 0, rf_max_shader_locations * sizeof(int));

    // NOTE: All locations must be reseted to -1 (no location)
    for (int i = 0; i < rf_max_shader_locations; i++) shader.locs[i] = -1;

    // Vertex shader directly defined, no external file required
    const char* defaultVShaderStr =
            #if defined(RF_GRAPHICS_API_OPENGL_21)
            "#version 120                       \n"
            #elif defined(RF_GRAPHICS_API_OPENGL_ES2)
            "#version 100                       \n"
            #endif
            #if defined(RF_GRAPHICS_API_OPENGL_ES2) || defined(RF_GRAPHICS_API_OPENGL_21)
            "attribute vec3 vertexPosition;     \n"
            "attribute vec2 vertexTexCoord;     \n"
            "attribute vec4 vertexColor;        \n"
            "varying vec2 fragTexCoord;         \n"
            "varying vec4 fragColor;            \n"
            #elif defined(RF_GRAPHICS_API_OPENGL_33)
            "#version 330                       \n"
            "in vec3 vertexPosition;            \n"
            "in vec2 vertexTexCoord;            \n"
            "in vec4 vertexColor;               \n"
            "out vec2 fragTexCoord;             \n"
            "out vec4 fragColor;                \n"
            #endif
            "uniform mat4 mvp;                  \n"
            "void main()                        \n"
            "{                                  \n"
            "    fragTexCoord = vertexTexCoord; \n"
            "    fragColor = vertexColor;       \n"
            "    gl_Position = mvp*vec4(vertexPosition, 1.0); \n"
            "}                                  \n";

    // Fragment shader directly defined, no external file required
    const char* defaultFShaderStr =
            #if defined(RF_GRAPHICS_API_OPENGL_21)
            "#version 120                       \n"
            #elif defined(RF_GRAPHICS_API_OPENGL_ES2)
            "#version 100                       \n"
            "precision mediump float;           \n"     // precision required for OpenGL ES2 (WebGL)
            #endif
            #if defined(RF_GRAPHICS_API_OPENGL_ES2) || defined(RF_GRAPHICS_API_OPENGL_21)
            "varying vec2 fragTexCoord;         \n"
            "varying vec4 fragColor;            \n"
            #elif defined(RF_GRAPHICS_API_OPENGL_33)
            "#version 330       \n"
            "in vec2 fragTexCoord;              \n"
            "in vec4 fragColor;                 \n"
            "out vec4 finalColor;               \n"
            #endif
            "uniform sampler2D texture0;        \n"
            "uniform vec4 colDiffuse;           \n"
            "void main()                        \n"
            "{                                  \n"
            #if defined(RF_GRAPHICS_API_OPENGL_ES2) || defined(RF_GRAPHICS_API_OPENGL_21)
            "    vec4 texelColor = texture2D(texture0, fragTexCoord); \n" // NOTE: texture2D() is deprecated on OpenGL 3.3 and ES 3.0
            "    gl_FragColor = texelColor*colDiffuse*fragColor;      \n"
            #elif defined(RF_GRAPHICS_API_OPENGL_33)
            "    vec4 texelColor = texture(texture0, fragTexCoord);   \n"
            "    finalColor = texelColor*colDiffuse*fragColor;        \n"
            #endif
            "}                                  \n";

    // NOTE: Compiled vertex/fragment shaders are kept for re-use
    _rf_global_context_ptr->gl_ctx.default_vertex_shader_id = _rf_compile_shader(defaultVShaderStr, GL_VERTEX_SHADER);     // Compile default vertex shader
    _rf_global_context_ptr->gl_ctx.default_frag_shader_id = _rf_compile_shader(defaultFShaderStr, GL_FRAGMENT_SHADER);   // Compile default fragment shader

    shader.id = _rf_load_shader_program(_rf_global_context_ptr->gl_ctx.default_vertex_shader_id, _rf_global_context_ptr->gl_ctx.default_frag_shader_id);

    if (shader.id > 0)
    {
        RF_LOG(RF_LOG_INFO, "[SHDR ID %i] Default shader loaded successfully", shader.id);

        // Set default shader locations: attributes locations
        shader.locs[rf_loc_vertex_position] = glGetAttribLocation(shader.id, "vertexPosition");
        shader.locs[rf_loc_vertex_texcoord01] = glGetAttribLocation(shader.id, "vertexTexCoord");
        shader.locs[rf_loc_vertex_color] = glGetAttribLocation(shader.id, "vertexColor");

        // Set default shader locations: uniform locations
        shader.locs[rf_loc_matrix_mvp]  = glGetUniformLocation(shader.id, "mvp");
        shader.locs[rf_loc_color_diffuse] = glGetUniformLocation(shader.id, "colDiffuse");
        shader.locs[rf_loc_map_diffuse] = glGetUniformLocation(shader.id, "texture0");

        // NOTE: We could also use below function but in case DEFAULT_ATTRIB_* points are
        // changed for external custom shaders, we just use direct bindings above
        //_rf_set_shader_default_locations(&shader);
    }
    else RF_LOG(RF_LOG_WARNING, "[SHDR ID %i] Default shader could not be loaded", shader.id);

    return shader;
}

// Get location handlers to for shader attributes and uniforms
// NOTE: If any location is not found, loc point becomes -1
RF_INTERNAL void _rf_set_shader_default_locations(rf_shader* shader)
{
    // NOTE: Default shader attrib locations have been fixed before linking:
    //          vertex position location    = 0
    //          vertex texcoord location    = 1
    //          vertex normal location      = 2
    //          vertex color location       = 3
    //          vertex tangent location     = 4
    //          vertex texcoord2 location   = 5

    // Get handles to GLSL input attibute locations
    shader->locs[rf_loc_vertex_position] = glGetAttribLocation(shader->id, DEFAULT_ATTRIB_POSITION_NAME);
    shader->locs[rf_loc_vertex_texcoord01] = glGetAttribLocation(shader->id, DEFAULT_ATTRIB_TEXCOORD_NAME);
    shader->locs[rf_loc_vertex_texcoord02] = glGetAttribLocation(shader->id, DEFAULT_ATTRIB_TEXCOORD2_NAME);
    shader->locs[rf_loc_vertex_normal] = glGetAttribLocation(shader->id, DEFAULT_ATTRIB_NORMAL_NAME);
    shader->locs[rf_loc_vertex_tangent] = glGetAttribLocation(shader->id, DEFAULT_ATTRIB_TANGENT_NAME);
    shader->locs[rf_loc_vertex_color] = glGetAttribLocation(shader->id, DEFAULT_ATTRIB_COLOR_NAME);

    // Get handles to GLSL uniform locations (vertex shader)
    shader->locs[rf_loc_matrix_mvp]  = glGetUniformLocation(shader->id, "mvp");
    shader->locs[rf_loc_matrix_projection]  = glGetUniformLocation(shader->id, "_rf_global_context_ptr->gl_ctx.projection");
    shader->locs[rf_loc_matrix_view]  = glGetUniformLocation(shader->id, "view");

    // Get handles to GLSL uniform locations (fragment shader)
    shader->locs[rf_loc_color_diffuse] = glGetUniformLocation(shader->id, "colDiffuse");
    shader->locs[rf_loc_map_diffuse] = glGetUniformLocation(shader->id, "texture0");
    shader->locs[rf_loc_map_specular] = glGetUniformLocation(shader->id, "texture1");
    shader->locs[rf_loc_map_normal] = glGetUniformLocation(shader->id, "texture2");
}

// Unload default shader
RF_INTERNAL void _rf_unlock_shader_default()
{
    glUseProgram(0);

    glDetachShader(_rf_global_context_ptr->gl_ctx.default_shader.id, _rf_global_context_ptr->gl_ctx.default_vertex_shader_id);
    glDetachShader(_rf_global_context_ptr->gl_ctx.default_shader.id, _rf_global_context_ptr->gl_ctx.default_frag_shader_id);
    glDeleteShader(_rf_global_context_ptr->gl_ctx.default_vertex_shader_id);
    glDeleteShader(_rf_global_context_ptr->gl_ctx.default_frag_shader_id);

    glDeleteProgram(_rf_global_context_ptr->gl_ctx.default_shader.id);
}

// Load default internal buffers
RF_INTERNAL void _rf_load_buffers_default()
{
    // Initialize CPU (RAM) arrays (vertex position, texcoord, color data and indexes)
    //--------------------------------------------------------------------------------------------
    for (int i = 0; i < rf_max_batch_buffering; i++)
    {
        _rf_global_context_ptr->gl_ctx.vertex_data[i].vertices = (float* )RF_MALLOC(sizeof(float)*3*4*rf_max_batch_elements);        // 3 float by vertex, 4 vertex by quad
        _rf_global_context_ptr->gl_ctx.vertex_data[i].texcoords = (float* )RF_MALLOC(sizeof(float)*2*4*rf_max_batch_elements);       // 2 float by texcoord, 4 texcoord by quad
        _rf_global_context_ptr->gl_ctx.vertex_data[i].colors = (unsigned char* )RF_MALLOC(sizeof(unsigned char)*4*4*rf_max_batch_elements);  // 4 float by color, 4 colors by quad
#if defined(RF_GRAPHICS_API_OPENGL_33)
        _rf_global_context_ptr->gl_ctx.vertex_data[i].indices = (unsigned int* )RF_MALLOC(sizeof(unsigned int)*6*rf_max_batch_elements);      // 6 int by quad (indices)
#elif defined(RF_GRAPHICS_API_OPENGL_ES2)
        _rf_global_context_ptr->gl_ctx.vertex_data[i].indices = (unsigned short *)RF_MALLOC(sizeof(unsigned short)*6*rf_max_batch_elements);  // 6 int by quad (indices)
#endif

        for (int j = 0; j < (3*4*rf_max_batch_elements); j++) _rf_global_context_ptr->gl_ctx.vertex_data[i].vertices[j] = 0.0f;
        for (int j = 0; j < (2*4*rf_max_batch_elements); j++) _rf_global_context_ptr->gl_ctx.vertex_data[i].texcoords[j] = 0.0f;
        for (int j = 0; j < (4*4*rf_max_batch_elements); j++) _rf_global_context_ptr->gl_ctx.vertex_data[i].colors[j] = 0;

        int k = 0;

        // Indices can be initialized right now
        for (int j = 0; j < (6*rf_max_batch_elements); j += 6)
        {
            _rf_global_context_ptr->gl_ctx.vertex_data[i].indices[j] = 4*k;
            _rf_global_context_ptr->gl_ctx.vertex_data[i].indices[j + 1] = 4*k + 1;
            _rf_global_context_ptr->gl_ctx.vertex_data[i].indices[j + 2] = 4*k + 2;
            _rf_global_context_ptr->gl_ctx.vertex_data[i].indices[j + 3] = 4*k;
            _rf_global_context_ptr->gl_ctx.vertex_data[i].indices[j + 4] = 4*k + 2;
            _rf_global_context_ptr->gl_ctx.vertex_data[i].indices[j + 5] = 4*k + 3;

            k++;
        }

        _rf_global_context_ptr->gl_ctx.vertex_data[i].vCounter = 0;
        _rf_global_context_ptr->gl_ctx.vertex_data[i].tcCounter = 0;
        _rf_global_context_ptr->gl_ctx.vertex_data[i].cCounter = 0;
    }

    RF_LOG(RF_LOG_INFO, "Internal buffers initialized successfully (CPU)");
    //--------------------------------------------------------------------------------------------

    // Upload to GPU (VRAM) vertex data and initialize VAOs/VBOs
    //--------------------------------------------------------------------------------------------
    for (int i = 0; i < rf_max_batch_buffering; i++)
    {
        if (_rf_global_context_ptr->gl_ctx.vao_supported)
        {
            // Initialize Quads VAO
            glGenVertexArrays(1, &_rf_global_context_ptr->gl_ctx.vertex_data[i].vao_id);
            glBindVertexArray(_rf_global_context_ptr->gl_ctx.vertex_data[i].vao_id);
        }

        // Quads - Vertex buffers binding and attributes enable
        // Vertex position buffer (shader-location = 0)
        glGenBuffers(1, &_rf_global_context_ptr->gl_ctx.vertex_data[i].vbo_id[0]);
        glBindBuffer(GL_ARRAY_BUFFER, _rf_global_context_ptr->gl_ctx.vertex_data[i].vbo_id[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*4*rf_max_batch_elements, _rf_global_context_ptr->gl_ctx.vertex_data[i].vertices, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(_rf_global_context_ptr->gl_ctx.current_shader.locs[rf_loc_vertex_position]);
        glVertexAttribPointer(_rf_global_context_ptr->gl_ctx.current_shader.locs[rf_loc_vertex_position], 3, GL_FLOAT, 0, 0, 0);

        // Vertex texcoord buffer (shader-location = 1)
        glGenBuffers(1, &_rf_global_context_ptr->gl_ctx.vertex_data[i].vbo_id[1]);
        glBindBuffer(GL_ARRAY_BUFFER, _rf_global_context_ptr->gl_ctx.vertex_data[i].vbo_id[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*4*rf_max_batch_elements, _rf_global_context_ptr->gl_ctx.vertex_data[i].texcoords, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(_rf_global_context_ptr->gl_ctx.current_shader.locs[rf_loc_vertex_texcoord01]);
        glVertexAttribPointer(_rf_global_context_ptr->gl_ctx.current_shader.locs[rf_loc_vertex_texcoord01], 2, GL_FLOAT, 0, 0, 0);

        // Vertex color buffer (shader-location = 3)
        glGenBuffers(1, &_rf_global_context_ptr->gl_ctx.vertex_data[i].vbo_id[2]);
        glBindBuffer(GL_ARRAY_BUFFER, _rf_global_context_ptr->gl_ctx.vertex_data[i].vbo_id[2]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned char)*4*4*rf_max_batch_elements, _rf_global_context_ptr->gl_ctx.vertex_data[i].colors, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(_rf_global_context_ptr->gl_ctx.current_shader.locs[rf_loc_vertex_color]);
        glVertexAttribPointer(_rf_global_context_ptr->gl_ctx.current_shader.locs[rf_loc_vertex_color], 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);

        // Fill index buffer
        glGenBuffers(1, &_rf_global_context_ptr->gl_ctx.vertex_data[i].vbo_id[3]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _rf_global_context_ptr->gl_ctx.vertex_data[i].vbo_id[3]);
#if defined(RF_GRAPHICS_API_OPENGL_33)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*6*rf_max_batch_elements, _rf_global_context_ptr->gl_ctx.vertex_data[i].indices, GL_STATIC_DRAW);
#elif defined(RF_GRAPHICS_API_OPENGL_ES2)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(short)*6*rf_max_batch_elements, _rf_global_context_ptr->gl_ctx.vertex_data[i].indices, GL_STATIC_DRAW);
#endif
    }

    RF_LOG(RF_LOG_INFO, "Internal buffers uploaded successfully (GPU)");

    // Unbind the current VAO
    if (_rf_global_context_ptr->gl_ctx.vao_supported) glBindVertexArray(0);
    //--------------------------------------------------------------------------------------------
}

// Update default internal buffers (VAOs/VBOs) with vertex array data
// NOTE: If there is not vertex data, buffers doesn't need to be updated (vertex_count > 0)
// TODO: If no data changed on the CPU arrays --> No need to re-update GPU arrays (change flag required)
RF_INTERNAL void _rf_update_buffers_default()
{
    // Update vertex buffers data
    if (_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vCounter > 0)
    {
        // Activate elements VAO
        if (_rf_global_context_ptr->gl_ctx.vao_supported) glBindVertexArray(_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vao_id);

        // Vertex positions buffer
        glBindBuffer(GL_ARRAY_BUFFER, _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vbo_id[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*3*_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vCounter, _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vertices);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*4*rf_max_batch_elements, _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vertices, GL_DYNAMIC_DRAW);  // Update all buffer

        // rf_texture coordinates buffer
        glBindBuffer(GL_ARRAY_BUFFER, _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vbo_id[1]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*2*_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vCounter, _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].texcoords);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*4*rf_max_batch_elements, _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].texcoords, GL_DYNAMIC_DRAW); // Update all buffer

        // Colors buffer
        glBindBuffer(GL_ARRAY_BUFFER, _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vbo_id[2]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(unsigned char)*4*_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vCounter, _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].colors);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*4*rf_max_batch_elements, _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].colors, GL_DYNAMIC_DRAW);    // Update all buffer

        // NOTE: glMapBuffer() causes sync issue.
        // If GPU is working with this buffer, glMapBuffer() will wait(stall) until GPU to finish its job.
        // To avoid waiting (idle), you can call first glBufferData() with NULL pointer before glMapBuffer().
        // If you do that, the previous data in PBO will be discarded and glMapBuffer() returns a new
        // allocated pointer immediately even if GPU is still working with the previous data.

        // Another option: map the buffer object into client's memory
        // Probably this code could be moved somewhere else...
        // _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vertices = (float* )glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
        // if (_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vertices)
        // {
        // Update vertex data
        // }
        // glUnmapBuffer(GL_ARRAY_BUFFER);

        // Unbind the current VAO
        if (_rf_global_context_ptr->gl_ctx.vao_supported) glBindVertexArray(0);
    }
}

// Draw default internal buffers vertex data
RF_INTERNAL void _rf_draw_buffers_default()
{
    rf_matrix matProjection = _rf_global_context_ptr->gl_ctx.projection;
    rf_matrix matModelView = _rf_global_context_ptr->gl_ctx.modelview;

    int eyesCount = 1;

    for (int eye = 0; eye < eyesCount; eye++)
    {
        // Draw buffers
        if (_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vCounter > 0)
        {
            // Set current shader and upload current MVP matrix
            glUseProgram(_rf_global_context_ptr->gl_ctx.current_shader.id);

            // Create _rf_global_context_ptr->gl_ctx.modelview-_rf_global_context_ptr->gl_ctx.projection matrix
            rf_matrix matMVP = rf_matrix_multiply(_rf_global_context_ptr->gl_ctx.modelview, _rf_global_context_ptr->gl_ctx.projection);

            glUniformMatrix4fv(_rf_global_context_ptr->gl_ctx.current_shader.locs[rf_loc_matrix_mvp], 1, false, rf_matrix_to_floatv(matMVP).v);
            glUniform4f(_rf_global_context_ptr->gl_ctx.current_shader.locs[rf_loc_color_diffuse], 1.0f, 1.0f, 1.0f, 1.0f);
            glUniform1i(_rf_global_context_ptr->gl_ctx.current_shader.locs[rf_loc_map_diffuse], 0);    // Provided value refers to the texture unit (active)

            // TODO: Support additional texture units on custom shader
            //if (_rf_global_context_ptr->gl_ctx.current_shader->locs[rf_loc_map_specular] > 0) glUniform1i(_rf_global_context_ptr->gl_ctx.current_shader.locs[rf_loc_map_specular], 1);
            //if (_rf_global_context_ptr->gl_ctx.current_shader->locs[rf_loc_map_normal] > 0) glUniform1i(_rf_global_context_ptr->gl_ctx.current_shader.locs[rf_loc_map_normal], 2);

            // NOTE: Right now additional map textures not considered for default buffers drawing

            int vertexOffset = 0;

            if (_rf_global_context_ptr->gl_ctx.vao_supported) glBindVertexArray(_rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vao_id);
            else
            {
                // Bind vertex attrib: position (shader-location = 0)
                glBindBuffer(GL_ARRAY_BUFFER, _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vbo_id[0]);
                glVertexAttribPointer(_rf_global_context_ptr->gl_ctx.current_shader.locs[rf_loc_vertex_position], 3, GL_FLOAT, 0, 0, 0);
                glEnableVertexAttribArray(_rf_global_context_ptr->gl_ctx.current_shader.locs[rf_loc_vertex_position]);

                // Bind vertex attrib: texcoord (shader-location = 1)
                glBindBuffer(GL_ARRAY_BUFFER, _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vbo_id[1]);
                glVertexAttribPointer(_rf_global_context_ptr->gl_ctx.current_shader.locs[rf_loc_vertex_texcoord01], 2, GL_FLOAT, 0, 0, 0);
                glEnableVertexAttribArray(_rf_global_context_ptr->gl_ctx.current_shader.locs[rf_loc_vertex_texcoord01]);

                // Bind vertex attrib: color (shader-location = 3)
                glBindBuffer(GL_ARRAY_BUFFER, _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vbo_id[2]);
                glVertexAttribPointer(_rf_global_context_ptr->gl_ctx.current_shader.locs[rf_loc_vertex_color], 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
                glEnableVertexAttribArray(_rf_global_context_ptr->gl_ctx.current_shader.locs[rf_loc_vertex_color]);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vbo_id[3]);
            }

            glActiveTexture(GL_TEXTURE0);

            for (int i = 0; i < _rf_global_context_ptr->gl_ctx.draws_counter; i++)
            {
                glBindTexture(GL_TEXTURE_2D, _rf_global_context_ptr->gl_ctx.draws[i].textureId);

                // TODO: Find some way to bind additional textures --> Use global texture IDs? Register them on draw[i]?
                //if (_rf_global_context_ptr->gl_ctx.current_shader->locs[rf_loc_map_specular] > 0) { glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, textureUnit1_id); }
                //if (_rf_global_context_ptr->gl_ctx.current_shader->locs[rf_loc_map_specular] > 0) { glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, textureUnit2_id); }

                if ((_rf_global_context_ptr->gl_ctx.draws[i].mode == GL_LINES) || (_rf_global_context_ptr->gl_ctx.draws[i].mode == GL_TRIANGLES)) glDrawArrays(_rf_global_context_ptr->gl_ctx.draws[i].mode, vertexOffset, _rf_global_context_ptr->gl_ctx.draws[i].vertex_count);
                else
                {
                    #if defined(RF_GRAPHICS_API_OPENGL_33)
                    // We need to define the number of indices to be processed: quadsCount*6
                    // NOTE: The final parameter tells the GPU the offset in bytes from the
                    // start of the index buffer to the location of the first index to process
                    glDrawElements(GL_TRIANGLES, _rf_global_context_ptr->gl_ctx.draws[i].vertex_count/4*6, GL_UNSIGNED_INT, (GLvoid* )(sizeof(GLuint)*vertexOffset/4*6));
                    #elif defined(RF_GRAPHICS_API_OPENGL_ES2)
                    glDrawElements(GL_TRIANGLES, _rf_global_context_ptr->gl_ctx.draws[i].vertex_count/4*6, GL_UNSIGNED_SHORT, (GLvoid* )(sizeof(GLushort)*vertexOffset/4*6));
                    #endif
                }

                vertexOffset += (_rf_global_context_ptr->gl_ctx.draws[i].vertex_count + _rf_global_context_ptr->gl_ctx.draws[i].vertexAlignment);
            }

            if (!_rf_global_context_ptr->gl_ctx.vao_supported)
            {
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            }

            glBindTexture(GL_TEXTURE_2D, 0);    // Unbind textures
        }

        if (_rf_global_context_ptr->gl_ctx.vao_supported) glBindVertexArray(0); // Unbind VAO

        glUseProgram(0);    // Unbind shader program
    }

    // Reset vertex counters for next frame
    _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].vCounter = 0;
    _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].tcCounter = 0;
    _rf_global_context_ptr->gl_ctx.vertex_data[_rf_global_context_ptr->gl_ctx.current_buffer].cCounter = 0;

    // Reset depth for next draw
    _rf_global_context_ptr->gl_ctx.current_depth = -1.0f;

    // Restore _rf_global_context_ptr->gl_ctx.projection/_rf_global_context_ptr->gl_ctx.modelview matrices
    _rf_global_context_ptr->gl_ctx.projection = matProjection;
    _rf_global_context_ptr->gl_ctx.modelview = matModelView;

    // Reset _rf_global_context_ptr->gl_ctx.draws array
    for (int i = 0; i < rf_max_drawcall_registered; i++)
    {
        _rf_global_context_ptr->gl_ctx.draws[i].mode = GL_QUADS;
        _rf_global_context_ptr->gl_ctx.draws[i].vertex_count = 0;
        _rf_global_context_ptr->gl_ctx.draws[i].textureId = _rf_global_context_ptr->gl_ctx.default_texture_id;
    }

    _rf_global_context_ptr->gl_ctx.draws_counter = 1;

    // Change to next buffer in the list
    _rf_global_context_ptr->gl_ctx.current_buffer++;
    if (_rf_global_context_ptr->gl_ctx.current_buffer >= rf_max_batch_buffering) _rf_global_context_ptr->gl_ctx.current_buffer = 0;
}

// Unload default internal buffers vertex data from CPU and GPU
RF_INTERNAL void _rf_unload_buffers_default()
{
    // Unbind everything
    if (_rf_global_context_ptr->gl_ctx.vao_supported) glBindVertexArray(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    for (int i = 0; i < rf_max_batch_buffering; i++)
    {
        // Delete VBOs from GPU (VRAM)
        glDeleteBuffers(1, &_rf_global_context_ptr->gl_ctx.vertex_data[i].vbo_id[0]);
        glDeleteBuffers(1, &_rf_global_context_ptr->gl_ctx.vertex_data[i].vbo_id[1]);
        glDeleteBuffers(1, &_rf_global_context_ptr->gl_ctx.vertex_data[i].vbo_id[2]);
        glDeleteBuffers(1, &_rf_global_context_ptr->gl_ctx.vertex_data[i].vbo_id[3]);

        // Delete VAOs from GPU (VRAM)
        if (_rf_global_context_ptr->gl_ctx.vao_supported) glDeleteVertexArrays(1, &_rf_global_context_ptr->gl_ctx.vertex_data[i].vao_id);

        // Free vertex arrays memory from CPU (RAM)
        RF_FREE(_rf_global_context_ptr->gl_ctx.vertex_data[i].vertices);
        RF_FREE(_rf_global_context_ptr->gl_ctx.vertex_data[i].texcoords);
        RF_FREE(_rf_global_context_ptr->gl_ctx.vertex_data[i].colors);
        RF_FREE(_rf_global_context_ptr->gl_ctx.vertex_data[i].indices);
    }
}

// Renders a 1x1 XY quad in NDC
RF_INTERNAL void _rf_gen_draw_quad(void)
{
    unsigned int quadVAO = 0;
    unsigned int quadVBO = 0;

    float vertices[] = {
            // Positions        // rf_texture Coords
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    // Set up plane VAO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);

    // Fill buffer
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

    // Link vertex attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void* )0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void* )(3*sizeof(float)));

    // Draw quad
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    glDeleteBuffers(1, &quadVBO);
    glDeleteVertexArrays(1, &quadVAO);
}

// Renders a 1x1 3D cube in NDC
RF_INTERNAL void _rf_gen_draw_cube(void)
{
    unsigned int cubeVAO = 0;
    unsigned int cubeVBO = 0;

    float vertices[] = {
            -1.0f, -1.0f, -1.0f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
            1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
            -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
            1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 1.0f , 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
            1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f
    };

    // Set up cube VAO
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    // Fill buffer
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Link vertex attributes
    glBindVertexArray(cubeVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void* )0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void* )(3*sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void* )(6*sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Draw cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDeleteBuffers(1, &cubeVBO);
    glDeleteVertexArrays(1, &cubeVAO);
}

#endif  // RF_GRAPHICS_API_OPENGL_33 || RF_GRAPHICS_API_OPENGL_ES2

#if defined(RF_GRAPHICS_API_OPENGL_11)
// Mipmaps data is generated after image data
// NOTE: Only works with RGBA (4 bytes) data!
RF_INTERNAL int _rf_generate_mipmaps( unsigned char* data, int baseWidth, int baseHeight)
{
    int mipmapCount = 1;                // Required mipmap levels count (including base level)
    int width = baseWidth;
    int height = baseHeight;
    int size = baseWidth*baseHeight*4;  // Size in bytes (will include mipmaps...), RGBA only

    // Count mipmap levels required
    while ((width != 1) && (height != 1))
    {
        if (width != 1) width /= 2;
        if (height != 1) height /= 2;

        RF_LOG(RF_LOG_DEBUG, "Next mipmap size: %i x %i", width, height);

        mipmapCount++;

        size += (width*height*4);       // Add mipmap size (in bytes)
    }

    RF_LOG(RF_LOG_DEBUG, "Total mipmaps required: %i", mipmapCount);
    RF_LOG(RF_LOG_DEBUG, "Total size of data required: %i", size);

    unsigned char* temp = realloc(data, size);

    if (temp != NULL) data = temp;
    else RF_LOG(RF_LOG_WARNING, "Mipmaps required memory could not be allocated");

    width = baseWidth;
    height = baseHeight;
    size = (width*height*4);

    // Generate mipmaps
    // NOTE: Every mipmap data is stored after data
    rf_color* image = (rf_color* )RF_MALLOC(width*height*sizeof(rf_color));
    rf_color* mipmap = NULL;
    int offset = 0;
    int j = 0;

    for (int i = 0; i < size; i += 4)
    {
        image[j].r = data[i];
        image[j].g = data[i + 1];
        image[j].b = data[i + 2];
        image[j].a = data[i + 3];
        j++;
    }

    RF_LOG(RF_LOG_DEBUG, "Mipmap base (%ix%i)", width, height);

    for (int mip = 1; mip < mipmapCount; mip++)
    {
        mipmap = _rf_gen_next_mipmap(image, width, height);

        offset += (width*height*4); // Size of last mipmap
        j = 0;

        width /= 2;
        height /= 2;
        size = (width*height*4);    // Mipmap size to store after offset

        // Add mipmap to data
        for (int i = 0; i < size; i += 4)
        {
            data[offset + i] = mipmap[j].r;
            data[offset + i + 1] = mipmap[j].g;
            data[offset + i + 2] = mipmap[j].b;
            data[offset + i + 3] = mipmap[j].a;
            j++;
        }

        RF_FREE(image);

        image = mipmap;
        mipmap = NULL;
    }

    RF_FREE(mipmap);       // free mipmap data

    return mipmapCount;
}

// Manual mipmap generation (basic scaling algorithm)
RF_INTERNAL rf_color* _rf_gen_next_mipmap( rf_color* srcData, int srcWidth, int srcHeight)
{
    int x2, y2;
    rf_color prow, pcol;

    int width = srcWidth/2;
    int height = srcHeight/2;

    rf_color* mipmap = (rf_color* )RF_MALLOC(width*height*sizeof(rf_color));

    // Scaling algorithm works perfectly (box-filter)
    for (int y = 0; y < height; y++)
    {
        y2 = 2*y;

        for (int x = 0; x < width; x++)
        {
            x2 = 2*x;

            prow.r = (srcData[y2*srcWidth + x2].r + srcData[y2*srcWidth + x2 + 1].r)/2;
            prow.g = (srcData[y2*srcWidth + x2].g + srcData[y2*srcWidth + x2 + 1].g)/2;
            prow.b = (srcData[y2*srcWidth + x2].b + srcData[y2*srcWidth + x2 + 1].b)/2;
            prow.a = (srcData[y2*srcWidth + x2].a + srcData[y2*srcWidth + x2 + 1].a)/2;

            pcol.r = (srcData[(y2+1)*srcWidth + x2].r + srcData[(y2+1)*srcWidth + x2 + 1].r)/2;
            pcol.g = (srcData[(y2+1)*srcWidth + x2].g + srcData[(y2+1)*srcWidth + x2 + 1].g)/2;
            pcol.b = (srcData[(y2+1)*srcWidth + x2].b + srcData[(y2+1)*srcWidth + x2 + 1].b)/2;
            pcol.a = (srcData[(y2+1)*srcWidth + x2].a + srcData[(y2+1)*srcWidth + x2 + 1].a)/2;

            mipmap[y*width + x].r = (prow.r + pcol.r)/2;
            mipmap[y*width + x].g = (prow.g + pcol.g)/2;
            mipmap[y*width + x].b = (prow.b + pcol.b)/2;
            mipmap[y*width + x].a = (prow.a + pcol.a)/2;
        }
    }

    RF_LOG(RF_LOG_DEBUG, "Mipmap generated successfully (%ix%i)", width, height);

    return mipmap;
}
#endif
//endregion

//region camera

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

// Select camera mode (multiple camera modes available)
RF_API void rf_set_camera_mode(rf_camera3d camera, int mode)
{
    rf_vector3 v1 = camera.position;
    rf_vector3 v2 = camera.target;

    float dx = v2.x - v1.x;
    float dy = v2.y - v1.y;
    float dz = v2.z - v1.z;

    _rf_global_context_ptr->gl_ctx.camera_target_distance = sqrtf(dx*dx + dy*dy + dz*dz);

    rf_vector2 distance = { 0.0f, 0.0f };
    distance.x = sqrtf(dx*dx + dz*dz);
    distance.y = sqrtf(dx*dx + dy*dy);

    // rf_camera3d angle calculation
    _rf_global_context_ptr->gl_ctx.camera_angle.x = asinf( (float)fabs(dx)/distance.x); // rf_camera3d angle in plane XZ (0 aligned with Z, move positive CCW)
    _rf_global_context_ptr->gl_ctx.camera_angle.y = -asinf( (float)fabs(dy)/distance.y); // rf_camera3d angle in plane XY (0 aligned with X, move positive CW)

    _rf_global_context_ptr->gl_ctx.player_eyes_position = camera.position.y;

    // Lock cursor for first person and third person cameras
//    if ((mode == rf_camera_first_person) || (mode == rf_camera_third_person)) DisableCursor();
//    else EnableCursor();

    _rf_global_context_ptr->gl_ctx.camera_mode = mode;
}

// Update camera depending on selected mode
// NOTE: rf_camera3d controls depend on some raylib functions:
//       System: EnableCursor(), DisableCursor()
//       Mouse: IsMouseButtonDown(), GetMousePosition(), GetMouseWheelMove()
//       Keys:  IsKeyDown()
// TODO: Port to quaternion-based camera
RF_API void rf_update_camera3d(rf_camera3d* camera, const rf_input_state_for_update_camera inputState)
{
    static int swingCounter = 0; // Used for 1st person swinging movement
    static rf_vector2 previousMousePosition = { 0.0f, 0.0f };

    // TODO: Compute _rf_global_context_ptr->gl_ctx.camera_target_distance and _rf_global_context_ptr->gl_ctx.camera_angle here

    // Mouse movement detection
    rf_vector2 mousePositionDelta = { 0.0f, 0.0f };
    rf_vector2 mouse_position = inputState.mouse_position;
    int mouse_wheel_move = inputState.mouse_wheel_move;

    // Keys input detection
    bool panKey = inputState.is_camera_pan_control_key_down;
    bool altKey = inputState.is_camera_alt_control_key_down;
    bool szoomKey = inputState.is_camera_smooth_zoom_control_key;

    bool direction[6];
    direction[0] = inputState.direction_keys[0];
    direction[1] = inputState.direction_keys[1];
    direction[2] = inputState.direction_keys[2];
    direction[3] = inputState.direction_keys[3];
    direction[4] = inputState.direction_keys[4];
    direction[5] = inputState.direction_keys[5];

    // TODO: Consider touch inputs for camera

    if (_rf_global_context_ptr->gl_ctx.camera_mode != rf_camera_custom)
    {
        mousePositionDelta.x = mouse_position.x - previousMousePosition.x;
        mousePositionDelta.y = mouse_position.y - previousMousePosition.y;

        previousMousePosition = mouse_position;
    }

    // Support for multiple automatic camera modes
    switch (_rf_global_context_ptr->gl_ctx.camera_mode)
    {
        case rf_camera_free:
        {
            // rf_camera3d zoom
            if ((_rf_global_context_ptr->gl_ctx.camera_target_distance < rf_camera_free_distance_max_clamp) && (mouse_wheel_move < 0))
            {
                _rf_global_context_ptr->gl_ctx.camera_target_distance -= (mouse_wheel_move*rf_camera_mouse_scroll_sensitivity);

                if (_rf_global_context_ptr->gl_ctx.camera_target_distance > rf_camera_free_distance_max_clamp) _rf_global_context_ptr->gl_ctx.camera_target_distance = rf_camera_free_distance_max_clamp;
            }
                // rf_camera3d looking down
                // TODO: Review, weird comparisson of _rf_global_context_ptr->gl_ctx.camera_target_distance == 120.0f?
            else if ((camera->position.y > camera->target.y) && (_rf_global_context_ptr->gl_ctx.camera_target_distance == rf_camera_free_distance_max_clamp) && (mouse_wheel_move < 0))
            {
                camera->target.x += mouse_wheel_move*(camera->target.x - camera->position.x)*rf_camera_mouse_scroll_sensitivity/_rf_global_context_ptr->gl_ctx.camera_target_distance;
                camera->target.y += mouse_wheel_move*(camera->target.y - camera->position.y)*rf_camera_mouse_scroll_sensitivity/_rf_global_context_ptr->gl_ctx.camera_target_distance;
                camera->target.z += mouse_wheel_move*(camera->target.z - camera->position.z)*rf_camera_mouse_scroll_sensitivity/_rf_global_context_ptr->gl_ctx.camera_target_distance;
            }
            else if ((camera->position.y > camera->target.y) && (camera->target.y >= 0))
            {
                camera->target.x += mouse_wheel_move*(camera->target.x - camera->position.x)*rf_camera_mouse_scroll_sensitivity/_rf_global_context_ptr->gl_ctx.camera_target_distance;
                camera->target.y += mouse_wheel_move*(camera->target.y - camera->position.y)*rf_camera_mouse_scroll_sensitivity/_rf_global_context_ptr->gl_ctx.camera_target_distance;
                camera->target.z += mouse_wheel_move*(camera->target.z - camera->position.z)*rf_camera_mouse_scroll_sensitivity/_rf_global_context_ptr->gl_ctx.camera_target_distance;

                // if (camera->target.y < 0) camera->target.y = -0.001;
            }
            else if ((camera->position.y > camera->target.y) && (camera->target.y < 0) && (mouse_wheel_move > 0))
            {
                _rf_global_context_ptr->gl_ctx.camera_target_distance -= (mouse_wheel_move*rf_camera_mouse_scroll_sensitivity);
                if (_rf_global_context_ptr->gl_ctx.camera_target_distance < rf_camera_free_distance_min_clamp) _rf_global_context_ptr->gl_ctx.camera_target_distance = rf_camera_free_distance_min_clamp;
            }
                // rf_camera3d looking up
                // TODO: Review, weird comparisson of _rf_global_context_ptr->gl_ctx.camera_target_distance == 120.0f?
            else if ((camera->position.y < camera->target.y) && (_rf_global_context_ptr->gl_ctx.camera_target_distance == rf_camera_free_distance_max_clamp) && (mouse_wheel_move < 0))
            {
                camera->target.x += mouse_wheel_move*(camera->target.x - camera->position.x)*rf_camera_mouse_scroll_sensitivity/_rf_global_context_ptr->gl_ctx.camera_target_distance;
                camera->target.y += mouse_wheel_move*(camera->target.y - camera->position.y)*rf_camera_mouse_scroll_sensitivity/_rf_global_context_ptr->gl_ctx.camera_target_distance;
                camera->target.z += mouse_wheel_move*(camera->target.z - camera->position.z)*rf_camera_mouse_scroll_sensitivity/_rf_global_context_ptr->gl_ctx.camera_target_distance;
            }
            else if ((camera->position.y < camera->target.y) && (camera->target.y <= 0))
            {
                camera->target.x += mouse_wheel_move*(camera->target.x - camera->position.x)*rf_camera_mouse_scroll_sensitivity/_rf_global_context_ptr->gl_ctx.camera_target_distance;
                camera->target.y += mouse_wheel_move*(camera->target.y - camera->position.y)*rf_camera_mouse_scroll_sensitivity/_rf_global_context_ptr->gl_ctx.camera_target_distance;
                camera->target.z += mouse_wheel_move*(camera->target.z - camera->position.z)*rf_camera_mouse_scroll_sensitivity/_rf_global_context_ptr->gl_ctx.camera_target_distance;

                // if (camera->target.y > 0) camera->target.y = 0.001;
            }
            else if ((camera->position.y < camera->target.y) && (camera->target.y > 0) && (mouse_wheel_move > 0))
            {
                _rf_global_context_ptr->gl_ctx.camera_target_distance -= (mouse_wheel_move*rf_camera_mouse_scroll_sensitivity);
                if (_rf_global_context_ptr->gl_ctx.camera_target_distance < rf_camera_free_distance_min_clamp) _rf_global_context_ptr->gl_ctx.camera_target_distance = rf_camera_free_distance_min_clamp;
            }

            // Input keys checks
            if (panKey)
            {
                if (altKey) // Alternative key behaviour
                {
                    if (szoomKey)
                    {
                        // rf_camera3d smooth zoom
                        _rf_global_context_ptr->gl_ctx.camera_target_distance += (mousePositionDelta.y*rf_camera_free_smooth_zoom_sensitivity);
                    }
                    else
                    {
                        // rf_camera3d rotation
                        _rf_global_context_ptr->gl_ctx.camera_angle.x += mousePositionDelta.x*-rf_camera_free_mouse_sensitivity;
                        _rf_global_context_ptr->gl_ctx.camera_angle.y += mousePositionDelta.y*-rf_camera_free_mouse_sensitivity;

                        // Angle clamp
                        if (_rf_global_context_ptr->gl_ctx.camera_angle.y > rf_camera_free_min_clamp*RF_DEG2RAD) _rf_global_context_ptr->gl_ctx.camera_angle.y = rf_camera_free_min_clamp*RF_DEG2RAD;
                        else if (_rf_global_context_ptr->gl_ctx.camera_angle.y < rf_camera_free_max_clamp*RF_DEG2RAD) _rf_global_context_ptr->gl_ctx.camera_angle.y = rf_camera_free_max_clamp*RF_DEG2RAD;
                    }
                }
                else
                {
                    // rf_camera3d panning
                    camera->target.x += ((mousePositionDelta.x*-rf_camera_free_mouse_sensitivity)*cosf(_rf_global_context_ptr->gl_ctx.camera_angle.x) + (mousePositionDelta.y*rf_camera_free_mouse_sensitivity)*sinf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*sinf(_rf_global_context_ptr->gl_ctx.camera_angle.y))*(_rf_global_context_ptr->gl_ctx.camera_target_distance/rf_camera_free_panning_divider);
                    camera->target.y += ((mousePositionDelta.y*rf_camera_free_mouse_sensitivity)*cosf(_rf_global_context_ptr->gl_ctx.camera_angle.y))*(_rf_global_context_ptr->gl_ctx.camera_target_distance/rf_camera_free_panning_divider);
                    camera->target.z += ((mousePositionDelta.x*rf_camera_free_mouse_sensitivity)*sinf(_rf_global_context_ptr->gl_ctx.camera_angle.x) + (mousePositionDelta.y*rf_camera_free_mouse_sensitivity)*cosf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*sinf(_rf_global_context_ptr->gl_ctx.camera_angle.y))*(_rf_global_context_ptr->gl_ctx.camera_target_distance/rf_camera_free_panning_divider);
                }
            }

            // Update camera position with changes
            camera->position.x = sinf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*_rf_global_context_ptr->gl_ctx.camera_target_distance*cosf(_rf_global_context_ptr->gl_ctx.camera_angle.y) + camera->target.x;
            camera->position.y = ((_rf_global_context_ptr->gl_ctx.camera_angle.y <= 0.0f)? 1 : -1)*sinf(_rf_global_context_ptr->gl_ctx.camera_angle.y)*_rf_global_context_ptr->gl_ctx.camera_target_distance*sinf(_rf_global_context_ptr->gl_ctx.camera_angle.y) + camera->target.y;
            camera->position.z = cosf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*_rf_global_context_ptr->gl_ctx.camera_target_distance*cosf(_rf_global_context_ptr->gl_ctx.camera_angle.y) + camera->target.z;

        } break;
        case rf_camera_orbital:
        {
            _rf_global_context_ptr->gl_ctx.camera_angle.x += rf_camera_orbital_speed; // rf_camera3d orbit angle
            _rf_global_context_ptr->gl_ctx.camera_target_distance -= (mouse_wheel_move*rf_camera_mouse_scroll_sensitivity); // rf_camera3d zoom

            // rf_camera3d distance clamp
            if (_rf_global_context_ptr->gl_ctx.camera_target_distance < rf_camera_third_person_distance_clamp) _rf_global_context_ptr->gl_ctx.camera_target_distance = rf_camera_third_person_distance_clamp;

            // Update camera position with changes
            camera->position.x = sinf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*_rf_global_context_ptr->gl_ctx.camera_target_distance*cosf(_rf_global_context_ptr->gl_ctx.camera_angle.y) + camera->target.x;
            camera->position.y = ((_rf_global_context_ptr->gl_ctx.camera_angle.y <= 0.0f)? 1 : -1)*sinf(_rf_global_context_ptr->gl_ctx.camera_angle.y)*_rf_global_context_ptr->gl_ctx.camera_target_distance*sinf(_rf_global_context_ptr->gl_ctx.camera_angle.y) + camera->target.y;
            camera->position.z = cosf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*_rf_global_context_ptr->gl_ctx.camera_target_distance*cosf(_rf_global_context_ptr->gl_ctx.camera_angle.y) + camera->target.z;

        } break;
        case rf_camera_first_person:
        {
            camera->position.x += (sinf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*direction[rf_move_back] -
                                   sinf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*direction[rf_move_front] -
                                   cosf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*direction[rf_move_left] +
                                   cosf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*direction[rf_move_right])/rf_player_movement_sensitivity;

            camera->position.y += (sinf(_rf_global_context_ptr->gl_ctx.camera_angle.y)*direction[rf_move_front] -
                                   sinf(_rf_global_context_ptr->gl_ctx.camera_angle.y)*direction[rf_move_back] +
                                   1.0f*direction[rf_move_up] - 1.0f*direction[rf_move_down])/rf_player_movement_sensitivity;

            camera->position.z += (cosf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*direction[rf_move_back] -
                                   cosf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*direction[rf_move_front] +
                                   sinf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*direction[rf_move_left] -
                                   sinf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*direction[rf_move_right])/rf_player_movement_sensitivity;

            bool isMoving = false; // Required for swinging

            for (int i = 0; i < 6; i++) if (direction[i]) { isMoving = true; break; }

            // rf_camera3d orientation calculation
            _rf_global_context_ptr->gl_ctx.camera_angle.x += (mousePositionDelta.x*-rf_camera_mouse_move_sensitivity);
            _rf_global_context_ptr->gl_ctx.camera_angle.y += (mousePositionDelta.y*-rf_camera_mouse_move_sensitivity);

            // Angle clamp
            if (_rf_global_context_ptr->gl_ctx.camera_angle.y > rf_camera_first_person_min_clamp*RF_DEG2RAD) _rf_global_context_ptr->gl_ctx.camera_angle.y = rf_camera_first_person_min_clamp*RF_DEG2RAD;
            else if (_rf_global_context_ptr->gl_ctx.camera_angle.y < rf_camera_first_person_max_clamp*RF_DEG2RAD) _rf_global_context_ptr->gl_ctx.camera_angle.y = rf_camera_first_person_max_clamp*RF_DEG2RAD;

            // rf_camera3d is always looking at player
            camera->target.x = camera->position.x - sinf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*rf_camera_first_person_focus_distance;
            camera->target.y = camera->position.y + sinf(_rf_global_context_ptr->gl_ctx.camera_angle.y)*rf_camera_first_person_focus_distance;
            camera->target.z = camera->position.z - cosf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*rf_camera_first_person_focus_distance;

            if (isMoving) swingCounter++;

            // rf_camera3d position update
            // NOTE: On rf_camera_first_person player Y-movement is limited to player 'eyes position'
            camera->position.y = _rf_global_context_ptr->gl_ctx.player_eyes_position - sinf(swingCounter/rf_camera_first_person_step_trigonometric_divider)/rf_camera_first_person_step_divider;

            camera->up.x = sinf(swingCounter/(rf_camera_first_person_step_trigonometric_divider*2))/rf_camera_first_person_waving_divider;
            camera->up.z = -sinf(swingCounter/(rf_camera_first_person_step_trigonometric_divider*2))/rf_camera_first_person_waving_divider;


        } break;
        case rf_camera_third_person:
        {
            camera->position.x += (sinf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*direction[rf_move_back] -
                                   sinf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*direction[rf_move_front] -
                                   cosf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*direction[rf_move_left] +
                                   cosf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*direction[rf_move_right])/rf_player_movement_sensitivity;

            camera->position.y += (sinf(_rf_global_context_ptr->gl_ctx.camera_angle.y)*direction[rf_move_front] -
                                   sinf(_rf_global_context_ptr->gl_ctx.camera_angle.y)*direction[rf_move_back] +
                                   1.0f*direction[rf_move_up] - 1.0f*direction[rf_move_down])/rf_player_movement_sensitivity;

            camera->position.z += (cosf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*direction[rf_move_back] -
                                   cosf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*direction[rf_move_front] +
                                   sinf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*direction[rf_move_left] -
                                   sinf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*direction[rf_move_right])/rf_player_movement_sensitivity;

            // rf_camera3d orientation calculation
            _rf_global_context_ptr->gl_ctx.camera_angle.x += (mousePositionDelta.x*-rf_camera_mouse_move_sensitivity);
            _rf_global_context_ptr->gl_ctx.camera_angle.y += (mousePositionDelta.y*-rf_camera_mouse_move_sensitivity);

            // Angle clamp
            if (_rf_global_context_ptr->gl_ctx.camera_angle.y > rf_camera_third_person_min_clamp*RF_DEG2RAD) _rf_global_context_ptr->gl_ctx.camera_angle.y = rf_camera_third_person_min_clamp*RF_DEG2RAD;
            else if (_rf_global_context_ptr->gl_ctx.camera_angle.y < rf_camera_third_person_max_clamp*RF_DEG2RAD) _rf_global_context_ptr->gl_ctx.camera_angle.y = rf_camera_third_person_max_clamp*RF_DEG2RAD;

            // rf_camera3d zoom
            _rf_global_context_ptr->gl_ctx.camera_target_distance -= (mouse_wheel_move*rf_camera_mouse_scroll_sensitivity);

            // rf_camera3d distance clamp
            if (_rf_global_context_ptr->gl_ctx.camera_target_distance < rf_camera_third_person_distance_clamp) _rf_global_context_ptr->gl_ctx.camera_target_distance = rf_camera_third_person_distance_clamp;

            // TODO: It seems camera->position is not correctly updated or some rounding issue makes the camera move straight to camera->target...
            camera->position.x = sinf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*_rf_global_context_ptr->gl_ctx.camera_target_distance*cosf(_rf_global_context_ptr->gl_ctx.camera_angle.y) + camera->target.x;
            if (_rf_global_context_ptr->gl_ctx.camera_angle.y <= 0.0f) camera->position.y = sinf(_rf_global_context_ptr->gl_ctx.camera_angle.y)*_rf_global_context_ptr->gl_ctx.camera_target_distance*sinf(_rf_global_context_ptr->gl_ctx.camera_angle.y) + camera->target.y;
            else camera->position.y = -sinf(_rf_global_context_ptr->gl_ctx.camera_angle.y)*_rf_global_context_ptr->gl_ctx.camera_target_distance*sinf(_rf_global_context_ptr->gl_ctx.camera_angle.y) + camera->target.y;
            camera->position.z = cosf(_rf_global_context_ptr->gl_ctx.camera_angle.x)*_rf_global_context_ptr->gl_ctx.camera_target_distance*cosf(_rf_global_context_ptr->gl_ctx.camera_angle.y) + camera->target.z;

        } break;
        default: break;
    }
}

// Set camera pan key to combine with mouse movement (free camera)
RF_API void rf_set_camera_pan_control(int panKey) { _rf_global_context_ptr->gl_ctx.camera_pan_control_key = panKey; }

// Set camera alt key to combine with mouse movement (free camera)
RF_API void rf_set_camera_alt_control(int altKey) { _rf_global_context_ptr->gl_ctx.camera_alt_control_key = altKey; }

// Set camera smooth zoom key to combine with mouse (free camera)
RF_API void rf_set_camera_smooth_zoom_control(int szoomKey) { _rf_global_context_ptr->gl_ctx.camera_smooth_zoom_control_key = szoomKey; }

// Set camera move controls (1st person and 3rd person cameras)
RF_API void rf_set_camera_move_controls(int frontKey, int backKey, int rightKey, int leftKey, int upKey, int downKey)
{
    _rf_global_context_ptr->gl_ctx.camera_move_control[rf_move_front] = frontKey;
    _rf_global_context_ptr->gl_ctx.camera_move_control[rf_move_back] = backKey;
    _rf_global_context_ptr->gl_ctx.camera_move_control[rf_move_right] = rightKey;
    _rf_global_context_ptr->gl_ctx.camera_move_control[rf_move_left] = leftKey;
    _rf_global_context_ptr->gl_ctx.camera_move_control[rf_move_up] = upKey;
    _rf_global_context_ptr->gl_ctx.camera_move_control[rf_move_down] = downKey;
}


//endregion

//region models

#define rf_max_mesh_vbo 7 // Maximum number of vbo per mesh

RF_INTERNAL rf_model _rf_load_obj(const char* fileName); // Load OBJ mesh data
RF_INTERNAL rf_model _rf_load_iqm(const char* fileName); // Load IQM mesh data
RF_INTERNAL rf_model _rf_load_gltf(const char* fileName); // Load GLTF mesh data

// Draw a line in 3D world space
RF_API void rf_draw_line3d(rf_vector3 startPos, rf_vector3 endPos, rf_color color)
{
    rf_gl_begin(GL_LINES);
    rf_gl_color4ub(color.r, color.g, color.b, color.a);
    rf_gl_vertex3f(startPos.x, startPos.y, startPos.z);
    rf_gl_vertex3f(endPos.x, endPos.y, endPos.z);
    rf_gl_end();
}

// Draw a circle in 3D world space
RF_API void rf_draw_circle3d(rf_vector3 center, float radius, rf_vector3 rotationAxis, float rotationAngle, rf_color color)
{
    if (rf_gl_check_buffer_limit(2*36)) rf_gl_draw();

    rf_push_matrix();
    rf_translatef(center.x, center.y, center.z);
    rf_rotatef(rotationAngle, rotationAxis.x, rotationAxis.y, rotationAxis.z);

    rf_gl_begin(GL_LINES);
    for (int i = 0; i < 360; i += 10)
    {
        rf_gl_color4ub(color.r, color.g, color.b, color.a);

        rf_gl_vertex3f(sinf(RF_DEG2RAD*i)*radius, cosf(RF_DEG2RAD*i)*radius, 0.0f);
        rf_gl_vertex3f(sinf(RF_DEG2RAD*(i + 10))*radius, cosf(RF_DEG2RAD*(i + 10))*radius, 0.0f);
    }
    rf_gl_end();
    rf_pop_matrix();
}

// Draw cube
// NOTE: Cube position is the center position
RF_API void rf_draw_cube(rf_vector3 position, float width, float height, float length, rf_color color)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    if (rf_gl_check_buffer_limit(36)) rf_gl_draw();

    rf_push_matrix();
    // NOTE: Transformation is applied in inverse order (scale -> rotate -> translate)
    rf_translatef(position.x, position.y, position.z);
    //rf_rotatef(45, 0, 1, 0);
    //rf_scalef(1.0f, 1.0f, 1.0f);   // NOTE: Vertices are directly scaled on definition

    rf_gl_begin(GL_TRIANGLES);
    rf_gl_color4ub(color.r, color.g, color.b, color.a);

    // Front face
    rf_gl_vertex3f(x - width/2, y - height/2, z + length/2); // Bottom Left
    rf_gl_vertex3f(x + width/2, y - height/2, z + length/2); // Bottom Right
    rf_gl_vertex3f(x - width/2, y + height/2, z + length/2); // Top Left

    rf_gl_vertex3f(x + width/2, y + height/2, z + length/2); // Top Right
    rf_gl_vertex3f(x - width/2, y + height/2, z + length/2); // Top Left
    rf_gl_vertex3f(x + width/2, y - height/2, z + length/2); // Bottom Right

    // Back face
    rf_gl_vertex3f(x - width/2, y - height/2, z - length/2); // Bottom Left
    rf_gl_vertex3f(x - width/2, y + height/2, z - length/2); // Top Left
    rf_gl_vertex3f(x + width/2, y - height/2, z - length/2); // Bottom Right

    rf_gl_vertex3f(x + width/2, y + height/2, z - length/2); // Top Right
    rf_gl_vertex3f(x + width/2, y - height/2, z - length/2); // Bottom Right
    rf_gl_vertex3f(x - width/2, y + height/2, z - length/2); // Top Left

    // Top face
    rf_gl_vertex3f(x - width/2, y + height/2, z - length/2); // Top Left
    rf_gl_vertex3f(x - width/2, y + height/2, z + length/2); // Bottom Left
    rf_gl_vertex3f(x + width/2, y + height/2, z + length/2); // Bottom Right

    rf_gl_vertex3f(x + width/2, y + height/2, z - length/2); // Top Right
    rf_gl_vertex3f(x - width/2, y + height/2, z - length/2); // Top Left
    rf_gl_vertex3f(x + width/2, y + height/2, z + length/2); // Bottom Right

    // Bottom face
    rf_gl_vertex3f(x - width/2, y - height/2, z - length/2); // Top Left
    rf_gl_vertex3f(x + width/2, y - height/2, z + length/2); // Bottom Right
    rf_gl_vertex3f(x - width/2, y - height/2, z + length/2); // Bottom Left

    rf_gl_vertex3f(x + width/2, y - height/2, z - length/2); // Top Right
    rf_gl_vertex3f(x + width/2, y - height/2, z + length/2); // Bottom Right
    rf_gl_vertex3f(x - width/2, y - height/2, z - length/2); // Top Left

    // Right face
    rf_gl_vertex3f(x + width/2, y - height/2, z - length/2); // Bottom Right
    rf_gl_vertex3f(x + width/2, y + height/2, z - length/2); // Top Right
    rf_gl_vertex3f(x + width/2, y + height/2, z + length/2); // Top Left

    rf_gl_vertex3f(x + width/2, y - height/2, z + length/2); // Bottom Left
    rf_gl_vertex3f(x + width/2, y - height/2, z - length/2); // Bottom Right
    rf_gl_vertex3f(x + width/2, y + height/2, z + length/2); // Top Left

    // Left face
    rf_gl_vertex3f(x - width/2, y - height/2, z - length/2); // Bottom Right
    rf_gl_vertex3f(x - width/2, y + height/2, z + length/2); // Top Left
    rf_gl_vertex3f(x - width/2, y + height/2, z - length/2); // Top Right

    rf_gl_vertex3f(x - width/2, y - height/2, z + length/2); // Bottom Left
    rf_gl_vertex3f(x - width/2, y + height/2, z + length/2); // Top Left
    rf_gl_vertex3f(x - width/2, y - height/2, z - length/2); // Bottom Right
    rf_gl_end();
    rf_pop_matrix();
}

// Draw cube (Vector version)
RF_API void rf_draw_cube_v(rf_vector3 position, rf_vector3 size, rf_color color)
{
    rf_draw_cube(position, size.x, size.y, size.z, color);
}

// Draw cube wires
RF_API void rf_draw_cube_wires(rf_vector3 position, float width, float height, float length, rf_color color)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    if (rf_gl_check_buffer_limit(36)) rf_gl_draw();

    rf_push_matrix();
    rf_translatef(position.x, position.y, position.z);

    rf_gl_begin(GL_LINES);
    rf_gl_color4ub(color.r, color.g, color.b, color.a);

    // Front Face -----------------------------------------------------
    // Bottom Line
    rf_gl_vertex3f(x-width/2, y-height/2, z+length/2); // Bottom Left
    rf_gl_vertex3f(x+width/2, y-height/2, z+length/2); // Bottom Right

    // Left Line
    rf_gl_vertex3f(x+width/2, y-height/2, z+length/2); // Bottom Right
    rf_gl_vertex3f(x+width/2, y+height/2, z+length/2); // Top Right

    // Top Line
    rf_gl_vertex3f(x+width/2, y+height/2, z+length/2); // Top Right
    rf_gl_vertex3f(x-width/2, y+height/2, z+length/2); // Top Left

    // Right Line
    rf_gl_vertex3f(x-width/2, y+height/2, z+length/2); // Top Left
    rf_gl_vertex3f(x-width/2, y-height/2, z+length/2); // Bottom Left

    // Back Face ------------------------------------------------------
    // Bottom Line
    rf_gl_vertex3f(x-width/2, y-height/2, z-length/2); // Bottom Left
    rf_gl_vertex3f(x+width/2, y-height/2, z-length/2); // Bottom Right

    // Left Line
    rf_gl_vertex3f(x+width/2, y-height/2, z-length/2); // Bottom Right
    rf_gl_vertex3f(x+width/2, y+height/2, z-length/2); // Top Right

    // Top Line
    rf_gl_vertex3f(x+width/2, y+height/2, z-length/2); // Top Right
    rf_gl_vertex3f(x-width/2, y+height/2, z-length/2); // Top Left

    // Right Line
    rf_gl_vertex3f(x-width/2, y+height/2, z-length/2); // Top Left
    rf_gl_vertex3f(x-width/2, y-height/2, z-length/2); // Bottom Left

    // Top Face -------------------------------------------------------
    // Left Line
    rf_gl_vertex3f(x-width/2, y+height/2, z+length/2); // Top Left Front
    rf_gl_vertex3f(x-width/2, y+height/2, z-length/2); // Top Left Back

    // Right Line
    rf_gl_vertex3f(x+width/2, y+height/2, z+length/2); // Top Right Front
    rf_gl_vertex3f(x+width/2, y+height/2, z-length/2); // Top Right Back

    // Bottom Face  ---------------------------------------------------
    // Left Line
    rf_gl_vertex3f(x-width/2, y-height/2, z+length/2); // Top Left Front
    rf_gl_vertex3f(x-width/2, y-height/2, z-length/2); // Top Left Back

    // Right Line
    rf_gl_vertex3f(x+width/2, y-height/2, z+length/2); // Top Right Front
    rf_gl_vertex3f(x+width/2, y-height/2, z-length/2); // Top Right Back
    rf_gl_end();
    rf_pop_matrix();
}

// Draw cube wires (vector version)
RF_API void rf_draw_cube_wires_v(rf_vector3 position, rf_vector3 size, rf_color color)
{
    rf_draw_cube_wires(position, size.x, size.y, size.z, color);
}

// Draw cube
// NOTE: Cube position is the center position
RF_API void rf_draw_cube_texture(rf_texture2d texture, rf_vector3 position, float width, float height, float length, rf_color color)
{
    float x = position.x;
    float y = position.y;
    float z = position.z;

    if (rf_gl_check_buffer_limit(36)) rf_gl_draw();

    rf_gl_enable_texture(texture.id);

    //rf_push_matrix();
    // NOTE: Transformation is applied in inverse order (scale -> rotate -> translate)
    //rf_translatef(2.0f, 0.0f, 0.0f);
    //rf_rotatef(45, 0, 1, 0);
    //rf_scalef(2.0f, 2.0f, 2.0f);

    rf_gl_begin(GL_QUADS);
    rf_gl_color4ub(color.r, color.g, color.b, color.a);
    // Front Face
    rf_gl_normal3f(0.0f, 0.0f, 1.0f); // Normal Pointing Towards Viewer
    rf_gl_tex_coord2f(0.0f, 0.0f); rf_gl_vertex3f(x - width/2, y - height/2, z + length/2); // Bottom Left Of The rf_texture and Quad
    rf_gl_tex_coord2f(1.0f, 0.0f); rf_gl_vertex3f(x + width/2, y - height/2, z + length/2); // Bottom Right Of The rf_texture and Quad
    rf_gl_tex_coord2f(1.0f, 1.0f); rf_gl_vertex3f(x + width/2, y + height/2, z + length/2); // Top Right Of The rf_texture and Quad
    rf_gl_tex_coord2f(0.0f, 1.0f); rf_gl_vertex3f(x - width/2, y + height/2, z + length/2); // Top Left Of The rf_texture and Quad
    // Back Face
    rf_gl_normal3f(0.0f, 0.0f, - 1.0f); // Normal Pointing Away From Viewer
    rf_gl_tex_coord2f(1.0f, 0.0f); rf_gl_vertex3f(x - width/2, y - height/2, z - length/2); // Bottom Right Of The rf_texture and Quad
    rf_gl_tex_coord2f(1.0f, 1.0f); rf_gl_vertex3f(x - width/2, y + height/2, z - length/2); // Top Right Of The rf_texture and Quad
    rf_gl_tex_coord2f(0.0f, 1.0f); rf_gl_vertex3f(x + width/2, y + height/2, z - length/2); // Top Left Of The rf_texture and Quad
    rf_gl_tex_coord2f(0.0f, 0.0f); rf_gl_vertex3f(x + width/2, y - height/2, z - length/2); // Bottom Left Of The rf_texture and Quad
    // Top Face
    rf_gl_normal3f(0.0f, 1.0f, 0.0f); // Normal Pointing Up
    rf_gl_tex_coord2f(0.0f, 1.0f); rf_gl_vertex3f(x - width/2, y + height/2, z - length/2); // Top Left Of The rf_texture and Quad
    rf_gl_tex_coord2f(0.0f, 0.0f); rf_gl_vertex3f(x - width/2, y + height/2, z + length/2); // Bottom Left Of The rf_texture and Quad
    rf_gl_tex_coord2f(1.0f, 0.0f); rf_gl_vertex3f(x + width/2, y + height/2, z + length/2); // Bottom Right Of The rf_texture and Quad
    rf_gl_tex_coord2f(1.0f, 1.0f); rf_gl_vertex3f(x + width/2, y + height/2, z - length/2); // Top Right Of The rf_texture and Quad
    // Bottom Face
    rf_gl_normal3f(0.0f, - 1.0f, 0.0f); // Normal Pointing Down
    rf_gl_tex_coord2f(1.0f, 1.0f); rf_gl_vertex3f(x - width/2, y - height/2, z - length/2); // Top Right Of The rf_texture and Quad
    rf_gl_tex_coord2f(0.0f, 1.0f); rf_gl_vertex3f(x + width/2, y - height/2, z - length/2); // Top Left Of The rf_texture and Quad
    rf_gl_tex_coord2f(0.0f, 0.0f); rf_gl_vertex3f(x + width/2, y - height/2, z + length/2); // Bottom Left Of The rf_texture and Quad
    rf_gl_tex_coord2f(1.0f, 0.0f); rf_gl_vertex3f(x - width/2, y - height/2, z + length/2); // Bottom Right Of The rf_texture and Quad
    // Right face
    rf_gl_normal3f(1.0f, 0.0f, 0.0f); // Normal Pointing Right
    rf_gl_tex_coord2f(1.0f, 0.0f); rf_gl_vertex3f(x + width/2, y - height/2, z - length/2); // Bottom Right Of The rf_texture and Quad
    rf_gl_tex_coord2f(1.0f, 1.0f); rf_gl_vertex3f(x + width/2, y + height/2, z - length/2); // Top Right Of The rf_texture and Quad
    rf_gl_tex_coord2f(0.0f, 1.0f); rf_gl_vertex3f(x + width/2, y + height/2, z + length/2); // Top Left Of The rf_texture and Quad
    rf_gl_tex_coord2f(0.0f, 0.0f); rf_gl_vertex3f(x + width/2, y - height/2, z + length/2); // Bottom Left Of The rf_texture and Quad
    // Left Face
    rf_gl_normal3f(-1.0f, 0.0f, 0.0f); // Normal Pointing Left
    rf_gl_tex_coord2f(0.0f, 0.0f); rf_gl_vertex3f(x - width/2, y - height/2, z - length/2); // Bottom Left Of The rf_texture and Quad
    rf_gl_tex_coord2f(1.0f, 0.0f); rf_gl_vertex3f(x - width/2, y - height/2, z + length/2); // Bottom Right Of The rf_texture and Quad
    rf_gl_tex_coord2f(1.0f, 1.0f); rf_gl_vertex3f(x - width/2, y + height/2, z + length/2); // Top Right Of The rf_texture and Quad
    rf_gl_tex_coord2f(0.0f, 1.0f); rf_gl_vertex3f(x - width/2, y + height/2, z - length/2); // Top Left Of The rf_texture and Quad
    rf_gl_end();
    //rf_pop_matrix();

    rf_gl_disable_texture();
}

// Draw sphere
RF_API void rf_draw_sphere(rf_vector3 centerPos, float radius, rf_color color)
{
    rf_draw_sphere_ex(centerPos, radius, 16, 16, color);
}

// Draw sphere with extended parameters
RF_API void rf_draw_sphere_ex(rf_vector3 centerPos, float radius, int rings, int slices, rf_color color)
{
    int numVertex = (rings + 2)*slices*6;
    if (rf_gl_check_buffer_limit(numVertex)) rf_gl_draw();

    rf_push_matrix();
    // NOTE: Transformation is applied in inverse order (scale -> translate)
    rf_translatef(centerPos.x, centerPos.y, centerPos.z);
    rf_scalef(radius, radius, radius);

    rf_gl_begin(GL_TRIANGLES);
    rf_gl_color4ub(color.r, color.g, color.b, color.a);

    for (int i = 0; i < (rings + 2); i++)
    {
        for (int j = 0; j < slices; j++)
        {
            rf_gl_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*i))*sinf(RF_DEG2RAD*(j*360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*i)),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*i))*cosf(RF_DEG2RAD*(j*360/slices)));
            rf_gl_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(RF_DEG2RAD*((j+1)*360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(RF_DEG2RAD*((j+1)*360/slices)));
            rf_gl_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(RF_DEG2RAD*(j*360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(RF_DEG2RAD*(j*360/slices)));

            rf_gl_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*i))*sinf(RF_DEG2RAD*(j*360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*i)),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*i))*cosf(RF_DEG2RAD*(j*360/slices)));
            rf_gl_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i)))*sinf(RF_DEG2RAD*((j+1)*360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*(i))),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i)))*cosf(RF_DEG2RAD*((j+1)*360/slices)));
            rf_gl_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(RF_DEG2RAD*((j+1)*360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(RF_DEG2RAD*((j+1)*360/slices)));
        }
    }
    rf_gl_end();
    rf_pop_matrix();
}

// Draw sphere wires
RF_API void rf_draw_sphere_wires(rf_vector3 centerPos, float radius, int rings, int slices, rf_color color)
{
    int numVertex = (rings + 2)*slices*6;
    if (rf_gl_check_buffer_limit(numVertex)) rf_gl_draw();

    rf_push_matrix();
    // NOTE: Transformation is applied in inverse order (scale -> translate)
    rf_translatef(centerPos.x, centerPos.y, centerPos.z);
    rf_scalef(radius, radius, radius);

    rf_gl_begin(GL_LINES);
    rf_gl_color4ub(color.r, color.g, color.b, color.a);

    for (int i = 0; i < (rings + 2); i++)
    {
        for (int j = 0; j < slices; j++)
        {
            rf_gl_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*i))*sinf(RF_DEG2RAD*(j*360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*i)),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*i))*cosf(RF_DEG2RAD*(j*360/slices)));
            rf_gl_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(RF_DEG2RAD*((j+1)*360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(RF_DEG2RAD*((j+1)*360/slices)));

            rf_gl_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(RF_DEG2RAD*((j+1)*360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(RF_DEG2RAD*((j+1)*360/slices)));
            rf_gl_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(RF_DEG2RAD*(j*360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(RF_DEG2RAD*(j*360/slices)));

            rf_gl_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(RF_DEG2RAD*(j*360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(RF_DEG2RAD*(j*360/slices)));

            rf_gl_vertex3f(cosf(RF_DEG2RAD*(270+(180/(rings + 1))*i))*sinf(RF_DEG2RAD*(j*360/slices)),
                           sinf(RF_DEG2RAD*(270+(180/(rings + 1))*i)),
                           cosf(RF_DEG2RAD*(270+(180/(rings + 1))*i))*cosf(RF_DEG2RAD*(j*360/slices)));
        }
    }

    rf_gl_end();
    rf_pop_matrix();
}

// Draw a cylinder
// NOTE: It could be also used for pyramid and cone
RF_API void rf_draw_cylinder(rf_vector3 position, float radiusTop, float radiusBottom, float height, int sides, rf_color color)
{
    if (sides < 3) sides = 3;

    int numVertex = sides*6;
    if (rf_gl_check_buffer_limit(numVertex)) rf_gl_draw();

    rf_push_matrix();
    rf_translatef(position.x, position.y, position.z);

    rf_gl_begin(GL_TRIANGLES);
    rf_gl_color4ub(color.r, color.g, color.b, color.a);

    if (radiusTop > 0)
    {
        // Draw Body -------------------------------------------------------------------------------------
        for (int i = 0; i < 360; i += 360/sides)
        {
            rf_gl_vertex3f(sinf(RF_DEG2RAD*i)*radiusBottom, 0, cosf(RF_DEG2RAD*i)*radiusBottom); //Bottom Left
            rf_gl_vertex3f(sinf(RF_DEG2RAD*(i + 360/sides))*radiusBottom, 0, cosf(RF_DEG2RAD*(i + 360/sides))*radiusBottom); //Bottom Right
            rf_gl_vertex3f(sinf(RF_DEG2RAD*(i + 360/sides))*radiusTop, height, cosf(RF_DEG2RAD*(i + 360/sides))*radiusTop); //Top Right

            rf_gl_vertex3f(sinf(RF_DEG2RAD*i)*radiusTop, height, cosf(RF_DEG2RAD*i)*radiusTop); //Top Left
            rf_gl_vertex3f(sinf(RF_DEG2RAD*i)*radiusBottom, 0, cosf(RF_DEG2RAD*i)*radiusBottom); //Bottom Left
            rf_gl_vertex3f(sinf(RF_DEG2RAD*(i + 360/sides))*radiusTop, height, cosf(RF_DEG2RAD*(i + 360/sides))*radiusTop); //Top Right
        }

        // Draw Cap --------------------------------------------------------------------------------------
        for (int i = 0; i < 360; i += 360/sides)
        {
            rf_gl_vertex3f(0, height, 0);
            rf_gl_vertex3f(sinf(RF_DEG2RAD*i)*radiusTop, height, cosf(RF_DEG2RAD*i)*radiusTop);
            rf_gl_vertex3f(sinf(RF_DEG2RAD*(i + 360/sides))*radiusTop, height, cosf(RF_DEG2RAD*(i + 360/sides))*radiusTop);
        }
    }
    else
    {
        // Draw Cone -------------------------------------------------------------------------------------
        for (int i = 0; i < 360; i += 360/sides)
        {
            rf_gl_vertex3f(0, height, 0);
            rf_gl_vertex3f(sinf(RF_DEG2RAD*i)*radiusBottom, 0, cosf(RF_DEG2RAD*i)*radiusBottom);
            rf_gl_vertex3f(sinf(RF_DEG2RAD*(i + 360/sides))*radiusBottom, 0, cosf(RF_DEG2RAD*(i + 360/sides))*radiusBottom);
        }
    }

    // Draw Base -----------------------------------------------------------------------------------------
    for (int i = 0; i < 360; i += 360/sides)
    {
        rf_gl_vertex3f(0, 0, 0);
        rf_gl_vertex3f(sinf(RF_DEG2RAD*(i + 360/sides))*radiusBottom, 0, cosf(RF_DEG2RAD*(i + 360/sides))*radiusBottom);
        rf_gl_vertex3f(sinf(RF_DEG2RAD*i)*radiusBottom, 0, cosf(RF_DEG2RAD*i)*radiusBottom);
    }

    rf_gl_end();
    rf_pop_matrix();
}

// Draw a wired cylinder
// NOTE: It could be also used for pyramid and cone
RF_API void rf_draw_cylinder_wires(rf_vector3 position, float radiusTop, float radiusBottom, float height, int sides, rf_color color)
{
    if (sides < 3) sides = 3;

    int numVertex = sides*8;
    if (rf_gl_check_buffer_limit(numVertex)) rf_gl_draw();

    rf_push_matrix();
    rf_translatef(position.x, position.y, position.z);

    rf_gl_begin(GL_LINES);
    rf_gl_color4ub(color.r, color.g, color.b, color.a);

    for (int i = 0; i < 360; i += 360/sides)
    {
        rf_gl_vertex3f(sinf(RF_DEG2RAD*i)*radiusBottom, 0, cosf(RF_DEG2RAD*i)*radiusBottom);
        rf_gl_vertex3f(sinf(RF_DEG2RAD*(i + 360/sides))*radiusBottom, 0, cosf(RF_DEG2RAD*(i + 360/sides))*radiusBottom);

        rf_gl_vertex3f(sinf(RF_DEG2RAD*(i + 360/sides))*radiusBottom, 0, cosf(RF_DEG2RAD*(i + 360/sides))*radiusBottom);
        rf_gl_vertex3f(sinf(RF_DEG2RAD*(i + 360/sides))*radiusTop, height, cosf(RF_DEG2RAD*(i + 360/sides))*radiusTop);

        rf_gl_vertex3f(sinf(RF_DEG2RAD*(i + 360/sides))*radiusTop, height, cosf(RF_DEG2RAD*(i + 360/sides))*radiusTop);
        rf_gl_vertex3f(sinf(RF_DEG2RAD*i)*radiusTop, height, cosf(RF_DEG2RAD*i)*radiusTop);

        rf_gl_vertex3f(sinf(RF_DEG2RAD*i)*radiusTop, height, cosf(RF_DEG2RAD*i)*radiusTop);
        rf_gl_vertex3f(sinf(RF_DEG2RAD*i)*radiusBottom, 0, cosf(RF_DEG2RAD*i)*radiusBottom);
    }
    rf_gl_end();
    rf_pop_matrix();
}

// Draw a plane
RF_API void rf_draw_plane(rf_vector3 centerPos, rf_vector2 size, rf_color color)
{
    if (rf_gl_check_buffer_limit(4)) rf_gl_draw();

    // NOTE: Plane is always created on XZ ground
    rf_push_matrix();
    rf_translatef(centerPos.x, centerPos.y, centerPos.z);
    rf_scalef(size.x, 1.0f, size.y);

    rf_gl_begin(GL_QUADS);
    rf_gl_color4ub(color.r, color.g, color.b, color.a);
    rf_gl_normal3f(0.0f, 1.0f, 0.0f);

    rf_gl_vertex3f(-0.5f, 0.0f, -0.5f);
    rf_gl_vertex3f(-0.5f, 0.0f, 0.5f);
    rf_gl_vertex3f(0.5f, 0.0f, 0.5f);
    rf_gl_vertex3f(0.5f, 0.0f, -0.5f);
    rf_gl_end();
    rf_pop_matrix();
}

// Draw a ray line
RF_API void rf_draw_ray(rf_ray ray, rf_color color)
{
    float scale = 10000;

    rf_gl_begin(GL_LINES);
    rf_gl_color4ub(color.r, color.g, color.b, color.a);
    rf_gl_color4ub(color.r, color.g, color.b, color.a);

    rf_gl_vertex3f(ray.position.x, ray.position.y, ray.position.z);
    rf_gl_vertex3f(ray.position.x + ray.direction.x*scale, ray.position.y + ray.direction.y*scale, ray.position.z + ray.direction.z*scale);
    rf_gl_end();
}

// Draw a grid centered at (0, 0, 0)
RF_API void rf_draw_grid(int slices, float spacing)
{
    int halfSlices = slices/2;

    if (rf_gl_check_buffer_limit(slices*4)) rf_gl_draw();

    rf_gl_begin(GL_LINES);
    for (int i = -halfSlices; i <= halfSlices; i++)
    {
        if (i == 0)
        {
            rf_gl_color3f(0.5f, 0.5f, 0.5f);
            rf_gl_color3f(0.5f, 0.5f, 0.5f);
            rf_gl_color3f(0.5f, 0.5f, 0.5f);
            rf_gl_color3f(0.5f, 0.5f, 0.5f);
        }
        else
        {
            rf_gl_color3f(0.75f, 0.75f, 0.75f);
            rf_gl_color3f(0.75f, 0.75f, 0.75f);
            rf_gl_color3f(0.75f, 0.75f, 0.75f);
            rf_gl_color3f(0.75f, 0.75f, 0.75f);
        }

        rf_gl_vertex3f((float)i*spacing, 0.0f, (float)-halfSlices*spacing);
        rf_gl_vertex3f((float)i*spacing, 0.0f, (float)halfSlices*spacing);

        rf_gl_vertex3f((float)-halfSlices*spacing, 0.0f, (float)i*spacing);
        rf_gl_vertex3f((float)halfSlices*spacing, 0.0f, (float)i*spacing);
    }
    rf_gl_end();
}

// Draw gizmo
RF_API void rf_draw_gizmo(rf_vector3 position)
{
    // NOTE: RGB = XYZ
    float length = 1.0f;

    rf_push_matrix();
    rf_translatef(position.x, position.y, position.z);
    rf_scalef(length, length, length);

    rf_gl_begin(GL_LINES);
    rf_gl_color3f(1.0f, 0.0f, 0.0f); rf_gl_vertex3f(0.0f, 0.0f, 0.0f);
    rf_gl_color3f(1.0f, 0.0f, 0.0f); rf_gl_vertex3f(1.0f, 0.0f, 0.0f);

    rf_gl_color3f(0.0f, 1.0f, 0.0f); rf_gl_vertex3f(0.0f, 0.0f, 0.0f);
    rf_gl_color3f(0.0f, 1.0f, 0.0f); rf_gl_vertex3f(0.0f, 1.0f, 0.0f);

    rf_gl_color3f(0.0f, 0.0f, 1.0f); rf_gl_vertex3f(0.0f, 0.0f, 0.0f);
    rf_gl_color3f(0.0f, 0.0f, 1.0f); rf_gl_vertex3f(0.0f, 0.0f, 1.0f);
    rf_gl_end();
    rf_pop_matrix();
}

// Load model from files (mesh and material)
RF_API rf_model rf_load_model(const char* fileName)
{
    rf_model model = { 0 };

    if (_rf_is_file_extension(fileName, ".obj")) model = _rf_load_obj(fileName);
    if (_rf_is_file_extension(fileName, ".iqm")) model = _rf_load_iqm(fileName);
    if (_rf_is_file_extension(fileName, ".gltf") || _rf_is_file_extension(fileName, ".glb")) model = _rf_load_gltf(fileName);

    // Make sure model transform is set to identity matrix!
    model.transform = rf_matrix_identity();

    if (model.mesh_count == 0)
    {
        RF_LOG(RF_LOG_WARNING, "[%s] No meshes can be loaded, default to cube mesh", fileName);

        model.mesh_count = 1;
        model.meshes = (rf_mesh*)RF_MALLOC(model.mesh_count * sizeof(rf_mesh));
        memset(model.meshes, 0, model.mesh_count * sizeof(rf_mesh));
        model.meshes[0] = rf_gen_mesh_cube(1.0f, 1.0f, 1.0f);
    }
    else
    {
        // Upload vertex data to GPU (static mesh)
        for (int i = 0; i < model.mesh_count; i++) rf_gl_load_mesh(&model.meshes[i], false);
    }

    if (model.material_count == 0)
    {
        RF_LOG(RF_LOG_WARNING, "[%s] No materials can be loaded, default to white material", fileName);

        model.material_count = 1;
        model.materials = (rf_material* )RF_MALLOC(model.material_count * sizeof(rf_material));
        memset(model.materials, 0, model.material_count * sizeof(rf_material));
        model.materials[0] = rf_load_material_default();

        if (model.mesh_material == NULL) model.mesh_material = (int*)RF_MALLOC(model.mesh_count * sizeof(int));
    }

    return model;
}

// Load model from generated mesh
// WARNING: A shallow copy of mesh is generated, passed by value,
// as long as struct contains pointers to data and some values, we get a copy
// of mesh pointing to same data as original version... be careful!
RF_API rf_model rf_load_model_from_mesh(rf_mesh mesh)
{
    rf_model model = { 0 };

    model.transform = rf_matrix_identity();

    model.mesh_count = 1;
    model.meshes = (rf_mesh* )RF_MALLOC(model.mesh_count * sizeof(rf_mesh));
    memset(model.meshes, 0, model.mesh_count * sizeof(rf_mesh));
    model.meshes[0] = mesh;

    model.material_count = 1;
    model.materials = (rf_material* )RF_MALLOC(model.material_count * sizeof(rf_material));
    memset(model.materials, 0, model.material_count * sizeof(rf_material));
    model.materials[0] = rf_load_material_default();

    model.mesh_material = (int* )RF_MALLOC(model.mesh_count * sizeof(int));
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
    for (int i = 0; i < model.material_count; i++) RF_FREE(model.materials[i].maps);

    RF_FREE(model.meshes);
    RF_FREE(model.materials);
    RF_FREE(model.mesh_material);

    // Unload animation data
    RF_FREE(model.bones);
    RF_FREE(model.bind_pose);

    RF_LOG(RF_LOG_INFO, "Unloaded model data from RAM and VRAM");
}

// Load meshes from model file
RF_API rf_mesh* rf_load_meshes(const char* fileName, int* mesh_count)
{
    rf_mesh* meshes = NULL;
    int count = 0;

    // TODO: Load meshes from file (OBJ, IQM, GLTF)

    *mesh_count = count;
    return meshes;
}

// Unload mesh from memory (RAM and/or VRAM)
RF_API void rf_unload_mesh(rf_mesh mesh)
{
    rf_gl_unload_mesh(mesh);
    RF_FREE(mesh.vbo_id);
}

// Export mesh data to file
RF_API void rf_export_mesh(rf_mesh mesh, const char* fileName)
{
    bool success = false;

    if (_rf_is_file_extension(fileName, ".obj"))
    {
        FILE* objFile = fopen(fileName, "wt");

        fprintf(objFile, "# //////////////////////////////////////////////////////////////////////////////////\n");
        fprintf(objFile, "# //                                                                              //\n");
        fprintf(objFile, "# // rMeshOBJ exporter v1.0 - rf_mesh exported as triangle faces and not optimized   //\n");
        fprintf(objFile, "# //                                                                              //\n");
        fprintf(objFile, "# // more info and bugs-report:  github.com/raysan5/raylib                        //\n");
        fprintf(objFile, "# // feedback and support:       ray[at]raylib.com                                //\n");
        fprintf(objFile, "# //                                                                              //\n");
        fprintf(objFile, "# // Copyright (c) 2018 Ramon Santamaria (@raysan5)                               //\n");
        fprintf(objFile, "# //                                                                              //\n");
        fprintf(objFile, "# //////////////////////////////////////////////////////////////////////////////////\n\n");
        fprintf(objFile, "# Vertex Count:     %i\n", mesh.vertex_count);
        fprintf(objFile, "# Triangle Count:   %i\n\n", mesh.triangle_count);

        fprintf(objFile, "g mesh\n");

        for (int i = 0, v = 0; i < mesh.vertex_count; i++, v += 3)
        {
            fprintf(objFile, "v %.2f %.2f %.2f\n", mesh.vertices[v], mesh.vertices[v + 1], mesh.vertices[v + 2]);
        }

        for (int i = 0, v = 0; i < mesh.vertex_count; i++, v += 2)
        {
            fprintf(objFile, "vt %.2f %.2f\n", mesh.texcoords[v], mesh.texcoords[v + 1]);
        }

        for (int i = 0, v = 0; i < mesh.vertex_count; i++, v += 3)
        {
            fprintf(objFile, "vn %.2f %.2f %.2f\n", mesh.normals[v], mesh.normals[v + 1], mesh.normals[v + 2]);
        }

        for (int i = 0; i < mesh.triangle_count; i += 3)
        {
            fprintf(objFile, "f %i/%i/%i %i/%i/%i %i/%i/%i\n", i, i, i, i + 1, i + 1, i + 1, i + 2, i + 2, i + 2);
        }

        fprintf(objFile, "\n");

        fclose(objFile);

        success = true;
    }
    else if (_rf_is_file_extension(fileName, ".raw")) { } // TODO: Support additional file formats to export mesh vertex data

    if (success) RF_LOG(RF_LOG_INFO, "rf_mesh exported successfully: %s", fileName);
    else RF_LOG(RF_LOG_WARNING, "rf_mesh could not be exported.");
}

// Load materials from model file
RF_API rf_material* rf_load_materials(const char* fileName, int* material_count)
{
    rf_material* materials = NULL;
    unsigned int count = 0;

    // TODO: Support IQM and GLTF for materials parsing

    if (_rf_is_file_extension(fileName, ".mtl"))
    {
        tinyobj_material_t *mats;

        int result = tinyobj_parse_mtl_file(&mats, (size_t*) &count, fileName);
        if (result != TINYOBJ_SUCCESS) {
            RF_LOG(RF_LOG_WARNING, "[%s] Could not parse Materials file", fileName);
        }

        // TODO: Process materials to return

        tinyobj_materials_free(mats, count);
    }

    // Set materials shader to default (DIFFUSE, SPECULAR, NORMAL)
    for (int i = 0; i < count; i++) materials[i].shader = rf_get_shader_default();

    *material_count = count;
    return materials;
}

// Load default material (Supports: DIFFUSE, SPECULAR, NORMAL maps)
RF_API rf_material rf_load_material_default()
{
    rf_material material = { 0 };
    material.maps = (rf_material_map *)RF_MALLOC(rf_max_material_maps * sizeof(rf_material_map));
    memset(material.maps, 0, rf_max_material_maps * sizeof(rf_material_map));

    material.shader = rf_get_shader_default();
    material.maps[rf_map_diffuse].texture = rf_get_texture_default(); // White texture (1x1 pixel)
    //material.maps[rf_map_normal].texture;         // NOTE: By default, not set
    //material.maps[rf_map_specular].texture;       // NOTE: By default, not set

    material.maps[rf_map_diffuse].color = rf_white; // Diffuse color
    material.maps[rf_map_specular].color = rf_white; // Specular color

    return material;
}

// Unload material from memory
RF_API void rf_unload_material(rf_material material)
{
    // Unload material shader (avoid unloading default shader, managed by raylib)
    if (material.shader.id != rf_get_shader_default().id) rf_unload_shader(material.shader);

    // Unload loaded texture maps (avoid unloading default texture, managed by raylib)
    for (int i = 0; i < rf_max_material_maps; i++)
    {
        if (material.maps[i].texture.id != rf_get_texture_default().id) rf_gl_delete_textures(material.maps[i].texture.id);
    }

    RF_FREE(material.maps);
}

// Set texture for a material map type (rf_map_diffuse, rf_map_specular...)
// NOTE: Previous texture should be manually unloaded
RF_API void rf_set_material_texture(rf_material* material, int mapType, rf_texture2d texture)
{
    material->maps[mapType].texture = texture;
}

// Load model animations from file
RF_API rf_model_animation* rf_load_model_animations(const char* filename, int* animCount)
{
    #define rf_iqm_magic "INTERQUAKEMODEL" // IQM file magic number
    #define rf_iqm_version 2 // only IQM version 2 supported

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

    FILE* iqmFile;
    rf_iqm_header iqm;

    iqmFile = fopen(filename,"rb");

    if (!iqmFile)
    {
        RF_LOG(RF_LOG_ERROR, "[%s] Unable to open file", filename);
    }

    // Read IQM header
    fread(&iqm, sizeof(rf_iqm_header), 1, iqmFile);

    if (strncmp(iqm.magic, rf_iqm_magic, sizeof(rf_iqm_magic)))
    {
        RF_LOG(RF_LOG_ERROR, "Magic Number \"%s\"does not match.", iqm.magic);
        fclose(iqmFile);

        return NULL;
    }

    if (iqm.version != rf_iqm_version)
    {
        RF_LOG(RF_LOG_ERROR, "IQM version %i is incorrect.", iqm.version);
        fclose(iqmFile);

        return NULL;
    }

    // Get bones data
    rf_iqm_pose* poses = (rf_iqm_pose*) RF_MALLOC(iqm.num_poses * sizeof(rf_iqm_pose));
    fseek(iqmFile, iqm.ofs_poses, SEEK_SET);
    fread(poses, iqm.num_poses*sizeof(rf_iqm_pose), 1, iqmFile);

    // Get animations data
    *animCount = iqm.num_anims;
    rf_iqm_anim* anim = (rf_iqm_anim*) RF_MALLOC(iqm.num_anims*sizeof(rf_iqm_anim));
    fseek(iqmFile, iqm.ofs_anims, SEEK_SET);
    fread(anim, iqm.num_anims*sizeof(rf_iqm_anim), 1, iqmFile);
    rf_model_animation* animations = (rf_model_animation*) RF_MALLOC(iqm.num_anims*sizeof(rf_model_animation));

    // frameposes
    unsigned short* framedata = (unsigned short*) RF_MALLOC(iqm.num_frames*iqm.num_framechannels*sizeof(unsigned short));
    fseek(iqmFile, iqm.ofs_frames, SEEK_SET);
    fread(framedata, iqm.num_frames*iqm.num_framechannels*sizeof(unsigned short), 1, iqmFile);

    for (int a = 0; a < iqm.num_anims; a++)
    {
        animations[a].frame_count = anim[a].num_frames;
        animations[a].bone_count = iqm.num_poses;
        animations[a].bones = (rf_bone_info*) RF_MALLOC(iqm.num_poses*sizeof(rf_bone_info));
        animations[a].frame_poses = (rf_transform**) RF_MALLOC(anim[a].num_frames*sizeof(rf_transform*));
        //animations[a].framerate = anim.framerate;     // TODO: Use framerate?

        for (int j = 0; j < iqm.num_poses; j++)
        {
            strcpy(animations[a].bones[j].name, "ANIMJOINTNAME");
            animations[a].bones[j].parent = poses[j].parent;
        }

        for (int j = 0; j < anim[a].num_frames; j++) animations[a].frame_poses[j] = (rf_transform*)RF_MALLOC(iqm.num_poses*sizeof(rf_transform));

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
                    animations[a].frame_poses[frame][i].rotation = rf_quaternion_multiply(animations[a].frame_poses[frame][animations[a].bones[i].parent].rotation, animations[a].frame_poses[frame][i].rotation);
                    animations[a].frame_poses[frame][i].translation = rf_vector3_rotate_by_quaternion(animations[a].frame_poses[frame][i].translation, animations[a].frame_poses[frame][animations[a].bones[i].parent].rotation);
                    animations[a].frame_poses[frame][i].translation = rf_vector3_add(animations[a].frame_poses[frame][i].translation, animations[a].frame_poses[frame][animations[a].bones[i].parent].translation);
                    animations[a].frame_poses[frame][i].scale = rf_vector3_multiply_v(animations[a].frame_poses[frame][i].scale, animations[a].frame_poses[frame][animations[a].bones[i].parent].scale);
                }
            }
        }
    }

    RF_FREE(framedata);
    RF_FREE(poses);
    RF_FREE(anim);

    fclose(iqmFile);

    return animations;
}

// Set the material for a mesh
RF_API void rf_set_model_mesh_material(rf_model* model, int meshId, int materialId)
{
    if (meshId >= model->mesh_count) RF_LOG(RF_LOG_WARNING, "rf_mesh id greater than mesh count");
    else if (materialId >= model->material_count) RF_LOG(RF_LOG_WARNING,"rf_material id greater than material count");
    else model->mesh_material[meshId] = materialId;
}

// Update model animated vertex data (positions and normals) for a given frame
// NOTE: Updated data is uploaded to GPU
RF_API void rf_update_model_animation(rf_model model, rf_model_animation anim, int frame)
{
    if ((anim.frame_count > 0) && (anim.bones != NULL) && (anim.frame_poses != NULL))
    {
        if (frame >= anim.frame_count) frame = frame%anim.frame_count;

        for (int m = 0; m < model.mesh_count; m++)
        {
            rf_vector3 animVertex = { 0 };
            rf_vector3 animNormal = { 0 };

            rf_vector3 inTranslation = { 0 };
            rf_quaternion inRotation = { 0 };
            rf_vector3 inScale = { 0 };

            rf_vector3 outTranslation = { 0 };
            rf_quaternion outRotation = { 0 };
            rf_vector3 outScale = { 0 };

            int vertex_pos_counter = 0;
            int boneCounter = 0;
            int boneId = 0;

            for (int i = 0; i < model.meshes[m].vertex_count; i++)
            {
                boneId = model.meshes[m].bone_ids[boneCounter];
                inTranslation = model.bind_pose[boneId].translation;
                inRotation = model.bind_pose[boneId].rotation;
                inScale = model.bind_pose[boneId].scale;
                outTranslation = anim.frame_poses[frame][boneId].translation;
                outRotation = anim.frame_poses[frame][boneId].rotation;
                outScale = anim.frame_poses[frame][boneId].scale;

                // Vertices processing
                // NOTE: We use meshes.vertices (default vertex position) to calculate meshes.anim_vertices (animated vertex position)
                animVertex = (rf_vector3){ model.meshes[m].vertices[vertex_pos_counter], model.meshes[m].vertices[vertex_pos_counter + 1], model.meshes[m].vertices[vertex_pos_counter + 2] };
                animVertex = rf_vector3_multiply_v(animVertex, outScale);
                animVertex = rf_vector3_substract(animVertex, inTranslation);
                animVertex = rf_vector3_rotate_by_quaternion(animVertex, rf_quaternion_multiply(outRotation, rf_quaternion_invert(inRotation)));
                animVertex = rf_vector3_add(animVertex, outTranslation);
                model.meshes[m].anim_vertices[vertex_pos_counter] = animVertex.x;
                model.meshes[m].anim_vertices[vertex_pos_counter + 1] = animVertex.y;
                model.meshes[m].anim_vertices[vertex_pos_counter + 2] = animVertex.z;

                // Normals processing
                // NOTE: We use meshes.baseNormals (default normal) to calculate meshes.normals (animated normals)
                animNormal = (rf_vector3){ model.meshes[m].normals[vertex_pos_counter], model.meshes[m].normals[vertex_pos_counter + 1], model.meshes[m].normals[vertex_pos_counter + 2] };
                animNormal = rf_vector3_rotate_by_quaternion(animNormal, rf_quaternion_multiply(outRotation, rf_quaternion_invert(inRotation)));
                model.meshes[m].anim_normals[vertex_pos_counter] = animNormal.x;
                model.meshes[m].anim_normals[vertex_pos_counter + 1] = animNormal.y;
                model.meshes[m].anim_normals[vertex_pos_counter + 2] = animNormal.z;
                vertex_pos_counter += 3;

                boneCounter += 4;
            }

            // Upload new vertex data to GPU for model drawing
            rf_gl_update_buffer(model.meshes[m].vbo_id[0], model.meshes[m].anim_vertices, model.meshes[m].vertex_count*3*sizeof(float)); // Update vertex position
            rf_gl_update_buffer(model.meshes[m].vbo_id[2], model.meshes[m].anim_vertices, model.meshes[m].vertex_count*3*sizeof(float)); // Update vertex normals
        }
    }
}

// Unload animation data
RF_API void rf_unload_model_animation(rf_model_animation anim)
{
    for (int i = 0; i < anim.frame_count; i++) RF_FREE(anim.frame_poses[i]);

    RF_FREE(anim.bones);
    RF_FREE(anim.frame_poses);
}

// Check model animation skeleton match
// NOTE: Only number of bones and parent connections are checked
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

// Generate polygonal mesh
RF_API rf_mesh rf_gen_mesh_poly(int sides, float radius)
{
    rf_mesh mesh = { 0 };
    mesh.vbo_id = (unsigned int* )RF_MALLOC(rf_max_mesh_vbo * sizeof(unsigned int));
    memset(mesh.vbo_id, 0, rf_max_mesh_vbo * sizeof(unsigned int));
    int vertex_count = sides*3;

    // Vertices definition
    rf_vector3* vertices = (rf_vector3* )RF_MALLOC(vertex_count*sizeof(rf_vector3));
    for (int i = 0, v = 0; i < 360; i += 360/sides, v += 3)
    {
        vertices[v] = (rf_vector3){ 0.0f, 0.0f, 0.0f };
        vertices[v + 1] = (rf_vector3){ sinf(RF_DEG2RAD*i)*radius, 0.0f, cosf(RF_DEG2RAD*i)*radius };
        vertices[v + 2] = (rf_vector3){ sinf(RF_DEG2RAD*(i + 360/sides))*radius, 0.0f, cosf(RF_DEG2RAD*(i + 360/sides))*radius };
    }

    // Normals definition
    rf_vector3* normals = (rf_vector3* )RF_MALLOC(vertex_count*sizeof(rf_vector3));
    for (int n = 0; n < vertex_count; n++) normals[n] = (rf_vector3){ 0.0f, 1.0f, 0.0f }; // rf_vector3.up;

    // TexCoords definition
    rf_vector2 *texcoords = (rf_vector2 *)RF_MALLOC(vertex_count*sizeof(rf_vector2));
    for (int n = 0; n < vertex_count; n++) texcoords[n] = RF_CLITERAL(rf_vector2){ 0.0f, 0.0f };

    mesh.vertex_count = vertex_count;
    mesh.triangle_count = sides;
    mesh.vertices = (float* )RF_MALLOC(mesh.vertex_count*3*sizeof(float));
    mesh.texcoords = (float* )RF_MALLOC(mesh.vertex_count*2*sizeof(float));
    mesh.normals = (float* )RF_MALLOC(mesh.vertex_count*3*sizeof(float));

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

    RF_FREE(vertices);
    RF_FREE(normals);
    RF_FREE(texcoords);

    // Upload vertex data to GPU (static mesh)
    rf_gl_load_mesh(&mesh, false);

    return mesh;
}

// Generate plane mesh (with subdivisions)
RF_API rf_mesh rf_gen_mesh_plane(float width, float length, int resX, int resZ)
{
    rf_mesh mesh = { 0 };
    mesh.vbo_id = (unsigned int* )RF_MALLOC(rf_max_mesh_vbo * sizeof(unsigned int));
    memset(mesh.vbo_id, 0, rf_max_mesh_vbo * sizeof(unsigned int));

#define rf_custom_mesh_gen_plane
    par_shapes_mesh *plane = par_shapes_create_plane(resX, resZ); // No normals/texcoords generated!!!
    par_shapes_scale(plane, width, length, 1.0f);
    float axis[] = { 1, 0, 0 };
    par_shapes_rotate(plane, -RF_PI / 2.0f, axis);
    par_shapes_translate(plane, -width/2, 0.0f, length/2);

    mesh.vertices = (float* )RF_MALLOC(plane->ntriangles*3*3*sizeof(float));
    mesh.texcoords = (float* )RF_MALLOC(plane->ntriangles*3*2*sizeof(float));
    mesh.normals = (float* )RF_MALLOC(plane->ntriangles*3*3*sizeof(float));
    mesh.vbo_id = (unsigned int* )RF_MALLOC(rf_max_mesh_vbo * sizeof(unsigned int));
    memset(mesh.vbo_id, 0, rf_max_mesh_vbo * sizeof(unsigned int));

    mesh.vertex_count = plane->ntriangles*3;
    mesh.triangle_count = plane->ntriangles;

    for (int k = 0; k < mesh.vertex_count; k++)
    {
        mesh.vertices[k*3] = plane->points[plane->triangles[k]*3];
        mesh.vertices[k*3 + 1] = plane->points[plane->triangles[k]*3 + 1];
        mesh.vertices[k*3 + 2] = plane->points[plane->triangles[k]*3 + 2];

        mesh.normals[k*3] = plane->normals[plane->triangles[k]*3];
        mesh.normals[k*3 + 1] = plane->normals[plane->triangles[k]*3 + 1];
        mesh.normals[k*3 + 2] = plane->normals[plane->triangles[k]*3 + 2];

        mesh.texcoords[k*2] = plane->tcoords[plane->triangles[k]*2];
        mesh.texcoords[k*2 + 1] = plane->tcoords[plane->triangles[k]*2 + 1];
    }

    par_shapes_free_mesh(plane);


    // Upload vertex data to GPU (static mesh)
    rf_gl_load_mesh(&mesh, false);

    return mesh;
}

// Generated cuboid mesh
RF_API rf_mesh rf_gen_mesh_cube(float width, float height, float length)
{
    rf_mesh mesh = { 0 };
    mesh.vbo_id = (unsigned int* )RF_MALLOC(rf_max_mesh_vbo * sizeof(unsigned int));
    memset(mesh.vbo_id, 0, rf_max_mesh_vbo * sizeof(unsigned int));

#define rf_custom_mesh_gen_cube
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
    par_shapes_mesh *cube = par_shapes_create_cube();
    cube->tcoords = PAR_MALLOC(float, 2*cube->npoints);
    for (int i = 0; i < 2*cube->npoints; i++) cube->tcoords[i] = 0.0f;
    par_shapes_scale(cube, width, height, length);
    par_shapes_translate(cube, -width/2, 0.0f, -length/2);
    par_shapes_compute_normals(cube);

    mesh.vertices = (float* )RF_MALLOC(cube->ntriangles*3*3*sizeof(float));
    mesh.texcoords = (float* )RF_MALLOC(cube->ntriangles*3*2*sizeof(float));
    mesh.normals = (float* )RF_MALLOC(cube->ntriangles*3*3*sizeof(float));

    mesh.vertex_count = cube->ntriangles*3;
    mesh.triangle_count = cube->ntriangles;

    for (int k = 0; k < mesh.vertex_count; k++)
    {
        mesh.vertices[k*3] = cube->points[cube->triangles[k]*3];
        mesh.vertices[k*3 + 1] = cube->points[cube->triangles[k]*3 + 1];
        mesh.vertices[k*3 + 2] = cube->points[cube->triangles[k]*3 + 2];

        mesh.normals[k*3] = cube->normals[cube->triangles[k]*3];
        mesh.normals[k*3 + 1] = cube->normals[cube->triangles[k]*3 + 1];
        mesh.normals[k*3 + 2] = cube->normals[cube->triangles[k]*3 + 2];

        mesh.texcoords[k*2] = cube->tcoords[cube->triangles[k]*2];
        mesh.texcoords[k*2 + 1] = cube->tcoords[cube->triangles[k]*2 + 1];
    }

    par_shapes_free_mesh(cube);


    // Upload vertex data to GPU (static mesh)
    rf_gl_load_mesh(&mesh, false);

    return mesh;
}

// Generate sphere mesh (standard sphere)
RF_API rf_mesh rf_gen_mesh_sphere(float radius, int rings, int slices)
{
    rf_mesh mesh = { 0 };
    mesh.vbo_id = (unsigned int*)RF_MALLOC(rf_max_mesh_vbo * sizeof(unsigned int));
    memset(mesh.vbo_id, 0, rf_max_mesh_vbo * sizeof(unsigned int));

    par_shapes_mesh *sphere = par_shapes_create_parametric_sphere(slices, rings);
    par_shapes_scale(sphere, radius, radius, radius);
    // NOTE: Soft normals are computed internally

    mesh.vertices = (float* )RF_MALLOC(sphere->ntriangles*3*3*sizeof(float));
    mesh.texcoords = (float* )RF_MALLOC(sphere->ntriangles*3*2*sizeof(float));
    mesh.normals = (float* )RF_MALLOC(sphere->ntriangles*3*3*sizeof(float));

    mesh.vertex_count = sphere->ntriangles*3;
    mesh.triangle_count = sphere->ntriangles;

    for (int k = 0; k < mesh.vertex_count; k++)
    {
        mesh.vertices[k*3] = sphere->points[sphere->triangles[k]*3];
        mesh.vertices[k*3 + 1] = sphere->points[sphere->triangles[k]*3 + 1];
        mesh.vertices[k*3 + 2] = sphere->points[sphere->triangles[k]*3 + 2];

        mesh.normals[k*3] = sphere->normals[sphere->triangles[k]*3];
        mesh.normals[k*3 + 1] = sphere->normals[sphere->triangles[k]*3 + 1];
        mesh.normals[k*3 + 2] = sphere->normals[sphere->triangles[k]*3 + 2];

        mesh.texcoords[k*2] = sphere->tcoords[sphere->triangles[k]*2];
        mesh.texcoords[k*2 + 1] = sphere->tcoords[sphere->triangles[k]*2 + 1];
    }

    par_shapes_free_mesh(sphere);

    // Upload vertex data to GPU (static mesh)
    rf_gl_load_mesh(&mesh, false);

    return mesh;
}

// Generate hemi-sphere mesh (half sphere, no bottom cap)
RF_API rf_mesh rf_gen_mesh_hemi_sphere(float radius, int rings, int slices)
{
    rf_mesh mesh = { 0 };
    mesh.vbo_id = (unsigned int*)RF_MALLOC(rf_max_mesh_vbo * sizeof(unsigned int));
    memset(mesh.vbo_id, 0, rf_max_mesh_vbo * sizeof(unsigned int));

    par_shapes_mesh *sphere = par_shapes_create_hemisphere(slices, rings);
    par_shapes_scale(sphere, radius, radius, radius);
    // NOTE: Soft normals are computed internally

    mesh.vertices = (float* )RF_MALLOC(sphere->ntriangles*3*3*sizeof(float));
    mesh.texcoords = (float* )RF_MALLOC(sphere->ntriangles*3*2*sizeof(float));
    mesh.normals = (float* )RF_MALLOC(sphere->ntriangles*3*3*sizeof(float));

    mesh.vertex_count = sphere->ntriangles*3;
    mesh.triangle_count = sphere->ntriangles;

    for (int k = 0; k < mesh.vertex_count; k++)
    {
        mesh.vertices[k*3] = sphere->points[sphere->triangles[k]*3];
        mesh.vertices[k*3 + 1] = sphere->points[sphere->triangles[k]*3 + 1];
        mesh.vertices[k*3 + 2] = sphere->points[sphere->triangles[k]*3 + 2];

        mesh.normals[k*3] = sphere->normals[sphere->triangles[k]*3];
        mesh.normals[k*3 + 1] = sphere->normals[sphere->triangles[k]*3 + 1];
        mesh.normals[k*3 + 2] = sphere->normals[sphere->triangles[k]*3 + 2];

        mesh.texcoords[k*2] = sphere->tcoords[sphere->triangles[k]*2];
        mesh.texcoords[k*2 + 1] = sphere->tcoords[sphere->triangles[k]*2 + 1];
    }

    par_shapes_free_mesh(sphere);

    // Upload vertex data to GPU (static mesh)
    rf_gl_load_mesh(&mesh, false);

    return mesh;
}

// Generate cylinder mesh
RF_API rf_mesh rf_gen_mesh_cylinder(float radius, float height, int slices)
{
    rf_mesh mesh = { 0 };
    mesh.vbo_id = (unsigned int*)RF_MALLOC(rf_max_mesh_vbo * sizeof(unsigned int));
    memset(mesh.vbo_id, 0, rf_max_mesh_vbo * sizeof(unsigned int));

    // Instance a cylinder that sits on the Z=0 plane using the given tessellation
    // levels across the UV domain.  Think of "slices" like a number of pizza
    // slices, and "stacks" like a number of stacked rings.
    // Height and radius are both 1.0, but they can easily be changed with par_shapes_scale
    par_shapes_mesh *cylinder = par_shapes_create_cylinder(slices, 8);
    par_shapes_scale(cylinder, radius, radius, height);
    float axis[] = { 1, 0, 0 };
    par_shapes_rotate(cylinder, -RF_PI / 2.0f, axis);

    // Generate an orientable disk shape (top cap)
    float center[] = { 0, 0, 0 };
    float normal[] = { 0, 0, 1 };
    float normal_minus_1[] = { 0, 0, -1 };
    par_shapes_mesh *capTop = par_shapes_create_disk(radius, slices, center, normal);
    capTop->tcoords = PAR_MALLOC(float, 2*capTop->npoints);
    for (int i = 0; i < 2*capTop->npoints; i++) capTop->tcoords[i] = 0.0f;

    par_shapes_rotate(capTop, -RF_PI / 2.0f, axis);
    par_shapes_translate(capTop, 0, height, 0);

    // Generate an orientable disk shape (bottom cap)
    par_shapes_mesh *capBottom = par_shapes_create_disk(radius, slices, center, normal_minus_1);
    capBottom->tcoords = PAR_MALLOC(float, 2*capBottom->npoints);
    for (int i = 0; i < 2*capBottom->npoints; i++) capBottom->tcoords[i] = 0.95f;
    par_shapes_rotate(capBottom, RF_PI / 2.0f, axis);

    par_shapes_merge_and_free(cylinder, capTop);
    par_shapes_merge_and_free(cylinder, capBottom);

    mesh.vertices = (float* )RF_MALLOC(cylinder->ntriangles*3*3*sizeof(float));
    mesh.texcoords = (float* )RF_MALLOC(cylinder->ntriangles*3*2*sizeof(float));
    mesh.normals = (float* )RF_MALLOC(cylinder->ntriangles*3*3*sizeof(float));

    mesh.vertex_count = cylinder->ntriangles*3;
    mesh.triangle_count = cylinder->ntriangles;

    for (int k = 0; k < mesh.vertex_count; k++)
    {
        mesh.vertices[k*3] = cylinder->points[cylinder->triangles[k]*3];
        mesh.vertices[k*3 + 1] = cylinder->points[cylinder->triangles[k]*3 + 1];
        mesh.vertices[k*3 + 2] = cylinder->points[cylinder->triangles[k]*3 + 2];

        mesh.normals[k*3] = cylinder->normals[cylinder->triangles[k]*3];
        mesh.normals[k*3 + 1] = cylinder->normals[cylinder->triangles[k]*3 + 1];
        mesh.normals[k*3 + 2] = cylinder->normals[cylinder->triangles[k]*3 + 2];

        mesh.texcoords[k*2] = cylinder->tcoords[cylinder->triangles[k]*2];
        mesh.texcoords[k*2 + 1] = cylinder->tcoords[cylinder->triangles[k]*2 + 1];
    }

    par_shapes_free_mesh(cylinder);

    // Upload vertex data to GPU (static mesh)
    rf_gl_load_mesh(&mesh, false);

    return mesh;
}

// Generate torus mesh
RF_API rf_mesh rf_gen_mesh_torus(float radius, float size, int radSeg, int sides)
{
    rf_mesh mesh = { 0 };
    mesh.vbo_id = (unsigned int* )RF_MALLOC(rf_max_mesh_vbo * sizeof(unsigned int));
    memset(mesh.vbo_id, 0, rf_max_mesh_vbo * sizeof(unsigned int));

    if (radius > 1.0f) radius = 1.0f;
    else if (radius < 0.1f) radius = 0.1f;

    // Create a donut that sits on the Z=0 plane with the specified inner radius
    // The outer radius can be controlled with par_shapes_scale
    par_shapes_mesh *torus = par_shapes_create_torus(radSeg, sides, radius);
    par_shapes_scale(torus, size/2, size/2, size/2);

    mesh.vertices = (float* )RF_MALLOC(torus->ntriangles*3*3*sizeof(float));
    mesh.texcoords = (float* )RF_MALLOC(torus->ntriangles*3*2*sizeof(float));
    mesh.normals = (float* )RF_MALLOC(torus->ntriangles*3*3*sizeof(float));

    mesh.vertex_count = torus->ntriangles*3;
    mesh.triangle_count = torus->ntriangles;

    for (int k = 0; k < mesh.vertex_count; k++)
    {
        mesh.vertices[k*3] = torus->points[torus->triangles[k]*3];
        mesh.vertices[k*3 + 1] = torus->points[torus->triangles[k]*3 + 1];
        mesh.vertices[k*3 + 2] = torus->points[torus->triangles[k]*3 + 2];

        mesh.normals[k*3] = torus->normals[torus->triangles[k]*3];
        mesh.normals[k*3 + 1] = torus->normals[torus->triangles[k]*3 + 1];
        mesh.normals[k*3 + 2] = torus->normals[torus->triangles[k]*3 + 2];

        mesh.texcoords[k*2] = torus->tcoords[torus->triangles[k]*2];
        mesh.texcoords[k*2 + 1] = torus->tcoords[torus->triangles[k]*2 + 1];
    }

    par_shapes_free_mesh(torus);

    // Upload vertex data to GPU (static mesh)
    rf_gl_load_mesh(&mesh, false);

    return mesh;
}

// Generate trefoil knot mesh
RF_API rf_mesh rf_gen_mesh_knot(float radius, float size, int radSeg, int sides)
{
    rf_mesh mesh = { 0 };
    mesh.vbo_id = (unsigned int* )RF_MALLOC(rf_max_mesh_vbo * sizeof(unsigned int));
    memset(mesh.vbo_id, 0, rf_max_mesh_vbo * sizeof(unsigned int));

    if (radius > 3.0f) radius = 3.0f;
    else if (radius < 0.5f) radius = 0.5f;

    par_shapes_mesh *knot = par_shapes_create_trefoil_knot(radSeg, sides, radius);
    par_shapes_scale(knot, size, size, size);

    mesh.vertices = (float* )RF_MALLOC(knot->ntriangles*3*3*sizeof(float));
    mesh.texcoords = (float* )RF_MALLOC(knot->ntriangles*3*2*sizeof(float));
    mesh.normals = (float* )RF_MALLOC(knot->ntriangles*3*3*sizeof(float));

    mesh.vertex_count = knot->ntriangles*3;
    mesh.triangle_count = knot->ntriangles;

    for (int k = 0; k < mesh.vertex_count; k++)
    {
        mesh.vertices[k*3] = knot->points[knot->triangles[k]*3];
        mesh.vertices[k*3 + 1] = knot->points[knot->triangles[k]*3 + 1];
        mesh.vertices[k*3 + 2] = knot->points[knot->triangles[k]*3 + 2];

        mesh.normals[k*3] = knot->normals[knot->triangles[k]*3];
        mesh.normals[k*3 + 1] = knot->normals[knot->triangles[k]*3 + 1];
        mesh.normals[k*3 + 2] = knot->normals[knot->triangles[k]*3 + 2];

        mesh.texcoords[k*2] = knot->tcoords[knot->triangles[k]*2];
        mesh.texcoords[k*2 + 1] = knot->tcoords[knot->triangles[k]*2 + 1];
    }

    par_shapes_free_mesh(knot);

    // Upload vertex data to GPU (static mesh)
    rf_gl_load_mesh(&mesh, false);

    return mesh;
}

// Generate a mesh from heightmap
// NOTE: Vertex data is uploaded to GPU
RF_API rf_mesh rf_gen_mesh_heightmap(rf_image heightmap, rf_vector3 size)
{
#define rf_gray_value(c) ((c.r+c.g+c.b)/3)

    rf_mesh mesh = { 0 };
    mesh.vbo_id = (unsigned int* )RF_MALLOC(rf_max_mesh_vbo * sizeof(unsigned int));
    memset(mesh.vbo_id, 0, rf_max_mesh_vbo * sizeof(unsigned int));

    int mapX = heightmap.width;
    int mapZ = heightmap.height;

    rf_color* pixels = rf_get_image_data(heightmap);

    // NOTE: One vertex per pixel
    mesh.triangle_count = (mapX-1)*(mapZ-1)*2; // One quad every four pixels

    mesh.vertex_count = mesh.triangle_count*3;

    mesh.vertices = (float* )RF_MALLOC(mesh.vertex_count*3*sizeof(float));
    mesh.normals = (float* )RF_MALLOC(mesh.vertex_count*3*sizeof(float));
    mesh.texcoords = (float* )RF_MALLOC(mesh.vertex_count*2*sizeof(float));
    mesh.colors = NULL;

    int vertex_pos_counter = 0; // Used to count vertices float by float
    int vertex_texcoord_counter = 0; // Used to count texcoords float by float
    int nCounter = 0; // Used to count normals float by float

    int trisCounter = 0;

    rf_vector3 scaleFactor = { size.x/mapX, size.y/255.0f, size.z/mapZ };

    for (int z = 0; z < mapZ-1; z++)
    {
        for (int x = 0; x < mapX-1; x++)
        {
            // Fill vertices array with data
            //----------------------------------------------------------

            // one triangle - 3 vertex
            mesh.vertices[vertex_pos_counter] = (float)x*scaleFactor.x;
            mesh.vertices[vertex_pos_counter + 1] = (float)rf_gray_value(pixels[x + z*mapX])*scaleFactor.y;
            mesh.vertices[vertex_pos_counter + 2] = (float)z*scaleFactor.z;

            mesh.vertices[vertex_pos_counter + 3] = (float)x*scaleFactor.x;
            mesh.vertices[vertex_pos_counter + 4] = (float)rf_gray_value(pixels[x + (z + 1)*mapX])*scaleFactor.y;
            mesh.vertices[vertex_pos_counter + 5] = (float)(z + 1)*scaleFactor.z;

            mesh.vertices[vertex_pos_counter + 6] = (float)(x + 1)*scaleFactor.x;
            mesh.vertices[vertex_pos_counter + 7] = (float)rf_gray_value(pixels[(x + 1) + z*mapX])*scaleFactor.y;
            mesh.vertices[vertex_pos_counter + 8] = (float)z*scaleFactor.z;

            // another triangle - 3 vertex
            mesh.vertices[vertex_pos_counter + 9] = mesh.vertices[vertex_pos_counter + 6];
            mesh.vertices[vertex_pos_counter + 10] = mesh.vertices[vertex_pos_counter + 7];
            mesh.vertices[vertex_pos_counter + 11] = mesh.vertices[vertex_pos_counter + 8];

            mesh.vertices[vertex_pos_counter + 12] = mesh.vertices[vertex_pos_counter + 3];
            mesh.vertices[vertex_pos_counter + 13] = mesh.vertices[vertex_pos_counter + 4];
            mesh.vertices[vertex_pos_counter + 14] = mesh.vertices[vertex_pos_counter + 5];

            mesh.vertices[vertex_pos_counter + 15] = (float)(x + 1)*scaleFactor.x;
            mesh.vertices[vertex_pos_counter + 16] = (float)rf_gray_value(pixels[(x + 1) + (z + 1)*mapX])*scaleFactor.y;
            mesh.vertices[vertex_pos_counter + 17] = (float)(z + 1)*scaleFactor.z;
            vertex_pos_counter += 18; // 6 vertex, 18 floats

            // Fill texcoords array with data
            //--------------------------------------------------------------
            mesh.texcoords[vertex_texcoord_counter] = (float)x/(mapX - 1);
            mesh.texcoords[vertex_texcoord_counter + 1] = (float)z/(mapZ - 1);

            mesh.texcoords[vertex_texcoord_counter + 2] = (float)x/(mapX - 1);
            mesh.texcoords[vertex_texcoord_counter + 3] = (float)(z + 1)/(mapZ - 1);

            mesh.texcoords[vertex_texcoord_counter + 4] = (float)(x + 1)/(mapX - 1);
            mesh.texcoords[vertex_texcoord_counter + 5] = (float)z/(mapZ - 1);

            mesh.texcoords[vertex_texcoord_counter + 6] = mesh.texcoords[vertex_texcoord_counter + 4];
            mesh.texcoords[vertex_texcoord_counter + 7] = mesh.texcoords[vertex_texcoord_counter + 5];

            mesh.texcoords[vertex_texcoord_counter + 8] = mesh.texcoords[vertex_texcoord_counter + 2];
            mesh.texcoords[vertex_texcoord_counter + 9] = mesh.texcoords[vertex_texcoord_counter + 3];

            mesh.texcoords[vertex_texcoord_counter + 10] = (float)(x + 1)/(mapX - 1);
            mesh.texcoords[vertex_texcoord_counter + 11] = (float)(z + 1)/(mapZ - 1);
            vertex_texcoord_counter += 12; // 6 texcoords, 12 floats

            // Fill normals array with data
            //--------------------------------------------------------------
            for (int i = 0; i < 18; i += 3)
            {
                mesh.normals[nCounter + i] = 0.0f;
                mesh.normals[nCounter + i + 1] = 1.0f;
                mesh.normals[nCounter + i + 2] = 0.0f;
            }

            // TODO: Calculate normals in an efficient way

            nCounter += 18; // 6 vertex, 18 floats
            trisCounter += 2;
        }
    }

    RF_FREE(pixels);

    // Upload vertex data to GPU (static mesh)
    rf_gl_load_mesh(&mesh, false);

    return mesh;
}

// Generate a cubes mesh from pixel data
// NOTE: Vertex data is uploaded to GPU
RF_API rf_mesh rf_gen_mesh_cubicmap(rf_image cubicmap, rf_vector3 cubeSize)
{
    rf_mesh mesh = { 0 };
    mesh.vbo_id = (unsigned int*)RF_MALLOC(rf_max_mesh_vbo * sizeof(unsigned int));
    memset(mesh.vbo_id, 0, rf_max_mesh_vbo * sizeof(unsigned int));

    rf_color* cubicmapPixels = rf_get_image_data(cubicmap);

    int mapWidth = cubicmap.width;
    int mapHeight = cubicmap.height;

    // NOTE: Max possible number of triangles numCubes*(12 triangles by cube)
    int maxTriangles = cubicmap.width*cubicmap.height*12;

    int vertex_pos_counter = 0; // Used to count vertices
    int vertex_texcoord_counter = 0; // Used to count texcoords
    int nCounter = 0; // Used to count normals

    float w = cubeSize.x;
    float h = cubeSize.z;
    float h2 = cubeSize.y;

    rf_vector3* mapVertices = (rf_vector3* )RF_MALLOC(maxTriangles*3*sizeof(rf_vector3));
    rf_vector2 *mapTexcoords = (rf_vector2 *)RF_MALLOC(maxTriangles*3*sizeof(rf_vector2));
    rf_vector3* mapNormals = (rf_vector3* )RF_MALLOC(maxTriangles*3*sizeof(rf_vector3));

    // Define the 6 normals of the cube, we will combine them accordingly later...
    rf_vector3 n1 = { 1.0f, 0.0f, 0.0f };
    rf_vector3 n2 = { -1.0f, 0.0f, 0.0f };
    rf_vector3 n3 = { 0.0f, 1.0f, 0.0f };
    rf_vector3 n4 = { 0.0f, -1.0f, 0.0f };
    rf_vector3 n5 = { 0.0f, 0.0f, 1.0f };
    rf_vector3 n6 = { 0.0f, 0.0f, -1.0f };

    // NOTE: We use texture rectangles to define different textures for top-bottom-front-back-right-left (6)
    typedef struct rf_rectanglef rf_rectanglef;
    struct rf_rectanglef
    {
        float x;
        float y;
        float width;
        float height;
    };

    rf_rectanglef rightTexUV = { 0.0f, 0.0f, 0.5f, 0.5f };
    rf_rectanglef leftTexUV = { 0.5f, 0.0f, 0.5f, 0.5f };
    rf_rectanglef frontTexUV = { 0.0f, 0.0f, 0.5f, 0.5f };
    rf_rectanglef backTexUV = { 0.5f, 0.0f, 0.5f, 0.5f };
    rf_rectanglef topTexUV = { 0.0f, 0.5f, 0.5f, 0.5f };
    rf_rectanglef bottomTexUV = { 0.5f, 0.5f, 0.5f, 0.5f };

    for (int z = 0; z < mapHeight; ++z)
    {
        for (int x = 0; x < mapWidth; ++x)
        {
            // Define the 8 vertex of the cube, we will combine them accordingly later...
            rf_vector3 v1 = { w*(x - 0.5f), h2, h*(z - 0.5f) };
            rf_vector3 v2 = { w*(x - 0.5f), h2, h*(z + 0.5f) };
            rf_vector3 v3 = { w*(x + 0.5f), h2, h*(z + 0.5f) };
            rf_vector3 v4 = { w*(x + 0.5f), h2, h*(z - 0.5f) };
            rf_vector3 v5 = { w*(x + 0.5f), 0, h*(z - 0.5f) };
            rf_vector3 v6 = { w*(x - 0.5f), 0, h*(z - 0.5f) };
            rf_vector3 v7 = { w*(x - 0.5f), 0, h*(z + 0.5f) };
            rf_vector3 v8 = { w*(x + 0.5f), 0, h*(z + 0.5f) };

            // We check pixel color to be rf_white, we will full cubes
            if ((cubicmapPixels[z*cubicmap.width + x].r == 255) &&
                (cubicmapPixels[z*cubicmap.width + x].g == 255) &&
                (cubicmapPixels[z*cubicmap.width + x].b == 255))
            {
                // Define triangles (Checking Collateral Cubes!)
                //----------------------------------------------

                // Define top triangles (2 tris, 6 vertex --> v1-v2-v3, v1-v3-v4)
                mapVertices[vertex_pos_counter] = v1;
                mapVertices[vertex_pos_counter + 1] = v2;
                mapVertices[vertex_pos_counter + 2] = v3;
                mapVertices[vertex_pos_counter + 3] = v1;
                mapVertices[vertex_pos_counter + 4] = v3;
                mapVertices[vertex_pos_counter + 5] = v4;
                vertex_pos_counter += 6;

                mapNormals[nCounter] = n3;
                mapNormals[nCounter + 1] = n3;
                mapNormals[nCounter + 2] = n3;
                mapNormals[nCounter + 3] = n3;
                mapNormals[nCounter + 4] = n3;
                mapNormals[nCounter + 5] = n3;
                nCounter += 6;

                mapTexcoords[vertex_texcoord_counter] = RF_CLITERAL(rf_vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[vertex_texcoord_counter + 1] = RF_CLITERAL(rf_vector2){ topTexUV.x, topTexUV.y + topTexUV.height };
                mapTexcoords[vertex_texcoord_counter + 2] = RF_CLITERAL(rf_vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                mapTexcoords[vertex_texcoord_counter + 3] = RF_CLITERAL(rf_vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[vertex_texcoord_counter + 4] = RF_CLITERAL(rf_vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                mapTexcoords[vertex_texcoord_counter + 5] = RF_CLITERAL(rf_vector2){ topTexUV.x + topTexUV.width, topTexUV.y };
                vertex_texcoord_counter += 6;

                // Define bottom triangles (2 tris, 6 vertex --> v6-v8-v7, v6-v5-v8)
                mapVertices[vertex_pos_counter] = v6;
                mapVertices[vertex_pos_counter + 1] = v8;
                mapVertices[vertex_pos_counter + 2] = v7;
                mapVertices[vertex_pos_counter + 3] = v6;
                mapVertices[vertex_pos_counter + 4] = v5;
                mapVertices[vertex_pos_counter + 5] = v8;
                vertex_pos_counter += 6;

                mapNormals[nCounter] = n4;
                mapNormals[nCounter + 1] = n4;
                mapNormals[nCounter + 2] = n4;
                mapNormals[nCounter + 3] = n4;
                mapNormals[nCounter + 4] = n4;
                mapNormals[nCounter + 5] = n4;
                nCounter += 6;

                mapTexcoords[vertex_texcoord_counter] = RF_CLITERAL(rf_vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[vertex_texcoord_counter + 1] = RF_CLITERAL(rf_vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[vertex_texcoord_counter + 2] = RF_CLITERAL(rf_vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[vertex_texcoord_counter + 3] = RF_CLITERAL(rf_vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[vertex_texcoord_counter + 4] = RF_CLITERAL(rf_vector2){ bottomTexUV.x, bottomTexUV.y };
                mapTexcoords[vertex_texcoord_counter + 5] = RF_CLITERAL(rf_vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                vertex_texcoord_counter += 6;

                if (((z < cubicmap.height - 1) &&
                     (cubicmapPixels[(z + 1)*cubicmap.width + x].r == 0) &&
                     (cubicmapPixels[(z + 1)*cubicmap.width + x].g == 0) &&
                     (cubicmapPixels[(z + 1)*cubicmap.width + x].b == 0)) || (z == cubicmap.height - 1))
                {
                    // Define front triangles (2 tris, 6 vertex) --> v2 v7 v3, v3 v7 v8
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vertex_pos_counter] = v2;
                    mapVertices[vertex_pos_counter + 1] = v7;
                    mapVertices[vertex_pos_counter + 2] = v3;
                    mapVertices[vertex_pos_counter + 3] = v3;
                    mapVertices[vertex_pos_counter + 4] = v7;
                    mapVertices[vertex_pos_counter + 5] = v8;
                    vertex_pos_counter += 6;

                    mapNormals[nCounter] = n6;
                    mapNormals[nCounter + 1] = n6;
                    mapNormals[nCounter + 2] = n6;
                    mapNormals[nCounter + 3] = n6;
                    mapNormals[nCounter + 4] = n6;
                    mapNormals[nCounter + 5] = n6;
                    nCounter += 6;

                    mapTexcoords[vertex_texcoord_counter] = RF_CLITERAL(rf_vector2){ frontTexUV.x, frontTexUV.y };
                    mapTexcoords[vertex_texcoord_counter + 1] = RF_CLITERAL(rf_vector2){ frontTexUV.x, frontTexUV.y + frontTexUV.height };
                    mapTexcoords[vertex_texcoord_counter + 2] = RF_CLITERAL(rf_vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y };
                    mapTexcoords[vertex_texcoord_counter + 3] = RF_CLITERAL(rf_vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y };
                    mapTexcoords[vertex_texcoord_counter + 4] = RF_CLITERAL(rf_vector2){ frontTexUV.x, frontTexUV.y + frontTexUV.height };
                    mapTexcoords[vertex_texcoord_counter + 5] = RF_CLITERAL(rf_vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y + frontTexUV.height };
                    vertex_texcoord_counter += 6;
                }

                if (((z > 0) &&
                     (cubicmapPixels[(z - 1)*cubicmap.width + x].r == 0) &&
                     (cubicmapPixels[(z - 1)*cubicmap.width + x].g == 0) &&
                     (cubicmapPixels[(z - 1)*cubicmap.width + x].b == 0)) || (z == 0))
                {
                    // Define back triangles (2 tris, 6 vertex) --> v1 v5 v6, v1 v4 v5
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vertex_pos_counter] = v1;
                    mapVertices[vertex_pos_counter + 1] = v5;
                    mapVertices[vertex_pos_counter + 2] = v6;
                    mapVertices[vertex_pos_counter + 3] = v1;
                    mapVertices[vertex_pos_counter + 4] = v4;
                    mapVertices[vertex_pos_counter + 5] = v5;
                    vertex_pos_counter += 6;

                    mapNormals[nCounter] = n5;
                    mapNormals[nCounter + 1] = n5;
                    mapNormals[nCounter + 2] = n5;
                    mapNormals[nCounter + 3] = n5;
                    mapNormals[nCounter + 4] = n5;
                    mapNormals[nCounter + 5] = n5;
                    nCounter += 6;

                    mapTexcoords[vertex_texcoord_counter] = RF_CLITERAL(rf_vector2){ backTexUV.x + backTexUV.width, backTexUV.y };
                    mapTexcoords[vertex_texcoord_counter + 1] = RF_CLITERAL(rf_vector2){ backTexUV.x, backTexUV.y + backTexUV.height };
                    mapTexcoords[vertex_texcoord_counter + 2] = RF_CLITERAL(rf_vector2){ backTexUV.x + backTexUV.width, backTexUV.y + backTexUV.height };
                    mapTexcoords[vertex_texcoord_counter + 3] = RF_CLITERAL(rf_vector2){ backTexUV.x + backTexUV.width, backTexUV.y };
                    mapTexcoords[vertex_texcoord_counter + 4] = RF_CLITERAL(rf_vector2){ backTexUV.x, backTexUV.y };
                    mapTexcoords[vertex_texcoord_counter + 5] = RF_CLITERAL(rf_vector2){ backTexUV.x, backTexUV.y + backTexUV.height };
                    vertex_texcoord_counter += 6;
                }

                if (((x < cubicmap.width - 1) &&
                     (cubicmapPixels[z*cubicmap.width + (x + 1)].r == 0) &&
                     (cubicmapPixels[z*cubicmap.width + (x + 1)].g == 0) &&
                     (cubicmapPixels[z*cubicmap.width + (x + 1)].b == 0)) || (x == cubicmap.width - 1))
                {
                    // Define right triangles (2 tris, 6 vertex) --> v3 v8 v4, v4 v8 v5
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vertex_pos_counter] = v3;
                    mapVertices[vertex_pos_counter + 1] = v8;
                    mapVertices[vertex_pos_counter + 2] = v4;
                    mapVertices[vertex_pos_counter + 3] = v4;
                    mapVertices[vertex_pos_counter + 4] = v8;
                    mapVertices[vertex_pos_counter + 5] = v5;
                    vertex_pos_counter += 6;

                    mapNormals[nCounter] = n1;
                    mapNormals[nCounter + 1] = n1;
                    mapNormals[nCounter + 2] = n1;
                    mapNormals[nCounter + 3] = n1;
                    mapNormals[nCounter + 4] = n1;
                    mapNormals[nCounter + 5] = n1;
                    nCounter += 6;

                    mapTexcoords[vertex_texcoord_counter] = RF_CLITERAL(rf_vector2){ rightTexUV.x, rightTexUV.y };
                    mapTexcoords[vertex_texcoord_counter + 1] = RF_CLITERAL(rf_vector2){ rightTexUV.x, rightTexUV.y + rightTexUV.height };
                    mapTexcoords[vertex_texcoord_counter + 2] = RF_CLITERAL(rf_vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y };
                    mapTexcoords[vertex_texcoord_counter + 3] = RF_CLITERAL(rf_vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y };
                    mapTexcoords[vertex_texcoord_counter + 4] = RF_CLITERAL(rf_vector2){ rightTexUV.x, rightTexUV.y + rightTexUV.height };
                    mapTexcoords[vertex_texcoord_counter + 5] = RF_CLITERAL(rf_vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y + rightTexUV.height };
                    vertex_texcoord_counter += 6;
                }

                if (((x > 0) &&
                     (cubicmapPixels[z*cubicmap.width + (x - 1)].r == 0) &&
                     (cubicmapPixels[z*cubicmap.width + (x - 1)].g == 0) &&
                     (cubicmapPixels[z*cubicmap.width + (x - 1)].b == 0)) || (x == 0))
                {
                    // Define left triangles (2 tris, 6 vertex) --> v1 v7 v2, v1 v6 v7
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vertex_pos_counter] = v1;
                    mapVertices[vertex_pos_counter + 1] = v7;
                    mapVertices[vertex_pos_counter + 2] = v2;
                    mapVertices[vertex_pos_counter + 3] = v1;
                    mapVertices[vertex_pos_counter + 4] = v6;
                    mapVertices[vertex_pos_counter + 5] = v7;
                    vertex_pos_counter += 6;

                    mapNormals[nCounter] = n2;
                    mapNormals[nCounter + 1] = n2;
                    mapNormals[nCounter + 2] = n2;
                    mapNormals[nCounter + 3] = n2;
                    mapNormals[nCounter + 4] = n2;
                    mapNormals[nCounter + 5] = n2;
                    nCounter += 6;

                    mapTexcoords[vertex_texcoord_counter] = RF_CLITERAL(rf_vector2){ leftTexUV.x, leftTexUV.y };
                    mapTexcoords[vertex_texcoord_counter + 1] = RF_CLITERAL(rf_vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y + leftTexUV.height };
                    mapTexcoords[vertex_texcoord_counter + 2] = RF_CLITERAL(rf_vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y };
                    mapTexcoords[vertex_texcoord_counter + 3] = RF_CLITERAL(rf_vector2){ leftTexUV.x, leftTexUV.y };
                    mapTexcoords[vertex_texcoord_counter + 4] = RF_CLITERAL(rf_vector2){ leftTexUV.x, leftTexUV.y + leftTexUV.height };
                    mapTexcoords[vertex_texcoord_counter + 5] = RF_CLITERAL(rf_vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y + leftTexUV.height };
                    vertex_texcoord_counter += 6;
                }
            }
                // We check pixel color to be rf_black, we will only draw floor and roof
            else if ((cubicmapPixels[z*cubicmap.width + x].r == 0) &&
                     (cubicmapPixels[z*cubicmap.width + x].g == 0) &&
                     (cubicmapPixels[z*cubicmap.width + x].b == 0))
            {
                // Define top triangles (2 tris, 6 vertex --> v1-v2-v3, v1-v3-v4)
                mapVertices[vertex_pos_counter] = v1;
                mapVertices[vertex_pos_counter + 1] = v3;
                mapVertices[vertex_pos_counter + 2] = v2;
                mapVertices[vertex_pos_counter + 3] = v1;
                mapVertices[vertex_pos_counter + 4] = v4;
                mapVertices[vertex_pos_counter + 5] = v3;
                vertex_pos_counter += 6;

                mapNormals[nCounter] = n4;
                mapNormals[nCounter + 1] = n4;
                mapNormals[nCounter + 2] = n4;
                mapNormals[nCounter + 3] = n4;
                mapNormals[nCounter + 4] = n4;
                mapNormals[nCounter + 5] = n4;
                nCounter += 6;

                mapTexcoords[vertex_texcoord_counter] = RF_CLITERAL(rf_vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[vertex_texcoord_counter + 1] = RF_CLITERAL(rf_vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                mapTexcoords[vertex_texcoord_counter + 2] = RF_CLITERAL(rf_vector2){ topTexUV.x, topTexUV.y + topTexUV.height };
                mapTexcoords[vertex_texcoord_counter + 3] = RF_CLITERAL(rf_vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[vertex_texcoord_counter + 4] = RF_CLITERAL(rf_vector2){ topTexUV.x + topTexUV.width, topTexUV.y };
                mapTexcoords[vertex_texcoord_counter + 5] = RF_CLITERAL(rf_vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                vertex_texcoord_counter += 6;

                // Define bottom triangles (2 tris, 6 vertex --> v6-v8-v7, v6-v5-v8)
                mapVertices[vertex_pos_counter] = v6;
                mapVertices[vertex_pos_counter + 1] = v7;
                mapVertices[vertex_pos_counter + 2] = v8;
                mapVertices[vertex_pos_counter + 3] = v6;
                mapVertices[vertex_pos_counter + 4] = v8;
                mapVertices[vertex_pos_counter + 5] = v5;
                vertex_pos_counter += 6;

                mapNormals[nCounter] = n3;
                mapNormals[nCounter + 1] = n3;
                mapNormals[nCounter + 2] = n3;
                mapNormals[nCounter + 3] = n3;
                mapNormals[nCounter + 4] = n3;
                mapNormals[nCounter + 5] = n3;
                nCounter += 6;

                mapTexcoords[vertex_texcoord_counter] = RF_CLITERAL(rf_vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[vertex_texcoord_counter + 1] = RF_CLITERAL(rf_vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[vertex_texcoord_counter + 2] = RF_CLITERAL(rf_vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[vertex_texcoord_counter + 3] = RF_CLITERAL(rf_vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[vertex_texcoord_counter + 4] = RF_CLITERAL(rf_vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[vertex_texcoord_counter + 5] = RF_CLITERAL(rf_vector2){ bottomTexUV.x, bottomTexUV.y };
                vertex_texcoord_counter += 6;
            }
        }
    }

    // Move data from mapVertices temp arays to vertices float array
    mesh.vertex_count = vertex_pos_counter;
    mesh.triangle_count = vertex_pos_counter/3;

    mesh.vertices = (float* )RF_MALLOC(mesh.vertex_count*3*sizeof(float));
    mesh.normals = (float* )RF_MALLOC(mesh.vertex_count*3*sizeof(float));
    mesh.texcoords = (float* )RF_MALLOC(mesh.vertex_count*2*sizeof(float));
    mesh.colors = NULL;

    int fCounter = 0;

    // Move vertices data
    for (int i = 0; i < vertex_pos_counter; i++)
    {
        mesh.vertices[fCounter] = mapVertices[i].x;
        mesh.vertices[fCounter + 1] = mapVertices[i].y;
        mesh.vertices[fCounter + 2] = mapVertices[i].z;
        fCounter += 3;
    }

    fCounter = 0;

    // Move normals data
    for (int i = 0; i < nCounter; i++)
    {
        mesh.normals[fCounter] = mapNormals[i].x;
        mesh.normals[fCounter + 1] = mapNormals[i].y;
        mesh.normals[fCounter + 2] = mapNormals[i].z;
        fCounter += 3;
    }

    fCounter = 0;

    // Move texcoords data
    for (int i = 0; i < vertex_texcoord_counter; i++)
    {
        mesh.texcoords[fCounter] = mapTexcoords[i].x;
        mesh.texcoords[fCounter + 1] = mapTexcoords[i].y;
        fCounter += 2;
    }

    RF_FREE(mapVertices);
    RF_FREE(mapNormals);
    RF_FREE(mapTexcoords);

    RF_FREE(cubicmapPixels); // Free image pixel data

    // Upload vertex data to GPU (static mesh)
    rf_gl_load_mesh(&mesh, false);

    return mesh;
}

// Compute mesh bounding box limits
// NOTE: minVertex and maxVertex should be transformed by model transform matrix
RF_API rf_bounding_box rf_mesh_bounding_box(rf_mesh mesh)
{
    // Get min and max vertex to construct bounds (AABB)
    rf_vector3 minVertex = { 0 };
    rf_vector3 maxVertex = { 0 };

    if (mesh.vertices != NULL)
    {
        minVertex = (rf_vector3){ mesh.vertices[0], mesh.vertices[1], mesh.vertices[2] };
        maxVertex = (rf_vector3){ mesh.vertices[0], mesh.vertices[1], mesh.vertices[2] };

        for (int i = 1; i < mesh.vertex_count; i++)
        {
            minVertex = rf_vector3_min(minVertex, (rf_vector3){ mesh.vertices[i*3], mesh.vertices[i*3 + 1], mesh.vertices[i*3 + 2] });
            maxVertex = rf_vector3_max(maxVertex, (rf_vector3){ mesh.vertices[i*3], mesh.vertices[i*3 + 1], mesh.vertices[i*3 + 2] });
        }
    }

    // Create the bounding box
    rf_bounding_box box = { 0 };
    box.min = minVertex;
    box.max = maxVertex;

    return box;
}

// Compute mesh tangents
// NOTE: To calculate mesh tangents and binormals we need mesh vertex positions and texture coordinates
// Implementation base don: https://answers.unity.com/questions/7789/calculating-tangents-vector4.html
RF_API void rf_mesh_tangents(rf_mesh* mesh)
{
    if (mesh->tangents == NULL) mesh->tangents = (float* )RF_MALLOC(mesh->vertex_count*4*sizeof(float));
    else RF_LOG(RF_LOG_WARNING, "rf_mesh tangents already exist");

    rf_vector3* tan1 = (rf_vector3* )RF_MALLOC(mesh->vertex_count*sizeof(rf_vector3));
    rf_vector3* tan2 = (rf_vector3* )RF_MALLOC(mesh->vertex_count*sizeof(rf_vector3));

    for (int i = 0; i < mesh->vertex_count; i += 3)
    {
        // Get triangle vertices
        rf_vector3 v1 = { mesh->vertices[(i + 0)*3 + 0], mesh->vertices[(i + 0)*3 + 1], mesh->vertices[(i + 0)*3 + 2] };
        rf_vector3 v2 = { mesh->vertices[(i + 1)*3 + 0], mesh->vertices[(i + 1)*3 + 1], mesh->vertices[(i + 1)*3 + 2] };
        rf_vector3 v3 = { mesh->vertices[(i + 2)*3 + 0], mesh->vertices[(i + 2)*3 + 1], mesh->vertices[(i + 2)*3 + 2] };

        // Get triangle texcoords
        rf_vector2 uv1 = { mesh->texcoords[(i + 0)*2 + 0], mesh->texcoords[(i + 0)*2 + 1] };
        rf_vector2 uv2 = { mesh->texcoords[(i + 1)*2 + 0], mesh->texcoords[(i + 1)*2 + 1] };
        rf_vector2 uv3 = { mesh->texcoords[(i + 2)*2 + 0], mesh->texcoords[(i + 2)*2 + 1] };

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

        float div = s1*t2 - s2*t1;
        float r = (div == 0.0f)? 0.0f : 1.0f/div;

        rf_vector3 sdir = { (t2*x1 - t1*x2)*r, (t2*y1 - t1*y2)*r, (t2*z1 - t1*z2)*r };
        rf_vector3 tdir = { (s1*x2 - s2*x1)*r, (s1*y2 - s2*y1)*r, (s1*z2 - s2*z1)*r };

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
        rf_vector3 normal = { mesh->normals[i*3 + 0], mesh->normals[i*3 + 1], mesh->normals[i*3 + 2] };
        rf_vector3 tangent = tan1[i];

        // TODO: Review, not sure if tangent computation is right, just used reference proposed maths...
        rf_vector3_ortho_normalize(&normal, &tangent);
        mesh->tangents[i*4 + 0] = tangent.x;
        mesh->tangents[i*4 + 1] = tangent.y;
        mesh->tangents[i*4 + 2] = tangent.z;
        mesh->tangents[i*4 + 3] = (rf_vector3_dot_product(rf_vector3_cross_product(normal, tangent), tan2[i]) < 0.0f)? -1.0f : 1.0f;

    }

    RF_FREE(tan1);
    RF_FREE(tan2);

    // Load a new tangent attributes buffer
    mesh->vbo_id[rf_loc_vertex_tangent] = rf_gl_load_attrib_buffer(mesh->vao_id, rf_loc_vertex_tangent, mesh->tangents, mesh->vertex_count*4*sizeof(float), false);

    RF_LOG(RF_LOG_INFO, "Tangents computed for mesh");
}

// Compute mesh binormals (aka bitangent)
RF_API void rf_mesh_binormals(rf_mesh* mesh)
{
    for (int i = 0; i < mesh->vertex_count; i++)
    {
        rf_vector3 normal = { mesh->normals[i*3 + 0], mesh->normals[i*3 + 1], mesh->normals[i*3 + 2] };
        rf_vector3 tangent = { mesh->tangents[i*4 + 0], mesh->tangents[i*4 + 1], mesh->tangents[i*4 + 2] };
        float tangentW = mesh->tangents[i*4 + 3];

        // TODO: Register computed binormal in mesh->binormal?
        // rf_vector3 binormal = rf_vector3_multiply(rf_vector3_cross_product(normal, tangent), tangentW);
    }
}

// Draw a model (with texture if set)
RF_API void rf_draw_model(rf_model model, rf_vector3 position, float scale, rf_color tint)
{
    rf_vector3 vScale = { scale, scale, scale };
    rf_vector3 rotationAxis = { 0.0f, 1.0f, 0.0f };

    rf_draw_model_ex(model, position, rotationAxis, 0.0f, vScale, tint);
}

// Draw a model with extended parameters
RF_API void rf_draw_model_ex(rf_model model, rf_vector3 position, rf_vector3 rotationAxis, float rotationAngle, rf_vector3 scale, rf_color tint)
{
    // Calculate transformation matrix from function parameters
    // Get transform matrix (rotation -> scale -> translation)
    rf_matrix matScale = rf_matrix_scale(scale.x, scale.y, scale.z);
    rf_matrix matRotation = rf_matrix_rotate(rotationAxis, rotationAngle*RF_DEG2RAD);
    rf_matrix matTranslation = rf_matrix_translate(position.x, position.y, position.z);

    rf_matrix matTransform = rf_matrix_multiply(rf_matrix_multiply(matScale, matRotation), matTranslation);

    // Combine model transformation matrix (model.transform) with matrix generated by function parameters (matTransform)
    model.transform = rf_matrix_multiply(model.transform, matTransform);

    for (int i = 0; i < model.mesh_count; i++)
    {
        // TODO: Review color + tint premultiplication mechanism
        rf_color color = model.materials[model.mesh_material[i]].maps[rf_map_diffuse].color;

        rf_color colorTint = rf_white;
        colorTint.r = (((float)color.r/255.0)*((float)tint.r/255.0))*255;
        colorTint.g = (((float)color.g/255.0)*((float)tint.g/255.0))*255;
        colorTint.b = (((float)color.b/255.0)*((float)tint.b/255.0))*255;
        colorTint.a = (((float)color.a/255.0)*((float)tint.a/255.0))*255;

        model.materials[model.mesh_material[i]].maps[rf_map_diffuse].color = colorTint;
        rf_gl_draw_mesh(model.meshes[i], model.materials[model.mesh_material[i]], model.transform);
        model.materials[model.mesh_material[i]].maps[rf_map_diffuse].color = color;
    }
}

// Draw a model wires (with texture if set)
RF_API void rf_draw_model_wires(rf_model model, rf_vector3 position, float scale, rf_color tint)
{
    rf_gl_enable_wire_mode();

    rf_draw_model(model, position, scale, tint);

    rf_gl_disable_wire_mode();
}

// Draw a model wires (with texture if set) with extended parameters
RF_API void rf_draw_model_wires_ex(rf_model model, rf_vector3 position, rf_vector3 rotationAxis, float rotationAngle, rf_vector3 scale, rf_color tint)
{
    rf_gl_enable_wire_mode();

    rf_draw_model_ex(model, position, rotationAxis, rotationAngle, scale, tint);

    rf_gl_disable_wire_mode();
}

// Draw a billboard
RF_API void rf_draw_billboard(rf_camera3d camera, rf_texture2d texture, rf_vector3 center, float size, rf_color tint)
{
    rf_rectangle source_rec = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };

    rf_draw_billboard_rec(camera, texture, source_rec, center, size, tint);
}

// Draw a billboard (part of a texture defined by a rectangle)
RF_API void rf_draw_billboard_rec(rf_camera3d camera, rf_texture2d texture, rf_rectangle source_rec, rf_vector3 center, float size, rf_color tint)
{
    // NOTE: Billboard size will maintain source_rec aspect ratio, size will represent billboard width
    rf_vector2 sizeRatio = { size, size*(float)source_rec.height/source_rec.width };

    rf_matrix matView = rf_matrix_look_at(camera.position, camera.target, camera.up);

    rf_vector3 right = { matView.m0, matView.m4, matView.m8 };
    //rf_vector3 up = { matView.m1, matView.m5, matView.m9 };

    // NOTE: Billboard locked on axis-Y
    rf_vector3 up = { 0.0f, 1.0f, 0.0f };
    /*
        a-------b
        |       |
        |   *   |
        |       |
        d-------c
    */
    right = rf_vector3_scale(right, sizeRatio.x/2);
    up = rf_vector3_scale(up, sizeRatio.y/2);

    rf_vector3 p1 = rf_vector3_add(right, up);
    rf_vector3 p2 = rf_vector3_substract(right, up);

    rf_vector3 a = rf_vector3_substract(center, p2);
    rf_vector3 b = rf_vector3_add(center, p1);
    rf_vector3 c = rf_vector3_add(center, p2);
    rf_vector3 d = rf_vector3_substract(center, p1);

    if (rf_gl_check_buffer_limit(4)) rf_gl_draw();

    rf_gl_enable_texture(texture.id);

    rf_gl_begin(GL_QUADS);
    rf_gl_color4ub(tint.r, tint.g, tint.b, tint.a);

    // Bottom-left corner for texture and quad
    rf_gl_tex_coord2f((float)source_rec.x/texture.width, (float)source_rec.y/texture.height);
    rf_gl_vertex3f(a.x, a.y, a.z);

    // Top-left corner for texture and quad
    rf_gl_tex_coord2f((float)source_rec.x/texture.width, (float)(source_rec.y + source_rec.height)/texture.height);
    rf_gl_vertex3f(d.x, d.y, d.z);

    // Top-right corner for texture and quad
    rf_gl_tex_coord2f((float)(source_rec.x + source_rec.width)/texture.width, (float)(source_rec.y + source_rec.height)/texture.height);
    rf_gl_vertex3f(c.x, c.y, c.z);

    // Bottom-right corner for texture and quad
    rf_gl_tex_coord2f((float)(source_rec.x + source_rec.width)/texture.width, (float)source_rec.y/texture.height);
    rf_gl_vertex3f(b.x, b.y, b.z);
    rf_gl_end();

    rf_gl_disable_texture();
}

// Draw a bounding box with wires
RF_API void rf_draw_bounding_box(rf_bounding_box box, rf_color color)
{
    rf_vector3 size;

    size.x = (float)fabs(box.max.x - box.min.x);
    size.y = (float)fabs(box.max.y - box.min.y);
    size.z = (float)fabs(box.max.z - box.min.z);

    rf_vector3 center = { box.min.x + size.x/2.0f, box.min.y + size.y/2.0f, box.min.z + size.z/2.0f };

    rf_draw_cube_wires(center, size.x, size.y, size.z, color);
}

// Detect collision between two spheres
RF_API bool rf_check_collision_spheres(rf_vector3 centerA, float radiusA, rf_vector3 centerB, float radiusB)
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
    if (rf_vector3_dot_product(rf_vector3_substract(centerB, centerA), rf_vector3_substract(centerB, centerA)) <= (radiusA + radiusB)*(radiusA + radiusB)) collision = true;

    return collision;
}

// Detect collision between two boxes
// NOTE: Boxes are defined by two points minimum and maximum
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
RF_API bool rf_check_collision_box_sphere(rf_bounding_box box, rf_vector3 center, float radius)
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
RF_API bool rf_check_collision_ray_sphere(rf_ray ray, rf_vector3 center, float radius)
{
    bool collision = false;

    rf_vector3 raySpherePos = rf_vector3_substract(center, ray.position);
    float distance = rf_vector3_length(raySpherePos);
    float vector = rf_vector3_dot_product(raySpherePos, ray.direction);
    float d = radius*radius - (distance*distance - vector*vector);

    if (d >= 0.0f) collision = true;

    return collision;
}

// Detect collision between ray and sphere with extended parameters and collision point detection
RF_API bool rf_check_collision_ray_sphere_ex(rf_ray ray, rf_vector3 center, float radius, rf_vector3* collisionPoint)
{
    bool collision = false;

    rf_vector3 raySpherePos = rf_vector3_substract(center, ray.position);
    float distance = rf_vector3_length(raySpherePos);
    float vector = rf_vector3_dot_product(raySpherePos, ray.direction);
    float d = radius*radius - (distance*distance - vector*vector);

    if (d >= 0.0f) collision = true;

    // Check if ray origin is inside the sphere to calculate the correct collision point
    float collisionDistance = 0;

    if (distance < radius) collisionDistance = vector + sqrtf(d);
    else collisionDistance = vector - sqrtf(d);

    // Calculate collision point
    rf_vector3 cPoint = rf_vector3_add(ray.position, rf_vector3_scale(ray.direction, collisionDistance));

    collisionPoint->x = cPoint.x;
    collisionPoint->y = cPoint.y;
    collisionPoint->z = cPoint.z;

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
                rf_vector3 a, b, c;
                rf_vector3* vertdata = (rf_vector3* )model.meshes[m].vertices;

                if (model.meshes[m].indices)
                {
                    a = vertdata[model.meshes[m].indices[i*3 + 0]];
                    b = vertdata[model.meshes[m].indices[i*3 + 1]];
                    c = vertdata[model.meshes[m].indices[i*3 + 2]];
                }
                else
                {
                    a = vertdata[i*3 + 0];
                    b = vertdata[i*3 + 1];
                    c = vertdata[i*3 + 2];
                }

                a = rf_vector3_transform(a, model.transform);
                b = rf_vector3_transform(b, model.transform);
                c = rf_vector3_transform(c, model.transform);

                rf_ray_hit_info triHitInfo = rf_get_collision_ray_triangle(ray, a, b, c);

                if (triHitInfo.hit)
                {
                    // Save the closest hit triangle
                    if ((!result.hit) || (result.distance > triHitInfo.distance)) result = triHitInfo;
                }
            }
        }
    }

    return result;
}

// Get collision info between ray and triangle
// NOTE: Based on https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
RF_API rf_ray_hit_info rf_get_collision_ray_triangle(rf_ray ray, rf_vector3 p1, rf_vector3 p2, rf_vector3 p3)
{
#define rf_epsilon 0.000001 // A small number

    rf_vector3 edge1, edge2;
    rf_vector3 p, q, tv;
    float det, invDet, u, v, t;
    rf_ray_hit_info result = {0};

    // Find vectors for two edges sharing V1
    edge1 = rf_vector3_substract(p2, p1);
    edge2 = rf_vector3_substract(p3, p1);

    // Begin calculating determinant - also used to calculate u parameter
    p = rf_vector3_cross_product(ray.direction, edge2);

    // If determinant is near zero, ray lies in plane of triangle or ray is parallel to plane of triangle
    det = rf_vector3_dot_product(edge1, p);

    // Avoid culling!
    if ((det > -rf_epsilon) && (det < rf_epsilon)) return result;

    invDet = 1.0f/det;

    // Calculate distance from V1 to ray origin
    tv = rf_vector3_substract(ray.position, p1);

    // Calculate u parameter and test bound
    u = rf_vector3_dot_product(tv, p)*invDet;

    // The intersection lies outside of the triangle
    if ((u < 0.0f) || (u > 1.0f)) return result;

    // Prepare to test v parameter
    q = rf_vector3_cross_product(tv, edge1);

    // Calculate V parameter and test bound
    v = rf_vector3_dot_product(ray.direction, q)*invDet;

    // The intersection lies outside of the triangle
    if ((v < 0.0f) || ((u + v) > 1.0f)) return result;

    t = rf_vector3_dot_product(edge2, q)*invDet;

    if (t > rf_epsilon)
    {
        // rf_ray hit, get hit point and normal
        result.hit = true;
        result.distance = t;
        result.hit = true;
        result.normal = rf_vector3_normalize(rf_vector3_cross_product(edge1, edge2));
        result.position = rf_vector3_add(ray.position, rf_vector3_scale(ray.direction, t));
    }

    return result;
}

// Get collision info between ray and ground plane (Y-normal plane)
RF_API rf_ray_hit_info rf_get_collision_ray_ground(rf_ray ray, float groundHeight)
{
#define rf_epsilon 0.000001 // A small number

    rf_ray_hit_info result = { 0 };

    if (fabs(ray.direction.y) > rf_epsilon)
    {
        float distance = (ray.position.y - groundHeight)/-ray.direction.y;

        if (distance >= 0.0)
        {
            result.hit = true;
            result.distance = distance;
            result.normal = (rf_vector3){ 0.0, 1.0, 0.0 };
            result.position = rf_vector3_add(ray.position, rf_vector3_scale(ray.direction, distance));
        }
    }

    return result;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Load OBJ mesh data
RF_INTERNAL rf_model _rf_load_obj(const char* fileName)
{
    rf_model model = { 0 };

    tinyobj_attrib_t attrib;
    tinyobj_shape_t *meshes = NULL;
    unsigned int mesh_count = 0;

    tinyobj_material_t *materials = NULL;
    unsigned int material_count = 0;

    int dataLength = 0;
    char* data = NULL;

    // Load model data
    FILE* objFile = fopen(fileName, "rb");

    if (objFile != NULL)
    {
        fseek(objFile, 0, SEEK_END);
        long length = ftell(objFile); // Get file size
        fseek(objFile, 0, SEEK_SET); // Reset file pointer

        data = (char* )RF_MALLOC(length);

        fread(data, length, 1, objFile);
        dataLength = length;
        fclose(objFile);
    }

    if (data != NULL)
    {
        unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;
        int ret = tinyobj_parse_obj(&attrib, &meshes, (size_t*) &mesh_count, &materials, (size_t*) &material_count, data, dataLength, flags);

        if (ret != TINYOBJ_SUCCESS) RF_LOG(RF_LOG_WARNING, "[%s] rf_model data could not be loaded", fileName);
        else RF_LOG(RF_LOG_INFO, "[%s] rf_model data loaded successfully: %i meshes / %i materials", fileName, mesh_count, material_count);

        // Init model meshes array
        // TODO: Support multiple meshes... in the meantime, only one mesh is returned
        //model.mesh_count = mesh_count;
        model.mesh_count = 1;
        model.meshes = (rf_mesh*)RF_MALLOC(model.mesh_count * sizeof(rf_mesh));
        memset(model.meshes, 0, model.mesh_count * sizeof(rf_mesh));

        // Init model materials array
        if (material_count > 0)
        {
            model.material_count = material_count;
            model.materials = (rf_material* )RF_MALLOC(model.material_count * sizeof(rf_material));
            memset(model.materials, 0, model.material_count * sizeof(rf_material));
        }

        model.mesh_material = (int* )RF_MALLOC(model.mesh_count * sizeof(int));
        memset(model.mesh_material, 0, model.mesh_count * sizeof(int));

        /*
        // Multiple meshes data reference
        // NOTE: They are provided as a faces offset
        typedef struct {
            char* name;         // group name or object name
            unsigned int face_offset;
            unsigned int length;
        } tinyobj_shape_t;
        */

        // Init model meshes
        for (int m = 0; m < 1; m++)
        {
            rf_mesh mesh = { 0 };
            memset(&mesh, 0, sizeof(rf_mesh));
            mesh.vertex_count = attrib.num_faces*3;
            mesh.triangle_count = attrib.num_faces;
            mesh.vertices = (float*)RF_MALLOC(mesh.vertex_count*3 * sizeof(float));
            memset(mesh.vertices, 0, mesh.vertex_count*3 * sizeof(float));
            mesh.texcoords = (float*)RF_MALLOC(mesh.vertex_count*2 * sizeof(float));
            memset(mesh.texcoords, 0, mesh.vertex_count*2 * sizeof(float));
            mesh.normals = (float*)RF_MALLOC(mesh.vertex_count*3 * sizeof(float));
            memset(mesh.normals, 0, mesh.vertex_count*3 * sizeof(float));
            mesh.vbo_id = (unsigned int*)RF_MALLOC(rf_max_mesh_vbo * sizeof(unsigned int));
            memset(mesh.vbo_id, 0, rf_max_mesh_vbo * sizeof(unsigned int));

            int vCount = 0;
            int vtCount = 0;
            int vnCount = 0;

            for (int f = 0; f < attrib.num_faces; f++)
            {
                // Get indices for the face
                tinyobj_vertex_index_t idx0 = attrib.faces[3*f + 0];
                tinyobj_vertex_index_t idx1 = attrib.faces[3*f + 1];
                tinyobj_vertex_index_t idx2 = attrib.faces[3*f + 2];

                // RF_LOG(RF_LOG_DEBUG, "Face %i index: v %i/%i/%i . vt %i/%i/%i . vn %i/%i/%i\n", f, idx0.v_idx, idx1.v_idx, idx2.v_idx, idx0.vt_idx, idx1.vt_idx, idx2.vt_idx, idx0.vn_idx, idx1.vn_idx, idx2.vn_idx);

                // Fill vertices buffer (float) using vertex index of the face
                for (int v = 0; v < 3; v++) { mesh.vertices[vCount + v] = attrib.vertices[idx0.v_idx*3 + v]; } vCount +=3;
                for (int v = 0; v < 3; v++) { mesh.vertices[vCount + v] = attrib.vertices[idx1.v_idx*3 + v]; } vCount +=3;
                for (int v = 0; v < 3; v++) { mesh.vertices[vCount + v] = attrib.vertices[idx2.v_idx*3 + v]; } vCount +=3;

                // Fill texcoords buffer (float) using vertex index of the face
                // NOTE: Y-coordinate must be flipped upside-down
                mesh.texcoords[vtCount + 0] = attrib.texcoords[idx0.vt_idx*2 + 0];
                mesh.texcoords[vtCount + 1] = 1.0f - attrib.texcoords[idx0.vt_idx*2 + 1]; vtCount += 2;
                mesh.texcoords[vtCount + 0] = attrib.texcoords[idx1.vt_idx*2 + 0];
                mesh.texcoords[vtCount + 1] = 1.0f - attrib.texcoords[idx1.vt_idx*2 + 1]; vtCount += 2;
                mesh.texcoords[vtCount + 0] = attrib.texcoords[idx2.vt_idx*2 + 0];
                mesh.texcoords[vtCount + 1] = 1.0f - attrib.texcoords[idx2.vt_idx*2 + 1]; vtCount += 2;

                // Fill normals buffer (float) using vertex index of the face
                for (int v = 0; v < 3; v++) { mesh.normals[vnCount + v] = attrib.normals[idx0.vn_idx*3 + v]; } vnCount +=3;
                for (int v = 0; v < 3; v++) { mesh.normals[vnCount + v] = attrib.normals[idx1.vn_idx*3 + v]; } vnCount +=3;
                for (int v = 0; v < 3; v++) { mesh.normals[vnCount + v] = attrib.normals[idx2.vn_idx*3 + v]; } vnCount +=3;
            }

            model.meshes[m] = mesh; // Assign mesh data to model

            // Assign mesh material for current mesh
            model.mesh_material[m] = attrib.material_ids[m];

            // Set unfound materials to default
            if (model.mesh_material[m] == -1) model.mesh_material[m] = 0;
        }

        // Init model materials
        for (int m = 0; m < material_count; m++)
        {
            // Init material to default
            // NOTE: Uses default shader, only rf_map_diffuse supported
            model.materials[m] = rf_load_material_default();

            /*
            typedef struct {
                char* name;
                float ambient[3];
                float diffuse[3];
                float specular[3];
                float transmittance[3];
                float emission[3];
                float shininess;
                float ior;          // index of refraction
                float dissolve;     // 1 == opaque; 0 == fully transparent
                // illumination model (see http://www.fileformat.info/format/material/)
                int illum;
                int pad0;
                char* ambient_texname;            // map_Ka
                char* diffuse_texname;            // map_Kd
                char* specular_texname;           // map_Ks
                char* specular_highlight_texname; // map_Ns
                char* bump_texname;               // map_bump, bump
                char* displacement_texname;       // disp
                char* alpha_texname;              // map_d
            } tinyobj_material_t;
            */
            model.materials[m].maps[rf_map_diffuse].texture = rf_get_texture_default(); // Get default texture, in case no texture is defined

            if (materials[m].diffuse_texname != NULL) model.materials[m].maps[rf_map_diffuse].texture = rf_load_texture(materials[m].diffuse_texname); //char* diffuse_texname; // map_Kd
            model.materials[m].maps[rf_map_diffuse].color = RF_CLITERAL(rf_color){ (float)(materials[m].diffuse[0]*255.0f), (float)(materials[m].diffuse[1]*255.0f), (float)(materials[m].diffuse[2]*255.0f), 255 }; //float diffuse[3];
            model.materials[m].maps[rf_map_diffuse].value = 0.0f;

            if (materials[m].specular_texname != NULL) model.materials[m].maps[rf_map_specular].texture = rf_load_texture(materials[m].specular_texname); //char* specular_texname; // map_Ks
            model.materials[m].maps[rf_map_specular].color = RF_CLITERAL(rf_color){ (float)(materials[m].specular[0]*255.0f), (float)(materials[m].specular[1]*255.0f), (float)(materials[m].specular[2]*255.0f), 255 }; //float specular[3];
            model.materials[m].maps[rf_map_specular].value = 0.0f;

            if (materials[m].bump_texname != NULL) model.materials[m].maps[rf_map_normal].texture = rf_load_texture(materials[m].bump_texname); //char* bump_texname; // map_bump, bump
            model.materials[m].maps[rf_map_normal].color = rf_white;
            model.materials[m].maps[rf_map_normal].value = materials[m].shininess;

            model.materials[m].maps[rf_map_emission].color = RF_CLITERAL(rf_color){ (float)(materials[m].emission[0]*255.0f), (float)(materials[m].emission[1]*255.0f), (float)(materials[m].emission[2]*255.0f), 255 }; //float emission[3];

            if (materials[m].displacement_texname != NULL) model.materials[m].maps[rf_map_height].texture = rf_load_texture(materials[m].displacement_texname); //char* displacement_texname; // disp
        }

        tinyobj_attrib_free(&attrib);
        tinyobj_shapes_free(meshes, mesh_count);
        tinyobj_materials_free(materials, material_count);

        RF_FREE(data);
    }

    // NOTE: At this point we have all model data loaded
    RF_LOG(RF_LOG_INFO, "[%s] rf_model loaded successfully in RAM (CPU)", fileName);

    return model;
}

// Load IQM mesh data
RF_INTERNAL rf_model _rf_load_iqm(const char* fileName)
{
    #define rf_iqm_magic "INTERQUAKEMODEL" // IQM file magic number
    #define rf_iqm_version 2 // only IQM version 2 supported

    #define rf_bone_name_length 32 // rf_bone_info name string length
    #define rf_mesh_name_length 32 // rf_mesh name string length

    // IQM file structs
    //-----------------------------------------------------------------------------------
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
        int parent;
        float translate[3], rotate[4], scale[3];
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

    // NOTE: Below IQM structures are not used but listed for reference
    /*
    typedef struct IQMAdjacency {
        unsigned int triangle[3];
    } IQMAdjacency;

    typedef struct rf_iqm_pose {
        int parent;
        unsigned int mask;
        float channeloffset[10];
        float channelscale[10];
    } rf_iqm_pose;

    typedef struct rf_iqm_anim {
        unsigned int name;
        unsigned int first_frame, num_frames;
        float framerate;
        unsigned int flags;
    } rf_iqm_anim;

    typedef struct IQMBounds {
        float bbmin[3], bbmax[3];
        float xyradius, radius;
    } IQMBounds;
    */
    //-----------------------------------------------------------------------------------

    // IQM vertex data types
    typedef enum rf_iqm_vertex_type
    {
        rf_iqm_position = 0,
        rf_iqm_texcoord = 1,
        rf_iqm_normal = 2,
        rf_iqm_tangent = 3, // NOTE: Tangents unused by default
        rf_iqm_blendindexes = 4,
        rf_iqm_blendweights = 5,
        rf_iqm_color = 6, // NOTE: Vertex colors unused by default
        rf_iqm_custom = 0x10 // NOTE: Custom vertex values unused by default
    }  rf_iqm_vertex_type;

    rf_model model = { 0 };

    FILE* iqmFile;
    rf_iqm_header iqm;

    rf_iqm_mesh *imesh;
    rf_iqm_triangle *tri;
    rf_iqm_vertex_array *va;
    rf_iqm_joint *ijoint;

    float* vertex = NULL;
    float* normal = NULL;
    float* text = NULL;
    char* blendi = NULL;
    unsigned char* blendw = NULL;

    iqmFile = fopen(fileName, "rb");

    if (iqmFile == NULL)
    {
        RF_LOG(RF_LOG_WARNING, "[%s] IQM file could not be opened", fileName);
        return model;
    }

    fread(&iqm,sizeof(rf_iqm_header), 1, iqmFile); // Read IQM header

    if (strncmp(iqm.magic, rf_iqm_magic, sizeof(rf_iqm_magic)))
    {
        RF_LOG(RF_LOG_WARNING, "[%s] IQM file does not seem to be valid", fileName);
        fclose(iqmFile);
        return model;
    }

    if (iqm.version != rf_iqm_version)
    {
        RF_LOG(RF_LOG_WARNING, "[%s] IQM file version is not supported (%i).", fileName, iqm.version);
        fclose(iqmFile);
        return model;
    }

    // Meshes data processing
    imesh = (rf_iqm_mesh*) RF_MALLOC(sizeof(rf_iqm_mesh)*iqm.num_meshes);
    fseek(iqmFile, iqm.ofs_meshes, SEEK_SET);
    fread(imesh, sizeof(rf_iqm_mesh)*iqm.num_meshes, 1, iqmFile);

    model.mesh_count = iqm.num_meshes;
    model.meshes = (rf_mesh*) RF_MALLOC(model.mesh_count * sizeof(rf_mesh));
    memset(model.meshes, 0, model.mesh_count * sizeof(rf_mesh));

    char name[rf_mesh_name_length] = { 0 };

    for (int i = 0; i < model.mesh_count; i++)
    {
        fseek(iqmFile, iqm.ofs_text + imesh[i].name, SEEK_SET);
        fread(name, sizeof(char)*rf_mesh_name_length, 1, iqmFile); // rf_mesh name not used...
        model.meshes[i].vertex_count = imesh[i].num_vertexes;

        model.meshes[i].vertices = (float*) RF_MALLOC(model.meshes[i].vertex_count*3 * sizeof(float)); // Default vertex positions
        memset(model.meshes[i].vertices, 0, model.meshes[i].vertex_count*3 * sizeof(float));
        model.meshes[i].normals = (float*) RF_MALLOC(model.meshes[i].vertex_count*3 * sizeof(float)); // Default vertex normals
        memset(model.meshes[i].normals, 0, model.meshes[i].vertex_count*3 * sizeof(float));
        model.meshes[i].texcoords = (float*) RF_MALLOC(model.meshes[i].vertex_count*2 * sizeof(float)); // Default vertex texcoords
        memset(model.meshes[i].texcoords, 0, model.meshes[i].vertex_count*2 * sizeof(float));

        model.meshes[i].bone_ids = (int*) RF_MALLOC(model.meshes[i].vertex_count*4 * sizeof(float)); // Up-to 4 bones supported!
        memset(model.meshes[i].bone_ids, 0, model.meshes[i].vertex_count*4 * sizeof(float));
        model.meshes[i].bone_weights = (float*) RF_MALLOC(model.meshes[i].vertex_count*4 * sizeof(float)); // Up-to 4 bones supported!
        memset(model.meshes[i].bone_weights, 0, model.meshes[i].vertex_count*4 * sizeof(float));

        model.meshes[i].triangle_count = imesh[i].num_triangles;
        model.meshes[i].indices = (unsigned short*) RF_MALLOC(model.meshes[i].triangle_count*3 * sizeof(unsigned short));
        memset(model.meshes[i].indices, 0, model.meshes[i].triangle_count*3 * sizeof(unsigned short));

        // Animated verted data, what we actually process for rendering
        // NOTE: Animated vertex should be re-uploaded to GPU (if not using GPU skinning)
        model.meshes[i].anim_vertices = (float*) RF_MALLOC(model.meshes[i].vertex_count*3 * sizeof(float));
        memset(model.meshes[i].anim_vertices, 0, model.meshes[i].vertex_count*3 * sizeof(float));
        model.meshes[i].anim_normals = (float*) RF_MALLOC(model.meshes[i].vertex_count*3 * sizeof(float));
        memset(model.meshes[i].anim_normals, 0, model.meshes[i].vertex_count*3 * sizeof(float));

        model.meshes[i].vbo_id = (unsigned int*)RF_MALLOC(rf_max_mesh_vbo * sizeof(unsigned int));
        memset(model.meshes[i].vbo_id, 0, rf_max_mesh_vbo * sizeof(unsigned int));
    }

    // Triangles data processing
    tri = (rf_iqm_triangle*) RF_MALLOC(iqm.num_triangles*sizeof(rf_iqm_triangle));
    fseek(iqmFile, iqm.ofs_triangles, SEEK_SET);
    fread(tri, iqm.num_triangles*sizeof(rf_iqm_triangle), 1, iqmFile);

    for (int m = 0; m < model.mesh_count; m++)
    {
        int tcounter = 0;

        for (int i = imesh[m].first_triangle; i < (imesh[m].first_triangle + imesh[m].num_triangles); i++)
        {
            // IQM triangles are stored counter clockwise, but raylib sets opengl to clockwise drawing, so we swap them around
            model.meshes[m].indices[tcounter + 2] = tri[i].vertex[0] - imesh[m].first_vertex;
            model.meshes[m].indices[tcounter + 1] = tri[i].vertex[1] - imesh[m].first_vertex;
            model.meshes[m].indices[tcounter] = tri[i].vertex[2] - imesh[m].first_vertex;
            tcounter += 3;
        }
    }

    // Vertex arrays data processing
    va = (rf_iqm_vertex_array*) RF_MALLOC(iqm.num_vertexarrays*sizeof(rf_iqm_vertex_array));
    fseek(iqmFile, iqm.ofs_vertexarrays, SEEK_SET);
    fread(va, iqm.num_vertexarrays*sizeof(rf_iqm_vertex_array), 1, iqmFile);

    for (int i = 0; i < iqm.num_vertexarrays; i++)
    {
        switch (va[i].type)
        {
            case rf_iqm_position:
            {
                vertex = (float*) RF_MALLOC(iqm.num_vertexes*3*sizeof(float));
                fseek(iqmFile, va[i].offset, SEEK_SET);
                fread(vertex, iqm.num_vertexes*3*sizeof(float), 1, iqmFile);

                for (int m = 0; m < iqm.num_meshes; m++)
                {
                    int vertex_pos_counter = 0;
                    for (int ii = imesh[m].first_vertex * 3; ii < (imesh[m].first_vertex + imesh[m].num_vertexes) * 3; ii++)
                    {
                        model.meshes[m].vertices[vertex_pos_counter] = vertex[ii];
                        model.meshes[m].anim_vertices[vertex_pos_counter] = vertex[ii];
                        vertex_pos_counter++;
                    }
                }
            } break;
            case rf_iqm_normal:
            {
                normal = (float*) RF_MALLOC(iqm.num_vertexes*3*sizeof(float));
                fseek(iqmFile, va[i].offset, SEEK_SET);
                fread(normal, iqm.num_vertexes*3*sizeof(float), 1, iqmFile);

                for (int m = 0; m < iqm.num_meshes; m++)
                {
                    int vertex_pos_counter = 0;
                    for (int ii = imesh[m].first_vertex * 3; ii < (imesh[m].first_vertex + imesh[m].num_vertexes) * 3; ii++)
                    {
                        model.meshes[m].normals[vertex_pos_counter] = normal[ii];
                        model.meshes[m].anim_normals[vertex_pos_counter] = normal[ii];
                        vertex_pos_counter++;
                    }
                }
            } break;
            case rf_iqm_texcoord:
            {
                text = (float*) RF_MALLOC(iqm.num_vertexes*2*sizeof(float));
                fseek(iqmFile, va[i].offset, SEEK_SET);
                fread(text, iqm.num_vertexes*2*sizeof(float), 1, iqmFile);

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
            case rf_iqm_blendindexes:
            {
                blendi = (char*) RF_MALLOC(iqm.num_vertexes*4*sizeof(char));
                fseek(iqmFile, va[i].offset, SEEK_SET);
                fread(blendi, iqm.num_vertexes*4*sizeof(char), 1, iqmFile);

                for (int m = 0; m < iqm.num_meshes; m++)
                {
                    int boneCounter = 0;
                    for (int ii = imesh[m].first_vertex * 4; ii < (imesh[m].first_vertex + imesh[m].num_vertexes) * 4; ii++)
                    {
                        model.meshes[m].bone_ids[boneCounter] = blendi[ii];
                        boneCounter++;
                    }
                }
            } break;
            case rf_iqm_blendweights:
            {
                blendw = (unsigned char*) RF_MALLOC(iqm.num_vertexes*4*sizeof(unsigned char));
                fseek(iqmFile, va[i].offset, SEEK_SET);
                fread(blendw, iqm.num_vertexes*4*sizeof(unsigned char), 1, iqmFile);

                for (int m = 0; m < iqm.num_meshes; m++)
                {
                    int boneCounter = 0;
                    for (int ii = imesh[m].first_vertex * 4; ii < (imesh[m].first_vertex + imesh[m].num_vertexes) * 4; ii++)
                    {
                        model.meshes[m].bone_weights[boneCounter] = blendw[ii] / 255.0f;
                        boneCounter++;
                    }
                }
            } break;
        }
    }

    // Bones (joints) data processing
    ijoint = (rf_iqm_joint*) RF_MALLOC(iqm.num_joints*sizeof(rf_iqm_joint));
    fseek(iqmFile, iqm.ofs_joints, SEEK_SET);
    fread(ijoint, iqm.num_joints*sizeof(rf_iqm_joint), 1, iqmFile);

    model.bone_count = iqm.num_joints;
    model.bones = (rf_bone_info*) RF_MALLOC(iqm.num_joints*sizeof(rf_bone_info));
    model.bind_pose = (rf_transform*) RF_MALLOC(iqm.num_joints*sizeof(rf_transform));

    for (int i = 0; i < iqm.num_joints; i++)
    {
        // Bones
        model.bones[i].parent = ijoint[i].parent;
        fseek(iqmFile, iqm.ofs_text + ijoint[i].name, SEEK_SET);
        fread(model.bones[i].name, rf_bone_name_length*sizeof(char), 1, iqmFile);

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
            model.bind_pose[i].rotation = rf_quaternion_multiply(model.bind_pose[model.bones[i].parent].rotation, model.bind_pose[i].rotation);
            model.bind_pose[i].translation = rf_vector3_rotate_by_quaternion(model.bind_pose[i].translation, model.bind_pose[model.bones[i].parent].rotation);
            model.bind_pose[i].translation = rf_vector3_add(model.bind_pose[i].translation, model.bind_pose[model.bones[i].parent].translation);
            model.bind_pose[i].scale = rf_vector3_multiply_v(model.bind_pose[i].scale, model.bind_pose[model.bones[i].parent].scale);
        }
    }

    fclose(iqmFile);
    RF_FREE(imesh);
    RF_FREE(tri);
    RF_FREE(va);
    RF_FREE(vertex);
    RF_FREE(normal);
    RF_FREE(text);
    RF_FREE(blendi);
    RF_FREE(blendw);
    RF_FREE(ijoint);

    return model;
}

RF_INTERNAL const unsigned char rf_base64_table[] = {
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

RF_INTERNAL int _rf_get_size_base64(char* input)
{
    int size = 0;

    for (int i = 0; input[4*i] != 0; i++)
    {
        if (input[4*i + 3] == '=')
        {
            if (input[4*i + 2] == '=') size += 1;
            else size += 2;
        }
        else size += 3;
    }

    return size;
}

RF_INTERNAL unsigned char* _rf_decode_base64(char* input, int* size)
{
    *size = _rf_get_size_base64(input);

    unsigned char* buf = (unsigned char* )RF_MALLOC(*size);
    for (int i = 0; i < *size/3; i++)
    {
        unsigned char a = rf_base64_table[(int)input[4*i]];
        unsigned char b = rf_base64_table[(int)input[4*i + 1]];
        unsigned char c = rf_base64_table[(int)input[4*i + 2]];
        unsigned char d = rf_base64_table[(int)input[4*i + 3]];

        buf[3*i] = (a << 2) | (b >> 4);
        buf[3*i + 1] = (b << 4) | (c >> 2);
        buf[3*i + 2] = (c << 6) | d;
    }

    if (*size%3 == 1)
    {
        int n = *size/3;
        unsigned char a = rf_base64_table[(int)input[4*n]];
        unsigned char b = rf_base64_table[(int)input[4*n + 1]];
        buf[*size - 1] = (a << 2) | (b >> 4);
    }
    else if (*size%3 == 2)
    {
        int n = *size/3;
        unsigned char a = rf_base64_table[(int)input[4*n]];
        unsigned char b = rf_base64_table[(int)input[4*n + 1]];
        unsigned char c = rf_base64_table[(int)input[4*n + 2]];
        buf[*size - 2] = (a << 2) | (b >> 4);
        buf[*size - 1] = (b << 4) | (c >> 2);
    }
    return buf;
}

// Load texture from cgltf_image
RF_INTERNAL rf_texture _rf_load_texture_from_cgltf_image(cgltf_image *image, const char* texPath, rf_color tint)
{
    rf_texture texture = { 0 };

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
                int size;
                unsigned char* data = _rf_decode_base64(image->uri + i + 1, &size);

                int w, h;
                unsigned char* raw = stbi_load_from_memory(data, size, &w, &h, NULL, 4);

                rf_image rimage = rf_load_image_pro(raw, w, h, rf_uncompressed_r8g8b8a8);

                // TODO: Tint shouldn't be applied here!
                rf_image_color_tint(&rimage, tint);
                texture = rf_load_texture_from_image(rimage);
                rf_unload_image(rimage);
            }
        }
        else
        {
            char buff[1024];
            snprintf(buff, 1024, "%s/%s", texPath, image->uri);
            rf_image rimage = rf_load_image(buff);

            // TODO: Tint shouldn't be applied here!
            rf_image_color_tint(&rimage, tint);
            texture = rf_load_texture_from_image(rimage);
            rf_unload_image(rimage);
        }
    }
    else if (image->buffer_view)
    {
        unsigned char* data = (unsigned char*) RF_MALLOC(image->buffer_view->size);
        int n = image->buffer_view->offset;
        int stride = image->buffer_view->stride ? image->buffer_view->stride : 1;

        for (int i = 0; i < image->buffer_view->size; i++)
        {
            data[i] = ((unsigned char* )image->buffer_view->buffer->data)[n];
            n += stride;
        }

        int w, h;
        unsigned char* raw = stbi_load_from_memory(data, image->buffer_view->size, &w, &h, NULL, 4);
        free(data);

        rf_image rimage = rf_load_image_pro(raw, w, h, rf_uncompressed_r8g8b8a8);
        free(raw);

        // TODO: Tint shouldn't be applied here!
        rf_image_color_tint(&rimage, tint);
        texture = rf_load_texture_from_image(rimage);
        rf_unload_image(rimage);
    }
    else
    {
        rf_image rimage = rf_load_image_ex(&tint, 1, 1);
        texture = rf_load_texture_from_image(rimage);
        rf_unload_image(rimage);
    }

    return texture;
}

// Load glTF mesh data
RF_INTERNAL rf_model _rf_load_gltf(const char* fileName)
{
    /***********************************************************************************
        Function implemented by Wilhem Barbier (@wbrbr)

        Features:
          - Supports .gltf and .glb files
          - Supports embedded (base64) or external textures
          - Loads the albedo/diffuse texture (other maps could be added)
          - Supports multiple mesh per model and multiple primitives per model

        Some restrictions (not exhaustive):
          - Triangle-only meshes
          - Not supported node hierarchies or transforms
          - Only loads the diffuse texture... but not too hard to support other maps (normal, roughness/metalness...)
          - Only supports unsigned short indices (no rf_byte/unsigned int)
          - Only supports float for texture coordinates (no rf_byte/unsigned short)
    *************************************************************************************/

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

    rf_model model = { 0 };

    // glTF file loading
    FILE* gltfFile = fopen(fileName, "rb");

    if (gltfFile == NULL)
    {
        RF_LOG(RF_LOG_WARNING, "[%s] glTF file could not be opened", fileName);
        return model;
    }

    fseek(gltfFile, 0, SEEK_END);
    int size = ftell(gltfFile);
    fseek(gltfFile, 0, SEEK_SET);

    void* buffer = RF_MALLOC(size);
    fread(buffer, size, 1, gltfFile);

    fclose(gltfFile);

    // glTF data loading
    cgltf_options options = { cgltf_file_type_invalid };
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse(&options, buffer, size, &data);

    if (result == cgltf_result_success)
    {
        RF_LOG(RF_LOG_INFO, "[%s][%s] rf_model meshes/materials: %i/%i", fileName, (data->file_type == 2)? "glb" : "gltf", data->meshes_count, data->materials_count);

        // Read data buffers
        result = cgltf_load_buffers(&options, data, fileName);
        if (result != cgltf_result_success) RF_LOG(RF_LOG_INFO, "[%s][%s] Error loading mesh/material buffers", fileName, (data->file_type == 2)? "glb" : "gltf");

        int primitivesCount = 0;

        for (int i = 0; i < data->meshes_count; i++) primitivesCount += (int)data->meshes[i].primitives_count;

        // Process glTF data and map to model
        model.mesh_count = primitivesCount;
        model.meshes = (rf_mesh*) RF_MALLOC(model.mesh_count * sizeof(rf_mesh));
        memset(model.meshes, 0, model.mesh_count * sizeof(rf_mesh));
        model.material_count = data->materials_count + 1;
        model.materials = (rf_material*) RF_MALLOC(model.material_count*sizeof(rf_material));
        model.mesh_material = (int*) RF_MALLOC(model.mesh_count*sizeof(int));

        for (int i = 0; i < model.mesh_count; i++)
        {
            model.meshes[i].vbo_id = (unsigned int* )RF_MALLOC(rf_max_mesh_vbo * sizeof(unsigned int));
            memset(model.meshes[i].vbo_id, 0, rf_max_mesh_vbo * sizeof(unsigned int));
        }

        //For each material
        for (int i = 0; i < model.material_count - 1; i++)
        {
            model.materials[i] = rf_load_material_default();
            rf_color tint = RF_CLITERAL(rf_color){ 255, 255, 255, 255 };
            const char* texPath = _rf_get_directory_path(fileName);

            //Ensure material follows raylib support for PBR (metallic/roughness flow)
            if (data->materials[i].has_pbr_metallic_roughness)
            {
                float roughness = data->materials[i].pbr_metallic_roughness.roughness_factor;
                float metallic = data->materials[i].pbr_metallic_roughness.metallic_factor;

                // NOTE: rf_material name not used for the moment
                //if (model.materials[i].name && data->materials[i].name) strcpy(model.materials[i].name, data->materials[i].name);

                // TODO: REview: shouldn't these be *255 ???
                tint.r = (unsigned char)(data->materials[i].pbr_metallic_roughness.base_color_factor[0]*255);
                tint.g = (unsigned char)(data->materials[i].pbr_metallic_roughness.base_color_factor[1]*255);
                tint.b = (unsigned char)(data->materials[i].pbr_metallic_roughness.base_color_factor[2]*255);
                tint.a = (unsigned char)(data->materials[i].pbr_metallic_roughness.base_color_factor[3]*255);

                model.materials[i].maps[rf_map_roughness].color = tint;

                if (data->materials[i].pbr_metallic_roughness.base_color_texture.texture)
                {
                    model.materials[i].maps[rf_map_albedo].texture = _rf_load_texture_from_cgltf_image(data->materials[i].pbr_metallic_roughness.base_color_texture.texture->image, texPath, tint);
                }

                // NOTE: Tint isn't need for other textures.. pass null or clear?
                // Just set as white, multiplying by white has no effect
                tint = rf_white;

                if (data->materials[i].pbr_metallic_roughness.metallic_roughness_texture.texture)
                {
                    model.materials[i].maps[rf_map_roughness].texture = _rf_load_texture_from_cgltf_image(data->materials[i].pbr_metallic_roughness.metallic_roughness_texture.texture->image, texPath, tint);
                }
                model.materials[i].maps[rf_map_roughness].value = roughness;
                model.materials[i].maps[rf_map_metalness].value = metallic;

                if (data->materials[i].normal_texture.texture)
                {
                    model.materials[i].maps[rf_map_normal].texture = _rf_load_texture_from_cgltf_image(data->materials[i].normal_texture.texture->image, texPath, tint);
                }

                if (data->materials[i].occlusion_texture.texture)
                {
                    model.materials[i].maps[rf_map_occlusion].texture = _rf_load_texture_from_cgltf_image(data->materials[i].occlusion_texture.texture->image, texPath, tint);
                }
            }
        }

        model.materials[model.material_count - 1] = rf_load_material_default();

        int primitiveIndex = 0;

        for (int i = 0; i < data->meshes_count; i++)
        {
            for (int p = 0; p < data->meshes[i].primitives_count; p++)
            {
                for (int j = 0; j < data->meshes[i].primitives[p].attributes_count; j++)
                {
                    if (data->meshes[i].primitives[p].attributes[j].type == cgltf_attribute_type_position)
                    {
                        cgltf_accessor *acc = data->meshes[i].primitives[p].attributes[j].data;
                        model.meshes[primitiveIndex].vertex_count = acc->count;
                        model.meshes[primitiveIndex].vertices = (float*) RF_MALLOC(sizeof(float)*model.meshes[primitiveIndex].vertex_count*3);

                        rf_load_accessor(float, 3, acc, model.meshes[primitiveIndex].vertices)
                    }
                    else if (data->meshes[i].primitives[p].attributes[j].type == cgltf_attribute_type_normal)
                    {
                        cgltf_accessor *acc = data->meshes[i].primitives[p].attributes[j].data;
                        model.meshes[primitiveIndex].normals = (float*) RF_MALLOC(sizeof(float)*acc->count*3);

                        rf_load_accessor(float, 3, acc, model.meshes[primitiveIndex].normals)
                    }
                    else if (data->meshes[i].primitives[p].attributes[j].type == cgltf_attribute_type_texcoord)
                    {
                        cgltf_accessor *acc = data->meshes[i].primitives[p].attributes[j].data;

                        if (acc->component_type == cgltf_component_type_r_32f)
                        {
                            model.meshes[primitiveIndex].texcoords = (float*) RF_MALLOC(sizeof(float)*acc->count*2);
                            rf_load_accessor(float, 2, acc, model.meshes[primitiveIndex].texcoords)
                        }
                        else
                        {
                            // TODO: Support normalized unsigned rf_byte/unsigned short texture coordinates
                            RF_LOG(RF_LOG_WARNING, "[%s] rf_texture coordinates must be float", fileName);
                        }
                    }
                }

                cgltf_accessor *acc = data->meshes[i].primitives[p].indices;

                if (acc)
                {
                    if (acc->component_type == cgltf_component_type_r_16u)
                    {
                        model.meshes[primitiveIndex].triangle_count = acc->count/3;
                        model.meshes[primitiveIndex].indices = (unsigned short*) RF_MALLOC(sizeof(unsigned short)*model.meshes[primitiveIndex].triangle_count*3);
                        rf_load_accessor(unsigned short, 1, acc, model.meshes[primitiveIndex].indices)
                    }
                    else
                    {
                        // TODO: Support unsigned rf_byte/unsigned int
                        RF_LOG(RF_LOG_WARNING, "[%s] Indices must be unsigned short", fileName);
                    }
                }
                else
                {
                    // Unindexed mesh
                    model.meshes[primitiveIndex].triangle_count = model.meshes[primitiveIndex].vertex_count/3;
                }

                if (data->meshes[i].primitives[p].material)
                {
                    // Compute the offset
                    model.mesh_material[primitiveIndex] = data->meshes[i].primitives[p].material - data->materials;
                }
                else
                {
                    model.mesh_material[primitiveIndex] = model.material_count - 1;
                }

                primitiveIndex++;
            }
        }

        cgltf_free(data);
    }
    else RF_LOG(RF_LOG_WARNING, "[%s] glTF data could not be loaded", fileName);

    RF_FREE(buffer);

    return model;
}

//endregion

//region shapes

RF_INTERNAL float _rf_shapes_ease_cubic_in_out(float t, float b, float c, float d); // Cubic easing
RF_INTERNAL rf_texture2d _rf_get_shapes_texture(); // Get texture to draw shapes

// Draw a pixel
RF_API void rf_draw_pixel(int posX, int posY, rf_color color)
{
    rf_gl_begin(GL_LINES);
    rf_gl_color4ub(color.r, color.g, color.b, color.a);
    rf_gl_vertex2i(posX, posY);
    rf_gl_vertex2i(posX + 1, posY + 1);
    rf_gl_end();
}

// Draw a pixel (Vector version)
RF_API void rf_draw_pixel_v(rf_vector2 position, rf_color color)
{
    rf_gl_begin(GL_LINES);
    rf_gl_color4ub(color.r, color.g, color.b, color.a);
    rf_gl_vertex2f(position.x, position.y);
    rf_gl_vertex2f(position.x + 1.0f, position.y + 1.0f);
    rf_gl_end();
}

// Draw a line
RF_API void rf_draw_line(int startPosX, int startPosY, int endPosX, int endPosY, rf_color color)
{
    rf_gl_begin(GL_LINES);
    rf_gl_color4ub(color.r, color.g, color.b, color.a);
    rf_gl_vertex2i(startPosX, startPosY);
    rf_gl_vertex2i(endPosX, endPosY);
    rf_gl_end();
}

// Draw a line  (Vector version)
RF_API void rf_draw_line_v(rf_vector2 startPos, rf_vector2 endPos, rf_color color)
{
    rf_gl_begin(GL_LINES);
    rf_gl_color4ub(color.r, color.g, color.b, color.a);
    rf_gl_vertex2f(startPos.x, startPos.y);
    rf_gl_vertex2f(endPos.x, endPos.y);
    rf_gl_end();
}

// Draw a line defining thickness
RF_API void rf_draw_line_ex(rf_vector2 startPos, rf_vector2 endPos, float thick, rf_color color)
{
    if (startPos.x > endPos.x)
    {
        rf_vector2 tempPos = startPos;
        startPos = endPos;
        endPos = tempPos;
    }

    float dx = endPos.x - startPos.x;
    float dy = endPos.y - startPos.y;

    float d = sqrtf(dx*dx + dy*dy);
    float angle = asinf(dy/d);

    rf_gl_enable_texture(_rf_get_shapes_texture().id);

    rf_push_matrix();
    rf_translatef((float)startPos.x, (float)startPos.y, 0.0f);
    rf_rotatef(RF_RAD2DEG * angle, 0.0f, 0.0f, 1.0f);
    rf_translatef(0, (thick > 1.0f)? -thick/2.0f : -1.0f, 0.0f);

    rf_gl_begin(GL_QUADS);
    rf_gl_color4ub(color.r, color.g, color.b, color.a);
    rf_gl_normal3f(0.0f, 0.0f, 1.0f);

    rf_gl_tex_coord2f(_rf_global_context_ptr->gl_ctx.rec_tex_shapes.x/_rf_global_context_ptr->gl_ctx.tex_shapes.width, _rf_global_context_ptr->gl_ctx.rec_tex_shapes.y/_rf_global_context_ptr->gl_ctx.tex_shapes.height);
    rf_gl_vertex2f(0.0f, 0.0f);

    rf_gl_tex_coord2f(_rf_global_context_ptr->gl_ctx.rec_tex_shapes.x/_rf_global_context_ptr->gl_ctx.tex_shapes.width, (_rf_global_context_ptr->gl_ctx.rec_tex_shapes.y + _rf_global_context_ptr->gl_ctx.rec_tex_shapes.height)/_rf_global_context_ptr->gl_ctx.tex_shapes.height);
    rf_gl_vertex2f(0.0f, thick);

    rf_gl_tex_coord2f((_rf_global_context_ptr->gl_ctx.rec_tex_shapes.x + _rf_global_context_ptr->gl_ctx.rec_tex_shapes.width)/_rf_global_context_ptr->gl_ctx.tex_shapes.width, (_rf_global_context_ptr->gl_ctx.rec_tex_shapes.y + _rf_global_context_ptr->gl_ctx.rec_tex_shapes.height)/_rf_global_context_ptr->gl_ctx.tex_shapes.height);
    rf_gl_vertex2f(d, thick);

    rf_gl_tex_coord2f((_rf_global_context_ptr->gl_ctx.rec_tex_shapes.x + _rf_global_context_ptr->gl_ctx.rec_tex_shapes.width)/_rf_global_context_ptr->gl_ctx.tex_shapes.width, _rf_global_context_ptr->gl_ctx.rec_tex_shapes.y/_rf_global_context_ptr->gl_ctx.tex_shapes.height);
    rf_gl_vertex2f(d, 0.0f);
    rf_gl_end();
    rf_pop_matrix();

    rf_gl_disable_texture();
}

// Draw line using cubic-bezier curves in-out
RF_API void rf_draw_line_bezier(rf_vector2 startPos, rf_vector2 endPos, float thick, rf_color color)
{
#define rf_line_divisions 24 // Bezier line divisions

    rf_vector2 previous = startPos;
    rf_vector2 current;

    for (int i = 1; i <= rf_line_divisions; i++)
    {
        // Cubic easing in-out
        // NOTE: Easing is calculated only for y position value
        current.y = _rf_shapes_ease_cubic_in_out((float)i, startPos.y, endPos.y - startPos.y, (float)rf_line_divisions);
        current.x = previous.x + (endPos.x - startPos.x)/ (float)rf_line_divisions;

        rf_draw_line_ex(previous, current, thick, color);

        previous = current;
    }
}

// Draw lines sequence
RF_API void rf_draw_line_strip(rf_vector2 *points, int pointsCount, rf_color color)
{
    if (pointsCount >= 2)
    {
        if (rf_gl_check_buffer_limit(pointsCount)) rf_gl_draw();

        rf_gl_begin(GL_LINES);
        rf_gl_color4ub(color.r, color.g, color.b, color.a);

        for (int i = 0; i < pointsCount - 1; i++)
        {
            rf_gl_vertex2f(points[i].x, points[i].y);
            rf_gl_vertex2f(points[i + 1].x, points[i + 1].y);
        }
        rf_gl_end();
    }
}

// Draw a color-filled circle
RF_API void rf_draw_circle(int centerX, int centerY, float radius, rf_color color)
{
    rf_draw_circle_v((rf_vector2){ (float)centerX, (float)centerY }, radius, color);
}

// Draw a piece of a circle
RF_API void rf_draw_circle_sector(rf_vector2 center, float radius, int startAngle, int endAngle, int segments, rf_color color)
{
    if (radius <= 0.0f) radius = 0.1f; // Avoid div by zero

    // Function expects (endAngle > startAngle)
    if (endAngle < startAngle)
    {
        // Swap values
        int tmp = startAngle;
        startAngle = endAngle;
        endAngle = tmp;
    }

    if (segments < 4)
    {
        // Calculate how many segments we need to draw a smooth circle, taken from https://stackoverflow.com/a/2244088
        float CIRCLE_ERROR_RATE = 0.5f;

        // Calculate the maximum angle between segments based on the error rate.
        float th = acosf(2*powf(1 - CIRCLE_ERROR_RATE/radius, 2) - 1);
        segments = (endAngle - startAngle) * ceilf(2 * RF_PI / th) / 360;

        if (segments <= 0) segments = 4;
    }

    float stepLength = (float)(endAngle - startAngle)/(float)segments;
    float angle = startAngle;
    if (rf_gl_check_buffer_limit(3*segments)) rf_gl_draw();

    rf_gl_begin(GL_TRIANGLES);
    for (int i = 0; i < segments; i++)
    {
        rf_gl_color4ub(color.r, color.g, color.b, color.a);

        rf_gl_vertex2f(center.x, center.y);
        rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*radius, center.y + cosf(RF_DEG2RAD*angle)*radius);
        rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + stepLength))*radius, center.y + cosf(RF_DEG2RAD*(angle + stepLength))*radius);

        angle += stepLength;
    }
    rf_gl_end();

}

RF_API void rf_draw_circle_sector_lines(rf_vector2 center, float radius, int startAngle, int endAngle, int segments, rf_color color)
{
    if (radius <= 0.0f) radius = 0.1f; // Avoid div by zero issue

    // Function expects (endAngle > startAngle)
    if (endAngle < startAngle)
    {
        // Swap values
        int tmp = startAngle;
        startAngle = endAngle;
        endAngle = tmp;
    }

    if (segments < 4)
    {
        // Calculate how many segments we need to draw a smooth circle, taken from https://stackoverflow.com/a/2244088

        float CIRCLE_ERROR_RATE = 0.5f;


        // Calculate the maximum angle between segments based on the error rate.
        float th = acosf(2*powf(1 - CIRCLE_ERROR_RATE/radius, 2) - 1);
        segments = (endAngle - startAngle) * ceilf(2 * RF_PI / th) / 360;

        if (segments <= 0) segments = 4;
    }

    float stepLength = (float)(endAngle - startAngle)/(float)segments;
    float angle = startAngle;

    // Hide the cap lines when the circle is full
    bool showCapLines = true;
    int limit = 2*(segments + 2);
    if ((endAngle - startAngle)%360 == 0) { limit = 2*segments; showCapLines = false; }

    if (rf_gl_check_buffer_limit(limit)) rf_gl_draw();

    rf_gl_begin(GL_LINES);
    if (showCapLines)
    {
        rf_gl_color4ub(color.r, color.g, color.b, color.a);
        rf_gl_vertex2f(center.x, center.y);
        rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*radius, center.y + cosf(RF_DEG2RAD*angle)*radius);
    }

    for (int i = 0; i < segments; i++)
    {
        rf_gl_color4ub(color.r, color.g, color.b, color.a);

        rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*radius, center.y + cosf(RF_DEG2RAD*angle)*radius);
        rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + stepLength))*radius, center.y + cosf(RF_DEG2RAD*(angle + stepLength))*radius);

        angle += stepLength;
    }

    if (showCapLines)
    {
        rf_gl_color4ub(color.r, color.g, color.b, color.a);
        rf_gl_vertex2f(center.x, center.y);
        rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*radius, center.y + cosf(RF_DEG2RAD*angle)*radius);
    }
    rf_gl_end();
}

// Draw a gradient-filled circle
// NOTE: Gradient goes from center (color1) to border (color2)
RF_API void rf_draw_circle_gradient(int centerX, int centerY, float radius, rf_color color1, rf_color color2)
{
    if (rf_gl_check_buffer_limit(3*36)) rf_gl_draw();

    rf_gl_begin(GL_TRIANGLES);
    for (int i = 0; i < 360; i += 10)
    {
        rf_gl_color4ub(color1.r, color1.g, color1.b, color1.a);
        rf_gl_vertex2f(centerX, centerY);
        rf_gl_color4ub(color2.r, color2.g, color2.b, color2.a);
        rf_gl_vertex2f(centerX + sinf(RF_DEG2RAD*i)*radius, centerY + cosf(RF_DEG2RAD*i)*radius);
        rf_gl_color4ub(color2.r, color2.g, color2.b, color2.a);
        rf_gl_vertex2f(centerX + sinf(RF_DEG2RAD*(i + 10))*radius, centerY + cosf(RF_DEG2RAD*(i + 10))*radius);
    }
    rf_gl_end();
}

// Draw a color-filled circle (Vector version)
// NOTE: On OpenGL 3.3 and ES2 we use QUADS to avoid drawing order issues (view rf_gl_draw)
RF_API void rf_draw_circle_v(rf_vector2 center, float radius, rf_color color)
{
    rf_draw_circle_sector(center, radius, 0, 360, 36, color);
}

// Draw circle outline
RF_API void rf_draw_circle_lines(int centerX, int centerY, float radius, rf_color color)
{
    if (rf_gl_check_buffer_limit(2*36)) rf_gl_draw();

    rf_gl_begin(GL_LINES);
    rf_gl_color4ub(color.r, color.g, color.b, color.a);

    // NOTE: Circle outline is drawn pixel by pixel every degree (0 to 360)
    for (int i = 0; i < 360; i += 10)
    {
        rf_gl_vertex2f(centerX + sinf(RF_DEG2RAD*i)*radius, centerY + cosf(RF_DEG2RAD*i)*radius);
        rf_gl_vertex2f(centerX + sinf(RF_DEG2RAD*(i + 10))*radius, centerY + cosf(RF_DEG2RAD*(i + 10))*radius);
    }
    rf_gl_end();
}

RF_API void rf_draw_ring(rf_vector2 center, float innerRadius, float outerRadius, int startAngle, int endAngle, int segments, rf_color color)
{
    if (startAngle == endAngle) return;

    // Function expects (outerRadius > innerRadius)
    if (outerRadius < innerRadius)
    {
        float tmp = outerRadius;
        outerRadius = innerRadius;
        innerRadius = tmp;

        if (outerRadius <= 0.0f) outerRadius = 0.1f;
    }

    // Function expects (endAngle > startAngle)
    if (endAngle < startAngle)
    {
        // Swap values
        int tmp = startAngle;
        startAngle = endAngle;
        endAngle = tmp;
    }

    if (segments < 4)
    {
        // Calculate how many segments we need to draw a smooth circle, taken from https://stackoverflow.com/a/2244088

        float CIRCLE_ERROR_RATE = 0.5f;


        // Calculate the maximum angle between segments based on the error rate.
        float th = acosf(2*powf(1 - CIRCLE_ERROR_RATE/outerRadius, 2) - 1);
        segments = (endAngle - startAngle) * ceilf(2 * RF_PI / th) / 360;

        if (segments <= 0) segments = 4;
    }

    // Not a ring
    if (innerRadius <= 0.0f)
    {
        rf_draw_circle_sector(center, outerRadius, startAngle, endAngle, segments, color);
        return;
    }

    float stepLength = (float)(endAngle - startAngle)/(float)segments;
    float angle = startAngle;
    if (rf_gl_check_buffer_limit(6*segments)) rf_gl_draw();

    rf_gl_begin(GL_TRIANGLES);
    for (int i = 0; i < segments; i++)
    {
        rf_gl_color4ub(color.r, color.g, color.b, color.a);

        rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*innerRadius, center.y + cosf(RF_DEG2RAD*angle)*innerRadius);
        rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*outerRadius, center.y + cosf(RF_DEG2RAD*angle)*outerRadius);
        rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + stepLength))*innerRadius, center.y + cosf(RF_DEG2RAD*(angle + stepLength))*innerRadius);

        rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + stepLength))*innerRadius, center.y + cosf(RF_DEG2RAD*(angle + stepLength))*innerRadius);
        rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*outerRadius, center.y + cosf(RF_DEG2RAD*angle)*outerRadius);
        rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + stepLength))*outerRadius, center.y + cosf(RF_DEG2RAD*(angle + stepLength))*outerRadius);

        angle += stepLength;
    }
    rf_gl_end();

}

RF_API void rf_draw_ring_lines(rf_vector2 center, float innerRadius, float outerRadius, int startAngle, int endAngle, int segments, rf_color color)
{
    if (startAngle == endAngle) return;

    // Function expects (outerRadius > innerRadius)
    if (outerRadius < innerRadius)
    {
        float tmp = outerRadius;
        outerRadius = innerRadius;
        innerRadius = tmp;

        if (outerRadius <= 0.0f) outerRadius = 0.1f;
    }

    // Function expects (endAngle > startAngle)
    if (endAngle < startAngle)
    {
        // Swap values
        int tmp = startAngle;
        startAngle = endAngle;
        endAngle = tmp;
    }

    if (segments < 4)
    {
        // Calculate how many segments we need to draw a smooth circle, taken from https://stackoverflow.com/a/2244088

        float CIRCLE_ERROR_RATE = 0.5f;

        // Calculate the maximum angle between segments based on the error rate.
        float th = acosf(2*powf(1 - CIRCLE_ERROR_RATE/outerRadius, 2) - 1);
        segments = (endAngle - startAngle) * ceilf(2 * RF_PI / th) / 360;

        if (segments <= 0) segments = 4;
    }

    if (innerRadius <= 0.0f)
    {
        rf_draw_circle_sector_lines(center, outerRadius, startAngle, endAngle, segments, color);
        return;
    }

    float stepLength = (float)(endAngle - startAngle)/(float)segments;
    float angle = startAngle;

    bool showCapLines = true;
    int limit = 4*(segments + 1);
    if ((endAngle - startAngle)%360 == 0) { limit = 4*segments; showCapLines = false; }

    if (rf_gl_check_buffer_limit(limit)) rf_gl_draw();

    rf_gl_begin(GL_LINES);
    if (showCapLines)
    {
        rf_gl_color4ub(color.r, color.g, color.b, color.a);
        rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*outerRadius, center.y + cosf(RF_DEG2RAD*angle)*outerRadius);
        rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*innerRadius, center.y + cosf(RF_DEG2RAD*angle)*innerRadius);
    }

    for (int i = 0; i < segments; i++)
    {
        rf_gl_color4ub(color.r, color.g, color.b, color.a);

        rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*outerRadius, center.y + cosf(RF_DEG2RAD*angle)*outerRadius);
        rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + stepLength))*outerRadius, center.y + cosf(RF_DEG2RAD*(angle + stepLength))*outerRadius);

        rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*innerRadius, center.y + cosf(RF_DEG2RAD*angle)*innerRadius);
        rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + stepLength))*innerRadius, center.y + cosf(RF_DEG2RAD*(angle + stepLength))*innerRadius);

        angle += stepLength;
    }

    if (showCapLines)
    {
        rf_gl_color4ub(color.r, color.g, color.b, color.a);
        rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*outerRadius, center.y + cosf(RF_DEG2RAD*angle)*outerRadius);
        rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*innerRadius, center.y + cosf(RF_DEG2RAD*angle)*innerRadius);
    }
    rf_gl_end();
}

// Draw a color-filled rectangle
RF_API void rf_draw_rectangle(int posX, int posY, int width, int height, rf_color color)
{
    rf_draw_rectangle_v((rf_vector2){ (float)posX, (float)posY }, RF_CLITERAL(rf_vector2){ (float)width, (float)height }, color);
}

// Draw a color-filled rectangle (Vector version)
// NOTE: On OpenGL 3.3 and ES2 we use QUADS to avoid drawing order issues (view rf_gl_draw)
RF_API void rf_draw_rectangle_v(rf_vector2 position, rf_vector2 size, rf_color color)
{
    rf_draw_rectangle_pro((rf_rectangle){ position.x, position.y, size.x, size.y }, RF_CLITERAL(rf_vector2){ 0.0f, 0.0f }, 0.0f, color);
}

// Draw a color-filled rectangle
RF_API void rf_draw_rectangle_rec(rf_rectangle rec, rf_color color)
{
    rf_draw_rectangle_pro(rec, RF_CLITERAL(rf_vector2){ 0.0f, 0.0f }, 0.0f, color);
}

// Draw a color-filled rectangle with pro parameters
RF_API void rf_draw_rectangle_pro(rf_rectangle rec, rf_vector2 origin, float rotation, rf_color color)
{
    rf_gl_enable_texture(_rf_get_shapes_texture().id);

    rf_push_matrix();
    rf_translatef(rec.x, rec.y, 0.0f);
    rf_rotatef(rotation, 0.0f, 0.0f, 1.0f);
    rf_translatef(-origin.x, -origin.y, 0.0f);

    rf_gl_begin(GL_QUADS);
    rf_gl_normal3f(0.0f, 0.0f, 1.0f);
    rf_gl_color4ub(color.r, color.g, color.b, color.a);

    rf_gl_tex_coord2f(_rf_global_context_ptr->gl_ctx.rec_tex_shapes.x/_rf_global_context_ptr->gl_ctx.tex_shapes.width, _rf_global_context_ptr->gl_ctx.rec_tex_shapes.y/_rf_global_context_ptr->gl_ctx.tex_shapes.height);
    rf_gl_vertex2f(0.0f, 0.0f);

    rf_gl_tex_coord2f(_rf_global_context_ptr->gl_ctx.rec_tex_shapes.x/_rf_global_context_ptr->gl_ctx.tex_shapes.width, (_rf_global_context_ptr->gl_ctx.rec_tex_shapes.y + _rf_global_context_ptr->gl_ctx.rec_tex_shapes.height)/_rf_global_context_ptr->gl_ctx.tex_shapes.height);
    rf_gl_vertex2f(0.0f, rec.height);

    rf_gl_tex_coord2f((_rf_global_context_ptr->gl_ctx.rec_tex_shapes.x + _rf_global_context_ptr->gl_ctx.rec_tex_shapes.width)/_rf_global_context_ptr->gl_ctx.tex_shapes.width, (_rf_global_context_ptr->gl_ctx.rec_tex_shapes.y + _rf_global_context_ptr->gl_ctx.rec_tex_shapes.height)/_rf_global_context_ptr->gl_ctx.tex_shapes.height);
    rf_gl_vertex2f(rec.width, rec.height);

    rf_gl_tex_coord2f((_rf_global_context_ptr->gl_ctx.rec_tex_shapes.x + _rf_global_context_ptr->gl_ctx.rec_tex_shapes.width)/_rf_global_context_ptr->gl_ctx.tex_shapes.width, _rf_global_context_ptr->gl_ctx.rec_tex_shapes.y/_rf_global_context_ptr->gl_ctx.tex_shapes.height);
    rf_gl_vertex2f(rec.width, 0.0f);
    rf_gl_end();
    rf_pop_matrix();

    rf_gl_disable_texture();
}

// Draw a vertical-gradient-filled rectangle
// NOTE: Gradient goes from bottom (color1) to top (color2)
RF_API void rf_draw_rectangle_gradient_v(int posX, int posY, int width, int height, rf_color color1, rf_color color2)
{
    rf_draw_rectangle_gradient_ex((rf_rectangle){ (float)posX, (float)posY, (float)width, (float)height }, color1, color2, color2, color1);
}

// Draw a horizontal-gradient-filled rectangle
// NOTE: Gradient goes from bottom (color1) to top (color2)
RF_API void rf_draw_rectangle_gradient_h(int posX, int posY, int width, int height, rf_color color1, rf_color color2)
{
    rf_draw_rectangle_gradient_ex((rf_rectangle){ (float)posX, (float)posY, (float)width, (float)height }, color1, color1, color2, color2);
}

// Draw a gradient-filled rectangle
// NOTE: Colors refer to corners, starting at top-lef corner and counter-clockwise
RF_API void rf_draw_rectangle_gradient_ex(rf_rectangle rec, rf_color col1, rf_color col2, rf_color col3, rf_color col4)
{
    rf_gl_enable_texture(_rf_get_shapes_texture().id);

    rf_push_matrix();
    rf_gl_begin(GL_QUADS);
    rf_gl_normal3f(0.0f, 0.0f, 1.0f);

    // NOTE: Default raylib font character 95 is a white square
    rf_gl_color4ub(col1.r, col1.g, col1.b, col1.a);
    rf_gl_tex_coord2f(_rf_global_context_ptr->gl_ctx.rec_tex_shapes.x/_rf_global_context_ptr->gl_ctx.tex_shapes.width, _rf_global_context_ptr->gl_ctx.rec_tex_shapes.y/_rf_global_context_ptr->gl_ctx.tex_shapes.height);
    rf_gl_vertex2f(rec.x, rec.y);

    rf_gl_color4ub(col2.r, col2.g, col2.b, col2.a);
    rf_gl_tex_coord2f(_rf_global_context_ptr->gl_ctx.rec_tex_shapes.x/_rf_global_context_ptr->gl_ctx.tex_shapes.width, (_rf_global_context_ptr->gl_ctx.rec_tex_shapes.y + _rf_global_context_ptr->gl_ctx.rec_tex_shapes.height)/_rf_global_context_ptr->gl_ctx.tex_shapes.height);
    rf_gl_vertex2f(rec.x, rec.y + rec.height);

    rf_gl_color4ub(col3.r, col3.g, col3.b, col3.a);
    rf_gl_tex_coord2f((_rf_global_context_ptr->gl_ctx.rec_tex_shapes.x + _rf_global_context_ptr->gl_ctx.rec_tex_shapes.width)/_rf_global_context_ptr->gl_ctx.tex_shapes.width, (_rf_global_context_ptr->gl_ctx.rec_tex_shapes.y + _rf_global_context_ptr->gl_ctx.rec_tex_shapes.height)/_rf_global_context_ptr->gl_ctx.tex_shapes.height);
    rf_gl_vertex2f(rec.x + rec.width, rec.y + rec.height);

    rf_gl_color4ub(col4.r, col4.g, col4.b, col4.a);
    rf_gl_tex_coord2f((_rf_global_context_ptr->gl_ctx.rec_tex_shapes.x + _rf_global_context_ptr->gl_ctx.rec_tex_shapes.width)/_rf_global_context_ptr->gl_ctx.tex_shapes.width, _rf_global_context_ptr->gl_ctx.rec_tex_shapes.y/_rf_global_context_ptr->gl_ctx.tex_shapes.height);
    rf_gl_vertex2f(rec.x + rec.width, rec.y);
    rf_gl_end();
    rf_pop_matrix();

    rf_gl_disable_texture();
}

// Draw rectangle outline
// NOTE: On OpenGL 3.3 and ES2 we use QUADS to avoid drawing order issues (view rf_gl_draw)
RF_API void rf_draw_rectangle_lines(int posX, int posY, int width, int height, rf_color color)
{
    rf_gl_begin(GL_LINES);
    rf_gl_color4ub(color.r, color.g, color.b, color.a);
    rf_gl_vertex2i(posX + 1, posY + 1);
    rf_gl_vertex2i(posX + width, posY + 1);

    rf_gl_vertex2i(posX + width, posY + 1);
    rf_gl_vertex2i(posX + width, posY + height);

    rf_gl_vertex2i(posX + width, posY + height);
    rf_gl_vertex2i(posX + 1, posY + height);

    rf_gl_vertex2i(posX + 1, posY + height);
    rf_gl_vertex2i(posX + 1, posY + 1);
    rf_gl_end();

}

// Draw rectangle outline with extended parameters
RF_API void rf_draw_rectangle_lines_ex(rf_rectangle rec, int lineThick, rf_color color)
{
    if (lineThick > rec.width || lineThick > rec.height)
    {
        if (rec.width > rec.height) lineThick = (int)rec.height/2;
        else if (rec.width < rec.height) lineThick = (int)rec.width/2;
    }

    rf_draw_rectangle((int)rec.x, (int)rec.y, (int)rec.width, lineThick, color);
    rf_draw_rectangle((int)(rec.x - lineThick + rec.width), (int)(rec.y + lineThick), lineThick, (int)(rec.height - lineThick*2.0f), color);
    rf_draw_rectangle((int)rec.x, (int)(rec.y + rec.height - lineThick), (int)rec.width, lineThick, color);
    rf_draw_rectangle((int)rec.x, (int)(rec.y + lineThick), lineThick, (int)(rec.height - lineThick*2), color);
}

// Draw rectangle with rounded edges
RF_API void rf_draw_rectangle_rounded(rf_rectangle rec, float roundness, int segments, rf_color color)
{
    // Not a rounded rectangle
    if ((roundness <= 0.0f) || (rec.width < 1) || (rec.height < 1 ))
    {
        rf_draw_rectangle_rec(rec, color);
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

    float stepLength = 90.0f/(float)segments;

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

    const rf_vector2 point[12] = { // coordinates of the 12 points that define the rounded rect (the idea here is to make things easier)
            {(float)rec.x + radius, rec.y}, {(float)(rec.x + rec.width) - radius, rec.y}, { rec.x + rec.width, (float)rec.y + radius }, // PO, P1, P2
            {rec.x + rec.width, (float)(rec.y + rec.height) - radius}, {(float)(rec.x + rec.width) - radius, rec.y + rec.height}, // P3, P4
            {(float)rec.x + radius, rec.y + rec.height}, { rec.x, (float)(rec.y + rec.height) - radius}, {rec.x, (float)rec.y + radius}, // P5, P6, P7
            {(float)rec.x + radius, (float)rec.y + radius}, {(float)(rec.x + rec.width) - radius, (float)rec.y + radius}, // P8, P9
            {(float)(rec.x + rec.width) - radius, (float)(rec.y + rec.height) - radius}, {(float)rec.x + radius, (float)(rec.y + rec.height) - radius} // P10, P11
    };

    const rf_vector2 centers[4] = { point[8], point[9], point[10], point[11] };
    const float angles[4] = { 180.0f, 90.0f, 0.0f, 270.0f };
    if (rf_gl_check_buffer_limit(12*segments + 5*6)) rf_gl_draw(); // 4 corners with 3 vertices per segment + 5 rectangles with 6 vertices each

    rf_gl_begin(GL_TRIANGLES);
    // Draw all of the 4 corners: [1] Upper Left Corner, [3] Upper Right Corner, [5] Lower Right Corner, [7] Lower Left Corner
    for (int k = 0; k < 4; ++k) // Hope the compiler is smart enough to unroll this loop
    {
        float angle = angles[k];
        const rf_vector2 center = centers[k];
        for (int i = 0; i < segments; i++)
        {
            rf_gl_color4ub(color.r, color.g, color.b, color.a);
            rf_gl_vertex2f(center.x, center.y);
            rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*radius, center.y + cosf(RF_DEG2RAD*angle)*radius);
            rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + stepLength))*radius, center.y + cosf(RF_DEG2RAD*(angle + stepLength))*radius);
            angle += stepLength;
        }
    }

    // [2] Upper rf_rectangle
    rf_gl_color4ub(color.r, color.g, color.b, color.a);
    rf_gl_vertex2f(point[0].x, point[0].y);
    rf_gl_vertex2f(point[8].x, point[8].y);
    rf_gl_vertex2f(point[9].x, point[9].y);
    rf_gl_vertex2f(point[1].x, point[1].y);
    rf_gl_vertex2f(point[0].x, point[0].y);
    rf_gl_vertex2f(point[9].x, point[9].y);

    // [4] Right rf_rectangle
    rf_gl_color4ub(color.r, color.g, color.b, color.a);
    rf_gl_vertex2f(point[9].x, point[9].y);
    rf_gl_vertex2f(point[10].x, point[10].y);
    rf_gl_vertex2f(point[3].x, point[3].y);
    rf_gl_vertex2f(point[2].x, point[2].y);
    rf_gl_vertex2f(point[9].x, point[9].y);
    rf_gl_vertex2f(point[3].x, point[3].y);

    // [6] Bottom rf_rectangle
    rf_gl_color4ub(color.r, color.g, color.b, color.a);
    rf_gl_vertex2f(point[11].x, point[11].y);
    rf_gl_vertex2f(point[5].x, point[5].y);
    rf_gl_vertex2f(point[4].x, point[4].y);
    rf_gl_vertex2f(point[10].x, point[10].y);
    rf_gl_vertex2f(point[11].x, point[11].y);
    rf_gl_vertex2f(point[4].x, point[4].y);

    // [8] Left rf_rectangle
    rf_gl_color4ub(color.r, color.g, color.b, color.a);
    rf_gl_vertex2f(point[7].x, point[7].y);
    rf_gl_vertex2f(point[6].x, point[6].y);
    rf_gl_vertex2f(point[11].x, point[11].y);
    rf_gl_vertex2f(point[8].x, point[8].y);
    rf_gl_vertex2f(point[7].x, point[7].y);
    rf_gl_vertex2f(point[11].x, point[11].y);

    // [9] Middle rf_rectangle
    rf_gl_color4ub(color.r, color.g, color.b, color.a);
    rf_gl_vertex2f(point[8].x, point[8].y);
    rf_gl_vertex2f(point[11].x, point[11].y);
    rf_gl_vertex2f(point[10].x, point[10].y);
    rf_gl_vertex2f(point[9].x, point[9].y);
    rf_gl_vertex2f(point[8].x, point[8].y);
    rf_gl_vertex2f(point[10].x, point[10].y);
    rf_gl_end();

}

// Draw rectangle with rounded edges outline
RF_API void rf_draw_rectangle_rounded_lines(rf_rectangle rec, float roundness, int segments, int lineThick, rf_color color)
{
    if (lineThick < 0) lineThick = 0;

    // Not a rounded rectangle
    if (roundness <= 0.0f)
    {
        rf_draw_rectangle_lines_ex((rf_rectangle){rec.x-lineThick, rec.y-lineThick, rec.width+2*lineThick, rec.height+2*lineThick}, lineThick, color);
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

    float stepLength = 90.0f/(float)segments;
    const float outerRadius = radius + (float)lineThick, innerRadius = radius;

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
    const rf_vector2 point[16] =
    {
            {(float)rec.x + innerRadius, rec.y - lineThick}, {(float)(rec.x + rec.width) - innerRadius, rec.y - lineThick}, { rec.x + rec.width + lineThick, (float)rec.y + innerRadius }, // PO, P1, P2
            {rec.x + rec.width + lineThick, (float)(rec.y + rec.height) - innerRadius}, {(float)(rec.x + rec.width) - innerRadius, rec.y + rec.height + lineThick}, // P3, P4
            {(float)rec.x + innerRadius, rec.y + rec.height + lineThick}, { rec.x - lineThick, (float)(rec.y + rec.height) - innerRadius}, {rec.x - lineThick, (float)rec.y + innerRadius}, // P5, P6, P7
            {(float)rec.x + innerRadius, rec.y}, {(float)(rec.x + rec.width) - innerRadius, rec.y}, // P8, P9
            { rec.x + rec.width, (float)rec.y + innerRadius }, {rec.x + rec.width, (float)(rec.y + rec.height) - innerRadius}, // P10, P11
            {(float)(rec.x + rec.width) - innerRadius, rec.y + rec.height}, {(float)rec.x + innerRadius, rec.y + rec.height}, // P12, P13
            { rec.x, (float)(rec.y + rec.height) - innerRadius}, {rec.x, (float)rec.y + innerRadius} // P14, P15
    };

    const rf_vector2 centers[4] =
    {
            {(float)rec.x + innerRadius, (float)rec.y + innerRadius}, {(float)(rec.x + rec.width) - innerRadius, (float)rec.y + innerRadius}, // P16, P17
            {(float)(rec.x + rec.width) - innerRadius, (float)(rec.y + rec.height) - innerRadius}, {(float)rec.x + innerRadius, (float)(rec.y + rec.height) - innerRadius} // P18, P19
    };

    const float angles[4] = { 180.0f, 90.0f, 0.0f, 270.0f };

    if (lineThick > 1)
    {
        if (rf_gl_check_buffer_limit(4*6*segments + 4*6)) rf_gl_draw(); // 4 corners with 6(2*3) vertices for each segment + 4 rectangles with 6 vertices each

        rf_gl_begin(GL_TRIANGLES);

        // Draw all of the 4 corners first: Upper Left Corner, Upper Right Corner, Lower Right Corner, Lower Left Corner
        for (int k = 0; k < 4; ++k) // Hope the compiler is smart enough to unroll this loop
        {
            float angle = angles[k];
            const rf_vector2 center = centers[k];

            for (int i = 0; i < segments; i++)
            {
                rf_gl_color4ub(color.r, color.g, color.b, color.a);

                rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*innerRadius, center.y + cosf(RF_DEG2RAD*angle)*innerRadius);
                rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*outerRadius, center.y + cosf(RF_DEG2RAD*angle)*outerRadius);
                rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + stepLength))*innerRadius, center.y + cosf(RF_DEG2RAD*(angle + stepLength))*innerRadius);

                rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + stepLength))*innerRadius, center.y + cosf(RF_DEG2RAD*(angle + stepLength))*innerRadius);
                rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*outerRadius, center.y + cosf(RF_DEG2RAD*angle)*outerRadius);
                rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + stepLength))*outerRadius, center.y + cosf(RF_DEG2RAD*(angle + stepLength))*outerRadius);

                angle += stepLength;
            }
        }

        // Upper rectangle
        rf_gl_color4ub(color.r, color.g, color.b, color.a);
        rf_gl_vertex2f(point[0].x, point[0].y);
        rf_gl_vertex2f(point[8].x, point[8].y);
        rf_gl_vertex2f(point[9].x, point[9].y);
        rf_gl_vertex2f(point[1].x, point[1].y);
        rf_gl_vertex2f(point[0].x, point[0].y);
        rf_gl_vertex2f(point[9].x, point[9].y);

        // Right rectangle
        rf_gl_color4ub(color.r, color.g, color.b, color.a);
        rf_gl_vertex2f(point[10].x, point[10].y);
        rf_gl_vertex2f(point[11].x, point[11].y);
        rf_gl_vertex2f(point[3].x, point[3].y);
        rf_gl_vertex2f(point[2].x, point[2].y);
        rf_gl_vertex2f(point[10].x, point[10].y);
        rf_gl_vertex2f(point[3].x, point[3].y);

        // Lower rectangle
        rf_gl_color4ub(color.r, color.g, color.b, color.a);
        rf_gl_vertex2f(point[13].x, point[13].y);
        rf_gl_vertex2f(point[5].x, point[5].y);
        rf_gl_vertex2f(point[4].x, point[4].y);
        rf_gl_vertex2f(point[12].x, point[12].y);
        rf_gl_vertex2f(point[13].x, point[13].y);
        rf_gl_vertex2f(point[4].x, point[4].y);

        // Left rectangle
        rf_gl_color4ub(color.r, color.g, color.b, color.a);
        rf_gl_vertex2f(point[7].x, point[7].y);
        rf_gl_vertex2f(point[6].x, point[6].y);
        rf_gl_vertex2f(point[14].x, point[14].y);
        rf_gl_vertex2f(point[15].x, point[15].y);
        rf_gl_vertex2f(point[7].x, point[7].y);
        rf_gl_vertex2f(point[14].x, point[14].y);
        rf_gl_end();

    }
    else
    {
        // Use LINES to draw the outline
        if (rf_gl_check_buffer_limit(8*segments + 4*2)) rf_gl_draw(); // 4 corners with 2 vertices for each segment + 4 rectangles with 2 vertices each

        rf_gl_begin(GL_LINES);

        // Draw all of the 4 corners first: Upper Left Corner, Upper Right Corner, Lower Right Corner, Lower Left Corner
        for (int k = 0; k < 4; ++k) // Hope the compiler is smart enough to unroll this loop
        {
            float angle = angles[k];
            const rf_vector2 center = centers[k];

            for (int i = 0; i < segments; i++)
            {
                rf_gl_color4ub(color.r, color.g, color.b, color.a);
                rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*angle)*outerRadius, center.y + cosf(RF_DEG2RAD*angle)*outerRadius);
                rf_gl_vertex2f(center.x + sinf(RF_DEG2RAD*(angle + stepLength))*outerRadius, center.y + cosf(RF_DEG2RAD*(angle + stepLength))*outerRadius);
                angle += stepLength;
            }
        }
        // And now the remaining 4 lines
        for(int i = 0; i < 8; i += 2)
        {
            rf_gl_color4ub(color.r, color.g, color.b, color.a);
            rf_gl_vertex2f(point[i].x, point[i].y);
            rf_gl_vertex2f(point[i + 1].x, point[i + 1].y);
        }
        rf_gl_end();
    }
}

// Draw a triangle
// NOTE: Vertex must be provided in counter-clockwise order
RF_API void rf_draw_triangle(rf_vector2 v1, rf_vector2 v2, rf_vector2 v3, rf_color color)
{
    if (rf_gl_check_buffer_limit(4)) rf_gl_draw();
    rf_gl_begin(GL_TRIANGLES);
    rf_gl_color4ub(color.r, color.g, color.b, color.a);
    rf_gl_vertex2f(v1.x, v1.y);
    rf_gl_vertex2f(v2.x, v2.y);
    rf_gl_vertex2f(v3.x, v3.y);
    rf_gl_end();

}

// Draw a triangle using lines
// NOTE: Vertex must be provided in counter-clockwise order
RF_API void rf_draw_triangle_lines(rf_vector2 v1, rf_vector2 v2, rf_vector2 v3, rf_color color)
{
    if (rf_gl_check_buffer_limit(6)) rf_gl_draw();

    rf_gl_begin(GL_LINES);
    rf_gl_color4ub(color.r, color.g, color.b, color.a);
    rf_gl_vertex2f(v1.x, v1.y);
    rf_gl_vertex2f(v2.x, v2.y);

    rf_gl_vertex2f(v2.x, v2.y);
    rf_gl_vertex2f(v3.x, v3.y);

    rf_gl_vertex2f(v3.x, v3.y);
    rf_gl_vertex2f(v1.x, v1.y);
    rf_gl_end();
}

// Draw a triangle fan defined by points
// NOTE: First vertex provided is the center, shared by all triangles
RF_API void rf_draw_triangle_fan(rf_vector2 *points, int pointsCount, rf_color color)
{
    if (pointsCount >= 3)
    {
        if (rf_gl_check_buffer_limit((pointsCount - 2)*4)) rf_gl_draw();

        rf_gl_enable_texture(_rf_get_shapes_texture().id);
        rf_gl_begin(GL_QUADS);
        rf_gl_color4ub(color.r, color.g, color.b, color.a);

        for (int i = 1; i < pointsCount - 1; i++)
        {
            rf_gl_tex_coord2f(_rf_global_context_ptr->gl_ctx.rec_tex_shapes.x/_rf_global_context_ptr->gl_ctx.tex_shapes.width, _rf_global_context_ptr->gl_ctx.rec_tex_shapes.y/_rf_global_context_ptr->gl_ctx.tex_shapes.height);
            rf_gl_vertex2f(points[0].x, points[0].y);

            rf_gl_tex_coord2f(_rf_global_context_ptr->gl_ctx.rec_tex_shapes.x/_rf_global_context_ptr->gl_ctx.tex_shapes.width, (_rf_global_context_ptr->gl_ctx.rec_tex_shapes.y + _rf_global_context_ptr->gl_ctx.rec_tex_shapes.height)/_rf_global_context_ptr->gl_ctx.tex_shapes.height);
            rf_gl_vertex2f(points[i].x, points[i].y);

            rf_gl_tex_coord2f((_rf_global_context_ptr->gl_ctx.rec_tex_shapes.x + _rf_global_context_ptr->gl_ctx.rec_tex_shapes.width)/_rf_global_context_ptr->gl_ctx.tex_shapes.width, (_rf_global_context_ptr->gl_ctx.rec_tex_shapes.y + _rf_global_context_ptr->gl_ctx.rec_tex_shapes.height)/_rf_global_context_ptr->gl_ctx.tex_shapes.height);
            rf_gl_vertex2f(points[i + 1].x, points[i + 1].y);

            rf_gl_tex_coord2f((_rf_global_context_ptr->gl_ctx.rec_tex_shapes.x + _rf_global_context_ptr->gl_ctx.rec_tex_shapes.width)/_rf_global_context_ptr->gl_ctx.tex_shapes.width, _rf_global_context_ptr->gl_ctx.rec_tex_shapes.y/_rf_global_context_ptr->gl_ctx.tex_shapes.height);
            rf_gl_vertex2f(points[i + 1].x, points[i + 1].y);
        }
        rf_gl_end();
        rf_gl_disable_texture();
    }
}

// Draw a triangle strip defined by points
// NOTE: Every new vertex connects with previous two
RF_API void rf_draw_triangle_strip(rf_vector2 *points, int pointsCount, rf_color color)
{
    if (pointsCount >= 3)
    {
        if (rf_gl_check_buffer_limit(pointsCount)) rf_gl_draw();

        rf_gl_begin(GL_TRIANGLES);
        rf_gl_color4ub(color.r, color.g, color.b, color.a);

        for (int i = 2; i < pointsCount; i++)
        {
            if ((i%2) == 0)
            {
                rf_gl_vertex2f(points[i].x, points[i].y);
                rf_gl_vertex2f(points[i - 2].x, points[i - 2].y);
                rf_gl_vertex2f(points[i - 1].x, points[i - 1].y);
            }
            else
            {
                rf_gl_vertex2f(points[i].x, points[i].y);
                rf_gl_vertex2f(points[i - 1].x, points[i - 1].y);
                rf_gl_vertex2f(points[i - 2].x, points[i - 2].y);
            }
        }
        rf_gl_end();
    }
}

// Draw a regular polygon of n sides (Vector version)
RF_API void rf_draw_poly(rf_vector2 center, int sides, float radius, float rotation, rf_color color)
{
    if (sides < 3) sides = 3;
    float centralAngle = 0.0f;

    if (rf_gl_check_buffer_limit(4*(360/sides))) rf_gl_draw();

    rf_push_matrix();
    rf_translatef(center.x, center.y, 0.0f);
    rf_rotatef(rotation, 0.0f, 0.0f, 1.0f);
    rf_gl_begin(GL_TRIANGLES);
    for (int i = 0; i < sides; i++)
    {
        rf_gl_color4ub(color.r, color.g, color.b, color.a);

        rf_gl_vertex2f(0, 0);
        rf_gl_vertex2f(sinf(RF_DEG2RAD*centralAngle)*radius, cosf(RF_DEG2RAD*centralAngle)*radius);

        centralAngle += 360.0f/(float)sides;
        rf_gl_vertex2f(sinf(RF_DEG2RAD*centralAngle)*radius, cosf(RF_DEG2RAD*centralAngle)*radius);
    }
    rf_gl_end();

    rf_pop_matrix();
}

// Define default texture used to draw shapes
RF_API void rf_set_shapes_texture(rf_texture2d texture, rf_rectangle source)
{
    _rf_global_context_ptr->gl_ctx.tex_shapes = texture;
    _rf_global_context_ptr->gl_ctx.rec_tex_shapes = source;
}

// Cubic easing in-out
// NOTE: Required for rf_draw_line_bezier()
RF_INTERNAL float _rf_shapes_ease_cubic_in_out(float t, float b, float c, float d)
{
    if ((t /= 0.5f*d) < 1) return 0.5f*c*t*t*t + b;

    t -= 2;

    return 0.5f*c*(t*t*t + 2.0f) + b;
}

// Get texture to draw shapes (RAII)
RF_INTERNAL rf_texture2d _rf_get_shapes_texture()
{
    if (_rf_global_context_ptr->gl_ctx.tex_shapes.id == 0)
    {
        _rf_global_context_ptr->gl_ctx.tex_shapes = rf_get_texture_default(); // Use default white texture
        _rf_global_context_ptr->gl_ctx.rec_tex_shapes = RF_CLITERAL(rf_rectangle){ 0.0f, 0.0f, 1.0f, 1.0f };
    }

    return _rf_global_context_ptr->gl_ctx.tex_shapes;
}

//endregion

//region texture

RF_INTERNAL rf_image _rf_load_animated_gif(const char* fileName, int* frames, int** delays); // Load animated GIF file

// Load image from file into CPU memory (RAM)
RF_API rf_image rf_load_image(const char* fileName)
{
    rf_image image = { 0 };

    if ((_rf_is_file_extension(fileName, ".png"))
        || (_rf_is_file_extension(fileName, ".bmp"))
        || (_rf_is_file_extension(fileName, ".tga"))
        || (_rf_is_file_extension(fileName, ".gif"))
        || (_rf_is_file_extension(fileName, ".pic"))
        || (_rf_is_file_extension(fileName, ".psd"))
            )
    {

        int imgWidth = 0;
        int imgHeight = 0;
        int imgBpp = 0;

        int file_size = rf_get_file_size(fileName);
        rf_byte* image_file_buffer = (rf_byte*) RF_MALLOC(file_size);
        rf_load_file_into_buffer(fileName, image_file_buffer, file_size);

        if (image_file_buffer != NULL)
        {
            // NOTE: Using stb_image to load images (Supports multiple image formats)
            image.data = stbi_load_from_memory(image_file_buffer, file_size, &imgWidth, &imgHeight, &imgBpp, 0);

            image.width = imgWidth;
            image.height = imgHeight;
            image.mipmaps = 1;

            if (imgBpp == 1) image.format = rf_uncompressed_grayscale;
            else if (imgBpp == 2) image.format = rf_uncompressed_gray_alpha;
            else if (imgBpp == 3) image.format = rf_uncompressed_r8g8b8;
            else if (imgBpp == 4) image.format = rf_uncompressed_r8g8b8a8;
        }

        RF_FREE(image_file_buffer);
    }
    else RF_LOG(RF_LOG_WARNING, "[%s] rf_image fileformat not supported", fileName);

    if (image.data != NULL) RF_LOG(RF_LOG_INFO, "[%s] rf_image loaded successfully (%ix%i)", fileName, image.width, image.height);
    else RF_LOG(RF_LOG_WARNING, "[%s] rf_image could not be loaded", fileName);

    return image;
}

// Load image from rf_color array data (RGBA - 32bit)
// NOTE: Creates a copy of pixels data array
RF_API rf_image rf_load_image_ex(rf_color* pixels, int width, int height)
{
    rf_image image;
    image.data = NULL;
    image.width = width;
    image.height = height;
    image.mipmaps = 1;
    image.format = rf_uncompressed_r8g8b8a8;

    int k = 0;

    image.data = (unsigned char* )RF_MALLOC(image.width*image.height*4*sizeof(unsigned char));

    for (int i = 0; i < image.width*image.height*4; i += 4)
    {
        ((unsigned char* )image.data)[i] = pixels[k].r;
        ((unsigned char* )image.data)[i + 1] = pixels[k].g;
        ((unsigned char* )image.data)[i + 2] = pixels[k].b;
        ((unsigned char* )image.data)[i + 3] = pixels[k].a;
        k++;
    }

    return image;
}

// Load image from raw data with parameters
// NOTE: This functions makes a copy of provided data
RF_API rf_image rf_load_image_pro(void* data, int width, int height, int format)
{
    rf_image srcImage = { 0 };

    srcImage.data = data;
    srcImage.width = width;
    srcImage.height = height;
    srcImage.mipmaps = 1;
    srcImage.format = format;

    rf_image dstImage = rf_image_copy(srcImage);

    return dstImage;
}

// Load an image from RAW file data
RF_API rf_image rf_load_image_raw(const char* fileName, int width, int height, int format, int headerSize)
{
    rf_image image = { 0 };

    FILE* rawFile = fopen(fileName, "rb");

    if (rawFile == NULL)
    {
        RF_LOG(RF_LOG_WARNING, "[%s] RAW image file could not be opened", fileName);
    }
    else
    {
        if (headerSize > 0) fseek(rawFile, headerSize, SEEK_SET);

        unsigned int size = rf_get_pixel_data_size(width, height, format);

        image.data = RF_MALLOC(size); // Allocate required memory in bytes

        // NOTE: fread() returns num read elements instead of bytes,
        // to get bytes we need to read (1 rf_byte size, elements) instead of (x rf_byte size, 1 element)
        int bytes = fread(image.data, 1, size, rawFile);

        // Check if data has been read successfully
        if (bytes < size)
        {
            RF_LOG(RF_LOG_WARNING, "[%s] RAW image data can not be read, wrong requested format or size", fileName);

            RF_FREE(image.data);
        }
        else
        {
            image.width = width;
            image.height = height;
            image.mipmaps = 1;
            image.format = format;
        }

        fclose(rawFile);
    }

    return image;
}

// Load texture from file into GPU memory (VRAM)
RF_API rf_texture2d rf_load_texture(const char* fileName)
{
    rf_texture2d texture = { 0 };

    rf_image image = rf_load_image(fileName);

    if (image.data != NULL)
    {
        texture = rf_load_texture_from_image(image);
        rf_unload_image(image);
    }
    else RF_LOG(RF_LOG_WARNING, "rf_texture could not be created");

    return texture;
}

// Load a texture from image data
// NOTE: image is not unloaded, it must be done manually
RF_API rf_texture2d rf_load_texture_from_image(rf_image image)
{
    rf_texture2d texture = { 0 };

    if ((image.data != NULL) && (image.width != 0) && (image.height != 0))
    {
        texture.id = rf_gl_load_texture(image.data, image.width, image.height, image.format, image.mipmaps);
    }
    else RF_LOG(RF_LOG_WARNING, "rf_texture could not be loaded from rf_image");

    texture.width = image.width;
    texture.height = image.height;
    texture.mipmaps = image.mipmaps;
    texture.format = image.format;

    return texture;
}

// Load texture for rendering (framebuffer)
// NOTE: Render texture is loaded by default with RGBA color attachment and depth RenderBuffer
RF_API rf_render_texture2d rf_load_render_texture(int width, int height)
{
    rf_render_texture2d target = rf_gl_load_render_texture(width, height, rf_uncompressed_r8g8b8a8, 24, false);

    return target;
}

// Unload image from CPU memory (RAM)
RF_API void rf_unload_image(rf_image image)
{
    RF_FREE(image.data);
}

// Unload texture from GPU memory (VRAM)
RF_API void rf_unload_texture(rf_texture2d texture)
{
    if (texture.id > 0)
    {
        rf_gl_delete_textures(texture.id);

        RF_LOG(RF_LOG_INFO, "[TEX ID %i] Unloaded texture data from VRAM (GPU)", texture.id);
    }
}

// Unload render texture from GPU memory (VRAM)
RF_API void rf_unload_render_texture(rf_render_texture2d target)
{
    if (target.id > 0) rf_gl_delete_render_textures(target);
}

// Get pixel data from image in the form of rf_color struct array
// @Todo: Good candidate for refactoring since its easy. Dont alloc a buffer for the user ffs
RF_API rf_color* rf_get_image_data(rf_image image)
{
    rf_color* pixels = (rf_color* )RF_MALLOC(image.width*image.height*sizeof(rf_color));

    if (pixels == NULL) return pixels;

    if (image.format >= rf_compressed_dxt1_rgb) RF_LOG(RF_LOG_WARNING, "Pixel data retrieval not supported for compressed image formats");
    else
    {
        if ((image.format == rf_uncompressed_r32) ||
            (image.format == rf_uncompressed_r32g32b32) ||
            (image.format == rf_uncompressed_r32g32b32a32)) RF_LOG(RF_LOG_WARNING, "32bit pixel format converted to 8bit per channel");

        for (int i = 0, k = 0; i < image.width*image.height; i++)
        {
            switch (image.format)
            {
                case rf_uncompressed_grayscale:
                {
                    pixels[i].r = ((unsigned char* )image.data)[i];
                    pixels[i].g = ((unsigned char* )image.data)[i];
                    pixels[i].b = ((unsigned char* )image.data)[i];
                    pixels[i].a = 255;

                } break;
                case rf_uncompressed_gray_alpha:
                {
                    pixels[i].r = ((unsigned char* )image.data)[k];
                    pixels[i].g = ((unsigned char* )image.data)[k];
                    pixels[i].b = ((unsigned char* )image.data)[k];
                    pixels[i].a = ((unsigned char* )image.data)[k + 1];

                    k += 2;
                } break;
                case rf_uncompressed_r5g5b5a1:
                {
                    unsigned short pixel = ((unsigned short *)image.data)[i];

                    pixels[i].r = (unsigned char)((float)((pixel & 0b1111100000000000) >> 11)*(255/31));
                    pixels[i].g = (unsigned char)((float)((pixel & 0b0000011111000000) >> 6)*(255/31));
                    pixels[i].b = (unsigned char)((float)((pixel & 0b0000000000111110) >> 1)*(255/31));
                    pixels[i].a = (unsigned char)((pixel & 0b0000000000000001)*255);

                } break;
                case rf_uncompressed_r5g6b5:
                {
                    unsigned short pixel = ((unsigned short *)image.data)[i];

                    pixels[i].r = (unsigned char)((float)((pixel & 0b1111100000000000) >> 11)*(255/31));
                    pixels[i].g = (unsigned char)((float)((pixel & 0b0000011111100000) >> 5)*(255/63));
                    pixels[i].b = (unsigned char)((float)(pixel & 0b0000000000011111)*(255/31));
                    pixels[i].a = 255;

                } break;
                case rf_uncompressed_r4g4b4a4:
                {
                    unsigned short pixel = ((unsigned short *)image.data)[i];

                    pixels[i].r = (unsigned char)((float)((pixel & 0b1111000000000000) >> 12)*(255/15));
                    pixels[i].g = (unsigned char)((float)((pixel & 0b0000111100000000) >> 8)*(255/15));
                    pixels[i].b = (unsigned char)((float)((pixel & 0b0000000011110000) >> 4)*(255/15));
                    pixels[i].a = (unsigned char)((float)(pixel & 0b0000000000001111)*(255/15));

                } break;
                case rf_uncompressed_r8g8b8a8:
                {
                    pixels[i].r = ((unsigned char* )image.data)[k];
                    pixels[i].g = ((unsigned char* )image.data)[k + 1];
                    pixels[i].b = ((unsigned char* )image.data)[k + 2];
                    pixels[i].a = ((unsigned char* )image.data)[k + 3];

                    k += 4;
                } break;
                case rf_uncompressed_r8g8b8:
                {
                    pixels[i].r = (unsigned char)((unsigned char* )image.data)[k];
                    pixels[i].g = (unsigned char)((unsigned char* )image.data)[k + 1];
                    pixels[i].b = (unsigned char)((unsigned char* )image.data)[k + 2];
                    pixels[i].a = 255;

                    k += 3;
                } break;
                case rf_uncompressed_r32:
                {
                    pixels[i].r = (unsigned char)(((float* )image.data)[k]*255.0f);
                    pixels[i].g = 0;
                    pixels[i].b = 0;
                    pixels[i].a = 255;

                } break;
                case rf_uncompressed_r32g32b32:
                {
                    pixels[i].r = (unsigned char)(((float* )image.data)[k]*255.0f);
                    pixels[i].g = (unsigned char)(((float* )image.data)[k + 1]*255.0f);
                    pixels[i].b = (unsigned char)(((float* )image.data)[k + 2]*255.0f);
                    pixels[i].a = 255;

                    k += 3;
                } break;
                case rf_uncompressed_r32g32b32a32:
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

// Get pixel data from image as rf_vector4 array (float normalized)
RF_API rf_vector4* rf_get_image_data_normalized(rf_image image)
{
    rf_vector4* pixels = (rf_vector4* )RF_MALLOC(image.width*image.height*sizeof(rf_vector4));

    if (image.format >= rf_compressed_dxt1_rgb) RF_LOG(RF_LOG_WARNING, "Pixel data retrieval not supported for compressed image formats");
    else
    {
        for (int i = 0, k = 0; i < image.width*image.height; i++)
        {
            switch (image.format)
            {
                case rf_uncompressed_grayscale:
                {
                    pixels[i].x = (float)((unsigned char* )image.data)[i]/255.0f;
                    pixels[i].y = (float)((unsigned char* )image.data)[i]/255.0f;
                    pixels[i].z = (float)((unsigned char* )image.data)[i]/255.0f;
                    pixels[i].w = 1.0f;

                } break;
                case rf_uncompressed_gray_alpha:
                {
                    pixels[i].x = (float)((unsigned char* )image.data)[k]/255.0f;
                    pixels[i].y = (float)((unsigned char* )image.data)[k]/255.0f;
                    pixels[i].z = (float)((unsigned char* )image.data)[k]/255.0f;
                    pixels[i].w = (float)((unsigned char* )image.data)[k + 1]/255.0f;

                    k += 2;
                } break;
                case rf_uncompressed_r5g5b5a1:
                {
                    unsigned short pixel = ((unsigned short *)image.data)[i];

                    pixels[i].x = (float)((pixel & 0b1111100000000000) >> 11)*(1.0f/31);
                    pixels[i].y = (float)((pixel & 0b0000011111000000) >> 6)*(1.0f/31);
                    pixels[i].z = (float)((pixel & 0b0000000000111110) >> 1)*(1.0f/31);
                    pixels[i].w = ((pixel & 0b0000000000000001) == 0)? 0.0f : 1.0f;

                } break;
                case rf_uncompressed_r5g6b5:
                {
                    unsigned short pixel = ((unsigned short *)image.data)[i];

                    pixels[i].x = (float)((pixel & 0b1111100000000000) >> 11)*(1.0f/31);
                    pixels[i].y = (float)((pixel & 0b0000011111100000) >> 5)*(1.0f/63);
                    pixels[i].z = (float)(pixel & 0b0000000000011111)*(1.0f/31);
                    pixels[i].w = 1.0f;

                } break;
                case rf_uncompressed_r4g4b4a4:
                {
                    unsigned short pixel = ((unsigned short *)image.data)[i];

                    pixels[i].x = (float)((pixel & 0b1111000000000000) >> 12)*(1.0f/15);
                    pixels[i].y = (float)((pixel & 0b0000111100000000) >> 8)*(1.0f/15);
                    pixels[i].z = (float)((pixel & 0b0000000011110000) >> 4)*(1.0f/15);
                    pixels[i].w = (float)(pixel & 0b0000000000001111)*(1.0f/15);

                } break;
                case rf_uncompressed_r8g8b8a8:
                {
                    pixels[i].x = (float)((unsigned char* )image.data)[k]/255.0f;
                    pixels[i].y = (float)((unsigned char* )image.data)[k + 1]/255.0f;
                    pixels[i].z = (float)((unsigned char* )image.data)[k + 2]/255.0f;
                    pixels[i].w = (float)((unsigned char* )image.data)[k + 3]/255.0f;

                    k += 4;
                } break;
                case rf_uncompressed_r8g8b8:
                {
                    pixels[i].x = (float)((unsigned char* )image.data)[k]/255.0f;
                    pixels[i].y = (float)((unsigned char* )image.data)[k + 1]/255.0f;
                    pixels[i].z = (float)((unsigned char* )image.data)[k + 2]/255.0f;
                    pixels[i].w = 1.0f;

                    k += 3;
                } break;
                case rf_uncompressed_r32:
                {
                    pixels[i].x = ((float* )image.data)[k];
                    pixels[i].y = 0.0f;
                    pixels[i].z = 0.0f;
                    pixels[i].w = 1.0f;

                } break;
                case rf_uncompressed_r32g32b32:
                {
                    pixels[i].x = ((float* )image.data)[k];
                    pixels[i].y = ((float* )image.data)[k + 1];
                    pixels[i].z = ((float* )image.data)[k + 2];
                    pixels[i].w = 1.0f;

                    k += 3;
                } break;
                case rf_uncompressed_r32g32b32a32:
                {
                    pixels[i].x = ((float* )image.data)[k];
                    pixels[i].y = ((float* )image.data)[k + 1];
                    pixels[i].z = ((float* )image.data)[k + 2];
                    pixels[i].w = ((float* )image.data)[k + 3];

                    k += 4;
                }
                default: break;
            }
        }
    }

    return pixels;
}

// Get image alpha border rectangle
RF_API rf_rectangle rf_get_image_alpha_border(rf_image image, float threshold)
{
    rf_rectangle crop = { 0 };

    rf_color* pixels = rf_get_image_data(image);

    if (pixels != NULL)
    {
        int xMin = 65536; // Define a big enough number
        int xMax = 0;
        int yMin = 65536;
        int yMax = 0;

        for (int y = 0; y < image.height; y++)
        {
            for (int x = 0; x < image.width; x++)
            {
                if (pixels[y*image.width + x].a > (unsigned char)(threshold*255.0f))
                {
                    if (x < xMin) xMin = x;
                    if (x > xMax) xMax = x;
                    if (y < yMin) yMin = y;
                    if (y > yMax) yMax = y;
                }
            }
        }

        crop = RF_CLITERAL(rf_rectangle){ xMin, yMin, (xMax + 1) - xMin, (yMax + 1) - yMin };

        RF_FREE(pixels);
    }

    return crop;
}

// Get pixel data size in bytes (image or texture)
// NOTE: Size depends on pixel format
RF_API int rf_get_pixel_data_size(int width, int height, int format)
{
    int dataSize = 0; // Size in bytes
    int bpp = 0; // Bits per pixel

    switch (format)
    {
        case rf_uncompressed_grayscale: bpp = 8; break;
        case rf_uncompressed_gray_alpha:
        case rf_uncompressed_r5g6b5:
        case rf_uncompressed_r5g5b5a1:
        case rf_uncompressed_r4g4b4a4: bpp = 16; break;
        case rf_uncompressed_r8g8b8a8: bpp = 32; break;
        case rf_uncompressed_r8g8b8: bpp = 24; break;
        case rf_uncompressed_r32: bpp = 32; break;
        case rf_uncompressed_r32g32b32: bpp = 32*3; break;
        case rf_uncompressed_r32g32b32a32: bpp = 32*4; break;
        case rf_compressed_dxt1_rgb:
        case rf_compressed_dxt1_rgba:
        case rf_compressed_etc1_rgb:
        case rf_compressed_etc2_rgb:
        case rf_compressed_pvrt_rgb:
        case rf_compressed_pvrt_rgba: bpp = 4; break;
        case rf_compressed_dxt3_rgba:
        case rf_compressed_dxt5_rgba:
        case rf_compressed_etc2_eac_rgba:
        case rf_compressed_astc_4x4_rgba: bpp = 8; break;
        case rf_compressed_astc_8x8_rgba: bpp = 2; break;
        default: break;
    }

    dataSize = width*height*bpp/8; // Total data size in bytes

    return dataSize;
}

// Get pixel data from GPU texture and return an rf_image
// NOTE: Compressed texture formats not supported
RF_API rf_image rf_get_texture_data(rf_texture2d texture)
{
    rf_image image = { 0 };

    if (texture.format < 8)
    {
        image.data = rf_gl_read_texture_pixels(texture);

        if (image.data != NULL)
        {
            image.width = texture.width;
            image.height = texture.height;
            image.format = texture.format;
            image.mipmaps = 1;

            // NOTE: Data retrieved on OpenGL ES 2.0 should be RGBA
            // coming from FBO color buffer, but it seems original
            // texture format is retrieved on RPI... weird...
            //image.format = rf_uncompressed_r8g8b8a8;

            RF_LOG(RF_LOG_INFO, "rf_texture pixel data obtained successfully");
        }
        else RF_LOG(RF_LOG_WARNING, "rf_texture pixel data could not be obtained");
    }
    else RF_LOG(RF_LOG_WARNING, "Compressed texture data could not be obtained");

    return image;
}

// Get pixel data from GPU frontbuffer and return an rf_image (screenshot)
RF_API rf_image rf_get_screen_data()
{
    rf_image image = { 0 };

    image.width = _rf_global_context_ptr->screen_width;
    image.height = _rf_global_context_ptr->screen_height;
    image.mipmaps = 1;
    image.format = rf_uncompressed_r8g8b8a8;
    image.data = rf_gl_read_screen_pixels(image.width, image.height);

    return image;
}

// Update GPU texture with new data
// NOTE: pixels data must match texture.format
RF_API void rf_update_texture(rf_texture2d texture, const void* pixels)
{
    rf_gl_update_texture(texture.id, texture.width, texture.height, texture.format, pixels);
}

// Export image data to file
// NOTE: File format depends on fileName extension
RF_API void rf_export_image(rf_image image, const char* fileName)
{
    int success = 0;

    // NOTE: Getting rf_color array as RGBA unsigned char values
    unsigned char* imgData = (unsigned char*)rf_get_image_data(image);

    if (_rf_is_file_extension(fileName, ".png")) success = stbi_write_png(fileName, image.width, image.height, 4, imgData, image.width*4);
    else if (_rf_is_file_extension(fileName, ".bmp")) success = stbi_write_bmp(fileName, image.width, image.height, 4, imgData);
    else if (_rf_is_file_extension(fileName, ".tga")) success = stbi_write_tga(fileName, image.width, image.height, 4, imgData);
    else if (_rf_is_file_extension(fileName, ".raw"))
    {
        // Export raw pixel data (without header)
        // NOTE: It's up to the user to track image parameters
        FILE* rawFile = fopen(fileName, "wb");
        success = fwrite(image.data, rf_get_pixel_data_size(image.width, image.height, image.format), 1, rawFile);
        fclose(rawFile);
    }

    RF_FREE(imgData);

    if (success != 0) RF_LOG(RF_LOG_INFO, "rf_image exported successfully: %s", fileName);
    else RF_LOG(RF_LOG_WARNING, "rf_image could not be exported.");
}

// Copy an image to a new image
RF_API rf_image rf_image_copy(rf_image image)
{
    rf_image newImage = { 0 };

    int width = image.width;
    int height = image.height;
    int size = 0;

    for (int i = 0; i < image.mipmaps; i++)
    {
        size += rf_get_pixel_data_size(width, height, image.format);

        width /= 2;
        height /= 2;

        // Security check for NPOT textures
        if (width < 1) width = 1;
        if (height < 1) height = 1;
    }

    newImage.data = RF_MALLOC(size);

    if (newImage.data != NULL)
    {
        // NOTE: Size must be provided in bytes
        memcpy(newImage.data, image.data, size);

        newImage.width = image.width;
        newImage.height = image.height;
        newImage.mipmaps = image.mipmaps;
        newImage.format = image.format;
    }

    return newImage;
}

// Create an image from another image piece
RF_API rf_image rf_image_from_image(rf_image image, rf_rectangle rec)
{
    rf_image result = rf_image_copy(image);

    rf_image_crop(&result, rec);

    return result;
}

// Convert image to POT (power-of-two)
// NOTE: It could be useful on OpenGL ES 2.0 (RPI, HTML5)
RF_API void rf_image_to_pot(rf_image* image, rf_color fillColor)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    rf_color* pixels = rf_get_image_data(*image); // Get pixels data

    // Calculate next power-of-two values
    // NOTE: Just add the required amount of pixels at the right and bottom sides of image...
    int potWidth = (int)powf(2, ceilf(logf((float)image->width)/logf(2)));
    int potHeight = (int)powf(2, ceilf(logf((float)image->height)/logf(2)));

    // Check if POT texture generation is required (if texture is not already POT)
    if ((potWidth != image->width) || (potHeight != image->height))
    {
        rf_color* pixelsPOT = NULL;

        // Generate POT array from NPOT data
        pixelsPOT = (rf_color* )RF_MALLOC(potWidth*potHeight*sizeof(rf_color));

        for (int j = 0; j < potHeight; j++)
        {
            for (int i = 0; i < potWidth; i++)
            {
                if ((j < image->height) && (i < image->width)) pixelsPOT[j*potWidth + i] = pixels[j*image->width + i];
                else pixelsPOT[j*potWidth + i] = fillColor;
            }
        }

        RF_LOG(RF_LOG_WARNING, "rf_image converted to POT: (%ix%i) -> (%ix%i)", image->width, image->height, potWidth, potHeight);

        RF_FREE(pixels); // Free pixels data
        RF_FREE(image->data); // Free old image data

        int format = image->format; // Store image data format to reconvert later

        // NOTE: rf_image size changes, new width and height
        *image = rf_load_image_ex(pixelsPOT, potWidth, potHeight);

        RF_FREE(pixelsPOT); // Free POT pixels data

        rf_image_format(image, format); // Reconvert image to previous format
    }
}

// Convert image data to desired format
RF_API void rf_image_format(rf_image* image, int newFormat)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    if ((newFormat != 0) && (image->format != newFormat))
    {
        if ((image->format < rf_compressed_dxt1_rgb) && (newFormat < rf_compressed_dxt1_rgb))
        {
            rf_vector4* pixels = rf_get_image_data_normalized(*image); // Supports 8 to 32 bit per channel

            RF_FREE(image->data); // WARNING! We loose mipmaps data --> Regenerated at the end...
            image->data = NULL;
            image->format = newFormat;

            int k = 0;

            switch (image->format)
            {
                case rf_uncompressed_grayscale:
                {
                    image->data = (unsigned char* )RF_MALLOC(image->width*image->height*sizeof(unsigned char));

                    for (int i = 0; i < image->width*image->height; i++)
                    {
                        ((unsigned char* )image->data)[i] = (unsigned char)((pixels[i].x*0.299f + pixels[i].y*0.587f + pixels[i].z*0.114f)*255.0f);
                    }

                } break;
                case rf_uncompressed_gray_alpha:
                {
                    image->data = (unsigned char* )RF_MALLOC(image->width*image->height*2*sizeof(unsigned char));

                    for (int i = 0; i < image->width*image->height*2; i += 2, k++)
                    {
                        ((unsigned char* )image->data)[i] = (unsigned char)((pixels[k].x*0.299f + (float)pixels[k].y*0.587f + (float)pixels[k].z*0.114f)*255.0f);
                        ((unsigned char* )image->data)[i + 1] = (unsigned char)(pixels[k].w*255.0f);
                    }

                } break;
                case rf_uncompressed_r5g6b5:
                {
                    image->data = (unsigned short *)RF_MALLOC(image->width*image->height*sizeof(unsigned short));

                    unsigned char r = 0;
                    unsigned char g = 0;
                    unsigned char b = 0;

                    for (int i = 0; i < image->width*image->height; i++)
                    {
                        r = (unsigned char)(round(pixels[i].x*31.0f));
                        g = (unsigned char)(round(pixels[i].y*63.0f));
                        b = (unsigned char)(round(pixels[i].z*31.0f));

                        ((unsigned short *)image->data)[i] = (unsigned short)r << 11 | (unsigned short)g << 5 | (unsigned short)b;
                    }

                } break;
                case rf_uncompressed_r8g8b8:
                {
                    image->data = (unsigned char* )RF_MALLOC(image->width*image->height*3*sizeof(unsigned char));

                    for (int i = 0, kk = 0; i < image->width * image->height * 3; i += 3, kk++)
                    {
                        ((unsigned char* )image->data)[i] = (unsigned char)(pixels[kk].x * 255.0f);
                        ((unsigned char* )image->data)[i + 1] = (unsigned char)(pixels[kk].y * 255.0f);
                        ((unsigned char* )image->data)[i + 2] = (unsigned char)(pixels[kk].z * 255.0f);
                    }
                } break;
                case rf_uncompressed_r5g5b5a1:
                {
                    int ALPHA_THRESHOLD = 50;

                    image->data = (unsigned short *)RF_MALLOC(image->width*image->height*sizeof(unsigned short));

                    unsigned char r = 0;
                    unsigned char g = 0;
                    unsigned char b = 0;
                    unsigned char a = 0;

                    for (int i = 0; i < image->width*image->height; i++)
                    {
                        r = (unsigned char)(round(pixels[i].x*31.0f));
                        g = (unsigned char)(round(pixels[i].y*31.0f));
                        b = (unsigned char)(round(pixels[i].z*31.0f));
                        a = (pixels[i].w > ((float)ALPHA_THRESHOLD/255.0f))? 1 : 0;

                        ((unsigned short *)image->data)[i] = (unsigned short)r << 11 | (unsigned short)g << 6 | (unsigned short)b << 1 | (unsigned short)a;
                    }

                } break;
                case rf_uncompressed_r4g4b4a4:
                {
                    image->data = (unsigned short *)RF_MALLOC(image->width*image->height*sizeof(unsigned short));

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
                case rf_uncompressed_r8g8b8a8:
                {
                    image->data = (unsigned char* )RF_MALLOC(image->width*image->height*4*sizeof(unsigned char));

                    for (int i = 0, kk = 0; i < image->width * image->height * 4; i += 4, kk++)
                    {
                        ((unsigned char* )image->data)[i] = (unsigned char)(pixels[kk].x * 255.0f);
                        ((unsigned char* )image->data)[i + 1] = (unsigned char)(pixels[kk].y * 255.0f);
                        ((unsigned char* )image->data)[i + 2] = (unsigned char)(pixels[kk].z * 255.0f);
                        ((unsigned char* )image->data)[i + 3] = (unsigned char)(pixels[kk].w * 255.0f);
                    }
                } break;
                case rf_uncompressed_r32:
                {
                    // WARNING: rf_image is converted to GRAYSCALE eqeuivalent 32bit

                    image->data = (float* )RF_MALLOC(image->width*image->height*sizeof(float));

                    for (int i = 0; i < image->width*image->height; i++)
                    {
                        ((float* )image->data)[i] = (float)(pixels[i].x*0.299f + pixels[i].y*0.587f + pixels[i].z*0.114f);
                    }
                } break;
                case rf_uncompressed_r32g32b32:
                {
                    image->data = (float* )RF_MALLOC(image->width*image->height*3*sizeof(float));

                    for (int i = 0, kk = 0; i < image->width * image->height * 3; i += 3, kk++)
                    {
                        ((float* )image->data)[i] = pixels[kk].x;
                        ((float* )image->data)[i + 1] = pixels[kk].y;
                        ((float* )image->data)[i + 2] = pixels[kk].z;
                    }
                } break;
                case rf_uncompressed_r32g32b32a32:
                {
                    image->data = (float* )RF_MALLOC(image->width*image->height*4*sizeof(float));

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

            RF_FREE(pixels);
            pixels = NULL;

            // In case original image had mipmaps, generate mipmaps for formated image
            // NOTE: Original mipmaps are replaced by new ones, if custom mipmaps were used, they are lost
            if (image->mipmaps > 1)
            {
                image->mipmaps = 1;

                if (image->data != NULL) rf_image_mipmaps(image);

            }
        }
        else RF_LOG(RF_LOG_WARNING, "rf_image data format is compressed, can not be converted");
    }
}

// Apply alpha mask to image
// NOTE 1: Returned image is GRAY_ALPHA (16bit) or RGBA (32bit)
// NOTE 2: alphaMask should be same size as image
RF_API void rf_image_alpha_mask(rf_image* image, rf_image alphaMask)
{
    if ((image->width != alphaMask.width) || (image->height != alphaMask.height))
    {
        RF_LOG(RF_LOG_WARNING, "Alpha mask must be same size as image");
    }
    else if (image->format >= rf_compressed_dxt1_rgb)
    {
        RF_LOG(RF_LOG_WARNING, "Alpha mask can not be applied to compressed data formats");
    }
    else
    {
        // Force mask to be Grayscale
        rf_image mask = rf_image_copy(alphaMask);
        if (mask.format != rf_uncompressed_grayscale) rf_image_format(&mask, rf_uncompressed_grayscale);

        // In case image is only grayscale, we just add alpha channel
        if (image->format == rf_uncompressed_grayscale)
        {
            unsigned char* data = (unsigned char* )RF_MALLOC(image->width*image->height*2);

            // Apply alpha mask to alpha channel
            for (int i = 0, k = 0; (i < mask.width*mask.height) || (i < image->width*image->height); i++, k += 2)
            {
                data[k] = ((unsigned char* )image->data)[i];
                data[k + 1] = ((unsigned char* )mask.data)[i];
            }

            RF_FREE(image->data);
            image->data = data;
            image->format = rf_uncompressed_gray_alpha;
        }
        else
        {
            // Convert image to RGBA
            if (image->format != rf_uncompressed_r8g8b8a8) rf_image_format(image, rf_uncompressed_r8g8b8a8);

            // Apply alpha mask to alpha channel
            for (int i = 0, k = 3; (i < mask.width*mask.height) || (i < image->width*image->height); i++, k += 4)
            {
                ((unsigned char* )image->data)[k] = ((unsigned char* )mask.data)[i];
            }
        }

        rf_unload_image(mask);
    }
}

// Clear alpha channel to desired color
// NOTE: Threshold defines the alpha limit, 0.0f to 1.0f
RF_API void rf_image_alpha_clear(rf_image* image, rf_color color, float threshold)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    rf_color* pixels = rf_get_image_data(*image);

    for (int i = 0; i < image->width*image->height; i++) if (pixels[i].a <= (unsigned char)(threshold*255.0f)) pixels[i] = color;

    rf_unload_image(*image);

    int prevFormat = image->format;
    *image = rf_load_image_ex(pixels, image->width, image->height);

    rf_image_format(image, prevFormat);
}

// Premultiply alpha channel
RF_API void rf_image_alpha_premultiply(rf_image* image)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    float alpha = 0.0f;
    rf_color* pixels = rf_get_image_data(*image);

    for (int i = 0; i < image->width*image->height; i++)
    {
        alpha = (float)pixels[i].a/255.0f;
        pixels[i].r = (unsigned char)((float)pixels[i].r*alpha);
        pixels[i].g = (unsigned char)((float)pixels[i].g*alpha);
        pixels[i].b = (unsigned char)((float)pixels[i].b*alpha);
    }

    rf_unload_image(*image);

    int prevFormat = image->format;
    *image = rf_load_image_ex(pixels, image->width, image->height);

    rf_image_format(image, prevFormat);
}

// Load cubemap from image, multiple image cubemap layouts supported
RF_API rf_texture_cubemap rf_load_texture_cubemap(rf_image image, int layoutType)
{
    rf_texture_cubemap cubemap = { 0 };

    if (layoutType == rf_cubemap_auto_detect) // Try to automatically guess layout type
    {
        // Check image width/height to determine the type of cubemap provided
        if (image.width > image.height)
        {
            if ((image.width/6) == image.height) { layoutType = rf_cubemap_line_horizontal; cubemap.width = image.width/6; }
            else if ((image.width/4) == (image.height/3)) { layoutType = rf_cubemap_cross_four_by_three; cubemap.width = image.width/4; }
            else if (image.width >= (int)((float)image.height*1.85f)) { layoutType = rf_cubemap_panorama; cubemap.width = image.width/4; }
        }
        else if (image.height > image.width)
        {
            if ((image.height/6) == image.width) { layoutType = rf_cubemap_line_vertical; cubemap.width = image.height/6; }
            else if ((image.width/3) == (image.height/4)) { layoutType = rf_cubemap_cross_three_by_four; cubemap.width = image.width/3; }
        }

        cubemap.height = cubemap.width;
    }

    if (layoutType != rf_cubemap_auto_detect)
    {
        int size = cubemap.width;

        rf_image faces = { 0 }; // Vertical column image
        rf_rectangle faceRecs[6] = { 0 }; // Face source rectangles
        for (int i = 0; i < 6; i++) faceRecs[i] = RF_CLITERAL(rf_rectangle){ 0, 0, size, size };

        if (layoutType == rf_cubemap_line_vertical)
        {
            faces = image;
            for (int i = 0; i < 6; i++) faceRecs[i].y = size*i;
        }
        else if (layoutType == rf_cubemap_panorama)
        {
            // TODO: Convert panorama image to square faces...
            // Ref: https://github.com/denivip/panorama/blob/master/panorama.cpp
        }
        else
        {
            if (layoutType == rf_cubemap_line_horizontal) for (int i = 0; i < 6; i++) faceRecs[i].x = size*i;
            else if (layoutType == rf_cubemap_cross_three_by_four)
            {
                faceRecs[0].x = size; faceRecs[0].y = size;
                faceRecs[1].x = size; faceRecs[1].y = 3*size;
                faceRecs[2].x = size; faceRecs[2].y = 0;
                faceRecs[3].x = size; faceRecs[3].y = 2*size;
                faceRecs[4].x = 0; faceRecs[4].y = size;
                faceRecs[5].x = 2*size; faceRecs[5].y = size;
            }
            else if (layoutType == rf_cubemap_cross_four_by_three)
            {
                faceRecs[0].x = 2*size; faceRecs[0].y = size;
                faceRecs[1].x = 0; faceRecs[1].y = size;
                faceRecs[2].x = size; faceRecs[2].y = 0;
                faceRecs[3].x = size; faceRecs[3].y = 2*size;
                faceRecs[4].x = size; faceRecs[4].y = size;
                faceRecs[5].x = 3*size; faceRecs[5].y = size;
            }

            // Convert image data to 6 faces in a vertical column, that's the optimum layout for loading
            faces = rf_gen_image_color(size, size*6, rf_magenta);
            rf_image_format(&faces, image.format);

            // TODO: rf_image formating does not work with compressed textures!
        }

        for (int i = 0; i < 6; i++) rf_image_draw(&faces, image, faceRecs[i], RF_CLITERAL(rf_rectangle){ 0, size*i, size, size }, rf_white);

        cubemap.id = rf_gl_load_texture_cubemap(faces.data, size, faces.format);
        if (cubemap.id == 0) RF_LOG(RF_LOG_WARNING, "Cubemap image could not be loaded.");

        rf_unload_image(faces);
    }
    else RF_LOG(RF_LOG_WARNING, "Cubemap image layout can not be detected.");

    return cubemap;
}

// Crop an image to area defined by a rectangle
// NOTE: Security checks are performed in case rectangle goes out of bounds
RF_API void rf_image_crop(rf_image* image, rf_rectangle crop)
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
        rf_color* pixels = rf_get_image_data(*image); // Get data as rf_color pixels array
        rf_color* cropPixels = (rf_color* )RF_MALLOC((int)crop.width*(int)crop.height*sizeof(rf_color));

        for (int j = (int)crop.y; j < (int)(crop.y + crop.height); j++)
        {
            for (int i = (int)crop.x; i < (int)(crop.x + crop.width); i++)
            {
                cropPixels[(j - (int)crop.y)*(int)crop.width + (i - (int)crop.x)] = pixels[j*image->width + i];
            }
        }

        RF_FREE(pixels);

        int format = image->format;

        rf_unload_image(*image);

        *image = rf_load_image_ex(cropPixels, (int)crop.width, (int)crop.height);

        RF_FREE(cropPixels);

        // Reformat 32bit RGBA image to original format
        rf_image_format(image, format);
    }
    else RF_LOG(RF_LOG_WARNING, "rf_image can not be cropped, crop rectangle out of bounds");
}

// Crop image depending on alpha value
RF_API void rf_image_alpha_crop(rf_image* image, float threshold)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    rf_color* pixels = rf_get_image_data(*image);

    int xMin = 65536; // Define a big enough number
    int xMax = 0;
    int yMin = 65536;
    int yMax = 0;

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            if (pixels[y*image->width + x].a > (unsigned char)(threshold*255.0f))
            {
                if (x < xMin) xMin = x;
                if (x > xMax) xMax = x;
                if (y < yMin) yMin = y;
                if (y > yMax) yMax = y;
            }
        }
    }

    rf_rectangle crop = { xMin, yMin, (xMax + 1) - xMin, (yMax + 1) - yMin };

    RF_FREE(pixels);

    // Check for not empty image brefore cropping
    if (!((xMax < xMin) || (yMax < yMin))) rf_image_crop(image, crop);
}

// Resize and image to new size
// NOTE: Uses stb default scaling filters (both bicubic):
// STBIR_DEFAULT_FILTER_UPSAMPLE    STBIR_FILTER_CATMULLROM
// STBIR_DEFAULT_FILTER_DOWNSAMPLE  STBIR_FILTER_MITCHELL   (high-quality Catmull-Rom)
RF_API void rf_image_resize(rf_image* image, int newWidth, int newHeight)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    // Get data as rf_color pixels array to work with it
    rf_color* pixels = rf_get_image_data(*image);
    rf_color* output = (rf_color* )RF_MALLOC(newWidth*newHeight*sizeof(rf_color));

    // NOTE: rf_color data is casted to (unsigned char* ), there shouldn't been any problem...
    stbir_resize_uint8((unsigned char* )pixels, image->width, image->height, 0, (unsigned char* )output, newWidth, newHeight, 0, 4);

    int format = image->format;

    rf_unload_image(*image);

    *image = rf_load_image_ex(output, newWidth, newHeight);
    rf_image_format(image, format); // Reformat 32bit RGBA image to original format

    RF_FREE(output);
    RF_FREE(pixels);
}

// Resize and image to new size using Nearest-Neighbor scaling algorithm
RF_API void rf_image_resize_nn(rf_image* image, int newWidth, int newHeight)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    rf_color* pixels = rf_get_image_data(*image);
    rf_color* output = (rf_color* )RF_MALLOC(newWidth*newHeight*sizeof(rf_color));

    // EDIT: added +1 to account for an early rounding problem
    int xRatio = (int)((image->width << 16)/newWidth) + 1;
    int yRatio = (int)((image->height << 16)/newHeight) + 1;

    int x2, y2;
    for (int y = 0; y < newHeight; y++)
    {
        for (int x = 0; x < newWidth; x++)
        {
            x2 = ((x*xRatio) >> 16);
            y2 = ((y*yRatio) >> 16);

            output[(y*newWidth) + x] = pixels[(y2*image->width) + x2] ;
        }
    }

    int format = image->format;

    rf_unload_image(*image);

    *image = rf_load_image_ex(output, newWidth, newHeight);
    rf_image_format(image, format); // Reformat 32bit RGBA image to original format

    RF_FREE(output);
    RF_FREE(pixels);
}

// Resize canvas and fill with color
// NOTE: Resize offset is relative to the top-left corner of the original image
RF_API void rf_image_resize_canvas(rf_image* image, int newWidth, int newHeight, int offset_x, int offset_y, rf_color color)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    if ((newWidth != image->width) || (newHeight != image->height))
    {
        // Support offsets out of canvas new size -> original image is cropped
        if (offset_x < 0)
        {
            rf_image_crop(image, RF_CLITERAL(rf_rectangle) { -offset_x, 0, image->width + offset_x, image->height });
            offset_x = 0;
        }
        else if (offset_x > (newWidth - image->width))
        {
            rf_image_crop(image, RF_CLITERAL(rf_rectangle) { 0, 0, image->width - (offset_x - (newWidth - image->width)), image->height });
            offset_x = newWidth - image->width;
        }

        if (offset_y < 0)
        {
            rf_image_crop(image, RF_CLITERAL(rf_rectangle) { 0, -offset_y, image->width, image->height + offset_y });
            offset_y = 0;
        }
        else if (offset_y > (newHeight - image->height))
        {
            rf_image_crop(image, RF_CLITERAL(rf_rectangle) { 0, 0, image->width, image->height - (offset_y - (newHeight - image->height)) });
            offset_y = newHeight - image->height;
        }

        if ((newWidth > image->width) && (newHeight > image->height))
        {
            rf_image imTemp = rf_gen_image_color(newWidth, newHeight, color);

            rf_rectangle srcRec = { 0.0f, 0.0f, (float)image->width, (float)image->height };
            rf_rectangle dstRec = { (float)offset_x, (float)offset_y, srcRec.width, srcRec.height };

            rf_image_draw(&imTemp, *image, srcRec, dstRec, rf_white);
            rf_image_format(&imTemp, image->format);
            rf_unload_image(*image);
            *image = imTemp;
        }
        else if ((newWidth < image->width) && (newHeight < image->height))
        {
            rf_rectangle crop = { (float)offset_x, (float)offset_y, (float)newWidth, (float)newHeight };
            rf_image_crop(image, crop);
        }
        else // One side is bigger and the other is smaller
        {
            rf_image imTemp = rf_gen_image_color(newWidth, newHeight, color);

            rf_rectangle srcRec = { 0.0f, 0.0f, (float)image->width, (float)image->height };
            rf_rectangle dstRec = { (float)offset_x, (float)offset_y, (float)image->width, (float)image->height };

            if (newWidth < image->width)
            {
                srcRec.x = offset_x;
                srcRec.width = newWidth;
                dstRec.x = 0.0f;
            }

            if (newHeight < image->height)
            {
                srcRec.y = offset_y;
                srcRec.height = newHeight;
                dstRec.y = 0.0f;
            }

            rf_image_draw(&imTemp, *image, srcRec, dstRec, rf_white);
            rf_image_format(&imTemp, image->format);
            rf_unload_image(*image);
            *image = imTemp;
        }
    }
}

// Generate all mipmap levels for a provided image
// NOTE 1: Supports POT and NPOT images
// NOTE 2: image.data is scaled to include mipmap levels
// NOTE 3: Mipmaps format is the same as base image
RF_API void rf_image_mipmaps(rf_image* image)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    int mipCount = 1; // Required mipmap levels count (including base level)
    int mipWidth = image->width; // Base image width
    int mipHeight = image->height; // Base image height
    int mipSize = rf_get_pixel_data_size(mipWidth, mipHeight, image->format); // rf_image data size (in bytes)

    // Count mipmap levels required
    while ((mipWidth != 1) || (mipHeight != 1))
    {
        if (mipWidth != 1) mipWidth /= 2;
        if (mipHeight != 1) mipHeight /= 2;

        // Security check for NPOT textures
        if (mipWidth < 1) mipWidth = 1;
        if (mipHeight < 1) mipHeight = 1;

        RF_LOG(RF_LOG_DEBUG, "Next mipmap level: %i x %i - current size %i", mipWidth, mipHeight, mipSize);

        mipCount++;
        mipSize += rf_get_pixel_data_size(mipWidth, mipHeight, image->format); // Add mipmap size (in bytes)
    }

    RF_LOG(RF_LOG_DEBUG, "Mipmaps available: %i - Mipmaps required: %i", image->mipmaps, mipCount);
    RF_LOG(RF_LOG_DEBUG, "Mipmaps total size required: %i", mipSize);
    RF_LOG(RF_LOG_DEBUG, "rf_image data memory start address: 0x%x", image->data);

    if (image->mipmaps < mipCount)
    {
        void* temp = realloc(image->data, mipSize);

        if (temp != NULL)
        {
            image->data = temp; // Assign new pointer (new size) to store mipmaps data
            RF_LOG(RF_LOG_DEBUG, "rf_image data memory point reallocated: 0x%x", temp);
        }
        else RF_LOG(RF_LOG_WARNING, "Mipmaps required memory could not be allocated");

        // Pointer to allocated memory point where store next mipmap level data
        unsigned char* nextmip = (unsigned char* )image->data + rf_get_pixel_data_size(image->width, image->height, image->format);

        mipWidth = image->width/2;
        mipHeight = image->height/2;
        mipSize = rf_get_pixel_data_size(mipWidth, mipHeight, image->format);
        rf_image imCopy = rf_image_copy(*image);

        for (int i = 1; i < mipCount; i++)
        {
            RF_LOG(RF_LOG_DEBUG, "Gen mipmap level: %i (%i x %i) - size: %i - offset: 0x%x", i, mipWidth, mipHeight, mipSize, nextmip);

            rf_image_resize(&imCopy, mipWidth, mipHeight); // Uses internally Mitchell cubic downscale filter

            memcpy(nextmip, imCopy.data, mipSize);
            nextmip += mipSize;
            image->mipmaps++;

            mipWidth /= 2;
            mipHeight /= 2;

            // Security check for NPOT textures
            if (mipWidth < 1) mipWidth = 1;
            if (mipHeight < 1) mipHeight = 1;

            mipSize = rf_get_pixel_data_size(mipWidth, mipHeight, image->format);
        }

        rf_unload_image(imCopy);
    }
    else RF_LOG(RF_LOG_WARNING, "rf_image mipmaps already available");
}

// Dither image data to 16bpp or lower (Floyd-Steinberg dithering)
// NOTE: In case selected bpp do not represent an known 16bit format,
// dithered data is stored in the LSB part of the unsigned short
RF_API void rf_image_dither(rf_image* image, int rBpp, int gBpp, int bBpp, int aBpp)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    if (image->format >= rf_compressed_dxt1_rgb)
    {
        RF_LOG(RF_LOG_WARNING, "Compressed data formats can not be dithered");
        return;
    }

    if ((rBpp + gBpp + bBpp + aBpp) > 16)
    {
        RF_LOG(RF_LOG_WARNING, "Unsupported dithering bpps (%ibpp), only 16bpp or lower modes supported", (rBpp+gBpp+bBpp+aBpp));
    }
    else
    {
        rf_color* pixels = rf_get_image_data(*image);

        RF_FREE(image->data); // free old image data

        if ((image->format != rf_uncompressed_r8g8b8) && (image->format != rf_uncompressed_r8g8b8a8))
        {
            RF_LOG(RF_LOG_WARNING, "rf_image format is already 16bpp or lower, dithering could have no effect");
        }

        // Define new image format, check if desired bpp match internal known format
        if ((rBpp == 5) && (gBpp == 6) && (bBpp == 5) && (aBpp == 0)) image->format = rf_uncompressed_r5g6b5;
        else if ((rBpp == 5) && (gBpp == 5) && (bBpp == 5) && (aBpp == 1)) image->format = rf_uncompressed_r5g5b5a1;
        else if ((rBpp == 4) && (gBpp == 4) && (bBpp == 4) && (aBpp == 4)) image->format = rf_uncompressed_r4g4b4a4;
        else
        {
            image->format = 0;
            RF_LOG(RF_LOG_WARNING, "Unsupported dithered OpenGL internal format: %ibpp (R%iG%iB%iA%i)", (rBpp+gBpp+bBpp+aBpp), rBpp, gBpp, bBpp, aBpp);
        }

        // NOTE: We will store the dithered data as unsigned short (16bpp)
        image->data = (unsigned short *)RF_MALLOC(image->width*image->height*sizeof(unsigned short));

        rf_color oldPixel = rf_white;
        rf_color newPixel = rf_white;

        int rError, gError, bError;
        unsigned short rPixel, gPixel, bPixel, aPixel; // Used for 16bit pixel composition

#define rf_raylib_min(a,b) (((a)<(b))?(a):(b))

        for (int y = 0; y < image->height; y++)
        {
            for (int x = 0; x < image->width; x++)
            {
                oldPixel = pixels[y*image->width + x];

                // NOTE: New pixel obtained by bits truncate, it would be better to round values (check rf_image_format())
                newPixel.r = oldPixel.r >> (8 - rBpp); // R bits
                newPixel.g = oldPixel.g >> (8 - gBpp); // G bits
                newPixel.b = oldPixel.b >> (8 - bBpp); // B bits
                newPixel.a = oldPixel.a >> (8 - aBpp); // A bits (not used on dithering)

                // NOTE: Error must be computed between new and old pixel but using same number of bits!
                // We want to know how much color precision we have lost...
                rError = (int)oldPixel.r - (int)(newPixel.r << (8 - rBpp));
                gError = (int)oldPixel.g - (int)(newPixel.g << (8 - gBpp));
                bError = (int)oldPixel.b - (int)(newPixel.b << (8 - bBpp));

                pixels[y*image->width + x] = newPixel;

                // NOTE: Some cases are out of the array and should be ignored
                if (x < (image->width - 1))
                {
                    pixels[y*image->width + x+1].r = rf_raylib_min((int)pixels[y*image->width + x+1].r + (int)((float)rError*7.0f/16), 0xff);
                    pixels[y*image->width + x+1].g = rf_raylib_min((int)pixels[y*image->width + x+1].g + (int)((float)gError*7.0f/16), 0xff);
                    pixels[y*image->width + x+1].b = rf_raylib_min((int)pixels[y*image->width + x+1].b + (int)((float)bError*7.0f/16), 0xff);
                }

                if ((x > 0) && (y < (image->height - 1)))
                {
                    pixels[(y+1)*image->width + x-1].r = rf_raylib_min((int)pixels[(y+1)*image->width + x-1].r + (int)((float)rError*3.0f/16), 0xff);
                    pixels[(y+1)*image->width + x-1].g = rf_raylib_min((int)pixels[(y+1)*image->width + x-1].g + (int)((float)gError*3.0f/16), 0xff);
                    pixels[(y+1)*image->width + x-1].b = rf_raylib_min((int)pixels[(y+1)*image->width + x-1].b + (int)((float)bError*3.0f/16), 0xff);
                }

                if (y < (image->height - 1))
                {
                    pixels[(y+1)*image->width + x].r = rf_raylib_min((int)pixels[(y+1)*image->width + x].r + (int)((float)rError*5.0f/16), 0xff);
                    pixels[(y+1)*image->width + x].g = rf_raylib_min((int)pixels[(y+1)*image->width + x].g + (int)((float)gError*5.0f/16), 0xff);
                    pixels[(y+1)*image->width + x].b = rf_raylib_min((int)pixels[(y+1)*image->width + x].b + (int)((float)bError*5.0f/16), 0xff);
                }

                if ((x < (image->width - 1)) && (y < (image->height - 1)))
                {
                    pixels[(y+1)*image->width + x+1].r = rf_raylib_min((int)pixels[(y+1)*image->width + x+1].r + (int)((float)rError*1.0f/16), 0xff);
                    pixels[(y+1)*image->width + x+1].g = rf_raylib_min((int)pixels[(y+1)*image->width + x+1].g + (int)((float)gError*1.0f/16), 0xff);
                    pixels[(y+1)*image->width + x+1].b = rf_raylib_min((int)pixels[(y+1)*image->width + x+1].b + (int)((float)bError*1.0f/16), 0xff);
                }

                rPixel = (unsigned short)newPixel.r;
                gPixel = (unsigned short)newPixel.g;
                bPixel = (unsigned short)newPixel.b;
                aPixel = (unsigned short)newPixel.a;

                ((unsigned short *)image->data)[y*image->width + x] = (rPixel << (gBpp + bBpp + aBpp)) | (gPixel << (bBpp + aBpp)) | (bPixel << aBpp) | aPixel;
            }

#undef rf_raylib_min
        }

        RF_FREE(pixels);
    }
}

// Extract color palette from image to maximum size
// NOTE: Memory allocated should be freed manually!
RF_API rf_color* rf_image_extract_palette(rf_image image, int maxPaletteSize, int* extractCount)
{
    #define rf_color_equal(col1, col2) ((col1.r == col2.r)&&(col1.g == col2.g)&&(col1.b == col2.b)&&(col1.a == col2.a))

    rf_color* pixels = rf_get_image_data(image);
    rf_color* palette = (rf_color* )RF_MALLOC(maxPaletteSize*sizeof(rf_color));

    int palCount = 0;
    for (int i = 0; i < maxPaletteSize; i++) palette[i] = rf_blank; // Set all colors to rf_blank

    for (int i = 0; i < image.width*image.height; i++)
    {
        if (pixels[i].a > 0)
        {
            bool colorInPalette = false;

            // Check if the color is already on palette
            for (int j = 0; j < maxPaletteSize; j++)
            {
                if (rf_color_equal(pixels[i], palette[j]))
                {
                    colorInPalette = true;
                    break;
                }
            }

            // Store color if not on the palette
            if (!colorInPalette)
            {
                palette[palCount] = pixels[i]; // Add pixels[i] to palette
                palCount++;

                // We reached the limit of colors supported by palette
                if (palCount >= maxPaletteSize)
                {
                    i = image.width*image.height; // Finish palette get
                    RF_LOG(RF_LOG_WARNING, "rf_image palette is greater than %i colors!", maxPaletteSize);
                }
            }
        }
    }

    RF_FREE(pixels);

    *extractCount = palCount;

    return palette;
}

// Draw an image (source) within an image (destination)
// NOTE: rf_color tint is applied to source image
RF_API void rf_image_draw(rf_image* dst, rf_image src, rf_rectangle srcRec, rf_rectangle dstRec, rf_color tint)
{
    // Security check to avoid program crash
    if ((dst->data == NULL) || (dst->width == 0) || (dst->height == 0) ||
        (src.data == NULL) || (src.width == 0) || (src.height == 0)) return;

    // Security checks to avoid size and rectangle issues (out of bounds)
    // Check that srcRec is inside src image
    if (srcRec.x < 0) srcRec.x = 0;
    if (srcRec.y < 0) srcRec.y = 0;

    if ((srcRec.x + srcRec.width) > src.width)
    {
        srcRec.width = src.width - srcRec.x;
        RF_LOG(RF_LOG_WARNING, "Source rectangle width out of bounds, rescaled width: %i", srcRec.width);
    }

    if ((srcRec.y + srcRec.height) > src.height)
    {
        srcRec.height = src.height - srcRec.y;
        RF_LOG(RF_LOG_WARNING, "Source rectangle height out of bounds, rescaled height: %i", srcRec.height);
    }

    rf_image srcCopy = rf_image_copy(src); // Make a copy of source image to work with it

    // Crop source image to desired source rectangle (if required)
    if ((src.width != (int)srcRec.width) && (src.height != (int)srcRec.height)) rf_image_crop(&srcCopy, srcRec);

    // Scale source image in case destination rec size is different than source rec size
    if (((int)dstRec.width != (int)srcRec.width) || ((int)dstRec.height != (int)srcRec.height))
    {
        rf_image_resize(&srcCopy, (int)dstRec.width, (int)dstRec.height);
    }

    // Check that dstRec is inside dst image
    // Allow negative position within destination with cropping
    if (dstRec.x < 0)
    {
        rf_image_crop(&srcCopy, RF_CLITERAL(rf_rectangle) { -dstRec.x, 0, dstRec.width + dstRec.x, dstRec.height });
        dstRec.width = dstRec.width + dstRec.x;
        dstRec.x = 0;
    }

    if ((dstRec.x + dstRec.width) > dst->width)
    {
        rf_image_crop(&srcCopy, RF_CLITERAL(rf_rectangle) { 0, 0, dst->width - dstRec.x, dstRec.height });
        dstRec.width = dst->width - dstRec.x;
    }

    if (dstRec.y < 0)
    {
        rf_image_crop(&srcCopy, RF_CLITERAL(rf_rectangle) { 0, -dstRec.y, dstRec.width, dstRec.height + dstRec.y });
        dstRec.height = dstRec.height + dstRec.y;
        dstRec.y = 0;
    }

    if ((dstRec.y + dstRec.height) > dst->height)
    {
        rf_image_crop(&srcCopy, RF_CLITERAL(rf_rectangle) { 0, 0, dstRec.width, dst->height - dstRec.y });
        dstRec.height = dst->height - dstRec.y;
    }

    // Get image data as rf_color pixels array to work with it
    rf_color* dstPixels = rf_get_image_data(*dst);
    rf_color* srcPixels = rf_get_image_data(srcCopy);

    rf_unload_image(srcCopy); // Source copy not required any more

    rf_vector4 fsrc, fdst, fout; // Normalized pixel data (ready for operation)
    rf_vector4 ftint = rf_color_normalize(tint); // Normalized color tint

    // Blit pixels, copy source image into destination
    // TODO: Maybe out-of-bounds blitting could be considered here instead of so much cropping
    for (int j = (int)dstRec.y; j < (int)(dstRec.y + dstRec.height); j++)
    {
        for (int i = (int)dstRec.x; i < (int)(dstRec.x + dstRec.width); i++)
        {
            // Alpha blending (https://en.wikipedia.org/wiki/Alpha_compositing)

            fdst = rf_color_normalize(dstPixels[j*(int)dst->width + i]);
            fsrc = rf_color_normalize(srcPixels[(j - (int)dstRec.y)*(int)dstRec.width + (i - (int)dstRec.x)]);

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

            dstPixels[j*(int)dst->width + i] = RF_CLITERAL(rf_color){ (unsigned char)(fout.x*255.0f),
                    (unsigned char)(fout.y*255.0f),
                    (unsigned char)(fout.z*255.0f),
                    (unsigned char)(fout.w*255.0f) };

            // TODO: Support other blending options
        }
    }

    rf_unload_image(*dst);

    *dst = rf_load_image_ex(dstPixels, (int)dst->width, (int)dst->height);
    rf_image_format(dst, dst->format);

    RF_FREE(srcPixels);
    RF_FREE(dstPixels);
}

// Create an image from text (default font)
RF_API rf_image rf_image_text(const char* text, int fontSize, rf_color color)
{
    int size = 10; // Default rf_font chars height in pixel
    if (fontSize < size) fontSize = size;
    int spacing = fontSize / size;

    rf_image imText = rf_image_text_ex(rf_get_font_default(), text, (float)fontSize, (float)spacing, color);

    return imText;
}

// Create an image from text (custom sprite font)
RF_API rf_image rf_image_text_ex(rf_font font, const char* text, float fontSize, float spacing, rf_color tint)
{
    int length = strlen(text);

    int index; // Index position in sprite font
    int letter = 0; // Current character
    int positionX = 0; // rf_image drawing position

    // NOTE: Text image is generated at font base size, later scaled to desired font size
    rf_vector2 imSize = rf_measure_text_ex(font, text, (float)font.base_size, spacing);

    // Create image to store text
    rf_image imText = rf_gen_image_color((int)imSize.x, (int)imSize.y, rf_blank);

    for (int i = 0; i < length; i++)
    {
        int next = 0;
        letter = _rf_get_next_utf8_codepoint(&text[i], &next);
        index = rf_get_glyph_index(font, letter);

        if (letter == 0x3f) next = 1;
        i += (next - 1);

        if (letter == '\n')
        {
            // TODO: Support line break
        }
        else
        {
            if (letter != ' ')
            {
                rf_image_draw(&imText, font.chars[index].image, RF_CLITERAL(rf_rectangle){ 0, 0, font.chars[index].image.width, font.chars[index].image.height },
                              RF_CLITERAL(rf_rectangle){ (float)(positionX + font.chars[index].offset_x),(float)font.chars[index].offset_y,
                                      font.chars[index].image.width, font.chars[index].image.height }, tint);
            }

            if (font.chars[index].advance_x == 0) positionX += (int)(font.recs[index].width + spacing);
            else positionX += font.chars[index].advance_x + (int)spacing;
        }
    }

    // Scale image depending on text size
    if (fontSize > imSize.y)
    {
        float scaleFactor = fontSize/imSize.y;
        RF_LOG(RF_LOG_INFO, "rf_image text scaled by factor: %f", scaleFactor);

        // Using nearest-neighbor scaling algorithm for default font
        if (font.texture.id == rf_get_font_default().texture.id) rf_image_resize_nn(&imText, (int)(imSize.x*scaleFactor), (int)(imSize.y*scaleFactor));
        else rf_image_resize(&imText, (int)(imSize.x*scaleFactor), (int)(imSize.y*scaleFactor));
    }

    return imText;
}

// Draw rectangle within an image
RF_API void rf_image_draw_rectangle(rf_image* dst, rf_rectangle rec, rf_color color)
{
    // Security check to avoid program crash
    if ((dst->data == NULL) || (dst->width == 0) || (dst->height == 0)) return;

    rf_image imRec = rf_gen_image_color((int)rec.width, (int)rec.height, color);
    rf_image_draw(dst, imRec, RF_CLITERAL(rf_rectangle){ 0, 0, rec.width, rec.height }, rec, rf_white);
    rf_unload_image(imRec);
}

// Draw rectangle lines within an image
RF_API void rf_image_draw_rectangle_lines(rf_image* dst, rf_rectangle rec, int thick, rf_color color)
{
    rf_image_draw_rectangle(dst, RF_CLITERAL(rf_rectangle){ rec.x, rec.y, rec.width, thick }, color);
    rf_image_draw_rectangle(dst, RF_CLITERAL(rf_rectangle){ rec.x, rec.y + thick, thick, rec.height - thick*2 }, color);
    rf_image_draw_rectangle(dst, RF_CLITERAL(rf_rectangle){ rec.x + rec.width - thick, rec.y + thick, thick, rec.height - thick*2 }, color);
    rf_image_draw_rectangle(dst, RF_CLITERAL(rf_rectangle){ rec.x, rec.y + rec.height - thick, rec.width, thick }, color);
}

// Draw text (default font) within an image (destination)
RF_API void rf_image_draw_text(rf_image* dst, rf_vector2 position, const char* text, int fontSize, rf_color color)
{
    // NOTE: For default font, sapcing is set to desired font size / default font size (10)
    rf_image_draw_text_ex(dst, position, rf_get_font_default(), text, (float)fontSize, (float)fontSize/10, color);
}

// Draw text (custom sprite font) within an image (destination)
RF_API void rf_image_draw_text_ex(rf_image* dst, rf_vector2 position, rf_font font, const char* text, float fontSize, float spacing, rf_color color)
{
    rf_image imText = rf_image_text_ex(font, text, fontSize, spacing, color);

    rf_rectangle srcRec = { 0.0f, 0.0f, (float)imText.width, (float)imText.height };
    rf_rectangle dstRec = { position.x, position.y, (float)imText.width, (float)imText.height };

    rf_image_draw(dst, imText, srcRec, dstRec, rf_white);

    rf_unload_image(imText);
}

// Flip image vertically
RF_API void rf_image_flip_vertical(rf_image* image)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    rf_color* srcPixels = rf_get_image_data(*image);
    rf_color* dstPixels = (rf_color* )RF_MALLOC(image->width*image->height*sizeof(rf_color));

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            dstPixels[y*image->width + x] = srcPixels[(image->height - 1 - y)*image->width + x];
        }
    }

    rf_image processed = rf_load_image_ex(dstPixels, image->width, image->height);
    rf_image_format(&processed, image->format);
    rf_unload_image(*image);

    RF_FREE(srcPixels);
    RF_FREE(dstPixels);

    image->data = processed.data;
}

// Flip image horizontally
RF_API void rf_image_flip_horizontal(rf_image* image)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    rf_color* srcPixels = rf_get_image_data(*image);
    rf_color* dstPixels = (rf_color* )RF_MALLOC(image->width*image->height*sizeof(rf_color));

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            dstPixels[y*image->width + x] = srcPixels[y*image->width + (image->width - 1 - x)];
        }
    }

    rf_image processed = rf_load_image_ex(dstPixels, image->width, image->height);
    rf_image_format(&processed, image->format);
    rf_unload_image(*image);

    RF_FREE(srcPixels);
    RF_FREE(dstPixels);

    image->data = processed.data;
}

// Rotate image clockwise 90deg
RF_API void rf_image_rotate_cw(rf_image* image)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    rf_color* srcPixels = rf_get_image_data(*image);
    rf_color* rotPixels = (rf_color* )RF_MALLOC(image->width*image->height*sizeof(rf_color));

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            rotPixels[x*image->height + (image->height - y - 1)] = srcPixels[y*image->width + x];
        }
    }

    rf_image processed = rf_load_image_ex(rotPixels, image->height, image->width);
    rf_image_format(&processed, image->format);
    rf_unload_image(*image);

    RF_FREE(srcPixels);
    RF_FREE(rotPixels);

    image->data = processed.data;
    image->width = processed.width;
    image->height = processed.height;
}

// Rotate image counter-clockwise 90deg
RF_API void rf_image_rotate_ccw(rf_image* image)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    rf_color* srcPixels = rf_get_image_data(*image);
    rf_color* rotPixels = (rf_color*)RF_MALLOC(image->width*image->height*sizeof(rf_color));

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            rotPixels[x*image->height + y] = srcPixels[y*image->width + (image->width - x - 1)];
        }
    }

    rf_image processed = rf_load_image_ex(rotPixels, image->height, image->width);
    rf_image_format(&processed, image->format);
    rf_unload_image(*image);

    RF_FREE(srcPixels);
    RF_FREE(rotPixels);

    image->data = processed.data;
    image->width = processed.width;
    image->height = processed.height;
}

// Modify image color: tint
RF_API void rf_image_color_tint(rf_image* image, rf_color color)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    rf_color* pixels = rf_get_image_data(*image);

    float cR = (float)color.r/255;
    float cG = (float)color.g/255;
    float cB = (float)color.b/255;
    float cA = (float)color.a/255;

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            int index = y * image->width + x;
            unsigned char r = 255*((float)pixels[index].r/255*cR);
            unsigned char g = 255*((float)pixels[index].g/255*cG);
            unsigned char b = 255*((float)pixels[index].b/255*cB);
            unsigned char a = 255*((float)pixels[index].a/255*cA);

            pixels[y*image->width + x].r = r;
            pixels[y*image->width + x].g = g;
            pixels[y*image->width + x].b = b;
            pixels[y*image->width + x].a = a;
        }
    }

    rf_image processed = rf_load_image_ex(pixels, image->width, image->height);
    rf_image_format(&processed, image->format);
    rf_unload_image(*image);
    RF_FREE(pixels);

    image->data = processed.data;
}

// Modify image color: invert
RF_API void rf_image_color_invert(rf_image* image)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    rf_color* pixels = rf_get_image_data(*image);

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            pixels[y*image->width + x].r = 255 - pixels[y*image->width + x].r;
            pixels[y*image->width + x].g = 255 - pixels[y*image->width + x].g;
            pixels[y*image->width + x].b = 255 - pixels[y*image->width + x].b;
        }
    }

    rf_image processed = rf_load_image_ex(pixels, image->width, image->height);
    rf_image_format(&processed, image->format);
    rf_unload_image(*image);
    RF_FREE(pixels);

    image->data = processed.data;
}

// Modify image color: grayscale
RF_API void rf_image_color_grayscale(rf_image* image)
{
    rf_image_format(image, rf_uncompressed_grayscale);
}

// Modify image color: contrast
// NOTE: Contrast values between -100 and 100
RF_API void rf_image_color_contrast(rf_image* image, float contrast)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    if (contrast < -100) contrast = -100;
    if (contrast > 100) contrast = 100;

    contrast = (100.0f + contrast)/100.0f;
    contrast *= contrast;

    rf_color* pixels = rf_get_image_data(*image);

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            float pR = (float)pixels[y*image->width + x].r/255.0f;
            pR -= 0.5;
            pR *= contrast;
            pR += 0.5;
            pR *= 255;
            if (pR < 0) pR = 0;
            if (pR > 255) pR = 255;

            float pG = (float)pixels[y*image->width + x].g/255.0f;
            pG -= 0.5;
            pG *= contrast;
            pG += 0.5;
            pG *= 255;
            if (pG < 0) pG = 0;
            if (pG > 255) pG = 255;

            float pB = (float)pixels[y*image->width + x].b/255.0f;
            pB -= 0.5;
            pB *= contrast;
            pB += 0.5;
            pB *= 255;
            if (pB < 0) pB = 0;
            if (pB > 255) pB = 255;

            pixels[y*image->width + x].r = (unsigned char)pR;
            pixels[y*image->width + x].g = (unsigned char)pG;
            pixels[y*image->width + x].b = (unsigned char)pB;
        }
    }

    rf_image processed = rf_load_image_ex(pixels, image->width, image->height);
    rf_image_format(&processed, image->format);
    rf_unload_image(*image);
    RF_FREE(pixels);

    image->data = processed.data;
}

// Modify image color: brightness
// NOTE: Brightness values between -255 and 255
RF_API void rf_image_color_brightness(rf_image* image, int brightness)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    if (brightness < -255) brightness = -255;
    if (brightness > 255) brightness = 255;

    rf_color* pixels = rf_get_image_data(*image);

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            int cR = pixels[y*image->width + x].r + brightness;
            int cG = pixels[y*image->width + x].g + brightness;
            int cB = pixels[y*image->width + x].b + brightness;

            if (cR < 0) cR = 1;
            if (cR > 255) cR = 255;

            if (cG < 0) cG = 1;
            if (cG > 255) cG = 255;

            if (cB < 0) cB = 1;
            if (cB > 255) cB = 255;

            pixels[y*image->width + x].r = (unsigned char)cR;
            pixels[y*image->width + x].g = (unsigned char)cG;
            pixels[y*image->width + x].b = (unsigned char)cB;
        }
    }

    rf_image processed = rf_load_image_ex(pixels, image->width, image->height);
    rf_image_format(&processed, image->format);
    rf_unload_image(*image);
    RF_FREE(pixels);

    image->data = processed.data;
}

// Modify image color: replace color
RF_API void rf_image_color_replace(rf_image* image, rf_color color, rf_color replace)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    rf_color* pixels = rf_get_image_data(*image);

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

    rf_image processed = rf_load_image_ex(pixels, image->width, image->height);
    rf_image_format(&processed, image->format);
    rf_unload_image(*image);
    RF_FREE(pixels);

    image->data = processed.data;
}

// Generate image: plain color
RF_API rf_image rf_gen_image_color(int width, int height, rf_color color)
{
    rf_color* pixels = (rf_color*)RF_MALLOC(width*height * sizeof(rf_color));
    memset(pixels, 0, width*height * sizeof(rf_color));

    for (int i = 0; i < width*height; i++) pixels[i] = color;

    rf_image image = rf_load_image_ex(pixels, width, height);

    RF_FREE(pixels);

    return image;
}

// Generate image: vertical gradient
RF_API rf_image rf_gen_image_gradient_v(int width, int height, rf_color top, rf_color bottom)
{
    rf_color* pixels = (rf_color* )RF_MALLOC(width*height*sizeof(rf_color));

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

    rf_image image = rf_load_image_ex(pixels, width, height);
    RF_FREE(pixels);

    return image;
}

// Generate image: horizontal gradient
RF_API rf_image rf_gen_image_gradient_h(int width, int height, rf_color left, rf_color right)
{
    rf_color* pixels = (rf_color* )RF_MALLOC(width*height*sizeof(rf_color));

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

    rf_image image = rf_load_image_ex(pixels, width, height);
    RF_FREE(pixels);

    return image;
}

// Generate image: radial gradient
RF_API rf_image rf_gen_image_gradient_radial(int width, int height, float density, rf_color inner, rf_color outer)
{
    rf_color* pixels = (rf_color* )RF_MALLOC(width*height*sizeof(rf_color));
    float radius = (width < height)? (float)width/2.0f : (float)height/2.0f;

    float centerX = (float)width/2.0f;
    float centerY = (float)height/2.0f;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float dist = hypotf((float)x - centerX, (float)y - centerY);
            float factor = (dist - radius*density)/(radius*(1.0f - density));

            factor = (float)fmax(factor, 0.f);
            factor = (float)fmin(factor, 1.f); // dist can be bigger than radius so we have to check

            pixels[y*width + x].r = (int)((float)outer.r*factor + (float)inner.r*(1.0f - factor));
            pixels[y*width + x].g = (int)((float)outer.g*factor + (float)inner.g*(1.0f - factor));
            pixels[y*width + x].b = (int)((float)outer.b*factor + (float)inner.b*(1.0f - factor));
            pixels[y*width + x].a = (int)((float)outer.a*factor + (float)inner.a*(1.0f - factor));
        }
    }

    rf_image image = rf_load_image_ex(pixels, width, height);
    RF_FREE(pixels);

    return image;
}

// Generate image: checked
RF_API rf_image rf_gen_image_checked(int width, int height, int checksX, int checksY, rf_color col1, rf_color col2)
{
    rf_color* pixels = (rf_color* )RF_MALLOC(width*height*sizeof(rf_color));

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if ((x/checksX + y/checksY)%2 == 0) pixels[y*width + x] = col1;
            else pixels[y*width + x] = col2;
        }
    }

    rf_image image = rf_load_image_ex(pixels, width, height);
    RF_FREE(pixels);

    return image;
}

// Generate image: white noise
RF_API rf_image rf_gen_image_white_noise(int width, int height, float factor)
{
    rf_color* pixels = (rf_color* )RF_MALLOC(width*height*sizeof(rf_color));

    for (int i = 0; i < width*height; i++)
    {
        if (rf_get_random_value(0, 99) < (int)(factor*100.0f)) pixels[i] = rf_white;
        else pixels[i] = rf_black;
    }

    rf_image image = rf_load_image_ex(pixels, width, height);
    RF_FREE(pixels);

    return image;
}

// Generate image: perlin noise
RF_API rf_image rf_gen_image_perlin_noise(int width, int height, int offset_x, int offset_y, float scale)
{
    rf_color* pixels = (rf_color* )RF_MALLOC(width*height*sizeof(rf_color));

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
            pixels[y*width + x] = RF_CLITERAL(rf_color){intensity, intensity, intensity, 255};
        }
    }

    rf_image image = rf_load_image_ex(pixels, width, height);
    RF_FREE(pixels);

    return image;
}

// Generate image: cellular algorithm. Bigger tileSize means bigger cells
RF_API rf_image rf_gen_image_cellular(int width, int height, int tileSize)
{
    rf_color* pixels = (rf_color* )RF_MALLOC(width*height*sizeof(rf_color));

    int seedsPerRow = width/tileSize;
    int seedsPerCol = height/tileSize;
    int seedsCount = seedsPerRow * seedsPerCol;

    rf_vector2 *seeds = (rf_vector2 *)RF_MALLOC(seedsCount*sizeof(rf_vector2));

    for (int i = 0; i < seedsCount; i++)
    {
        int y = (i/seedsPerRow)*tileSize + rf_get_random_value(0, tileSize - 1);
        int x = (i%seedsPerRow)*tileSize + rf_get_random_value(0, tileSize - 1);
        seeds[i] = RF_CLITERAL(rf_vector2){ (float)x, (float)y};
    }

    for (int y = 0; y < height; y++)
    {
        int tileY = y/tileSize;

        for (int x = 0; x < width; x++)
        {
            int tileX = x/tileSize;

            float minDistance = (float)strtod("Inf", NULL);

            // Check all adjacent tiles
            for (int i = -1; i < 2; i++)
            {
                if ((tileX + i < 0) || (tileX + i >= seedsPerRow)) continue;

                for (int j = -1; j < 2; j++)
                {
                    if ((tileY + j < 0) || (tileY + j >= seedsPerCol)) continue;

                    rf_vector2 neighborSeed = seeds[(tileY + j)*seedsPerRow + tileX + i];

                    float dist = (float)hypot(x - (int)neighborSeed.x, y - (int)neighborSeed.y);
                    minDistance = (float)fmin(minDistance, dist);
                }
            }

            // I made this up but it seems to give good results at all tile sizes
            int intensity = (int)(minDistance*256.0f/tileSize);
            if (intensity > 255) intensity = 255;

            pixels[y*width + x] = RF_CLITERAL(rf_color){ intensity, intensity, intensity, 255 };
        }
    }

    RF_FREE(seeds);

    rf_image image = rf_load_image_ex(pixels, width, height);
    RF_FREE(pixels);

    return image;
}

// Generate GPU mipmaps for a texture
RF_API void rf_gen_texture_mipmaps(rf_texture2d* texture)
{
    // NOTE: NPOT textures support check inside function
    // On WebGL (OpenGL ES 2.0) NPOT textures support is limited
    rf_gl_generate_mipmaps(texture);
}

// Set texture scaling filter mode
RF_API void rf_set_texture_filter(rf_texture2d texture, int filterMode)
{
    switch (filterMode)
    {
        case rf_filter_point:
        {
            if (texture.mipmaps > 1)
            {
                // GL_NEAREST_MIPMAP_NEAREST - tex filter: POINT, mipmaps filter: POINT (sharp switching between mipmaps)
                rf_gl_texture_parameters(texture.id, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

                // GL_NEAREST - tex filter: POINT (no filter), no mipmaps
                rf_gl_texture_parameters(texture.id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            }
            else
            {
                // GL_NEAREST - tex filter: POINT (no filter), no mipmaps
                rf_gl_texture_parameters(texture.id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                rf_gl_texture_parameters(texture.id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            }
        } break;
        case rf_filter_bilinear:
        {
            if (texture.mipmaps > 1)
            {
                // GL_LINEAR_MIPMAP_NEAREST - tex filter: BILINEAR, mipmaps filter: POINT (sharp switching between mipmaps)
                // Alternative: GL_NEAREST_MIPMAP_LINEAR - tex filter: POINT, mipmaps filter: BILINEAR (smooth transition between mipmaps)
                rf_gl_texture_parameters(texture.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

                // GL_LINEAR - tex filter: BILINEAR, no mipmaps
                rf_gl_texture_parameters(texture.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
            else
            {
                // GL_LINEAR - tex filter: BILINEAR, no mipmaps
                rf_gl_texture_parameters(texture.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                rf_gl_texture_parameters(texture.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
        } break;
        case rf_filter_trilinear:
        {
            if (texture.mipmaps > 1)
            {
                // GL_LINEAR_MIPMAP_LINEAR - tex filter: BILINEAR, mipmaps filter: BILINEAR (smooth transition between mipmaps)
                rf_gl_texture_parameters(texture.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

                // GL_LINEAR - tex filter: BILINEAR, no mipmaps
                rf_gl_texture_parameters(texture.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
            else
            {
                RF_LOG(RF_LOG_WARNING, "[TEX ID %i] No mipmaps available for TRILINEAR texture filtering", texture.id);

                // GL_LINEAR - tex filter: BILINEAR, no mipmaps
                rf_gl_texture_parameters(texture.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                rf_gl_texture_parameters(texture.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
        } break;
        case rf_filter_anisotropic_4x: rf_gl_texture_parameters(texture.id, GL_TEXTURE_ANISOTROPIC_FILTER, 4); break;
        case rf_filter_anisotropic_8x: rf_gl_texture_parameters(texture.id, GL_TEXTURE_ANISOTROPIC_FILTER, 8); break;
        case rf_filter_anisotropic_16x: rf_gl_texture_parameters(texture.id, GL_TEXTURE_ANISOTROPIC_FILTER, 16); break;
        default: break;
    }
}

// Set texture wrapping mode
RF_API void rf_set_texture_wrap(rf_texture2d texture, int wrapMode)
{
    switch (wrapMode)
    {
        case rf_wrap_repeat:
        {
            rf_gl_texture_parameters(texture.id, GL_TEXTURE_WRAP_S, GL_REPEAT);
            rf_gl_texture_parameters(texture.id, GL_TEXTURE_WRAP_T, GL_REPEAT);
        } break;
        case rf_wrap_clamp:
        {
            rf_gl_texture_parameters(texture.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            rf_gl_texture_parameters(texture.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        } break;
        case rf_wrap_mirror_repeat:
        {
            rf_gl_texture_parameters(texture.id, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
            rf_gl_texture_parameters(texture.id, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        } break;
        case rf_wrap_mirror_clamp:
        {
            rf_gl_texture_parameters(texture.id, GL_TEXTURE_WRAP_S, GL_MIRROR_CLAMP_EXT);
            rf_gl_texture_parameters(texture.id, GL_TEXTURE_WRAP_T, GL_MIRROR_CLAMP_EXT);
        } break;
        default: break;
    }
}

// Draw a rf_texture2d
RF_API void rf_draw_texture(rf_texture2d texture, int posX, int posY, rf_color tint)
{
    rf_draw_texture_ex(texture, RF_CLITERAL(rf_vector2){ (float)posX, (float)posY }, 0.0f, 1.0f, tint);
}

// Draw a rf_texture2d with position defined as rf_vector2
RF_API void rf_draw_texture_v(rf_texture2d texture, rf_vector2 position, rf_color tint)
{
    rf_draw_texture_ex(texture, position, 0, 1.0f, tint);
}

// Draw a rf_texture2d with extended parameters
RF_API void rf_draw_texture_ex(rf_texture2d texture, rf_vector2 position, float rotation, float scale, rf_color tint)
{
    rf_rectangle source_rec = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
    rf_rectangle destRec = { position.x, position.y, (float)texture.width*scale, (float)texture.height*scale };
    rf_vector2 origin = { 0.0f, 0.0f };

    rf_draw_texture_pro(texture, source_rec, destRec, origin, rotation, tint);
}

// Draw a part of a texture (defined by a rectangle)
RF_API void rf_draw_texture_rec(rf_texture2d texture, rf_rectangle source_rec, rf_vector2 position, rf_color tint)
{
    rf_rectangle destRec = { position.x, position.y, (float)fabs(source_rec.width), (float)fabs(source_rec.height) };
    rf_vector2 origin = { 0.0f, 0.0f };

    rf_draw_texture_pro(texture, source_rec, destRec, origin, 0.0f, tint);
}

// Draw texture quad with tiling and offset parameters
// NOTE: Tiling and offset should be provided considering normalized texture values [0..1]
// i.e tiling = { 1.0f, 1.0f } refers to all texture, offset = { 0.5f, 0.5f } moves texture origin to center
RF_API void rf_draw_texture_quad(rf_texture2d texture, rf_vector2 tiling, rf_vector2 offset, rf_rectangle quad, rf_color tint)
{
    rf_rectangle source = { offset.x*texture.width, offset.y*texture.height, tiling.x*texture.width, tiling.y*texture.height };
    rf_vector2 origin = { 0.0f, 0.0f };

    rf_draw_texture_pro(texture, source, quad, origin, 0.0f, tint);
}

// Draw a part of a texture (defined by a rectangle) with 'pro' parameters
// NOTE: origin is relative to destination rectangle size
RF_API void rf_draw_texture_pro(rf_texture2d texture, rf_rectangle source_rec, rf_rectangle destRec, rf_vector2 origin, float rotation, rf_color tint)
{
    // Check if texture is valid
    if (texture.id > 0)
    {
        float width = (float)texture.width;
        float height = (float)texture.height;

        bool flipX = false;

        if (source_rec.width < 0) { flipX = true; source_rec.width *= -1; }
        if (source_rec.height < 0) source_rec.y -= source_rec.height;

        rf_gl_enable_texture(texture.id);

        rf_push_matrix();
        rf_translatef(destRec.x, destRec.y, 0.0f);
        rf_rotatef(rotation, 0.0f, 0.0f, 1.0f);
        rf_translatef(-origin.x, -origin.y, 0.0f);

        rf_gl_begin(GL_QUADS);
        rf_gl_color4ub(tint.r, tint.g, tint.b, tint.a);
        rf_gl_normal3f(0.0f, 0.0f, 1.0f); // Normal vector pointing towards viewer

        // Bottom-left corner for texture and quad
        if (flipX) rf_gl_tex_coord2f((source_rec.x + source_rec.width)/width, source_rec.y/height);
        else rf_gl_tex_coord2f(source_rec.x/width, source_rec.y/height);
        rf_gl_vertex2f(0.0f, 0.0f);

        // Bottom-right corner for texture and quad
        if (flipX) rf_gl_tex_coord2f((source_rec.x + source_rec.width)/width, (source_rec.y + source_rec.height)/height);
        else rf_gl_tex_coord2f(source_rec.x/width, (source_rec.y + source_rec.height)/height);
        rf_gl_vertex2f(0.0f, destRec.height);

        // Top-right corner for texture and quad
        if (flipX) rf_gl_tex_coord2f(source_rec.x/width, (source_rec.y + source_rec.height)/height);
        else rf_gl_tex_coord2f((source_rec.x + source_rec.width)/width, (source_rec.y + source_rec.height)/height);
        rf_gl_vertex2f(destRec.width, destRec.height);

        // Top-left corner for texture and quad
        if (flipX) rf_gl_tex_coord2f(source_rec.x/width, source_rec.y/height);
        else rf_gl_tex_coord2f((source_rec.x + source_rec.width)/width, source_rec.y/height);
        rf_gl_vertex2f(destRec.width, 0.0f);
        rf_gl_end();
        rf_pop_matrix();

        rf_gl_disable_texture();
    }
}

// Draws a texture (or part of it) that stretches or shrinks nicely using n-patch info
RF_API void rf_draw_texture_npatch(rf_texture2d texture, rf_npatch_info nPatchInfo, rf_rectangle destRec, rf_vector2 origin, float rotation, rf_color tint)
{
    if (texture.id > 0)
    {
        float width = (float)texture.width;
        float height = (float)texture.height;

        float patchWidth = (destRec.width <= 0.0f)? 0.0f : destRec.width;
        float patchHeight = (destRec.height <= 0.0f)? 0.0f : destRec.height;

        if (nPatchInfo.source_rec.width < 0) nPatchInfo.source_rec.x -= nPatchInfo.source_rec.width;
        if (nPatchInfo.source_rec.height < 0) nPatchInfo.source_rec.y -= nPatchInfo.source_rec.height;
        if (nPatchInfo.type == rf_npt_3patch_horizontal) patchHeight = nPatchInfo.source_rec.height;
        if (nPatchInfo.type == rf_npt_3patch_vertical) patchWidth = nPatchInfo.source_rec.width;

        bool drawCenter = true;
        bool drawMiddle = true;
        float leftBorder = (float)nPatchInfo.left;
        float topBorder = (float)nPatchInfo.top;
        float rightBorder = (float)nPatchInfo.right;
        float bottomBorder = (float)nPatchInfo.bottom;

        // adjust the lateral (left and right) border widths in case patchWidth < texture.width
        if (patchWidth <= (leftBorder + rightBorder) && nPatchInfo.type != rf_npt_3patch_vertical)
        {
            drawCenter = false;
            leftBorder = (leftBorder / (leftBorder + rightBorder)) * patchWidth;
            rightBorder = patchWidth - leftBorder;
        }
        // adjust the lateral (top and bottom) border heights in case patchHeight < texture.height
        if (patchHeight <= (topBorder + bottomBorder) && nPatchInfo.type != rf_npt_3patch_horizontal)
        {
            drawMiddle = false;
            topBorder = (topBorder / (topBorder + bottomBorder)) * patchHeight;
            bottomBorder = patchHeight - topBorder;
        }

        rf_vector2 vertA, vertB, vertC, vertD;
        vertA.x = 0.0f; // outer left
        vertA.y = 0.0f; // outer top
        vertB.x = leftBorder; // inner left
        vertB.y = topBorder; // inner top
        vertC.x = patchWidth - rightBorder; // inner right
        vertC.y = patchHeight - bottomBorder; // inner bottom
        vertD.x = patchWidth; // outer right
        vertD.y = patchHeight; // outer bottom

        rf_vector2 coordA, coordB, coordC, coordD;
        coordA.x = nPatchInfo.source_rec.x / width;
        coordA.y = nPatchInfo.source_rec.y / height;
        coordB.x = (nPatchInfo.source_rec.x + leftBorder) / width;
        coordB.y = (nPatchInfo.source_rec.y + topBorder) / height;
        coordC.x = (nPatchInfo.source_rec.x + nPatchInfo.source_rec.width - rightBorder) / width;
        coordC.y = (nPatchInfo.source_rec.y + nPatchInfo.source_rec.height - bottomBorder) / height;
        coordD.x = (nPatchInfo.source_rec.x + nPatchInfo.source_rec.width) / width;
        coordD.y = (nPatchInfo.source_rec.y + nPatchInfo.source_rec.height) / height;

        rf_gl_enable_texture(texture.id);

        rf_push_matrix();
        rf_translatef(destRec.x, destRec.y, 0.0f);
        rf_rotatef(rotation, 0.0f, 0.0f, 1.0f);
        rf_translatef(-origin.x, -origin.y, 0.0f);

        rf_gl_begin(GL_QUADS);
        rf_gl_color4ub(tint.r, tint.g, tint.b, tint.a);
        rf_gl_normal3f(0.0f, 0.0f, 1.0f); // Normal vector pointing towards viewer

        if (nPatchInfo.type == rf_npt_9patch)
        {
            // ------------------------------------------------------------
            // TOP-LEFT QUAD
            rf_gl_tex_coord2f(coordA.x, coordB.y); rf_gl_vertex2f(vertA.x, vertB.y); // Bottom-left corner for texture and quad
            rf_gl_tex_coord2f(coordB.x, coordB.y); rf_gl_vertex2f(vertB.x, vertB.y); // Bottom-right corner for texture and quad
            rf_gl_tex_coord2f(coordB.x, coordA.y); rf_gl_vertex2f(vertB.x, vertA.y); // Top-right corner for texture and quad
            rf_gl_tex_coord2f(coordA.x, coordA.y); rf_gl_vertex2f(vertA.x, vertA.y); // Top-left corner for texture and quad
            if (drawCenter)
            {
                // TOP-CENTER QUAD
                rf_gl_tex_coord2f(coordB.x, coordB.y); rf_gl_vertex2f(vertB.x, vertB.y); // Bottom-left corner for texture and quad
                rf_gl_tex_coord2f(coordC.x, coordB.y); rf_gl_vertex2f(vertC.x, vertB.y); // Bottom-right corner for texture and quad
                rf_gl_tex_coord2f(coordC.x, coordA.y); rf_gl_vertex2f(vertC.x, vertA.y); // Top-right corner for texture and quad
                rf_gl_tex_coord2f(coordB.x, coordA.y); rf_gl_vertex2f(vertB.x, vertA.y); // Top-left corner for texture and quad
            }
            // TOP-RIGHT QUAD
            rf_gl_tex_coord2f(coordC.x, coordB.y); rf_gl_vertex2f(vertC.x, vertB.y); // Bottom-left corner for texture and quad
            rf_gl_tex_coord2f(coordD.x, coordB.y); rf_gl_vertex2f(vertD.x, vertB.y); // Bottom-right corner for texture and quad
            rf_gl_tex_coord2f(coordD.x, coordA.y); rf_gl_vertex2f(vertD.x, vertA.y); // Top-right corner for texture and quad
            rf_gl_tex_coord2f(coordC.x, coordA.y); rf_gl_vertex2f(vertC.x, vertA.y); // Top-left corner for texture and quad
            if (drawMiddle)
            {
                // ------------------------------------------------------------
                // MIDDLE-LEFT QUAD
                rf_gl_tex_coord2f(coordA.x, coordC.y); rf_gl_vertex2f(vertA.x, vertC.y); // Bottom-left corner for texture and quad
                rf_gl_tex_coord2f(coordB.x, coordC.y); rf_gl_vertex2f(vertB.x, vertC.y); // Bottom-right corner for texture and quad
                rf_gl_tex_coord2f(coordB.x, coordB.y); rf_gl_vertex2f(vertB.x, vertB.y); // Top-right corner for texture and quad
                rf_gl_tex_coord2f(coordA.x, coordB.y); rf_gl_vertex2f(vertA.x, vertB.y); // Top-left corner for texture and quad
                if (drawCenter)
                {
                    // MIDDLE-CENTER QUAD
                    rf_gl_tex_coord2f(coordB.x, coordC.y); rf_gl_vertex2f(vertB.x, vertC.y); // Bottom-left corner for texture and quad
                    rf_gl_tex_coord2f(coordC.x, coordC.y); rf_gl_vertex2f(vertC.x, vertC.y); // Bottom-right corner for texture and quad
                    rf_gl_tex_coord2f(coordC.x, coordB.y); rf_gl_vertex2f(vertC.x, vertB.y); // Top-right corner for texture and quad
                    rf_gl_tex_coord2f(coordB.x, coordB.y); rf_gl_vertex2f(vertB.x, vertB.y); // Top-left corner for texture and quad
                }

                // MIDDLE-RIGHT QUAD
                rf_gl_tex_coord2f(coordC.x, coordC.y); rf_gl_vertex2f(vertC.x, vertC.y); // Bottom-left corner for texture and quad
                rf_gl_tex_coord2f(coordD.x, coordC.y); rf_gl_vertex2f(vertD.x, vertC.y); // Bottom-right corner for texture and quad
                rf_gl_tex_coord2f(coordD.x, coordB.y); rf_gl_vertex2f(vertD.x, vertB.y); // Top-right corner for texture and quad
                rf_gl_tex_coord2f(coordC.x, coordB.y); rf_gl_vertex2f(vertC.x, vertB.y); // Top-left corner for texture and quad
            }

            // ------------------------------------------------------------
            // BOTTOM-LEFT QUAD
            rf_gl_tex_coord2f(coordA.x, coordD.y); rf_gl_vertex2f(vertA.x, vertD.y); // Bottom-left corner for texture and quad
            rf_gl_tex_coord2f(coordB.x, coordD.y); rf_gl_vertex2f(vertB.x, vertD.y); // Bottom-right corner for texture and quad
            rf_gl_tex_coord2f(coordB.x, coordC.y); rf_gl_vertex2f(vertB.x, vertC.y); // Top-right corner for texture and quad
            rf_gl_tex_coord2f(coordA.x, coordC.y); rf_gl_vertex2f(vertA.x, vertC.y); // Top-left corner for texture and quad
            if (drawCenter)
            {
                // BOTTOM-CENTER QUAD
                rf_gl_tex_coord2f(coordB.x, coordD.y); rf_gl_vertex2f(vertB.x, vertD.y); // Bottom-left corner for texture and quad
                rf_gl_tex_coord2f(coordC.x, coordD.y); rf_gl_vertex2f(vertC.x, vertD.y); // Bottom-right corner for texture and quad
                rf_gl_tex_coord2f(coordC.x, coordC.y); rf_gl_vertex2f(vertC.x, vertC.y); // Top-right corner for texture and quad
                rf_gl_tex_coord2f(coordB.x, coordC.y); rf_gl_vertex2f(vertB.x, vertC.y); // Top-left corner for texture and quad
            }

            // BOTTOM-RIGHT QUAD
            rf_gl_tex_coord2f(coordC.x, coordD.y); rf_gl_vertex2f(vertC.x, vertD.y); // Bottom-left corner for texture and quad
            rf_gl_tex_coord2f(coordD.x, coordD.y); rf_gl_vertex2f(vertD.x, vertD.y); // Bottom-right corner for texture and quad
            rf_gl_tex_coord2f(coordD.x, coordC.y); rf_gl_vertex2f(vertD.x, vertC.y); // Top-right corner for texture and quad
            rf_gl_tex_coord2f(coordC.x, coordC.y); rf_gl_vertex2f(vertC.x, vertC.y); // Top-left corner for texture and quad
        }
        else if (nPatchInfo.type == rf_npt_3patch_vertical)
        {
            // TOP QUAD
            // -----------------------------------------------------------
            // rf_texture coords                 Vertices
            rf_gl_tex_coord2f(coordA.x, coordB.y); rf_gl_vertex2f(vertA.x, vertB.y); // Bottom-left corner for texture and quad
            rf_gl_tex_coord2f(coordD.x, coordB.y); rf_gl_vertex2f(vertD.x, vertB.y); // Bottom-right corner for texture and quad
            rf_gl_tex_coord2f(coordD.x, coordA.y); rf_gl_vertex2f(vertD.x, vertA.y); // Top-right corner for texture and quad
            rf_gl_tex_coord2f(coordA.x, coordA.y); rf_gl_vertex2f(vertA.x, vertA.y); // Top-left corner for texture and quad
            if (drawCenter)
            {
                // MIDDLE QUAD
                // -----------------------------------------------------------
                // rf_texture coords                 Vertices
                rf_gl_tex_coord2f(coordA.x, coordC.y); rf_gl_vertex2f(vertA.x, vertC.y); // Bottom-left corner for texture and quad
                rf_gl_tex_coord2f(coordD.x, coordC.y); rf_gl_vertex2f(vertD.x, vertC.y); // Bottom-right corner for texture and quad
                rf_gl_tex_coord2f(coordD.x, coordB.y); rf_gl_vertex2f(vertD.x, vertB.y); // Top-right corner for texture and quad
                rf_gl_tex_coord2f(coordA.x, coordB.y); rf_gl_vertex2f(vertA.x, vertB.y); // Top-left corner for texture and quad
            }
            // BOTTOM QUAD
            // -----------------------------------------------------------
            // rf_texture coords                 Vertices
            rf_gl_tex_coord2f(coordA.x, coordD.y); rf_gl_vertex2f(vertA.x, vertD.y); // Bottom-left corner for texture and quad
            rf_gl_tex_coord2f(coordD.x, coordD.y); rf_gl_vertex2f(vertD.x, vertD.y); // Bottom-right corner for texture and quad
            rf_gl_tex_coord2f(coordD.x, coordC.y); rf_gl_vertex2f(vertD.x, vertC.y); // Top-right corner for texture and quad
            rf_gl_tex_coord2f(coordA.x, coordC.y); rf_gl_vertex2f(vertA.x, vertC.y); // Top-left corner for texture and quad
        }
        else if (nPatchInfo.type == rf_npt_3patch_horizontal)
        {
            // LEFT QUAD
            // -----------------------------------------------------------
            // rf_texture coords                 Vertices
            rf_gl_tex_coord2f(coordA.x, coordD.y); rf_gl_vertex2f(vertA.x, vertD.y); // Bottom-left corner for texture and quad
            rf_gl_tex_coord2f(coordB.x, coordD.y); rf_gl_vertex2f(vertB.x, vertD.y); // Bottom-right corner for texture and quad
            rf_gl_tex_coord2f(coordB.x, coordA.y); rf_gl_vertex2f(vertB.x, vertA.y); // Top-right corner for texture and quad
            rf_gl_tex_coord2f(coordA.x, coordA.y); rf_gl_vertex2f(vertA.x, vertA.y); // Top-left corner for texture and quad
            if (drawCenter)
            {
                // CENTER QUAD
                // -----------------------------------------------------------
                // rf_texture coords                 Vertices
                rf_gl_tex_coord2f(coordB.x, coordD.y); rf_gl_vertex2f(vertB.x, vertD.y); // Bottom-left corner for texture and quad
                rf_gl_tex_coord2f(coordC.x, coordD.y); rf_gl_vertex2f(vertC.x, vertD.y); // Bottom-right corner for texture and quad
                rf_gl_tex_coord2f(coordC.x, coordA.y); rf_gl_vertex2f(vertC.x, vertA.y); // Top-right corner for texture and quad
                rf_gl_tex_coord2f(coordB.x, coordA.y); rf_gl_vertex2f(vertB.x, vertA.y); // Top-left corner for texture and quad
            }
            // RIGHT QUAD
            // -----------------------------------------------------------
            // rf_texture coords                 Vertices
            rf_gl_tex_coord2f(coordC.x, coordD.y); rf_gl_vertex2f(vertC.x, vertD.y); // Bottom-left corner for texture and quad
            rf_gl_tex_coord2f(coordD.x, coordD.y); rf_gl_vertex2f(vertD.x, vertD.y); // Bottom-right corner for texture and quad
            rf_gl_tex_coord2f(coordD.x, coordA.y); rf_gl_vertex2f(vertD.x, vertA.y); // Top-right corner for texture and quad
            rf_gl_tex_coord2f(coordC.x, coordA.y); rf_gl_vertex2f(vertC.x, vertA.y); // Top-left corner for texture and quad
        }
        rf_gl_end();
        rf_pop_matrix();

        rf_gl_disable_texture();

    }
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Load animated GIF data
//  - rf_image.data buffer includes all frames: [image#0][image#1][image#2][...]
//  - Number of frames is returned through 'frames' parameter
//  - Frames delay is returned through 'delays' parameter (int array)
//  - All frames are returned in RGBA format
RF_INTERNAL rf_image _rf_load_animated_gif(const char* fileName, int* frames, int** delays)
{
    rf_image image = { 0 };

    FILE* gifFile = fopen(fileName, "rb");

    if (gifFile == NULL)
    {
        RF_LOG(RF_LOG_WARNING, "[%s] Animated GIF file could not be opened", fileName);
    }
    else
    {
        fseek(gifFile, 0L, SEEK_END);
        int size = ftell(gifFile);
        fseek(gifFile, 0L, SEEK_SET);

        unsigned char* buffer = (unsigned char*)RF_MALLOC(size * sizeof(char));
        memset(buffer, 0, size * sizeof(char));
        fread(buffer, sizeof(char), size, gifFile);

        fclose(gifFile); // Close file pointer

        int comp = 0;
        image.data = stbi_load_gif_from_memory(buffer, size, delays, &image.width, &image.height, frames, &comp, 4);

        image.mipmaps = 1;
        image.format = rf_uncompressed_r8g8b8a8;

        free(buffer);
    }

    return image;
}
//endregion

#endif

//endregion