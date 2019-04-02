// This is the echo SERVER server.c
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

// Define variables:
struct sockaddr_in  server_addr, client_addr, name_addr;
struct hostent *hp;

int  mysock, client_sock;              // socket descriptors
int  serverPort;                     // server port number
int  r, length, n;                   // help variables
char *pathname;
char buf[MAX];

void send_error(int r) {

    sprintf(buf, "F");
    write(client_sock, buf, MAX);
    sprintf(buf, "%d", r);
    write(client_sock, buf, MAX);
}
void send_success() {

    sprintf(buf, "S");
    write(client_sock, buf, MAX);
}

int get() {

  struct stat stinit;
  struct stat *st = &stinit;
  int r = stat(pathname, st);
  if(r) {
    send_error(r);
    return;
  }
  sprintf(buf, "%ld", st->st_size);
  write(client_sock, buf, MAX);
  int fd = open(pathname, O_RDONLY, 0755);
  int nbytes = 0;
  while(nbytes = read(fd, buf, MAX)) {
    write(client_sock, buf, nbytes);
  }
  close(fd);
}

int put() {
  read(client_sock, buf, 1);
  if (buf[0] == 'F') {
    printf("put failed\n");
    return -1;
  }
  int fd = open(basename(pathname), O_CREAT | O_WRONLY, 0755);
  long size = 0;
  read(client_sock, buf, MAX);
  sscanf(buf, "%ld", &size);
  long currbytes = 0;
  long readbytes = 0;
  while(currbytes < size) {
    readbytes = read(client_sock, buf, MAX);
    write(fd, buf, readbytes);
    currbytes += readbytes;
  }
  close(fd);
}

int ls_file(char *pathname) {
  struct stat fstat, *sp;
  int r;
  sp = &fstat;
  if ( (r = lstat(pathname, &fstat)) < 0){
    printf("can’t stat %s\n", pathname);
    exit(1);
  }

  ls_file_to(pathname, sp, buf);
  write(client_sock, buf, MAX);
}

int ls_dir(char *pathname) {
  printf("startinglsdir\n");
  DIR *dirstruct = opendir(pathname);
  struct dirent *dire;
  while (dire = readdir(dirstruct)){
    write(client_sock, "C", 1);
    ls_file(dire->d_name);
  }
}

int ls() {
  struct stat fstat;
  if(strcmp(pathname, "") == 0) {
    strcpy(pathname, ".");
  }
  int r = stat(pathname, &fstat);
  if (r) {
    send_error(r);
    return;
  }
  send_success();

  if (S_ISREG(fstat.st_mode)) {
    write(client_sock, "C", 1);
    ls_file(pathname);
  } else if (S_ISDIR(fstat.st_mode)) {
    ls_dir(pathname);
  }
  write(client_sock, "D", 1);
}

int cd() {
  int r = chdir(pathname);
  if(r)
    send_error(r);
  else
    send_success();
}

int pwd() {
  getcwd(buf, MAX);
  write(client_sock, buf, MAX);
}

int smkdir() {
  int r = mkdir(pathname, 0755);
  if(r)
    send_error(r);
  else
    send_success();
}
int srmdir() {
  int r = rmdir(pathname);
  if(r)
    send_error(r);
  else
    send_success();
}
int srm() {
  int r = unlink(pathname);
  if(r)
    send_error(r);
  else
    send_success();
}

int (*f[])() = {get, put, ls, cd, pwd, smkdir, srmdir, srm};
char *cmds[] = {"get", "put", "ls", "cd", "pwd", "smkdir", "srmdir", "srm", NULL};

int (*cmd_id(char *cmd))(int, int) {
  int i = 0;
  while(*(cmds + i)) {
    if (strcmp(cmd, *(cmds + i)) == 0) {
      return f[i];
    }

    i++;
  }
  return NULL;
}



// Server initialization code:
int server_init(char *name)
{
  printf("==================== server init ======================\n");
  // get DOT name and IP address of this host

  printf("1 : get and show server host info\n");
  hp = gethostbyname(name);
  if (hp == 0){
    printf("unknown host\n");
    exit(1);
  }
  printf("    hostname=%s  IP=%s\n",
         hp->h_name,  inet_ntoa(*(struct in_addr *)hp->h_addr));

  //  create a TCP socket by socket() syscall
  printf("2 : create a socket\n");
  mysock = socket(AF_INET, SOCK_STREAM, 0);
  if (mysock < 0){
    printf("socket call failed\n");
    exit(2);
  }

  printf("3 : fill server_addr with host IP and PORT# info\n");
  // initialize the server_addr structure
  server_addr.sin_family = AF_INET;                  // for TCP/IP
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);   // THIS HOST IP address
  server_addr.sin_port = 0;   // let kernel assign port

  printf("4 : bind socket to host info\n");
  // bind syscall: bind the socket to server_addr info
  r = bind(mysock,(struct sockaddr *)&server_addr, sizeof(server_addr));
  if (r < 0){
    printf("bind failed\n");
    exit(3);
  }

  printf("5 : find out Kernel assigned PORT# and show it\n");
  // find out socket port number (assigned by kernel)
  length = sizeof(name_addr);
  r = getsockname(mysock, (struct sockaddr *)&name_addr, &length);
  if (r < 0){
    printf("get socketname error\n");
    exit(4);
  }

  // show port number
  serverPort = ntohs(name_addr.sin_port);   // convert to host ushort
  printf("    Port=%d\n", serverPort);

  // listen at port with a max. queue of 5 (waiting clients)
  printf("5 : server is listening ....\n");
  listen(mysock, 5);
  printf("===================== init done =======================\n");
}


int main(int argc, char *argv[])
{
  char *hostname;
  char line[MAX];

  char *cmd;
  int fd = 0;

  if (argc < 2)
    hostname = "localhost";
  else
    hostname = argv[1];

  server_init(hostname);
  getcwd(buf, MAX);
  chroot(buf);

  // Try to accept a client request
  while(1){
    printf("server: accepting new connection ....\n");

    // Try to accept a client connection as descriptor newsock
    length = sizeof(client_addr);
    client_sock = accept(mysock, (struct sockaddr *)&client_addr, &length);
    if (client_sock < 0){
      printf("server: accept error\n");
      exit(1);
    }
    printf("server: accepted a client connection from\n");
    printf("-----------------------------------------------\n");
    printf("        IP=%s  port=%d\n", inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));
    printf("-----------------------------------------------\n");

    // Processing loop: newsock <----> client
    while(1){
      n = read(client_sock, line, MAX);
      if (n==0){
        printf("server: client died, server loops\n");
        close(client_sock);
        break;
      }

      // show the line string
      printf("server: read  n=%d bytes; line=[%s]\n", n, line);

      int r;
      cmd = strtok(line, " ");
      pathname = strtok(NULL, " ");
      if(!pathname) {
        pathname = line + strlen(line) + 1;
      }
      int (*f)() = cmd_id(cmd);
      f();
      printf("server: ready for next request\n");
    }

  }
}

