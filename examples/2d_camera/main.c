//In this file we only initialise the window using sokol_app

#define SOKOL_IMPL
#define SOKOL_GLCORE33
#define SOKOL_WIN32_NO_GL_LOADER
#include "sokol_app.h"
#include "game.h"

input_data global_input_data;

void platform_on_frame()
{
    on_frame(global_input_data);
}

void platform_on_event(const sapp_event* event)
{
    if (event->type == SAPP_EVENTTYPE_KEY_DOWN && event->key_code == SAPP_KEYCODE_RIGHT) global_input_data.right_pressed = 1;
    if (event->type == SAPP_EVENTTYPE_KEY_DOWN && event->key_code == SAPP_KEYCODE_LEFT) global_input_data.left_pressed = 1;
    if (event->type == SAPP_EVENTTYPE_KEY_DOWN && event->key_code == SAPP_KEYCODE_UP) global_input_data.up_pressed = 1;
    if (event->type == SAPP_EVENTTYPE_KEY_DOWN && event->key_code == SAPP_KEYCODE_DOWN) global_input_data.down_pressed = 1;
    if (event->type == SAPP_EVENTTYPE_KEY_DOWN && event->key_code == SAPP_KEYCODE_A) global_input_data.a_pressed = 1;
    if (event->type == SAPP_EVENTTYPE_KEY_DOWN && event->key_code == SAPP_KEYCODE_S) global_input_data.s_pressed = 1;
    if (event->type == SAPP_EVENTTYPE_KEY_DOWN && event->key_code == SAPP_KEYCODE_R) global_input_data.r_pressed = 1;


    if (event->type == SAPP_EVENTTYPE_KEY_UP && event->key_code == SAPP_KEYCODE_RIGHT) global_input_data.right_pressed = 0;
    if (event->type == SAPP_EVENTTYPE_KEY_UP && event->key_code == SAPP_KEYCODE_LEFT) global_input_data.left_pressed = 0;
    if (event->type == SAPP_EVENTTYPE_KEY_UP && event->key_code == SAPP_KEYCODE_UP) global_input_data.up_pressed = 0;
    if (event->type == SAPP_EVENTTYPE_KEY_UP && event->key_code == SAPP_KEYCODE_DOWN) global_input_data.down_pressed = 0;
    if (event->type == SAPP_EVENTTYPE_KEY_UP && event->key_code == SAPP_KEYCODE_A) global_input_data.a_pressed = 0;
    if (event->type == SAPP_EVENTTYPE_KEY_UP && event->key_code == SAPP_KEYCODE_S) global_input_data.s_pressed = 0;
    if (event->type == SAPP_EVENTTYPE_KEY_UP && event->key_code == SAPP_KEYCODE_R) global_input_data.r_pressed = 0;
}

sapp_desc sokol_main(int argc, char** argv) 
{
    return (sapp_desc) 
    {
        .width = 800,
        .height = 450,
        .init_cb = on_init,
        .frame_cb = platform_on_frame,
        .event_cb = platform_on_event,
        .window_title = "raylib [core] example - keyboard input",
    };
}