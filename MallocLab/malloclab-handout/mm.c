/*-------------------------------------------------------------------
 * maxmir - Max Miroff - mamiroff@email.wm.edu
 * Malloc lab Starter code:
 *        single doubly-linked free block list with LIFO policy
 *        with support for coalescing adjacent free blocks
 *
 * Terminology:
 * o We will implement an explicit free list allocator
 * o We use "next" and "previous" to refer to blocks as ordered in
 *   the free list.
 * o We use "following" and "preceding" to refer to adjacent blocks
 *   in memory.
 *-------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "memlib.h"
#include "mm.h"

team_t team={
    "maxmir team",                                        /* team name */
    "Max Miroff",                                        /* first member full name */
    "maxmir",                                        /* first member email address*/
    "",                                        /* second member full name (blank if none)*/
    ""                                         /* second member email address (blank if none) */
};
/* Macros for unscaled pointer arithmetic to keep other code cleaner.
   Casting to a char* has the effect that pointer arithmetic happens at
   the byte granularity (i.e. POINTER_ADD(0x1, 1) would be 0x2).  (By
   default, incrementing a pointer in C has the effect of incrementing
   it by the size of the type to which it points (e.g. BlockInfo).)
   We cast the result to void* to force you to cast back to the
   appropriate type and ensure you don't accidentally use the resulting
   pointer as a char* implicitly.  You are welcome to implement your
   own pointer arithmetic instead of using these macros.
*/
#define UNSCALED_POINTER_ADD(p,x) ((void*)((char*)(p) + (x)))
#define UNSCALED_POINTER_SUB(p,x) ((void*)((char*)(p) - (x)))


/******** FREE LIST IMPLEMENTATION ***********************************/


/* A BlockInfo contains information about a block, including the size
   and usage tags, as well as pointers to the next and previous blocks
   in the free list.  This is exactly the "explicit free list" structure
   illustrated in the lecture slides.

   Note that the next and prev pointers and the boundary tag are only
   needed when the block is free.  To achieve better utilization, mm_malloc
   should use the space for next and prev as part of the space it returns.

   +--------------+
   | sizeAndTags  |  <-  BlockInfo pointers in free list point here
   |  (header)    |
   +--------------+
   |     next     |  <-  Pointers returned by mm_malloc point here
   +--------------+
   |     prev     |
   +--------------+
   |  space and   |
   |   padding    |
   |     ...      |
   |     ...      |
   +--------------+
   | boundary tag |
   |  (footer)    |
   +--------------+
*/
struct BlockInfo {
  // Size of the block (in the high bits) and tags for whether the
  // block and its predecessor in memory are in use.  See the SIZE()
  // and TAG macros, below, for more details.
  size_t sizeAndTags;
  // Pointer to the next block in the free list.
  struct BlockInfo* next;
  // Pointer to the previous block in the free list.
  struct BlockInfo* prev;
};
typedef struct BlockInfo BlockInfo;


/* Pointer to the first BlockInfo in the free list, the list's head.

   A pointer to the head of the free list in this implementation is
   always stored in the first word in the heap.  mem_heap_lo() returns
   a pointer to the first word in the heap, so we cast the result of
   mem_heap_lo() to a BlockInfo** (a pointer to a pointer to
   BlockInfo) and dereference this to get a pointer to the first
   BlockInfo in the free list. */
#define FREE_LIST_HEAD *((BlockInfo **)mem_heap_lo())

/* Size of a word on this architecture. */
#define WORD_SIZE sizeof(void*)

/* Minimum block size (to account for size header, next ptr, prev ptr,
   and boundary tag) */
#define MIN_BLOCK_SIZE (sizeof(BlockInfo) + WORD_SIZE)

/* Alignment of blocks returned by mm_malloc. */
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* SIZE(blockInfo->sizeAndTags) extracts the size of a 'sizeAndTags' field.
   Also, calling SIZE(size) selects just the higher bits of 'size' to ensure
   that 'size' is properly aligned.  We align 'size' so we can use the low
   bits of the sizeAndTags field to tag a block as free/used, etc, like this:

      sizeAndTags:
      +-------------------------------------------+
      | 63 | 62 | 61 | 60 |  . . . .  | 2 | 1 | 0 |
      +-------------------------------------------+
        ^                                       ^
      high bit                               low bit

   Since ALIGNMENT == 8, we reserve the low 3 bits of sizeAndTags for tag
   bits, and we use bits 3-63 to store the size.

   Bit 0 (2^0 == 1): TAG_USED
   Bit 1 (2^1 == 2): TAG_PRECEDING_USED
