#pragma once

typedef struct input_data input_data;
struct input_data
{
    float scroll_y;
};

void on_init(void);
void on_frame(input_data);