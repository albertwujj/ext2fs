//
//  util.c
//  
//
//  Created by Albert Wu on 3/7/19.
//

#include <stdio.h>

int get_block(int dev, int blk, char *buf)
{
  lseek(dev, blk*BLKSIZE, SEEK_SET);
  int n = read(dev, buf, BLKSIZE);
  if (n<0) printf(“get_block [%d %d] error\n”, dev, blk);
}
int put_block(int dev, int blk, char *buf)
{
  lseek(dev, blk*BLKSIZE, SEEK_SET);
  int n = write(dev, buf, BLKSIZE);
  if (n != BLKSIZE)
    printf(“put_block [%d %d] error\n”, dev, blk);
}

MINODE *iget(int dev, int ino)
{
  MINODE *mip;
  MTABLE *mp;
  INODE *ip;
  int i, block, offset;
  char buf[BLKSIZE];
  // serach in-memory minodes first
  for (i=0; i<NMINODES; i++){
    MINODE *mip = &MINODE[i];
    if (mip->refCount && (mip->dev==dev) && (mip->ino==ino)){
      mip->refCount++;
      return mip;
    }
  }
  // needed INODE=(dev,ino) not in memory
  mip = mialloc(); // allocate a FREE minode
  mip->dev = dev; mip->ino = ino; // assign to (dev, ino)
  block = (ino-1)/8 + iblock; // disk block containing this inode
  offset= (ino-1)%8; // which inode in this block
  get_block(dev, block, buf);
  ip = (INODE *)buf + offset;
  mip->INODE = *ip; // copy inode to minode.INODE
  // initialize minode
  mip->refCount = 1;
  mip->mounted = 0;
  mip->dirty = 0;
  mip->mountptr = 0;
  return mip;
}

int iput(MINODE *mip)
{
  INODE *ip;
  int i, block, offset;
  char buf[BLKSIZE];
  if (mip==0) return;
  mip->refCount--; // dec refCount by 1
  if (mip->refCount > 0) return; // still has user
  if (mip->dirty == 0) return; // no need to write back
  // write INODE back to disk
  block = (mip->ino - 1) / 8 + iblock;
  offset = (mip->ino - 1) % 8;
  // get block containing this inode
  get_block(mip->dev, block, buf);
  ip = (INODE *)buf + offset; // ip points at INODE
  *ip = mip->INODE; // copy INODE to inode in block
  put_block(mip->dev, block, buf); // write back to disk
  midalloc(mip); // mip->refCount = 0;
}

char *name[64]; // token string pointers
char gline[256]; // holds token strings, each pointed by a name[i]
int nname; // number of token strings
int tokenize(char *pathname)
{
  char *s;
  strcpy(gline, pathname);
  nname = 0;
  s = strtok(gline, "/");
  while(s){
    name[nname++] = s;
    s = strtok(0, "/");
  }
}
int search(MINODE *mip, char *name)
{
  int i;
  char *cp, temp[256], sbuf[BLKSIZE];
  DIR *dp;
  for (i=0; i<12; i++){ // search DIR direct blocks only
  if (mip->INODE.i_block[i] == 0)
    return 0;
  get_block(mip->dev, mip->INODE.i_block[i], sbuf);
  dp = (DIR *)sbuf;
  cp = sbuf;
  while (cp < sbuf + BLKSIZE){
    strncpy(temp, dp->name, dp->name_len);
    temp[dp->name_len] = 0;
    printf("%8d%8d%8u %s\n",
    dp->inode, dp->rec_len, dp->name_len, temp);
    if (strcmp(name, temp)==0){
      printf(“found %s : inumber = %d\n", name, dp->inode);
      return dp->inode;
    }
    cp += dp->rec_len;
    dp = (DIR *)cp;
    }
  }
  return 0;
}

int getino(char *pathname)
{
  MINODE *mip;
  int i, ino;
  if (strcmp(pathname, "/")==0){
    return 2; // return root ino=2
  }
  if (pathname[0] == ’/’)
    mip = root; // if absolute pathname: start from root
  else
    mip = running->cwd; // if relative pathname: start from CWD

  mip->refCount++; // in order to iput(mip) later
  tokenize(pathname); // assume: name[ ], nname are globals
  for (i=0; i<nname; i++){ // search for each component string
  if (!S_ISDIR(mip->INODE.i_mode)){ // check DIR type
    printf("%s is not a directory\n", name[i]);
    iput(mip);
    return 0;
  }
  ino = search(mip, name[i]);
  if (!ino){
    printf("no such component name %s\n", name[i]);
    iput(mip);
    return 0;
  }
  iput(mip); // release current minode
  mip = iget(dev, ino); // switch to new minode
  }
  iput(mip);
  return ino;
}