*/
#define SIZE(x) ((x) & ~(ALIGNMENT - 1))

/* TAG_USED is the bit mask used in sizeAndTags to mark a block as used. */
#define TAG_USED 1

/* TAG_PRECEDING_USED is the bit mask used in sizeAndTags to indicate
   that the block preceding it in memory is used. (used in turn for
   coalescing).  If the previous block is not used, we can learn the size
   of the previous block from its boundary tag */
#define TAG_PRECEDING_USED 2


/* Find a free block of the requested size in the free list.  Returns
   NULL if no free block is large enough. */
static void * searchFreeList(size_t reqSize) {
  BlockInfo* freeBlock;

  freeBlock = FREE_LIST_HEAD;
  while (freeBlock != NULL){
    if (SIZE(freeBlock->sizeAndTags) >= reqSize) {
      return freeBlock;
    } else {
      freeBlock = freeBlock->next;
    }
  }
  return NULL;
}

/* Insert freeBlock at the head of the list.  (LIFO) */
static void insertFreeBlock(BlockInfo* freeBlock) {
  BlockInfo* oldHead = FREE_LIST_HEAD;
  freeBlock->next = oldHead;
  if (oldHead != NULL) {
    oldHead->prev = freeBlock;
  }
  //  freeBlock->prev = NULL;
  FREE_LIST_HEAD = freeBlock;
}

/* Remove a free block from the free list. */
static void removeFreeBlock(BlockInfo* freeBlock) {
  BlockInfo *nextFree, *prevFree;

  nextFree = freeBlock->next;
  prevFree = freeBlock->prev;

  // If the next block is not null, patch its prev pointer.
  if (nextFree != NULL) {
    nextFree->prev = prevFree;
  }

  // If we're removing the head of the free list, set the head to be
  // the next block, otherwise patch the previous block's next pointer.
  if (freeBlock == FREE_LIST_HEAD) {
    FREE_LIST_HEAD = nextFree;
  } else {
    prevFree->next = nextFree;
  }
}

/* Coalesce 'oldBlock' with any preceeding or following free blocks. */
static void coalesceFreeBlock(BlockInfo* oldBlock) {
  BlockInfo *blockCursor;
  BlockInfo *newBlock;
  BlockInfo *freeBlock;
  // size of old block
  size_t oldSize = SIZE(oldBlock->sizeAndTags);
  // running sum to be size of final coalesced block
  size_t newSize = oldSize;

  // Coalesce with any preceding free block
  blockCursor = oldBlock;
  while ((blockCursor->sizeAndTags & TAG_PRECEDING_USED)==0) {
    // While the block preceding this one in memory (not the
    // prev. block in the free list) is free:

    // Get the size of the previous block from its boundary tag.
    size_t size = SIZE(*((size_t*)UNSCALED_POINTER_SUB(blockCursor, WORD_SIZE)));
    // Use this size to find the block info for that block.
    freeBlock = (BlockInfo*)UNSCALED_POINTER_SUB(blockCursor, size);
    // Remove that block from free list.
    removeFreeBlock(freeBlock);

    // Count that block's size and update the current block pointer.
    newSize += size;
    blockCursor = freeBlock;
  }
  newBlock = blockCursor;

  // Coalesce with any following free block.
  // Start with the block following this one in memory
  blockCursor = (BlockInfo*)UNSCALED_POINTER_ADD(oldBlock, oldSize);
  while ((blockCursor->sizeAndTags & TAG_USED)==0) {
    // While the block is free:

    size_t size = SIZE(blockCursor->sizeAndTags);
    // Remove it from the free list.
    removeFreeBlock(blockCursor);
    // Count its size and step to the following block.
    newSize += size;
    blockCursor = (BlockInfo*)UNSCALED_POINTER_ADD(blockCursor, size);
  }

  // If the block actually grew, remove the old entry from the free
  // list and add the new entry.
  if (newSize != oldSize) {
    // Remove the original block from the free list
    removeFreeBlock(oldBlock);

    // Save the new size in the block info and in the boundary tag
    // and tag it to show the preceding block is used (otherwise, it
    // would have become part of this one!).
    newBlock->sizeAndTags = newSize | TAG_PRECEDING_USED;
    // The boundary tag of the preceding block is the word immediately
    // preceding block in memory where we left off advancing blockCursor.
    *(size_t*)UNSCALED_POINTER_SUB(blockCursor, WORD_SIZE) = newSize | TAG_PRECEDING_USED;

    // Put the new block in the free list.
    insertFreeBlock(newBlock);
  }
  return;
}

