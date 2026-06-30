#include <libubus.h>
#include <libubox/blobmsg_json.h>

#include "error_handler.h"
#include "sputils.h"
#include "types.h"

enum {
    ON_OFF_PORT,
    ON_OFF_PIN,
    __ON_OFF_MAX
};

static const struct blobmsg_policy on_off_policy[] = {
    [ON_OFF_PORT] = { .name = "port", .type = BLOBMSG_TYPE_STRING },
    [ON_OFF_PIN] = { .name = "pin", .type = BLOBMSG_TYPE_INT32 },
};

enum {
    GET_PORT,
    GET_PIN,
    GET_MODEL,
    GET_SENSOR,
    __GET_MAX
};

static const struct blobmsg_policy get_policy[] = {
    [GET_PORT] = { .name = "port", .type = BLOBMSG_TYPE_STRING },
    [GET_PIN] = { .name = "pin", .type = BLOBMSG_TYPE_INT32 },
    [GET_MODEL] = { .name = "model", .type = BLOBMSG_TYPE_STRING },
    [GET_SENSOR] = { .name = "sensor", .type = BLOBMSG_TYPE_STRING },
};


static void devices_to_blobmsg(struct blob_buf *b, Device *devices, int count)
{
    void *array;

    array = blobmsg_open_array(b, "devices");

    for (int i = 0; i < count; i++) {
        void *obj = blobmsg_open_table(b, NULL);

        blobmsg_add_string(b, "port", devices[i].port);
        blobmsg_add_u32(b, "vid", devices[i].vid);
        blobmsg_add_u32(b, "pid", devices[i].pid);

        blobmsg_close_table(b, obj);
    }

    blobmsg_close_array(b, array);
}

int esp_controller_devices(struct ubus_context *ctx, struct ubus_object *obj, 
                           struct ubus_request_data *req, const char *method, 
                           struct blob_attr *msg)
{
    struct blob_buf b = {0};
    blob_buf_init(&b, 0);

    char err_buf[BUF_SIZE];
    Device devices[MAX_DEV];
    int dev_count = check(get_devices(devices, MAX_DEV), err_buf, BUF_SIZE);
    if (dev_count < 0) {
        blobmsg_add_u32(&b, "ret", dev_count);
        blobmsg_add_string(&b, "msg", err_buf);
        ubus_send_reply(ctx, req, b.head);

        blob_buf_free(&b);
        return -1;
    }

    blobmsg_add_u32(&b, "ret", 0);
    devices_to_blobmsg(&b, devices, dev_count);
    ubus_send_reply(ctx, req, b.head);

    blob_buf_free(&b);
    return 0;
}

int esp_controller_on(struct ubus_context *ctx, struct ubus_object *obj, 
                      struct ubus_request_data *req, const char *method, 
                      struct blob_attr *msg)
{
    struct blob_attr *tb[__ON_OFF_MAX];
    struct blob_buf b = {0};

    blobmsg_parse(on_off_policy, __ON_OFF_MAX, tb, blob_data(msg), blob_len(msg));
    if (!tb[ON_OFF_PORT] || !tb[ON_OFF_PIN])
        return UBUS_STATUS_INVALID_ARGUMENT;
    
    char *port = blobmsg_get_string(tb[ON_OFF_PORT]);
    int pin = blobmsg_get_u32(tb[ON_OFF_PIN]);
    
    char err_buf[BUF_SIZE];
    Response resp;

    blob_buf_init(&b, 0);

    int ret = check(send_on_action(port, pin, &resp), err_buf, BUF_SIZE);

    blobmsg_add_u32(&b, "ret", ret);
    if (ret && err_buf[0] != '\0')
        blobmsg_add_string(&b, "msg", err_buf);
    else
        blobmsg_add_string(&b, "msg", resp.msg);
    ubus_send_reply(ctx, req, b.head);
    
    blob_buf_free(&b);
    return 0;
}

int esp_controller_off(struct ubus_context *ctx, struct ubus_object *obj, 
                       struct ubus_request_data *req, const char *method, 
                       struct blob_attr *msg)
{
    struct blob_attr *tb[__ON_OFF_MAX];
    struct blob_buf b = {0};

    blobmsg_parse(on_off_policy, __ON_OFF_MAX, tb, blob_data(msg), blob_len(msg));
    if (!tb[ON_OFF_PORT] || !tb[ON_OFF_PIN])
        return UBUS_STATUS_INVALID_ARGUMENT;
    
    char *port = blobmsg_get_string(tb[ON_OFF_PORT]);
    int pin = blobmsg_get_u32(tb[ON_OFF_PIN]);
    
    char err_buf[BUF_SIZE];
    Response resp;

    blob_buf_init(&b, 0);

    int ret = check(send_off_action(port, pin, &resp), err_buf, BUF_SIZE);
    
    blobmsg_add_u32(&b, "ret", ret);
    if (ret && err_buf[0] != '\0')
        blobmsg_add_string(&b, "msg", err_buf);
    else
        blobmsg_add_string(&b, "msg", resp.msg);
    ubus_send_reply(ctx, req, b.head);
    
    blob_buf_free(&b);
    return 0;
}

static void data_to_blobmsg(struct blob_buf *b, Response resp)
{
    if (!resp.has_data)
        return;
    
    void *obj;
    obj = blobmsg_open_table(b, "data");

    for (int i = 0; i < resp.data_count; i++)
        blobmsg_add_double(b, resp.data[i].name, resp.data[i].value);

    blobmsg_close_table(b, obj);
}

int esp_controller_get(struct ubus_context *ctx, struct ubus_object *obj, 
                       struct ubus_request_data *req, const char *method, 
                       struct blob_attr *msg)
{
    struct blob_attr *tb[__GET_MAX];
    struct blob_buf b = {0};

    blobmsg_parse(get_policy, __GET_MAX, tb, blob_data(msg), blob_len(msg));
    if (!tb[GET_PORT] || !tb[GET_PIN] || !tb[GET_MODEL] || !tb[GET_SENSOR])
        return UBUS_STATUS_INVALID_ARGUMENT;
    
    char *port = blobmsg_get_string(tb[GET_PORT]);
    int pin = blobmsg_get_u32(tb[GET_PIN]);
    char *model = blobmsg_get_string(tb[GET_MODEL]);
    char *sensor = blobmsg_get_string(tb[GET_SENSOR]);
    
    char err_buf[BUF_SIZE];
    Response resp;

    blob_buf_init(&b, 0);

    int ret = check(send_get_action(port, pin, sensor, model, &resp), err_buf, BUF_SIZE);

    blobmsg_add_u32(&b, "ret", ret);
    if (ret && err_buf[0] != '\0')
        blobmsg_add_string(&b, "msg", err_buf);
    else {
        blobmsg_add_string(&b, "msg", resp.msg);
        data_to_blobmsg(&b, resp);
    }
    ubus_send_reply(ctx, req, b.head);
    
    blob_buf_free(&b);
    return 0;
}

const struct ubus_method esp_controller_methods[] = {
    UBUS_METHOD_NOARG("devices", esp_controller_devices),
    UBUS_METHOD("on", esp_controller_on, on_off_policy),
    UBUS_METHOD("off", esp_controller_off, on_off_policy),
    UBUS_METHOD("get", esp_controller_get, get_policy)
};

struct ubus_object_type esp_controller_object_type = 
    UBUS_OBJECT_TYPE("esp_controller", esp_controller_methods);


struct ubus_object esp_controller_object = {
    .name = "esp_controller",
    .type = &esp_controller_object_type,
    .methods = esp_controller_methods,
    .n_methods = ARRAY_SIZE(esp_controller_methods),
};
