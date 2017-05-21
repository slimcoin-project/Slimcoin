#include "vanitygenpage.h"
#include "ui_vanitygenpage.h"
#include "vanitygenwork.h"

#include "bitcoingui.h"
#include "util.h"

#include "walletmodel.h"
#include "askpassphrasedialog.h"

#include <boost/thread.hpp>

#include <QClipboard>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QTimer>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>

#include <QDebug>

VanityGenPage::VanityGenPage(QWidget *parent):
    QMainWindow(parent),
    walletModel(0),
    ui(new Ui::VanityGenPage)
{
    ui->setupUi(this);

    model = new QStandardItemModel(0,3,this);

    QStringList headerLabels;
    headerLabels << "Pattern" << "Privkey" << "Chance";
    model->setHorizontalHeaderLabels(headerLabels);

    ui->tableView->setModel(model);

    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->resizeSection(0,250);
    ui->tableView->horizontalHeader()->resizeSection(2,150);

    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);//MultiSelection);

    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(tableViewClicked(QItemSelection,QItemSelection)));
    connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customMenuRequested(QPoint)));

    ui->tableView->setFocusPolicy(Qt::StrongFocus);
    ui->tableView->installEventFilter(this);

    VanityGenKeysChecked = 0;
    VanityGenHashrate = 0;//"0.0";
    VanityGenNThreads = 0;
    VanityGenMatchCase = 0;

    //Input field:

    ui->lineEdit->setValidator(new QRegExpValidator(QRegExp("[S]{1,1}[MNP-Za-k]{1,1}[1-9A-HJ-NP-Za-km-z]{10,10}"), NULL));
    ui->lineEdit->setMaxLength(16);

    connect(ui->lineEdit, SIGNAL(textChanged(QString)), this, SLOT(changeAllowedText()));
    connect(ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(addPatternClicked()));

    checkAllowedText(0);


    //"Add Pattern" - Buttton:

    connect(ui->buttonPattern, SIGNAL(clicked()), this, SLOT(addPatternClicked()));


    int nThreads = boost::thread::hardware_concurrency();
    int nUseThreads = GetArg("-genproclimit", -1);
    if (nUseThreads < 0)
        nUseThreads = nThreads;
    ui->horizontalSlider->setMaximum(nUseThreads);

    ui->checkBoxAutoImport->setEnabled(false);
    ui->buttonImport->setEnabled(false);
    ui->buttonDelete->setEnabled(false);

    connect(ui->checkBoxMatchCase, SIGNAL(clicked(bool)), this, SLOT(changeMatchCase(bool)));

    connect(ui->buttonDelete, SIGNAL(clicked(bool)),this, SLOT(deleteRows()));

    connect(ui->buttonImport, SIGNAL(clicked(bool)), this, SLOT(importIntoWallet()));

    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(updateLabelNrThreads(int)));
    connect(ui->horizontalSlider, SIGNAL(sliderReleased()), this, SLOT(saveFile()));

    connect(ui->checkBoxAutoImport, SIGNAL(released()), this, SLOT(saveFile()));
    connect(ui->checkBoxShowPrivKeys, SIGNAL(released()), this, SLOT(saveFile()));


    connect(ui->buttonStart,SIGNAL(clicked()), this, SLOT(startThread()));
    connect(ui->buttonUnlock,SIGNAL(clicked()), this, SLOT(unlockWallet()));


    copyAddressAction = new QAction("Copy Address", this);
    copyPrivateKeyAction = new QAction("Copy PrivateKey", this);
    importIntoWalletAction = new QAction("Import into Wallet", this);
    deleteAction = new QAction("Delete", this);

    contextMenu = new QMenu();
    contextMenu->addAction(importIntoWalletAction);
    contextMenu->addSeparator();
    contextMenu->addAction(copyAddressAction);
    contextMenu->addAction(copyPrivateKeyAction);
    contextMenu->addSeparator();
    contextMenu->addAction(deleteAction);

    connect(copyAddressAction, SIGNAL(triggered()), this, SLOT(copyAddress()));
    connect(copyPrivateKeyAction, SIGNAL(triggered()), this, SLOT(copyPrivateKey()));
    connect(importIntoWalletAction, SIGNAL(triggered()), this, SLOT(importIntoWallet()));
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteEntry()));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateVanityGenUI()));
    timer->start(250);

    updateUi();

    loadFile();

}

