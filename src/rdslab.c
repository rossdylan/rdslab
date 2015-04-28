#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "rdheap.h"
#include "rdslab.h"

/**
 * Turn an object index and a slab into a pointer into the slab.
 */
static inline void *rdslab_get_object(rdslab_t *slab, uint64_t obj_num);

/**
 * Turn a pointer into its object index. This is just doing some pointer math
 * to figure out its offset into the slab.
 */
static uint64_t rdslab_object_for_ptr(rdslab_t *slab, void *ptr);

/**
 * Object layout.
 *  +----------+------+
 *  | obj_size | meta |
 *  +----------+------+
 *  The obj_size chunk contains the actual user specified memory
 *  the meta chunk contains a pointer to the slab this object is in.
 *  This way when a rdcache_t gets a rdcache_free call it can use ptr math to
 *  figure out which slab we need to push the free call to.
 */


/**
 * Allocate a new slab
 */
rdslab_t *rdslab_new(uint64_t obj_size) {
    rdslab_t *new_slab = NULL;
    if((new_slab = malloc(sizeof(rdslab_t))) == NULL) {
        perror("malloc");
        return NULL;
    }
    uint64_t slab_size =  RDSLAB_PAGES * sysconf(_SC_PAGE_SIZE);
    if((new_slab->raw = malloc(slab_size)) == NULL) {
        perror("malloc");
        free(new_slab);
        return NULL;
    }
    new_slab->obj_size = obj_size;
    new_slab->full_obj_size = obj_size + sizeof(rdslab_obj_meta_t);
    new_slab->max = slab_size / new_slab->full_obj_size;
    new_slab->count = 0;
    new_slab->list_head = NULL;
    new_slab->next = NULL;
    new_slab->prev = NULL;

    // This next bit populates our free heap
    uint64_t *free_objects = NULL;
    if((free_objects = calloc(new_slab->max, sizeof(uint64_t))) == NULL) {
        perror("calloc");
        free(new_slab->raw);
        free(new_slab);
        return NULL;
    }
    for(uint64_t i=0; i<new_slab->max; i++) {
        free_objects[i] = i;
    }
    new_slab->free = rdheap_from_array(free_objects, new_slab->max);
    free(free_objects);
    return new_slab;
}

/**
 * Get the ptr to a portion of the slab that represents the object described
 * by the given index and size
 */
static inline void *rdslab_get_object(rdslab_t *slab, uint64_t obj_num) {
    return slab->raw + (obj_num * slab->full_obj_size);
}

static uint64_t rdslab_object_for_ptr(rdslab_t *slab, void *ptr) {
    if(rdslab_contains_ptr(slab, ptr)) {
        return (ptr - slab->raw) / slab->full_obj_size;
    }
    return 0;
}

/** Free the given pointer if it is a part of this slab.
 * We check if its in our slab, and if it is we calculate the object number
 * and then add that number to our free heap.
 *
 */
void rdslab_free(rdslab_t *slab, void *ptr) {
    uint64_t obj = rdslab_object_for_ptr(slab, ptr);
    rdheap_insert(slab->free, obj);
    slab->count--;
}

/**
 * Get the next free object in the slab. We also ensure that our meta oject
 * is in place.
 */
void *rdslab_next(rdslab_t *slab) {
    uint64_t free_obj_index = rdheap_pop(slab->free);
    void *free_obj = rdslab_get_object(slab, free_obj_index);
    rdslab_obj_meta_t *meta = free_obj + slab->obj_size;
    meta->slab = slab;
    slab->count++;
    return free_obj;
}

/**
 * Check if a ptr is in this slab
 */
bool rdslab_contains_ptr(rdslab_t *slab, void *ptr) {
    return ptr >= slab->raw && ptr < slab->raw + (slab->max * slab->full_obj_size);
}

rdlist_t *rdlist_new(void) {
    rdlist_t *new = NULL;
    if((new = malloc(sizeof(rdlist_t))) == NULL) {
        perror("malloc");
        return NULL;
    }
    new->head = NULL;
    new->tail = NULL;
    new->length = 0;
    return new;
}

void rdlist_append(rdlist_t *list, rdslab_t *slab) {
    slab->list_head = list;
    if(list->head == NULL) {
        list->head = slab;
        list->tail = slab;
        slab->prev = NULL;
        slab->next = NULL;
    }
    else {
        list->tail->next = slab;
        slab->prev = list->tail;
        list->tail = slab;
        list->tail->next = NULL;
    }
    list->length++;
}

rdslab_t *rdlist_pop(rdlist_t *list) {
    rdslab_t *popped_tail = list->tail;
    if(list->tail->prev == NULL) { //this was the first node
        list->tail = NULL;
        list->head = NULL;
    }
    else {
        list->tail = popped_tail->prev;
        list->tail->next = NULL;
    }
    list->length--;
    return popped_tail;
}

void rdlist_splice(rdslab_t *elem) {
    // splice the only element in a list
    if(elem->next == NULL && elem->prev == NULL) {
        elem->list_head->tail = NULL;
        elem->list_head->head = NULL;
    }
    // splice the last element of the list
    else if(elem->next == NULL) {
        elem->prev->next = NULL;
        elem->list_head->tail = elem->prev;
    }
    // splice the first element of the list
    else if(elem->prev == NULL) {
        elem->list_head->head = elem->next;
        elem->prev = NULL;
    }
    // We are actually removing a node in the middle of the list
    else {
        elem->prev->next = elem->next;
        elem->next->prev = elem->prev;
    }
    elem->list_head->length--;
}
