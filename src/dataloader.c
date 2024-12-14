#include "dataloader.h"

#include <stdio.h>
#include <stdlib.h>

#include "logging.h"

int* loadData_i(const char* file, const char* sep) {
    if (!file || !sep)
        return NULL;

    FILE* df = fopen(file, "r");
    if (!df) {
        LOG_ERROR("Unable to open data file");
        return NULL;
    }

    // pre-allocate at least 20
    const size_t BUCKET = 20;
    size_t n = 0;

}

