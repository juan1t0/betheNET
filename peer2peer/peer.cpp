/*************************************************/
////////////////////// PEER ///////////////////////
/*************************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <utility>
#include <thread> 
#include <string>

using namespace std;

int SocketFD;
int var_y = 15;
int speed = 2;

vector<string> neibots;

int createServer(){
    struct sockaddr_in stSockAddr;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(1100);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;

    bind(SocketFD,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in));

    listen(SocketFD, 10);
    return SocketFD;
}

int createClient(string ip, int port){
	
	struct sockaddr_in stSockAddr;
    int SocketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(1100);
    stSockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in));
	return SocketFD;
}
void aPeerLeft(string ip){
	for(int i=0;i<neibots.size();++i){
		if(neibots[i] == ip){
			neibots.erase(neibots.begin()+i);
			break;
		}
	}
}
void aPeerJoin(string ip){
	neibots.push_back(ip);
}
void pPeer(){
	char buffer[17];
	bzero(buffer,17);
	read(SocketFD, buffer, 16);
	cout<<"pp:"<<string(buffer)<<endl;
	switch (buffer[0])
	{
	case 'J':
		aPeerJoin((string(buffer)).substr(1));
		break;
	case 'E':
		aPeerLeft((string(buffer)).substr(1));
		break;
	default:
		break;
	}
}
bool registrOnTracker(){
	char buffer[2];
	bzero(buffer,2);
	write(SocketFD, "R", 1);
	read(SocketFD,buffer,1);
	if(buffer[0] == 'I'){
		write(SocketFD, "192.168.110.72", 14);
		bzero(buffer,2);
		read(SocketFD,buffer,1);
	    printf(": %s \n",buffer);
		if(buffer[0] == 's') return true;
	}
	return false;
}
void getListPeer(){
	char buffer[10];
	char b[150];
	bzero(buffer,9);
	write(SocketFD, "L", 1);
	read(SocketFD,buffer,9); //recive el header
	string l = string(buffer);
	int x = stoi(l.substr(4,5)); //cant del cuerpo
	int ii = stoi(l.substr(1,3)); //cant del resto de peers
	if(ii == 0) return;//no mas peers
	read(SocketFD,b,x); //lee cuerpo
	string auxIp="";
	for(int i=0;i<ii;++i){
		if(b[i]==','){
			neibots.push_back(auxIp);
			auxIp = "";
			continue;
		}
		auxIp+=b[i];
	}
	neibots.push_back(auxIp);
//	cout<<string(b)<<endl;
}

void readServer(){
	char buffer[2];
//	string buf;
	for(;;){
		//cout<<".";
		bzero(buffer,2);
		read(SocketFD, buffer, 1);
	    printf(": %s \n",buffer);
		switch (buffer[0])
		{
		case 'A':
		//	cout<<"pp:"<<"si p"<<endl;
			write(SocketFD, "s", 1);
			break;
		case 'P': /// aqui si puedes cambia a J y E para peerJoin y peerLeft
			pPeer();
			break;
		default:
			break;
		}
		if(SocketFD < 0){
			cout<<"saldra"<<endl;
			break;
		}
	}
}
int main(int argc, char **argv)
{
	SocketFD = createClient("127.0.0.0",1100);
	if(registrOnTracker()){
		cout<<"registrated"<<endl;
		//getListPeer(); ///esta funcion me sale error, no la he revisado XD, creo q era porque leia de más
		thread(readServer).detach();
	}
	char a;
	cin>>a;
    shutdown(SocketFD, SHUT_RDWR);
	close(SocketFD);
	return 0;
}

  
 // fuser -k -n tcp 1100