#include "sharescanner.h"
#include "ui_sharescanner.h"

ShareScanner::ShareScanner(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ShareScanner)
{
    ui->setupUi(this);
    this->showMaximized();                             // To maximize the window at startup
    statusLabel=new QLabel(ui->statusBar);             // Creating a display label object for status bar
    ui->statusBar->addWidget(statusLabel);             // Adding label object to status bar
    progressBar=new QProgressBar(ui->statusBar);       // Creating a progress bar object for status bar
    progressBar->setMaximumSize(170,19);
    ui->statusBar->addPermanentWidget(progressBar);    // Adding progress bar object to status bar
    scanProcess = NULL;
    oldProcess = NULL;
    scanchildProcess = NULL;
    readSettings();                                    // Load settings
    isScanning=false;
    compIcon= QIcon(":/icons/icons/computer.png");
    dirIcon= QIcon(":/icons/icons/public.png");
    fileIcon= this->style()->standardIcon(QStyle::SP_FileIcon);
    noReadIcon= QIcon(":/icons/icons/public_locked.png");
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    createCustomMenu();
    connect(ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(onItemDoubleClicked(QTreeWidgetItem*)));
    connect(progressBar, SIGNAL(valueChanged(int)),this, SLOT(showProgress(int)));
    connect(ui->treeWidget,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(showContextMenu(QPoint)));
    stop=false;
    count=0;

}

ShareScanner::~ShareScanner()
{
    delete ui;
}


void ShareScanner::generateIpAddresses()
{
    ipList.clear();
    QHostAddress sAddress,eAddress,cAddress;
    sAddress.setAddress(ui->ip_start->text ());
    eAddress.setAddress(ui->ip_end->text ());
    if(eAddress.toIPv4Address()<sAddress.toIPv4Address())
    {
            return;                      // invalid range
    }
    quint32 i=sAddress.toIPv4Address();
    quint32 j=eAddress.toIPv4Address();

    for(;i<=j;i++)
    {
        cAddress.setAddress(i);
        ipList.push_back(cAddress.toString());
    }

}


void ShareScanner::writeSettings()
{
    int index[2];
    QString writeSet[3];
    QSettings settings("ShareScannerv2", "ShareScanner");

    writeSet[0]=ui->ip_start->text ();
    writeSet[1]=ui->ip_end->text ();
    index[0]=ui->chooseFileManager->currentIndex();
    index[1]=ui->chooseSpeed->currentIndex();
    settings.setValue("ipStart", writeSet[0]);
    settings.setValue("ipEnd", writeSet[1]);
    settings.setValue("fileManager", index[0]);
    settings.setValue("speed", index[1]);
}

void ShareScanner::readSettings()
{
    int index[2];
    QString readSet[3];
    QSettings settings("ShareScannerv2", "ShareScanner");

    readSet[0]=settings.value ("ipStart").toString ();
    readSet[1]=settings.value ("ipEnd").toString ();
    index[0]=settings.value ("fileManager").toInt();
    index[1]=settings.value ("speed").toInt();
    ui->ip_start->setText (readSet[0]);
    ui->ip_end->setText (readSet[1]);
    ui->chooseFileManager->setCurrentIndex(index[0]);
    ui->chooseSpeed->setCurrentIndex(index[1]);
}

void ShareScanner::closeEvent(QCloseEvent *event)
{
    writeSettings();
    stop = true;
    event->accept();
     //if scanner is running,
     //then stop the scanner first
     //then exit the application
}

