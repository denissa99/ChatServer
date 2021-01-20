#include "CDatabaseManager.hpp"

CDatabaseManager::CDatabaseManager(){

}

CDatabaseManager::~CDatabaseManager(){
    closeConnectionToServer();
}

CDatabaseManager::CDatabaseManager(string sqlServerUsername, string sqlServerPassword, string sqlServerDatabase){
    con = mysql_init(NULL);

    if (con == NULL){
      fprintf(stderr, "%s\n", mysql_error(con));
      exit(1);
    }
    
    if (mysql_real_connect(con, "localhost", sqlServerUsername.c_str(), sqlServerPassword.c_str(), sqlServerDatabase.c_str(), 0, NULL, 0) == NULL){
      printError(con);
    }

    /*if (mysql_query(con, "SELECT * FROM Users")){
      printError(con);
    }

    MYSQL_RES *result = mysql_store_result(con);

    if (result == NULL){
      printError(con);
    }

    int num_fields = mysql_num_fields(result);

    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result))){
      for(int i = 0; i < num_fields; i++){
          printf("%s ", row[i] ? row[i] : "NULL");
      }
      printf("\n");
    }

    mysql_free_result(result);*/
}

void CDatabaseManager::printError(MYSQL* con){
    fprintf(stderr, "%s\n", mysql_error(con));
}

void CDatabaseManager::closeConnectionToServer(){
    mysql_close(con);
}

bool CDatabaseManager::getCommandExitStatus(string command){
    if (mysql_query(con, command.c_str())){
        printError(con);
        return false;
    }
    return true;
}

int CDatabaseManager::getNumberOfSelectedEntries(){
    MYSQL_RES *result = mysql_store_result(con);
    return mysql_num_rows(result);
}

string CDatabaseManager::stringifySignupSelect(string username){
    string command="SELECT Username FROM Users WHERE Username='";
    command+=username;
    command+="'";
    //printf("%s\n", command.c_str());
    return command;
}

string CDatabaseManager::stringifyLoginSelect(string username, string password){
    string command="SELECT Username, Password FROM Users WHERE Username='";
    command+=username;
    command+="' AND Password='";
    command+=password;
    command+="'";
    //printf("%s\n", command.c_str());
    return command;
}

string CDatabaseManager::stringifySignupInsert(string username, string password){
    string command="INSERT INTO Users VALUES('";
    command+=username;
    command+="','";
    command+=password;
    command+="',0)";
    //printf("%s\n", command.c_str());
    return command;
}

string CDatabaseManager::stringifyChangeUsernameUpdate(string newusername, string oldusername){
    string command="UPDATE Users SET Username='";
    command+=newusername;
    command+="' WHERE Username='";
    command+=oldusername;
    command+="'";
    //printf("%s\n", command.c_str());
    return command;
}

string CDatabaseManager::stringifyChangePasswordUpdate(string newpassword, string username, string oldpassword){
    string command="UPDATE Users SET Password='";
    command+=newpassword;
    command+="' WHERE Username='";
    command+=username;
    command+="' AND Password='";
    command+=oldpassword;
    command+="'";
    //printf("%s\n", command.c_str());
    return command;
}

string CDatabaseManager::stringifyInitMainWindowSelect(string username){
    string command;
    command+="SELECT latest.Username, Messages.Message FROM Messages INNER JOIN ";
    command+="(SELECT Username, MAX(Date) AS MaxDate FROM ";
    command+="(SELECT FromUsername AS Username, Date FROM Messages WHERE ToUsername='";
    command+=username;
    command+="'";
    command+=" UNION ";
    command+="SELECT ToUsername AS Username, Date FROM Messages WHERE FromUsername='";
    command+=username;
    command+="')temp GROUP BY Username)latest ";
    command+="ON ((Messages.FromUsername='";
    command+=username;
    command+="' AND Messages.ToUsername=latest.Username) OR ";
    command+="(Messages.FromUsername=latest.Username AND Messages.ToUsername='";
    command+=username;
    command+="')) AND (Messages.Date=latest.MaxDate) ORDER BY Date DESC";
    //printf("%s\n", command.c_str());
    return command;
}

