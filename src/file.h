#ifndef _BGEN_FILE_H
#define _BGEN_FILE_H

#include "bgen.h"
#include "io.h"
#include <stdio.h>

struct bgen_file
{
    char *filepath;
    FILE *file;
    int nvariants;
    int nsamples;
    int compression;
    int layout;
    int contain_sample;
    int sample_ids_presence BGEN_DEPRECATED;
    int samples_start;
    int variants_start;
};

#define bopen_or_leave(BGEN)                                                           \
    if (!(BGEN->file = fopen(BGEN->filepath, "rb"))) {                                 \
        perror_fmt("Could not open bgen file %s", BGEN->filepath);                     \
        goto err;                                                                      \
    }

int close_bgen_file(struct bgen_file *bgen);

#endif /* _BGEN_FILE_H */
