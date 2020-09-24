#include <stdio.h>
#define GRAPHICS_MATH_IMPLEMENT
#define HOGL_IMPLEMENT
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <ho_gl.h>
#include <gm.h>
#include <GLFW/glfw3.h>
#include "batcher.h"
#include <stdlib.h>
#include <string.h>
#include "font_load.h"
#include "font_render.h"
#include "input.h"
#include "os.h"

#define MIN(A, B) (((A) < (B)) ? (A) : (B))
#define MAX(A, B) (((A) > (B)) ? (A) : (B))

void set_tex_linear()
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}
void set_tex_nearest()
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

static Font_Info font_info;

typedef struct {
    char* data;
    char* filename;
    u32   texture;
    int   width;
    int   height;

    float x, y;
    float scale;
} Image;

extern double scroll_y;
static Image g_image;

r64 os_time_us();

int image_destroy()
{
    stbi_image_free(g_image.data);
    g_image.data = 0;
    g_image.filename = 0;
    // TODO(psv): free texture
    g_image.width = 0;
    g_image.height = 0;
    //glDeleteTextures(1, &g_image.texture);
    return 0;
}

int image_load(char* filename)
{
    FILE* file = fopen(filename, "rb");
    if(!file) return -1;
    int x = 0, y = 0, comp = 0;
    stbi_set_flip_vertically_on_load(1);
    g_image.data = stbi_load_from_file(file, &x, &y, &comp, 4);
    g_image.width = x;
    g_image.height = y;
    g_image.filename = filename; // TODO(psv): maybe duplicate the string
    g_image.texture = batch_texture_create_from_data(g_image.data, x, y);
    g_image.scale = 1.0f;
    return 0;
}

int init(int argc, char** argv)
{
    if(argc > 1)
    {
        image_load(argv[1]);
    }
    return 0;
}

void command_handle()
{
    command.entered = false;
    command.running = false;

    image_destroy();
    command.buffer[command.length] = 0;
    char* str = unicode_to_utf8(command.buffer + 1, command.length - 1);
    image_load(str);
    free(str);
    command.length = 0;
}

void render(Hobatch_Context* ctx, int ww, int wh)
{
    if(command.entered)
    {
        command_handle();
    }

    if(!command.running)
    {
        double scroll_y = input_mouse_scroll_handle().y;
        if(input_key_handle_event('L') > 0)
        {
            set_tex_linear();
        }
        if(input_key_handle_event('N') > 0)
        {
            set_tex_nearest();
        }
        if(input_key_handle_event('F') > 0)
        {
            g_image.x = 0;
            g_image.y = 0;
            float sw = (float)ww / (float)g_image.width;
            float sh = (float)wh / (float)g_image.height;
            g_image.scale = (sw < sh) ? sw : sh;
        }
        if(input_key_state('=') || scroll_y > 0.0f)
        {
            g_image.scale *= (1.01f - scroll_y * 0.1f);
        }
        
        if(input_key_state('-') || scroll_y < 0.0f)
        {
            g_image.scale *= (0.99f - scroll_y * 0.1f);
        }
        if(g_image.scale < 0.1f) g_image.scale = 0.1f;
        if(g_image.scale > 10.0f) g_image.scale = 10.0f;

        if(input_key_handle_event('R') > 0)
        {
            g_image.scale = 1.0f;
            g_image.x = 0.0f;
            g_image.y = 0.0f;
        }

        {
            if(input_key_state(GLFW_KEY_LEFT))
            {
                g_image.x -= (input_key_state_mod(GLFW_KEY_LEFT, INPUT_MOD_SHIFT)) ? 1.0f : 10.0f;
            }
            if(input_key_state(GLFW_KEY_RIGHT))
            {
                g_image.x += (input_key_state_mod(GLFW_KEY_RIGHT, INPUT_MOD_SHIFT)) ? 1.0f : 10.0f;
            }
            if(input_key_state(GLFW_KEY_UP))
            {
                g_image.y += (input_key_state_mod(GLFW_KEY_UP, INPUT_MOD_SHIFT)) ? 1.0f : 10.0f;
            }
            if(input_key_state(GLFW_KEY_DOWN))
            {
                g_image.y -= (input_key_state_mod(GLFW_KEY_DOWN, INPUT_MOD_SHIFT)) ? 1.0f : 10.0f;
            }
        }
    }

    int w = g_image.width;
    int h = g_image.height;
    float aspect_ratio = (float)w / (float)h;

    w = w * g_image.scale;
    h = h * g_image.scale;

    batch_render_quad_textured(ctx, 
        (vec3){g_image.x + ww / 2.0f - w / 2.0f, g_image.y + wh / 2.0f - h / 2.0f, 0.0},
        w, h, g_image.texture);
    text_render_unicode(ctx, &font_info, command.buffer, command.length, 
        0, (vec2){10, 10}, (vec4){0, 0, 100000, 100000}, (vec4){1,1,1,1});
}

#if defined(_WIN32) || defined(_WIN64)
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR lpCmdLine, INT nCmdShow)
{
    int argc = 0;
    PCHAR* argv = CommandLineToArgvA(GetCommandLineA(), &argc);
#else
int main(int argc, char** argv)
{
#endif
	if(glfwInit() == -1) {
        printf("Error: glfw could not initialize\n");
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(1024, 768, "HoImageView", 0, 0);
    if (!window)
    {
        printf("Error: glfw could not create window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if(hogl_init_gl_extensions() == -1) {
        printf("Error: ho_gl could not initialize extensions\n");
        return -1;
    }

    input_set_callbacks(window);
	
	glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

    Hobatch_Context ctx = {0};
    batch_init(&ctx);

    Font_Load_Status fi = font_load(OS_DEFAULT_FONT, &font_info, 20);
    if(fi != FONT_LOAD_OK)
    {
        printf("Error loading font %d\n", fi);
        return 1;
    }

    glEnable(GL_CULL_FACE);
    glCullFace(GL_CCW);

    init(argc, argv);
    glfwSetWindowSize(window, MAX(g_image.width + 10.0f, 640), MAX(g_image.height + 10.0f, 480));

    int width = 0, height = 0;
	while(!glfwWindowShouldClose(window))
	{
        r64 start = os_time_us();
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        ctx.window_width = (r32)width;
        ctx.window_height = (r32)height;
        // Show this frame

        // Poll events for next frame
        glfwPollEvents();

        // clear next frame
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        render(&ctx, width, height);

        batch_flush(&ctx);
        //printf("Flush count: %d\n", ctx.flush_count);
        ctx.flush_count = 0;

#if defined(__linux__)
        double st = 16.0 * 1000.0 - (os_time_us() - start);
        if(st > 0.0) os_usleep(st);
#endif
        //printf("Frame took %f ms\n", (os_time_us() - start)/1000.0);

        glfwSwapBuffers(window);
	}
	return 0;
}