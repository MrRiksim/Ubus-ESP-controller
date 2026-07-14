#ifndef ERROR_CODES_H
#define ERROR_CODES_H

#define SPARG_ERR -1
#define SPFAIL_ERR -2
#define SPMEM_ERR -3
#define UNSUPDEV_ERR -4
#define SPTIMEOUT_ERR -5
#define NODEV_ERR -6
#define JSONPARSE_ERR -7
#define PORTREAD_ERR -8
#define ACTION_NOT_VALID -9
#define PIN_NOT_VALID -10
#define MODE_NOT_VALID -11
#define SENSOR_NOT_SUPPORTED -12
#define SENSOR_RETURNED_NO_DATA -13
#define MISSING_ARGUMENTS -14

int check(int result, char *buffer, size_t size);

#endif