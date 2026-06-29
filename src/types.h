#ifndef TYPES_H
#define TYPES_H

#define MAX_DEV 20
#define MAX_PORT_LEN 20
#define BUF_SIZE 256

typedef struct {
    char port[MAX_PORT_LEN];
    int vid;
    int pid;
} Device;

#define MAX_RESP_DATA 20
#define MAX_RESP_DATA_NAME_LEN 20

typedef struct {
    int rc;
    char msg[BUF_SIZE];

    int has_data;

    struct {
        char name[MAX_RESP_DATA_NAME_LEN];
        double value;
    } data[MAX_RESP_DATA];

    int data_count;
} Response;


#endif