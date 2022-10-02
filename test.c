#include <stdlib.h>
#include <stdio.h>

typedef struct {
    int i1;
    int i2;
    long long ll __attribute__ ((warn_if_not_aligned (16)));
} _struct ;

int main() {
    _struct str;
    str.i1 = 1;
    str.i2 = 2;
    str.ll = 2138203128;
}