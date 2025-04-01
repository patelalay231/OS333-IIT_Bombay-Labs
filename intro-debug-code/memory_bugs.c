#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfree-nonheap-object"

int main()
{
    char *p;
    int *q;

    int arr[10] = {0,0,0,0,0,0,0,0,0,0};
    // Allocation #1 of 30 bytes
    p = (char *)malloc(30);
    free(p);
    // Print element of array
    write(fileno(stdout), arr, 10);

    // Allocation #2 of 12 bytes
    p = (char *)malloc(12);


    // Store character 'A' in memory pointed by p


    // Print the value stored at memory pointed by p
    printf("%c\n", *p);
    
	    // Allocation #3 of 50 bytes
    q = (int *)malloc(50);
    free(q);
    free(p);
    // Free memory allocated to array arr
    

    return 0;
}
