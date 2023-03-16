#ifndef CANSSEND_H
#define CANSSEND_H

#include <map>
#include <vector>
#include <string>

class CanSend {

  private :
   // CanSend(){};

    std::map<std::string, int> sockmap;  //< device-socket

    int socketopen(char* device );

  public:
    
    void socketclose();
    int send(std::vector<unsigned char> body, unsigned int msgid, char* device ); //< can network 연동, cansend.c 참조
    int socketopen(std::vector<std::string> device);
    bool isConnected(char* device);
    bool isConnect();
};

#endif