/* Get more heap space of size at least reqSize. */
static void requestMoreSpace(size_t reqSize) {
  size_t pagesize = mem_pagesize();
  size_t numPages = (reqSize + pagesize - 1) / pagesize;
  BlockInfo *newBlock;
  size_t totalSize = numPages * pagesize;
  size_t prevLastWordMask;

  void* mem_sbrk_result = mem_sbrk(totalSize);
  if ((size_t)mem_sbrk_result == -1) {
    printf("ERROR: mem_sbrk failed in requestMoreSpace\n");
    exit(0);
  }
  newBlock = (BlockInfo*)UNSCALED_POINTER_SUB(mem_sbrk_result, WORD_SIZE);

  /* initialize header, inherit TAG_PRECEDING_USED status from the
     previously useless last word however, reset the fake TAG_USED
     bit */
  prevLastWordMask = newBlock->sizeAndTags & TAG_PRECEDING_USED;
  newBlock->sizeAndTags = totalSize | prevLastWordMask;
  // Initialize boundary tag.
  ((BlockInfo*)UNSCALED_POINTER_ADD(newBlock, totalSize - WORD_SIZE))->sizeAndTags =
    totalSize | prevLastWordMask;

  /* initialize "new" useless last word
     the previous block is free at this moment
     but this word is useless, so its use bit is set
     This trick lets us do the "normal" check even at the end of
     the heap and avoid a special check to see if the following
     block is the end of the heap... */
  *((size_t*)UNSCALED_POINTER_ADD(newBlock, totalSize)) = TAG_USED;

  // Add the new block to the free list and immediately coalesce newly
  // allocated memory space
  insertFreeBlock(newBlock);
  coalesceFreeBlock(newBlock);
}


/* Initialize the allocator. */
int mm_init () {
  // Head of the free list.
  BlockInfo *firstFreeBlock;

  // Initial heap size: WORD_SIZE byte heap-header (stores pointer to head
  // of free list), MIN_BLOCK_SIZE bytes of space, WORD_SIZE byte heap-footer.
  size_t initSize = WORD_SIZE+MIN_BLOCK_SIZE+WORD_SIZE;
  size_t totalSize;

  void* mem_sbrk_result = mem_sbrk(initSize);
  //  printf("mem_sbrk returned %p\n", mem_sbrk_result);
  if ((ssize_t)mem_sbrk_result == -1) {
    printf("ERROR: mem_sbrk failed in mm_init, returning %p\n",
           mem_sbrk_result);
    exit(1);
  }

  firstFreeBlock = (BlockInfo*)UNSCALED_POINTER_ADD(mem_heap_lo(), WORD_SIZE);

  // Total usable size is full size minus heap-header and heap-footer words
  // NOTE: These are different than the "header" and "footer" of a block!
  // The heap-header is a pointer to the first free block in the free list.
  // The heap-footer is used to keep the data structures consistent (see
  // requestMoreSpace() for more info, but you should be able to ignore it).
  totalSize = initSize - WORD_SIZE - WORD_SIZE;

  // The heap starts with one free block, which we initialize now.
  firstFreeBlock->sizeAndTags = totalSize | TAG_PRECEDING_USED;
  firstFreeBlock->next = NULL;
  firstFreeBlock->prev = NULL;
  // boundary tag
  *((size_t*)UNSCALED_POINTER_ADD(firstFreeBlock, totalSize - WORD_SIZE)) = totalSize | TAG_PRECEDING_USED;

  // Tag "useless" word at end of heap as used.
  // This is the is the heap-footer.
  *((size_t*)UNSCALED_POINTER_SUB(mem_heap_hi(), WORD_SIZE - 1)) = TAG_USED;

  // set the head of the free list to this new free block.
  FREE_LIST_HEAD = firstFreeBlock;
  return 0;
}

