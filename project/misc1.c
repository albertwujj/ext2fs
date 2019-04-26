//
//  misc1.c
//  
//
//  Created by Albert Wu on 4/7/19.
//

#include <stdio.h>



int stat_file() {
  struct stat stv;
  struct stat *st = &stv;
  MINODE *mip = getmino(pathname);
  if(!mip){
    return -1;
  }
  st->st_dev = mip->dev;
  st->st_ino = mip->ino;
  st->st_nlink = mip->INODE.i_links_count;
  st->st_uid = mip->INODE.i_uid;
  st->st_size = mip->INODE.i_size;
  st->st_blksize = BLKSIZE;
  st->st_blocks = mip->INODE.i_blocks;
  iput(mip);
}

int chmod_file() {
  MINODE *mip = getmino(pathname2);
  if(!mip){
    return -1;
  }
  int mode = 0;
  sscanf(pathname, "%o", &mode);
  printf("%d\n", mode);
  mip->INODE.i_mode |= mode;
  mip->dirty = 1;
  iput(mip);
}

int utime_file() {
  MINODE *mip = getmino(pathname);
  if(!mip){
    return -1;
  }
  INODE *ip = &mip->INODE;
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
  mip->dirty = 1;
  iput(mip);
}
