#include "geno.h"
#include "bgen/bgen.h"
#include "index.h"
#include "file.h"
#include "layout/one.h"
#include "layout/two.h"
#include "mem.h"
#include <assert.h>

struct bgen_vg* create_vg()
{
    struct bgen_vg* vg = dalloc(sizeof(struct bgen_vg));
    vg->plo_miss = NULL;
    vg->chunk = NULL;
    vg->current_chunk = NULL;
    return vg;
}

int free_vg(struct bgen_vg* vg)
{
    if (vg) {
        vg->plo_miss = free_nul(vg->plo_miss);
        vg->current_chunk = vg->chunk = free_nul(vg->chunk);
    }
    return free_nul(vg) != NULL;
}

struct bgen_vg* bgen_open_genotype(struct bgen_file* bgen, long vaddr)
{
    struct bgen_vg* vg = create_vg();
    if (!vg) {
        error("Could not open genotype");
        goto err;
    }
    vg->vaddr = (OFF_T)vaddr;

    if (LONG_SEEK(bgen->file, vaddr, SEEK_SET)) {
        perror_fmt(bgen->file, "Could not seek a variant in %s", bgen->filepath);
        goto err;
    }

    struct bgen_vi vi;
    init_index(bgen, &vi);

    if (bgen->layout == 1) {
        bgen_read_probs_header_one(&vi, vg, bgen->file);
    } else if (bgen->layout == 2) {
        read_probs_header_two(&vi, vg, bgen->file);
    } else {
        error("Unrecognized layout type %d", bgen->layout);
        goto err;
    }

    free_nul(vi.filepath);
    free_nul(vi.start);
    return vg;

err:
    free_nul(vi.filepath);
    free_nul(vi.start);
    return free_nul(vg);
}

void bgen_close_genotype(struct bgen_vg* vg) { free_vg(vg); }

int bgen_read_genotype(const struct bgen_file* bgen, struct bgen_vg* vg, double* probs)
{
    if (bgen->layout == 1) {
        bgen_read_probs_one(vg, probs);
    } else if (bgen->layout == 2) {
        read_probs_two(vg, probs);
    } else {
        error("Unrecognized layout type %d", bgen->layout);
        return 1;
    }
    return 0;
}

int bgen_nalleles(const struct bgen_vg* vg) { return vg->nalleles; }

int bgen_missing(const struct bgen_vg* vg, int index) { return vg->plo_miss[index] >> 7; }

int bgen_ploidy(const struct bgen_vg* vg, int index) { return vg->plo_miss[index] & 127; }

int bgen_min_ploidy(const struct bgen_vg* vg) { return vg->min_ploidy; }

int bgen_max_ploidy(const struct bgen_vg* vg) { return vg->max_ploidy; }

int bgen_ncombs(const struct bgen_vg* vg) { return vg->ncombs; }

int bgen_phased(const struct bgen_vg* vg) { return vg->phased; }