string CDatabaseManager::stringifyInitMessageWindowSelect(string userRequesting, string userRequested){
    string command="SELECT FromUsername, Message FROM Messages WHERE FromUsername='";
    command+=userRequested;
    command+="' AND ToUsername='";
    command+=userRequesting;
    command+="' OR FromUsername='";
    command+=userRequesting;
    command+="' AND ToUsername='";
    command+=userRequested;
    command+="' ORDER BY Date";
    //printf("%s\n", command.c_str()); 
    return command;
}

string CDatabaseManager::stringifySearchRequestSelect(string username){
    string command="SELECT Username FROM Users WHERE Username LIKE '%";
    command+=username;
    command+="%'";
    //printf("%s\n", command.c_str());
    return command;
}

string CDatabaseManager::stringifySendMessageRequestInsert(string userRequesting, string userRequested, string message){
    string command="INSERT INTO Messages VALUES(NULL,'";
    command+=userRequesting;
    command+="','";
    command+=userRequested;
    command+="','";
    command+=message;
    command+="',NOW(),0)";
    //printf("%s\n", command.c_str());
    return command;
}

string CDatabaseManager::stringifyOnlineFieldUpdate(string& username, int value){
    string command="UPDATE Users SET Online=";
    command+=to_string(value);
    command+=" WHERE Username='";
    command+=username;
    command+="'";
    //printf("%s\n", command.c_str());
    return command;
}

string CDatabaseManager::stringifyOnlineFieldSelect(string& username){
    string command="SELECT Online FROM Users WHERE Username='";
    command+=username;
    command+="'";
    //printf("%s\n", command.c_str());
    return command;
}

/*string CDatabaseManager::stringifySeenValueUpdate(int ID){
    string command="UPDATE Messages SET SeenFlag=1 WHERE ID=";
    command+=to_string(ID);
    printf("%s\n", command.c_str());
    return command;
}

string CDatabaseManager::stringifyGetOfflineMessagesSelect(string& userRequesting, string& userRequested){
    string command="SELECT FromUsername, Message FROM Messages WHERE ToUsername='";
    command+=userRequested;
    command+="' AND FromUsername='";
    command+=userRequested;
    command+="' AND SeenFlag=0 ORDER BY Date";
    printf("%s\n", command.c_str());
    return command;
}*/

string CDatabaseManager::stringifySendMessageRequestSelectLastID(string& userRequesting, string& userRequested){
    string command="SELECT ID, MAX(Date) AS MaxDate FROM Messages WHERE FromUsername='";
    command+=userRequesting;
    command+="' AND ToUsername='";
    command+=userRequested;
    command+="' GROUP BY FromUsername, ToUsername";
    //printf("%s\n", command.c_str());
    return command;
}

string CDatabaseManager::stringifyDeleteConversation(string& userRequesting, string& userRequested){
    string command="DELETE FROM Messages WHERE FromUsername='";
    command+=userRequesting;
    command+="' AND ToUsername='";
    command+=userRequested;
    command+="' OR FromUsername='";
    command+=userRequested;
    command+="' AND ToUsername='";
    command+=userRequesting;
    command+="'";
    //printf("%s\n", command.c_str());
    return command;
}

string CDatabaseManager::getSignupResponse(string& username, string& password){
    string command=stringifySignupSelect(username);     //pt a verifica daca mai exista username-ul
    if(!getCommandExitStatus(command)){
        return "signup`no";
    }
    else{
        if(getNumberOfSelectedEntries())
            return "signup`no"; 
        else {
            command=stringifySignupInsert(username, password);
            if(!getCommandExitStatus(command))
                return "signup`no";
            else{
                return "signup`yes";
            }           
        }
    } 
}

