#include "gtl.h"
// clang-format off
#include "glad/gl.h"
#include "GLFW/glfw3.h"
// clang-format on
#include "linked_list.h"
#include "shaders_embedded.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    u32 VAO;           // Vertex Array Object: stores vertex attribute configuration and EBO binding
    u32 VBO;           // Vertex Buffer Object: stores the raw (X, Y) corner coordinates on the GPU
    u32 EBO;           // Element Buffer Object: stores the vertex draw order (6 indices for 2 triangles)
    u32 shaderProgram; // ID of the compiled and linked vertex/fragment shader program
    i32 modelLoc;      // Cached uniform location ID for the 'model' transformation matrix
    i32 colorLoc;      // Cached uniform location ID for the 'ourColor' RGBA color vector
} RectRenderer;

typedef struct
{
    u32 VAO;           // Vertex Array Object: stores vertex attribute configuration
    u32 VBO;           // Vertex Buffer Object: stores the raw (X, Y) fan coordinates on the GPU
    u32 shaderProgram; // ID of the compiled and linked vertex/fragment shader program
    i32 modelLoc;      // Cached uniform location ID for the 'model' transformation matrix
    i32 colorLoc;      // Cached uniform location ID for the 'ourColor' RGBA color vector
} CircleRenderer;

typedef struct
{
    u32 VAO;           // Vertex Array Object: stores vertex attribute configuration and EBO binding
    u32 VBO;           // Vertex Buffer Object: stores the raw (X, Y) corner coordinates on the GPU
    u32 EBO;           // Element Buffer Object: stores the vertex draw order (3 indices for 1 triangle)
    u32 shaderProgram; // ID of the compiled and linked vertex/fragment shader program
    i32 modelLoc;      // Cached uniform location ID for the 'model' transformation matrix
    i32 colorLoc;      // Cached uniform location ID for the 'ourColor' RGBA color vector
} TriangleRenderer;

typedef struct GTLContext
{
    llist_t windows_list;
    GTLWindow *main_window;
    RectRenderer rect_renderer;
    CircleRenderer circle_renderer;
    TriangleRenderer triangle_renderer;
} GTLContext;

typedef struct GTLWindow
{
    GLFWwindow *glfw_window;
    GTLContext *context;
    bool is_main;
    lnode_t lnode;
} GTLWindow;

