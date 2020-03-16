#include "bgen/metafile_04.h"
#include "bgen/file.h"
#include "bgen/variant.h"
#include "bmath.h"
#include "bstring.h"
#include "file.h"
#include "free.h"
#include "io.h"
#include "metafile_04.h"
#include "metafile_write_04.h"
#include "partition.h"
#include "report.h"
#include "bgen/bstring.h"
#include <string.h>

static struct bgen_metafile_04* metafile_alloc_04(char const* filepath);
static uint32_t                 compute_nvariants_04(uint32_t nvariants, uint32_t npartitions,
                                                     uint32_t partition);

struct bgen_metafile_04* bgen_metafile_create_04(struct bgen_file* bgen_file,
                                                 char const* filepath, uint32_t npartitions,
                                                 int verbose)
{
    struct bgen_metafile_04* metafile = metafile_alloc_04(filepath);
    metafile->npartitions = npartitions;
    metafile->nvariants = bgen_file_nvariants(bgen_file);

    if (!(metafile->stream = fopen(filepath, "w+b"))) {
        bgen_perror("could not create file %s", filepath);
        goto err;
    }

    if (write_metafile_header_04(metafile->stream, metafile->nvariants, metafile->npartitions))
        goto err;
    bgen_error("offset after header write: %lld", bgen_ftell(metafile->stream));

    /* seek to metadata block */
    if (bgen_fseek(metafile->stream, sizeof(uint64_t) * metafile->npartitions, SEEK_CUR))
        goto err;

    bgen_error("offset of metadata block: %lld", bgen_ftell(metafile->stream));

    bgen_error("Npartitions: %d\n", metafile->npartitions);
    metafile->partition_offset = malloc(sizeof(uint64_t) * metafile->npartitions);

    if (bgen_file_seek_variants_start(bgen_file))
        goto err;

    if (write_metafile_metadata_block_04(metafile->stream, metafile->partition_offset,
                                         metafile->npartitions, metafile->nvariants, bgen_file,
                                         verbose))
        goto err;

    if (bgen_fseek(metafile->stream, BGEN_METAFILE_04_HEADER_SIZE, SEEK_SET))
        goto err;

    if (write_metafile_offsets_block_04(metafile->stream, metafile->npartitions,
                                        metafile->partition_offset))
        goto err;

    if (fflush(metafile->stream)) {
        bgen_perror("could not fflush metafile");
        goto err;
    }

    return metafile;
err:
    bgen_metafile_close_04(metafile);
    return NULL;
}

struct bgen_metafile_04* bgen_metafile_open_04(char const* filepath)
{
    struct bgen_metafile_04* metafile = metafile_alloc_04(filepath);

    if (!(metafile->stream = fopen(metafile->filepath, "rb"))) {
        bgen_perror("could not open %s", metafile->filepath);
        goto err;
    }

    char header[13];
    if (fread(header, 13 * sizeof(char), 1, metafile->stream) != 1) {
        bgen_perror("could not fetch the metafile header");
        goto err;
    }

    if (strncmp(header, "bgen index 03", 13)) {
        bgen_error("unrecognized bgen index version: %.13s", header);
        goto err;
    }

    if (fread(&(metafile->nvariants), sizeof(uint32_t), 1, metafile->stream) != 1) {
        bgen_perror("could not read the number of variants from metafile");
        goto err;
    }

    /* if (fread(&(metafile->metadata_size), sizeof(uint64_t), 1, metafile->stream) != 1) { */
    /*     bgen_perror("could not read the metasize from metafile"); */
    /*     goto err; */
    /* } */

    /* if (metafile->metadata_size > INT64_MAX) { */
    /*     bgen_error("`metadata_size` overflow"); */
    /*     goto err; */
    /* } */

    /* if (bgen_fseek(metafile->stream, (int64_t)metafile->metadata_size, SEEK_CUR)) { */
    /*     bgen_error("could to fseek to the number of partitions"); */
    /*     goto err; */
    /* } */

