//
//  mkdir_creat.c
//  
//
//  Created by Albert Wu on 4/1/19.
//

#include <stdio.h>
/*
1. pahtname = "/a/b/c" start mip = root;         dev = root->dev;
            =  "a/b/c" start mip = running->cwd; dev = running->cwd->dev;

2. Let
     parent = dirname(pathname);   parent= "/a/b" OR "a/b"
     child  = basename(pathname);  child = "c"

   WARNING: strtok(), dirname(), basename() destroy pathname

3. Get the In_MEMORY minode of parent:

         pino  = getino(parent);
         pip   = iget(dev, pino);

   Verify : (1). parent INODE is a DIR (HOW?)   AND
            (2). child does NOT exists in the parent directory (HOW?);

4. call mymkdir(pip, child);

5. inc parent inodes's link count by 1;
   touch its atime and mark it DIRTY

6. iput(pip);
*/
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
      printf("occupied dir %s\n", temp);
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

int mymkdir(MINODE *dmip, char *name)
{

  char sbuf[BLKSIZE] = {0};
  int ino = ialloc(dev);
  int bno = balloc(dev);
  printf("mkdir: new ino: %d, bno: %d\n", ino, bno);
  MINODE *mip = iget(dev, ino);
  INODE *ip = &mip->INODE;

  ip->i_mode = 0x41ED;    // OR 040755: DIR type and permissions
  ip->i_uid  = running->uid;  // Owner uid
  ip->i_size = BLKSIZE;    // Size in bytes
  ip->i_links_count = 2;          // Links count=2 because of .
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
  ip->i_blocks = 2;                  // LINUX: Blocks count in 512-byte chunks

  ip->i_block[0] = bno;             // new DIR has one data block
  int i = 1;
  for (i=1;i<15;i++) {
    ip->i_block[i] = 0;
  }
  mip->dirty = 1;               // mark minode dirty
  iput(mip);                    // write INODE to disk

  // curr
  get_block(dev, bno, sbuf);
  DIR *dp = sbuf;
  dp->inode = ino;
  dp->name_len = 1;
  strcpy(dp->name, ".");
  dp->rec_len = 12;

  // parent
  dp = (char *)dp + 12;
  dp->inode = dmip->ino;
  dp->name_len = 2;
  strcpy(dp->name, "..");
  dp->rec_len = 1012;
  dp = (char *)dp + dp->rec_len;


  put_block(dev, bno, sbuf);


  enter_name(dmip, ino, name);
}


int make_dir()
{
  MINODE *dmip;
  INODE *dip;

  char *dir = dirname(strdup(pathname));
  char *base = basename(strdup(pathname));
  printf("doing mymkdir %s %s\n", dir, base);
  int dino = getino(dir);
  dmip = iget(dev, dino);
  dip = &dmip->INODE;

  if (!S_ISDIR(dip->i_mode)) {
    printf("%s not a dir\n", dir);
  }
  if (search(dmip, base) != 0) {
    printf("dir already exists\n");
  }
  mymkdir(dmip, base);

  dip->i_links_count += 1;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  dip->i_atime = tv.tv_sec;
  dmip->dirty = 1;

  iput(dmip);
  
}

int my_creat(MINODE *dmip, char *name)
{

  char sbuf[BLKSIZE] = {0};
  int ino = ialloc(dev);
  printf("creat: new ino: %d\n");
  MINODE *mip = iget(dev, ino);
  INODE *ip = &mip->INODE;

  ip->i_mode = 0x81A4;    // OR 040755: DIR type and permissions
  ip->i_uid  = running->uid;  // Owner uid
  ip->i_size = 0;    // Size in bytes
  ip->i_links_count = 1;          // Links count=2 because of .
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
  ip->i_blocks = 2;                  // LINUX: Blocks count in 512-byte chunks

  mip->dirty = 1;               // mark minode dirty
  iput(mip);                    // write INODE to disk
  enter_name(dmip, ino, name);
}

int creat_file()
{
  MINODE *dmip;
  INODE *dip;

  char *dir = dirname(strdup(pathname));
  char *base = basename(strdup(pathname));
  printf("doing mycreat %s %s\n", dir, base);
  int dino = getino(dir);
  dmip = iget(dev, dino);
  dip = &dmip->INODE;

  if (!S_ISREG(dip->i_mode)) {
    printf("%s not a dir\n", dir);
  }
  if (search(dmip, base) != 0) {
    printf("file already exists\n");
  }
  my_creat(dmip, base);

  struct timeval tv;
  gettimeofday(&tv, NULL);
  dip->i_atime = tv.tv_sec;
  dmip->dirty = 1;

  iput(dmip);

}
