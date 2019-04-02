//
//  ls_util.c
//  
//
//  Created by Albert Wu on 3/6/19.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <libgen.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define MAX 256


int ls_file_to(char *pathname, struct stat *sp, char *buf) {
  buf[0] = '\0';
  char ftime[64];
  char *t1 = "xwrxwrxwr-------";
  char *t2 = "----------------";
  if ((sp->st_mode & 0xF000) == 0x8000) // if (S_ISREG())
    sprintf(buf,"%c\0",'-');
  if ((sp->st_mode & 0xF000) == 0x4000) // if (S_ISDIR())
    sprintf(buf + strlen(buf),"%c\0",'d');
  if ((sp->st_mode & 0xF000) == 0xA000) // if (S_ISLNK())
    sprintf(buf + strlen(buf), "%c\0",'l');
  int i;
  for (i=8; i >= 0; i--){
    if (sp->st_mode & (1 << i)) // print r|w|x
      sprintf(buf + strlen(buf),"%c\0", t1[i]);
    else
      sprintf(buf + strlen(buf), "%c\0", t2[i]); // or print -
  }
  sprintf(buf + strlen(buf), "%4d ",sp->st_nlink); // link count
  sprintf(buf + strlen(buf), "%4d ",sp->st_gid); // gid
  sprintf(buf + strlen(buf), "%4d ",sp->st_uid); // uid
  sprintf(buf + strlen(buf), "%8d ",sp->st_size); // file size
  // print time
  strcpy(ftime, ctime(&sp->st_ctime)); // print time in calendar form
  ftime[strlen(ftime)-1] = 0; // kill \n at end
  sprintf(buf + strlen(buf), "%s ",ftime);
  // print name
  sprintf(buf + strlen(buf), "%s", basename(pathname)); // print file basename
  sprintf(buf + strlen(buf), "\n");
}
