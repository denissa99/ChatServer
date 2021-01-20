#include "CSocket.hpp"
#include <list>
#include <vector>
#include <map>

class CConnectionManager{
public:
    CConnectionManager();
    CConnectionManager(string IPaddress, int portNumber);
    ~CConnectionManager();
    void setStopThread1(int value);
    void listenForConnections(int maxNumOfConnections);
    void* acceptConnections();  //daca nu ar fi fost un thread, atunci nu am fi ajuns la read in main fiindca e un while infinit
    vector<pollfd>& getClientPollSet();
    CSocket* findPollFDinList(int pollDescriptor);
    void deleteSocket(CSocket* clientToDelete, int index);
    void insertPairToUserSocketMap(pair<string, int>& p);
    void deleteUserPairFromMap(string username);
    int getClientSocketDescriptorBasedOnUsername(string username);
    string getUsernameBasedOnClientSocketDescriptor(int clientDescriptor);
private:
    CSocket* serverSocket;
    list<CSocket*> clientSocketList;
    vector<pollfd> clientPollSet;
    map<string, int> usernameToSocketMap;
    CSocket* getServerSocket();
    void addClientToList(CSocket* clientSocket);
    void addClientToPollSet(int clientSocketDescriptor);
    int stopThread1;
    int stopThread2;
    void stop();
};