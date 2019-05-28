/*************************************************/
////////////////////// PEER ///////////////////////
/*************************************************/
#include "lib/cliser.h"
#include <unistd.h>
#include <vector>
#include <utility>
#include <fstream>

#define INF 10000
using namespace std;

int SocketClient;

///////     port & ip   ////////
vector<pair<int, string>> neibots;
///puertos por defecto : 110x

const int number_peers = 3; //number of peers to connect
string my_ip = "127.0.0.1";
int my_port=1101;

bool MakeRegistre(int portTracker){
    SocketClient = createClient("127.0.0.1",portTracker);
    string mss = "R";
    mss+= to_string(my_port);
    mss+=my_ip;
    while(mss.size() < 20)	
        mss.insert(4,"-");
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

//----------------------- DOWNLOAD AND UPLOAD A FILE -------------------------------//
//SERVER
int get_num_chunks(string file_name){   //0037001 -> FILE ID = 37, NUM OF CHUNK = 1
                                        //la mision es encontrar la parte restante(NUMBER OF CHUNKS OF THE FILE)

    return 1;

}

void upload( int client_socket, string file_name){    //0037001010 -> FILE ID = 37, NUM OF CHUNK = 1, NUMBER OF CHUNKS OF THE FILE = 10
    string aux_text, text;
    std::ifstream ifs ( (file_name + ".txt").c_str() );

    if (ifs.is_open()) {    //si existe el archivo
        while(getline(ifs, aux_text)){
            text += aux_text + "\n";
        }
        string text_size = to_string(text.size());   //tres primeros caracteres seran el tamano del archivo
        while(text_size.size() < 3) text_size.insert(0,"0");

        text = "S" + text_size + text;
        write(client_socket, text.c_str() , text.size());     //responde con S005hola. S, el tamanio del texto y luego el texto
    }   
    else{
        write(client_socket, "N" , 1);     //responde con N si no existe el chunk pedido
    }

    shutdown(client_socket, SHUT_RDWR);
    close(client_socket);
}

//CLIENT
inline int hash_(string filename){
    int sum = 0;
    for(int i=0; i<filename.size();++i){
        sum += (int)filename[i];
    }
    return sum/filename.size();
}

int num_chunks;
int chunks_obtained;

bool download( string filename, int peer_pos, string dir){
    char buf[1000];
    int ServerSocket = createServer(neibots[peer_pos].first);
    write(ServerSocket, ("U" + filename).c_str() , 11);

    bzero(buf,1000);
    read(ServerSocket,buf,1);

    if(buf[0] == 'N'){
        shutdown(ServerSocket, SHUT_RDWR);
        close(ServerSocket);
        return 0;
    }
    else if(buf[0] == 'S'){
        bzero(buf,1000);
        read(ServerSocket,buf,3);

        int text_size = atoi(buf);

        bzero(buf,1000);
        read(ServerSocket,buf,text_size);

        ofstream out( (dir + filename +".txt").c_str() );
        out << string(buf);
        out.close();

        chunks_obtained += 1;
        
        shutdown(ServerSocket, SHUT_RDWR);
        close(ServerSocket);
        return 1;
    }
}

void get_list_of_chunks(int  file_id_, vector<int> random_pos, string dir){
    string file_id = to_string(file_id_);
    while (file_id.size() < 4) file_id.insert(0,"0");

    string filename = file_id + "000";//0037001 -> FILE ID = 37, NUM OF CHUNK = 1, PRIMER CHUNK A BUSCAR ES 0, EN BASE A ESE SE SABE LA CANTIDAD DE CHUNKS
    num_chunks = get_num_chunks(filename);

    string str_num_chunks = to_string(num_chunks);
    while (str_num_chunks.size() < 3) str_num_chunks.insert(0,"0");
    
    for(int i=0; i<num_chunks; ++i){
        string str_c = to_string(i);
        while (str_c.size() < 3) str_c.insert(0,"0");

        filename = file_id + str_c + str_num_chunks;
        for(int j=0; j<random_pos.size(); ++j){
            thread(download, filename, random_pos[j], dir).detach();
        }
    }
}

void requestDataOfArchive(){
    num_chunks = INF;
    chunks_obtained = 0;
    string arch;

    cout<<"What archive do you want download? ";
    //getline(cin,arch);
    cin>>arch;
    
    string dirr = "/peer/";
    dirr += to_string(my_port);//"xx" -> peer id
    dirr += "/downloads/";
    //dirr += arch + ".txt";
    system( ("mkdir -p " + dirr).c_str() );   //crear carpeta

    int checksum = hash_(arch);
    
    vector<int> random_pos;
    vector<int>availables_pos(neibots.size());//crear vector de todas las posiciones disponibles
    for(int i=0; i< neibots.size();++i){
        availables_pos[i]=i;
    }

    do{
        if(neibots.size() <= number_peers){
            random_pos = availables_pos;
        }
        else{
            random_pos.clear();
            for(int i=0; i < number_peers;++i){
                random_pos.push_back(rand() % availables_pos.size());
                availables_pos.erase(availables_pos.begin()+random_pos[random_pos.size()-1]);
            }
        }
        get_list_of_chunks(checksum, random_pos, dirr);
    }while( num_chunks != chunks_obtained || !availables_pos.empty());

    cout << "Complete, your archive is in " << dirr << endl;;
    cin >> arch;
}
//----------------------- END DOWNLOAD AND UPLOAD A FILE -------------------------------//


void server_thread(int socketSer){
    for(;;){
        char buffer[20];
        int ConnectFD = accept(socketSer, NULL, NULL);
        if(ConnectFD < 0){
            cout<<"Server cannot connect to client"<<endl;
            continue;
        }else if(ConnectFD >0){
            bzero(buffer,20);
            read(ConnectFD,buffer,1);
            switch (buffer[0]){
            case 'U':      //U0037001010 -> FILE ID = 37, NUM OF CHUNK = 1, NUMBER OF CHUNKS OF THE FILE = 10
                bzero(buffer,20);
                read(ConnectFD,buffer,10);
                thread(upload, ConnectFD, string(buffer)).detach();
			    break;
            default:
                break;
            }
        }
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
                thread(server_thread,SocketServer).detach();
            }
            else 
                cout<<"You aren't registered"<<endl;
            cin>>op;
            break;
        case '2':
            system("clear");
            requestDataOfArchive();
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