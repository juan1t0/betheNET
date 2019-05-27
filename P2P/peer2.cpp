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
        /*pedir lista*/
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
                cout<<"pp:"<<"si p"<<endl;
			    write(ConnectFD, "As", 2);
			    break;
            case 'E':
                cout<<"muerto "<<string(buffer)<<endl;
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