static void rect_init(RectRenderer *renderer, u32 shader)
{
    renderer->shaderProgram = shader;
    renderer->modelLoc = glGetUniformLocation(shader, "model");
    renderer->colorLoc = glGetUniformLocation(shader, "userColor");

    // 1x1 unit square in 2D (X, Y only)
    // clang-format off
    f32 vertices[] = {
        1.0f, 1.0f,  // Top Right
        1.0f, 0.0f,  // Bottom Right
        0.0f, 0.0f,  // Bottom Left
        0.0f, 1.0f   // Top Left
    };
    // clang-format on
    u32 indices[] = {0, 1, 3, 1, 2, 3};

    glGenVertexArrays(1, &renderer->VAO);
    glGenBuffers(1, &renderer->VBO);
    glGenBuffers(1, &renderer->EBO);

    glBindVertexArray(renderer->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 2 floats per vertex (X, Y)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
}

// 2. Reusable Draw Function (using pixel coordinates and RGBA 0.0-1.0)
static void rect_draw(RectRenderer *renderer, float x, float y, float w, float h, float r, float g, float b, float a)
{
    glUseProgram(renderer->shaderProgram);

    // Column-major 4x4 matrix combining Scale (w, h) and Translation (x, y)
    // clang-format off
    float model[16] = {
        w,    0.0f, 0.0f, 0.0f,
        0.0f, h,    0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        x,    y,    0.0f, 1.0f
    };
    // clang-format on

    glUniformMatrix4fv(renderer->modelLoc, 1, GL_FALSE, model);
    glUniform4f(renderer->colorLoc, r, g, b, a);

    glBindVertexArray(renderer->VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

// 3. Free GPU memory when done
static void rect_cleanup(RectRenderer *renderer)
{
    glDeleteVertexArrays(1, &renderer->VAO);
    glDeleteBuffers(1, &renderer->VBO);
    glDeleteBuffers(1, &renderer->EBO);
}

// Unit circle (diameter 1) centered at (0.5, 0.5), drawn as a triangle fan:
// one center vertex followed by a ring of points around the edge.
#define GTL_CIRCLE_SEGMENTS 64

static void circle_init(CircleRenderer *renderer, u32 shader)
{
    renderer->shaderProgram = shader;
    renderer->modelLoc = glGetUniformLocation(shader, "model");
    renderer->colorLoc = glGetUniformLocation(shader, "userColor");

    // 2 * PI (kept local: M_PI is not available in strict C11 mode)
    const f32 two_pi = 6.283185307179586f;

    // clang-format off
    f32 vertices[GTL_CIRCLE_SEGMENTS + 2][2];
    vertices[0][0] = 0.5f; // Fan center
    vertices[0][1] = 0.5f;
    for (u32 i = 0; i <= GTL_CIRCLE_SEGMENTS; i++)
    {
        f32 angle = ((f32)i / (f32)GTL_CIRCLE_SEGMENTS) * two_pi;
        vertices[i + 1][0] = 0.5f + 0.5f * cosf(angle);
        vertices[i + 1][1] = 0.5f + 0.5f * sinf(angle);
    }
    // clang-format on

    glGenVertexArrays(1, &renderer->VAO);
    glGenBuffers(1, &renderer->VBO);

    glBindVertexArray(renderer->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 2 floats per vertex (X, Y)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
}

// Reusable Draw Function (center point + radius, pixel coordinates and RGBA 0.0-1.0)
static void circle_draw(CircleRenderer *renderer, float cx, float cy, float radius, float r, float g, float b, float a)
{
    glUseProgram(renderer->shaderProgram);

    // Column-major 4x4 matrix scaling the unit circle to 'radius' and
    // translating it so the center lands on (cx, cy)
    float d = 2.0f * radius;
    // clang-format off
    float model[16] = {
        d,          0.0f,      0.0f, 0.0f,
        0.0f,       d,         0.0f, 0.0f,
        0.0f,       0.0f,      1.0f, 0.0f,
        cx - radius, cy - radius, 0.0f, 1.0f
    };
    // clang-format on

    glUniformMatrix4fv(renderer->modelLoc, 1, GL_FALSE, model);
    glUniform4f(renderer->colorLoc, r, g, b, a);

    glBindVertexArray(renderer->VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, GTL_CIRCLE_SEGMENTS + 2);
}

// Free GPU memory when done
static void circle_cleanup(CircleRenderer *renderer)
{
    glDeleteVertexArrays(1, &renderer->VAO);
    glDeleteBuffers(1, &renderer->VBO);
}

// Upward-pointing triangle filling a w x h bounding box, in the same
// unit-space as the rectangle: local (0, 0) is the top-left of the box and
// local (1, 1) the bottom-right.
static void triangle_init(TriangleRenderer *renderer, u32 shader)
{
    renderer->shaderProgram = shader;
    renderer->modelLoc = glGetUniformLocation(shader, "model");
    renderer->colorLoc = glGetUniformLocation(shader, "userColor");

    // Unit triangle (X, Y only); apex at the top, base along the bottom
    // clang-format off
    f32 vertices[] = {
        0.5f, 0.0f,  // Apex (top center)
        0.0f, 1.0f,  // Bottom Left
        1.0f, 1.0f   // Bottom Right
    };
    // clang-format on
    u32 indices[] = {0, 1, 2};

    glGenVertexArrays(1, &renderer->VAO);
    glGenBuffers(1, &renderer->VBO);
    glGenBuffers(1, &renderer->EBO);

    glBindVertexArray(renderer->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 2 floats per vertex (X, Y)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
}

// Reusable Draw Function (pixel coordinates and RGBA 0.0-1.0)
static void triangle_draw(TriangleRenderer *renderer, float x, float y, float w, float h, float r, float g, float b, float a)
{
    glUseProgram(renderer->shaderProgram);

    // Column-major 4x4 matrix combining Scale (w, h) and Translation (x, y)
    // clang-format off
    float model[16] = {
        w,    0.0f, 0.0f, 0.0f,
        0.0f, h,    0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        x,    y,    0.0f, 1.0f
    };
    // clang-format on

    glUniformMatrix4fv(renderer->modelLoc, 1, GL_FALSE, model);
    glUniform4f(renderer->colorLoc, r, g, b, a);

    glBindVertexArray(renderer->VAO);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
}

// Free GPU memory when done
static void triangle_cleanup(TriangleRenderer *renderer)
{
    glDeleteVertexArrays(1, &renderer->VAO);
    glDeleteBuffers(1, &renderer->VBO);
    glDeleteBuffers(1, &renderer->EBO);
}

static GLuint gtl_compile_shader(GLenum type, const char *source)
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

// GL objects can only be created while a GL context is current, which does not
// exist until the first window is created. This builds the program lazily on
// first use instead of in gtl_init().
static u32 gtl_create_program(const char *vs_source, const char *fs_source)
{
    GLuint vs = gtl_compile_shader(GL_VERTEX_SHADER, vs_source);
    GLuint fs = gtl_compile_shader(GL_FRAGMENT_SHADER, fs_source);

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

static void set_ortho_projection(unsigned int shader, float width, float height)
{
    // clang-format off
    float proj[16] = {
        2.0f / width,  0.0f,           0.0f, 0.0f,
        0.0f,         -2.0f / height,  0.0f, 0.0f,
        0.0f,          0.0f,          -1.0f, 0.0f,
       -1.0f,          1.0f,           0.0f, 1.0f
    };
    // clang-format on
    glUseProgram(shader);
    int projLoc = glGetUniformLocation(shader, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, proj);
}

GTLContext *gtl_init_window(const int h, const int w, const char *name)
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

    context->main_window = gtl_create_new_window(context, w, h, name);
    context->main_window->is_main = true;

    u32 program = gtl_create_program(VERTEX_SHADER, FRAGMENT_SHADER);
    if (!program)
    {
        fprintf(stderr, "gtl: rectangle shader program failed to compile/link\n");
        return false;
    }

    rect_init(&context->rect_renderer, program);
    circle_init(&context->circle_renderer, program);
    triangle_init(&context->triangle_renderer, program);

    return context;
}

GTLWindow *gtl_create_new_window(GTLContext *context, const int h, const int w, const char *name)
{
    GTLWindow *gtl_window = (GTLWindow *)calloc(1, sizeof(GTLWindow));
    if (!gtl_window)
        return NULL;

    gtl_window->context = context;
    gtl_window->is_main = false;

    gtl_window->glfw_window = glfwCreateWindow(w, h, name, NULL, NULL);
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

    llist_init_node(&gtl_window->lnode, gtl_window);
    llist_append(&context->windows_list, &gtl_window->lnode);

    return gtl_window;
}

void gtl_destroy_window(GTLWindow *window)
{
    if (!window)
        return;

    // Free the renderers' GL objects while this window's GL context is still
    // current; destroying the window destroys the context.
    rect_cleanup(&window->context->rect_renderer);
    circle_cleanup(&window->context->circle_renderer);
    triangle_cleanup(&window->context->triangle_renderer);

    glfwDestroyWindow(window->glfw_window);
    free(window);
}

bool gtl_main_window_should_close(const GTLContext *context)
{
    return glfwWindowShouldClose(context->main_window->glfw_window);
}

void gtl_main_window_get_framebuffer_size(GTLContext *context, int *h, int *w)
{
    glfwGetFramebufferSize(context->main_window->glfw_window, w, h);
}

void gtl_begin_draw(GTLContext *context, const GTLColor bg)
{
    int width, height;
    glfwGetFramebufferSize(context->main_window->glfw_window, &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(bg.r, bg.g, bg.b, bg.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void gtl_draw_rectangle(GTLContext *context, const GTLColor c, const f32 x, const f32 y, const f32 w, const f32 h)
{
    RectRenderer *renderer = &context->rect_renderer;

    int fw, fh;
    glfwGetFramebufferSize(context->main_window->glfw_window, &fw, &fh);
    set_ortho_projection(renderer->shaderProgram, (float)fw, (float)fh);

    rect_draw(renderer, x, y, w, h, c.r, c.g, c.b, c.a);
}

void gtl_draw_circle(GTLContext *context, const GTLColor c, const f32 cx, const f32 cy, const f32 radius)
{
    CircleRenderer *renderer = &context->circle_renderer;

    int fw, fh;
    glfwGetFramebufferSize(context->main_window->glfw_window, &fw, &fh);
    set_ortho_projection(renderer->shaderProgram, (float)fw, (float)fh);

    circle_draw(renderer, cx, cy, radius, c.r, c.g, c.b, c.a);
}

void gtl_draw_triangle(GTLContext *context, const GTLColor c, const f32 x, const f32 y, const f32 w, const f32 h)
{
    TriangleRenderer *renderer = &context->triangle_renderer;

    int fw, fh;
    glfwGetFramebufferSize(context->main_window->glfw_window, &fw, &fh);
    set_ortho_projection(renderer->shaderProgram, (float)fw, (float)fh);

    triangle_draw(renderer, x, y, w, h, c.r, c.g, c.b, c.a);
}

void gtl_poll_events(void)
{
    glfwPollEvents();
}

void gtl_end_draw(GTLContext *context)
{
    glfwSwapBuffers(context->main_window->glfw_window);
}

rc_t gtl_terminate(GTLContext *context)
{
    glfwTerminate();
    free(context);
    return RC_OK;
}