/* Print the heap by iterating through it as an implicit free list */
/*
static void examine_heap() {
  BlockInfo *block;

  fprintf(stderr, "FREE_LIST_HEAD: %p\n", (void *)FREE_LIST_HEAD);

  for(block = (BlockInfo *)UNSCALED_POINTER_ADD(mem_heap_lo(), WORD_SIZE);
      SIZE(block->sizeAndTags) != 0 && block < mem_heap_hi();
      block = (BlockInfo *)UNSCALED_POINTER_ADD(block, SIZE(block->sizeAndTags))) {

    fprintf(stderr, "%p: %ld %ld %ld\t",
    (void *)block,
    SIZE(block->sizeAndTags),
    block->sizeAndTags & TAG_PRECEDING_USED,
    block->sizeAndTags & TAG_USED);

    if (block->sizeAndTags & TAG_USED) {
      fprintf(stderr, "ALLOCATED\n");
    } else {
      fprintf(stderr, "FREE\tnext: %p, prev: %p\n",
      (void *)block->next,
      (void *)block->prev);
    }
  }
  fprintf(stderr, "END OF HEAP\n\n");
}
*/

// TOP-LEVEL ALLOCATOR INTERFACE ------------------------------------


/* Allocate a block of size size and return a pointer to it. */
void* mm_malloc (size_t size) {
  size_t reqSize, blockSize, prevBlockUse;
  BlockInfo* freeBlock = NULL;
  BlockInfo* splitBlock = NULL;

//	printf("entering mm_malloc with size %d\n", size);
  // Zero-size requests get NULL.
  if (size == 0) {
    return NULL;
  }

  // Add one word for the initial size header.
  // Note that we don't need to boundary tag when the block is used!
  size += WORD_SIZE;
  if (size <= MIN_BLOCK_SIZE) {
    // Make sure we allocate enough space for a blockInfo in case we
    // free this block (when we free this block, we'll need to use the
    // next pointer, the prev pointer, and the boundary tag).
    reqSize = MIN_BLOCK_SIZE;
  } else {
    // Round up for correct alignment
    reqSize = ALIGNMENT * ((size + ALIGNMENT - 1) / ALIGNMENT);
  }


  freeBlock = searchFreeList(reqSize);
  if (freeBlock == NULL) {
  //No free block of requisite size is found. Solution: allocate more memory.
    requestMoreSpace(reqSize);
    freeBlock = searchFreeList(reqSize);
  }

  prevBlockUse = freeBlock->sizeAndTags & TAG_PRECEDING_USED; //previous block in use?
  blockSize = SIZE(freeBlock->sizeAndTags); //extract size of free block

  if ((blockSize-reqSize) >= MIN_BLOCK_SIZE) {
  //condition checks if we have leftover space, creates a split block.

    //splitBlock starts after the block we're allocating (freeBlock + reqSize)
    splitBlock = (BlockInfo*)UNSCALED_POINTER_ADD(freeBlock, reqSize);

    //now that we have splitBlock we can remove freeBlock
    removeFreeBlock(freeBlock);
    freeBlock->sizeAndTags = reqSize|prevBlockUse|TAG_USED;
    //update splitBlock tags and add it to the freelist
    splitBlock->sizeAndTags = (blockSize-reqSize)|TAG_PRECEDING_USED;
    ((BlockInfo*)UNSCALED_POINTER_ADD(freeBlock, blockSize - WORD_SIZE))->sizeAndTags=(blockSize-reqSize)|TAG_PRECEDING_USED;
    insertFreeBlock(splitBlock);
    }
  else {
    //otherwise we just update tags and remove the block from the freelist
    freeBlock->sizeAndTags |= TAG_USED;
    ((BlockInfo*)UNSCALED_POINTER_ADD(freeBlock, blockSize))->sizeAndTags |= TAG_PRECEDING_USED;
    removeFreeBlock(freeBlock);
  }

   return UNSCALED_POINTER_ADD(freeBlock, WORD_SIZE);
}

