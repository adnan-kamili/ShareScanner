#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include <QDesktopWidget>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());
    this->setWindowFlags( Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint );
    const QRect screen = QApplication::desktop()->screenGeometry();
    this->move( screen.center() - this->rect().center() );
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::on_close_button_clicked()
{
    this->close();
}

void AboutDialog::on_credits_button_clicked()
{
    if(ui->stackedWidget->currentIndex()!=1)
        ui->stackedWidget->setCurrentIndex(1);
    else
        ui->stackedWidget->setCurrentIndex(0);
}

void AboutDialog::on_License_button_clicked()
{
    if(ui->stackedWidget->currentIndex()!=2)
        ui->stackedWidget->setCurrentIndex(2);
    else
        ui->stackedWidget->setCurrentIndex(0);
}
