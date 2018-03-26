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


struct __attribute__((__packed__)) dir_entry_timedate_t {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour; 
    uint8_t minute; 
    uint8_t second;
};

struct __attribute__((__packed__)) dir_entry_t {
    uint8_t status;
    uint32_t starting_block;
    uint32_t block_count;
    uint32_t size;
    struct dir_entry_timedate_t modify_time;
    struct dir_entry_timedate_t create_time;
    uint8_t filename[31];
    uint8_t unused[6];
};

void format_timedate(struct dir_entry_timedate_t* t){
    printf("%d/%02d/%02d %02d:%02d:%02d\n", t->year, t->month, t->day, t->hour, t->minute, t->second);
}

void load_dir_entry(void* start, struct dir_entry_t* dir_entry){
    memcpy(dir_entry, start, sizeof(struct dir_entry_t));
    dir_entry->starting_block = htonl(dir_entry->starting_block);
    dir_entry->block_count = htonl(dir_entry->block_count);
    dir_entry->size = htonl(dir_entry->size);
    dir_entry->create_time.year = htons(dir_entry->create_time.year);
    dir_entry->modify_time.year = htons(dir_entry->modify_time.year);
}

void dump_dir_entry(void* dest, struct dir_entry_t* dir_entry){
    dir_entry->starting_block = ntohl(dir_entry->starting_block);
    dir_entry->block_count = ntohl(dir_entry->block_count);
    dir_entry->size = ntohl(dir_entry->size);
    dir_entry->create_time.year = ntohs(dir_entry->create_time.year);
    dir_entry->modify_time.year = ntohs(dir_entry->modify_time.year);
    memcpy(dest, dir_entry, sizeof(struct dir_entry_t));
}

void format_dir_entry(struct dir_entry_t* dir_entry){
    printf(
        "status: %d, starting_block: %d, block_count: %d, size: %d, filename: %s\
        \n", dir_entry->status, dir_entry->starting_block, \
        dir_entry->block_count, dir_entry->size, dir_entry->filename);
    format_timedate(&dir_entry->create_time);
    format_timedate(&dir_entry->modify_time);
}


char** directory_parts(char* str){
    char a[100];
    strcpy(a, str);
    char** parts = malloc(100*sizeof(void*));
    for (int i=0; i<100; i++){
        parts[i] = (char*)malloc(100*sizeof(char));
        memset(parts[i], '\0', 100);
    }

    char *token = strtok(a, "/");
    int index = 0;
   
    while (token != NULL)
    {
        // parts[index++] = token;
        memcpy(parts[index++], token, strlen(token));
        token = strtok(NULL, "/");
    }
    return parts;
}

int directory_length(char* str){
    int length = 0;
    if (strlen(str) == 1 && strcmp(str, "/") == 0){
        return length;
    }
    for (int i=0; i<strlen(str); i++){
        if (str[i] == '/'){
            length++;
        }
    }
    return length;
}

char* dir_basename(char* path){
    char** parts = directory_parts(path);
    int length = directory_length(path);
    return parts[length-1];
}

// global variables; using function get_distinfo to init them

int fd;
struct stat buffer;
void* address;  // fat system img start address

int blocksize, blockcount, fat_starts, fat_blocks, root_dir_start, root_dir_blocks;
int free_blocks, reserve_blocks, allocate_blocks;

void* fat_addr;  // fat table start address
void* root_start_addr; // root directory start address


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

    root_start_addr = address + root_dir_start*blocksize;
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

