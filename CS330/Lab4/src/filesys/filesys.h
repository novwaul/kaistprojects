#ifndef FILESYS_FILESYS_H
#define FILESYS_FILESYS_H

#include <stdbool.h>
#include "filesys/off_t.h"

/* Sectors of system file inodes. */
#define FREE_MAP_SECTOR 0       /* Free map file inode sector. */
#define ROOT_DIR_SECTOR 1       /* Root directory file inode sector. */

/* Disk used for file system. */
extern struct disk *filesys_disk;

void filesys_init (bool format);
void filesys_done (void);
bool filesys_create (const char *path, off_t initial_size);
struct file *filesys_open (const char *path);
bool filesys_remove (const char *path);
bool filesys_mkdir (const char *path);
bool filesys_chdir (const char *path);

#endif /* filesys/filesys.h */
