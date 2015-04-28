#ifndef _RDCACHE_H
#define _RDCACHE_H

#include "stdint.h"
#include "rdslab.h"

// how many slabs to make initially
#define RDCACHE_INITIAL_SLABS 8

/**
 * rdcache has 3 lists which are rdlist_t structs which act as heads to a
 * linkedlist of rdslab_t structs. Each rdslab_t has a reference to the head of
 * its list so it knows which list its in.
 *   +------------+-----------+-----------+
 *   |            ^           ^           ^
 *   V            |           |           |
 * rdlist_t -> rdslab_t -> rdslab_t -> rdslab_t
 *   |                                    ^
 *   |                                    |
 *   +------------------------------------+
 */

typedef struct {
    rdlist_t *free_slabs; // empty slabs
    rdlist_t *partial_slabs; // slabs currently being filled
    rdlist_t *full_slabs; // slabs that are full.
    uint64_t obj_size; // the size of the objects we are storing
    uint64_t count; // the total number of ojects we have.
    uint64_t max; // max number of objects we can allocate acrossed all slabs
} rdcache_t;

rdcache_t *rdcache_new(uint64_t obj_size);
void *rdcache_malloc(rdcache_t * cache);
void rdcache_free(rdcache_t *cache, void *ptr);
void rdcache_destroy(rdcache_t *cache);
#endif