void VanityGenPage::loadFile(){

    QString settings;
    QFile file;
    file.setFileName(GetDataDir().string().c_str() + QString("/vanitygen.json"));

    if(!file.exists()){
        saveFile();
        return;
    }

    file.open(QFile::ReadOnly | QFile::Text);

    settings = file.readAll();
    file.close();

    QJsonDocument jsonResponse = QJsonDocument::fromJson(settings.toUtf8());
    QJsonObject jsonObj = jsonResponse.object();

    VanityGenNThreads = jsonObj["state"].toObject()["threads"].toString().toInt();
    ui->horizontalSlider->setSliderPosition(VanityGenNThreads);
    ui->horizontalSlider->setValue(VanityGenNThreads);

    ui->checkBoxMatchCase->setChecked((jsonObj["state"].toObject()["matchCase"].toString() == "true") ? true : false);
    VanityGenMatchCase = (ui->checkBoxMatchCase->checkState() == 2) ? 1 : 0;

    ui->checkBoxShowPrivKeys->setChecked((jsonObj["state"].toObject()["showPrivKeys"].toString() == "true") ? true : false);

    ui->checkBoxAutoImport->setChecked((jsonObj["state"].toObject()["autoImport"].toString() == "true") ? true : false);

    QJsonArray workList = jsonObj["workList"].toArray();

    for(int i = workList.count()-1; i>=0; i--){
        VanityGenWorkList.prepend(VanGenStruct());
        VanityGenWorkList[0].pattern = workList[i].toObject()["pattern"].toString();
        VanityGenWorkList[0].privkey = workList[i].toObject()["privkey"].toString();
        VanityGenWorkList[0].pubkey = workList[i].toObject()["pubkey"].toString();
        VanityGenWorkList[0].difficulty = "";
        VanityGenWorkList[0].pubkey != "" ? VanityGenWorkList[0].state = 2 : VanityGenWorkList[0].state = 0;
        VanityGenWorkList[0].notification = 0;
    }

    rebuildTableView();

    if(jsonObj["state"].toObject()["running"].toString() == "true" && VanityGenNThreads > 0){
        startThread();
    }

}

void VanityGenPage::saveFile(){

    file.setFileName(GetDataDir().string().c_str() + QString("/vanitygen.json"));

    file.open(QFile::ReadWrite | QFile::Text | QFile::Truncate);

    QString json;

    json.append("{\n");
    json.append("\t\"state\": {\n");
    json.append("\t\t\"running\": \""+ QString(VanityGenRunning ? "true" : "false")+"\",\n");
    json.append("\t\t\"threads\": \""+ QString::number(VanityGenNThreads)+"\",\n");
    json.append("\t\t\"matchCase\": \""+QString(ui->checkBoxMatchCase->checkState() == 2 ? "true" : "false")+"\",\n");
    json.append("\t\t\"showPrivKeys\": \""+QString(ui->checkBoxShowPrivKeys->checkState() == 2 ? "true" : "false")+"\",\n");
    json.append("\t\t\"autoImport\": \""+QString(ui->checkBoxAutoImport->checkState() == 2 ? "true" : "false")+"\"\n");
    json.append("\t},\n");
    json.append("\t\"workList\": [\n");
    for(int i = 0;i<VanityGenWorkList.length();i++){
        json.append("\t\t{\"pattern\": \""+QString(VanityGenWorkList[i].pattern)+
                    "\", \"pubkey\": \""+QString(VanityGenWorkList[i].pubkey)+
                    "\", \"privkey\": \""+QString(VanityGenWorkList[i].privkey)+
                    "\"}"+((i<VanityGenWorkList.length()-1) ? ",": "") +"\n");
    }
    json.append("\t]\n");
    json.append("}\n");

    file.write(json.toUtf8());//.toJson());
    file.close();
}

