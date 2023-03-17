#ifndef CAN_SEND_H
#define CAN_SEND_H

#include <map>
#include <vector>
#include <string>

class CanSend {

  private :
    std::map<std::string, int> sockmap;  //< device-socket

    int SocketOpen(char* device );

  public:

    void SocketClose();
    int Send(std::vector<unsigned char> body, unsigned int msgid, char* device ); //< can network 연동, cansend.c 참조
    int SocketOpen(std::vector<std::string> device);
    
    bool IsConnected(char* device);
    bool IsConnect();    

};

#endif
