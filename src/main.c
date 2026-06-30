#include <stdio.h>
#include <libubus.h>

#include "ubusutils.h"

int main()
{
    struct ubus_context *ctx;

    uloop_init();

    ctx = ubus_connect(NULL);
    if (!ctx) {
        printf("Error: Failed to connect to ubus.\n");
        return 1;
    }

    ubus_add_uloop(ctx);
    ubus_add_object(ctx, &esp_controller_object);
    uloop_run();

    ubus_free(ctx);
    uloop_done();

    return 0;
}