void VanityGenPage::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
    (this->walletModel->getEncryptionStatus() == 1) ? buttonUnlockState = true : buttonUnlockState = false;
    updateUi();
}

void VanityGenPage::unlockWallet(){

    if(buttonUnlockState){

        WalletModel::UnlockContext ctx(this->walletModel->requestUnlock());

        if (ctx.isValid()){
            gui->externCommand((const QString) QString("walletpassphrase "+VanityGenPassphrase+" 1000000000"));

            ui->checkBoxAutoImport->setChecked(true);
            buttonUnlockState = !buttonUnlockState;
        }
    } else{
        lockWallet();
    }

    updateUi();
}

void VanityGenPage::lockWallet(){

    if(this->walletModel->getEncryptionStatus() == 2){
        gui->externCommand((const QString) QString("walletlock"));
        buttonUnlockState = !buttonUnlockState;
        ui->checkBoxAutoImport->setChecked(false);
        updateUi();
    } else{
        AskPassphraseDialog dlg(AskPassphraseDialog::Encrypt, this);
        dlg.setModel(walletModel);
        dlg.exec();

        gui->setEncryptionStatus(walletModel->getEncryptionStatus());
    }
}

void VanityGenPage::keyPressEvent(QKeyEvent *event)
{
     if(event->key() == Qt::Key_Return && ui->lineEdit->hasFocus()){
         addPatternClicked();
     }
  if(event->key() == Qt::Key_Delete && !VanityGenRunning)
      deleteRows();
}

void VanityGenPage::customMenuRequested(QPoint pos)
{
    QModelIndex index = ui->tableView->indexAt(pos);

    tableIndexClicked = index.row();

    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();

    if(index.isValid())
    {
        importIntoWalletAction->setText("Import into Wallet");
        deleteAction->setText("Delete");

        importIntoWalletAction->setEnabled(false);
        copyPrivateKeyAction->setEnabled(false);
        copyAddressAction->setEnabled(false);
        deleteAction->setEnabled(false);

        if(VanityGenWorkList[tableIndexClicked].privkey != ""){
            if(this->walletModel->getEncryptionStatus() != 1){
                int atLeastOneImportable = 0;
                for(int i=0; i< selection.count(); i++)
                {
                    if(VanityGenWorkList[selection.at(i).row()].privkey != ""){
                        atLeastOneImportable++;
                    }
                }
                importIntoWalletAction->setText("Import into Wallet ("+QString::number(atLeastOneImportable)+")");
                importIntoWalletAction->setEnabled(true);
            }
            if(selection.count() == 1){
                copyPrivateKeyAction->setEnabled(true);
            }
        }
        if(VanityGenWorkList[tableIndexClicked].pubkey != "" && selection.count() == 1){
            copyAddressAction->setEnabled(true);
        }
        if(!VanityGenRunning){
            deleteAction->setText("Delete ("+QString::number(selection.count())+")");
            deleteAction->setEnabled(true);
        }

        contextMenu->popup(ui->tableView->viewport()->mapToGlobal(pos));
    }
}

void VanityGenPage::copyAddress()
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText( VanityGenWorkList[tableIndexClicked].pubkey );
}

void VanityGenPage::copyPrivateKey()
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText( VanityGenWorkList[tableIndexClicked].privkey );
}

void VanityGenPage::importIntoWallet()
{
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();

    QList<int> sortIndex;

    for(int i=0; i< selection.count(); i++)
    {
        QModelIndex index = selection.at(i);

        if(VanityGenWorkList[selection.at(i).row()].privkey != ""){
            sortIndex.append(index.row());
            AddressIsMine = true;
            gui->externCommand((const QString) QString("importprivkey "+VanityGenWorkList[selection.at(i).row()].privkey+" VANITYGEN false" ));
        }
    }

    qSort(sortIndex);

    for(int i=sortIndex.length()-1; i>=0 ; i--)
    {
        VanityGenWorkList.removeAt(sortIndex[i]);
    }

    rebuildTableView();
    updateUi();
    saveFile();
}

