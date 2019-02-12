#include "utils.h"

int utils::getStateCode(const char *buf) {
  std::string bufString = std::string(buf);
  std::istringstream ism(bufString);
  int code;
  if (ism >> code) {
    return code;
  } else {
    return -1;
  }
}

std::tuple<std::string, unsigned short> utils::getDataURL(const char *buf) {
  std::string bufString = std::string(buf);
  std::regex reg("\\(([0-9]+),([0-9]+),([0-9]+),([0-9]+),([0-9]+),([0-9]+)\\)");
  std::string hostName;
  for (int i = 1; i != 5; i++) {
    std::sregex_token_iterator it(bufString.begin(), bufString.end(), reg, i);
    hostName += it->str();
    if (i != 4) {
      hostName += ".";
    } else {
      hostName += "\r\n";
    }
  }
  std::sregex_token_iterator it5(bufString.begin(), bufString.end(), reg, 5);
  int p1 = std::stoi(it5->str());
  std::sregex_token_iterator it6(bufString.begin(), bufString.end(), reg, 6);
  int p2 = std::stoi(it6->str());
  return {hostName, p1 * 256 + p2};
}

bool utils::command(SOCKET Socket, std::string cmd, std::string &ret, int stateCode) {
  const int len = 100;
  send(Socket, cmd.c_str(), int(cmd.length()), 0);
  char buf[len + 1];
  int bufLen = recv(Socket, buf, len, 0);
  buf[bufLen] = '\0';
  ret = std::string(buf);
  if (getStateCode(buf) == stateCode) {
    return true;
  } else {
    std::cout << "utils::command() : The StateCode " << getStateCode(buf) << " is Error!" << std::endl;
    return false;
  }
}

int utils::recvCode(SOCKET Socket) {
  const int len = 100;
  char buf[len + 1];
  int bufLen = recv(Socket, buf, len, 0);
  buf[bufLen] = '\0';
  return getStateCode(buf);
}

bool utils::getStringData(SOCKET Socket, std::string &ret) {
  const int len = 256;
  char buf[len + 1];
  int recvBytes;
  ret = std::string("");
  while ((recvBytes = recv(Socket, buf, len, 0)) > 0) {
    buf[recvBytes] = '\0';
    ret += buf;
  }
  return true;
}

bool utils::listParser(const std::string &listString, std::vector<struct fileAttr> &fileList) {
  std::istringstream iss(listString);
  std::string x;
  fileList = std::vector<struct fileAttr>(0);
  while (iss >> x && std::regex_match(x, std::regex(std::string("\\d\\d-\\d\\d-\\d\\d")))) {
    if (x.length() != 8) {
      break;
    }
    struct fileAttr tmp;
    strcpy(tmp.date, x.c_str());
    iss >> x;
    strcpy(tmp.time, x.c_str());
    iss >> x;
    if (x == "<DIR>") {
      tmp.isDIR = true;
      tmp.size = 0;
    } else {
      tmp.isDIR = false;
      tmp.size = std::stoi(x);
    }
    iss >> x;
    tmp.name = x;
    fileList.push_back(tmp);
  }
  return true;
}

bool utils::downloadFile(SOCKET Socket, const std::string dir, const std::string localDir, int fileSize) {
  std::ofstream ofs(localDir, std::ios::out | std::ios::binary);
  if (!ofs.is_open()) {
    std::cout << "utils::downloadFile() : File can't be open! FileDir : " << localDir << std::endl;
    return false;
  }
  int sizeNow = 0;
  const int len = 2048;
  char buf[len + 1];
  int recvBytes;
  while ((recvBytes = recv(Socket, buf, len, 0)) > 0) {
    buf[recvBytes] = '\0';
    sizeNow += recvBytes;
    ofs.write(buf, recvBytes);
  }
  if (sizeNow != fileSize) {
    std::cout << "utils::downloadFile() : File is incomplete!" << std::endl;
    return false;
  }
  ofs.close();
  return true;
}

bool utils::uploadFile(SOCKET Socket, std::string dir, std::string localDir) {
  std::ifstream ifs(localDir, std::ios::in | std::ios::binary);
  if (!ifs.is_open()) {
    std::cout << "utils::uploadFile() : File can't be open! FileDir : " << localDir << std::endl;
    return false;
  }
  const int len = 2048;
  char buf[len + 1];
  ifs.seekg(0, ifs.end);
  long long int length = ifs.tellg();
  ifs.seekg(0, ifs.beg);
  while (!ifs.eof()) {
    ifs.read(buf, len);
    length -= len;
    if (length >= 0) {
      send(Socket, buf, len, 0);
    } else {
      send(Socket, buf, len + length, 0);
      break;
    }
  }
  ifs.close();
  return true;
}