    if (fread(&(metafile->npartitions), sizeof(uint32_t), 1, metafile->stream) != 1) {
        bgen_perror("could not read the number of partitions");
        goto err;
    }

    metafile->partition_offset = malloc(metafile->npartitions * sizeof(uint64_t));

    for (size_t i = 0; i < metafile->npartitions; ++i) {
        if (fread(metafile->partition_offset + i, sizeof(uint64_t), 1, metafile->stream) !=
            1) {
            bgen_perror("Could not read partition offsets");
            goto err;
        }
    }

    return metafile;
err:
    bgen_metafile_close_04(metafile);
    return NULL;
}

uint32_t bgen_metafile_npartitions_04(struct bgen_metafile_04 const* metafile)
{
    return metafile->npartitions;
}

uint32_t bgen_metafile_nvariants_04(struct bgen_metafile_04 const* metafile)
{
    return metafile->nvariants;
}

struct bgen_partition const* bgen_metafile_read_partition_04(
    struct bgen_metafile_04 const* metafile, uint32_t partition)
{
    FILE* file = metafile->stream;

    if (partition >= metafile->npartitions) {
        bgen_error("the provided partition number %" PRIu32 " is out-of-range", partition);
        return NULL;
    }

    uint32_t const nvariants =
        compute_nvariants_04(metafile->nvariants, metafile->npartitions, partition);

    struct bgen_partition* part = bgen_partition_create(nvariants);

    if (metafile->partition_offset[partition] > INT64_MAX) {
        bgen_error("`partition_offset` overflow");
        goto err;
    }

    if (bgen_fseek(file, (int64_t) metafile->partition_offset[partition], SEEK_SET)) {
        bgen_perror("could not fseek partition");
        goto err;
    }

    for (uint32_t i = 0; i < nvariants; ++i) {
        /* bgen_error("reading variant %ld", i); */
        struct bgen_variant* vm = bgen_variant_create();

        bgen_error("ponto 1");
        if (fread_ui64(file, &vm->genotype_offset, 8))
            goto err;

        bgen_error("ponto 2");
        if ((vm->id = bgen_string_fread(file, 2)) == NULL)
            goto err;

        /* bgen_error("id-length: %lld", bgen_string_length(vm->id)); */

        bgen_error("ponto 3");
        if ((vm->rsid = bgen_string_fread(file, 2)) == NULL)
            goto err;

        bgen_error("ponto 4");
        if ((vm->chrom = bgen_string_fread(file, 2)) == NULL)
            goto err;

        bgen_error("ponto 5");
        if (fread_ui32(file, &vm->position, 4))
            goto err;

        bgen_error("ponto 6");
        if (fread_ui16(file, &vm->nalleles, 2))
            goto err;

        bgen_variant_create_alleles(vm, vm->nalleles);

        for (uint16_t j = 0; j < vm->nalleles; ++j) {
            if ((vm->allele_ids[j] = bgen_string_fread(file, 4)) == NULL)
                goto err;
        }

        bgen_partition_set(part, i, vm);
    }

    return part;

err:
    bgen_partition_destroy(part);
    return NULL;
}

int bgen_metafile_close_04(struct bgen_metafile_04 const* metafile)
{
    free_c(metafile->filepath);
    if (metafile->stream && fclose(metafile->stream)) {
        bgen_perror("could not close %s", metafile->filepath);
        return 1;
    }
    free_c(metafile->partition_offset);
    free_c(metafile);
    return 0;
}

static struct bgen_metafile_04* metafile_alloc_04(char const* filepath)
{
    struct bgen_metafile_04* metafile = malloc(sizeof(struct bgen_metafile_04));
    metafile->filepath = strdup(filepath);
    metafile->stream = NULL;
    metafile->partition_offset = NULL;
    return metafile;
}

static uint32_t compute_nvariants_04(uint32_t nvariants, uint32_t npartitions,
                                     uint32_t partition)
{
    uint32_t size = ceildiv_uint32(nvariants, npartitions);
    return min_uint32(size, nvariants - size * partition);
}
