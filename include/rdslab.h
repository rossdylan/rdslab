#ifndef _RDSLAB_H
#define _RDSLAB_H

#include <stdint.h>
#include <stdbool.h>
#include "rdheap.h"


// define the size of a slab in pages
#define RDSLAB_PAGES 2

struct _rdslab_t;

typedef struct {
    uint64_t length;
    struct _rdslab_t *head;
    struct _rdslab_t *tail;
} rdlist_t;

struct _rdslab_t {
    rdlist_t *list_head; // the head of the list we are in
    struct _rdslab_t *next;
    struct _rdslab_t *prev;
    uint64_t obj_size; // size of objects in this slab
    uint64_t full_obj_size; // size of object + rdslab_obj_meta_t
    uint64_t max; // max number of objects
    uint64_t count; // number of objects in the slab
    rdheap_t *free;
    void *raw; // raw memory of the slab
};

typedef struct _rdslab_t rdslab_t;

/**
 * Meta object placed directly after user object data in the slab
 */
typedef struct {
    rdslab_t *slab;
} rdslab_obj_meta_t;

/**
 * Create a new rdlist, this just makes a new rdlist_t, and sets head/tail to
 * NULL.
 */
rdlist_t *rdlist_new(void);

/**
 * Append a slab to a list.
 */
void rdlist_append(rdlist_t *list, rdslab_t *slab);

/**
 * Remove the head of a list and return it
 */
rdslab_t *rdlist_pop(rdlist_t *list);

/**
 * Splice out a node in the list. This means we remove a node from the middle
 * of whatever list it is in. prev->next = elem->next
 */
void rdlist_splice(rdslab_t *elem);

/**
 * Create a new slab and initialize all its values to their defaults. This
 * function has a bit of complexity when it builds its free heap which contains
 * a list of all available indexes
 */
rdslab_t *rdslab_new(uint64_t obj_size);

/**
 * Free an object specified by the index. This just inserts the obj_num into
 * the free heap.
 */
void rdslab_free(rdslab_t *slab, void *ptr);

/**
 * Get the next free object from this slab by poping a value of the free heap
 */
void *rdslab_next(rdslab_t *slab);

/**
 * Check if this slab contains a given ptr
 * slab_start <= ptr < slab_start + (max * obj_size)
 */
bool rdslab_contains_ptr(rdslab_t *slab, void *ptr);
#endif
