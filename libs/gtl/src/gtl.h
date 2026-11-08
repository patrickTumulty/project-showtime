#ifndef GTL_H
#define GTL_H

#include "crox_types.h"
#include <stdbool.h>

typedef struct GTLWindow GTLWindow;

typedef struct GTLContext GTLContext;

GTLContext *gtl_init(void);

GTLWindow *gtl_create_window(GTLContext *context, int width, int height, const char *name);
void gtl_destroy_window(GTLWindow *window);

bool gtl_window_should_close(const GTLWindow *window);
void gtl_window_get_framebuffer_size(const GTLWindow *window, int *width, int *height);

void gtl_begin_frame(GTLWindow *window, float r, float g, float b, float a);
void gtl_draw_rectangle(GTLWindow *window, float x, float y, float w, float h);

void gtl_poll_events(void);

void gtl_swap_buffers(GTLWindow *window);

rc_t gtl_terminate(GTLContext *context);

#endif // GTL_H
