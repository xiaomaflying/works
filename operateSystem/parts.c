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


int* get_diskinfo(char* image){
    int fd = open(image, O_RDWR);
    struct stat buffer;
    fstat(fd, &buffer);

    void* address=mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    int blocksize, blockcount, fat_starts, fat_blocks, root_dir_start, root_dir_blocks;

    memcpy(&blocksize,address+8,2);
    blocksize=htons(blocksize);

    memcpy(&blockcount, address+10, 4);
    blockcount=htonl(blockcount);

    memcpy(&fat_starts, address+14, 4);
    fat_starts=htonl(fat_starts);

    memcpy(&fat_blocks, address+18, 4);
    fat_blocks=htonl(fat_blocks);

    memcpy(&root_dir_start, address+22, 4);
    root_dir_start=htonl(root_dir_start);

    memcpy(&root_dir_blocks, address+26, 4);
    root_dir_blocks=htonl(root_dir_blocks);

    int free_blocks = 0;
    int reserve_blocks = 0;
    int allocate_blocks = 0;

    void* fat_addr = address + fat_starts * blocksize;
    int fat_item_value;
    void* fat_item_addr = NULL;

    for (int i = 0; i < fat_blocks * blocksize / 4; i++){
        fat_item_addr = fat_addr + i * 4;
        memcpy(&fat_item_value, fat_item_addr, 4);
        fat_item_value = htonl(fat_item_value);
        if (fat_item_value == 0)
            free_blocks += 1;
        else if (fat_item_value == 1)
            reserve_blocks += 1;
        else
            allocate_blocks += 1;
    }

    static int info_result[9];
    info_result[0] = blocksize;
    info_result[1] = blockcount;
    info_result[2] = fat_starts;
    info_result[3] = fat_blocks;
    info_result[4] = root_dir_start;
    info_result[5] = root_dir_blocks;
    info_result[6] = free_blocks;
    info_result[7] = reserve_blocks;
    info_result[8] = allocate_blocks;

    munmap(address,buffer.st_size);
    close(fd);
    return info_result;
}


void diskinfo(int argc, char* argv[]){
    if (argc != 2){
		fprintf(stderr, "usage: ./diskinfo test.img\n");
		exit(0);
    }

    int blocksize, blockcount, fat_starts, fat_blocks, root_dir_start, root_dir_blocks;
    int free_blocks, reserve_blocks, allocate_blocks;
    int* info_result = get_diskinfo(argv[1]);
    blocksize = info_result[0];
    blockcount = info_result[1];
    fat_starts = info_result[2];
    fat_blocks = info_result[3];
    root_dir_start = info_result[4];
    root_dir_blocks = info_result[5];
    free_blocks = info_result[6];
    reserve_blocks = info_result[7];
    allocate_blocks = info_result[8];

    printf("Super block information: \n");

    printf("Block size: %d\n",blocksize);

    printf("Block count: %d\n", blockcount);

    printf("FAT starts: %d\n", fat_starts);

    printf("FAT blocks: %d\n", fat_blocks);

    printf("Root directory start: %d\n", root_dir_start);

    printf("Root directory blocks: %d\n", root_dir_blocks);

    printf("\n");
    printf("FAT information: \n");

    printf("Free Blocks: %d\n", free_blocks);
    printf("Reserved Blocks: %d\n", reserve_blocks);
    printf("Allocated Blocks: %d\n", allocate_blocks);

}


void disklist(int argc, char* argv[]){
    if (argc != 3){
		fprintf(stderr, "usage: ./disklist test.img /sub_dir\n");
		exit(0);
    }

    char* dirname = argv[2];
    printf("dirname: %s", dirname);

    int blocksize, blockcount, fat_starts, fat_blocks, root_dir_start, root_dir_blocks;
    int free_blocks, reserve_blocks, allocate_blocks;
    int* info_result = get_diskinfo(argv[1]);
    blocksize = info_result[0];
    blockcount = info_result[1];
    fat_starts = info_result[2];
    fat_blocks = info_result[3];
    root_dir_start = info_result[4];
    root_dir_blocks = info_result[5];
    free_blocks = info_result[6];
    reserve_blocks = info_result[7];
    allocate_blocks = info_result[8];

    
}


void diskget(int argc, char* argv[]){
    if (argc != 4){
		fprintf(stderr, "usage: ./diskget test.img /sub_dir/foo2.txt foo.txt\n");
		exit(0);
    }
}


void diskput(int argc, char* argv[]){
    if (argc != 4){
		fprintf(stderr, "usage: ./diskput test.img foo.txt /sub_dir/foo3.txt\n");
		exit(0);
    }

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