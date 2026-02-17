#include "intmath.h"

int16_t Ceiling(int16_t x, int16_t y) {
    int16_t z;
    z = x / y;
    if (x > (z * y)) {
        z++;
    }
    return z;
}
