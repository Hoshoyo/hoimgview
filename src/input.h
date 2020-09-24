#pragma once
#include <gm.h>

typedef struct {
    unsigned int buffer[1024];
    int          length;
    bool         running;
    bool         entered;
} Command;

void input_set_callbacks(void* glfw_window);

#define INPUT_MOUSE_LBUTTON 0
#define INPUT_MOUSE_RBUTTON 1
#define INPUT_MOUSE_MBUTTON 2

#define INPUT_KEY_RIGHT 262
#define INPUT_KEY_LEFT  263
#define INPUT_KEY_DOWN  264
#define INPUT_KEY_UP    265

#define INPUT_MOD_SHIFT (1 << 0)
#define INPUT_MOD_CTRL (1 << 1)
#define INPUT_MOD_ALT (1 << 2)
#define INPUT_MOD_SUPER (1 << 3)

int  input_key_state(int key);
int  input_button_state(int button);
void input_mouse_pos(float* x, float* y);
int  input_key_state_mod(int key, int mod_filter);
vec2 input_mouse_pos_vec2(float window_height);
int input_mouse_event_press(int button, vec2* at);
int input_mouse_event_release(int button, vec2* at);
vec2 input_mouse_pressed_at(int button);
vec2 input_mouse_released_at(int button);
int input_key_handle_event(int key);
vec2 input_mouse_scroll_handle();

extern Command command;