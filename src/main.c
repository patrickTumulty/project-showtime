#include "gtl.h"

int main(void)
{
    GTLContext *context = gtl_init();
    if (!context)
    {
        gtl_terminate(context);
        return -1;
    }

    GTLWindow *window = gtl_create_window(context, 800, 600, "Rectangle");
    if (!window)
    {
        gtl_terminate(context);
        return -1;
    }

    int width, height;
    gtl_window_get_framebuffer_size(window, &width, &height);

    // Rendering loop
    while (!gtl_window_should_close(window))
    {
        gtl_begin_frame(window, 0.1f, 0.1f, 0.1f, 1.0f);

        // Rectangle in the middle half of the window
        gtl_draw_rectangle(window, width / 4.0f, height / 4.0f, width / 2.0f, height / 2.0f);

        gtl_swap_buffers(window);

        gtl_poll_events();
    }

    // Cleanup
    gtl_destroy_window(window);
    gtl_terminate(context);

    return 0;
}
