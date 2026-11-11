#ifndef GTL_H
#define GTL_H

#include "crox_types.h"
#include <stdbool.h>

typedef struct GTLWindow GTLWindow;

typedef struct GTLContext GTLContext;

typedef struct GTLColor
{
    float r;
    float g;
    float b;
    float a;
} GTLColor;

static const GTLColor GTL_COLOR_BLACK = {0.0f, 0.0f, 0.0f, 1.0f};
static const GTLColor GTL_COLOR_WHITE = {1.0f, 1.0f, 1.0f, 1.0f};
static const GTLColor GTL_COLOR_RED = {1.0f, 0.0f, 0.0f, 1.0f};
static const GTLColor GTL_COLOR_GREEN = {0.0f, 1.0f, 0.0f, 1.0f};
static const GTLColor GTL_COLOR_BLUE = {0.0f, 0.0f, 1.0f, 1.0f};

static const GTLColor GTL_COLOR_YELLOW = {1.0f, 1.0f, 0.0f, 1.0f};
static const GTLColor GTL_COLOR_CYAN = {0.0f, 1.0f, 1.0f, 1.0f};
static const GTLColor GTL_COLOR_MAGENTA = {1.0f, 0.0f, 1.0f, 1.0f};

static const GTLColor GTL_COLOR_ORANGE = {1.0f, 0.647f, 0.0f, 1.0f};
static const GTLColor GTL_COLOR_PURPLE = {0.5f, 0.0f, 0.5f, 1.0f};
static const GTLColor GTL_COLOR_PINK = {1.0f, 0.753f, 0.796f, 1.0f};
static const GTLColor GTL_COLOR_BROWN = {0.647f, 0.165f, 0.165f, 1.0f};

static const GTLColor GTL_COLOR_GRAY = {0.5f, 0.5f, 0.5f, 1.0f};
static const GTLColor GTL_COLOR_LIGHT_GRAY = {0.75f, 0.75f, 0.75f, 1.0f};
static const GTLColor GTL_COLOR_DARK_GRAY = {0.25f, 0.25f, 0.25f, 1.0f};

static const GTLColor GTL_COLOR_TRANSPARENT = {0.0f, 0.0f, 0.0f, 0.0f};

GTLContext *gtl_init_window(const int h, const int w, const char *name);
GTLWindow *gtl_create_new_window(GTLContext *context, const int h, const int w, const char *name);

void gtl_destroy_window(GTLWindow *window);

bool gtl_main_window_should_close(const GTLContext *context);
void gtl_main_window_get_framebuffer_size(GTLContext *context, int *h, int *w);

void gtl_begin_draw(GTLContext *context, const GTLColor bg);
void gtl_end_draw(GTLContext *context);

void gtl_draw_rectangle(GTLContext *context, const GTLColor color, const f32 x, const f32 y, const f32 w, const f32 h);

void gtl_poll_events(void);
rc_t gtl_terminate(GTLContext *context);

#endif // GTL_H
