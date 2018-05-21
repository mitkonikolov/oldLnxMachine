#ifndef NUFS_STORAGE_H
#define NUFS_STORAGE_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

void storage_init(const char* path);
int         get_stat(const char* path, struct stat* st);
const char* get_data(const char* path);

int   storage_has_inode();
int   storage_has_blocks(int num_blocks);
void* storage_get_block(const char* path);
int   storage_get_stat(const char* path, struct stat* st);

#endif
