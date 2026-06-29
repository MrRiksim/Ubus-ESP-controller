#ifndef SPUTILS_H
#define SPUTILS_H

#include "types.h"

extern struct sp_port *port;

int get_devices(Device *list, int max_dev);
int send_on_action(char *port, int pin, Response *resp);
int send_off_action(char *port, int pin, Response *resp);
int send_get_action(char *port, int pin, const char *sensor, const char *model, Response *resp);

#endif