#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "socket.h"
#include <QString>
#include <QStringList>
#include "logindialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void clearRightLW();
    void login();



private:
    Ui::MainWindow *ui;
    bool LOADING = false;
    void loading();
    QString localDirectory;
    QStringList localfilenames;
    QStringList remotefilenames;
    Socket *FTP;
    int currentRow_left;    //左侧行号
    int currentRow_right;   //右侧行号
    bool flag_left = false;
    bool flag_right = false;
    typedef enum mode {INIT, UPLOAD, DOWNLOAD} MODE;
    MODE mode = INIT;
    LogInDialog *lid;


private slots:
    void chooseMode();
    void start();
    void openFolder();
    void addLocalFile();
    void deleteLocalFile();
    void search();
    void on_lwlocal_clicked();
    void on_lwremote_clicked();
    void on_lwremote_doubleclicked();
};



#endif // MAINWINDOW_H
