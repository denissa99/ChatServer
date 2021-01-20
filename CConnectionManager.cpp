#include "CConnectionManager.hpp"
#include "CLogger.hpp"
#include <unistd.h>                 
//read si write, exit
#include <string.h>
#include <sys/types.h>              
//contine definitiile diferitor tipuri de date folosite in apelurile de sistem
#include <sys/socket.h>             
//contine definitiile structurilor necesare pentru socket-uri
#include <netinet/in.h>             
//contine constante si structuri necesare pentru adrese internet
#include <arpa/inet.h>              
//inet_addr, inet_ntoa
#include <sys/poll.h>

CConnectionManager::CConnectionManager(){

}

CConnectionManager::CConnectionManager(string IPaddress, int portNumber){
    serverSocket=new CSocket(IPaddress, portNumber);
    stopThread1=1;
}

CConnectionManager::~CConnectionManager(){
    stop();                         
    //serverul se poate inchide atunci cand clientii sunt conectati, si prin urmare trebuie sa inchidem conexiunile

    list<CSocket*>::iterator it;
    for(it=clientSocketList.begin(); it!=clientSocketList.end();++it){
        delete *it;
        *it=NULL;
    }
    clientSocketList.clear();
    clientPollSet.clear();

    delete serverSocket;
    serverSocket=NULL;
}

void CConnectionManager::setStopThread1(int value){
    stopThread1=value;
}

CSocket* CConnectionManager::getServerSocket(){
    return serverSocket;
}

void CConnectionManager::listenForConnections(int maxNumOfConnections){
    listen(serverSocket->getSocketDescriptor(), maxNumOfConnections); 
    /*
    5 is the maximum size permitted by most systems - the number of connections that can be waiting while the process is handling a particular one
    f the first argument is a valid socket the call cannot fail
    */
}

void* CConnectionManager::acceptConnections(){
    int clientSocketDescriptor;
    struct sockaddr_in clientSocketAddress;
    int clientSocketAddressLength = sizeof(clientSocketAddress);

    char clientIP[20];
    int clientPort;

    while(stopThread1){
    if((clientSocketDescriptor=accept(serverSocket->getSocketDescriptor(), (struct sockaddr *)&clientSocketAddress, (socklen_t *)&clientSocketAddressLength))==-1){//causes the process to block until a client connects to the server, and then wakes up
        if(!stopThread1){
            /*
            adica la shutdown nu semnalizeaza nimic
            aici se presupune ca stopThread1 se schimba pana la if, e 1 la inceput si pana la if ajunge 0
            la urmatoare iteratie se iese din while
            */
        }
        else{
            perror("Accept error");
		    _exit(1);
        }
    }
    else{
        strcpy(clientIP, inet_ntoa(clientSocketAddress.sin_addr));
        clientPort=ntohs(clientSocketAddress.sin_port);
        //printf("Connected %s %d\n", clientIP, clientPort);
        CLogger::printClientConnect(clientIP, clientPort);
        
        CSocket* client=new CSocket();
        client->setDescriptor(clientSocketDescriptor);
        client->setAddress(clientSocketAddress);
        addClientToList(client);
        addClientToPollSet(clientSocketDescriptor);
    }
    }
    pthread_exit(NULL);
}

void CConnectionManager::addClientToList(CSocket* clientSocket){
    clientSocketList.push_back(clientSocket);
}

void CConnectionManager::addClientToPollSet(int clientSocketDescriptor){
    pollfd client;
    client.fd=clientSocketDescriptor;
    client.events=POLLIN;
    clientPollSet.push_back(client);
}

vector<pollfd>& CConnectionManager::getClientPollSet(){
    return clientPollSet;
}

CSocket* CConnectionManager::findPollFDinList(int pollDescriptor){
    list<CSocket*>::iterator it;
    for(it=clientSocketList.begin(); it!=clientSocketList.end();++it){
        if((*it)->getSocketDescriptor()==pollDescriptor)
             return *it;
    }
    return NULL;
}

void CConnectionManager::deleteSocket(CSocket* clientToDelete, int index){
    clientSocketList.remove(clientToDelete);
    clientPollSet.erase(clientPollSet.begin()+index);
    shutdown(clientToDelete->getSocketDescriptor(), 2);
    close(clientToDelete->getSocketDescriptor());
}

void CConnectionManager::stop(){
    list<CSocket*>::iterator it;
    for(it=clientSocketList.begin(); it!=clientSocketList.end();++it){
        int descriptor=(*it)->getSocketDescriptor();
        shutdown(descriptor, 2);
        close(descriptor);
    }
    shutdown(serverSocket->getSocketDescriptor(), 2);
    close(serverSocket->getSocketDescriptor()); 
    /*
    va scoate thread-ul cu accept din kernel, dand eroare, iar la verificare stopthread, thread exit, khaching
    Closing the listening socket is a right way to go. Then accept() returns -1 and sets errno to EBADF, as you already observed. 
    You just need some more logic in the "threading stuff" to analyze what have actually happened. 
    For example, test not_ended: if it is false, you know for sure that the error is intended, and that the shutdown is in progress; otherwise bla bla bla
    */
}

void CConnectionManager::insertPairToUserSocketMap(pair<string, int>& p){
    usernameToSocketMap.insert(p);
}

void CConnectionManager::deleteUserPairFromMap(string username){
    usernameToSocketMap.erase(username);
}

int CConnectionManager::getClientSocketDescriptorBasedOnUsername(string username){
    map<string, int>::iterator it;
    it=usernameToSocketMap.find(username);
    return it->second;
}

string CConnectionManager::getUsernameBasedOnClientSocketDescriptor(int clientDescriptor){
    map<string, int>::iterator it;
    for(it=usernameToSocketMap.begin(); it!=usernameToSocketMap.end();++it){
        if(it->second==clientDescriptor)
            return it->first;
    }
    return "";
}