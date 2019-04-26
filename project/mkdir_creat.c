//
//  mkdir_creat.c
//  
//
//  Created by Albert Wu on 4/1/19.
//

#include <stdio.h>


int mymkdir(MINODE *dmip, char *name)
{

  char sbuf[BLKSIZE] = {0};
  int ino = ialloc(dmip->dev);
  int bno = balloc(dmip->dev);
  printf("mkdir: new ino: %d, bno: %d\n", ino, bno);
  MINODE *mip = iget(dmip->dev, ino);
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
  get_block(mip->dev, bno, sbuf);
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


  put_block(mip->dev, bno, sbuf);


  enter_name(dmip, ino, name);
}


int make_dir()
{
  MINODE *dmip;
  INODE *dip;

  char *dir = dirname(strdup(pathname));
  char *base = basename(strdup(pathname));
  printf("doing mymkdir %s %s\n", dir, base);
  dmip = getmino(dir);
  printf("minode goten\n");
  if(!dmip->ino) {
    printf("%s not found\n", dirname);
    return;
  }
  dip = &dmip->INODE;

  if (!S_ISDIR(dip->i_mode)) {
    printf("%s not a dir\n", dir);
    return;
  }
  if (search(dmip, base) != 0) {
    printf("dir already exists\n");
    return;
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
  int ino = ialloc(dmip->dev);
  MINODE *mip = iget(dmip->dev, ino);
  INODE *ip = &mip->INODE;

  ip->i_mode = 0x81A4;    // OR 040755: DIR type and permissions
  ip->i_uid  = running->uid;  // Owner uid
  ip->i_size = 0;    // Size in bytes
  ip->i_links_count = 1;          
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
  ip->i_blocks = 2;                  // LINUX: Blocks count in 512-byte chunks

  mip->dirty = 1;               // mark minode dirty
  iput(mip);                    // write INODE to disk
  enter_name(dmip, ino, name);
}

int creat_file()
{
  return my_creat_file(pathname);

}
int my_creat_file(char *pathname) {
  MINODE *dmip;
  INODE *dip;

  char *dir = dirname(strdup(pathname));
  char *base = basename(strdup(pathname));
  printf("doing mycreat %s %s\n", dir, base);
  dmip = getmino(dir);
  if(!dmip->ino) {
    printf("%s not found\n", dirname);
    return;
  }
  dip = &dmip->INODE;

  if (!S_ISDIR(dip->i_mode)) {
    printf("%s not a dir\n", dir);
    return;
  }
  if (search(dmip, base) != 0) {
    printf("file already exists\n");
    return;
  }
  my_creat(dmip, base);

  struct timeval tv;
  gettimeofday(&tv, NULL);
  dip->i_atime = tv.tv_sec;
  dmip->dirty = 1;

  iput(dmip);
}
