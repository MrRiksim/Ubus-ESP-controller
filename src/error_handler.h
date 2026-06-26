#ifndef ERROR_CODES_H
#define ERROR_CODES_H

#define SPARG_ERR -1
#define SPFAIL_ERR -2
#define SPMEM_ERR -3
#define SPSUPP_ERR -4
#define SPTIMEOUT_ERR -5
#define NODEV_ERR -6
#define JSONPARSE_ERR -7
#define PORTREAD_ERR -8

int check(int result, char *buffer, size_t size);

#endif