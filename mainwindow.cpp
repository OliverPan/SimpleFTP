#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QApplication>
#include <iostream>
#include <socket.h>
#include <QString>
#include <QStringList>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QListView>
#include <QDialog>
#include "logindialog.h"
//#include "socket.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setFixedSize(751,625);
    connect(ui->cbChoose, SIGNAL(currentIndexChanged(int)), this, SLOT(chooseMode()));
    connect(ui->btnStart, SIGNAL(clicked(bool)), this, SLOT(start()));
    connect(ui->btnBrowse, SIGNAL(clicked(bool)), this, SLOT(openFolder()));
    connect(ui->btnAddFile, SIGNAL(clicked(bool)), this, SLOT(addLocalFile()));
    connect(ui->btnClearFile, SIGNAL(clicked(bool)), this, SLOT(deleteLocalFile()));
    connect(ui->lwLocal, SIGNAL(clicked(QModelIndex)), this, SLOT(on_lwlocal_clicked()));
    connect(ui->btnSearch, SIGNAL(clicked(bool)), this, SLOT(search()));
    connect(ui->lwRemote, SIGNAL(clicked(QModelIndex)), this, SLOT(on_lwremote_clicked()));
    connect(ui->lwRemote, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(on_lwremote_doubleclicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::chooseMode() {
    mode = MODE(ui->cbChoose->currentIndex());
    qDebug() << int(mode);
}

void MainWindow::loading() {
    if (LOADING == true) {
        ui->progressBar->setMinimum(0);
        ui->progressBar->setMaximum(0);
    }
    else {
        ui->progressBar->setMinimum(100);
        ui->progressBar->setMaximum(100);
    }
}

void MainWindow::start() /* press start button */
{
    LOADING = true;
    loading();
    if (mode == UPLOAD) {
        int item = localfilenames.length();
        QString tmp = localDirectory;
        tmp += "\\";
        for (int i = 0; i < item; i ++) {
            QString tmp_name = localfilenames[i];
            QString remote_name = tmp_name.split("/").last();
            qDebug() << tmp_name << ", " << remote_name;
            FTP->sendFile(remote_name.toStdString(), tmp_name.toStdString());
        }
    }
    else if (mode == DOWNLOAD) {
        if (localDirectory.isEmpty()) {
            QMessageBox::warning(NULL, "Warning", "No local directory.", QMessageBox::Yes, QMessageBox::Yes);
            return;
        }
        int num = remotefilenames.length();
        if (!(ui->lwRemote->currentRow()<num && ui->lwRemote->currentRow() >= 0)){
            QMessageBox::warning(NULL, "Warning", "Please choose a file!", QMessageBox::Yes, QMessageBox::Yes);
            return;
        }

        QString filename = remotefilenames[num - 1 - ui->lwRemote->currentRow()];
        qDebug() << filename;
        QString tmp = localDirectory;
        tmp += "\\";
        tmp += filename;
        qDebug() << tmp;
        FTP->getFile(filename.toStdString(), tmp.toStdString());
    }
    else {
        QMessageBox::warning(NULL, "Warning", "Please choose the mode!", QMessageBox::Yes, QMessageBox::Yes);
        LOADING = false;
        loading();
        return;
    }
    LOADING = false;
    loading();
    delete FTP;
}

void MainWindow::openFolder() {
    QString directory = QDir::toNativeSeparators(
                QFileDialog::getExistingDirectory(this, tr("Save Path"), QDir::currentPath()));
    if (!directory.isEmpty()) {
        ui->leLocal->setText(directory);
        //可以添加一个创建目录
    }
    localDirectory = directory;
}

void MainWindow::addLocalFile() {
    flag_left = true;
    flag_right = false;
    QStringList newFileNames = QFileDialog::getOpenFileNames(this,tr("Open File"), "/",tr("All Files (*.*)"));
    //qDebug()<<"filenames:"<<filenames;
    localfilenames.append(newFileNames);
    for(int i = 0; i < newFileNames.length(); i ++) {
        QString tmp = newFileNames[i];
        ui->lwLocal->insertItem(0, tmp);
    }
    //ui->teLocalFiles->setText(filenames.join("\n"));
}

void MainWindow::deleteLocalFile() {
    int index = localfilenames.length() - 1 - currentRow_left;
    localfilenames.removeAt(index);
    ui->lwLocal->takeItem(currentRow_left);
    //ui->teLocalFiles->setText("");
}


void MainWindow::search() {
    clearRightLW();
    if (ui->leRemote->text().isEmpty())
        return;
    FTP = new Socket(ui->leRemote->text().toStdString(), 21);
    qDebug() << ui->leRemote->text();
    if (FTP->isConnected) {
        qDebug() << "success connection";
    }else {
        QMessageBox::warning(NULL, "Warning", "Error when connecting to the server.", QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    //FTP->login("anonymous", "123@qq.com");
    login();
    FTP->list("/");
    ui->leCurrentPath->setText("/");
    int length = FTP->fileList.size();
    QStringList remoteFileNames;
    for (int i = 0; i < length; i ++) {
        remoteFileNames << QString::fromStdString(FTP->fileList[i].name);
    }
    qDebug() << remoteFileNames;
    remotefilenames = remoteFileNames;
    for(int i = 0; i < remotefilenames.length(); i ++) {
        QString tmp = remotefilenames[i];
        ui->lwRemote->insertItem(0, tmp);

    flag_right = true;
    flag_left = false;
    //返回值转换为一个QStringlist，然后ui->teRemoteFiles->setText();
    }
}

void MainWindow::on_lwlocal_clicked() {
    if (flag_left) {
        currentRow_left = ui->lwLocal->currentRow();
        //qDebug() << currentRow_left;
    }
    return;
}

void MainWindow::on_lwremote_clicked() {
    if (flag_right) {
        currentRow_right = ui->lwRemote->currentRow();
    }
}

void MainWindow::clearRightLW() {
    while (ui->lwRemote->item(0)) {
        ui->lwRemote->takeItem(0);
    }
}

void MainWindow::on_lwremote_doubleclicked() {
    if (currentRow_right > remotefilenames.length() - 1) {
        if (currentRow_right == remotefilenames.length()) {
            return;
        }
        else if(currentRow_right == remotefilenames.length() + 1) {
            clearRightLW();
            QString current = ui->leCurrentPath->text();
            QStringList tmp_list = current.split("/");
            tmp_list.removeLast();
            tmp_list.removeLast();
            QString target = tmp_list.join("/") + "/";
            FTP->list(target.toStdString());
            ui->leCurrentPath->setText(target);
            int length = FTP->fileList.size();
            QStringList remoteFileNames;
            for (int i = 0; i < length; i ++) {
                remoteFileNames << QString::fromStdString(FTP->fileList[i].name);
            }
            //qDebug() << remoteFileNames;
            remotefilenames = remoteFileNames;
            for(int i = 0; i < remotefilenames.length(); i ++) {
                QString tmp = remotefilenames[i];
                ui->lwRemote->insertItem(0, tmp);
            }
            if (!(target.toStdString() == "/")) {
                ui->lwRemote->insertItem(remotefilenames.length(), "../");
                ui->lwRemote->insertItem(remotefilenames.length(), "./");
            }
            return;
        }
    }
    int index = remotefilenames.length() - 1 - currentRow_right;
    if(!FTP->fileList[index].isDIR)
        return;
    QString currentDir = remotefilenames[index];
    clearRightLW();
    QString current = ui->leCurrentPath->text();
    FTP->list(current.toStdString()+ currentDir.toStdString() + "/");
    ui->leCurrentPath->setText(current + currentDir + "/");
    int length = FTP->fileList.size();
    QStringList remoteFileNames;
    for (int i = 0; i < length; i ++) {
        remoteFileNames << QString::fromStdString(FTP->fileList[i].name);
    }
    //qDebug() << remoteFileNames;
    remotefilenames = remoteFileNames;
    for(int i = 0; i < remotefilenames.length(); i ++) {
        QString tmp = remotefilenames[i];
        ui->lwRemote->insertItem(0, tmp);
    }
    ui->lwRemote->insertItem(remotefilenames.length(), "../");
    ui->lwRemote->insertItem(remotefilenames.length(), "./");
}


void MainWindow::login() {
    lid = new LogInDialog(this);
    if (lid->exec() == QDialog::Accepted) {
        lid->init();
    }
    FTP->login(lid->account.toStdString(), lid->password.toStdString());
}



