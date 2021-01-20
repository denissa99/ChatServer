#include "CServer.hpp"
#include <stdio.h>                              
//printf 
#include <iostream>                             
//sha encryption, cin
#include <iomanip>                              
//sha encryption
#include <openssl/sha.h>                        
//sha encryption
#include <sstream>                              
//sha encryption
#include <string.h>                             
//strings
#include <unistd.h>                             
//read, write, exit
#include <sys/types.h>                          
//typedefinitions of data used in system calls
#include <sys/socket.h>                         
//definitions of used socket structures
#include <netinet/in.h>                         
//constants and structures used for internet addresses
#include <arpa/inet.h>                          
//inet_addr, inet_ntoa
#include <pthread.h>                            
//threads
#include <sys/poll.h>                           
//poll feature
#include <stdlib.h>

#define MAX 5                                   
//maximum number of connections to be listened

typedef void * (*THREADFUNCPTR)(void *);        
//the cast used for the thread function pointer

CServer* CServer::instance = NULL;              
//CServer uses the Singleton pattern because there must not be multiple instances of this class

CServer* CServer::getInstance(string IPaddress, int portNumber){
    if(!CServer::instance){
        CServer::instance=new CServer(IPaddress, portNumber);
    }
    return CServer::instance;
}

void CServer::destroyInstance(){
    if(CServer::instance){
        delete CServer::instance;
        CServer::instance = NULL;
    }
}

CServer::CServer(string IPaddress, int portNumber){ 
    connectionManager=new CConnectionManager(IPaddress, portNumber);
    databaseManager=new CDatabaseManager("phpmyadmin", "argint99","ChatServer");
    parser=new CParser();
    stopThread2=1;
    CLogger::getInstance();
}

CServer::~CServer(){
    delete connectionManager;
    connectionManager=NULL;

    delete databaseManager;
    databaseManager=NULL;

    delete parser;
    parser=NULL;
    CLogger::destroyInstance();
}

void CServer::initConnectionSetup(){                
    connectionManager->listenForConnections(MAX);   
    pthread_t thread1;                                                                                             
    /*
    because the program would get stuck in the accept system call until a new connection would be requested
    the new connection will be added in a connection list by the connection manager, and also in the pollset
    the other threads were made before this one and when they exited, because the main program was in the kernel during the accept system call, it will cause the error: accept invalid argument                                                                    
    */
    int rc1=pthread_create(&thread1, NULL, (THREADFUNCPTR) &CConnectionManager::acceptConnections, (void*) connectionManager);
    if (rc1) {
        printf("ERROR; return code from pthread_create() is %d\n", rc1);
        _exit(1);
    }
}

void CServer::readClientData(){
    pthread_t thread2;   
    /*
    infinite while loop that checks the connection list and the pollset and reads                           
    even if all the clients disconnected, it still checks
    */
    int rc2=pthread_create(&thread2, NULL, (THREADFUNCPTR) &CServer::readFromClients, (void*) this);
    /*
    C++ class member functions have a hidden this parameter passed in
    pthread_create() has no idea what value of this to use
    without providing any arguments, you have to use a static class method (which has no this parameter), or a plain ordinary function to bootstrap the class
    */    
    if (rc2) {                                                                                    
        printf("ERROR; return code from pthread_create() is %d\n", rc2);                            
        _exit(1);
    }
}

void CServer::readAdminCommands(){
    pthread_t thread3;                              
    //server administration - mainly exit
    
    int rc3=pthread_create(&thread3, NULL, (THREADFUNCPTR) &CServer::manageServer, (void*) this);
    if (rc3) {
        printf("ERROR; return code from pthread_create() is %d\n", rc3);
        _exit(1);
    }
    pthread_join(thread3, NULL);                    
    //wait till threads are complete before main continues
}

