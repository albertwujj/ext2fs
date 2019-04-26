//
//  open_close_lseek.c
//  
//
//  Created by Albert Wu on 4/15/19.
//

#include <stdio.h>


char *mode_name[] = {"READ", "WRITE","READ/WRITE","APPEND"};

int open_file() {
  int mode;
  sscanf(pathname2, "%d", &mode);
  return myopen(pathname, mode);
}
int myopen(char *pathname, int mode) {

  MINODE *mip = getmino(pathname);
  if(!mip) {
    return -1;
  }
  int dev = mip->dev;
  if(!mip) {
    return -1;
  }
  if (!S_ISREG(mip->INODE.i_mode)) {
    printf("not a regular file\n");
    return -1;
  }

  if (!get_permissions(running, mip, mode)) {
    printf("do not have permission\n");
    return -1;
  }
  int i = 0;
  OFT *oftp = NULL;
  for(i;i<NFD;i++) {
    if (running->fd[i] == NULL) {
      oftp = malloc(sizeof(OFT));
      running->fd[i] = oftp;
      break;
    }
    if(running->fd[i]->mptr == mip) {

      printf("%d %p file %s already opened %d \n", i, mip, pathname, mip->ino);
      return -1;
    }
  }
  if (i==NFD) {
    printf("too many files open\n");
    return -1;
  }

  switch(mode){
   case 0 : oftp->offset = 0;     // R: offset = 0
            break;
   case 1 : truncate(mip);        // W: truncate file to 0 size
            oftp->offset = 0;
            break;
   case 2 : oftp->offset = 0;     // RW: do NOT truncate file
            break;
   case 3 : oftp->offset =  mip->INODE.i_size;  // APPEND mode
            break;
   default: printf("invalid mode\n");
            return(-1);
  }
  oftp->mode = mode;
  oftp->refCount = 1;
  if(mode == 0) {
    mip->INODE.i_atime= time(0L);
  } else {
    mip->INODE.i_atime=mip->INODE.i_mtime=time(0L);
  }
  oftp->mptr = mip;

  return i;
}

int close_file() {
  int fd = 0;
  sscanf(pathname, "%d", &fd);
  return myclose(fd);
}
int myclose(int fd) {
  if(!verify_fd(fd))
    return -1;
  OFT *oftp = running->fd[fd];
  running->fd[fd] = 0;
  oftp->refCount--;
  if (oftp->refCount > 0)
    return 0;

  MINODE *mip = oftp->mptr;
  //free(oftp);
  mip->dirty=1;
  iput(mip);
  return 0;
}


int mylseek(int fd, int position) {
  if(!verify_fd(fd))
    return -1;
  OFT *oftp = running->fd[fd];
  if (oftp->mptr->INODE.i_size) {
    printf("pos greater than filesize\n");
    return -1;
  }
  int old = oftp->offset;
  oftp->offset = position;
  return old;
}

/*
 fd     mode    offset    INODE
----    ----    ------   --------
 0     READ      1234     [dev, ino]
 1     WRITE      0   [dev, ino]
--------------------------------------
*/
int pfd()
{
  int i = 0;
  printf(" fd     mode    offset    INODE\n");
  printf("----    ----    ------   --------\n");
  char *spaces = "     ";
  for(i; i<NFD && running->fd[i] != 0;i++) {
    printf(" %d%s%s%d%s[%d,%d]\n",i,spaces,mode_name,spaces,running->fd[i]->offset,spaces,running->fd[i]->mptr->dev,running->fd[i]->mptr->ino);
  }
}

int dup_fd() {
  int fd = 0;
  sscanf(pathname, "%d", &fd);
  if(!verify_fd(fd))
    return -1;
  int i = 0;
  for(i;i<NFD;i++) {
    if (running->fd[i] == 0) {
      running->fd[i] = running->fd[fd];
      running->fd[i]->refCount++;
      return 0;
    }
  }
  if (i==NFD) {
    printf("too many files open\n");
    return 1;
  }

}

int dup_fd2()
{
  printf("dup 2ing\n");
  int fd = 0, gd=0;
  sscanf(pathname, "%d", &fd);
  sscanf(pathname2, "%d", &gd);
  if(!verify_fd(fd))
    return -1;
  if (fd >= NFD) {
    printf("fd out of range\n");
    return;
  }
  if(!running->fd[fd]) {
    printf("fd %d not open\n", running->fd[fd]);
    return;
  }
  if(running->fd[gd]) {
    close_file(gd);
  }
  running->fd[gd] = running->fd[fd];
  running->fd[gd]->refCount++;
}
