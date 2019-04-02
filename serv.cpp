/* Server code in C */
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
using namespace std;

int main(void)
{
  struct sockaddr_in stSockAddr;
  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  char buffer[256];
  char res[256];
  string aux;
  int n;

  if(-1 == SocketFD)
  {
    perror("can not create socket");
    exit(EXIT_FAILURE);
  }

  memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(1100);
  stSockAddr.sin_addr.s_addr = INADDR_ANY;

  if(-1 == bind(SocketFD,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
  {
    perror("error bind failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  if(-1 == listen(SocketFD, 10))
  {
    perror("error listen failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  int ConnectFD = accept(SocketFD, NULL, NULL);

  if(0 > ConnectFD)
  {
    perror("error accept failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  for(;;){
    bzero(buffer,256);
    n = read(ConnectFD,buffer,sizeof(buffer));
    if (n < 0) perror("ERROR reading from socket");
    printf("Cliente: [%s]\n",buffer);
    getline(cin,aux);
    strcpy(res,aux.c_str());
    n = write(ConnectFD,(char*)&res,strlen(res));
      if (n < 0) perror("ERROR writing to socket");
  }
  shutdown(ConnectFD, SHUT_RDWR);
  close(ConnectFD);
  close(SocketFD);
  return 0;
}