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
#include <math.h>


struct info{
    char c;
    int i;
    short j;
};


char** directory_parts(char* str){
    char a[100];
    strcpy(a, str);
    char** parts = malloc(32*sizeof(char*));
    for (int i=0; i<32; i++){
        parts[i] = (char*)malloc(32*sizeof(char));
    }

    char *token = strtok(a, "/");
    int index = 0;
   
    while (token != NULL)
    {
        parts[index++] = token;
        token = strtok(NULL, "/");
    }
    return parts;
}

int directory_length(char* str){
    int length = 0;
    // printf("%lu\n", strlen(str));
    for (int i=0; i<strlen(str); i++){
        if (str[i] == '/'){
            length++;
        }
    }
    return length;
}


// global variables; using function get_distinfo to init them

int fd;
struct stat buffer;
void* address;  // fat system img start address

int blocksize, blockcount, fat_starts, fat_blocks, root_dir_start, root_dir_blocks;
int free_blocks, reserve_blocks, allocate_blocks;

void* fat_addr;  // fat table address


// init the global variables
void get_diskinfo(char* image){
    fd = open(image, O_RDWR);
    fstat(fd, &buffer);

    address=mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

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

    int _free_blocks = 0;
    int _reserve_blocks = 0;
    int _allocate_blocks = 0;

    fat_addr = address + fat_starts * blocksize;
    int fat_item_value;
    void* fat_item_addr = NULL;

    for (int i = 0; i < fat_blocks * blocksize / 4; i++){
        fat_item_addr = fat_addr + i * 4;
        memcpy(&fat_item_value, fat_item_addr, 4);
        fat_item_value = htonl(fat_item_value);
        if (fat_item_value == 0)
            _free_blocks += 1;
        else if (fat_item_value == 1)
            _reserve_blocks += 1;
        else
            _allocate_blocks += 1;
    }

    free_blocks = _free_blocks;
    reserve_blocks = _reserve_blocks;
    allocate_blocks = _allocate_blocks;

}


void release_img(){
    munmap(address, buffer.st_size);
    close(fd);
}


void diskinfo(int argc, char* argv[]){
    if (argc != 2){
		fprintf(stderr, "usage: ./diskinfo test.img\n");
		exit(0);
    }

    get_diskinfo(argv[1]);

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

    release_img();

}

struct __attribute__((__packed__)) dir_entry_timedate_t {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour; 
    uint8_t minute; 
    uint8_t second;
};


