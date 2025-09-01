/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/**
 * explicit list:
 *      4B             4B                 nB                  4B           4B
 * [imp_header,    exp_header,         payload          , exp_footer,   imp_footer]
 * [size 3B, 1B]   [dist 4B]                               [dist 4B]  [size 3B, 1B]
 */

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 16

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0xF)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1 << 12)

#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define PACK(size, alloc) ((size) | (alloc))

#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = val)

#define GET_SIZE(p) (GET(p) & ~0x7) // get payload size (header/footer not included)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define IMP_HDRP(bp) ((char *)(bp) - DSIZE)
#define EXP_HDRP(bp) ((char *)(bp) - WSIZE)
#define IMP_FTRP(bp) ((char *)(bp) + GET_SIZE(IMP_HDRP(bp)) + WSIZE)
#define EXP_FTRP(bp) ((char *)(bp) + GET_SIZE(IMP_HDRP(bp)))

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(IMP_HDRP(bp)) + 2*DSIZE)
#define NEXT_FREE_BLKP(bp) ((char *)(bp) + GET_SIZE(EXP_HDRP(bp)))
#define PREV_BLKP(bp) ((char *)(bp) - 2*DSIZE - (GET_SIZE((char *)(bp) - 3*WSIZE)))
#define PREV_FREE_BLKP(bp) ((char *)(bp) - GET_SIZE(EXP_FTRP(bp)))

static char *heap_listp;
static char *last_free;
static char *first_free;
static char *epilogue;