string CDatabaseManager::getLoginResponse(string& username, string& password){
    string command=stringifyLoginSelect(username, password);
    if(!getCommandExitStatus(command)){
        return "login`no";
    }
    else{
        if(getNumberOfSelectedEntries()){
            if(setOnlineField(username, 1))
                return "login`yes";
            else
                return "login`no";
        }
        else {
            return "login`no";
        }    
    } 
}

string CDatabaseManager::getChangeUsernameResponse(string& newusername, string& oldusername){
    string command=stringifyChangeUsernameUpdate(newusername, oldusername);
    if(!getCommandExitStatus(command)){
        return "changeu`no";
    }
    else{
        if(mysql_affected_rows(con))
            return "changeu`yes";
        else
            return "changeu`no";
    }
}

string CDatabaseManager::getChangePasswordResponse (string& newpassword, string& username, string& oldpassword){
    string command=stringifyChangePasswordUpdate(newpassword, username, oldpassword);
    if(!getCommandExitStatus(command)){
        return "changep`no";
    }
    else{
        if(mysql_affected_rows(con))
            return "changep`yes";
        else
            return "changep`no";
    }
}

string CDatabaseManager::getInitMainWindowResponse(string& username){
    string command=stringifyInitMainWindowSelect(username);
    if(!getCommandExitStatus(command)){
        return "initmain`no";
    }
    else{                                               //if(!getNumberOfSelectedEntries()) -> dadea nul rezultatul mai jos daca faceam asta
        MYSQL_RES *result = mysql_store_result(con);
        int numRows=mysql_num_rows(result);
        if(!numRows)
            return "initmain`firsttime"; 
        else {
            if (result == NULL){
                printError(con);
                mysql_free_result(result);
                //return an error
            }
            else{
            int num_fields = mysql_num_fields(result);
            MYSQL_ROW row;
            string response="initmain`";
            int ind=0;
            while ((row = mysql_fetch_row(result))){
                ind++;
                int index=0;
                for(int i = 0; i < num_fields; i++){
                    index++;
                    //printf("%s ", row[i] ? row[i] : "NULL");
                    response+=row[i];
                    if(index<num_fields || index==1)
                        response+="`"; 
                }
                if(ind<numRows && numRows>1) //ind>1
                    response+="`";
                //printf("\n");
            }
            mysql_free_result(result);
            return response;    
        }      
        }
    } 
}

string CDatabaseManager::getInitMessageWindowResponse(string& userRequesting, string& userRequested){
    string command=stringifyInitMessageWindowSelect(userRequesting, userRequested);
    if(!getCommandExitStatus(command)){
        return "initmess`no";
    }
    else{
        MYSQL_RES *result = mysql_store_result(con);
        int numRows=mysql_num_rows(result);
        if(!numRows)
            return "initmess`firsttime"; 
        else {
            if (result == NULL){
                printError(con);
                mysql_free_result(result);
                //return an error
            }
            else{
            int num_fields = mysql_num_fields(result);
            MYSQL_ROW row;
            string response="initmess`";
            int ind=0;
            while ((row = mysql_fetch_row(result))){
                ind++;
                int index=0;
                for(int i = 0; i < num_fields; i++){
                    index++;
                    //printf("%s ", row[i] ? row[i] : "NULL");
                    response+=row[i];
                    if(index<num_fields || index==1)
                        response+="`";
                }
                if(ind<numRows && numRows>1)
                    response+="`";
                //printf("\n");
            } 
            mysql_free_result(result);
            return response;   
        }        
        }
    } 
}

