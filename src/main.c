#include "crox_types.h"
#include "crox_utils.h"
#include "gtl.h"
#include <math.h>

int main(void)
{
    GTLContext *context = gtl_init_window(1000, 1000, "Hello, GTL!!!");
    if (!context)
    {
        gtl_terminate(context);
        return -1;
    }

    int x = 450;
    int y = 450;
    int h = 100;
    int w = 100;
    u64 nextDraw = 0;

    while (!gtl_main_window_should_close(context))
    {
        u64 t1 = crox_time_get_monotonic_ms();
        if (t1 >= nextDraw)
        {
            nextDraw = t1 + 16;
            gtl_begin_draw(context, GTL_COLOR_PINK);
            {
                // f32 now_seconds = ((f32)t1) * 0.001f;
                // y += (int)roundf(sinf(2.0f * CROX_PI_F * 0.5f * now_seconds) * 10);
                gtl_draw_rectangle(context, GTL_COLOR_BLUE, x, y, w, h);
            }
            gtl_end_draw(context);
        }
        gtl_poll_events();
    }

    // Cleanup
    gtl_terminate(context);

    return 0;
}