void disklist(int argc, char* argv[]){
    if (argc != 3){
		fprintf(stderr, "usage: ./disklist test.img /sub_dir\n");
		exit(0);
    }

    char* dirname = argv[2];
    printf("dirname: %s\n", dirname);

    get_diskinfo(argv[1]);

    char status;
    int starting_block, number_block, file_size;
    char ftype;
    // short int year;
    // char month, day, hour, minute, second;
    struct dir_entry_timedate_t ct;
    struct dir_entry_timedate_t mt;
    void* root_item_addr = NULL;
    char file_name[32];

    void* root_start_addr = address + root_dir_start*blocksize;
    for(int i=0; i<root_dir_blocks*blocksize/64; i++){
        root_item_addr = root_start_addr + i * 64;
        memcpy(&status, root_item_addr, 1);
        if ((status&0x01) == 0) {
            continue;
        }
        if ((status&0x02) == 0x02) {
            ftype = 'F';
        }
        else{
            ftype = 'D';
        }
        memcpy(&starting_block, root_item_addr+1, 4);
        starting_block = htonl(starting_block);
        memcpy(&number_block, root_item_addr+5, 4);
        number_block = htonl(number_block);
        memcpy(&file_size, root_item_addr+9, 4);
        file_size = htonl(file_size);
        // printf("index: %d, status %d, start block: %d, block number: %d, file size: %d\n", i, status, starting_block, number_block, file_size);

        memcpy(&ct, root_item_addr+13, 7);
        ct.year = htons(ct.year);
        // printf("create time: %d-%02d-%02d %02d:%02d:%02d\n", ct.year, ct.month, ct.day, ct.hour, ct.minute, ct.second);

        memcpy(&mt, root_item_addr+20, 7);
        mt.year = htons(mt.year);
        // printf("modify time: %d-%02d-%02d %02d:%02d:%02d\n", mt.year, mt.month, mt.day, mt.hour, mt.minute, mt.second);

        memcpy(&file_name, root_item_addr+27, 31);

        printf("%c %10d %30s %d/%02d/%02d %02d:%02d:%02d\n", ftype, file_size, file_name, mt.year, mt.month, mt.day, mt.hour, mt.minute, mt.second);
        // printf("name: %s\n", file_name);


        // copy to local 
        if (strcmp(file_name, "disk.img.gz") == 0) {
            void* fat_start_addr = address + blocksize*fat_starts;
            int index = 0;
            int start = starting_block;
            char* localfile = "disk.img.gz";
            void* buf = malloc(file_size);
            printf("blocksize: %d\n", number_block);
            while (index < number_block){
                int fat_value = htonl(*(int*)(fat_start_addr + 4*start));
                printf("%d\n", fat_value);
                if (index == (number_block-1)){
                    int left = file_size - (number_block-1)*blocksize;
                    memcpy(buf+index*blocksize, address+start*blocksize, left);
                }
                else{
                    memcpy(buf+index*blocksize, address+start*blocksize, blocksize);
                }
                index++;
                start = fat_value;
            }

            // write memory data to local file
            FILE* stream;
            stream = fopen(localfile, "w");
            fwrite(buf, 1, file_size, stream);
            fclose(stream);
        }
    }

    

    // put file to fat system

    char* localf = "example.c";
    int localfd = open(localf, O_RDWR);
    struct stat local_buffer;
    fstat(localfd, &local_buffer);

    void* local_address=mmap(NULL, local_buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, localfd, 0);
    printf("local file size: %lld\n", local_buffer.st_size);

    int block_need = (int)ceil((float)local_buffer.st_size/blocksize);
    // int block_need = 100000;
    printf("block needed: %d\n", block_need); 

    // 循环遍历block Fat 找到block_need 个block number (即fat的index)如果没有，那就说，空间不够了
    // 检查directory entry 还有没有空闲的了(后做)
    
    int local_free = 0;
    int fat_free[100];
    void* fat_addr = address + fat_starts * blocksize;
    for (int i = 0; i < fat_blocks * blocksize / 4; i++){
        int fat_item_value;
        void* fat_item_addr = fat_addr + i * 4;
        memcpy(&fat_item_value, fat_item_addr, 4);
        fat_item_value = htonl(fat_item_value);
        if (fat_item_value == 0){
            fat_free[local_free] = i;
            local_free += 1;
        }
        if (local_free == block_need)
            break;
    }
    if (local_free != block_need){
        printf("no space\n");
    }

    // for (int i=0; i<local_free; i++){
    //     printf("blcok free number: %d\n", fat_free[i]);
    // }

    // 拷贝内容到对应的block
    for (int i=0; i<local_free; i++){
        if (i == (block_need-1)){
            int left = local_buffer.st_size - (block_need-1)*blocksize;
            memcpy(address + fat_free[i]*blocksize, local_address + i*blocksize, left);
        }
        else{
            memcpy(address + fat_free[i]*blocksize, local_address + i*blocksize, blocksize);
        }
    }

    // 更新fat
    for (int i=0; i<local_free; i++){
        int blocknum = fat_free[i];
        if (i+1<local_free){
            *(int*)(fat_addr+4*blocknum) = ntohl(fat_free[i+1]);
        }
        else {
            *(int*)(fat_addr+4*blocknum) = ntohl(0xFFFFFFFF);
        }
    }

    // for (int i=0; i<local_free; i++){
    //     printf("blocknum: %d, block next num: %d\n", fat_free[i], htonl(*(int*)(fat_addr+fat_free[i]*4)));
    // }

    // 更新directory entry
    void* free_d_entry = NULL;
    int find = 0;
    for(int i=0; i<root_dir_blocks*blocksize/64; i++){
        free_d_entry = root_start_addr + i * 64;
        memcpy(&status, free_d_entry, 1);
        if ((status&0x01) == 0) {
            find = 1;
            break;
        }
    }
    if (!find){
        printf("can find drectory entry");
    }

    char f_status = 0x03;
    int start_block = ntohl(fat_free[0]);
    int fsize = ntohl(local_buffer.st_size);
    int block_num = ntohl(block_need);
    time_t now;
    struct tm *timeinfo;
    time(&now);
    timeinfo = localtime(&now);
    struct dir_entry_timedate_t t;
    t.year = ntohs(timeinfo->tm_year+1900);
    t.month = timeinfo->tm_mon + 1;
    t.day = timeinfo->tm_mday;
    t.hour = timeinfo->tm_hour;
    t.minute = timeinfo->tm_min;
    t.second = timeinfo->tm_sec;

    memcpy(free_d_entry, &f_status, sizeof(f_status));
    memcpy(free_d_entry+1, &start_block, 4);
    memcpy(free_d_entry+5, &block_num, 4);
    memcpy(free_d_entry+9, &fsize, 4);
    memcpy(free_d_entry+13, &(t), sizeof(t));
    memcpy(free_d_entry+20, &(t), sizeof(t));
    memset(free_d_entry+27, '\0', 31);
    memcpy(free_d_entry+27, localf, strlen(localf));

    // 这个做完了，通过上一个程序检验，看看这个文件能取出来不

    // 做递归找目录的程序


    //整理代码，把公用的变量当做全局变量

    munmap(local_address, local_buffer.st_size);
    close(localfd);


    munmap(address,buffer.st_size);
    close(fd);
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