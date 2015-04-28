#include "rdcache.h"
#include <stdlib.h>
#include <stdio.h>

rdcache_t *rdcache_new(uint64_t obj_size) {
    rdcache_t *new_cache = NULL;
    if((new_cache = malloc(sizeof(rdcache_t))) == NULL) {
        perror("malloc");
        goto rdcache_new_free_cache;
    }
    if((new_cache->free_slabs = rdlist_new()) == NULL) {
        perror("rdlist_new");
        goto rdcache_new_free_free_slabs;
    }
    if((new_cache->partial_slabs = rdlist_new()) == NULL) {
        perror("rdlist_new");
        goto rdcache_new_free_partial_slabs;
    }
    if((new_cache->full_slabs = rdlist_new()) == NULL) {
        perror("rdlist_new");
        goto rdcache_new_free_full_slabs;
    }
    new_cache->obj_size = obj_size;
    new_cache->count = 0;
    rdslab_t *temp_slab = NULL; // temp holding of ptr before appending to list
    for(uint64_t i=0; i<RDCACHE_INITIAL_SLABS; i++) {
        if((temp_slab = rdslab_new(obj_size)) == NULL) {
            perror("rdslab_new");
            goto rdcache_new_free_full_slabs;
            //TODO handle slab cleanup on error
        }
        rdlist_append(new_cache->free_slabs, temp_slab);
        temp_slab->list_head = new_cache->free_slabs;
    }
    new_cache->max = RDCACHE_INITIAL_SLABS * new_cache->free_slabs->head->max;
    goto rdcache_new_return;

    // This is a series of goto labels for error handling and clean up
rdcache_new_free_full_slabs:
    free(new_cache->full_slabs);
rdcache_new_free_partial_slabs:
    free(new_cache->partial_slabs);
rdcache_new_free_free_slabs:
    free(new_cache->free_slabs);
rdcache_new_free_cache:
    free(new_cache);
    new_cache = NULL;
rdcache_new_return:
    return new_cache;
}

void *rdcache_malloc(rdcache_t *cache) {
    void *object = NULL;
    if(cache->partial_slabs->length > 0) {
        object = rdslab_next(cache->partial_slabs->tail);
        if(cache->partial_slabs->tail->count == cache->partial_slabs->tail->max) { // move to full list
            rdlist_append(cache->full_slabs, rdlist_pop(cache->partial_slabs));
            cache->full_slabs->tail->list_head = cache->full_slabs;
        }
        cache->count++;
    }
    else if(cache->free_slabs->length > 0) {
        object = rdslab_next(cache->free_slabs->tail);
        rdlist_append(cache->partial_slabs, rdlist_pop(cache->free_slabs)); // move to partial list
        cache->partial_slabs->tail->list_head = cache->partial_slabs;
        cache->count++;
    }
    else {
        // shit son we ran out of slabs...
        // eventually we should add more slabs rather then error
        return object = NULL;
    }
    return object;
}

static inline rdslab_obj_meta_t *rdcache_meta_for_object(rdcache_t *cache, void *ptr) {
    return (ptr + cache->obj_size);
}

void rdcache_free(rdcache_t *cache, void *ptr) {
    rdslab_t *slab = rdcache_meta_for_object(cache, ptr)->slab;
    rdslab_free(slab, ptr);
    if(slab->count == 0) { // move into free list
        rdlist_splice(slab);
        rdlist_append(cache->free_slabs, slab);
        slab->list_head = cache->free_slabs;
    }
    else if(slab->list_head == cache->full_slabs) {
        rdlist_splice(slab);
        rdlist_append(cache->partial_slabs, slab);
        slab->list_head = cache->partial_slabs;
    }
    cache->count--;
}
