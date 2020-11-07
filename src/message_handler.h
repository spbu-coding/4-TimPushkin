#ifndef ERROR_HANDLE_H
#define ERROR_HANDLE_H

enum message_format_t {
    report,
    warning,
    error,
    critical_error
};

void message_handle(const char* message, enum message_format_t message_format);

#endif
