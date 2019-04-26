//
//  read_cat.c
//  
//
//  Created by Albert Wu on 4/16/19.
//

#include <stdio.h>

int read_file() {
  strcpy(pathname2, "0");
  int fd = open_file();
  char sbuf[BLKSIZE];
  myread(fd, sbuf, BLKSIZE);
}
int myread(int fd, char *buf, int nbytes){
  char sbuf[BLKSIZE];

  OFT *oftp = running->fd[fd];
  if(!oftp || (oftp->mode != 0 && oftp->mode != 2)) {

    printf("file not opened for read\n");
    return 0;
  }

  int offset=oftp->offset;

  MINODE *mip = oftp->mptr;

  int avil = mip->INODE.i_size - oftp->offset;

  int count = 0;
  char *cq = buf;
  while(nbytes > 0 && avil > 0) {
    int lbk = oftp->offset/BLKSIZE;
    avil = mip->INODE.i_size - oftp->offset;
    int index = oftp->offset%BLKSIZE;
    
    int bno = lbktobno(mip, lbk, 0);
    if(!bno) {
      break;
    }
    get_block(mip->dev, bno, sbuf);
    char *cp = sbuf + index;
    int remain = BLKSIZE - index;
    int max_lim = avil;
    if(remain < avil)
      max_lim = remain;
    if(nbytes < max_lim)
      max_lim = nbytes;
    memcpy(cq, cp, max_lim);
    cq += max_lim; count += max_lim; oftp->offset += max_lim; index += max_lim; nbytes -= max_lim;
  }

  return count;
}

int cat_file() {
  char buf[BLKSIZE], dummy = 0;
  int n;

  strcpy(pathname2, "0");

  int fd = open_file();

  if(fd < 0) {
    return;
  }
  while(n = myread(fd, buf, BLKSIZE)){
   buf[n] = 0;
   printf("%s",buf);
  }

  close_file(fd);
}
