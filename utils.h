#ifndef SIMPLEFTP_UTILS_H
#define SIMPLEFTP_UTILS_H

#endif //SIMPLEFTP_UTILS_H

#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <tuple>
#include <regex>
#include <vector>
#include <winsock2.h>

struct fileAttr {
  char date[9];
  char time[8];
  bool isDIR;
  int size;
  std::string name;
};
class utils {
 public:
  static int getStateCode(const char *buf);
  static std::tuple<std::string, unsigned short> getDataURL(const char *buf);
  static bool command(SOCKET Socket, std::string cmd, std::string &ret, int stateCode);
  static int recvCode(SOCKET Socket);
  static bool getStringData(SOCKET Socket, std::string &ret);
  static bool listParser(const std::string &listString, std::vector<struct fileAttr> &fileList);
  static bool downloadFile(SOCKET Socket, std::string dir, std::string localDir, int fileSize);
  static bool uploadFile(SOCKET Socket, std::string dir, std::string localDir);
};
