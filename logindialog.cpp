#include "logindialog.h"
#include "ui_logindialog.h"

LogInDialog::LogInDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LogInDialog)
{
    ui->setupUi(this);
    setFixedSize(501, 259);
}

LogInDialog::~LogInDialog()
{
    delete ui;
}

void LogInDialog::init() {
    account = ui->leAccount->text();
    password = ui->lePassword->text();
}
