
//textual error if STRUCT_DEFINE not set
STRUCT_DEFINE;};

export void simple(uniform float vin[], uniform float vout[],
                   uniform int count) {
    foreach (index = 0 ... count) {
        float v = vin[index];
        if (v < M_PI)
            v = v * v;
        else
            v = sqrt(v);
        vout[index] = v;
    }
}
