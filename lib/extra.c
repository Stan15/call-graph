#include "extra.h"
#include "logger.h"

int extra_multiply(int a, int b) {
    log_debug("Multiplying numbers with extra module");
    return a * b;
}
