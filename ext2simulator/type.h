#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/ext2_fs.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>
// define shorter TYPES for convenience
typedef struct ext2_group_desc GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode INODE;
typedef struct ext2_dir_entry_2 DIR;
#define BLKSIZE 1024
// Block number of EXT2 FS on FD
#define SUPERBLOCK 1
#define GDBLOCK 2
#define ROOT_INODE 2
// Default dir and regular file modes
#define DIR_MODE 0x41ED
#define FILE_MODE 0x81AE
#define SUPER_MAGIC 0xEF53
#define SUPER_USER 0
// Proc status
#define FREE 0
#define BUSY 1
// file system table sizes
#define NMINODE 100
#define NMTABLE 10
#define NPROC 2
#define NFD 10
#define NOFT 40
// Open File Table
typedef struct oft{
int mode;
int refCount;
struct minode *minodePtr;
int offset;
}OFT;
// PROC structure
typedef struct proc{
struct Proc *next;
int pid;
int uid;
int gid;
320 11 EXT2 File System
int ppid;
int status;
struct minode *cwd;
OFT *fd[NFD];
}PROC;
// In-memory inodes structure
typedef struct minode{
INODE INODE; // disk inode
int dev, ino;
int refCount; // use count
int dirty; // modified flag
int mounted; // mounted flag
struct mount *mntPtr; // mount table pointer
// int lock; // ignored for simple FS
}MINODE;
// Open file Table // opened file instance
typedef struct oft{
int mode; // mode of opened file
int refCount; // number of PROCs sharing this instance
MINODE *minodePtr; // pointer to minode of file
int offset; // byte offset for R|W
}OFT;
// Mount Table structure
typedef struct mtable{
int dev; // device number; 0 for FREE
int ninodes; // from superblock
int nblocks;
int free_blocks // from superblock and GD
int free_inodes
int bmap; // from group descriptor
int imap;
int iblock; // inodes start block
MINODE *mntDirPtr; // mount point DIR pointer
char devName[64]; //device name
char mntName[64]; // mount point DIR name
}MTABLE;
