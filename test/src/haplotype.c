#include "bgen/bgen.h"
#include "cass.h"
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_haplotype(void);

int main(void)
{
    test_haplotype();
    return cass_status();
}

void test_haplotype(void)
{
    size_t            i, j, ii, jj;
    const char        filename[] = "data/haplotypes.bgen";
    struct bgen_file* bgen;
    uint32_t          nsamples, nvariants;

    cass_cond((bgen = bgen_file_open(filename)) != NULL);
    cass_cond((nsamples = bgen_file_nsamples(bgen)) == 4);
    cass_cond((nvariants = bgen_file_nvariants(bgen)) == 4);

    struct bgen_samples* samples = bgen_file_read_samples(bgen, 0);

    cass_cond(bgen_str_equal(BGEN_STR("sample_0"), *bgen_samples_get(samples, 0)));

    bgen_samples_free(samples);

    struct bgen_mf* mf = bgen_metafile_create(bgen, "complex.23bits.bgen.metadata.1", 1, 0);

    cass_cond(mf != NULL);
    cass_cond(bgen_metafile_npartitions(mf) == 1);

    struct bgen_partition* partition = bgen_metafile_read_partition2(mf, 0);
    cass_cond(partition != NULL);

    struct bgen_variant const* vm = bgen_partition_get(partition, 0);
    cass_cond(bgen_str_equal(BGEN_STR("RS1"), *vm->rsid));
    cass_cond(vm->nalleles == 2);

    for (i = 0; i < (size_t)nvariants; ++i) {
        vm = bgen_partition_get(partition, i);
        cass_cond(bgen_str_equal(BGEN_STR("A"), *vm->allele_ids[0]));
        cass_cond(bgen_str_equal(BGEN_STR("G"), *vm->allele_ids[1]));
    }

    vm = bgen_partition_get(partition, 1);
    struct bgen_genotype* vg = bgen_file_open_genotype(bgen, vm->genotype_offset);

    cass_cond(bgen_genotype_ncombs(vg) == 4);
    cass_cond(bgen_genotype_ploidy(vg, 0) == 2);
    cass_cond(bgen_genotype_min_ploidy(vg) == 2);
    cass_cond(bgen_genotype_max_ploidy(vg) == 2);
    cass_cond(bgen_genotype_nalleles(vg) == 2);
    cass_cond(bgen_genotype_phased(vg) == 1);

    bgen_genotype_close(vg);

    double real_probs[] = {1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0,
                           1.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0,
                           0.0, 1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0,
                           1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0,
                           1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0};

    jj = 0;
    for (i = 0; i < (size_t)nvariants; ++i) {
        vm = bgen_partition_get(partition, i);
        vg = bgen_file_open_genotype(bgen, vm->genotype_offset);

        double* probabilities = malloc(nsamples * bgen_genotype_ncombs(vg) * sizeof(double));

        cass_cond(bgen_genotype_read(vg, probabilities) == 0);

        for (ii = 0; ii < (size_t)nsamples; ++ii) {
            for (j = 0; j < (size_t)bgen_genotype_ncombs(vg); ++j) {
                cass_cond(probabilities[ii * bgen_genotype_ncombs(vg) + j] == real_probs[jj]);
                jj++;
            }
        }
        bgen_genotype_close(vg);
        free(probabilities);
    }

    bgen_partition_destroy(partition);
    cass_cond(bgen_mf_close(mf) == 0);
    bgen_file_close(bgen);
}
