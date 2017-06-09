#include "bgen_reader.h"
#include "file.h"
#include "util.h"
#include "list.h"
#include "layout1.h"
#include "layout2.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

// Variant identifying data
//
// ---------------------------------------------
// | 4     | # samples (layout > 1)            |
// | 2     | # variant id length, Lid          |
// | Lid   | variant id                        |
// | 2     | variant rsid length, Lrsid        |
// | Lrsid | rsid                              |
// | 2     | chrom length, Lchr                |
// | Lchr  | chromossome                       |
// | 4     | variant position                  |
// | 2     | number of alleles, K (layout > 1) |
// | 4     | first allele length, La1          |
// | La1   | first allele                      |
// | 4     | second allele length, La2         |
// | La2   | second allele                     |
// | ...   |                                   |
// | 4     | last allele length, LaK           |
// | LaK   | last allele                       |
// ---------------------------------------------
int64_t bgen_reader_variant_block(BGenFile *bgenfile, uint64_t idx,
                                  VariantBlock *vb)
{
    if (bgen_fopen(bgenfile) == EXIT_FAILURE) return EXIT_FAILURE;


    if (idx >= bgen_reader_nvariants(bgenfile)) return EXIT_FAILURE;

    fseek(bgenfile->file, bgenfile->variants_start, SEEK_SET);

    assert(bgen_reader_layout(bgenfile) != 0);

    if (bgen_reader_layout(bgenfile) == 1)
    {
        if (fread_check(bgenfile, &(vb->nsamples), 4)) return EXIT_FAILURE;
    }

    if (fread_check(bgenfile, &(vb->id_length), 2)) return EXIT_FAILURE;

    vb->id = malloc(vb->id_length);

    if (fread_check(bgenfile, vb->id, vb->id_length)) return EXIT_FAILURE;

    if (fread_check(bgenfile, &(vb->rsid_length), 2)) return EXIT_FAILURE;

    vb->rsid = malloc(vb->rsid_length);

    if (fread_check(bgenfile, vb->rsid, vb->rsid_length)) return EXIT_FAILURE;

    if (fread_check(bgenfile, &(vb->chrom_length), 2)) return EXIT_FAILURE;

    vb->chrom = malloc(vb->chrom_length);

    if (fread_check(bgenfile, vb->chrom, vb->chrom_length)) return EXIT_FAILURE;

    if (fread_check(bgenfile, &(vb->position), 4)) return EXIT_FAILURE;

    if (bgen_reader_layout(bgenfile) == 1) vb->nalleles = 2;
    else if (fread_check(bgenfile, &(vb->nalleles), 2)) return EXIT_FAILURE;

    size_t i;
    vb->alleles = malloc(vb->nalleles * sizeof(Allele));

    for (i = 0; i < vb->nalleles; ++i)
    {
        if (fread_check(bgenfile, &(vb->alleles[i].length), 4)) return EXIT_FAILURE;

        vb->alleles[i].id = malloc(vb->alleles[i].length);

        if (fread_check(bgenfile, vb->alleles[i].id, vb->alleles[i].length)) return EXIT_FAILURE;
    }

    vb->genotype_start = ftell(bgenfile->file);

    if (bgen_fclose(bgenfile) == EXIT_FAILURE) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

// int64_t read_phased_probabilities(const BYTE *chunk, uint8_t nbits,
//                                    uint8_t max_ploidy, BYTE *ploidy_miss,
//                                    uint32_t nsamples, struct node **root)
// {
//     BYTE   *G;
//     size_t  i, j;
//     uint8_t ploidy;
//     uint8_t miss;
//
//     for (i = 0; i < max_ploidy; ++i)
//     {
//         for (j = 0; j < nsamples; ++j)
//         {
//             ploidy = bgen_read_ploidy(ploidy_miss[j]);
//             miss   = bgen_read_missingness(ploidy_miss[j]);
//         }
//     }
//     return EXIT_SUCCESS;
// }

// Interface functions

int64_t bgen_reader_genotype_block(BGenFile *bgenfile, uint64_t idx,
                                   VariantBlock *vb)
{
    bgen_reader_variant_block(bgenfile, idx, vb);

    if (bgen_fopen(bgenfile) == EXIT_FAILURE) return EXIT_FAILURE;

    fseek(bgenfile->file, vb->genotype_start, SEEK_SET);

    int64_t layout = bgen_reader_layout(bgenfile);

    if (layout == 1)
    {
        bgen_genotype_block_layout1(bgenfile, bgen_reader_compression(bgenfile),
                                bgen_reader_nsamples(bgenfile),
                                vb);
    } else if (layout == 2) {
        bgen_genotype_block_layout2(bgenfile, bgen_reader_compression(bgenfile),
                                bgen_reader_nsamples(bgenfile),
                                vb);
    }

    if (bgen_fclose(bgenfile) == EXIT_FAILURE) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int64_t bgen_reader_variantid(BGenFile *bgenfile, uint64_t idx, BYTE **id,
                              uint64_t *length)
{
    if (bgen_fopen(bgenfile) == EXIT_FAILURE) return EXIT_FAILURE;

    if (idx >= bgen_reader_nvariants(bgenfile)) return EXIT_FAILURE;

    VariantBlock vb;

    bgen_reader_variant_block(bgenfile, idx, &vb);

    *length = vb.id_length;
    *id     = vb.id;

    if (bgen_fclose(bgenfile) == EXIT_FAILURE) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int64_t bgen_reader_variant_rsid(BGenFile *bgenfile, uint64_t idx, BYTE **rsid,
                                 uint64_t *length)
{
    if (bgen_fopen(bgenfile) == EXIT_FAILURE) return EXIT_FAILURE;

    if (idx >= bgen_reader_nvariants(bgenfile)) return EXIT_FAILURE;

    VariantBlock vb;

    bgen_reader_variant_block(bgenfile, idx, &vb);

    *length = vb.rsid_length;
    *rsid   = vb.rsid;

    if (bgen_fclose(bgenfile) == EXIT_FAILURE) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int64_t bgen_reader_variant_chrom(BGenFile *bgenfile, uint64_t idx, BYTE **chrom,
                                  uint64_t *length)
{
    if (bgen_fopen(bgenfile) == EXIT_FAILURE) return EXIT_FAILURE;

    if (idx >= bgen_reader_nvariants(bgenfile)) return EXIT_FAILURE;

    VariantBlock vb;

    bgen_reader_variant_block(bgenfile, idx, &vb);

    *length = vb.chrom_length;
    *chrom  = vb.chrom;

    if (bgen_fclose(bgenfile) == EXIT_FAILURE) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int64_t bgen_reader_variant_position(BGenFile *bgenfile,
                                     uint64_t idx, uint64_t *position)
{
    if (bgen_fopen(bgenfile) == EXIT_FAILURE) return EXIT_FAILURE;

    if (idx >= bgen_reader_nvariants(bgenfile)) return EXIT_FAILURE;

    VariantBlock vb;

    bgen_reader_variant_block(bgenfile, idx, &vb);

    if (bgen_fclose(bgenfile) == EXIT_FAILURE) return EXIT_FAILURE;

    *position = vb.position;

    return EXIT_SUCCESS;
}

int64_t bgen_reader_variant_nalleles(BGenFile *bgenfile,
                                     uint64_t idx, uint64_t *nalleles)
{
    if (bgen_fopen(bgenfile) == EXIT_FAILURE) return EXIT_FAILURE;

    if (idx >= bgen_reader_nvariants(bgenfile)) return EXIT_FAILURE;

    VariantBlock vb;

    bgen_reader_variant_block(bgenfile, idx, &vb);

    if (bgen_fclose(bgenfile) == EXIT_FAILURE) return EXIT_FAILURE;

    *nalleles = vb.nalleles;

    return EXIT_SUCCESS;
}

int64_t bgen_reader_variant_alleleid(BGenFile *bgenfile, uint64_t idx0,
                                     uint64_t idx1, BYTE **id,
                                     uint64_t *length)
{
    if (bgen_fopen(bgenfile) == EXIT_FAILURE) return EXIT_FAILURE;

    if (idx0 >= bgen_reader_nvariants(bgenfile)) return -1;

    uint64_t nalleles;

    bgen_reader_variant_nalleles(bgenfile, idx0, &nalleles);

    if (idx1 >= nalleles) return EXIT_FAILURE;

    VariantBlock vb;

    bgen_reader_variant_block(bgenfile, idx0, &vb);

    *id = vb.alleles[idx1].id;

    if (bgen_fclose(bgenfile) == EXIT_FAILURE) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
