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

//string ip_client = "192.168.110.72";	//ip juanito
string ip_client = "192.168.122.1";	//ip Anthony

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
    stSockAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in));
	return SocketFD;
}
void aPeerLeft(){
	char buffer[17];
	bzero(buffer,17);
	read(SocketFD, buffer, 15);
	cout<<"A peer left with ip:"<<string(buffer)<<endl;
	string ip = string(buffer);

	for(int i=0;i<neibots.size();++i){
		if(neibots[i] == ip){
			neibots.erase(neibots.begin()+i);
			break;
		}
	}
}
void aPeerJoin(){
	char buffer[17];
	bzero(buffer,17);
	read(SocketFD, buffer, 15);
	cout<<"A peer joint with ip:"<<string(buffer)<<endl;
	string ip = string(buffer);
	neibots.push_back(ip);
}

bool registrOnTracker(){
	char buffer[2];
	bzero(buffer,2);
	write(SocketFD, "R", 1);	//preguntar si se puede unir al torrent
	read(SocketFD,buffer,1);
	if(buffer[0] == 'I'){		// el tracker solicita la IP
		while(ip_client.size() < 15)	
			ip_client = "0" + ip_client;	// 0's delanteros de relleno 

		write(SocketFD, ip_client.c_str() , 15);
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
	read(SocketFD,buffer,4); //recibe el header [LO CAMBIE](ahora solo recibe L002, L y la cantidad de peers que hay, los ips todos serán de tamaño 15)
	
	cout << "buffer: " << buffer << endl;

	string l = string(buffer);
	int num_peers = stoi(l.substr(1,3)); //cantidad de peers
	if(num_peers == 0) return;//no hay mas peers
	read(SocketFD, b , 15*num_peers); //lee cuerpo
	string auxIp = string(b);
	for(int i=0; i < 15*num_peers; i=i+15 ){//los ips van cada 15 sin coma, todo junto
		neibots.push_back(auxIp.substr(i,15));
	}

	cout << "cuerpo: " << b << endl;
}

void readServer(){
	char buffer[2];
//	string buf;
	for(;;){
		//cout<<".";
		bzero(buffer,2);
		read(SocketFD, buffer, 1);
	    printf("Message to client: %s \n",buffer);
		switch (buffer[0])
		{
		case 'A':
		//	cout<<"pp:"<<"si p"<<endl;
			write(SocketFD, "s", 1);
			break;
		case 'J': ///peerJoin 
			aPeerJoin();
			break;
		case 'E': ///PeerLeft
			aPeerLeft();
			break;
		default:
			break;
		}
	}
}

int main(int argc, char **argv)
{
	SocketFD = createClient(ip_client,1100);
	if(registrOnTracker()){
		cout<<"registrated"<<endl;
		//getListPeer(); 
		thread(readServer).detach();
	}
	else{
		cout << "fallo al registrar en tracker" << endl;
	}

    shutdown(SocketFD, SHUT_RDWR);
	close(SocketFD);
	return 0;
}

  
 // fuser -k -n tcp 1100