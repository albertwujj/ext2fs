//
//  rmdir.c
//  
//
//  Created by Albert Wu on 4/2/19.
//

#include <stdio.h>

int remove_dir() {
  char sbuf[BLKSIZE];
  int ino = getino(pathname);
  MINODE *mip = iget(dev, ino);

  // TODO: also check for root?
  if(running->uid != mip->INODE.i_uid) {
    printf("not same uid, canceling rmdir\n");
    return;
  }
  int pino = 0;
  if (mip->INODE.i_links_count <= 2){
    int bno = mip->INODE.i_block[0];
    get_block(mip->dev, bno, sbuf);

    DIR *dp = (DIR *) sbuf;
    char *cp = dp;
    while (cp < sbuf + BLKSIZE){
      char temp[256];
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;
      if (!strcmp(temp, "..")) {
        pino = dp->inode;
      }
      else if(strcmp(temp, ".")) {
        printf("%s not empty\n");
        iput(mip);
        return;
      }
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }
  if(!S_ISDIR(mip->INODE.i_mode)){
    printf("%s not a dir\n");
    iput(mip);
    return;
  }
  if(mip->dirty){
    printf("inode busy\n");
    return;
  }

  int i = 0;
  for (i=0; i<12; i++){
    if (mip->INODE.i_block[i]==0)
      continue;
    bdealloc(mip->dev, mip->INODE.i_block[i]);
  }
  idealloc(mip->dev, mip->ino);
  iput(mip);
  MINODE *pmip = iget(mip->dev, pino);

  rm_child(pmip, basename(strdup(pathname)));
}

int rm_child(MINODE *pmip, char *name) {
  printf("starting rm child\n");
  char sbuf[BLKSIZE] = {0};
  int *b = pmip->INODE.i_block;
  int i = 0;
  for(i; i < 12; i++) {
    DIR *dp;
    char *cp;
    DIR *prev_dp;
    get_block(dev, b[i], sbuf);
    dp = (DIR *) sbuf;
    cp = dp;
    while (cp < sbuf + BLKSIZE){
      char temp[256];
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;
      printf("occupied dir during rmdir %s\n", temp);
      if(strcmp(temp, name) == 0) {
        if(cp == sbuf) {  // is first entry
          bdealloc(b[i]);
          for(i; i < 11; i++) {
            b[i] = b[i + 1];
          }
          b[11] = 0;
          goto double_break;
        }
        if(cp + dp->rec_len == sbuf + BLKSIZE) { // is last entry
          prev_dp->rec_len += dp->rec_len;
          goto double_break;
        }

        // is middle entry
        char *dest = cp;
        char *source = cp + dp->rec_len;
        int size = dp->rec_len;

        while(cp + dp->rec_len < sbuf + BLKSIZE) {
          cp += dp->rec_len;
          dp = cp;
        }
        dp->rec_len += size;
        memmove(dest, source, sbuf+BLKSIZE-source);
        goto double_break;
      }
      prev_dp = dp;
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }
  double_break:;
  put_block(pmip->dev, b[i],sbuf);
}
