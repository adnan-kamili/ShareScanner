#ifndef SHARESCANNER_H
#define SHARESCANNER_H

#include <QMainWindow>
#include <QLabel>
#include <QProgressBar>
#include <QSettings>
#include <QMessageBox>
#include <QObject>
#include <QtNetwork/QHostAddress>
#include <QStringList>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QClipboard>
#include <Qt/qtest.h>
#include <QTreeWidgetItem>
#include <aboutdialog.h>

namespace Ui {
class ShareScanner;
}

class ShareScanner : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit ShareScanner(QWidget *parent = 0);
    ~ShareScanner();    

    
private slots:

    void on_startButton_clicked();
    void updateTree();
    void on_stopbutton_clicked();
    void onItemDoubleClicked(QTreeWidgetItem*);
    void showProgress(int);
    void showContextMenu(QPoint);
    void openDirectory();
    void exploreDirectory();
    void updateTreeChild();
    void copyip();
    void copydetails();
    void showdetails();


    void on_actionClear_triggered();

    void on_actionExit_triggered();

    void on_actionExpandAll_triggered();

    void on_actionCollapseAll_triggered();

    void on_actionAbout_triggered();

    void on_actionCheckUpdate_triggered();

    void on_actionLicense_triggered();

signals:
    void addEntry(int,int);

private:
    Ui::ShareScanner *ui;
    QLabel *statusLabel;
    QProgressBar *progressBar;
    QStringList ipList;
    bool isScanning;
    QIcon dirIcon,noReadIcon,compIcon,fileIcon;
    QProcess open;
    QPoint currPoint;
    QProcess *scanProcess,*oldProcess,*scanchildProcess;//[6000];
    int count;
    float progress;
    bool stop;
    AboutDialog aboutDialog;

    /**************** context menu *************************/
    QMenu *contextMenu;
    QAction *separator1;
    QAction *separator2;
    QAction *separator3;
    QAction *separator4;
    QAction *openFolder;
    QAction *rescan;
    QAction *explore;
    QAction *copyIP;
    QAction *copyDetails;
    QAction *showDetails;


private:
    void generateIpAddresses();
    void readSettings();
    void writeSettings();
    void closeEvent(QCloseEvent *event);
    void scanChild(QTreeWidgetItem*);
    void scan();
    void createCustomMenu();
    QStringList extractShares(QString);


};



#endif // SHARESCANNER_H

