#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

int main() {
    printf("Process ID: %d\n", getpid());

    // Allocate heap memory (~4MB)
    int *arr = (int *)malloc(1000000 * sizeof(int));
    if (arr == NULL) {
        perror("malloc failed");
        return 1;
    }

    for (int i = 0; i < 1000000; ++i) {
        arr[i] = i;
    }

    // Memory map one page (4KB)
    void *mapped_page = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mapped_page == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    // 📝 Now actually write to the page to force physical allocation
    ((char *)mapped_page)[0] = 'A';

    printf("Memory allocated, mapped, and written to.\n");
    printf("Press Enter to exit...\n");
    getchar();  // Pause to inspect

    // Cleanup
    munmap(mapped_page, 4096);
    free(arr);
    return 0;
}
