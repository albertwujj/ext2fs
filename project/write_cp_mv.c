



int mywrite(int fd, char buf[ ], int nbytes)
{
  OFT *oftp =running->fd[fd];
  if(!oftp || (oftp->mode!=0 && oftp->mode!=1)) {
    printf("file not opened for write\n");
    return;
  }
  MINODE *mip = oftp->mptr;
  char *cp = buf;
  int count = 0;
  while(nbytes > 0) {
    int lbk = oftp->offset/BLKSIZE;
    int index = oftp->offset%BLKSIZE;
    int bno = lbktobno(mip, lbk, 1);
    char sbuf[BLKSIZE];
    get_block(mip->dev, bno, sbuf);
    char *cq = sbuf + index;
    int remain = BLKSIZE - index;
    int max_lim = remain;
    if(nbytes < max_lim)
      max_lim = nbytes;
    memcpy(cq, cp, max_lim);
    put_block(mip->dev, bno, sbuf);
    cp += max_lim; count += max_lim; oftp->offset += max_lim; index += max_lim; nbytes -= max_lim;
  }

  mip->INODE.i_size += count;
  mip->dirty = 1;       // mark mip dirty for iput()
  mip->refCount++;
  iput(mip);
  printf("mywrite: wrote %d char into file descriptor %d\n", count, fd);
  return count;
}

int cp_file() {
  char buf[BLKSIZE], dummy = 0;
  int n;
  int fd = myopen(pathname, 0);

  if(fd < 0) {
    myclose(fd);
    return;
  }
  my_creat_file(pathname2);
  int dfd = myopen(pathname2, 1);

  if(dfd < 0) {
    myclose(dfd);
    return;
  }
  while(n = myread(fd, buf, BLKSIZE)){
     mywrite(dfd, buf, n);
  }

  myclose(fd);
  myclose(dfd);
}

int mv_file() {
  int dev;
  if (pathname[0]=='/') {
    dev = root->dev;
  } else {
    dev = running->cwd->dev;
  }

  MINODE *mip = getmino(pathname);
  if(!mip) {
    return -1;
  }
  if (!S_ISREG(mip->INODE.i_mode)) {
    printf("not a regular file\n");
    return -1;
  }
  MINODE *dmip = getmino(strdup(dirname(pathname2)));
  if(!dmip) {
    iput(mip);
    return -1;
  }
  if(dev == dmip->dev) {
    link_file();
    unlink_file();
  } else {
    cp_file();
    unlink_file();
  }
}
