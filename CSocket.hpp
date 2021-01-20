#include <string>
#include <sys/socket.h>             
//contine definitiile structurilor necesare pentru socket-uri
#include <netinet/in.h>             
//contine constante si structuri necesare pentru adrese internet
#include <sys/poll.h>               
//contine structura pollfd care e necesare pentru a intreba in sistem ce fds e gata de citit

using namespace std;

class CSocket{
public:
    CSocket();
    CSocket(string IPaddress, int portNumber);
    int getSocketDescriptor();
    sockaddr_in getSocketAddress();
    void setDescriptor(int descriptor);
    void setAddress(sockaddr_in address);
private:
    int socketDescriptor;
    struct sockaddr_in socketAddress; 
    //o structura ce contine adrese internet din netinet/in.h
    void initDescriptor();
    void initAddress(string IPaddress, int portNumber);
    void bindAddressToDescriptor();
};