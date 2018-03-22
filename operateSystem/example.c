/*This example code is for reading from test.img and obtain data with byte ordering of Big Endian. Also try to directly print binary data as a string and make modification.*/

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {

    int fd = open(argv[1], O_RDWR);
    struct stat buffer;
    int status = fstat(fd, &buffer);

    //tamplate:   pa=mmap(addr, len, prot, flags, fildes, off);
    //c will implicitly cast void* to char*, while c++ does NOT
    char* address=mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    int blocksize=0;
    memcpy(&blocksize,address+8,2);
    printf("directly getting block size: %d\n",blocksize);
    blocksize=htons(blocksize);
    printf("getting block size after htons: %d\n",blocksize);

    //printf("print img as string:%s(END)\n",address);
    
    //int temp=0x32;
    //memcpy(address+8,&temp,1);
    //printf("print img as string:%s(END)\n",address);

    munmap(address,buffer.st_size);
    close(fd);
}

