//Implementation of the mouse input example from raylib using rayfork

#include "rayfork.h"
#include "glad/glad.h"
#include "game.h"

rf_context rf_ctx;
rf_renderer_memory_buffers rf_mem;

const int screen_width = 800;
const int screen_height = 450;

rf_vec2 ball_position = {(float) screen_width / 2, (float) screen_height / 2 };
rf_color ballColor = RF_DARKBLUE;

void on_init(void)
{
    //Load opengl with glad
    gladLoadGL();

    //Initialise rayfork and load the default font
    rf_init(&rf_ctx, &rf_mem, screen_width, screen_height, RF_DEFAULT_OPENGL_PROCS);
    rf_set_target_fps(60);
    rf_load_default_font(RF_DEFAULT_ALLOCATOR, RF_DEFAULT_ALLOCATOR);
}

void on_frame(const input_data input)
{
    //Update
    ball_position.x = input.mouse_x;
    ball_position.y = input.mouse_y;

    if (input.mouse_left_pressed) ballColor = RF_MAROON;
    else if (input.mouse_middle_pressed) ballColor = RF_LIME;
    else if (input.mouse_right_pressed) ballColor = RF_DARKBLUE;

    //Render
    rf_begin();

    rf_clear(RF_RAYWHITE);

    rf_draw_text("move ball with mouse and click mouse button to change color", 10, 10, 20, RF_DARKGRAY);

    rf_draw_circle_v(ball_position, 40, ballColor);

    rf_end();
}