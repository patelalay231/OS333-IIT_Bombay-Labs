#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

int main()
{
    const int SIZE = 4096;
    const char *NAME = "OS";
    char* FREE = "freeeee";
    char* MESSAGE = "OSisFUN";
    int MESSAGE_SIZE = 16;
    int TOTAL_SLOTS = SIZE / MESSAGE_SIZE;
    int shm_fd;
    void *shm_ptr;
    /* create the shared memory segment */
    shm_fd = shm_open(NAME, O_CREAT | O_RDWR, 0666);

    /* configure the size of the shared memory segment */
    ftruncate(shm_fd,SIZE);

    /* now map the shared memory segment in the address space of the process */
    shm_ptr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        printf("Map failed\n");
        return -1;
    }

    mkfifo("fifo_prod_to_cons", 0777);

    int fd_prod_to_cons = open("fifo_prod_to_cons", O_WRONLY);
    void* ptr = shm_ptr;
    for (int i = 0; i < TOTAL_SLOTS; i++) {
        sprintf(ptr, "%s", FREE);
        ptr += MESSAGE_SIZE;
    }    

    int itr = 1000;
    
    while(itr--){
        // find out the free slot
        int offset = -1;
        ptr = shm_ptr;
        for(int i=0;i < TOTAL_SLOTS;i++){
            if(strcmp(ptr,FREE) == 0){
                offset = i;
                break;
            }
            ptr = (char*)ptr + MESSAGE_SIZE;
        }
        if(offset != -1){
            // write to the free slot
            ptr = (char*)shm_ptr + offset * MESSAGE_SIZE;
            // write the message to the shared memory
            snprintf(ptr, MESSAGE_SIZE, "%s %d", MESSAGE,itr);
            // write the offset to the FIFO
            write(fd_prod_to_cons, &offset, sizeof(offset));
        }
        else{
            sleep(1);
        }
    }
    
    close(fd_prod_to_cons);
    unlink("fifo_prod_to_cons");
    return 0;
}
