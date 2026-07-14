#include <libserialport.h>
#include <string.h>
#include <stdio.h>

#include "sputils.h"
#include "error_handler.h"
#include "cJSON.h"

static int is_possible_esp_board(struct sp_port *port)
{
    int vid, pid;

    if (port == NULL)
        return 0;

    if (sp_get_port_usb_vid_pid(port, &vid, &pid) != SP_OK)
        return 0;

    switch (vid)
    {
        /* Espressif native USB */
        case 0x303A:
            return 1;

        /* Silicon Labs CP210x */
        case 0x10C4:
            return 1;

        /* QinHeng / WCH CH340, CH9102 */
        // case 0x1A86:
        //     return 1;

        /* FTDI FT232 */
        // case 0x0403:
        //     return 1;

        default:
            return 0;
    }
}

int get_devices(Device *list, int max_dev)
{
    struct sp_port **port_list;

    enum sp_return result = sp_list_ports(&port_list);
    if (result)
        return result;

    int i;
    int count = 0;
    for (i = 0; port_list[i] != NULL; i++) {
        struct sp_port *port = port_list[i];
        if (!is_possible_esp_board(port))
            continue;
        Device temp;

        int vid, pid;
        sp_get_port_usb_vid_pid(port, &vid, &pid);
        char *port_name = sp_get_port_name(port);

        strncpy(temp.port, port_name, MAX_PORT_LEN);
        temp.port[MAX_PORT_LEN - 1] = '\0';
        temp.vid = vid;
        temp.pid = pid;

        list[count] = temp;
        count++;
    }

    sp_free_port_list(port_list);

    return count;
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

static int change_error_code(int code)
{
    switch (code) {
        case 101:
            return ACTION_NOT_VALID;
        case 102:
            return PIN_NOT_VALID;
        case 103:
            return MODE_NOT_VALID;
        case 104:
            return SENSOR_NOT_SUPPORTED;
        case 105:
            return SENSOR_RETURNED_NO_DATA;
        case 106:
            return MISSING_ARGUMENTS;
    }

    return code;
}

static int parse_json(const char *src, Response *resp)
{
    cJSON *json = cJSON_Parse(src);
    if (!json)
        return JSONPARSE_ERR;

    cJSON *rc_item = cJSON_GetObjectItem(json, "rc");
    if (!cJSON_IsNumber(rc_item)) {
        cJSON_Delete(json);
        return JSONPARSE_ERR;
    }
    resp->rc = change_error_code(rc_item->valueint);

    cJSON *msg_item = cJSON_GetObjectItem(json, "msg");
    if (cJSON_IsString(msg_item) && msg_item->valuestring) {
        strncpy(resp->msg, msg_item->valuestring, sizeof(resp->msg)-1);
        resp->msg[sizeof(resp->msg)-1] = '\0';
    } else {
        resp->msg[0] = '\0';
    }

    resp->data_count = 0;
    resp->has_data = 0;

    cJSON *data_item = cJSON_GetObjectItem(json, "data");
    if (cJSON_IsObject(data_item)) {
        resp->has_data = 1;

        for (cJSON *child = data_item->child; child != NULL && resp->data_count < MAX_RESP_DATA; child = child->next) {
            if (child->string && cJSON_IsNumber(child)) {
                strncpy(resp->data[resp->data_count].name, child->string, MAX_RESP_DATA_NAME_LEN);
                resp->data[resp->data_count].value = child->valuedouble;

                resp->data_count++;
            }
        }
    }

    cJSON_Delete(json);
    return resp->rc;
}

static int send_message(char *port_name, char *data, Response *resp)
{
    struct sp_port *port;
    char resp_json[BUF_SIZE];
    int size = strlen(data);
    unsigned int timeout = 3000;

    int ret = sp_get_port_by_name(port_name, &port);
    if (ret)
        return ret;
    
    if (!is_possible_esp_board(port)) {
        sp_free_port(port);
        return UNSUPDEV_ERR;
    } 
    
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

    ret = sp_blocking_read(port, resp_json, BUF_SIZE, timeout);
    if (ret < 0) {
        resp_json[0] = '\0';
        goto cleanup;
    } else if (ret == 0) {
        ret = PORTREAD_ERR;
        resp_json[0] = '\0';
        goto cleanup;
    }
    resp_json[ret] = '\0';
    ret = parse_json(resp_json, resp);
    if (ret)
        goto cleanup;

    ret = 0;
    goto cleanup;

cleanup:
    sp_close(port);
    sp_free_port(port);
    return ret;
}

int send_on_action(char *port, int pin, Response *resp)
{
    char params[64];
    snprintf(params, sizeof(params), "{\"pin\": %d, \"action\": \"on\"}", pin);

    return send_message(port, params, resp);
}

int send_off_action(char *port, int pin, Response *resp)
{
    char params[64];
    snprintf(params, sizeof(params), "{\"pin\": %d, \"action\": \"off\"}", pin);

    return send_message(port, params, resp);
}

int send_get_action(char *port, int pin, const char *sensor, const char *model, Response *resp)
{
    char params[128];
    snprintf(params, sizeof(params), 
            "{\"action\": \"get\", \"sensor\": \"%s\", \"pin\": %d, \"model\": \"%s\"}", 
            sensor, pin, model);

    return send_message(port, params, resp);
}