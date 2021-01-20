#include "CServer.hpp"
#include <unistd.h>         //_exit
#include <pthread.h>
#include <mysql/mysql.h>

int main(int argc, char** argv){

    if(argc<2){
        printf("Error - Enter server IP as an argument\n");
        printf("Ex. ./server 127.0.0.1\n");
        _exit(1);
    }

    CServer* server=CServer::getInstance(argv[1], 64000);

    server->initConnectionSetup();

    server->readClientData();

    server->readAdminCommands();
    
    server->destroyInstance();

    _exit(1);
}