void VanityGenPage::deleteEntry()
{
    deleteRows();
}

void VanityGenPage::updateVanityGenUI(){

    // ui->checkBoxAutoImport->setEnabled((this->walletModel->getEncryptionStatus() != 1) ? true : false);

    ui->labelKeysChecked->setText("Keys checked:  "+QString::number(VanityGenKeysChecked,'g',15));

    double targ;
    QString unit;//char *unit;

    targ = VanityGenHashrate;
    unit = "key/s";
    if (targ > 1000) {
        unit = "Kkey/s";
        targ /= 1000.0;
        if (targ > 1000) {
            unit = "Mkey/s";
            targ /= 1000.0;
        }
    }
    ui->labelHashrate->setText("Your hashrate:  "+QString::number(targ,'f', 2)+QString(" ")+ QString(unit));

    QString busyString = "";

    if(VanityGenRunning){
        for(int i = 0; i<busyCounter;i++){
            busyString.append(".");
        }
    }

    QString addage;
    for(int i = 0; i<VanityGenWorkList.length(); i++)
    {
        if(VanityGenWorkList[i].state == 2){
            if(VanityGenWorkList[i].notification == 1){

                addage = "";

                VanityGenWorkList[i].notification = 0;
                if(ui->checkBoxAutoImport->checkState() == 2 && !buttonUnlockState){
                    AddressIsMine = true;
                    gui->externCommand((const QString) QString("importprivkey "+VanityGenWorkList[i].privkey+" VANITYGEN false" ));
                    VanityGenWorkList[i].privkey = "";
                    VanityGenWorkList[i].state = 3;
                    addage = "\n\n(...importing address into wallet...)";
                }
/*
#ifndef Q_OS_MAC
                trayIcon->showMessage("Slimcoin: VanityGen",
                                           "\nAddress found for pattern "+QString(VanityGenWorkList[i].pattern)+ ":\n\n"+QString(VanityGenWorkList[i].pubkey+addage),
                                           QSystemTrayIcon::Information,
                                           10000);
#endif
*/
                saveFile();
            }
        }
    }

    bool rowsChanged = false;
    for(int i = 0; i<VanityGenWorkList.length(); i++)
    {
        if(VanityGenWorkList[i].state == 3){
            VanityGenWorkList.removeAt(i);
            i--;
            rowsChanged = true;
        }
    }

    if(rowsChanged){
        saveFile();
        rebuildTableView();
    }


    for(int i = 0; i<VanityGenWorkList.length(); i++)
    {
        for(int col= 0; col <3;col ++){
            QModelIndex index = model->index(i,col, QModelIndex());
            if(col == 0){
                if(VanityGenWorkList[i].state == 2){
                    model->setData(index,VanityGenWorkList[i].pubkey);
                } else{
                    model->setData(index, VanityGenWorkList[i].pattern + ((VanityGenWorkList[i].state == 2) ? "" : busyString));
                }
            }
            if(col == 1){
                if(ui->checkBoxShowPrivKeys->checkState() > 0){
                    model->setData(index, VanityGenWorkList[i].privkey + ((VanityGenWorkList[i].state == 2) ? "" : busyString));
                } else{
                    if(VanityGenWorkList[i].privkey != ""){
                        model->setData(index, "*********************************************");
                    } else{
                        model->setData(index, (VanityGenWorkList[i].state == 2) ? "" : busyString);
                    }
                }
            }
            if(col == 2){
                if(VanityGenWorkList[i].state == 0 || !VanityGenRunning){
                    model->setData(index, "");
                } else{
                    if(VanityGenWorkList[i].state != 2){
                        double time;
                        char * unit;
                        time = VanityGenWorkList[i].difficulty.toDouble()/VanityGenHashrate;
                        unit = "s";
                        if (time > 60) {
                            time /= 60;
                            unit = "min";
                            if (time > 60) {
                                time /= 60;
                                unit = "h";
                                if (time > 24) {
                                    time /= 24;
                                    unit = "d";
                                    if (time > 365) {
                                        time /= 365;
                                        unit = "y";
                                    }
                                }
                            }
                        }

                        model->setData(index, "50% in "+QString::number(time,'f', 2)+QString(" ")+ QString(unit));//QString(VanityGenWorkList[i].difficulty));
                    } else{
                        model->setData(index,"");
                    }
                }

            }
        }
    }

    (busyCounter>10) ? busyCounter = 1 : busyCounter ++ ;

    updateUi();
}

