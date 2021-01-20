#include <mysql/mysql.h>
#include <string>
#include <vector>
using namespace std;

class CDatabaseManager{
public:
    CDatabaseManager();
    ~CDatabaseManager();
    CDatabaseManager(string sqlServerUsername, string sqlServerPassword, string sqlServerDatabase);
    string getSignupResponse(string& username, string& password);
    string getLoginResponse(string& username, string& password);
    string getChangeUsernameResponse(string& newusername, string& oldusername);
    string getChangePasswordResponse (string& newpassword, string& username, string& oldpassword);
    string getInitMainWindowResponse(string& username);
    string getInitMessageWindowResponse(string& userRequesting, string& userRequested);
    string getSearchRequestResponse(string& username);
    string getSendMessageRequestResponse(string& userRequesting, string& userRequested, string& message, int& ID);
    string getReceiveMessageServiceResponse(string& userRequesting, string& userRequested, string& message);
    bool setOnlineField(string& username, int value);
    bool getOnlineField(string& username);
    string getDeleteConversationResponse(string& userRequesting, string& userRequested);
    //bool updateMessageSeenValue(int ID);
    //bool getOfflineMessages(string& userRequesting, string& userRequested, vector<string>& receiveMessages);
private:
    MYSQL* con;
    void printError(MYSQL* con);
    bool getCommandExitStatus(string command);
    int getNumberOfSelectedEntries();
    string stringifySignupSelect(string username);
    string stringifySignupInsert(string username, string password);
    string stringifyLoginSelect(string username, string password);
    string stringifyChangeUsernameUpdate(string newusername, string oldusername);
    string stringifyChangePasswordUpdate(string newpassword, string username, string oldpassword);
    string stringifyInitMainWindowSelect(string username);
    string stringifyInitMessageWindowSelect(string userRequesting, string userRequested);
    string stringifySearchRequestSelect(string username);
    string stringifySendMessageRequestInsert(string userRequesting, string userRequested, string message);
    string stringifyOnlineFieldUpdate(string& username, int value);
    string stringifyOnlineFieldSelect(string& username);
    //string stringifySeenValueUpdate(int ID);
    //string stringifyGetOfflineMessagesSelect(string& userRequesting, string& userRequested);
    string stringifySendMessageRequestSelectLastID(string& userRequesting, string& userRequested);
    void closeConnectionToServer();
    string stringifyDeleteConversation(string& userRequesting, string& userRequested);
};