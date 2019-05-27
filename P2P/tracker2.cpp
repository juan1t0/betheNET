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
vector<int> bolper;

//    write(socket, aux.c_str(), 2);
void reset_bolper(){
    for(int i=0;i<bolper.size();i++){
        if(bolper[i]==3) continue;
        bolper[i]=0;
    }
}
//////////////////puede que se use luego//////////////
void Enviar(){
    int timen = 0;
    bool d = 0;
    for(;;){
        while(peers.size()>=1){
            d=1;
            for(size_t x=0;x<peers.size();++x){
                if(peers[x].second != ""){
                    d = 0; ///No todos han sido aceptados
                }
            }
            if(!d)continue;
            usleep(seg);//
            timen++;
            //cout<<timen<<"\' time"<<endl;
        }
    }
}
///////////////////////////////////////

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
void aPeerJoin(string ip){
    string m = "J" + ip;
    for(int i =0; i<peers.size();++i){
        write(peers[i].first, m.c_str(), 16);
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
    aPeerJoin(mss.substr(1));
    peers.push_back(make_pair(paux,ip));
    bolper.push_back(1);
    return 1;//confirma registro
}

void getList(int sok){
    int cnt=0;
    string list = "l";
    for(int i =0; i<peers.size();++i){
        if(peers[i].first != sok) { //los ips van juntos sin coma
            cnt++;
            list +=  peers[i].second;
        }
    }
    if(cnt==0){
        write(sok, "L000", 4);
        return;
    }
    string cnstr = to_string(cnt);
    while (cnstr.length() < 3) cnstr.insert(0,"0");
    cnstr = "L" + cnstr;
    write(sok, cnstr.c_str(), 9);//envia header
    write(sok, list.c_str(), list.length()); //el cuerpo
}
/*
void checkPeers(){
    for(int i=0;i<bolper.size();i++){
        if(bolper[i]==0){
            cout<<"morira "<<peers[i].second<<"_"<<i<<endl;
            aPeerLeft(peers[i].second);
            cout<<"muerto"<<endl;
            peers.erase(peers.begin()+i);
            bolper.erase(bolper.begin()+i);
          //  i--;
        }
    }
}*/

void chek_is_alive(){
    clock_t start = clock();
    char buff[3];
    for(;;){
    //    char buffer[2];
        while(peers.size()>0){
            double ques = (double) (clock() - start)/CLOCKS_PER_SEC;
            if(ques >= 5){//cada 10 seg
            for(size_t x = 0; x < peers.size(); ++x){
                int tempClient = createClient(peers[x].second,peers[x].first);
                cout<<tempClient<<"*-"<<endl;
                int n=write(tempClient, "A", 1);//pregunta
                cout<<n<<"**"<<endl;
                if(n <= 0){
                    cout<<peers[x].second<<" mueto"<<endl;
                    aPeerLeft(peers[x],tempClient);
                    peers.erase(peers.begin()+x);
                }
                bzero(buff,2);
                int nn = read(tempClient,buff,2);
                if(nn<=0){
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
        printf(": %s \n",mensaje);
        switch (mensaje[0]){
        case 'R':
            if(registr(string(mensaje).substr(1)))
                write(SocketFD,"Rs",2);
            else 
                write(SocketFD,"Rn",2);
            break;
        case 'L':
            getList(SocketFD);
            break;
        case 'x':
            //if(SocketFD != peers[el_indice].first)continue;
/*            printf(":: %c \n",mensaje[1]);
            for(size_t x = 0; x < peers.size(); ++x){
                if(peers[x].first == SocketFD){
                    cout<<"<- "<<peers[x].first<<endl;
                    if(mensaje[1]=='s'){
                        bolper[x] = 1;//esta activo
                    }
                }
            ^â^respuesta de vivo 
            }*/
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