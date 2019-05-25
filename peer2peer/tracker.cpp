/*************************************************/
///////////////////// TRACKER /////////////////////
/*************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <utility>
#include <time.h>

#define seg 1000000 ///un segundo
#define min 60000000 ///un minuto

using namespace std;
mutex mtx;
vector<pair<int,string>> peers;

//    write(socket, aux.c_str(), 2);

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
int create_server(int port){
    struct sockaddr_in stSockAddr;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
//    stSockAddr.sin_port = htons(port);
    stSockAddr.sin_port = htons(1100);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;

    bind(SocketFD,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in));

    listen(SocketFD, 10);
    return SocketFD;
}
void aPeerLeft(string ip){
    string m = "E" + ip;
    for(int i =0; i<peers.size();++i){
        write(peers[i].first, m.c_str(), 16);
    }
}
void aPeerJoin(string ip){
    string m = "J" + ip;
    for(int i =0; i<peers.size();++i){
        write(peers[i].first, m.c_str(), 16);
    }
}
void registr(int sok){
    char buffer[16];
    write(sok, "I", 1);///pide ip
    bzero(buffer,15);
    read(sok, buffer, 15); //lee ip
    printf(": %s \n",buffer);

    string ip = string(buffer);
    //recortar los 0 de adelante
    /*
    while(ip[0] == '0')
        ip = ip.substr(1,ip.size()-1);
    cout << "ip: " << ip << endl;
    */

    /**verificar si se repite, o algun error*/
    aPeerJoin(ip);
    peers.push_back(make_pair(sok,ip));
    write(sok, "s", 1);//confirma registro
    cout<<"sok: "<<sok<<endl; ///tiene este soket
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
void chek_is_alive(){
    clock_t start = clock();
    for(;;){
        char buffer[2];
        while(peers.size()>0){
            double ques = (double) (clock() - start)/CLOCKS_PER_SEC;
            if(ques >= 10){//cada 10 seg
                for(size_t x = 0; x < peers.size(); ++x){
   //                 mtx.lock();
                        cout<<"-> "<<peers[x].first<<endl;
                        write(peers[x].first, "A", 1);//pregunta
                        bzero(buffer,1);
                        read(peers[x].first, buffer, 1);//recive respuesta
                        printf(":: %s \n",buffer);
 //                   mtx.unlock();
                    if(buffer[0] != 's'){//si no es 's' morira
                        string ip = peers[x].second;
                        cout<<ip<<" muere"<<endl;
                        peers.erase(peers.begin()+x);
                        aPeerLeft(ip);
                    }
                }
                start = clock();
            }
        }
    }
}

void leer_de(int SocketFD){
    char mensaje[2];
    for(;;){
        bzero(mensaje,1);
        read(SocketFD, mensaje, 1);
        //printf(": %s \n",mensaje);
        switch (mensaje[0])
        {
        case 'R':
            registr(SocketFD);
            break;
        case 'L':
            getList(SocketFD);
            break;
        default:
            break;
        }
    }
}

int main()
{
    int Socket = create_server(1100);
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