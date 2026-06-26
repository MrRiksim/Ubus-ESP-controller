#ifndef TYPES_H
#define TYPES_H

#define MAX_DEV 20
#define MAX_PORT_LEN 20

typedef struct {
    char port[MAX_PORT_LEN];
    int vid;
    int pid;
} Device;

#endif