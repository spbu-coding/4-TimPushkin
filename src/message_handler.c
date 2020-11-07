#include <stdlib.h>
#include <stdio.h>

#include "message_handler.h"

void message_handle(const char* const message, enum message_format_t message_format) {
    switch (message_format) {
        case report:
            printf("%s", message);
            break;
        case warning:
            fprintf(stderr, "Warning: %s", message);
            break;
        case error:
            fprintf(stderr, "Error: %s", message);
            break;
        case critical_error:
            fprintf(stderr, "Critical error: %s", message);
            exit(EXIT_FAILURE);
        default:
            fprintf(stderr, "Unknown message format (terminating...): %s", message);
            exit(EXIT_FAILURE);
    }
}
