#include "app_core.h"
#include <stdio.h>

void app_init(void) {
    printf("[app core] init");
}

void app_run(void) {
    printf("[app_core] run");
}

void app_shutdown(void) {
    printf("[app_core' shutdown]");
}

int main(void) {
    app_init();
    app_run();
    app_shutdown();
    return 0;
}
