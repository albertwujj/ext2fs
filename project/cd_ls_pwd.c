/************* cd_ls_pwd.c file **************/

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern char   gpath[256];
extern char   *name[64];
extern int    n;
extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, inode_start;
extern char line[256], cmd[32], pathname[256];

#define OWNER  000700
#define GROUP  000070
#define OTHER  000007

int ls_file(INODE *ip, char *bname) {

  char ftime[64];
  char *t1 = "xwr";

  if ((ip->i_mode & 0xF000) == 0x8000) // if (S_ISREG())
    printf("%c\0",'-');
  if ((ip->i_mode & 0xF000) == 0x4000) // if (S_ISDIR())
    printf("%c\0",'d');
  if ((ip->i_mode & 0xF000) == 0xA000) // if (S_ISLNK())
    printf("%c\0",'l');
  int i;
  for (i=8; i >= 0; i--){
    if (ip->i_mode & (1 << i)) // print r|w|x
      printf("%c\0", t1[i % 3]);
    else
      printf("%c\0", '-'); // or print -
  }

  printf("%4d ",ip->i_links_count); // link count
  printf("%4d ",ip->i_gid); // gid
  printf("%4d ",ip->i_uid); // uid
  printf("%8d ",ip->i_size); // file size

  // print time
  strcpy(ftime, ctime(&ip->i_mtime)); // print time in calendar form
  ftime[strlen(ftime)-1] = 0; // kill \n at end
  printf("%s ",ftime);
  // print name
  printf("%s", bname); // print file basename
  printf("\n");
}

int ls_dir(INODE *ip) {

  char sbuf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;
  int i;

  //assume dir is 1 block
  get_block(dev, ip->i_block[0], sbuf);
  dp = (DIR *)sbuf;
  cp = sbuf;
  while(cp < sbuf + BLKSIZE){
     strncpy(temp, dp->name, dp->name_len);
     temp[dp->name_len] = 0;
     MINODE *fmip = iget(dev, dp->inode);
     INODE *fip = &fmip->INODE;
     ls_file(fip, temp);
     cp += dp->rec_len;
     dp = (DIR *)cp;
  }
}

change_dir()
{
  if(strcmp(pathname, "")==0) {
    running->cwd = root;
  } else {
    int ino = getino(pathname);
    MINODE *mip = iget(dev, ino);
    if (!S_ISDIR(mip->INODE.i_mode)) {
      printf("%s not a dir\n", pathname);
      return;
    }
    iput(running->cwd);
    running->cwd = mip;
  }
}


int list_file()
{
  //pass inode to lsdir/lsfile
  INODE *ip = NULL;
  if(strcmp(pathname, "")==0) {
    ip = &running->cwd->INODE;
  } else {
    int ino = getino(pathname);
    MINODE *mip = iget(dev, ino);
    ip = &mip->INODE;
  }

  if S_ISDIR(ip->i_mode) {
    ls_dir(ip);
  } else {
    ls_file(ip, basename(pathname));
  }
}


rpwd(MINODE *mip) {
  if (mip == root) {
    return;
  }
  char sbuf[BLKSIZE];
  int myino = search(mip, ".");
  int pino = search(mip, ".."); // parent

  //search in parent directory for myfile
  MINODE *pmip = iget(dev, pino);
  get_block(dev, pmip->INODE.i_block[0], sbuf);
  DIR *dp = (DIR *)sbuf;
  char *cp = (char *)dp;
  char temp[256];

  while (cp < sbuf + BLKSIZE){
    if (dp->inode == myino){
      rpwd(pmip); // run on parent minode

      //print myfile
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;
      printf("/%s", temp);
      return;
    }
    cp += dp->rec_len;
    dp = (DIR *)cp;
  }
}


int pwd()
{
  if (running->cwd == root) {
    printf("/");
  }
  else {
      rpwd(running->cwd);
  }
  printf("\n");
}
