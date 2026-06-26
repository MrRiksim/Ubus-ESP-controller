#include <libserialport.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "sputils.h"
#include "error_handler.h"
#include "cJSON.h"

int get_devices(Device *list, int max_dev)
{
    struct sp_port **port_list;

    enum sp_return result = sp_list_ports(&port_list);
    if (result)
        return result;

    int i;
    for (i = 0; port_list[i] != NULL; i++) {
        struct sp_port *port = port_list[i];
        Device temp;

        char *port_name = sp_get_port_name(port);
        int vid;
        int pid;
        int ret = sp_get_port_usb_vid_pid(port, &vid, &pid);

        strncpy(temp.port, port_name, MAX_PORT_LEN);
        temp.port[MAX_PORT_LEN - 1] = '\0';
        if (!ret) {
            temp.vid = vid;
            temp.pid = pid;
        } else {
            temp.vid = -1;
            temp.pid = -1;
        }

        list[i] = temp;
    }

    sp_free_port_list(port_list);

    return i;
}

int identify_device(Device *target, int target_vid, int target_pid)
{
    struct sp_port **port_list;

    enum sp_return result = sp_list_ports(&port_list);
    if (result)
        return result;

    for (int i = 0; port_list[i] != NULL; i++) {
        struct sp_port *port = port_list[i];

        if (sp_get_port_transport(port) != SP_TRANSPORT_USB)
            continue;

        int vid;
        int pid;
        int ret = sp_get_port_usb_vid_pid(port, &vid, &pid);
        if (ret)
            continue;

        if (vid == target_vid && pid == target_pid) {
            char *port_name = sp_get_port_name(port);

            strncpy((*target).port, port_name, MAX_PORT_LEN);
            (*target).port[MAX_PORT_LEN - 1] = '\0';
            (*target).vid = vid;
            (*target).pid = pid;

            sp_free_port_list(port_list);
            return 0;
        }
    }

    if (port_list != NULL)
        sp_free_port_list(port_list);
    
    return NODEV_ERR;
}

static int setup_port(struct sp_port *port)
{
    int ret;

    if ((ret = sp_open(port, SP_MODE_READ_WRITE), "sp_open")) return ret;
    if ((ret = sp_set_baudrate(port, 9600), "sp_set_baudrate")) return ret;
    if ((ret = sp_set_bits(port, 8), "sp_set_bits")) return ret;
    if ((ret = sp_set_parity(port, SP_PARITY_NONE), "sp_set_parity")) return ret;
    if ((ret = sp_set_stopbits(port, 1), "sp_set_stopbits")) return ret;
    if ((ret = sp_set_flowcontrol(port, SP_FLOWCONTROL_NONE), "sp_set_flowcontrol")) return ret;

    return 0;
}

static int parse_json(char *src, char *out, int size)
{
    cJSON *json = cJSON_Parse(src);
    if (!json)
        return JSONPARSE_ERR;

    cJSON *rc_item = cJSON_GetObjectItem(json, "rc");
    if (!cJSON_IsNumber(rc_item)) {
        cJSON_Delete(json);
        return JSONPARSE_ERR;
    }

    int rc = rc_item->valueint;

    cJSON *msg_item = cJSON_GetObjectItem(json, "msg");
    if (cJSON_IsString(msg_item) && msg_item->valuestring) {
        strncpy(out, msg_item->valuestring, size - 1);
        out[size - 1] = '\0';
    } else {
        out[0] = '\0';
    }

    cJSON_Delete(json);
    return rc;
}

static int send_message(char *port_name, char *data, char *resp, int resp_size)
{
    struct sp_port *port;
    char resp_json[resp_size];
    int size = strlen(data);
    unsigned int timeout = 3000;

    int ret = sp_get_port_by_name(port_name, &port);
    if (ret)
        return ret;
    
    ret = setup_port(port);
    if (ret)
        goto cleanup;

    ret = sp_blocking_write(port, data, size, timeout);
    if (ret < 0) {
        goto cleanup;
    } else if (ret < size) {
        ret = SPTIMEOUT_ERR;
        goto cleanup;
    }

    sp_flush(port, SP_BUF_INPUT);

    ret = sp_blocking_read(port, resp_json, resp_size, timeout);
    if (ret < 0) {
        resp_json[0] = '\0';
        goto cleanup;
    } else if (ret == 0) {
        ret = PORTREAD_ERR;
        resp_json[0] = '\0';
        goto cleanup;
    }
    resp_json[ret] = '\0';
    ret = parse_json(resp_json, resp, resp_size);
    if (ret)
        goto cleanup;

    ret = 0;
    goto cleanup;

cleanup:
    sp_close(port);
    sp_free_port(port);
    return ret;
}

int send_on_action(char *port, int pin, char *resp, int resp_size)
{
    char params[64];
    snprintf(params, sizeof(params), "{\"pin\": %d, \"action\": \"on\"}", pin);

    return send_message(port, params, resp, resp_size);
}

int send_off_action(char *port, int pin, char *resp, int resp_size)
{
    char params[64];
    snprintf(params, sizeof(params), "{\"pin\": %d, \"action\": \"off\"}", pin);

    return send_message(port, params, resp, resp_size);
}

int send_get_action(char *port, int pin, const char *sensor, const char *model, char *resp, int resp_size)
{
    char params[128];
    snprintf(params, sizeof(params), "{\"action\": \"get\", \"sensor\": \"%s\", \"pin\": %d, \"model\": \"%s\"}", sensor, pin, model);

    return send_message(port, params, resp, resp_size);
}