/* Coalesce adjacent free blocks */
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(IMP_HDRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(IMP_HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(IMP_HDRP(bp));

    if (prev_alloc && next_alloc) {
        return bp;
    } else if (prev_alloc && !next_alloc) {
        size += GET_SIZE(IMP_HDRP(NEXT_BLKP(bp))) + 4*WSIZE;
        PUT(IMP_HDRP(bp), PACK(size, 0));
        PUT(IMP_FTRP(NEXT_BLKP(bp)), PACK(size, 0));
    } else if (!prev_alloc && next_alloc) {
        size += GET_SIZE(IMP_HDRP(PREV_BLKP(bp))) + 4*WSIZE;
        PUT(IMP_FTRP(bp), PACK(size, 0));
        PUT(IMP_HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    } else {
        size += GET_SIZE(IMP_HDRP(PREV_BLKP(bp))) + GET_SIZE(IMP_HDRP(NEXT_BLKP(bp))) + 8*WSIZE;
        PUT(IMP_HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(IMP_FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}

/* Find first-fit free block */
static void *find_fit(size_t asize)
{
    void *bp;
    for (bp=first_free; bp != last_free; bp=NEXT_FREE_BLKP(bp)) {
        // printf("bp:%p\n", bp);
        if (asize<=GET_SIZE(IMP_HDRP(bp)))
            return bp;
    }
    return NULL;
}

/* Place block of asize bytes at start of free block bp */
static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(IMP_HDRP(bp));
    printf("place: %lld %lld\n", asize, csize);
    if ((csize-asize)>=4*DSIZE) {
        PUT(IMP_HDRP(bp), PACK(asize, 1));
        PUT(IMP_FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(IMP_HDRP(bp), PACK(csize-asize-2*DSIZE, 0));
        PUT(IMP_FTRP(bp), PACK(csize-asize-2*DSIZE, 0));
        bp = PREV_BLKP(bp);
        if (bp == first_free && bp == last_free) {
            first_free = last_free = NEXT_BLKP(bp);
        }
        if (bp != last_free) {
            unsigned int free_dist = GET(EXP_HDRP(bp));
            PUT(EXP_FTRP(NEXT_FREE_BLKP(bp)), free_dist - asize - 2*DSIZE);
            if (bp == first_free) first_free = NEXT_BLKP(bp);
        }
        if (bp != first_free) {
            unsigned int free_dist = GET(EXP_FTRP(bp));
            PUT(EXP_HDRP(PREV_FREE_BLKP(bp)), free_dist + asize + 2*DSIZE);
            if (bp == last_free) last_free = NEXT_BLKP(bp);
        }
    } else {
        PUT(IMP_HDRP(bp), PACK(csize, 1));
        PUT(IMP_FTRP(bp), PACK(csize, 1));
        if (bp == first_free) {
            first_free = NEXT_FREE_BLKP(bp);
            PUT(EXP_FTRP(first_free), 0);
        } else if (bp == last_free) {
            last_free = PREV_FREE_BLKP(bp);
            PUT(EXP_HDRP(last_free), 0);
        } else {
            PUT(EXP_HDRP(PREV_FREE_BLKP(bp)), NEXT_FREE_BLKP(bp) - PREV_FREE_BLKP(bp));
            PUT(EXP_FTRP(NEXT_FREE_BLKP(bp)), NEXT_FREE_BLKP(bp) - PREV_FREE_BLKP(bp));
        }
    }
}

static void *extend_heap(size_t words)
{
    char *bp;
    size_t size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((void *)(bp = mem_sbrk(size)) == (void *)-1)
        return NULL;

    size_t payload_size = size - 4*WSIZE;
    PUT(IMP_HDRP(bp), PACK(payload_size, 0)); // old brk became imp header of new block
    PUT(EXP_HDRP(bp), size); 

    PUT(EXP_FTRP(bp), bp-last_free);
    PUT(IMP_FTRP(bp), PACK(payload_size, 0));

    last_free = bp;
    epilogue = NEXT_BLKP(last_free);

    return coalesce(bp);
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(3*DSIZE))== (void *)-1)
        return -1;

    // PUT(heap_listp, 0);
    PUT(heap_listp, PACK(0, 1)); // prologue implicit header
    PUT(heap_listp + 1*WSIZE, 2*DSIZE); // prologue explicit header
    PUT(heap_listp + 2*WSIZE, 0); // prologue explicit footer
    PUT(heap_listp + 3*WSIZE, PACK(0, 1)); // prologue implicit footer

    PUT(heap_listp + 4*WSIZE, PACK(0, 1)); // epilogue implicit header
    PUT(heap_listp + 5*WSIZE, 0); // epilogue explicit header
    heap_listp += 2*WSIZE; // reach the start of payload
    epilogue = last_free;
    last_free = heap_listp + 4*WSIZE;
    first_free = last_free;

    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
    // print_heap();
    return 0;
}


/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    char *bp;
    size_t extend_size;

    if (size == 0) {
        return NULL;
    }

    if (size <= DSIZE) {
        asize = 2 * DSIZE;
    } else {
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
    }

    if ((bp=find_fit(asize)) != NULL) {
        // printf("THIS IS FIND FIT\n");
        // print_heap();
        place(bp, asize);
        return bp;
    }

    extend_size = MAX(asize, CHUNKSIZE);
    if ((bp=extend_heap(extend_size/WSIZE)) == NULL) {
        return NULL;
    }
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    // if (ptr == NULL) return;
    // size_t size = GET_SIZE(HDRP(ptr));
    // PUT(HDRP(ptr), PACK(size, 0));
    // PUT(FTRP(ptr), PACK(size, 0));
    // coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    // void *oldptr = ptr;
    // void *newptr;
    // size_t copySize;
    
    // newptr = mm_malloc(size);
    // if (newptr == NULL)
    //   return NULL;
    // copySize = GET_SIZE(HDRP(ptr))-DSIZE;
    // if (size < copySize)
    //   copySize = size;
    // memcpy(newptr, oldptr, copySize);
    // mm_free(oldptr);
    // return newptr;
    return NULL;
}


/**
 * print heap
 * header size, alloc per block
 * footer size, alloc
 * ...
 * epilogue header
 */
void print_heap()
{
    char *bp = heap_listp;
    printf("heap list: %p, first free: %p, last free: %p\n", heap_listp, first_free, last_free);
    while (1)
    {
        short end = 0;
        if (bp == epilogue) end = 1;
        size_t hd_size = GET_SIZE(IMP_HDRP(bp));
        size_t hd_alloc = GET_ALLOC(IMP_HDRP(bp)); 
        size_t exp_hd = GET(EXP_HDRP(bp));

        size_t exp_ft = GET(EXP_FTRP(bp));
        size_t ft_size = GET_SIZE(IMP_FTRP(bp));
        size_t ft_alloc = GET_ALLOC(IMP_FTRP(bp)); 

        printf("Block @ %p  header size=%zu  header alloc=%zu  explicit header=%zu \
            explicit footer=%zu footer size=%zu  footer alloc=%zu\n", (void *)bp, hd_size, hd_alloc, exp_hd, exp_ft, ft_size, ft_alloc);
        if (end) break;
        bp = NEXT_BLKP(bp);
    }
    printf("[Epilogue]\n");
}