void ShareScanner::updateTree()
{
    QString shares;
    QStringList shareList;
    shares=scanProcess[count].readAll();

    progress=(progress+(100.0/(float)ipList.length()));
    showProgress(progress);

    if(count>=ipList.length()-1)
    {
      ui->startButton->setEnabled(true);
      progressBar->reset();
      statusLabel->setText("Scan Complete !");
      ui->startButton->setEnabled(1);
    }

    if(shares.isEmpty())
    {
        count++;
        return;
    }



    shareList=extractShares(shares);
    if(shareList.at(0).contains("SMBC_readdir_ctx()\n"))
    {
        QString temp;
        temp = shareList[0];
        temp.remove("Invalid dir in SMBC_readdir_ctx()\n");
        shareList[0]=temp;
    }
    int len;
    QString host,name,mac,time;
    QTreeWidgetItem *item0 = new QTreeWidgetItem();
    host=shareList.at(0).section("/",0,0);
    name=shareList.at(1);
    mac=shareList.at(2);
    time=shareList.at(3);
    item0->setText(0,host);
    item0->setIcon(0,compIcon);
    item0->setText(1,name);
    item0->setText(2,mac);
    item0->setText(3,time.left(5)+" ms");
    for(int i=4;i<shareList.length();i++)
    {
        QTreeWidgetItem* child = new QTreeWidgetItem();
        len = shareList.at(i).length();
        name = shareList.at(i);
        name=name.remove(len-3,3);
        child->setText(0,name);
        char dollar='$';
        if(name.at(name.length()-1)!=dollar)
            child->setIcon(0,dirIcon);
        else
            child->setIcon(0,noReadIcon);
        item0->addChild(child);
    }

    ui->treeWidget->addTopLevelItem(item0);
    count++;

}



QStringList ShareScanner::extractShares(QString shares)
{
    QStringList shareList;
    int sep = shares.count("<*>");
    for(int i=0;i<sep;i++)
    {
        shareList << shares.section("<*>",i,i);
    }
    return shareList;
}

void ShareScanner::updateTreeChild()
{
    QString shares;
    QStringList shareList;
    QTreeWidgetItem *item;
    shares=scanchildProcess->readAll();
    shareList=extractShares(shares);
    item = ui->treeWidget->itemAt(currPoint);
    if(!item->parent())
    {
        while(item->childCount())
        {
            item->removeChild(item->child(0));
        }

        for(int i=1;i<shareList.length();i++)
        {
            QTreeWidgetItem *child = new QTreeWidgetItem(item);
            int len = shareList.at(i).length();
            QString name = shareList.at(i);
            name=name.remove(len-3,3);
            child->setText(0,name);
            if(shareList.at(i).right(3)=="SDR")
                child->setIcon(0,dirIcon);
            else
                child->setIcon(0,fileIcon);
            item->addChild(child);
        }
    }
    else if(item->parent())
    {
        while(item->childCount())
        {
            item->removeChild(item->child(0));
        }
        for(int i=3;i<shareList.length();i++)
        {
            QTreeWidgetItem *child = new QTreeWidgetItem(item);
            int len = shareList.at(i).length();
            QString name = shareList.at(i);
            name=name.remove(len-3,3);
            child->setText(0,name);
            if(shareList.at(i).right(3)=="SDR")
                child->setIcon(0,dirIcon);
            else
                child->setIcon(0,fileIcon);
            item->addChild(child);
        }

    }

}


void ShareScanner::scan()
{
    QString path;

    if(QFile::exists("/usr/bin/ip-share-scan"))
        path = "/usr/bin/ip-share-scan";
    else if(QFile::exists("/opt/sharescanner/ip-share-scan"))
        path = "/opt/sharescanner/ip-share-scan";
    else if(QFile::exists("./ip-share-scan"))
        path = "./ip-share-scan";
    else
        path = "ip-share-scan";

    for(int i=0;i<ipList.length();i++)
    {
        if(stop)
            return;
        QStringList ip;
        ip << "smb://"+ipList[i];//+"/";
        scanProcess[i].start (path,ip);
        connect(&scanProcess[i], SIGNAL(finished(int)),this, SLOT(updateTree()));

        if(ui->chooseSpeed->currentIndex()==0)
        {
            if(i%60==0&&i!=0)
            {
                QTest::qWait(2000);
            }
        }
        else if(ui->chooseSpeed->currentIndex()==1)
        {
            if(i%60==0&&i!=0)
            {
                QTest::qWait(2000);
            }
            if(i%200==0&&i!=0)
            {
                QTest::qWait(2000);
            }
            if(i%600==0&&i!=0)
            {
                QTest::qWait(1000);
            }
        }

        else if(ui->chooseSpeed->currentIndex()==2)
        {
            if(i%60==0&&i!=0)
            {
                QTest::qWait(4000);
            }
            if(i%200==0&&i!=0)
            {
                QTest::qWait(4000);
            }
            if(i%600==0&&i!=0)
            {
                QTest::qWait(5000);
            }
        }
    }

}

