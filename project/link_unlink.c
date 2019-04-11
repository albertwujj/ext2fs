//
//  link_unlink.c
//  
//
//  Created by Albert Wu on 4/5/19.
//

#include <stdio.h>
int link_file(){

  //find old file
  int ino=getino(pathname);
  if(!ino) {
    printf("%s not found\n", pathname);
    return;
  }
  MINODE *omip = iget(dev, ino);
  if (!(S_ISREG(omip->INODE.i_mode) || S_ISLNK(omip->INODE.i_mode))) {
    printf("link: %s is dir, cannot link to dir\n", pathname);
    return;
  }

  //find new file
  MINODE *dmip;
  INODE *dip;
  char *dir = dirname(strdup(pathname2));
  char *base = basename(strdup(pathname2));
  printf("linking new file %s %s\n", dir, base);
  int dino = getino(dir);
  if(!dino) {
    printf("%s not found\n", dirname);
    return;
  }



  dmip = iget(dev, dino);
  if (!S_ISDIR(dmip->INODE.i_mode)) {
    printf("%s not a dir\n", dir);
    return;
  }
  if (search(dmip, base) != 0) {
    printf("file already exists\n");
    return;
  }
  enter_name(dmip, ino, base);
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
  int ino = getino(pathname);
  if(!ino) {
    printf("%s not found\n", pathname);
    return;
  }
  MINODE *mip = iget(dev, ino);
  if (!(S_ISREG(mip->INODE.i_mode) || S_ISLNK(mip->INODE.i_mode))) {
    printf("unlink: %s is dir\n", pathname);
    return;
  }
  mip->INODE.i_links_count -= 1;
  if(!mip->INODE.i_links_count) {
    truncate(mip);
    idealloc(mip->dev,mip->ino);
  }

  int dino = getino(dirname(strdup(pathname)));
  MINODE *dmip = iget(dev, dino);
  rm_child(dmip, basename(strdup(pathname)));

  mip->dirty = 1;
  iput(mip);

  dmip->dirty=1;
  iput(dmip);
}

int symlink_file() {
  //find old file
  int oino=getino(pathname);
  if(!oino) {
    printf("%s not found\n", pathname);
    return;
  }
  MINODE *omip = iget(dev, oino);
  if (!(S_ISREG(omip->INODE.i_mode) || S_ISLNK(omip->INODE.i_mode))) {
    printf("link: %s is dir, cannot link to dir\n", pathname);
    return;
  }

  //find new file
  MINODE *dmip;
  INODE *dip;
  char *dir = dirname(strdup(pathname2));
  char *base = basename(strdup(pathname2));
  printf("linking new file %s %s\n", dir, base);
  int dino = getino(dir);
  if(!dino) {
    printf("%s not found\n", dirname);
    return;
  }

  dmip = iget(dev, dino);
  if (!S_ISDIR(dmip->INODE.i_mode)) {
    printf("%s not a dir\n", dir);
    return;
  }
  if (search(dmip, base) != 0) {
    printf("file already exists\n");
    return;
  }
  int ino = ialloc(dmip->dev);
  printf("symlink: new ino: %d\n");
  MINODE *mip = iget(dev, ino);
  INODE *ip = &mip->INODE;

  ip->i_mode = 0120000;    // OR 040755: DIR type and permissions
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
  int ino = getino(pathname);
  MINODE *mip = iget(dev, ino);
  if (!S_ISLNK(mip->INODE.i_mode)) {
    printf("%s not a link\n", pathname);
    return;
  }
  printf("readlink returning %s\n", mip->INODE.i_block);
  return mip->INODE.i_block;
}
