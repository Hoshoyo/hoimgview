#include "input.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

#define ARRAY_LENGTH(A) (sizeof(A) / sizeof(*A))

typedef struct {
    int state;  // 1 pressed, 0 released, 2 repeat
    int mod;
} Input_Keyboard_Key;

typedef struct {
    bool released;
    bool pressed;
    int state; // 1 pressed, 0 unpressed
    int mod;
    float x_press;
    float y_press;
    float x_release;
    float y_release;
} Input_Mouse_Button;

typedef struct {
    Input_Keyboard_Key keys[1024];
    Input_Keyboard_Key keys_events[1024];
} Input_Keyboard_State;

typedef struct {
    float x;
    float y;
    Input_Mouse_Button button[16];
} Input_Mouse_State;

typedef struct {
    float width;
    float height;
    float x;
    float y;
} Input_Window_State;

typedef struct {
    Input_Keyboard_State keyboard;
    Input_Mouse_State    mouse;
    Input_Window_State   window;
} Input_State;

static Input_State istate;
Command command = {0};

void 
window_size_callback(GLFWwindow* window, int width, int height) {
    istate.window.width = (float)width;
    istate.window.height = (float)height;
}

void
window_character_callback(GLFWwindow* window, unsigned int codepoint) {
    if(command.length >= ARRAY_LENGTH(command.buffer))
        return;
    if(codepoint == ':' || command.running)
    {
        command.buffer[command.length++] = codepoint;
        command.running = true;
    }
}

void
window_pos_callback(GLFWwindow* window, int xpos, int ypos) {
    istate.window.x = (float)xpos;
    istate.window.y = (float)ypos;
}

void
mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if(button > ARRAY_LENGTH(istate.mouse.button)) return;
	switch (action) {
		case GLFW_PRESS: {
            if(istate.mouse.button[button].state == 0)
            {
                //printf("pressed at %f %f\n", istate.mouse.x, istate.mouse.y);
                istate.mouse.button[button].pressed = true;
                istate.mouse.button[button].x_press = istate.mouse.x;
                istate.mouse.button[button].y_press = istate.mouse.y;
            }
            istate.mouse.button[button].mod = mods;
            istate.mouse.button[button].state = 1;
		}break;
		case GLFW_RELEASE: {
            //printf("released at %f %f\n", istate.mouse.x, istate.mouse.y);
            istate.mouse.button[button].mod = mods;
            istate.mouse.button[button].released = true;
            istate.mouse.button[button].state = 0;
            istate.mouse.button[button].x_release = istate.mouse.x;
            istate.mouse.button[button].y_release = istate.mouse.y;
		}break;
		default: return;
	}
}

static void
handle_keypress(int key, int action, int mods)
{
    if(key == GLFW_KEY_BACKSPACE)
    {
        if(command.length > 0) command.length--;
        if(command.length == 0) command.running = false;
    }
    else if(key == GLFW_KEY_ENTER)
    {
        if(command.length > 0)
            command.entered = true;
        command.running = false;
    }
}

void
key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if(key >= 0 && key <= 1024) {
        istate.keyboard.keys[key].mod = mods;
        istate.keyboard.keys_events[key].mod = mods;
        switch (action) {
            case GLFW_PRESS: {
                istate.keyboard.keys[key].state = 1;
                istate.keyboard.keys_events[key].state = 1;
                handle_keypress(key, action, mods);
            }break;
            case GLFW_RELEASE: {
                istate.keyboard.keys[key].state = 0;
                istate.keyboard.keys_events[key].state = 0;
            }break;
            case GLFW_REPEAT:{
                istate.keyboard.keys[key].state = 2;
                istate.keyboard.keys_events[key].state = 2;
                handle_keypress(key, action, mods);
            }break;
            default: return;
        }
    }
}

void
cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    istate.mouse.x = (float)xpos;
    istate.mouse.y = (float)ypos;
}

void input_set_callbacks(void* glfw_window) {
    glfwSetMouseButtonCallback(glfw_window, mouse_button_callback);
    glfwSetCursorPosCallback(glfw_window, cursor_position_callback);
    glfwSetKeyCallback(glfw_window, key_callback);
    glfwSetWindowSizeCallback(glfw_window, window_size_callback);
    glfwSetWindowPosCallback(glfw_window, window_pos_callback);
    glfwSetCharCallback(glfw_window, window_character_callback);
}

int input_key_state(int key) {
    return istate.keyboard.keys[key].state;
}

int input_key_state_mod(int key, int mod_filter) {
    if(istate.keyboard.keys[key].mod & mod_filter) {
        return istate.keyboard.keys[key].state;
    }
    return 0;
}

int input_button_state(int button) {
    return istate.mouse.button[button].state;
}

void input_mouse_pos(float* x, float* y) {
    *x = istate.mouse.x;
    *y = istate.mouse.y;
}
vec2 input_mouse_pos_vec2(float window_height) {
    return (vec2){istate.mouse.x, window_height - istate.mouse.y};
}

void input_window_size(float* w, float* h) {
    *w = istate.window.width;
    *h = istate.window.height;
}

int input_mouse_event_press(int button, vec2* at) {
    if(istate.mouse.button[button].pressed)
    {
        istate.mouse.button[button].pressed = false;
        if(at) {
            at->x = istate.mouse.button[button].x_press;
            at->y = istate.mouse.button[button].y_press;
        }
        return 1;
    }
    return 0;
}
int input_mouse_event_release(int button, vec2* at) {
    if(istate.mouse.button[button].released)
    {
        istate.mouse.button[button].released = false;
        if(at) {
            at->x = istate.mouse.button[button].x_release;
            at->y = istate.mouse.button[button].y_release;
        }
        return 1;
    }
    return 0;
}

int input_key_handle_event(int key) {
    int ev = istate.keyboard.keys_events[key].state;
    istate.keyboard.keys_events[key].state = -1;
    return ev;
}

vec2 input_mouse_pressed_at(int button) {
    return (vec2){istate.mouse.button[button].x_press, istate.mouse.button[button].y_press};
}

vec2 input_mouse_released_at(int button) {
    return (vec2){istate.mouse.button[button].x_release, istate.mouse.button[button].y_release};
}