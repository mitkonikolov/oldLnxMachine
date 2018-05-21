#ifndef NUFS_STORAGE_H
#define NUFS_STORAGE_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct dirData {
    int inum;   // inode number associated with the file
    char* path; // path to the file
    char* name; // name of the file
} dirData;
 
typedef struct dirList {
    dirData data; // data for an entry in the dir file
    struct dirList* next; // pointer to the next entry in the dir file
} dirList;

typedef struct inode {
    void* block; // data for each file
    int refCount; // how many files have this inode
    int fileType; // 0 - dir, 1 - file
} inode;

void storage_init(const char* path);
int         get_stat(const char* path, struct stat* st); // directory
const char* get_data(const char* path);
dirList*    get_dir_list(const char* path);

int   storage_has_inode();
//int   storage_has_blocks(int num_blocks);
void* storage_get_block(const char* path);
int   storage_get_stat(const char* path, struct stat* st); // file stat

#endif
