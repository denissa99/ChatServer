#include <string>
using namespace std;

class CParser{
public:
    CParser();
    ~CParser();
    void getOperationType(char*& receivedMessage, string& operationType);
    void splitMessage(string& firstContainer);
    void splitMessage(string& firstContainer, string& secondContainer);
    void splitMessage(string& firstContainer, string& secondContainer, string& thirdContainer);
private:
    char* p;
    void getToken(string& token);
};