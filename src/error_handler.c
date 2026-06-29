#include <stdio.h>
#include <libserialport.h>
#include <string.h>

#include "error_handler.h"

int check(int result, char *buffer, size_t size)
{
    if (buffer == NULL || size == 0)
        return result;

    buffer[0] = '\0';
    char *error_message;

    switch (result) {
    case SPARG_ERR:
        snprintf(buffer, size, "Error: Invalid argument.");
        return SPARG_ERR;

    case SPFAIL_ERR:
        error_message = sp_last_error_message();
        snprintf(buffer, size, "Error: Failed: %s", error_message);
        sp_free_error_message(error_message);
        return SPFAIL_ERR;

    case SPSUPP_ERR:
        snprintf(buffer, size, "Error: Not supported.");
        return SPSUPP_ERR;

    case SPMEM_ERR:
        snprintf(buffer, size, "Error: Couldn't allocate memory.");
        return SPMEM_ERR;

    case SPTIMEOUT_ERR:
        snprintf(buffer, size, "Error: Timed out while sending data.");
        return SPTIMEOUT_ERR;

    case NODEV_ERR:
        snprintf(buffer, size, "Error: Device not found.");
        return NODEV_ERR;

    case JSONPARSE_ERR:
        snprintf(buffer, size, "Error: Response message JSON parse failed.");
        return JSONPARSE_ERR;
    
    case PORTREAD_ERR:
        snprintf(buffer, size, "Error: Port did not respond.");
        return PORTREAD_ERR;

    default:
        return result;
    }
}
