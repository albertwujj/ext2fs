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
int mymkdir(MINODE *dmip, char *name)
{
  char sbuf[BLKSIZE] = {0};
  int ino = ialloc(dev);
  int bno = balloc(dev);
  printf("mkdir: new ino: %d, bno: %d\n", ino, bno);
  MINODE *mip = iget(dev, ino);
  INODE *ip = &mip->INODE;
  ip->i_mode = ((ip->i_mode) & ~(0xF000)) | 0x4000;
  ip->i_block[0] = bno;

  char sbuf[BLKSIZE];
  ip->i_mode = 0x41ED;    // OR 040755: DIR type and permissions
  ip->i_uid  = running->uid;  // Owner uid
  ip->i_gid  = running->gid;  // Group Id
  ip->i_size = BLKSIZE;    // Size in bytes
  ip->i_links_count = 2;          // Links count=2 because of . and ..
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
  ip->i_blocks = 2;                  // LINUX: Blocks count in 512-byte chunks

  ip->i_block[0] = bno;             // new DIR has one data block
  int i = 1;
  for (i=1;i<15;i++) {
    ip->iblock[i] = 0;
  }
  mip->dirty = 1;               // mark minode dirty
  iput(mip);                    // write INODE to disk

  get_block(dev, bno, sbuf);
  (DIR *) curr = sbuf;
  curr->inode = ino;
  curr->name_len = strlen(name);
  strcpy(curr->name, name);
  curr->rec_len = 8 + strlen(name) + 1;
  put_block(dev, bno, sbuf);

  enter_name(dmip, ino, name);

}
 int enter_name(MINODE *dmip, int myino, char *myname)
{
  char sbuf[BLKSIZE];
  int* b = dmip->INODE.i_block;
  int i = 0;
  for(i; i < 12 && b[i] != 0; i++) {
    get_block(dev, b[12], sbuf);
    DIR* dp = (DIR *) sbuf;
    cp = (char *) dp;
    while (cp < sbuf + BLKSIZE) {
      

      cp += dp->rec_len;
      dp = (DIR *) cp;
    }
  }
}
int make_dir()
{
  char spathname[256] = {0};
  MINODE *dmip;
  INODE *dip;

  char *dir = dirname(spathname);
  char *base = basename(spathname);
  int dino = getino(dir);

  dmip = iget(dev, ino);
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
