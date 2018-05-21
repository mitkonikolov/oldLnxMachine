
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#include "storage.h"

static void* mem_base = 0;
static int   mem_fd = -1;

static size_t BLOCK_SIZE = 4096;
static size_t MEM_SIZE = 1048576;
/*
typedef struct file_data {
    const char* path;
    int         mode;
    const char* data;
} file_data;

static file_data file_table[] = {
    {"/", 040755, 0},
    {"/hello.txt", 0100644, "hello\n"},
    {0, 0, 0},
};
*/

typedef struct file_data {
    int inum;
    char* path;
    int strLen;
    int recMem;
} file_data;

struct inode {
    void* block;
    int refCount;
    int is_directory;
    int size;
}inode;

static inode* inodes;
static int num_ints = 0;

static file_data default_list[] = {
     { 2, ".", 1, 1 },
     { 3, "..", 2, 1 },
     { 0, 0, 0, 0}
};

static inode*
get_inode(int inumber) {
    return inodes + 256 * inumber;
}

int
get_block_count(size_t mem_size, size_t block_size) {
    //TODO: if supporting sizes other than 1MB update this
    return 238; //256;//TODO: may cause problems if 256 is too many for 1MB
}

void
init_bitmaps(void* ptr, int block_count) {
    // determine number of ints for bitmap
    num_ints = (block_count / 32) + 1;

    void* old_ptr = ptr;
    // initialize inode bitmap in first page
    // TODO: this loop could cause problems if the bitmap doesnt fit
    //       in one memory block
    for (int ii = 0; ii < num_ints; ii++) {
        *((uint32_t*)ptr) = 0;
        ptr += sizeof(uint32_t);
    }

    // set the pointer to the next page
    ptr = old_ptr + BLOCK_SIZE;

    // initialize data blocks bitmap in first page
    // TODO: this loop could cause problems if the bitmap doesnt fit
    //       in one memory block
    for (int ii = 0; ii < num_ints; ii++) {
        *((uint32_t*)ptr) = 0;
        ptr += sizeof(uint32_t);
    }
}

void
init_inodes(void* ptr, int inode_count) {
    // set an inode at every 256 bytes
//    for (int ii = 0; ii < block_count; ii++) {
//        //set an inode at ptr
//        ptr = (inode*)ptr;
//        // increment pointer
//        ptr += 256;
//    }
     inodes = (inode*)ptr;
}

//initializes all of the data
void
init(void* memory) {
    //init super block
    int block_count = get_block_count(MEM_SIZE, BLOCK_SIZE);//TODO
    void* ptr = memory;

    // num of inodes
    *((int*)ptr) = block_count + 2;
    ptr += sizeof(int);

    // num of blocks
    *((int*)ptr) = block_count;
    ptr += sizeof(int);

    // inode start address
    *((uint32_t*)ptr) = (uint32_t)(memory + (BLOCK_SIZE * 3));
    //TODO: this may need to be an address not a ptr
    ptr += sizeof(uint32_t);

    // fsName
    *((char**)ptr) = "FS NAME";//TODO

    // init bitmaps
    init_bitmaps(memory + BLOCK_SIZE, block_count);
    // init inodes
    init_inodes(memory + (BLOCK_SIZE * 3), block_count + 2);
    // init root directory
    void* root_ptr = memory + (BLOCK_SIZE * 18);
    memcpy(root_ptr, default_list, BLOCK_SIZE);
    // initialize root inode
    inode* root = get_inode(2);
    root->block = root_ptr;
    root->size = BLOCK_SIZE;
}


void
storage_init(const char* path)
{
    //printf("TODO: Store file system data in: %s\n", path);
    //TODO: O_CREAT will need to be changed

    mem_fd = open(path, O_CREAT|O_RDWR, 0644);

    ftruncate(mem_fd, MEM_SIZE);
    
    mem_base = mmap(NULL, MEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, 0);

    init(mem_base);
}

int  
storage_has_inode() {
    //check bitmaps
    void* ptr = mem_base+BLOCK_SIZE;
    for (int ii = 0; ii < num_ints; ii++) {
        ptr = ptr + ii*sizeof(int);
        for (int p=0; p<32; p++) {
            uint32_t mask= (int)pow(2, ptr);
        }
    } 
}

int   storage_has_blocks(int num_blocks) {
    return 0;
}

void*
storage_get_block(const char* path) {
    // check root directory for the file path
    inode* root = get_inode(2);// go to inode

    // get file_data[] from inode->block
    file_data* dir = (file_data*)(root->block);
    
    for (int ii = 0; 1; ii++) {
        if (dir == 0)
            break;
        if (streq(path, dir[ii]->path)) {
            inode* file = get_inode(dir[ii]->inum);
            return file->block;
        }
    }
    return 0;
}

static int
streq(const char* aa, const char* bb)
{
    return strcmp(aa, bb) == 0;
}

static file_data*
get_file_data(const char* path) {
    for (int ii = 0; 1; ++ii) {
        file_data row = file_table[ii];

        if (file_table[ii].path == 0) {
            break;
        }

        if (streq(path, file_table[ii].path)) {
            return &(file_table[ii]);
        }
    }

    return 0;
}

int
get_stat(const char* path, struct stat* st)
{
    file_data* dat = get_file_data(path);
    if (!dat) {
        return -1;
    }

    memset(st, 0, sizeof(struct stat));
    st->st_uid  = getuid();
    st->st_mode = dat->mode;
    if (dat->data) {
        st->st_size = strlen(dat->data);
    }
    else {
        st->st_size = 0;
    }
    return 0;
}

const char*
get_data(const char* path)
{
    file_data* dat = get_file_data(path);
    if (!dat) {
        return 0;
    }

    return dat->data;
}
