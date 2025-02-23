#include <stdio.h>
#include "config.h"
#include "arithmetic.h"
#include "logger.h"
#include "extra.h"

int main() {
    log_info("Starting project: " PROJECT_NAME);
    
    int sum = add(3, 4);
    log_info("Sum is calculated.");

    int diff = subtract(10, 5);
    log_info("Difference is calculated.");

    int prod = extra_multiply(sum, diff);
    log_info("Product is calculated.");

    printf("Final result: %d\n", prod);
    return 0;
}
