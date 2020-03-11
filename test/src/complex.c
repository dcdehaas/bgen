#include "bgen/bgen.h"
#include "cass.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_complex(void);

int main(void)
{
    test_complex();
    return cass_status();
}

void test_complex(void)
{
    size_t j, ii;
    const char filename[] = "data/complex.23bits.bgen";
    struct bgen_file* bgen;
    uint32_t nsamples, nvariants;
    double* probabilities;

    cass_cond((bgen = bgen_file_open(filename)) != NULL);
    cass_cond((nsamples = bgen_file_nsamples(bgen)) == 4);
    cass_cond((nvariants = bgen_file_nvariants(bgen)) == 10);

    struct bgen_samples* samples = bgen_file_read_samples(bgen, 0);

    cass_cond(bgen_str_equal(BGEN_STR("sample_0"), *bgen_samples_get(samples, 0)));
    cass_cond(bgen_str_equal(BGEN_STR("sample_3"), *bgen_samples_get(samples, 3)));

    bgen_samples_free(samples);

    struct bgen_mf* mf = bgen_metafile_create(bgen, "complex.23bits.bgen.metadata", 1, 0);

    struct bgen_partition* partition = bgen_metafile_read_partition2(mf, 0);
    cass_cond(nvariants == 10);

    struct bgen_variant const* vm = bgen_partition_get(partition, 0);
    cass_cond(bgen_str_equal(BGEN_STR("V1"), *vm->rsid));
    vm = bgen_partition_get(partition, 9);
    cass_cond(bgen_str_equal(BGEN_STR("M10"), *vm->rsid));

    int position[] = {1, 2, 3, 4, 5, 7, 7, 8, 9, 10};
    int correct_nalleles[] = {2, 2, 2, 3, 2, 4, 6, 7, 8, 2};
    char* allele_ids[] = {"A",    "G",     "A",      "G",       "A",     "G",  "A",   "G",
                          "T",    "A",     "G",      "A",       "G",     "GT", "GTT", "A",
                          "G",    "GT",    "GTT",    "GTTT",    "GTTTT", "A",  "G",   "GT",
                          "GTT",  "GTTT",  "GTTTT",  "GTTTTT",  "A",     "G",  "GT",  "GTT",
                          "GTTT", "GTTTT", "GTTTTT", "GTTTTTT", "A",     "G"};

    uint32_t jj = 0;
    for (uint32_t i = 0; i < nvariants; ++i) {
        vm = bgen_partition_get(partition, i);
        cass_cond(vm->nalleles == correct_nalleles[i]);
        cass_cond(vm->position == position[i]);

        for (uint16_t j = 0; j < vm->nalleles; ++j) {

            cass_cond(bgen_str_equal(BGEN_STR(allele_ids[jj]), *vm->allele_ids[j]));
            ++jj;
        }
    }

    int phased[] = {0, 1, 1, 0, 1, 1, 1, 1, 0, 0};

    int ploidys[] = {1, 2, 2, 2, 1, 1, 1, 1, 1, 2, 2, 2, 1, 2, 2, 2, 1, 3, 3, 2,
                     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 4, 4, 4, 4};
    double real_probs[] = {
        1.000000, 0.000000, NAN,      1.000000, 0.000000, 0.000000, 1.000000, 0.000000,
        0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.000000,
        1.000000, 0.000000, 0.000000, 1.000000, 1.000000, 0.000000, NAN,      NAN,
        0.000000, 1.000000, 0.000000, 1.000000, 1.000000, 0.000000, 1.000000, 0.000000,
        1.000000, 0.000000, 0.000000, 1.000000, 1.000000, 0.000000, 0.000000, NAN,
        NAN,      NAN,      1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
        0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
        0.000000, 1.000000, 0.000000, 0.000000, 1.000000, 0.000000, NAN,      NAN,
        NAN,      NAN,      1.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.000000,
        1.000000, 0.000000, 1.000000, 0.000000, 0.000000, 1.000000, 1.000000, 0.000000,
        0.000000, 1.000000, NAN,      NAN,      1.000000, 0.000000, 0.000000, 0.000000,
        0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000,
        0.000000, 0.000000, 0.000000, 1.000000, 1.000000, 0.000000, 0.000000, 0.000000,
        0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000,
        0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000,
        0.000000, 0.000000, 0.000000, 1.000000, 1.000000, 0.000000, 0.000000, 0.000000,
        0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000,
        0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000,
        0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000,
        1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
        NAN,      NAN,      NAN,      NAN,      NAN,      NAN,      NAN,      NAN,
        NAN,      NAN,      NAN,      NAN,      NAN,      NAN,      NAN,      NAN,
        NAN,      NAN,      NAN,      NAN,      NAN,      NAN,      NAN,      NAN,
        NAN,      NAN,      NAN,      NAN,      0.000000, 0.000000, 0.000000, 0.000000,
        0.000000, 0.000000, 1.000000, 0.000000, NAN,      NAN,      NAN,      NAN,
        NAN,      NAN,      NAN,      NAN,      NAN,      NAN,      NAN,      NAN,
        NAN,      NAN,      NAN,      NAN,      NAN,      NAN,      NAN,      NAN,
        NAN,      NAN,      NAN,      NAN,      NAN,      NAN,      NAN,      NAN,
        0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000,
        NAN,      NAN,      NAN,      NAN,      NAN,      NAN,      NAN,      NAN,
        NAN,      NAN,      NAN,      NAN,      NAN,      NAN,      NAN,      NAN,
        NAN,      NAN,      NAN,      NAN,      NAN,      NAN,      NAN,      NAN,
        NAN,      NAN,      NAN,      NAN,      0.000000, 0.000000, 0.000000, 0.000000,
        0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
        0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000,
        0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
        0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
        1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000,
        0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000,
        0.000000, 0.000000, 1.000000, 0.000000};

    double* rp = real_probs;

    jj = 0;
    for (uint32_t i = 0; i < nvariants; ++i) {
        vm = bgen_partition_get(partition, i);
        struct bgen_genotype* vg = bgen_file_open_genotype(bgen, vm->genotype_offset);
        cass_cond(bgen_genotype_phased(vg) == phased[i]);

        probabilities = malloc(nsamples * bgen_genotype_ncombs(vg) * sizeof(double));
        bgen_genotype_read(vg, probabilities);

        double* p = probabilities;
        for (uint16_t j = 0; j < nsamples; ++j) {

            cass_cond(ploidys[jj] == bgen_genotype_ploidy(vg, j));
            cass_cond(bgen_genotype_missing(vg, j) == 0);

            for (ii = 0; ii < (size_t)bgen_genotype_ncombs(vg); ++ii) {
                cass_cond(!(*rp != *p && !(isnan(*rp) && isnan(*p))));
                ++rp;
                ++p;
            }
            ++jj;
        }
        bgen_genotype_close(vg);
        free(probabilities);
    }

    bgen_partition_destroy(partition);
    cass_cond(bgen_mf_close(mf) == 0);
    bgen_file_close(bgen);
}
