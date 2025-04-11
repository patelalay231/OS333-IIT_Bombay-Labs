#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

int main()
{
    const char *name = "OS";
    const int SIZE = 4096;
    char* FREE = "freeeee";
    int MESSAGE_SIZE = 16;
    int shm_fd;
    void *shm_ptr;
    int i;

    /* open the shared memory segment */
    shm_fd = shm_open(name, O_RDWR, 0666);
    if (shm_fd == -1) {
        printf("shared memory failed\n");
        exit(-1);
    }

    /* now map the shared memory segment in the address space of the process */
    shm_ptr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        printf("Map failed\n");
        exit(-1);
    }
    /* open the FIFO */
    int fd_prod_to_cons = open("fifo_prod_to_cons", O_RDWR);
    while(1){
        /* read the message from the FIFO */
        int offset;
        void *ptr = shm_ptr;
        // read the data from FIFO
        int bytes_read = read(fd_prod_to_cons, &offset, sizeof(offset));
        if (bytes_read <= 0) {
            // sleep for a while if no data is available
            sleep(1);
            continue;
        }
        // read message from shared memory using offset
        ptr = (char*)shm_ptr + offset * MESSAGE_SIZE;
        printf("Received: %s\n", (char*)ptr);
        sprintf(ptr, "%s", FREE);
    }

    /* remove the shared memory segment */
    if (shm_unlink(name) == -1) {
        printf("Error removing %s\n",name);
        exit(-1);
    }
    
    close(fd_prod_to_cons);
    unlink("fifo_prod_to_cons");
    return 0;
}
