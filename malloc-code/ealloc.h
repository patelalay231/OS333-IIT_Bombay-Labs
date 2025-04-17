#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>

#define PAGESIZE 4096
#define MINALLOC 256
#define NUMCHUNKS (PAGESIZE / MINALLOC)  // 16
#define MAXPAGES 4

char *pages[MAXPAGES];           // Points to mmapped pages
char *free_chunks[MAXPAGES];     // Each points to array of 16 chars (1 = free, 0 = used)
int *alloc_sizes[MAXPAGES];      // Stores allocation size at the start chunk, 0 elsewhere
int num_pages = 0;               // How many pages currently allocated

// function declarations to support
void init_alloc(void);
char *alloc(int);
void dealloc(char *);
void cleanup(void);

void init_alloc() {
    num_pages = 0;
    for (int i = 0; i < MAXPAGES; i++) {
        pages[i] = NULL;
        free_chunks[i] = NULL;
        alloc_sizes[i] = NULL;
    }
}

char *alloc(int size){
    if (size <= 0 || size > PAGESIZE || size % MINALLOC != 0) {
        return NULL;
    }
    int required_chunks = size / MINALLOC;

    for(int i=0;i<num_pages;i++){
        for(int j=0;j<=NUMCHUNKS - required_chunks;j++){
            int free = 1;
            for(int k=0;k<required_chunks;k++){
                if(free_chunks[i][j+k] == 0){
                    free = 0;
                    break;
                }
            }
            if(free){
                for(int k=0;k<required_chunks;k++){
                    free_chunks[i][j+k] = 0;
                }
                alloc_sizes[i][j] = required_chunks;
                return pages[i] + (j * MINALLOC);
            }
        }
    }

    if (num_pages >= MAXPAGES) return NULL;

    char *new_page = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (new_page == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    pages[num_pages] = new_page;
    free_chunks[num_pages] = malloc(NUMCHUNKS);
    alloc_sizes[num_pages] = malloc(NUMCHUNKS * sizeof(int));

    memset(free_chunks[num_pages], 1, NUMCHUNKS);
    memset(alloc_sizes[num_pages], 0, NUMCHUNKS * sizeof(int));

    for (int i = 0; i < required_chunks; i++)
        free_chunks[num_pages][i] = 0;

    alloc_sizes[num_pages][0] = required_chunks;
    char *ptr = pages[num_pages];
    num_pages++;
    return ptr;
}

void dealloc(char *ptr) {
    for (int i = 0; i < num_pages; i++) {
        if (ptr >= pages[i] && ptr < pages[i] + PAGESIZE) {
            int offset = ptr - pages[i];
            int chunk_index = offset / MINALLOC;
            int size = alloc_sizes[i][chunk_index];

            if (size <= 0) {
                fprintf(stderr, "Invalid or double free\n");
                return;
            }

            for (int j = 0; j < size; j++)
                free_chunks[i][chunk_index + j] = 1;
            alloc_sizes[i][chunk_index] = 0;

            // Optional: merge adjacent free blocks for optimization
            return;
        }
    }
    fprintf(stderr, "Pointer not found in any page\n");
}


void cleanup() {
    // Not required to unmap pages
    for (int i = 0; i < MAXPAGES; i++) {
        if (free_chunks[i]) free(free_chunks[i]);
        if (alloc_sizes[i]) free(alloc_sizes[i]);
        free_chunks[i] = NULL;
        alloc_sizes[i] = NULL;
        pages[i] = NULL;
    }
    num_pages = 0;
}
