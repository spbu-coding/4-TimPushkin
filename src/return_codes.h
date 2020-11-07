#ifndef RETURN_CODES_H
#define RETURN_CODES_H

enum return_codes_t {
    success = 0,
    runtime_error = -1,
    pixel_data_corruption = -2,
    qdbmp_runtime_error = -3
};

#endif
