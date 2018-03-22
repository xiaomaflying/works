#include <stdio.h>
#include <stdint.h> 
#include <stdlib.h> 
#include <sys/stat.h> 
#include <string.h> 
#include <limits.h> 
#include <assert.h> 
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h> 


void diskinfo(int argc, char* argv[]){

    int fd = open(argv[1], O_RDWR);
    struct stat buffer;
    fstat(fd, &buffer);

    void* address=mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    int blocksize, blockcount, fat_starts, fat_blocks, root_dir_start, root_dir_blocks;
    printf("Super block information: \n");
    memcpy(&blocksize,address+8,2);
    blocksize=htons(blocksize);
    printf("Block size: %d\n",blocksize);

    memcpy(&blockcount, address+10, 4);
    blockcount=htonl(blockcount);
    printf("Block count: %d\n", blockcount);

    memcpy(&fat_starts, address+14, 4);
    fat_starts=htonl(fat_starts);
    printf("FAT starts: %d\n", fat_starts);

    memcpy(&fat_blocks, address+18, 4);
    fat_blocks=htonl(fat_blocks);
    printf("FAT blocks: %d\n", fat_blocks);

    memcpy(&root_dir_start, address+22, 4);
    root_dir_start=htonl(root_dir_start);
    printf("Root directory start: %d\n", root_dir_start);

    memcpy(&root_dir_blocks, address+26, 4);
    root_dir_blocks=htonl(root_dir_blocks);
    printf("Root directory blocks: %d\n", root_dir_blocks);

    printf("\n");
    printf("FAT information: \n");
    int free_blocks = 0;
    int reserve_blocks = 0;
    int allocate_blocks = 0;

    void* fat_addr = address + fat_starts * blocksize;
    int fat_item_value;
    void* fat_item_addr = NULL;
    printf("fat item number : %d\n", fat_blocks * blocksize / 4);


    for (int i = 0; i < fat_blocks * blocksize / 4; i++){
        fat_item_addr = fat_addr + i * 4;
        // printf("i :%d, address: %p, block_addr : %p\n", i, address, block_addr);
        memcpy(&fat_item_value, fat_item_addr, 4);
        fat_item_value = htonl(fat_item_value);
        if (fat_item_value == 0)
            free_blocks += 1;
        else if (fat_item_value == 1)
            reserve_blocks += 1;
        else
            allocate_blocks += 1;

        // printf("block num : %d, available : %d, reserve: %d\n", i, available, reserve);
    }
    printf("Free Blocks: %d\n", free_blocks);
    printf("Reserved Blocks: %d\n", reserve_blocks);
    printf("Allocated Blocks: %d\n", allocate_blocks);


    munmap(address,buffer.st_size);
    close(fd);
}


void disklist(int argc, char* argv[]){
	printf("in disklist");
}


void diskget(int argc, char* argv[]){
	printf("in diskget");
}


void diskput(int argc, char* argv[]){
	printf("in diskput");
}


int main(int argc, char* argv[]) {
#if defined(PART1)
	diskinfo(argc, argv); 
#elif defined(PART2)
	disklist(argc, argv); 
#elif defined(PART3)
	diskget(argc, argv); 
#elif defined(PART4)
	diskput(argc,argv); 
#else
# 	error "PART[1234] must be defined" 
#endif
	return 0;
}