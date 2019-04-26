//
//  link_unlink.c
//  
//
//  Created by Albert Wu on 4/5/19.
//

#include <stdio.h>
int link_file(){

  //find old file
  MINODE *omip = getmino(pathname);
  if(!omip) {
    return -1;
  }
  if(!omip->ino) {
    printf("%s not found\n", pathname);
    return -1;
  }

  if (!(S_ISREG(omip->INODE.i_mode) || S_ISLNK(omip->INODE.i_mode))) {
    printf("link: %s is dir, cannot link to dir\n", pathname);
    return -1;
  }

  //find new file
  MINODE *dmip;
  INODE *dip;
  char *dir = dirname(strdup(pathname2));
  char *base = basename(strdup(pathname2));
  printf("linking new file %s %s\n", dir, base);
  dmip = getmino(dir);
  if(!dmip) {
    iput(omip);
    return -1;
  }
  if(!dmip->ino) {
    printf("%s not found\n", dirname);
    return -1;
  }

  if (!S_ISDIR(dmip->INODE.i_mode)) {
    printf("%s not a dir\n", dir);
    return -1;
  }
  if (search(dmip, base) != 0) {
    printf("file already exists\n");
    return -1;
  }
  enter_name(dmip, omip->ino, base);
  struct timeval tv;
  gettimeofday(&tv, NULL);
  dmip->INODE.i_atime = tv.tv_sec;
  dmip->dirty = 1;

  omip->INODE.i_links_count += 1;
  omip->dirty=1;
  iput(omip);

  iput(dmip);
}

int unlink_file() {
  MINODE *mip = getmino(pathname);
  if(!mip->ino) {
    printf("%s not found\n", pathname);
    return -1;
  }
  if (!(S_ISREG(mip->INODE.i_mode) || S_ISLNK(mip->INODE.i_mode))) {
    printf("unlink: %s is dir\n", pathname);
    return -1;
  }

  mip->INODE.i_links_count -= 1;
  if(!mip->INODE.i_links_count) {
    if (S_ISLNK(mip->INODE.i_mode))
      memset(mip->INODE.i_block, 0, sizeof(mip->INODE.i_block));
    else
      printf("unlink deleting file\n");
      truncate(mip);
    idealloc(mip->dev,mip->ino);
  }

  MINODE *dmip = getmino(dirname(strdup(pathname)));

  rm_child(dmip, basename(strdup(pathname)));

  mip->dirty = 1;
  iput(mip);

  dmip->dirty=1;
  iput(dmip);
}

int symlink_file() {
  //find old file
  MINODE *omip = getmino(pathname);
  if(!omip) {
    return -1;
  }
  if(!omip->ino) {
    printf("%s not found\n", pathname);
    return -1;
  }
  if (!(S_ISREG(omip->INODE.i_mode) || S_ISLNK(omip->INODE.i_mode))) {
    printf("link: %s is dir, cannot link to dir\n", pathname);
    return -1;
  }

  //find new file
  MINODE *dmip;
  INODE *dip;
  char *dir = dirname(strdup(pathname2));
  char *base = basename(strdup(pathname2));
  printf("linking new file %s %s\n", dir, base);
  dmip = getmino(dir);
  if(!dmip) {
    iput(omip);
    return -1;
  }
  if(!dmip->ino) {
    printf("%s not found\n", dirname);
    return -1;
  }

  if (!S_ISDIR(dmip->INODE.i_mode)) {
    printf("%s not a dir\n", dir);
    return -1;
  }
  if (search(dmip, base) != 0) {
    printf("file already exists\n");
    return -1;
  }
  int ino = ialloc(dmip->dev);
  printf("symlink: new ino: %d\n", ino);
  MINODE *mip = iget(dmip->dev, ino);
  INODE *ip = &mip->INODE;

  ip->i_mode = 0120000;    
  ip->i_uid  = running->uid;  // Owner uid

  ip->i_links_count = 1;
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
  ip->i_blocks = 2;                  // LINUX: Blocks count in 512-byte chunks
  ip->i_size = strlen(pathname) + 1;    // Size in bytes
  strcpy(ip->i_block, pathname);
  mip->dirty = 1;               // mark minode dirty
  iput(mip);                    // write INODE to disk

  enter_name(dmip, ino, base);
  struct timeval tv;
  gettimeofday(&tv, NULL);
  dmip->INODE.i_atime = tv.tv_sec;
  dmip->dirty = 1;

  iput(dmip);
}

char* readlink_file() {
  MINODE *mip = getmino(pathname);
  if (!S_ISLNK(mip->INODE.i_mode)) {
    printf("%s not a link\n", pathname);
    return;
  }
  printf("readlink returning %s\n", mip->INODE.i_block);
  return mip->INODE.i_block;
}
