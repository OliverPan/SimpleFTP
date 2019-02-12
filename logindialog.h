#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QString>
#include <QWidget>

namespace Ui {
class LogInDialog;
}

class LogInDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LogInDialog(QWidget *parent = 0);
    ~LogInDialog();
    QString account;
    QString password;
    //bool runDialog();
    void init();

private:
    Ui::LogInDialog *ui;

};

#endif // LOGINDIALOG_H
