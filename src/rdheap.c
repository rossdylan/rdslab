#include "rdheap.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



/**
 * Internal function definitions used to do all the behind the scense
 * heap magic
 */
static void _rdheap_swap(rdheap_t *heap, uint64_t a, uint64_t b);
static void _rdheap_sink(rdheap_t *heap, uint64_t index);
static void _rdheap_rise(rdheap_t *heap, uint64_t index);
static void _rdheap_rise(rdheap_t *heap, uint64_t index);
static void _rdheap_heapify(rdheap_t *heap, uint64_t i);
static void _rdheap_build(rdheap_t *heap);


/**
 * Public functions are beyond this point
 */


/**
 * Print out the backing array of the heap
 */
void rdheap_print(rdheap_t *heap) {
    for(uint64_t i=0; i<heap->size; i++) {
        if(heap->array[i] == heap->size+1) {
            printf("_ ");
        }
        else {
            printf("%llu ", heap->array[i]);
        }
    }
    printf("\n");
}


/**
 * private function used to swap two elements in the heap
 */
static void _rdheap_swap(rdheap_t *heap, uint64_t a, uint64_t b) {
    uint64_t temp = heap->array[a];
    heap->array[a] = heap->array[b];
    heap->array[b] = temp;
}

/**
 * private function used to sink an element downwards until it finds its
 * position in the heap.
 */
static void _rdheap_sink(rdheap_t *heap, uint64_t index) {
    while(2*index+1 < heap->length) {
        uint64_t j = (2*index)+1;
        if(heap->array[2*index+1] < heap->array[index] && heap->array[2*index+1] != heap->size+1) {
            _rdheap_swap(heap, index, 2*index+1);
            index = 2*index+1;
        }
        else if(heap->array[2*index+2] < heap->array[index] && heap->array[2*index+1] != heap->size+1) {
            _rdheap_swap(heap, index, 2*index+2);
            index = 2*index+2;
        }
        else {
            break;
        }
    }
}

/**
 * Private function to rise an element upwards until it finds its position in
 * the heap.
 */
static void _rdheap_rise(rdheap_t *heap, uint64_t index) {
    uint64_t cur = index;
    while(cur > 0) {
        uint64_t parent = (index - 1) / 2;
        if(heap->array[cur] < heap->array[parent]) {
            _rdheap_swap(heap, cur, parent);
            cur = parent;
            continue;
        }
        break;
    }
}

/**
 * Private function that Heapifies the backing array of a heap from the given
 * index.
 */
static void _rdheap_heapify(rdheap_t *heap, uint64_t i) {
    uint64_t left = 0;
    uint64_t right = 0;
    uint64_t smallest = 0;
    while(i > 0) {
        left = (i*2) + 1;
        right = (i*2) + 2;
        smallest = i;
        if(left < heap->length && heap->array[left] != heap->size+1) { // left child
            if(heap->array[left] < heap->array[i]) {
                smallest = left;
            }
        }
        if(right < heap->length && heap->array[right] != heap->size+1) { // right child
            if(heap->array[right] < heap->array[i] && heap->array[right] < heap->array[smallest]) {
                smallest = right;
            }
        }
        if(smallest != i) {
            _rdheap_swap(heap, smallest, i);
        }
        else {
            break;
        }
    }
}

/**
 * Private function that builds a minheap
 */
static void _rdheap_build(rdheap_t *heap) {
    for(uint64_t i=heap->length/2; i>0; i--) {
        _rdheap_heapify(heap, i);
    }
    _rdheap_sink(heap, 0);
}

/**
 * Public function that inserts a value into the heap
 * returns the new length of the heap, or -1 if insertion would overflow.
 */
int rdheap_insert(rdheap_t *heap, uint64_t value) {
    if(heap->length != heap->size) {
        heap->array[heap->length] = value;
        _rdheap_rise(heap, heap->length);
        heap->length++;
        _rdheap_sink(heap, 0);
        return heap->length;
    }
    return -1;
}

/**
 * Pops the minimum value from the root of the heap.
 */
uint64_t rdheap_pop(rdheap_t *heap) {
    uint64_t max = heap->array[0];
    uint64_t end = heap->length-1;
    _rdheap_swap(heap, 0, end);
    heap->array[end] = heap->size+1; //null the last element
    heap->length--;
    _rdheap_sink(heap, 0);
    return max;
}

/**
 * Create a new empty heap of the given size
 */
rdheap_t *rdheap_new(const uint64_t size) {
    rdheap_t *new = NULL;
    if((new = malloc(sizeof(rdheap_t))) == NULL) {
        perror("malloc");
        return NULL;
    }
    if((new->array = calloc(size, sizeof(uint64_t))) == NULL) {
        perror("calloc");
        free(new);
        return NULL;
    }
    for(uint64_t i=0; i<size; i++) {
        new->array[i] = size+1;
    }
    new->length = 0;
    new->size = size;
    return new;
}

/**
 * Create a new heap from the given array and array size
 */
rdheap_t *rdheap_from_array(uint64_t array[], const uint64_t size) {
    rdheap_t *new = rdheap_new(size);
    memcpy(new->array, array, sizeof(uint64_t)*size);
    new->length = size;
    _rdheap_build(new);
    return new;
}

int main(int argc, char *argv[]) {
    uint64_t derp[10] = {4,3,2,1,0,8,5,6,7,9};
    rdheap_t *boop = rdheap_new(10);
    rdheap_print(boop);
    for(uint64_t i=0; i<10; i++) {
        rdheap_insert(boop, derp[i]);
        printf("[%llu] length=%llu: ", i, boop->length);
        rdheap_print(boop);
    }
    rdheap_pop(boop);
    rdheap_print(boop);

}
