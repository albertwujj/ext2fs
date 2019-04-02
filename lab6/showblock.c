//
//  showblock.c
//  
//
//  Created by Albert Wu on 3/21/19.
//

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#define BLKSIZE 1024


typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc GD;
typedef struct ext2_inode INODE;
typedef struct ext2_dir_entry_2 DIR;

int dev;

// get_block() reads a disk BLOCK into a char buf[BLKSIZE].
int get_block(int dev, int blk, char *buf)
{
    lseek(dev, blk*BLKSIZE, SEEK_SET);
    return read(dev, buf, BLKSIZE);
}

int show_dir(INODE *ip)
{
  char sbuf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;
  int i;

  for (i=0; i < 12; i++){  // assume DIR at most 12 direct blocks
      if (ip->i_block[i] == 0)
         break;
      printf("i = %d, i_block[%d] = %lu\n", i, i, ip->i_block[i]);
      get_block(dev, ip->i_block[i], sbuf);

      dp = (DIR *)sbuf;
      cp = sbuf;

      while(cp < sbuf + BLKSIZE){
         strncpy(temp, dp->name, dp->name_len);
         temp[dp->name_len] = 0;
         printf("%4d %4d %4d %s\n", dp->inode, dp->rec_len, dp->name_len, temp);

         cp += dp->rec_len;
         dp = (DIR *)cp;
     }
  }
}

int search(INODE *ip, char *name)
{
  printf("search for %s\n", name);
  char sbuf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;
  int i;

  for (i=0; i < 12; i++){  // assume DIR at most 12 direct blocks
      if (ip->i_block[i] == 0)
         break;

      printf("i = %d, i_block[%d] = %lu\n", i, i, ip->i_block[i]);
      get_block(dev, ip->i_block[i], sbuf);

      dp = (DIR *)sbuf;
      cp = sbuf;

      while(cp < sbuf + BLKSIZE){
         strncpy(temp, dp->name, dp->name_len);
         temp[dp->name_len] = 0;
         printf("%4d %4d %4d %s\n", dp->inode, dp->rec_len, dp->name_len, temp);
         if (strcmp(temp, name) == 0) {
          printf("found %s: ino = %d\n", name, dp->inode);
          return dp->inode;
         }

         cp += dp->rec_len;
         dp = (DIR *)cp;
     }
  }
  return 0;
}

int tokpath(char **name, char *pathname) {

  char *tok = strtok(pathname, "/");
  int i = 0;
  for(i = 0; tok; i++) {
    name[i] = tok;
    tok = strtok(NULL, "/");
  }
  name[i] = NULL;
}

//depth-first search, print at base level
int print_indirects(int directs, int block) {
  char sbuf[BLKSIZE];
  if (block == 0) {
    return 1;
  }
  if (directs == 0) {
    printf("%d ", block);
    return 0;
  }
  else {
    get_block(dev, block, sbuf);
    int i = 0;
    for (i = 0; i < BLKSIZE; i += 4) {
      int next_block = *(int *)(sbuf + i);
      print_indirects(directs - 1, next_block);
    }
  }
}

int main(int argc, char *argv[]) {
  char buf[BLKSIZE];
  dev = open(argv[1], O_RDONLY);   // OR  O_RDWR
  get_block(dev, 1, buf);
  struct ext2_super_block *sblock = buf;
  if (sblock->s_magic != 0xEF53) {
    printf("not ext2 filesystem");
    exit(0);
  }
  get_block(dev, 2, buf);
  struct ext2_group_desc *gblock = buf;
  unsigned long bmap = gblock->bg_block_bitmap;
  unsigned long imap = gblock->bg_inode_bitmap;
  unsigned long inodes_start = gblock->bg_inode_table;
  printf("bmap: %lu, imap: %lu, inodes_start: %lu\n", bmap, imap, inodes_start);

  get_block(dev, inodes_start, buf);
  INODE *ip = (INODE *)buf + 1;
  printf("Show ROOT dir contents\n");
  show_dir(ip);

  char *name[20];
  tokpath(name, argv[2]);
  int blk, offset, ino;
  int i = 0;
  for (i = 0; name[i] != NULL; i++) {
    ino = search(ip, name[i]);
    if (ino==0){
      printf("can't find %s\n", name[i]);
      exit(1);
    }

    // Mailman's algorithm: Convert (dev, ino) to INODE pointer
    blk    = (ino - 1) / 8 + inodes_start;
    offset = (ino - 1) % 8;
    get_block(dev, blk, buf);
    ip = (INODE *)buf + offset;   // ip -> new INODE

  }

  printf("size = %lu\n", ip->i_size);
  for (i = 0; i < 12; i++) {
    printf("iblock[%d] = %lu\n", i, ip->i_block[i]);
  }
  printf("----------- INDIRECT BLOCKS ---------------\n");
  print_indirects(1, ip->i_block[12]);
  printf("\n----------- DOUBLE INDIRECT BLOCKS ---------------\n");
  print_indirects(2, ip->i_block[13]);
  printf("\n");
}
