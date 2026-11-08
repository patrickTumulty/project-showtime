#include "gtl.h"
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "linked_list.h"
#include "shaders_embedded.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct GTLContext
{
    u32 gl_program_shape;
    llist_t windows_list;
} GTLContext;

typedef struct GTLWindow
{
    GLFWwindow *glfw_window;
    GTLContext *context;
    lnode_t lnode;
} GTLWindow;

static GLuint gtl_vao = 0;
static GLuint gtl_vbo = 0;

static void gtl_init_rectangle(void)
{
    glGenVertexArrays(1, &gtl_vao);
    glGenBuffers(1, &gtl_vbo);

    glBindVertexArray(gtl_vao);
    glBindBuffer(GL_ARRAY_BUFFER, gtl_vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);
}

static GLuint compile_shader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        fprintf(stderr, "Shader compilation failed: %s\n", log);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

static u32 gtl_create_program(const char *vs_source, const char *fs_source)
{
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vs_source);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fs_source);

    if (!vs || !fs)
    {
        glDeleteShader(vs);
        glDeleteShader(fs);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        char log[512];
        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        fprintf(stderr, "Shader linking failed: %s\n", log);
        program = 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

GTLContext *gtl_init(void)
{
    GTLContext *context = (GTLContext *)calloc(1, sizeof(GTLContext));
    if (!context)
    {
        return NULL;
    }

    if (!glfwInit())
    {
        free(context);
        return NULL;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    context->gl_program_shape = gtl_create_program(VERTEX_SHADER, FRAGMENT_SHADER);

    return context;
}

GTLWindow *gtl_create_window(GTLContext *context, int width, int height, const char *name)
{
    GTLWindow *gtl_window = (GTLWindow *)calloc(1, sizeof(GTLWindow));
    if (!gtl_window)
        return NULL;

    llist_init_node(&gtl_window->lnode, gtl_window);
    llist_append(&context->windows_list, &gtl_window->lnode);

    gtl_window->glfw_window = glfwCreateWindow(width, height, name, NULL, NULL);
    if (!gtl_window->glfw_window)
    {
        free(gtl_window);
        return NULL;
    }

    glfwMakeContextCurrent(gtl_window->glfw_window);

    if (!gladLoadGL(glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize GLAD\n");
        glfwDestroyWindow(gtl_window->glfw_window);
        free(gtl_window);
        return NULL;
    }

    return gtl_window;
}

void gtl_destroy_window(GTLWindow *window)
{
    if (!window)
        return;

    glfwDestroyWindow(window->glfw_window);
    free(window);
}

bool gtl_window_should_close(const GTLWindow *window)
{
    return glfwWindowShouldClose(window->glfw_window);
}

void gtl_window_get_framebuffer_size(const GTLWindow *window, int *width, int *height)
{
    glfwGetFramebufferSize(window->glfw_window, width, height);
}

void gtl_begin_frame(GTLWindow *window, float r, float g, float b, float a)
{
    int width, height;
    glfwGetFramebufferSize(window->glfw_window, &width, &height);
    glViewport(0, 0, width, height);

    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void gtl_draw_rectangle(GTLWindow *window, float x, float y, float w, float h)
{
    u32 program = window->context->gl_program_shape;
    if (!program)
        return;

    if (!gtl_vao)
        gtl_init_rectangle();

    int fw, fh;
    glfwGetFramebufferSize(window->glfw_window, &fw, &fh);

    float left = x / fw * 2.0f - 1.0f;
    float right = (x + w) / fw * 2.0f - 1.0f;
    float bottom = 1.0f - (y + h) / fh * 2.0f;
    float top = 1.0f - y / fh * 2.0f;

    const float rect[] = {left, top, right, top, right, bottom, left, top, right, bottom, left, bottom};

    glBindVertexArray(gtl_vao);
    glBindBuffer(GL_ARRAY_BUFFER, gtl_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rect), rect, GL_DYNAMIC_DRAW);

    glUseProgram(program);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void gtl_poll_events(void)
{
    glfwPollEvents();
}

void gtl_swap_buffers(GTLWindow *window)
{
    glfwSwapBuffers(window->glfw_window);
}

rc_t gtl_terminate(GTLContext *context)
{
    glfwTerminate();
    return RC_OK;
}
