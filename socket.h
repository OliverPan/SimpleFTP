#ifndef SIMPLEFTP_SOCKET_H
#define SIMPLEFTP_SOCKET_H

#include "utils.h"
#include <winsock2.h>
#include <iostream>
#include <string>
#include <tuple>

class Socket {
 public:
  WSADATA wsaData;
  SOCKET socketServer, socketData;
  bool isConnected;
  std::vector<struct fileAttr> fileList;
  std::string path;
  explicit Socket(std::string hostName,u_short port);
  ~Socket();
  int login(std::string username, std::string password);
  int setSocketData();
  int list(std::string dir);
  int getFile(std::string dir,std::string localDir);
  int sendFile(std::string dir,std::string localDir);
  int mkdir(std::string dir);
};
#endif //SIMPLEFTP_SOCKET_H

