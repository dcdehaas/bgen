#include "bgen.h"
#include <stdio.h>
#include <string.h>

int main() {
    struct bgen_file *bgen;

    if ((bgen = bgen_open("data/pbgen_example.pgen")) == NULL) {
        return 0;
    }

    return 1;
}