string CDatabaseManager::getSearchRequestResponse(string& username){
    string command=stringifySearchRequestSelect(username);
    if(!getCommandExitStatus(command)){
        return "search`no";
    }
    else{
        MYSQL_RES *result = mysql_store_result(con);
        int numRows=mysql_num_rows(result);
        if(!numRows)
            return "search`no"; 
        else {
            if (result == NULL){
                printError(con);
                mysql_free_result(result);
                //return an error
            }
            else{
            int num_fields = mysql_num_fields(result);
            MYSQL_ROW row;
            string response="search`";
            while ((row = mysql_fetch_row(result))){
                int index=0;
                for(int i = 0; i < num_fields; i++){
                    index++;
                    //printf("%s ", row[i] ? row[i] : "NULL");
                    response+=row[i];
                    if(index<numRows)
                        response+="`";
                }
                //printf("\n");
            }
            mysql_free_result(result);
            return response;  
        }         
        }
    } 
}

string CDatabaseManager::getSendMessageRequestResponse(string& userRequesting, string& userRequested, string& message, int& ID){
    string command=stringifySendMessageRequestInsert(userRequesting, userRequested, message);
    if(!getCommandExitStatus(command)){
        command=stringifySendMessageRequestSelectLastID(userRequesting, userRequested);
        if(!getCommandExitStatus(command)){
            //send some type of message -> lastID could not be be selected
        }
        else{
            MYSQL_RES *result = mysql_store_result(con);
            MYSQL_ROW row;
            row = mysql_fetch_row(result);
            ID=atoi(row[0]);
        }
        return "message`no";
    }
    else
        return "message`yes";
}

string CDatabaseManager::getReceiveMessageServiceResponse(string& userRequesting, string& userRequested, string& message){
    string response = "receive`";
    response+=userRequesting;
    response+="`";
    response+=message;
    return response;
}

bool CDatabaseManager::setOnlineField(string& username, int value){
    string command=stringifyOnlineFieldUpdate(username, value);
    if(!getCommandExitStatus(command)){
        return false;
    }
    else
        return true;
}

bool CDatabaseManager::getOnlineField(string& username){
    string command=stringifyOnlineFieldSelect(username);
    if(!getCommandExitStatus(command)){
        return false; //nu se mai va trimite mesajul destinatarului la acel moment, desi a fost pus in baza de date
    }
    else{
        MYSQL_RES *result = mysql_store_result(con);
        MYSQL_ROW row;
        row = mysql_fetch_row(result);
        if(row[0])  //daca e online
            return true;
        else
            return false;
    }
}

/*bool CDatabaseManager::updateMessageSeenValue(int ID){
    string command=stringifySeenValueUpdate(ID);
    if(!getCommandExitStatus(command)){
        return false;
    }
    else
        return true;
}

bool CDatabaseManager::getOfflineMessages(string& userRequesting, string& userRequested, vector<string>& receiveMessages){
    string command=stringifyGetOfflineMessagesSelect(userRequesting, userRequested);
    if(!getCommandExitStatus(command)){
        return false;
    }
    else{
        MYSQL_RES *result = mysql_store_result(con);
        int numRows=mysql_num_rows(result);
        if(!numRows){
            return false;
        }
        else {
            if (result == NULL){
                printError(con);
                mysql_free_result(result);
                //return an error
            }
            else{
            int num_fields = mysql_num_fields(result);
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))){
                int index=0;
                string response="receive`";
                for(int i = 0; i < num_fields; i++){
                    index++;
                    printf("%s ", row[i] ? row[i] : "NULL");
                    response+=row[i];
                    if(index<num_fields || index==1)
                        response+="`";
                }
                printf("%s", response);
                receiveMessages.push_back(response);
                printf("\n");
            } 
            }
            mysql_free_result(result);
            return true;           
        }
    } 
}
*/

string CDatabaseManager::getDeleteConversationResponse(string& userRequesting, string& userRequested){
    string command=stringifyDeleteConversation(userRequesting, userRequested);
    if(!getCommandExitStatus(command)){
        return "delete`no";
    }
    else{
        if(mysql_affected_rows(con))
            return "delete`yes";
        else
            return "delete`no";
    }
}