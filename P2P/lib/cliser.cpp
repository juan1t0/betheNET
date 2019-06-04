#include "cliser.h"

int createServer(int port){
    struct sockaddr_in stSockAddr;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(port);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;

    bind(SocketFD,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in));

    listen(SocketFD, 10);
    return SocketFD;
}

int createClient(std::string ip="127.0.0.1", int port=0){
	
	struct sockaddr_in stSockAddr;
    int SocketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(port);
    stSockAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    int n=connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in));
    if(n<0) return -1;
	return SocketFD;
}