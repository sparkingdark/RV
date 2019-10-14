#ifndef MEMORY_H                    
#define MEMORY_H                    

#define GROW_NUM_OF_ALLOCATED(numOfAllocated) ((numOfAllocated) < 8 ? 8 : (numOfAllocated) * 2)

#define GROW_ARRAY(previous, type, oldCount, count) \
    (type*)reallocate(previous, sizeof(type) * (oldCount), sizeof(type) * (count))

#define FREE_ARRAY(type, pointer, oldCount) reallocate(pointer, sizeof(type) * (oldCount), 0)           

void *reallocate(void *previous, size_t oldSize, size_t newSize);

/*
oldSize		newSize						operation
-----------------------------------------------------------------
0			non-zero					allocate new block
non-zero	0							free allocation
non-zero	less than oldSize			shrink exsiting allocation
non-zero	larger than oldSize			grow exsiting allocation
*/

#endif                                   
