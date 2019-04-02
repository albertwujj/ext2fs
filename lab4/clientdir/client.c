// The echo client client.c
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
#include "../ls_util.c"

#define MAX 256

// Define variables
struct hostent *hp;              
struct sockaddr_in  server_addr;

int server_sock, r;
int SERVER_IP, SERVER_PORT;

char *pathname;
char line[MAX], ans[MAX], linecopy[MAX];

int get() {
  read(server_sock, ans, MAX);
  if (ans[0] != 'F') {
    int fd = open(pathname, O_CREAT | O_WRONLY, 0755);
    long size = 0;
    long currbytes = 0;
    long readbytes = 0;
    sscanf(ans, "%ld", &size);
    while(currbytes < size) {
      readbytes = read(server_sock, ans, MAX);
      printf("%ld\n",readbytes);
      write(fd, ans, readbytes);
      currbytes += readbytes;
    }
    close(fd);
  } else {
    read(server_sock, ans, MAX);
    int fail = 0;
    sscanf(ans, "%d", &fail);
    printf("get failed w/ code %d", fail);
  }
}


int put() {
  struct stat stinit;
  struct stat *st = &stinit;
  int r = stat(pathname, st);
  if(r) {
    write(server_sock, "F", 1);
    printf("failed to read filename: stat returned error %d\n", r);
    return;
  } else {
    write(server_sock, "S", 1);
  }
  sprintf(ans, "%ld", st->st_size);
  write(server_sock, ans, MAX);
  int fd = open(pathname, O_RDONLY, 0755);
  int nbytes = 0;
  while(nbytes = read(fd, ans, MAX)) {
    write(server_sock, ans, nbytes);
  }
  close(fd);
}

int ls() {
  read(server_sock, ans, MAX);
  if(ans[0] == 'F') {
    read(server_sock, ans, MAX);
    int error = 0;
    sscanf(ans, "%d", &error);
    printf("ls failed, error: %d", error);
    return;
  } else {
    read(server_sock, ans, 1);
    while (ans[0] != 'D') {
      read(server_sock, ans, MAX);
      printf("%s", ans);
      read(server_sock, ans, 1);
    }
  }
}


int pwd() {
   read(server_sock, ans, MAX);
   printf("%s", ans);
}

int printcode() {
  read(server_sock, ans, MAX);
  if(ans[0] == 'F') {
    read(server_sock, ans, MAX);
    int r = 0;
    sscanf(ans, "%d", &r);
    printf("command failed with error code %d\n", r);
  } else {
    printf("command success\n", r);
  }
}

int lcat() {
  int c;
  FILE *file;
  file = fopen(pathname, "r");
  if (file) {
    while ((c = getc(file)) != EOF)
        putchar(c);
    fclose(file);
  }
}

int lpwd() {
  getcwd(ans, MAX);
  printf("%s\n", ans);
}

int lls_file(char *pathname) {
  struct stat fstat, *sp;
  int r;
  sp = &fstat;
  if ( (r = lstat(pathname, &fstat)) < 0){
    printf("can’t stat %s\n", pathname);
    exit(1);
  }
  ls_file_to(pathname, sp, ans);
  printf("%s", ans);
}

int lls_dir(char *pathname) {
  DIR *dirstruct = opendir(pathname);
  struct dirent *dire;
  while (dire = readdir(dirstruct)){
    char fpath[256];
    strcpy(fpath, pathname);
    strcat(fpath, dire->d_name);
    lls_file(fpath);
  }
}

int lls() {
  struct stat fstat;
  if(strcmp(pathname, "") == 0) {
    strcpy(pathname, ".");
  }
  int r = stat(pathname, &fstat);
  if (r) {
    printf("dir not found\n");
    return;
  }
  if (S_ISREG(fstat.st_mode)) {
    lls_file(pathname);
  } else if (S_ISDIR(fstat.st_mode)) {
    lls_dir(pathname);
  }
}

int lcd() {
  int r = chdir(pathname);
  if (r) {
    printf("chdir failed\n");
  }
}
int lmkdir() {
  int r = mkdir(pathname, 0755);
  if (r) {
    printf("mkdir failed\n");
  }
}
int lrmdir() {
  int r = rmdir(pathname);
  if (r) {
    printf("rmdir failed\n");
  }
}
int lrm() {
  int r = unlink(pathname);
  if (r) {
    printf("rm failed\n");
  }
}
char *localcmds[] = {"lcat", "lpwd", "lls", "lcd", "lmkdir", "lrmdir", "lrm", NULL};
int (*lfuncs[])() = {lcat, lpwd, lls, lcd, lmkdir, lrmdir, lrm};

int (*sfuncs[])() = {get, put, ls, printcode, pwd, printcode, printcode, printcode};
char *scmds[] = {"get", "put", "ls", "cd", "pwd", "smkdir", "srmdir", "srm", NULL};

int (*cmd_id(char *cmd, int (*f[])(), char *cmds[]))(int, int) {
  int i = 0;
  while(*(cmds + i)) {
    if (strcmp(cmd, *(cmds + i)) == 0) {
      return f[i];
    }
    i++;
  }
  return NULL;
}

// clinet initialization code
int client_init(char *argv[])
{
  printf("======= clinet init ==========\n");

  printf("1 : get server info\n");
  hp = gethostbyname(argv[1]);
  if (hp==0){
    printf("unknown host %s\n", argv[1]);
    exit(1);
  }

  SERVER_IP   = *(long *)hp->h_addr;
  SERVER_PORT = atoi(argv[2]);

  printf("2 : create a TCP socket\n");
  server_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sock<0){
    printf("socket call failed\n");
    exit(2);
  }

  printf("3 : fill server_addr with server's IP and PORT#\n");
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = SERVER_IP;
  server_addr.sin_port = htons(SERVER_PORT);

  // Connect to server
  printf("4 : connecting to server ....\n");
  r = connect(server_sock,(struct sockaddr *)&server_addr, sizeof(server_addr));
  if (r < 0){
    printf("connect failed\n");
    exit(1);
  }

  printf("5 : connected OK to \007\n");
  printf("---------------------------------------------------------\n");
  printf("hostname=%s  IP=%s  PORT=%d\n",
         hp->h_name, inet_ntoa(*(struct in_addr *)(&SERVER_IP)), SERVER_PORT);
  printf("---------------------------------------------------------\n");

  printf("========= init done ==========\n");
}

main(int argc, char *argv[ ])
{
  int n;

  if (argc < 3){
    printf("Usage : client ServerName SeverPort\n");
    exit(1);
  }

  client_init(argv);
  // sock <---> server
  printf("********  processing loop  *********\n");
  while (1){
    printf("input a line : ");
    bzero(line, MAX);                // zero out line[ ]
    fgets(line, MAX, stdin);         // get a line (end with \n) from stdin

    line[strlen(line)-1] = 0;        // kill \n at end
    if (line[0]==0)                  // exit if NULL line
      exit(0);

    strcpy(linecopy, line);
    char *cmd = strtok(linecopy, " ");
    pathname = strtok(NULL, " ");
    if(!pathname) {
      pathname = cmd + strlen(cmd) + 1;
    }
    int (*lfunc)() = cmd_id(cmd, lfuncs, localcmds);
    if (lfunc) {
      //execute local cmd
      lfunc();
      continue;
    }

    int (*sfunc)() = cmd_id(cmd, sfuncs, scmds);
    if (!sfunc) {
      printf("invalid command\n\n");
      continue;
    }
    // Send ENTIRE line to server
    n = write(server_sock, line, MAX);
    printf("client: wrote n=%d bytes; line=(%s)\n", n, line);
    sfunc();

  }
}