void ShareScanner::scanChild(QTreeWidgetItem *item)
{
    QString path;

    if(QFile::exists("/usr/bin/ip-share-scan"))
        path = "/usr/bin/ip-share-scan";
    else if(QFile::exists("/opt/sharescanner/ip-share-scan"))
        path = "/opt/sharescanner/ip-share-scan";
    else if(QFile::exists("./ip-share-scan"))
        path = "./ip-share-scan";
    else
        path = "ip-share-scan";
    QString url;
    QStringList ip;
    if(!item->parent())
    {
        url="esmb://"+item->text(0);
    }
    else if(item->parent())
    {
        url=item->text(0);
        while(item->parent())
        {
            item=item->parent();
            url.prepend(item->text(0)+"/");

        }

        url.prepend("esmb://");
    }
    ip << url;
    if(scanchildProcess)
        delete scanchildProcess;
    scanchildProcess = new QProcess();

    scanchildProcess->start (path,ip);
    connect(scanchildProcess, SIGNAL(finished(int)),this, SLOT(updateTreeChild()));
}


void ShareScanner::on_startButton_clicked()
{
    generateIpAddresses();
    ui->treeWidget->clear();
    progressBar->reset();
    progress=0;
    count=0;
    stop=false;
    oldProcess=scanProcess;
    statusLabel->setText("Scanning ...");
    scanProcess = new QProcess[ipList.length()+1];
    if(oldProcess)
        delete[] oldProcess;
    ui->startButton->setEnabled(0);
    scan();
}

void ShareScanner::on_stopbutton_clicked()
{
    stop = true;
    statusLabel->setText("Stopped ...");
    ui->startButton->setEnabled(1);

}

void ShareScanner::onItemDoubleClicked(QTreeWidgetItem *item)
{
    QString browser,url;
    browser=ui->chooseFileManager->currentText();
    browser=browser.toLower();

    if(!item->parent())
    {
        url=browser+" smb://"+item->text(0);
        open.start(url);
    }
    else if(item->parent())
    {
        url=item->text(0)+"\"";
        while(item->parent())
        {
            item=item->parent();
            url.prepend(item->text(0)+"/");

        }

        url.prepend(browser+" smb://\"");
        open.start(url);
    }
}

void ShareScanner::showProgress(int value)
{
    progressBar->setValue(value);
}

