#include "arithmetic.h"
#include "logger.h"

int add(int a, int b) {
    log_info("Adding numbers");
    return a + b;
}

int subtract(int a, int b) {
    log_info("Subtracting numbers");
    return a - b;
}
