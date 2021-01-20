#include "CLogger.hpp"
#include <time.h>
#include <unistd.h>

CLogger* CLogger::instance = NULL;              
//CLogger uses the Singleton pattern because there must not be multiple instances of this class

FILE* CLogger::fptr = NULL; 

CLogger* CLogger::getInstance(){
    if(!CLogger::instance){
        CLogger::instance=new CLogger();
    }
    return CLogger::instance;
}

void CLogger::destroyInstance(){
    if(CLogger::instance){
        delete CLogger::instance;
        CLogger::instance = NULL;
    }
}

CLogger::CLogger(){
    time_t now = time(0); //current date/time based on current system
    struct tm *t = localtime(&now);
    char text[50];
    strftime(text, sizeof(text)-1, "%d-%m-%Y-%H:%M", t); // convert now to string form
    string filename=text;
    filename+=".log";
    //printf("numele fisierului e %s\n", filename.c_str());
    //fptr = fopen(filename.c_str(),"w");
    //if(fptr == NULL){
        //printf("Log file could not be created!\n");   
        //_exit(1);             
    //}
}

CLogger::~CLogger(){
    //fclose(fptr);
}

void CLogger::printClientEvent(string& clientIP, string& clientPort, string receivedMsg){
    printCurrTime();
    printf("FROM (%s, %s) %s\n", clientIP.c_str(), clientPort.c_str(), receivedMsg.c_str());
    //fprintf(fptr, "FROM (%s, %s) %s\n", clientIP.c_str(), clientPort.c_str(), receivedMsg.c_str());
}

void CLogger::printServerResponse(string& clientIP, string& clientPort, string& response){
    printCurrTime();
    printf("TO (%s, %s) %s\n", clientIP.c_str(), clientPort.c_str(), response.c_str());
    //fprintf(fptr, "%s: TO (%s, %s) %s\n", clientIP.c_str(), clientPort.c_str(), response.c_str());
}

void CLogger::printClientConnect(char* clientIP, int clientPort){
    printCurrTime();
    printf("CONNECTED (%s, %d)\n", clientIP, clientPort);
    //fprintf(fptr, "CONNECTED (%s, %d)\n", clientIP, clientPort);
}

void CLogger::printClientDisconnect(char* clientIP, int clientPort){
    printCurrTime();
    printf("DISCONNECTED (%s, %d)\n", clientIP, clientPort);
    //fprintf(fptr, "DISCONNECTED (%s, %d)\n", clientIP, clientPort);
}

void CLogger::printCurrTime(){
    time_t now = time(0); //current date/time based on current system
    struct tm *t = localtime(&now);
    char text[50];
    strftime(text, sizeof(text)-1, "%H:%M", t);
    printf("%s ", text);
    //fprintf(fptr, "%s ", text);
}