int block_stat(int blocknum){
    void* fat_entry_addr = fat_addr + blocknum * 4;
    int value;
    memcpy(&value, fat_entry_addr, 4);
    value = htonl(value);
    return value;
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

void* find_dir_entry_and_blocknum(char* dirname, int* blocknum){
    char file_name[32];
    char** dir_parts = directory_parts(dirname);
    int part_length = directory_length(dirname);
    int index = 0;
    int dir_blocks_num = root_dir_blocks; 
    void* dir_entry = root_start_addr;
    while (index<part_length){
        char* name = dir_parts[index];
        int bool_find = 0;
        int starting_block, number_block;
        for(int i=0; i<dir_blocks_num*blocksize/64; i++){
            void* item_entry = dir_entry + i * 64;
            char status;

            memcpy(&status, item_entry, 1);
            if ((status&0x01) == 0) {
                continue;
            }

            memset(file_name, '\0', 32);
            memcpy(&file_name, item_entry+27, 31);
            if ((status&0x02) != 0x02 && strcmp(file_name, name)==0) {
                bool_find = 1;
                memcpy(&starting_block, item_entry+1, 4);
                starting_block = htonl(starting_block);
                memcpy(&number_block, item_entry+5, 4);
                number_block = htonl(number_block);
                break;
            }
        }
        if (!bool_find){
            printf("dirname %s not found\n", dirname);
            exit(0);
        }
        else{
            // update pointers
            index++;
            dir_blocks_num = number_block;
            dir_entry = address + starting_block*blocksize;
        }
    }

    *blocknum = dir_blocks_num;
    return dir_entry;
}


void disklist(int argc, char* argv[]){
    if (argc != 3){
		fprintf(stderr, "usage: ./disklist test.img /sub_dir\n");
		exit(0);
    }

    char* dirname = argv[2];
    printf("dirname: %s\n", dirname);

    int dir_blocks_num;
    void* dir_entry = find_dir_entry_and_blocknum(dirname, &dir_blocks_num);

    for(int i=0; i<dir_blocks_num*blocksize/64; i++){
        void* root_item_addr = dir_entry + i * 64;
        struct dir_entry_t* dir_entry = (struct dir_entry_t*)malloc(sizeof(struct dir_entry_t));
        load_dir_entry(root_item_addr, dir_entry);

        int status = dir_entry->status;
        char ftype;

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
        struct dir_entry_timedate_t mt = dir_entry->modify_time;

        printf(
            "%c %10d %30s %d/%02d/%02d %02d:%02d:%02d\n", \
            ftype, dir_entry->size, dir_entry->filename, mt.year, mt.month, mt.day, mt.hour, mt.minute, mt.second);
    }
}


void diskget(int argc, char* argv[]){
    if (argc != 4){
		fprintf(stderr, "usage: ./diskget test.img /sub_dir/foo2.txt foo.txt\n");
		exit(0);
    }
    char* filepath = argv[2];
    char* basename = dir_basename(filepath);
    char* localpath = argv[3];
    printf("base name :%s, localpath: %s\n", basename, localpath);
    get_diskinfo(argv[1]);

    char status;
    int starting_block, number_block, file_size;
    char ftype='F';

    int bool_find = 0;

    struct dir_entry_timedate_t ct;
    struct dir_entry_timedate_t mt;
    void* root_item_addr = NULL;
    char file_name[32];

    for(int i=0; i<root_dir_blocks*blocksize/64; i++){
        root_item_addr = root_start_addr + i * 64;
        memcpy(&status, root_item_addr, 1);
        memset(file_name, '\0', 32);
        memcpy(&file_name, root_item_addr+27, 31);
        if ((status&0x01) == 0 || strcmp(basename, file_name) != 0) {
            continue;
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

        memcpy(&mt, root_item_addr+20, 7);
        mt.year = htons(mt.year);

        printf("%c %10d %30s %d/%02d/%02d %02d:%02d:%02d\n", ftype, file_size, file_name, mt.year, mt.month, mt.day, mt.hour, mt.minute, mt.second);
        bool_find = 1;
        break;
    }
    if (!bool_find){
        printf("File not found\n");
        exit(0);
    }

    // copy to local 
    void* fat_start_addr = address + blocksize*fat_starts;
    int index = 0;
    int start = starting_block;
    void* buf = malloc(file_size);
    printf("blocksize: %d\n", number_block);
    while (index < number_block){
        int fat_value = htonl(*(int*)(fat_start_addr + 4*start));
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
    stream = fopen(localpath, "w");
    fwrite(buf, 1, file_size, stream);
    fclose(stream);

    release_img();
}


void diskput(int argc, char* argv[]){
    if (argc != 4){
		fprintf(stderr, "usage: ./diskput test.img foo.txt /sub_dir/foo3.txt\n");
		exit(0);
    }
    get_diskinfo(argv[1]);

    char* localf = argv[2];
    char* basename = dir_basename(argv[3]);
    int localfd = open(localf, O_RDWR);
    struct stat local_buffer;
    fstat(localfd, &local_buffer);

    void* local_address=mmap(NULL, local_buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, localfd, 0);
    printf("local file size: %lld\n", local_buffer.st_size);

    int block_need = (int)ceil((float)local_buffer.st_size/blocksize);
    printf("block needed: %d\n", block_need); 

    int local_free = 0;
    int fat_free[100];

    // find the enough free blocks to store the file
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
        exit(0);
    }

    // for (int i=0; i<local_free; i++){
    //     printf("blcok free number: %d\n", fat_free[i]);
    // }

    // copy data to blocks
    for (int i=0; i<local_free; i++){
        if (i == (block_need-1)){
            int left = local_buffer.st_size - (block_need-1)*blocksize;
            memcpy(address + fat_free[i]*blocksize, local_address + i*blocksize, left);
        }
        else{
            memcpy(address + fat_free[i]*blocksize, local_address + i*blocksize, blocksize);
        }
    }

    // update fat
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

    // find subdirectory and update directory entry
    void* free_d_entry = NULL;
    int find = 0;
    char status;
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
    memcpy(free_d_entry+27, basename, strlen(basename));

    release_img();
}

void test_insert_dir_entry(){
    int start_block = 200;
    struct dir_entry_t* dir_entry = (struct dir_entry_t*)malloc(sizeof(struct dir_entry_t));
    dir_entry->status = 5;
    memcpy(dir_entry->filename, "subdirectory", strlen("subdirectory"));
    dir_entry->starting_block = start_block;
    dir_entry->size = 64;
    dir_entry->block_count = 1;

    struct dir_entry_timedate_t* ct = &dir_entry->create_time;
    ct->year = 2018;
    ct->month = 1;
    ct->day = 1;
    ct->hour = 1;
    ct->minute = 1;
    ct->second = 1;

    struct dir_entry_timedate_t* mt = &dir_entry->modify_time;
    mt->year = 2018;
    mt->month = 1;
    mt->day = 1;
    mt->hour = 1;
    mt->minute = 1;
    mt->second = 1;

    void* start = address + root_dir_start * blocksize + 64*5;
    dump_dir_entry(start, dir_entry);

    // update fat
    *(int*)(fat_addr+4*start_block) = ntohl(0xFFFFFFFF);

    // insert 2 files
    void* file1_start = address + start_block*blocksize;
    struct dir_entry_t* f1_entry = (struct dir_entry_t*)malloc(sizeof(struct dir_entry_t));
    f1_entry->status=3;
    f1_entry->size = 100;
    char* fname1 = "file1.txt";
    memcpy(f1_entry->filename, fname1, strlen(fname1));
    memcpy(&f1_entry->modify_time, mt, sizeof(struct dir_entry_timedate_t));
    f1_entry->modify_time.year=2018;
    dump_dir_entry(file1_start, f1_entry);

    void* file2_start = address + start_block*blocksize+64;
    struct dir_entry_t* f2_entry = (struct dir_entry_t*)malloc(sizeof(struct dir_entry_t));
    f2_entry->status=3;
    f2_entry->size = 120;
    char* fname2 = "file2.txt";
    memcpy(f2_entry->filename, fname1, strlen(fname2));
    memcpy(&f2_entry->modify_time, mt, sizeof(struct dir_entry_timedate_t));
    f2_entry->modify_time.year=2018;
    dump_dir_entry(file2_start, f2_entry);
    free(dir_entry);
}

void test_load_dump_dir_entry(){
    void* start = address + root_dir_start * blocksize + 64*1;
    struct dir_entry_t* dir_entry = (struct dir_entry_t*)malloc(sizeof(struct dir_entry_t));
    load_dir_entry(start, dir_entry);
    format_dir_entry(dir_entry);


    dump_dir_entry(start, dir_entry);
    load_dir_entry(start, dir_entry);
    format_dir_entry(dir_entry);
}

void disktest(int argc, char* argv[]){
    // printf("block stat %s: %d\n", argv[2],  block_stat(atoi(argv[2])));
    // int block_start_num = 200;
    test_insert_dir_entry();
}

int main(int argc, char* argv[]) {
    if (argc < 2){
        fprintf(stderr, "at least 2 arguments: ./xxx_command test.img\n");
        exit(0);
    }
    get_diskinfo(argv[1]);
#if defined(PART1)
	diskinfo(argc, argv); 
#elif defined(PART2)
	disklist(argc, argv); 
#elif defined(PART3)
	diskget(argc, argv); 
#elif defined(PART4)
	diskput(argc,argv); 
#elif defined(TEST)
    disktest(argc,argv); 
#else
# 	error "PART[1234] must be defined" 
#endif
    release_img();
	return 0;
}