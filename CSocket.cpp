#include "CSocket.hpp"
#include <stdlib.h>             
//pt exit
#include <unistd.h>             
//exit
#include <stdio.h> 
#include <sys/types.h>          
//contine definitiile diferitor tipuri de date folosite in apelurile de sistem
#include <arpa/inet.h>          
//inet_addr, inet_ntoa

CSocket::CSocket(){
}

CSocket::CSocket(string IPaddress, int portNumber){
    initDescriptor();
    initAddress(IPaddress, portNumber);
    bindAddressToDescriptor();
}

void CSocket::initDescriptor(){
    socketDescriptor = socket(AF_INET, SOCK_STREAM, 0); 
    /*socket system call
    AF_INET the address domain of the socket
    SOCK_STREAM charachters are read in a continuous stream, as if from a file
    PROTOCOL - if 0, the OS will choose TCP sockets for stream sockets and UDP for datagram sockets
    */
    if(socketDescriptor<0){
        perror("Socket server nu a putut fi creat");                
        //it displays a message about the error on stderr
        _exit(1);
    }
}

void CSocket::initAddress(string IPaddress, int portNumber){
    socketAddress.sin_family=AF_INET;
    socketAddress.sin_port=htons(portNumber);                       
    //it is necessary to convert this integer from host byte order to network byte order
    socketAddress.sin_addr.s_addr=inet_addr((IPaddress.c_str()));   
    //function from arpa/inet.h - the IP address of the machine on which the server is running, INADDR_ANY - constant that gets this address
}

void CSocket::bindAddressToDescriptor(){
    if(bind(socketDescriptor, (struct sockaddr *)&socketAddress, sizeof(socketAddress))<0){
        perror("Bind could not be realised.");
        _exit(1);
    }
}

int CSocket::getSocketDescriptor(){
    return socketDescriptor;
}

sockaddr_in CSocket::getSocketAddress(){
    return socketAddress;
}

void CSocket::setDescriptor(int descriptor){
    socketDescriptor=descriptor;
}

void CSocket::setAddress(sockaddr_in address){
    socketAddress=address;
}
