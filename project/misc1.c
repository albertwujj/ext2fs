//
//  misc1.c
//  
//
//  Created by Albert Wu on 4/7/19.
//

#include <stdio.h>



int stat_file() {
  struct stat st;
  int ino = getino(pathname);
  MINODE *mip = iget(dev, ino);
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
  int ino = getino(pathname2);
  MINODE *mip = iget(dev, ino);
  int mode = 0;
  sscanf(pathname, "%d", &mode);
  printf("%d\n", mode);
  mip->INODE.i_mode |= mode;
  mip->dirty = 1;
  iput(mip);
}
