
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
static size_t INODE_SIZE = 256;
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

// get an inode pointer for the given inode number
static inode*
get_inode(int inodeNum) {
    // set a pointer to the beginning of the inode section
    inode* inode = (mem_base + BLOCK_SIZE);
    // move the pointer to the specific inode in memory
    inode = inode + (inodeNum * sizeof(INODE_SIZE));
    return inode;
}

// update the data associated with an inode
void
update_inode(int inodeNum, void* block, int ftype) {
    inode* inode = get_inode(inodeNum);

    inode->block = block;
    inode->refCount += 1;
    inode->fileType = ftype;
}


// initialize all data
void
init(void* memory) {
    mem_base = memory;
    void* superBlock=memory;

    // num of inodes
    int numInodes = 240;
    // init super block
    int numBlock = 238;
    // pointer to inodeStartAddr
    void* inodeStart = memory + 3 * BLOCK_SIZE;

    // store the data in the superBlock
    *((int*)superBlock) = numInodes;
    superBlock = superBlock + sizeof(int*);

    *((int*)superBlock) = numBlock;
    superBlock = superBlock + sizeof(int*);

    superBlock = inodeStart;
    superBlock = superBlock + sizeof(void*);

    printf("got heeeeere\n");


    // initialize the bitmaps
    superBlock = memory + BLOCK_SIZE;
    // initialize inodes bitmap
    for(int i=0; i<240; i++) {
        superBlock = superBlock + (i * sizeof(int16_t));
        *((int16_t*)superBlock) = 0;
    }

    // initialize dataBlocks bitmap
    superBlock = memory + (2*BLOCK_SIZE);
    for(int i=0; i<238; i++) {
        superBlock = superBlock + (i * sizeof(int16_t));
        *((int16_t*)superBlock) = 0;
    }


    printf("1.11111\n");

    // init root directory
    superBlock = memory + (19*BLOCK_SIZE) - sizeof(dirList);

    dirList* temp = (dirList*)superBlock;
    dirData tempData = temp->data;
    
    printf("1.1555\n");

    // set first directory
    temp->next = NULL;
    printf("assigning to null is fine\n");
    tempData.inum = 2;
    printf("there is nothing for this pointer\n");
    tempData.path = "/";
    tempData.name = ".";



    printf("1.23242\n");    



    // move pointer to set second directory
    temp = temp - sizeof(dirList);
    tempData = temp->data;

    temp->next = temp + sizeof(dirList);
    tempData.inum = 2;
    tempData.path = "/";
    tempData.name = "..";


    printf("1.343433\n");

    update_inode(2, (void*)temp, 0);
}

void
storage_init(const char* path)
{
    //printf("TODO: Store file system data in: %s\n", path);
    //TODO: O_CREAT will need to be changed

    mem_fd = open(path, O_CREAT|O_RDWR, 0644);
    assert(mem_fd!=-1);
    
    int res = ftruncate(mem_fd, MEM_SIZE);
    assert(res==0);
    
    mem_base = mmap(NULL, MEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, 0);
    assert(mem_base!=MAP_FAILED);

    init(mem_base);
}

dirList*
get_dir_list(const char* path) {
    inode* inode = get_inode(2);
    return (dirList*)(inode->block);
}

// find a free inode index
int  
storage_has_inode() {
    // a pointer to the first integer in the inodeBitMap section
    int* nodeBit = (int*)(mem_base + BLOCK_SIZE);

    for(int i=3; i<239; i++) {
        nodeBit = nodeBit + (i * INODE_SIZE);
        if(*nodeBit == 0) {
            *nodeBit = 1;
            return i;
        }
    }
}

// compares two strings
static int
streq(const char* aa, const char* bb)
{
    return strcmp(aa, bb) == 0;
}

// gets the string data block for a file with inode inodenum
static char*
get_data_inodenum(int inodenum) {
    inode* inode = get_inode(inodenum);
    return (char*)(inode->block);
}

// gets the string data block for a file from the given path
static char*
get_file_data(const char* path) {

    // go to root directory's inode
    inode* inode = get_inode(2);
    printf("1.1\n");
    // go to root directory
    dirList* dir = (dirList*)(inode->block);
printf("1.2\n");
    dirData temp;
printf("1.3\n");

    temp = dir->data;
    printf("assigned data to temp\n");
    /*dirData* temp2;
    *temp2=temp;

    if(!dir) {
        printf("dir list pointer is NULL\n");
    }
    else {
        if(!temp2) {
            printf("dir data is null\n");
        }
    }*/
    
    // traverse the root directory
    for(int i=0; 1; i++) {
        printf("got in the loop\n");
        temp = dir->data;
        printf("assigned data to temp\n");
        // if given path and the path are the same, get the data
        printf("given path is %s\n)", *path);
        //printf("stored path is %s\n", *(temp.path));        
/*if(strcmp(path, temp.path)==0) {
            printf("1.4\n");
           return get_data_inodenum(temp.inum); 
        }*/

        printf("passed the if\n");
        
        // if there are no more entries in the directory, break
        if(dir->next) {
            printf("going for another loop\n");
            dir = dir->next;
        }
        else {
            printf("need to break\n");
            break;
        }
    }

    printf("returning 0\n");
    return 0;
}

// gets the data for the file pointed to by the path path
const char*
get_data(const char* path)
{
    char* data = get_file_data(path);
    if (!data) {
        return 0;
    }

    return data;
}




// get stat about a file
int
storage_get_stat(const char* path, struct stat* st)
{
    printf("came to st get stat\n");
    char* dat = get_file_data(path);
    if (!dat) {
        return -1;
    }
        printf("1\n");
    memset(st, 0, sizeof(struct stat));
    //st->st_uid  = getuid();
    //st->st_mode = dat->mode;
    st->st_uid = 1;
    st->st_mode = 040755;
    printf("2\n");
    /*if (dat->data) {
        st->st_size = strlen(dat->data);
    }
    else {
        st->st_size = 0;
    }*/
    st->st_size = strlen(dat);
    printf("3\n");
    return 0;
}