void* CServer::readFromClients(){
    list<CSocket*>::iterator it;
    while(stopThread2){
        /*
        -> bug interesant: cand un socket se deconecteaza, poll crede ca e ceva de citit, citeste ce am trimis ultima oara, si cand se incearca sa se scrie, thread-ul se termina din cauza semnalului SIGPIPE
        l-am reparat verificand return code-ul de la read
        sterg socketul si din lista de conexiuni si din setul de polling
        deoarece am modificat codul ca sa stearga descriptorii socketurilor deconectate, trebuie sa las sa recalculeze size-ul in for, ca sa nu am eroare cand am size-ul mai mic si incearca sa citeasca de la o locatie mai mare
        */
        vector<pollfd>& clientPollSet=connectionManager->getClientPollSet();
        poll(&clientPollSet[0], clientPollSet.size(), 5); 
        /*
        if timeout is greater than zero, it specifies a maximum interval (in milliseconds) to wait for any file descriptor to become ready
        if timeout is zero, then poll() will return without blocking. If the value of timeout is -1, the poll blocks indefinitely.
        timeout-ul e setat pe 5 ms ca sa nu se astepte o eternitate ca atunci cand ai pus 60000, *wink, wink #stupid
        */
        for(int i=0; i<clientPollSet.size();i++){
            if(clientPollSet[i].revents & POLLIN){
                startReading(clientPollSet[i].fd, i);
            }
        }
    }
    pthread_exit(NULL);
}

void CServer::startReading(int fd, int index){
    char receivedMsg[250]; 
    char clientIP[20];
    int clientPort;

    CSocket* clientSocket = connectionManager->findPollFDinList(fd);
    if (!clientSocket) {
        perror("Descriptor not found in pollset");
		_exit(1);
    }

    strcpy(clientIP, inet_ntoa(clientSocket->getSocketAddress().sin_addr));
    clientPort=ntohs(clientSocket->getSocketAddress().sin_port);

    int rc = read(clientSocket->getSocketDescriptor(), receivedMsg, sizeof(receivedMsg)); 
    //the read will block until there is smth to read in the socket, after the client has executed a write() - it returns the number of charachters read            
    
    if (rc==0){
        //printf("Disconnected %s %d\n", clientIP, clientPort);
        CLogger::printClientDisconnect(clientIP, clientPort);
        string username=connectionManager->getUsernameBasedOnClientSocketDescriptor(clientSocket->getSocketDescriptor());
        if(username!=""){                                           
            //va gasi username-ul asociat descriptorului doar cand a dat crash sau a dat close fara sa dea logout
            strcpy(receivedMsg,"logout`");
            strcat(receivedMsg, username.c_str());
            processMessage(clientSocket,receivedMsg);               
            //logout message
            connectionManager->deleteUserPairFromMap(username);     
            //delete pair from map
        }
        connectionManager->deleteSocket(clientSocket, index);
    }
    else{
        receivedMsg[rc+1]='\0';
        //printf("Message: %s\nFrom: %s %d\n", receivedMsg, clientIP, clientPort);
        processMessage(clientSocket, receivedMsg);
    }
}

