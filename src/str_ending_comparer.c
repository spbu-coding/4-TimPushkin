#include "str_ending_comparer.h"

_Bool is_str_ending(const char* const str, const char* const ending) {
    const unsigned int str_len = strlen(str), ending_len = strlen(ending);
    if (str_len >= ending_len && strcmp(str + str_len - ending_len, ending) == 0) {
        return 1;
    } else {
        return 0;
    }
}
