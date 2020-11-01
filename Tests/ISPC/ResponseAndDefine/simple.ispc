
STRUCT_DEFINE;};

#include "simple_include.h"

export void simple(uniform FLOAT_TYPE vin[], uniform FLOAT_TYPE vout[],
                   uniform int count) {
    foreach (index = 0 ... count) {
        FLOAT_TYPE v = vin[index];
        if (v < M_PI)
            v = v * v;
        else
            v = sqrt(v);
        vout[index] = v;
    }
}
