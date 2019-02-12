#include "mainwindow.h"
#include <QApplication>
#include <QPushButton>
#include "socket.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

/*
int main() {
  Socket ftp = Socket(std::string("127.0.0.1"), 21);
  if (ftp.isConnected) {
    std::cout << "Successful connected to the server." << std::endl;
  } else {
    std::cout << "Error when connecting to the server." << std::endl;
    return 1;
  }
  ftp.login("anonymous", "123@qq.com");
  ftp.list("/");
  //(server, local);
  ftp.getFile("/readme.pdf","readme.pdf");
  ftp.getFile("/dir1/readme.pdf","readme1.pdf");
  ftp.sendFile("r1.pdf","readme1.pdf");
  ftp.sendFile("/dir1/r2.pdf","readme.pdf");
  ftp.mkdir("/dd");
  ftp.~Socket();
  return 0;
}
*/
