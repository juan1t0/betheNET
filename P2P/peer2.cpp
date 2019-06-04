/*************************************************/
////////////////////// PEER ///////////////////////
/*************************************************/
#include "lib/cliser.h"
#include <unistd.h>
#include <vector>
#include <utility>
#include <fstream>

using namespace std;

int SocketClient;

///////     port & ip   ////////
vector<pair<int, string>> neibots;
vector< pair<string, vector<bool> > > my_chunks;
string my_ip = "127.0.0.1";
int my_port=0;
//bool registrado =0
string my_Dir = "";

void ToList(int ClienSock){
    char buf[5];
    char buff[200];
    write(SocketClient,"L",1);
    read(SocketClient,buf,4);
    //cout<<">-< "<<string(buf)<<endl;
    int num_peers = stoi(string(buf).substr(1))*19;
    if(num_peers != 0){
        read(SocketClient,buff,num_peers);
   //     cout<<num_peers<<">< "<<string(buff)<<endl;
        string auxIp = string(buff).substr(0,num_peers);
    //    cout<<"--:-- "<<auxIp<<" |"<<auxIp.size()<<endl;
        int temPort;
        for(int i=0; i < num_peers; i=i+19 ){//los ips van cada 15 sin coma, todo junto
            temPort = stoi(auxIp.substr(i,4));
            if(temPort == my_port)continue;
    //        cout<<"-:- "<<temPort<<endl;
            string aux = auxIp.substr(i+4,15);
   //         cout<<"-- "<<aux<<endl;
            while(aux[0] == '-'){
                aux = aux.substr(1);
            }
            neibots.push_back(make_pair(temPort,aux));
        }
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
        ToList(SocketClient);
        
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
            cout<<"hecho, ahora: "<<neibots.size()<<endl;
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
inline int hash_(string filename){
    int sum = 0;
    for(int i=0; i<filename.size();++i){
        sum += (int)filename[i];
    }
    return sum/filename.size();
}
bool download(vector<int>&TR, int fileId, int chunkId){
    string logmsn = "O";
    logmsn+= to_string(my_port);
    logmsn+=my_ip;
    while(logmsn.size() < 20)	
        logmsn.insert(5,"-");
    
    string file_id = to_string(fileId);
    while(file_id.size() < 4)
        file_id.insert(0,"0");
    string chunk_id = to_string(chunkId);
    while(chunk_id.size() < 3)
        chunk_id.insert(0,"0");
    
    char buffr[1050];
    
    if(chunkId < 0){
        for(int i=0;i<TR.size();++i){
            int DownClient = createClient(neibots[TR[i]].second,neibots[TR[i]].first);
            if(DownClient<0){
                continue;
            }
            write(DownClient,logmsn.c_str(),20);
            bzero(buffr,3);
            read(DownClient,buffr,2);
            if(buffr[0]=='O' && buffr[1]=='1'){
                string temp = "D";
                temp += file_id;
                temp += "any";
                cout<<temp<<"|......"<<endl;
                DownClient = createClient(neibots[TR[i]].second,neibots[TR[i]].first);
                if(DownClient<0){
                    continue;
                }
                int a=write(DownClient,temp.c_str(),temp.size());
                cout<<"escrito "<<a<<endl;
            }else{
                shutdown(DownClient,SHUT_RDWR);
                close(DownClient);
                continue;
            }
            bzero(buffr,1050);
            int a = read(DownClient,buffr,1050);
            cout<<a<<" somos"<<endl;
            if(buffr[0] == '-')continue;
            string aux = string(buffr);
            ofstream out ((my_Dir + aux.substr(0,10)+".txt").c_str() );
            out << aux.substr(10);
            out.close();
            my_chunks.push_back(make_pair(file_id,vector<bool>(stoi(aux.substr(7,3)),0)));
            my_chunks[my_chunks.size()-1].second[stoi(aux.substr(4,3))] = true;
            shutdown(DownClient,SHUT_RDWR);
            close(DownClient);
            return true;
        }
        return false;
    }else{
        for(int i=0;i<TR.size();++i){
            int DownClient = createClient(neibots[TR[i]].second,neibots[TR[i]].first);
            if(DownClient<0)
                continue;
            write(DownClient,logmsn.c_str(),20);
            bzero(buffr,2);
            read(DownClient,buffr,2);
            if(buffr[0]=='O' && buffr[1]=='1'){
                string temp = "D";
                temp += file_id;
                temp += chunk_id;////------------------------------rev
                write(DownClient,temp.c_str(),temp.size());
            }else{
                shutdown(DownClient,SHUT_RDWR);
                close(DownClient);
                continue;
            }
            bzero(buffr,1050);
            read(DownClient,buffr,1050);
            if(buffr[0] == '-'){
                shutdown(DownClient,SHUT_RDWR);
                close(DownClient);
                continue;
            }
            string aux = string(buffr);
            ofstream out ((my_Dir + aux.substr(0,10)+".txt").c_str() );
            out << aux.substr(10);
            out.close();
            my_chunks.push_back(make_pair(file_id,vector<bool>(stoi(aux.substr(7,3)),0)));
            my_chunks[my_chunks.size()-1].second[stoi(aux.substr(4,3))] = true;
            shutdown(DownClient,SHUT_RDWR);
            close(DownClient);
            return true;
        }
        return false;
    }
}
void requestFirstChunk(int fileID){
    vector<int> toReqest;
    bool first = 0;
    do{
        if(neibots.size() <= 3){
            toReqest = vector<int>(neibots.size());
        }
        else{
            toReqest.clear();
            for(int i=0; i < 3;++i){
                int x =rand() % neibots.size();
                for(int j=0;j<toReqest.size();++j){
                    if(toReqest[j]==x)
                        x = rand() % neibots.size();
                }
                toReqest.push_back(x);
            }
        }
        first = download(toReqest,fileID,-1);
    }while(!first);
}
void completeDownload(int fileID){
    bool incomplete = true;
    int index;
    vector<int> toReqest;
    string file_id = to_string(fileID);
    while(file_id.size() < 4)
        file_id.insert(0,"0");
    for(index=0;index<my_chunks.size();++index)
        if(my_chunks[index].first == file_id) break;
   
    while(incomplete){
        for(int i=0;i<my_chunks[index].second.size();++i){
            if(my_chunks[index].second[i]!=true){
                if(neibots.size() <= 3){
                    toReqest = vector<int>(neibots.size());
                }
                else{
                    toReqest.clear();
                    for(int i=0; i < 3;++i){
                        int x =rand() % neibots.size();
                        for(int j=0;j<toReqest.size();++j){
                            if(toReqest[j]==x)
                                x = rand() % neibots.size();
                        }
                        toReqest.push_back(x);
                    }
                }
                my_chunks[index].second[i] = download(toReqest,fileID,i);
                cout<<".";
                break;
            }
        }
        ///suma de bools que tengo, parar cuando tenga todos
        incomplete = false;
    }
}
void requestDataOfArchive(){
    string arch;
    cout<<"What archive do you want download? ";
    cin>>arch;
    string dirr = "downloads/peer" + to_string(my_port) + "/";
    system( ("mkdir -p " + dirr).c_str() );   //crear carpeta
    my_Dir = dirr;
    //int arcHash = hash_(arch);
    int arcHash = stoi(arch);
    requestFirstChunk(arcHash);
  //  completeDownload(arcHash);
    cin>>arch;
}
void upload(string filehash, string chunq,int soqtTO,int portPeer,string ipPeer){ //message type D
    string chun,aux;
    string auxDir = "downloads/peer" + to_string(my_port) + "/"; //////quitar a los otros el primer '/'
    cout<<"/"<<auxDir<<endl;
    string localT="-1";
    for(int index=0;index<my_chunks.size();++index){
        if(my_chunks[index].first == filehash){
            localT = to_string(my_chunks[index].second.size());
            if(chunq == "any"){
                for(int i=0;i<my_chunks[index].second.size();++i){
                    if(my_chunks[index].second[i]==true){
                        chunq = to_string(i);
                        while(chunq.size()<3)chunq.insert(0,"0");
                        break;
                    }    
                }
            }
            break;
        }
    }
    if(localT == "-1"){
        write(soqtTO,"-1",2);
        return;
    }
    while (localT.size() < 3) 
        localT.insert(0,"0");
    ifstream ifs ( (auxDir + filehash + chunq + localT +".txt").c_str() );
    cout<<":::"<<auxDir + filehash + chunq + localT +".txt"<<endl;
    if (ifs.is_open()) {    //si existe el archivo
        cout<<"open"<<endl;
        while(getline(ifs, aux)){
            chun += aux + "\n";
        }
        //string text_size = to_string(chun.size());   //tres primeros caracteres seran el tamano del archivo
        //while(text_size.size() < 3) text_size.insert(0,"0");

        chun = filehash + chunq + localT + chun;
        write(soqtTO,chun.c_str(),chun.size()); //responde con (name_of_chunk)(datos_del_chunk)
    }   
    else{
        cout<<"no open"<<endl;
        write(soqtTO,"-1",2);
    }
}

void loadChunk(){
    string nameChunk;
    cout<<"What's the name of your chunk?: (without extention)";//0037001003
    cin>>nameChunk;
    string file = nameChunk.substr(0,4);
    int actualchun = stoi(nameChunk.substr(4,3));
    int totalchun = stoi(nameChunk.substr(7,3));
    my_chunks.push_back(make_pair(file,vector<bool>(totalchun,0)));
    my_chunks[my_chunks.size()-1].second[actualchun] = true;

    char a;
    cin>>a;
}
void PeerServer(int socketSer){
    int toupPort=0;
    string toupIp="";
    string file="";
    string chu="";
    bool a=1;
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
            case 'O':
                toupPort = stoi((string(buffer)).substr(1,4));
                toupIp = (string(buffer)).substr(5);
                while(toupIp[0] == '-')toupIp = toupIp.substr(1);
                for(int i=0;i<neibots.size();++i){
                    if(neibots[i].first == toupPort && neibots[i].second == toupIp){
                        write(ConnectFD, "O1", 2);
                        a=0;
                        break;
                    }
                }
                if(a) write(ConnectFD, "O0", 2);
			    break;
            case 'D':
                file =(string(buffer)).substr(1,4);
                chu =(string(buffer)).substr(5,3);
                cout<<file<<" | "<<chu<<endl;
                upload(file,chu,ConnectFD,toupPort,toupIp);
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
void seeNeibots(){
    cout<<"       ------ LIST of PEERS ------"<<endl;
    cout<<endl;
    for(int i=0;i<neibots.size();++i){
        cout<<"Peer "<<i<<", with ip \""<<neibots[i].second<<"\" and use port "<<neibots[i].first<<endl;
    }
    char x;
    cin>>x;
}
void seeArchives(){
    cout<<"     ------ LIST of YOUR ARCHIVES ------"<<endl;
    cout<<endl;
    for(int i=0;i<my_chunks.size();++i){
        cout<<"* archivo \'"<<my_chunks[i].first<<"\' "<<endl;
    }
    char x;
    cin>>x;
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
    cout<<"|   3  ->  See neighborhood            |"<<endl;
    cout<<"|   4  ->  See actual files            |"<<endl;
    cout<<"|   5  ->  Load a single chunk         |"<<endl;
    cout<<" ======================================"<<endl;
    cout<<">>";
}
int main(int argc, char **argv)
{
    char op;
    cout<<"Create a empty peer?(y/n) ";cin>>op;
    string ipp;
    switch (op){
    case 'n':
        srand(time(NULL));
        my_port = rand() %100+1200;
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
                thread(PeerServer,SocketServer).detach();
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
        case '3':
            system("clear");
            seeNeibots();
            break;
        case '4':
            system("clear");
            seeArchives();
            break;
        case '5':
            system("clear");
            loadChunk();
            break;
        default:
            break;
        }
        seeMenu();
    }
    shutdown(SocketServer,SHUT_RDWR);
    close(SocketServer);
	return 0;
}

  
 // fuser -k -n tcp 1100

 /** solo lee el thread */

 /* revisar el connect de cliente, to keep alive */