void ShareScanner::createCustomMenu()
{
    openFolder = new QAction(tr("Open   "),ui->treeWidget);
    connect(openFolder, SIGNAL(triggered()),this,SLOT(openDirectory()));

    explore = new QAction(tr("Explore  "),ui->treeWidget);
    connect(explore, SIGNAL(triggered()),this,SLOT(exploreDirectory()));

    rescan = new QAction(tr("Rescan "),ui->treeWidget);
    connect(rescan, SIGNAL(triggered()),this,SLOT(exploreDirectory()));

    copyIP = new QAction(tr("Copy IP"),ui->treeWidget);
    connect(copyIP, SIGNAL(triggered()),this,SLOT(copyip()));

    copyDetails = new QAction(tr("Copy Details     "),ui->treeWidget);
    connect(copyDetails, SIGNAL(triggered()),this,SLOT(copydetails()));

    showDetails = new QAction(tr("Show Details        "),ui->treeWidget);
    connect(showDetails, SIGNAL(triggered()),this,SLOT(showdetails()));


    separator1 = new QAction(ui->treeWidget);
    separator2 = new QAction(ui->treeWidget);
    separator3 = new QAction(ui->treeWidget);
    //separator4 = new QAction(ui->tableWidget);

    separator1->setSeparator(true);
    separator2->setSeparator(true);
    separator3->setSeparator(true);
    //separator4->setSeparator(true);

    contextMenu=new QMenu(ui->treeWidget);

    contextMenu->addAction(openFolder);
    contextMenu->addAction(explore);
    contextMenu->addAction(separator1);
    contextMenu->addAction(rescan);
    contextMenu->addAction(separator2);
    contextMenu->addAction(copyIP);
    contextMenu->addAction(copyDetails);
    contextMenu->addAction(separator3);
    contextMenu->addAction(showDetails);


}

void ShareScanner::showContextMenu(QPoint point)
{
    if(!ui->treeWidget->itemAt(point))
        return;
    if(ui->treeWidget->itemAt(point)->parent())
    {
        rescan->setEnabled(0);
        copyDetails->setEnabled(0);
        copyIP->setEnabled(0);
        showDetails->setEnabled(0);
    }
    else
    {
        rescan->setEnabled(1);
        copyDetails->setEnabled(1);
        copyIP->setEnabled(1);
        showDetails->setEnabled(1);
    }
    currPoint=point;
    contextMenu->exec(QCursor::pos());
}

void ShareScanner::openDirectory()
{
    if(!ui->treeWidget->itemAt(currPoint))
        return;

    onItemDoubleClicked(ui->treeWidget->itemAt(currPoint));
}

void ShareScanner::exploreDirectory()
{
    if(!ui->treeWidget->itemAt(currPoint))
        return;

    scanChild(ui->treeWidget->itemAt(currPoint));
}

void ShareScanner::copyip()
{
    if(!ui->treeWidget->itemAt(currPoint))
        return;
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui->treeWidget->itemAt(currPoint)->text(0));
}

void ShareScanner::copydetails()
{
    if(!ui->treeWidget->itemAt(currPoint))
        return;

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui->treeWidget->itemAt(currPoint)->text(0)+" "+ui->treeWidget->itemAt(currPoint)->text(1)+" "
                       +ui->treeWidget->itemAt(currPoint)->text(2)+" "+ui->treeWidget->itemAt(currPoint)->text(3));
}

void ShareScanner::showdetails()
{
    if(!ui->treeWidget->itemAt(currPoint))
        return;
    QMessageBox::information(this,"Details",ui->treeWidget->itemAt(currPoint)->text(0)+"\n"+ui->treeWidget->itemAt(currPoint)->text(1)+"\n"
                             +ui->treeWidget->itemAt(currPoint)->text(2)+"\n"+ui->treeWidget->itemAt(currPoint)->text(3));
}

void ShareScanner::on_actionClear_triggered()
{
    generateIpAddresses();
    ui->treeWidget->clear();
    progressBar->reset();
    progress=0;
}

void ShareScanner::on_actionExit_triggered()
{
    stop = true;
    writeSettings();
    qApp->quit();
}

void ShareScanner::on_actionExpandAll_triggered()
{
    ui->treeWidget->expandAll();
}

void ShareScanner::on_actionCollapseAll_triggered()
{
    ui->treeWidget->collapseAll();
}

void ShareScanner::on_actionAbout_triggered()
{
    aboutDialog.show();
}

void ShareScanner::on_actionCheckUpdate_triggered()
{
    QDesktopServices::openUrl(QUrl("http://sourceforge.net/projects/sharescanner"));
}

void ShareScanner::on_actionLicense_triggered()
{
    QDesktopServices::openUrl(QUrl("http://gnu.org/licenses/gpl.html"));

}
