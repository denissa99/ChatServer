#include "CParser.hpp"
#include <string.h>
#include <stdio.h>

CParser::CParser(){

}
CParser::~CParser(){

}

void CParser::getOperationType(char*& receivedMessage, string& operationType){
    char* p=NULL;
    p = strtok (receivedMessage,"`");
    operationType=p;
    //printf ("%s\n",operationType.c_str());
}

void CParser::getToken(string& token){
    p = strtok (NULL, "`");
    token=p;
    //printf ("%s\n", token.c_str());
}

void CParser::splitMessage(string& firstContainer){
    getToken(firstContainer);
}

void CParser::splitMessage(string& firstContainer, string& secondContainer){
    getToken(firstContainer);
    getToken(secondContainer);
}

void CParser::splitMessage(string& firstContainer, string& secondContainer, string& thirdContainer){
    getToken(firstContainer);
    getToken(secondContainer);
    getToken(thirdContainer);
}