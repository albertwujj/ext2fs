
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <sys/stat.h>
#include <ext2fs/ext2_fs.h>

#include "type.h"

MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;

MNT mnttable[NMNT];


char   gpath[256]; // global for tokenized components
char   *name[64];  // assume at most 64 components in pathname
int    n;          // number of component strings

int    nblocks, ninodes, bmap, imap, inode_start;
char   line[256], cmd[32], pathname[256], pathname2[256];

#define OWNER  000700
#define GROUP  000070
#define OTHER  000007

#include "util.c"
#include "cd_ls_pwd.c"
#include "mkdir_creat.c"
#include "rmdir.c"
#include "link_unlink.c"
#include "misc1.c"
#include "open_close_lseek.c"
#include "read_cat.c"
#include "write_cp_mv.c"
#include "mount_umount.c"

int init()
{
  int i, j;
  MINODE *mip;
  PROC   *p;

  printf("init()\n");

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->refCount = 0;
    mip->mounted = 0;
    mip->mptr = 0;
  }
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->uid = 0;
    p->cwd = 0;
    p->status = FREE;
    for (j=0; j<NFD; j++)
      p->fd[j] = 0;
  }
}

// load root INODE and set root pointer to it
int mount_root(int fd)
{  
  printf("mount_root()\n");
  root = iget(fd, 2);
}

char *disk = "mydisk";
int main(int argc, char *argv[ ])
{
  int fd;
  int ino;
  char buf[BLKSIZE];
  if (argc > 1)
    disk = argv[1];

  printf("checking EXT2 FS ....");
  if ((fd = open(disk, O_RDWR)) < 0){
    printf("open %s failed\n", disk);  exit(1);
  }

  /********** read super block at 1024 ****************/
  get_block(fd, 1, buf);
  sp = (SUPER *)buf;

  /* verify it's an ext2 file system *****************/
  if (sp->s_magic != 0xEF53){
      printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
      exit(1);
  }     
  printf("OK\n");
  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;

  get_block(fd, 2, buf);
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  inode_start = gp->bg_inode_table;
  printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, inode_start);

  init();  
  mount_root(fd);

  printf("root refCount = %d\n", root->refCount);
  
  printf("creating P0 as running process\n");
  running = &proc[0];
  running->status = READY;
  running->cwd = iget(fd, 2);
  
  printf("root refCount = %d\n", root->refCount);

  printf("start tests\n");


  //printf("hit a key to continue : "); getchar();
  while(1){
    printf("input command : [ls|cd|pwd|mkdir|creat|rmdir|link|unlink|symlink|readlink|stat|chmod|utime|open|read|cat|cp|mv|mount|umount|quit] ");
    fgets(line, 128, stdin);
    line[strlen(line)-1] = 0;
    if (line[0]==0)
      continue;
    pathname[0] = 0;
    cmd[0] = 0;
    
    sscanf(line, "%s %s %s", cmd, pathname, pathname2);
    printf("cmd=%s pathname=%s\n", cmd, pathname);

    if (strcmp(cmd, "ls")==0)
       list_file();
    if (strcmp(cmd, "cd")==0)
       change_dir();
    if (strcmp(cmd, "pwd")==0)
       pwd();

    if (strcmp(cmd, "mkdir")==0)
      make_dir();
    if (strcmp(cmd, "creat")==0)
      creat_file();
    if (strcmp(cmd, "rmdir")==0)
      remove_dir();
    if (strcmp(cmd, "link")==0)
      link_file();
    if (strcmp(cmd, "unlink")==0)
      unlink_file();
    if (strcmp(cmd, "symlink")==0)
      symlink_file();
    if (strcmp(cmd, "readlink")==0)
      readlink_file();
    if (strcmp(cmd, "stat")==0)
      stat_file();
    if (strcmp(cmd, "chmod")==0)
      chmod_file();
    if (strcmp(cmd, "utime")==0)
      utime_file();
    if (strcmp(cmd, "pfd")==0)
      pfd();
    if (strcmp(cmd, "dup")==0)
      dup_fd();
    if (strcmp(cmd, "dup2")==0)
      dup_fd2();
    if (strcmp(cmd, "read")==0)
      read_file();
    if (strcmp(cmd, "cat")==0)
      cat_file();
    if (strcmp(cmd, "cp")==0)
      cp_file();
    if (strcmp(cmd, "mv")==0)
      mv_file();
    if (strcmp(cmd, "mount")==0) {
      if(strlen(pathname)==0) {
        show_mount();
      } else {
        mount_dev();
      }
    }
     if (strcmp(cmd, "umount")==0)
      umount_dev();
    if (strcmp(cmd, "quit")==0)
       quit();
  }
}


int quit()
{
  int i;
  MINODE *mip;
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount > 0)
      iput(mip);
  }
  exit(0);
}