void CServer::processMessage(CSocket* clientSocket, char* receivedMsg){  
    string clientIP=inet_ntoa(clientSocket->getSocketAddress().sin_addr);
    string clientPort=to_string(ntohs(clientSocket->getSocketAddress().sin_port));
    CLogger::printClientEvent(clientIP, clientPort, receivedMsg); 

    string response;

    string operationType;
    parser->getOperationType(receivedMsg, operationType);

    //string command;

    if(operationType=="signup"){
        string username;
        string password;
        parser->splitMessage(username, password);
        password=sha256(password);
        response=databaseManager->getSignupResponse(username, password);
        writeToClient(clientSocket->getSocketDescriptor(), response.c_str());
    }
    else if(operationType=="login"){
        string username;
        string password;
        parser->splitMessage(username, password);
        password=sha256(password);
        pair<string, int> p;
        p.first=username;
        p.second=clientSocket->getSocketDescriptor();
       // printf("PERECHE: %d %s\n", p.second, p.first.c_str());
        connectionManager->insertPairToUserSocketMap(p);
        response=databaseManager->getLoginResponse(username, password);
        writeToClient(clientSocket->getSocketDescriptor(), response.c_str());
    }    
    else if(operationType=="changeu"){
        string oldusername;
        string newusername;
        string password;
        parser->splitMessage(oldusername, newusername, password);
        //sa schimbi userul in map, ca nu o sa ii dea logout
        response=databaseManager->getChangeUsernameResponse(newusername, oldusername);
        writeToClient(clientSocket->getSocketDescriptor(), response.c_str());
    }    
    else if(operationType=="changep"){
        string username;
        string oldpassword;
        string newpassword;
        parser->splitMessage(username, oldpassword, newpassword);
        oldpassword=sha256(oldpassword);
        newpassword=sha256(newpassword);
        response=databaseManager->getChangePasswordResponse(newpassword, username, oldpassword);
        writeToClient(clientSocket->getSocketDescriptor(), response.c_str());
    }   
    else if(operationType=="logout"){
        string username;
        parser->splitMessage(username);
        connectionManager->deleteUserPairFromMap(username);
        if(databaseManager->setOnlineField(username, 0)){
            response="logout`yes";
            writeToClient(clientSocket->getSocketDescriptor(), response.c_str());
        }
        else{
            response="logout`no";
            writeToClient(clientSocket->getSocketDescriptor(), response.c_str());
        }
    }
    else if(operationType=="initmain"){
        string username;
        parser->splitMessage(username);
        response=databaseManager->getInitMainWindowResponse(username);
        writeToClient(clientSocket->getSocketDescriptor(), response.c_str());
    }
    else if(operationType=="initmess"){
        string userRequesting;
        string userRequested;
        parser->splitMessage(userRequesting, userRequested);
        response=databaseManager->getInitMessageWindowResponse(userRequesting, userRequested);
        writeToClient(clientSocket->getSocketDescriptor(), response.c_str());
        //vector<string> receiveMessages;
        //if(databaseManager->getOfflineMessages(userRequesting, userRequested, receiveMessages)){
            //for(auto i = receiveMessages.begin(); i != receiveMessages.end(); ++i)
                //writeToClient(clientSocket->getSocketDescriptor(), *i);
        //}
    }
    else if(operationType=="search"){
        string username;
        parser->splitMessage(username);
        response=databaseManager->getSearchRequestResponse(username);
        writeToClient(clientSocket->getSocketDescriptor(), response.c_str());
    }
    else if(operationType=="send"){
        string userRequesting;
        string userRequested;
        string message;
        int ID;
        parser->splitMessage(userRequesting, userRequested, message);
        response=databaseManager->getSendMessageRequestResponse(userRequesting, userRequested, message, ID);
        writeToClient(clientSocket->getSocketDescriptor(), response.c_str());     
        //insereaza mesajul in tabel si trimite confirmarea de primire clientului care a trimis mesajul
        if(databaseManager->getOnlineField(userRequested)){     //daca utilizatorul caruia ii este destinat mesajul e online
            response=databaseManager->getReceiveMessageServiceResponse(userRequesting, userRequested, message);
            writeToClient(connectionManager->getClientSocketDescriptorBasedOnUsername(userRequested), response.c_str());
            //databaseManager->updateMessageSeenValue(ID);
        }
    }
    else if(operationType=="typing"){
        string userRequesting;
        string userRequested;
        parser->splitMessage(userRequesting, userRequested);
        printf("typing`%s`%s\n", userRequesting.c_str(), userRequested.c_str());
        if(databaseManager->getOnlineField(userRequested)){ 
            response="typing`";
            response+=userRequesting;
            writeToClient(connectionManager->getClientSocketDescriptorBasedOnUsername(userRequested), response);
        }
    }
    else if(operationType=="delete"){
        string userRequesting;
        string userRequested;
        parser->splitMessage(userRequesting, userRequested);
        response=databaseManager->getDeleteConversationResponse(userRequesting, userRequested);
        writeToClient(clientSocket->getSocketDescriptor(), response.c_str());
    }
    else{
        response="Received incorrect message";
        writeToClient(clientSocket->getSocketDescriptor(), "Received incorrect message");
    }

    CLogger::printServerResponse(clientIP, clientPort, response);
}

void CServer::writeToClient(int socketDescriptor, string message){
    char* buffer=strdup(message.c_str());
    write(socketDescriptor, buffer, strlen(buffer)+1);
    delete buffer;
}

void* CServer::manageServer(){ 
    while(true){
        char command[10];
        scanf("%s", command);
        if(strcmp(command,"exit")==0){                          
            //killareste celelalte thread-uri, mwahahahaha
            stopThread2=0;
            connectionManager->setStopThread1(0); 
            /*
            degeaba fac stopThread1=0, fiindca atunci se afla in kernel, si thread-ul e blocat, nu se misca while-ul
            cum opresc thread-ul ala? aparent nu prea am cum daca e in kernel space
            lol, si scria in eroare, accept: Invalid argument Cannot find user-level thread for LWP 6288: generic error 
            am rezolvat in metoda stop din connection manager prin shutdown si close server socket  
            */
            break;
        }
    }
    pthread_exit(NULL);
}

string CServer::sha256(string& password){
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, password.c_str(), password.size());
    SHA256_Final(hash, &sha256);
    stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++){
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}