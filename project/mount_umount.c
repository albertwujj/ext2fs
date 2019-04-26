//
//  mount_umount.c
//  
//
//  Created by Albert Wu on 4/23/19.
//

#include <stdio.h>

int mount_dev() {
  int true_mount_fd = 0;

  if ((true_mount_fd = open(pathname, O_RDWR)) < 0){
    printf("open %s failed\n", pathname);  exit(1);
  }

  MINODE *mip = getmino(pathname2);

  if(!S_ISDIR(mip->INODE.i_mode)) {
    printf("mountpoint not a dir\n");
    return;
  }
  //TODO: check if mip is BUSY

  int i = 0;
  for(i; i < NMNT; i++) {
    if(strcmp(mnttable[i].name, pathname)==0) {
      printf("point already mounted\n");
      iput(mip);
      return;
    }
    if(mnttable[i].dev==0) {
      mnttable[i].dev = true_mount_fd;
      mnttable[i].mntpoint=mip;
      strcpy(mnttable[i].name, pathname);
      mip->mounted = 1;
      mip->mptr = mnttable+i;
      return 0;
    }
  }
}

int umount_dev() {
  int i = 0;
  for(i; i < NMNT; i++) {
    if(strcmp(mnttable[i].name, pathname)==0) {
      int j = 0;
      for(j; j < NMINODE && *((int *)&minode[j].INODE) != 0; j++) {
        if(minode[j].dev == mnttable[i].dev) {
          printf("dev is busy\n");
        }
      }
      MNT *mnt = mnttable + i;
      mnt->mntpoint->mounted=0;
      mnt->mntpoint->mptr=0;
      iput(mnt->mntpoint);
      close(mnt->dev);
      mnt->dev=0;
      mnt->name[0]=0;
      mnt->mntpoint=0;
      return 0;
    }
  }
}

int show_mount() {
  int i = 0;
  printf("All mounts \n:");
  for(i; i < NMNT && mnttable[i].dev != 0; i++) {
    MNT *mnt = mnttable + i;
    printf("dev %d name mounted on ino %d\n", mnt->dev, mnt->mntpoint->ino);
  }
}
