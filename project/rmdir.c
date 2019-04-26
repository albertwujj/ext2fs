//
//  rmdir.c
//  
//
//  Created by Albert Wu on 4/2/19.
//

#include <stdio.h>

int remove_dir() {
  char sbuf[BLKSIZE];

  MINODE *mip = getmino(pathname);
  if(!mip) {
    return;
  }

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
        printf("%s not empty\n", pathname);
        iput(mip);
        return;
      }
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  } else {
    printf("%s not empty\n", pathname);
    return;
  }
  if(!S_ISDIR(mip->INODE.i_mode)){
    printf("%s not a dir\n", pathname);
    iput(mip);
    return;
  }
  if(mip->dirty){
    printf("%s inode %d busy\n", pathname, mip->ino);
    return;
  }

  printf("starting deallocs\n");
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

  char sbuf[BLKSIZE] = {0};
  int *b = pmip->INODE.i_block;
  int i = 0;
  for(i; i < 12; i++) {
    DIR *dp;
    char *cp;
    DIR *prev_dp;
    get_block(pmip->dev, b[i], sbuf);
    dp = (DIR *) sbuf;
    cp = dp;
    while (cp < sbuf + BLKSIZE){
      char temp[256];
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;
      printf("rmchild name %s\n", temp);
      printf("occupied dir during rmchild %s\n", temp);
      if(strcmp(temp, name) == 0) {
        if(cp == sbuf) {  // is first entry
          bdealloc(pmip->dev, b[i]);
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
  iput(pmip);
}
