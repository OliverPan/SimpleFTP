#include "socket.h"

Socket::Socket(const std::string hostName, const u_short port) {
  WORD sockVersion = MAKEWORD(2, 2);
  //init socket lib
  if (WSAStartup(sockVersion, &wsaData) != 0) {
    std::cout << "Socket::Socket() : socket lib error! \n" << std::endl;
    isConnected = false;
    return;
  }
  //create socket
  socketServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socketServer == INVALID_SOCKET) {
    std::cout << "Socket::Socket() : socket create error!\n" << std::endl;
    WSACleanup();
    isConnected = false;
    return;
  }

  SOCKADDR_IN serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = inet_addr(hostName.c_str());
  serverAddr.sin_port = htons(port);

  int retValue = connect(socketServer, (LPSOCKADDR) &serverAddr, sizeof(serverAddr));
  //check the TCP connection
  if (retValue == SOCKET_ERROR) {
    std::cout << "Socket::Socket() : can't connect to the server!" << std::endl;
    closesocket(socketServer);
    WSACleanup();
    isConnected = false;
    return;
  }
  char controlCodeBuffer[101];
  recv(socketServer, controlCodeBuffer, 100, 0);
  //220 means service ready for new user.
  if (utils::getStateCode(controlCodeBuffer) != 220) {
    std::cout << controlCodeBuffer << std::endl;
    std::cout << "Socket::Socket() : State Code Error!" << std::endl;
    closesocket(socketServer);
    WSACleanup();
    isConnected = false;
    return;
  }
  isConnected = true;
  fileList = std::vector<struct fileAttr>(0);
  path = std::string("/");
}

Socket::~Socket() {
  closesocket(socketServer);
  closesocket(socketData);
  WSACleanup();
}

int Socket::login(const std::string username, const std::string password) {
  char buf[101];
  std::string cmd;
  std::ostringstream oss;
  oss << "USER " << username << "\r\n";
  cmd = oss.str();
  send(socketServer, cmd.c_str(), int(cmd.length()), 0);
  recv(socketServer, buf, 100, 0);
  //331 means password required
  if (utils::getStateCode(buf) != 331) {
    std::cout << buf << std::endl;
    std::cout << "Socket::login() : Username Error!" << std::endl;
    return -1;
  }
  oss.str("");
  oss.clear();
  memset(buf, 0, 100);
  oss << "PASS " << password << "\r\n";
  cmd = oss.str();
  send(socketServer, cmd.c_str(), int(cmd.length()), 0);
  recv(socketServer, buf, 100, 0);
  //230 means User logged in.
  if (utils::getStateCode(buf) != 230) {
    std::cout << buf << std::endl;
    std::cout << "Socket::login() : Password Error!" << std::endl;
    return -1;
  }
  return 0;
}

int Socket::setSocketData() {
  char buf[101];
  std::string cmd;
  std::ostringstream oss;
  oss << "PASV\r\n";
  cmd = oss.str();
  send(socketServer, cmd.c_str(), int(cmd.length()), 0);
  recv(socketServer, buf, 100, 0);
  //227 means Entering Passive Mode
  if (utils::getStateCode(buf) != 227) {
    std::cout << buf << std::endl;
    std::cout << "Socket::setSocketData() : Error Entering Passive Mode!" << std::endl;
    return -1;
  }
  std::string hostName;
  unsigned short port;
  std::tie(hostName, port) = utils::getDataURL(buf);
  socketData = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socketData == INVALID_SOCKET) {
    std::cout << "Socket::setSocketData() : socketData create error!\n" << std::endl;
    return -1;
  }
  SOCKADDR_IN serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = inet_addr(hostName.c_str());
  serverAddr.sin_port = htons(port);
  int retValue = connect(socketData, (LPSOCKADDR) &serverAddr, sizeof(serverAddr));
  if (retValue == SOCKET_ERROR) {
    std::cout << "Socket::setSocketData() : can't connect to the Data Socket!" << std::endl;
    closesocket(socketData);
    return -1;
  }
  return 0;
}

int Socket::list(const std::string dir) {
  if (this->setSocketData()) {
    std::cout << "Error when connecting to the data socket." << std::endl;
    return -1;
  }
  std::ostringstream oss;
  oss << "LIST " << dir << "\r\n";
  std::string ret;
  if (!utils::command(socketServer, oss.str(), ret, 125)) {
    return -1;
  }
  if (!utils::getStringData(socketData, ret)) {
    return -1;
  }
  if (!utils::listParser(ret, fileList)) {
    return -1;
  }
  //226 means Data transfer complete
  if (utils::recvCode(socketServer) != 226) {
    std::cout << "Socket::list() : No 226 Error!" << std::endl;
    return -1;
  }
  closesocket(socketData);
  path = dir;
  return 0;
}

int Socket::getFile(const std::string dir, const std::string localDir) {
  if (setSocketData()) {
    std::cout << "Error when connecting to the data socket." << std::endl;
    return -1;
  }
  std::ostringstream oss;
  std::istringstream iss;
  oss << "SIZE " << dir << "\r\n";
  std::string ret;
  //213 means File status is OK!
  if (!utils::command(socketServer, oss.str(), ret, 213)) {
    return -1;
  }
  iss.str(ret);
  int fileSize;
  iss >> fileSize;
  iss >> fileSize;
  oss.str("");
  oss << "RETR " << dir << "\r\n";
  //125 means File can be downloaded!
  if (!utils::command(socketServer, oss.str(), ret, 125)) {
    return -1;
  }
  if (!utils::downloadFile(socketData, dir, localDir, fileSize)) {
    return -1;
  }
  //226 means Data transfer complete
  if (utils::recvCode(socketServer) != 226) {
    std::cout << "Socket::getFile() : No 226 Error!" << std::endl;
    return -1;
  }
  closesocket(socketData);
  return 0;
}

int Socket::sendFile(std::string dir, std::string localDir) {
  if (setSocketData()) {
    std::cout << "Error when connecting to the data socket." << std::endl;
    return -1;
  }
  std::ostringstream oss;
  std::string ret;
  oss << "STOR " << dir << "\r\n";
  //125 means File can be uploaded!
  if (!utils::command(socketServer, oss.str(), ret, 125)) {
    return -1;
  }
  if (!utils::uploadFile(socketData, dir, localDir)) {
    return -1;
  }
  closesocket(socketData);
  if (utils::recvCode(socketServer) != 226) {
    std::cout << "Socket::getFile() : No 226 Error!" << std::endl;
    return -1;
  }
  return 0;
}

int Socket::mkdir(const std::string dir) {
  std::ostringstream oss;
  std::string ret;
  oss << "MKD " << dir << "\r\n";
  //257 means the dir created!
  if (!utils::command(socketServer, oss.str(), ret, 257)) {
    std::cout << ret;
    return -1;
  }
  return 0;
}
