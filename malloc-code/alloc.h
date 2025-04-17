#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>

#define PAGESIZE 4096 //size of memory to allocate from OS
#define MINALLOC 8 //allocations will be 8 bytes or multiples of it
#define NUMCHUNKS PAGESIZE / MINALLOC 

// This is a simple memory allocator that uses mmap to allocate memory from the OS.
void *mapped_page = NULL; // Pointer to the memory pool
char free_chunks[NUMCHUNKS]; // 1 = free , 0 = used
int alloc_sizes[NUMCHUNKS];  // Stores number of chunks allocated at start index, 0 otherwise

// function declarations
int init_alloc();
int cleanup();
char *alloc(int);
void dealloc(char *);

// function definitions
int init_alloc() {
    // Initialize the memory pool
    mapped_page = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mapped_page == MAP_FAILED) {
        perror("mmap");
        return -1;
    }
    
    memset(free_chunks,1,sizeof(free_chunks));
    memset(alloc_sizes, 0, sizeof(alloc_sizes));
    return 0;
}

int cleanup() {
    // Clean up the memory pool
    if(mapped_page != NULL) {
        if (munmap(mapped_page, PAGESIZE) == -1) {
            perror("munmap");
            return -1;
        }
    }
    // Free the memory pool
    mapped_page = NULL;
    return 0;
}

char *alloc(int size) {
    // Allocate memory of the given size
    if (size <= 0 || size > PAGESIZE || size % MINALLOC != 0) {
        return NULL;
    }
   
    int required_chunks = size / MINALLOC;

    for (int i = 0; i <= NUMCHUNKS - required_chunks; i++) {
        int found = 1;
        for (int j = 0; j < required_chunks; j++) {
            if (!free_chunks[i + j]) {
                found = 0;
                break;
            }
        }

        if (found) {
            for (int j = 0; j < required_chunks; j++) {
                free_chunks[i + j] = 0;
            }
            alloc_sizes[i] = required_chunks; // store size externally
            return (char *)mapped_page + i * MINALLOC;
        }
    }
    return NULL;
}

void dealloc(char *ptr) {
    // Deallocate the memory
    if (ptr == NULL || ptr < (char *)mapped_page ||
        ptr >= (char *)mapped_page + PAGESIZE) {
        fprintf(stderr, "Invalid pointer: %p\n", ptr);
        return;
    }

    int index = (ptr - (char *)mapped_page) / MINALLOC;
    int size = alloc_sizes[index];

    if (size <= 0 || index >= NUMCHUNKS) {
        fprintf(stderr, "Pointer not pointing to allocated block\n");
        return;
    }

    for (int i = index; i < index + size && i < NUMCHUNKS; i++) {
        free_chunks[i] = 1;
    }
    alloc_sizes[index] = 0;
}