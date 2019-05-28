/*************************************************/
////////////////////// PEER ///////////////////////
/*************************************************/
#include "lib/cliser.h"
#include <unistd.h>
#include <vector>
#include <utility>

using namespace std;

int SocketClient;

///////     port & ip   ////////
vector<pair<int, string>> neibots;
///puertos por defecto : 110x

string my_ip = "127.0.0.1";
int my_port=1101;
//bool registrado =0
void ToList(int ClienSock){
    char buf[5];
    char buff[200];
    write(ClienSock,"L",1);
    read(ClienSock,buf,4);
    int num_peers = stoi(string(buf).substr(1))*19;
    read(ClienSock,buff,num_peers);
    string auxIp = string(buff);
    cout<<"--:-- "<<auxIp<<endl;
    int temPort;
    for(int i=0; i < 15*num_peers; i=i+15 ){//los ips van cada 15 sin coma, todo junto
        temPort = stoi(auxIp.substr(i,4));
        string aux = auxIp.substr(i+4,15);
        while(aux[0] == '-'){
            aux = aux.substr(1);
        }
        neibots.push_back(make_pair(temPort,aux));
    }
}
bool MakeRegistre(int portTracker){
    SocketClient = createClient("127.0.0.1",portTracker);
    string mss = "R";
    mss+= to_string(my_port);
    mss+=my_ip;
    while(mss.size() < 20)	
        mss.insert(5,"-");	// 0's delanteros de relleno 
    write(SocketClient, mss.c_str() , 20);
    char buf[3];
    bzero(buf,2);
    read(SocketClient,buf,2);
    if(buf[1]=='s'){
        //ToList(SocketClient);
        char buf[5];
        char buff[200];
        write(SocketClient,"L",1);
        read(SocketClient,buf,4);
        cout<<">-< "<<string(buf)<<endl;
        int num_peers = stoi(string(buf).substr(1))*19;
        if(num_peers != 0){
            read(SocketClient,buff,num_peers);
            cout<<num_peers<<">< "<<string(buff)<<endl;
            string auxIp = string(buff).substr(0,num_peers);
            cout<<"--:-- "<<auxIp<<" |"<<auxIp.size()<<endl;
            int temPort;
            for(int i=0; i < num_peers; i=i+19 ){//los ips van cada 15 sin coma, todo junto
                temPort = stoi(auxIp.substr(i,4));
                if(temPort == my_port)continue;
                cout<<"-:- "<<temPort<<endl;
                string aux = auxIp.substr(i+4,15);
                cout<<"-- "<<aux<<endl;
                while(aux[0] == '-'){
                    aux = aux.substr(1);
                }
                neibots.push_back(make_pair(temPort,aux));
            }
        }
        write(SocketClient, "x" , 1);
        shutdown(SocketClient, SHUT_RDWR);
    	close(SocketClient);
        return true;
    }
    write(SocketClient, "x" , 1);
    shutdown(SocketClient, SHUT_RDWR);
	close(SocketClient);
    return false;
}
void APL(string peper){
    int paux= stoi(peper.substr(1,4));
    string ip = peper.substr(5);
    while(ip[0] == '-'){
        ip = ip.substr(1);
    }
    for(int i=0;i<neibots.size();i++){
        if(neibots[i].first == paux && neibots[i].second == ip){
            neibots.erase(neibots.begin()+i);
            break;
        }
    }
}
void APJ(string peper){
    int paux= stoi(peper.substr(1,4));
    string ip = peper.substr(5);
    while(ip[0] == '-'){
        ip = ip.substr(1);
    }
    if(my_port == paux && my_ip == ip)return;
    neibots.push_back(make_pair(paux,ip));
}
void lanza(int socketSer){
    for(;;){
        int ConnectFD = accept(socketSer, NULL, NULL);
        if(ConnectFD < 0){
            //cout<<"nah"<<endl;
            continue;
        }else if(ConnectFD >0){
            char buffer[21];
            bzero(buffer,20);
            read(ConnectFD,buffer,20);
            switch (buffer[0]){
            case 'A':
//                cout<<"pp:"<<"si p"<<endl;
			    write(ConnectFD, "As", 2);
			    break;
            case 'E':
                cout<<"muerto "<<string(buffer)<<endl;
			    APL(string(buffer));
                //write(ConnectFD, "As", 2);
			    break;
            case 'J':
                cout<<"nuevo "<<string(buffer)<<endl;
                APJ(string(buffer));
			    //write(ConnectFD, "As", 2);
			    break;
            default:
                break;
            }
        }
        //peers.push_back(make_pair(ConnectFD,""));
      //  if(peers.empty())continue;
    }
}
void seeMenu(){
    system("clear");
    cout<<"----------- HAR TORRENT -----------"<<endl;
    cout<<"<<<<<<<<<<<<<<<  o  >>>>>>>>>>>>>>>"<<endl;
    cout<<endl;
    cout<<"Choose and option:"<<endl;
    cout<<" ======================================"<<endl;
    cout<<"|   1  ->  Register on Tracker         |"<<endl;
    cout<<"|   2  ->  Download archive            |"<<endl;
    cout<<" ======================================"<<endl;
    cout<<">>";
}

void requestDataOfArchive(){
    string arch;
    cout<<"What archive do you want download? ";
    //getline(cin,arch);
    cin>>arch;
    string dirr = "./peer";
    dirr += "xx";//"xx" -> peer id
    dirr += "/downloads/";
    dirr += arch;
    cout<<"Complete, your archive is in "<<dirr<<endl;;
    cin>>arch;
}
int main(int argc, char **argv)
{
    char op;
    cout<<"Create a empty peer?(y/n) ";cin>>op;
    string ipp;
    switch (op){
    case 'n':
        /* code */
        break;
    case 'y':
        cout<<"Your ip: ";
        cin>>ipp;
        my_ip = ipp;
        cout<<"Your port you'll use: ";
        cin>>my_port;
        break;    
    default:
        return 0;
        break;
    }
    bool continu = true;
    cin>>op;
    int SocketServer = createServer(my_port);
    seeMenu();
    while(continu){
        cin>>op;
        switch (op){
        case '1':
            system("clear");
            if(MakeRegistre(1100)){
                cout<<"You're registered"<<endl;
                //int SocketServer = createServer(my_port);
                thread(lanza,SocketServer).detach();
            }
            else 
                cout<<"You aren't registered"<<endl;
            cin>>op;
            break;
        case '2':
            system("clear");
            requestDataOfArchive();
//            Download();
            break;
        default:
            break;
        }
        seeMenu();
    }
	return 0;
}

  
 // fuser -k -n tcp 1100

 /** solo lee el thread */