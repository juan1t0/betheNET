/*************************************************/
///////////////////// TRACKER /////////////////////
/*************************************************/

#include "lib/cliser.h" 
#include <unistd.h>
#include <mutex>
#include <vector>
#include <utility>
#include <time.h>

#define seg 1000000 ///un segundo
#define min 60000000 ///un minuto

using namespace std;
mutex mtx;
vector<pair<int,string>> peers;

void aPeerLeft(pair<int,string> &dele,int sock){
    string m = "E" + dele.second;
    m.insert(1,to_string(dele.first));
    while(m.size() < 20)
        m.insert(5,"-");
    for(int i =0; i<peers.size();++i){
        if(peers[i].first != dele.first){
            write(sock,m.c_str(),20);
        }
    }
}
void aPeerJoin(string mss){
    string m = "J" + mss;
    for(int i =0; i<peers.size();++i){
//        int paux= stoi(mss.substr(0,4));
        int tempClient = createClient(peers[i].second,peers[i].first);
        write(tempClient, m.c_str(), 20);
        shutdown(tempClient, SHUT_RDWR);
        close(tempClient);
    }

}
bool registr(string mss){
//    char buffer[20];
    int paux= stoi(mss.substr(0,4));
    string ip = mss.substr(4);
    while(ip[0] == '-'){
        ip = ip.substr(1);
    }
    /**verificar si se repite, o algun error*/
    aPeerJoin(mss);
    peers.push_back(make_pair(paux,ip));
    return 1;//confirma registro
}

void getList(int sok){
    int cnt=0;
    string list = "";
    for(int i =0; i<peers.size();++i){
        //if(peers[i].first != ) { //los ips van juntos sin coma
            cnt++;
            string aux =  to_string(peers[i].first);
            aux +=  peers[i].second;
            while(aux.size() < 19)	
                aux.insert(4,"-");
        //} 
            list += aux;
    }
    if(cnt==0){
        write(sok, "L000", 4);
        return;
    }
    string cnstr = to_string(cnt);
    while (cnstr.length() < 3) cnstr.insert(0,"0");
    cnstr = "L" + cnstr;
    write(sok, cnstr.c_str(), 4);//envia header
    write(sok, list.c_str(), list.length()); //el cuerpo
}
void chek_is_alive(){
    clock_t start = clock();
    char buff[3];
    for(;;){
    while(peers.size()>0){
        double ques = (double) (clock() - start)/CLOCKS_PER_SEC;
        if(ques >= 3){ 
        for(size_t x = 0; x < peers.size(); ++x){
            int tempClient = createClient(peers[x].second,peers[x].first);//-1
            cout<<peers[x].second<<"*-"<<peers[x].first<<endl;
            cout<<tempClient<<"*+++-"<<endl;
            int n=write(tempClient, "A", 1);//pregunta
            cout<<n<<"**"<<endl;
            if(n <= 0){
                cout<<peers[x].second<<" mueto"<<endl;
                aPeerLeft(peers[x],tempClient);
                peers.erase(peers.begin()+x);
            }
            bzero(buff,2);
            int nn = read(tempClient,buff,2);
            if(nn<0){
                cout<<peers[x].second<<" mueto"<<endl;
                aPeerLeft(peers[x],tempClient);
                peers.erase(peers.begin()+x);
            }
            if(buff[0]=='A' && buff[1]=='s'){
                cout<<"somos"<<endl;
            }
            shutdown(tempClient, SHUT_RDWR);
            close(tempClient);
        }
            start = clock();
        }
    }
    }
}
void leer_de(int SocketFD){
    char mensaje[20];
    bool a = true;
    while(a){
        bzero(mensaje,20);
        read(SocketFD, mensaje, 20);
        //printf(": %s \n",mensaje);
        switch (mensaje[0]){
        case 'R':
            if(registr(string(mensaje).substr(1)))
                write(SocketFD,"Rs",2);
            else 
                write(SocketFD,"Rn",2);
            break;
        case 'L':
            printf(": %s \n",mensaje);
            getList(SocketFD);
            break;
        case 'x':
            a = false;
            break;
        default:
            break;
        }
    }
}

int main()
{
    int Socket = createServer(1100);
    thread(chek_is_alive).detach();
    for(;;){
        int ConnectFD = accept(Socket, NULL, NULL);
        if(ConnectFD < 0){
            cout<<"nah"<<endl;
            continue;
        }
        thread(leer_de, ConnectFD).detach();
        //peers.push_back(make_pair(ConnectFD,""));
        if(peers.empty())continue;
    }

    close(Socket);
    return 0;
}

 /** solo lee el thread */