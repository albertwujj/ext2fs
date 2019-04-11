/*********** util.c file ****************/

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern char   gpath[256];
extern char   *name[64];
extern int    n;
extern int    fd, dev;
extern int    nblocks, ninodes, bmap, imap, inode_start;
extern char   line[256], cmd[32], pathname[256];

int get_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   read(dev, buf, BLKSIZE);
}   

int put_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   write(dev, buf, BLKSIZE);
}   

int tokenize(char *pathname)
{
  char sname[256];
  strcpy(sname, pathname);
  char *tok = strtok(sname, "/");
  strcpy(gpath, tok);
  name[0] = gpath;
  int i = 1;
  for (i; tok = strtok(NULL, "/"); i++) {
    name[i] = name[i - 1] + strlen(name[i-1]) + 1;
    strcpy(name[i], tok);
  }
  name[i] = NULL;
  *(&n) = i;
}

// return minode pointer to loaded INODE
MINODE *iget(int dev, int ino)
{
  int i;
  MINODE *mip;
  char buf[BLKSIZE];
  int blk, disp;
  INODE *ip;

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->dev == dev && mip->ino == ino){
       mip->refCount++;
       printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
       return mip;
    }
  }
    
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount == 0){
      //printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
       mip->refCount = 1;
       mip->dev = dev;
       mip->ino = ino;

       // get INODE of ino to buf    
       blk  = (ino-1) / 8 + inode_start;
       disp = (ino-1) % 8;

       //printf("iget: ino=%d blk=%d disp=%d\n", ino, blk, disp);

       get_block(dev, blk, buf);
       ip = (INODE *)buf + disp;
       // copy INODE to mp->INODE
       mip->INODE = *ip;

       return mip;
    }
  }   
  printf("PANIC: no more free minodes\n");
  return 0;
}

iput(MINODE *mip)
{
 int i, block, offset;
 char buf[BLKSIZE];
 INODE *ip;

 if (mip==0) 
     return;

 mip->refCount--;
 
 if (mip->refCount > 0) return;
 if (!mip->dirty)       return;
 
 /* write back */
 printf("iput: dev=%d ino=%d\n", mip->dev, mip->ino); 

 block =  ((mip->ino - 1) / 8) + inode_start;
 offset =  (mip->ino - 1) % 8;

 /* first get the block containing this inode */
 get_block(mip->dev, block, buf);

 ip = (INODE *)buf + offset;
 *ip = mip->INODE;

 put_block(mip->dev, block, buf);
 mip->dirty = 0;

}

  


int search(MINODE *mip, char *name)
{
  char sbuf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;
  int i;
  INODE *ip = &mip->INODE;
  for (i=0; i < 12; i++){  // assume DIR at most 12 direct blocks
      if (ip->i_block[i] == 0)
         break;

      get_block(dev, ip->i_block[i], sbuf);

      dp = (DIR *)sbuf;
      cp = sbuf;

      while(cp < sbuf + BLKSIZE){
         strncpy(temp, dp->name, dp->name_len);
         temp[dp->name_len] = 0;
         if (strcmp(temp, name) == 0) {
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
  int i, ino, blk, disp;
  INODE *ip;
  MINODE *mip;

  printf("getino: pathname=%s\n", pathname);
  if (strcmp(pathname, "/")==0)
      return 2;

  if (pathname[0]=='/')
    mip = iget(dev, 2);
  else
    mip = iget(running->cwd->dev, running->cwd->ino);
  tokenize(pathname);
  for (i=0; i<n; i++){
      printf("===========================================\n");
      ino = search(mip, name[i]);
      if (ino==0){
         iput(mip);
         printf("name %s does not exist\n", name[i]);
         return 0;
      }
      iput(mip);
      mip = iget(dev, ino);
   }
   return ino;
}

int tst_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  if (buf[i] & (1 << j))
     return 1;
  return 0;
}

int set_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] |= (1 << j);
}

int clr_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] &= ~(1 << j);
}

int ialloc(int dev)
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, imap, buf);

  for (i=0; i < ninodes; i++){
    if (tst_bit(buf, i)==0){
       set_bit(buf,i);
       put_block(dev, imap, buf);
       return i+1;
    }
  }
  return 0;
}

int balloc(dev)
{
  int  i;
  char buf[BLKSIZE];

  get_block(dev, bmap, buf);

  for (i=0; i < ninodes; i++){
    if (tst_bit(buf, i)==0){
       set_bit(buf,i);
       put_block(dev, bmap, buf);
       return i+1;
    }
  }
  return 0;
}
int idealloc(int dev, int ino)
{
  char buf[BLKSIZE] = {0};
  // read inode_bitmap block
  get_block(dev, imap, buf);
  clr_bit(buf,ino - 1);
  put_block(dev, imap, buf);
  return 0;
}

int bdealloc(int dev, int ino)
{
  char buf[BLKSIZE] = {0};

  // read inode_bitmap block

  get_block(dev, bmap, buf);
  clr_bit(buf,ino - 1);
  put_block(dev, bmap, buf);
  return 0;
}

int enter_name(MINODE *dmip, int myino, char *myname)
{
  printf("entering name\n");
  char sbuf[BLKSIZE];
  int* b = dmip->INODE.i_block;
  int i = 0;
  int need = 0;
  DIR* dp = 0;
  char *cp = 0;
  for(i; i < 12 && b[i] != 0; i++) {
    get_block(dev, b[i], sbuf);
    dp = (DIR *) sbuf;
    cp = dp;
    while (cp + dp->rec_len < sbuf + BLKSIZE){
      char temp[256];
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;
      printf("enter name- occupied dir %s\n", temp);
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
    int last_need = 4*((8 + dp->name_len + 3)/4);
    int remain = dp->rec_len - last_need;
    need = 4*((8 + strlen(myname) + 3)/4);
    if (need <= remain) {
      dp->rec_len = last_need;
      dp = (char *)dp + last_need;
      goto double_break;
    }
  }
  double_break:;

  if (!b[i]) { // open space not found, allocate new
    b[i] = balloc(dev);
    printf("alloc block for mkdir: %d\n",b[i]);
    get_block(dev, b[i], sbuf);
    dp = (DIR *) sbuf;
  }
  dp->inode = myino;
  strcpy(dp->name, myname);
  dp->name_len = strlen(myname);
  dp->rec_len = sbuf + BLKSIZE - (char *)dp;
  put_block(dev, b[i], sbuf);
  printf("put block for mkdir: %d\n",b[i]);
}

int truncate(MINODE *mip) {
  int i = 0;
  for(i = 0; i < 15; i++){
    int directs = i - 11 > 0 ? i - 11: 0;
    del_indirects(directs, mip->INODE.i_block[i], mip->dev);
  }
}

//depth-first search, bdealloc each
int del_indirects(int directs, int block, int dev) {
  char sbuf[BLKSIZE];
  if (block == 0) {
    return 1;
  }
  if (directs == 0) {
    bdealloc(dev, block);
    return 0;
  }
  else {
    get_block(dev, block, sbuf);
    bdealloc(dev, block);
    int i = 0;
    for (i = 0; i < BLKSIZE; i += 4) {
      int next_block = *(int *)(sbuf + i);
      del_indirects(directs - 1, next_block, dev);
    }
  }
}

