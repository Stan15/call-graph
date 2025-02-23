#include <stdio.h>
#include "logger.h"

void log_info(const char *message) {
    printf("[INFO] %s\n", message);
}

void log_debug(const char *message) {
    printf("[DEBUG] %s\n", message);
}