/* Free the block referenced by ptr and updates the free list. */
void mm_free (void *ptr) {
  size_t blockSize, payloadSize, prevBlockUse;
  BlockInfo * freeBlock;
  BlockInfo * nextBlock;

  //calculate block size and payload size
  freeBlock = (BlockInfo*)UNSCALED_POINTER_SUB(ptr, WORD_SIZE);
  blockSize = SIZE(freeBlock->sizeAndTags);
  payloadSize = blockSize - WORD_SIZE;

  //deal with next block's tag info
  prevBlockUse = freeBlock->sizeAndTags & TAG_PRECEDING_USED;
  nextBlock = (BlockInfo*)UNSCALED_POINTER_ADD(freeBlock, payloadSize);
  nextBlock->sizeAndTags = blockSize|prevBlockUse;

  //finally, free and coalesce
  insertFreeBlock(freeBlock);
  coalesceFreeBlock(freeBlock);
}

/* reallocates space as requested. takes a pointer to a block as well as the size of memory
   required and returns a pointer to the new allocated region of at least size bytes. */
void* mm_realloc(void* ptr, size_t size) {
  size_t blockUseTag, reqSize, blockSize, payloadSize, newSize, copySize;
  BlockInfo * oldPtr;
  BlockInfo * newPtr;

  //if no ptr given, same as a malloc for the size.
  if(ptr == NULL) { return mm_malloc(size); }

  //if no size given, same as a free at that ptr location.
  if(size == 0) {
    mm_free(ptr);
    return NULL;
  }

  //save the old pointer
  oldPtr = (BlockInfo *)UNSCALED_POINTER_SUB(ptr, WORD_SIZE);
  blockUseTag = oldPtr->sizeAndTags & TAG_USED;
  if (!blockUseTag) { return NULL; }

  //we require a correctly aligned memory space of size + word_size. use integer rounding
  reqSize = ALIGNMENT * ((size + WORD_SIZE + ALIGNMENT - 1) / ALIGNMENT);

  //define size of old block and payload size
  blockSize = SIZE(oldPtr->sizeAndTags);
  payloadSize = blockSize - WORD_SIZE;

  //if the block size given is at least equal to the space we need
  if (blockSize >= reqSize) {
    //and if the payload size is at least equal to the space we need
    if (payloadSize >= (size + WORD_SIZE)){
      oldPtr->sizeAndTags |= (TAG_PRECEDING_USED|TAG_USED); //set usage
      ((BlockInfo *)UNSCALED_POINTER_ADD(oldPtr, payloadSize))->sizeAndTags = size; //set size
      return ptr; //return the pointer
    }
  } else {
  //otherwise we don't have enough memory. solution: get more memory
    newSize = (32)*size + WORD_SIZE;
    //32 is an arbitrary choice, works just as well with 2. 32 is optimal for utilization, however (counterintuitive???)
    //handle creation of new pointer to block area with more space
    newPtr = (BlockInfo*)UNSCALED_POINTER_SUB(mm_malloc(newSize), WORD_SIZE);
    newPtr->sizeAndTags |= (TAG_PRECEDING_USED|TAG_USED);
    reqSize = SIZE(newPtr->sizeAndTags);
    *(size_t *)UNSCALED_POINTER_ADD(newPtr, reqSize - WORD_SIZE) = size;

    //determine how much data we'll actually be copying
    if (!(oldPtr->sizeAndTags&(TAG_PRECEDING_USED|TAG_USED))) {
      copySize = *(size_t *)UNSCALED_POINTER_ADD(ptr, payloadSize - WORD_SIZE);
    } else {
      copySize = payloadSize;
    }
    memcpy(UNSCALED_POINTER_ADD(newPtr, WORD_SIZE),ptr,copySize); //copies value of ptr to address of new pointer
    mm_free(ptr); //we can free the old pointer
    return UNSCALED_POINTER_ADD(newPtr, WORD_SIZE); //return the new pointer
   }
  //if we reach here, something has gone very wrong
   return NULL;
}


// Implement a heap consistency checker as needed.
//  optional but recommended
int mm_check() {
  return 0;
}
