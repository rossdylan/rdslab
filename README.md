rdslab is an implementation of a simple slab allocator. Slabs are currently
defined as 2 pages of memory. These pages are then split into objects of a user
specified size. These objects look like this:

   object 1          object 2          object 3
 +----------+------+----------+------+----------+------+
 | obj_size | meta | obj_size | meta | obj_size | meta |
 +----------+------+----------+------+----------+------+

 The obj_size chunk contains the actual user specified memory
 the meta chunk contains a pointer to the slab this object is in. This pointer
 is used to figure out which slab a particular chunk of memory is in.  This is
 needed in order to simplify the user facing api. The user facing api for
 rdslab works like malloc/free in that you ask it for memory it gives you a
 void pointer, and when you are done you just pass that pointer back to free.

A slab keeps track of the objects which are free in a minheap. I'm not sure if
this was a good idea since initial profiling indicates a lot of time is taken
up in the heap implementation. I might change this to be a simple linked list.

slabs are used within a struct called rdcache_t. an rdcache_t has 3 linked
lists which represent the various states of slabs. When a slab is first
initialized it is inserted into the free_slabs list. After it has started to be
filled it is promoted to the partial_slabs list. Once it is totally filled it
is promoted once more to the full_slabs list. After some objects within the slab
are freed it is moved back to the partial_slabs list. The allocation algorithm
takes objects first from the partial_slabs list, and if it is empty it takes
a slab from the free_slabs list, allocates an object from it, and promotes it.

The 3 linked lists are built as follows:

   +------------+-----------+-----------+
   |            ^           ^           ^
   V            |           |           |
 rdlist_t -> rdslab_t -> rdslab_t -> rdslab_t
   |                                    ^
   |                                    |
   +------------------------------------+

The rdlist_t struct keeps track of the head and tail of the list, which are
just slab structs. Each slab has a pointer to the list it is in so when an
object is freed it knows which list to operate on.

The actual api has 3 calls and is defined in rdcache.h. rdcache_t is
essentially a memory pool that contains some number of slabs. This is currently
defined as 8 slabs, which totals 16 pages worth of memory.

To create a new rdcache_t you call rdcache_t *rdcache_new(uint64_t obj_size);

```c
    #include "rdcache.h"
    #include <stdio.h>

    int main(int argc, char *argv[]) {
        rdcache_t cache = rdcache_new(24);
        printf("Objects in Cache: %llu\n", cache->count);
        printf("Max number of Objects: %llu\n", cache->max);
        printf("Cache Object Size: %llu\n", cache->obj_size);
    }
```

To allocate some mmeory call void *rdcache_malloc(rdcache_t *cache);
To free memory call void rdcache_free(rdcache_t *cache, void *ptr);
NOTE: rdcache_free doesn't actually free any memory, it just marks it as
available for reuse.

```c
    #include "rdcache.h"
    #include <stdio.h>
    typedef struct {
        int foo;
        char * bar;
    } baz;

    int main(int argc, char *argv[]) {
        rdcache_t cache = rdcache_new(sizeof(baz));
        baz *the_baz = rdcache_malloc(cache);
        rdcache_free(cache, the_baz);
    }
```
