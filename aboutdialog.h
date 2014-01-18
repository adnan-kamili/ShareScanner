#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AboutDialog(QWidget *parent = 0);
    ~AboutDialog();
    
private slots:
    void on_close_button_clicked();

    void on_credits_button_clicked();

    void on_License_button_clicked();

private:
    Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H
