#include "bgen/bgen.h"
#include "cass.h"

int main(void)
{
    struct bgen_file* bgen = bgen_file_open("nexist");
    cass_cond(bgen == NULL);

    bgen = bgen_file_open("data/example.v11.bgen");

    cass_equal_int(bgen_file_nsamples(bgen), 500);
    cass_equal_int(bgen_file_nvariants(bgen), 199);
    cass_equal_int(bgen_file_contain_samples(bgen), 0);

    struct bgen_samples* samples = bgen_file_read_samples(bgen, 0);
    cass_cond(samples == NULL);

    bgen_file_close(bgen);

    bgen = bgen_file_open("data/example.14bits.bgen");
    cass_cond(bgen != NULL);

    cass_equal_int(bgen_file_nsamples(bgen), 500);
    cass_equal_int(bgen_file_nvariants(bgen), 199);
    cass_equal_int(bgen_file_contain_samples(bgen), 1);

    samples = bgen_file_read_samples(bgen, 0);
    cass_cond(samples != NULL);
    bgen_samples_free(samples);

    bgen_file_close(bgen);

    bgen = bgen_file_open("data/example.14bits.bgen");
    struct bgen_metafile* metafile = bgen_metafile_create(
        bgen, "assert_interface_2.tmp/example.14bits.bgen.metadata", 1, 0);
    struct bgen_partition const* partition = bgen_metafile_read_partition(metafile, 0);

    struct bgen_variant const* vm = bgen_partition_get(partition, 0);
    cass_equal_int(vm->position, 2000);
    cass_equal_int(vm->nalleles, 2);
    cass_cond(bgen_str_equal(BGEN_STR("RSID_2"), *vm->rsid));
    cass_cond(bgen_str_equal(BGEN_STR("SNPID_2"), *vm->id));
    cass_cond(bgen_str_equal(BGEN_STR("01"), *vm->chrom));
    cass_cond(bgen_str_equal(BGEN_STR("A"), *vm->allele_ids[0]));

    bgen_partition_destroy(partition);
    bgen_metafile_close(metafile);
    bgen_file_close(bgen);

    bgen = bgen_file_open("data/example.14bits.bgen");
    metafile = bgen_metafile_open("assert_interface_2.tmp/example.14bits.bgen.metadata");
    partition = bgen_metafile_read_partition(metafile, 0);
    vm = bgen_partition_get(partition, 3);
    struct bgen_genotype* genotype = bgen_file_open_genotype(bgen, vm->genotype_offset);

    cass_equal_int(bgen_genotype_nalleles(genotype), 2);
    cass_equal_int(bgen_genotype_missing(genotype, 3), 0);
    cass_equal_int(bgen_genotype_ploidy(genotype, 3), 2);
    cass_equal_int(bgen_genotype_min_ploidy(genotype), 2);
    cass_equal_int(bgen_genotype_max_ploidy(genotype), 2);
    cass_equal_int(bgen_genotype_ncombs(genotype), 3);
    cass_equal_int(bgen_genotype_phased(genotype), 0);

    double* probs = malloc(500 * 3 * sizeof(double));
    bgen_genotype_read(genotype, probs);
    cass_close(probs[0], 0.00488311054141488121);
    cass_close(probs[1], 0.02838308002197399704);
    cass_close(probs[2], 0.96673380943661113562);
    cass_close(probs[3], 0.99047793444424092613);

    bgen_partition_destroy(partition);
    bgen_genotype_close(genotype);
    bgen_metafile_close(metafile);
    bgen_file_close(bgen);

    return cass_status();
}
