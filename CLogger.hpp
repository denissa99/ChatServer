#include <stdio.h>
#include <stdlib.h>
#include <string>
using namespace std;

class CLogger{
public:
    static CLogger* getInstance();
    static void destroyInstance();
    static void printClientEvent(string& clientIP, string& clientPort, string receivedMsg);
    static void printServerResponse(string& clientIP, string& clientPort, string& response);
    static void printClientConnect(char* clientIP, int clientPort);
    static void printClientDisconnect(char* clientIP, int clientPort);
private:
    static CLogger* instance; 
    CLogger();
    ~CLogger();
    static FILE* fptr;
    static void printCurrTime();
};