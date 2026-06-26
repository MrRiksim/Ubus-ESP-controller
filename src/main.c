#include <libserialport.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libubus.h>

#include "types.h"
#include "sputils.h"
#include "error_handler.h"
#include "ubusutils.h"

#define TARGET_VID 4292
#define TARGET_PID 60000

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