void VanityGenPage::tableViewClicked(QItemSelection sel1, QItemSelection sel2)
{
    if(!VanityGenRunning){
        QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();

        if(selection.count()>0){
            ui->buttonDelete->setEnabled(true);
        } else{
            ui->buttonDelete->setEnabled(false);
        }

        int atLeastOneImportable = 0;
        for(int i=0; i< selection.count(); i++)
        {
            if(VanityGenWorkList[selection.at(i).row()].privkey != ""){
                atLeastOneImportable++;
            }
        }
        if(atLeastOneImportable>0 && this->walletModel->getEncryptionStatus() != 1){
            ui->buttonImport->setEnabled(true);
        } else{
            ui->buttonImport->setEnabled(false);
        }
    }
}

void VanityGenPage::deleteRows()
{
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();//selectedIndexes();//;
    QList<int> sortIndex;
    for(int i=0; i< selection.count(); i++)
    {
        QModelIndex index = selection.at(i);
        sortIndex.append(index.row());
    }
    qSort(sortIndex);

    for(int i=sortIndex.length()-1; i>=0 ; i--)
    {
        VanityGenWorkList.removeAt(sortIndex[i]);
    }

    rebuildTableView();
    updateUi();

    VanityGenRunning = false;
    saveFile();

}

int VanityGenPage::getNewJobsCount()
{
    int nrNewJobs = 0;
    for(int i = 0; i<VanityGenWorkList.length(); i++)
    {
        if(VanityGenWorkList[i].state == 0 || VanityGenWorkList[i].state == 1){
            nrNewJobs ++;
        }
    }
    return nrNewJobs;
}

void VanityGenPage::startThread(){
    int nrNewJobs = getNewJobsCount();

    if(nrNewJobs > 0){
        VanityGenRunning = !VanityGenRunning;
    } else{
        VanityGenRunning = false;
    }

    if(VanityGenRunning){

        for(int i = 0; i<VanityGenWorkList.length(); i++)
        {
            qDebug() << VanityGenWorkList[i].pattern << VanityGenWorkList[i].state;
            if(VanityGenWorkList[i].state == 1){
                VanityGenWorkList[i].state = 0;
            }
        }

        if(nrNewJobs>0){
            threadVan = new QThread();

            van = new VanityGenWork();

            van->vanityGenSetup(threadVan);
            van->moveToThread(threadVan);

            threadVan->start();
        }
    }
    else{
        stopThread();
    }
    updateUi();
    saveFile();
}

void VanityGenPage::stopThread()
{
    van->stop_threads();
    saveFile();
}

void VanityGenPage::changeAllowedText()
{
    int curpos = ui->lineEdit->cursorPosition();
    checkAllowedText(curpos);
}

