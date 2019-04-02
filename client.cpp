/* Client code in C */
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <unistd.h>

using namespace std;

int main(void)
{
  struct sockaddr_in stSockAddr;
  int Res;
  int ConnectFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  int n;
  char buffer[256];
  char msn[256];
  string aux;

  if (-1 == ConnectFD)
  {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }

  memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(1100);
  Res = inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr);

  if (0 > Res)
  {
    perror("error: first parameter is not a valid address family");
    close(ConnectFD);
    exit(EXIT_FAILURE);
  }
  else if (0 == Res)
  {
    perror("char string (second parameter does not contain valid ipaddress");
    close(ConnectFD);
    exit(EXIT_FAILURE);
  }

  if (-1 == connect(ConnectFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
  {
    perror("connect failed");
    close(ConnectFD);
    exit(EXIT_FAILURE);
  }
  int SocketFD = accept(ConnectFD, NULL, NULL);
  for(;;)
  {
    bzero(msn,256);
    getline(cin, aux);
    strcpy(msn, aux.c_str());
    if(aux=="x")break;
    n = write(ConnectFD,(char*)&msn,strlen(msn));
    if (n < 0) perror("ERROR writing from socket1");
    bzero(buffer,256);
    n = read(ConnectFD,buffer,sizeof(buffer));
    if (n < 0) perror("ERROR reading to socket2");
    printf("servidor: [%s]\n",buffer);
  }
  shutdown(ConnectFD, SHUT_RDWR);
  close(ConnectFD);
  return 0;
}