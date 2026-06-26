#ifndef SPUTILS_H
#define SPUTILS_H

#include "types.h"

extern struct sp_port *port;

int get_devices(Device *list, int max_dev);
int identify_device(Device *target, int target_vid, int target_pid);
int send_on_action(char *port, int pin, char *resp, int resp_size);
int send_off_action(char *port, int pin, char *resp, int resp_size);
int send_get_action(char *port, int pin, const char *sensor, const char *model, char *resp, int resp_size);

#endif