void VanityGenPage::checkAllowedText(int curpos)
{
    ui->lineEdit->setValidator(new QRegExpValidator(QRegExp("[S]{1,1}[MNP-Za-k]{1,1}[1-9A-HJ-NP-Za-km-z]{14,14}"), NULL));
    QChar secondChar;
    if(ui->lineEdit->text().length() > 1){
        secondChar = ui->lineEdit->text().at(1);
    }
    if(curpos == 0){
        ui->labelAllowed->setText("Allowed(@"+QString::number(curpos)+"): S");
    } else if(curpos == 1){
        ui->labelAllowed->setText("Allowed(@"+QString::number(curpos)+"): MNPQRSTUVWXYZabcdefghijk");
    } else if(curpos == 2){
        if(secondChar == 'M'){
            ui->lineEdit->setValidator(new QRegExpValidator(QRegExp("[S]{1,1}[MNP-Za-k]{1,1}[J-NP-Za-km-z]{1,1}[1-9A-HJ-NP-Za-km-z]{13,13}"), NULL));
            ui->labelAllowed->setText("Allowed(@"+QString::number(curpos)+"): JKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz");
        } else if(secondChar == 'k'){
            ui->labelAllowed->setText("Allowed(@"+QString::number(curpos)+"): 123456789ABCDEFGHJKLMNPQRSTUVWXYZabcd");
            ui->lineEdit->setValidator(new QRegExpValidator(QRegExp("[S]{1,1}[MNP-Za-k]{1,1}[1-9A-HJNP-Za-d]{1,1}[1-9A-HJ-NP-Za-km-z]{13,13}"), NULL));
        } else{
            ui->labelAllowed->setText("Allowed(@"+QString::number(curpos)+"): 123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz");
        }
    } else{
        ui->labelAllowed->setText("Allowed(@"+QString::number(curpos)+"): 123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz");
    }
}

void VanityGenPage::updateLabelNrThreads(int nThreads)
{
    VanityGenNThreads = nThreads;
    ui->labelNrThreads->setText("Number of threads to use : " + QString::number(nThreads));
    updateUi();
}

void VanityGenPage::addPatternClicked()
{
    if(ui->lineEdit->text().length() >=1){
        VanityGenWorkList.prepend(VanGenStruct());
        // VanityGenWorkList[0].id = i;
        VanityGenWorkList[0].pattern = ui->lineEdit->text();
        VanityGenWorkList[0].privkey = "";
        VanityGenWorkList[0].pubkey = "";
        VanityGenWorkList[0].difficulty = "";
        VanityGenWorkList[0].state = 0;
        VanityGenWorkList[0].notification = 0;
    }
    rebuildTableView();
    saveFile();
}

void VanityGenPage::rebuildTableView()
{
    model->removeRows(0, model->rowCount(), QModelIndex());//clear();

    for(int row=VanityGenWorkList.length()-1;row>= 0;row--){
        QStandardItem *item = new QStandardItem();
        model->insertRow(0,item);
        for(int col= 0; col <3;col ++){
            QModelIndex index = model->index(0,col, QModelIndex());
            if(col == 0){
                model->setData(index, (VanityGenWorkList[row].state == 2) ? VanityGenWorkList[row].pubkey : VanityGenWorkList[row].pattern);//.length()ui->lineEdit->text());
            }
            if(col == 1){
                if(ui->checkBoxShowPrivKeys->checkState() == 0 && VanityGenWorkList[row].privkey != ""){
                    model->setData(index, "*********************************************");
                } else{
                    model->setData(index, VanityGenWorkList[row].privkey);
                }
            }
            if(col == 2){
                model->setData(index, "");
            }
        }
    }

    updateUi();
}

void VanityGenPage::changeMatchCase(bool state){
    VanityGenMatchCase = (state) ? 1 : 0;
    saveFile();
}

VanityGenPage::~VanityGenPage()
{
    delete ui;
}

void VanityGenPage::updateUi()
{
    ui->horizontalSlider->setEnabled(VanityGenRunning ? false : true);
    ui->checkBoxMatchCase->setEnabled(VanityGenRunning ? false : true);
    ui->lineEdit->setEnabled(VanityGenRunning ? false : true);
    ui->buttonPattern->setEnabled(VanityGenRunning ? false : true);

    ui->buttonStart->setEnabled((ui->horizontalSlider->value() > 0 && (getNewJobsCount() > 0)) ? true : false);
    ui->buttonUnlock->setText(buttonUnlockState ? "Unlock wallet" :"Lock wallet");

    VanityGenRunning ?  ui->buttonStart->setText("Stop") : ui->buttonStart->setText("Start");
}
