#include "inscriptiondialog.h"
#include "ui_inscriptiondialog.h"
#include "burncoinsdialog.h"

// #include "version.h"
#include "init.h" // for pwalletMain
#include "util.h"
#include "guiutil.h"

#include "addresstablemodel.h"
#include "optionsmodel.h"
#include "script.h"
#include "bitcoinunits.h"

#include "sendcoinsdialog.h"
#include "sendcoinsentry.h"
#include "clientmodel.h"
#include "walletmodel.h"

#include <QCloseEvent>
#include <QLabel>
#include <QRegExp>
#include <QTextTable>
#include <QTextCursor>
#include <QVBoxLayout>
#include <QtDebug>


/** class "InscriptionDialog" dialog box */
InscriptionDialog::InscriptionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InscriptionDialog),
    walletModel(0),
    clientModel(0)
{
    ui->setupUi(this);
}

void InscriptionDialog::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if(model)
    {
    }
}

void InscriptionDialog::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
    if(model)
    {
    }
}

InscriptionDialog::~InscriptionDialog()
{
    delete ui;
}

void InscriptionDialog::on_insertButton_clicked()
{
    QString txmsg = ui->lineEditMsg->text();
    // qDebug() << "Text message:" << txmsg << "Other value:" << i;
    qDebug() << "Text message:" << txmsg;

    if ( std::string(txmsg.toStdString().c_str()).length() > 80 )
    {
        QMessageBox::question(this, tr("Message error"),
                              tr("Message length exceeds the limit (80 bytes)!"), // MAX_OP_RETURN_RELAY
              QMessageBox::Cancel,
              QMessageBox::Cancel);
        return;
    }

    QList<SendCoinsRecipient> recipients;
    SendCoinsRecipient rcpt;
    QString vchDefaultAddr = CBitcoinAddress(pwalletMain->vchDefaultKey.GetID()).ToString().c_str();
    rcpt.address = vchDefaultAddr;
    rcpt.label = "inscription";
    rcpt.amount = 1*CENT;
    recipients.append(rcpt);

    qDebug() << "Recipient created" << vchDefaultAddr << " amount" << 1*CENT;

    // send the transaction
    WalletModel::SendCoinsReturn sendstatus = walletModel->sendCoins(recipients, txmsg, false);

    switch(sendstatus.status)
    {
    case WalletModel::InvalidAddress:
        QMessageBox::warning(this, tr("Send Coins"),
                             tr("The recepient address is not valid, please recheck."),
                             QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::InvalidAmount:
        QMessageBox::warning(this, tr("Send Coins"),
                             tr("The amount to pay must be at least one cent (0.01)."),
                             QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::AmountExceedsBalance:
        QMessageBox::warning(this, tr("Send Coins"),
                             tr("Amount exceeds your balance"),
                             QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::AmountWithFeeExceedsBalance:
        QMessageBox::warning(this, tr("Send Coins"),
                             tr("Total exceeds your balance when the %1 transaction fee is included").
                             arg(BitcoinUnits::formatWithUnit(BitcoinUnits::BTC, sendstatus.fee)),
                             QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::DuplicateAddress:
        QMessageBox::warning(this, tr("Send Coins"),
                             tr("Duplicate address found, can only send to each address once in one send operation"),
                             QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::TransactionCreationFailed:
        QMessageBox::warning(this, tr("Send Coins"),
                             tr("Error: Transaction creation failed  "),
                             QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::TransactionCommitFailed:
        QMessageBox::warning(this, tr("Send Coins"),
                             tr("Error: The transaction was rejected.  This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here."),
                             QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::BadBurningCoins:
        QMessageBox::warning(this, tr("Send Coins"),
                             tr("You are sending coins to a burn address without using the dedicated \"" BURN_COINS_DIALOG_NAME "\" tab. If you want to burn coins, use the dedicated tab instead of this \"" SEND_COINS_DIALOG_NAME "\" tab. \n\nSending coins aborted."),
                             QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::Aborted: // User aborted, nothing to do
        break;
    case WalletModel::OK:
        accept();
        QMessageBox::information(NULL, tr("Inscription"), tr("Inscribe on blockchain, Yes"), QMessageBox::Yes , QMessageBox::Yes);
        ui->lineEditMsg->setText("Message inscribed.");
        break;
    }

    /*
    {
        QMessageBox::information(NULL, tr("Failed"), tr("Inscription failed"), QMessageBox::Yes , QMessageBox::Yes);
        ui->lineEditMsg->setText(txmsg);
    }
    */
}


void InscriptionDialog::on_pushButton_clicked()
{
    close();
}

void InscriptionDialog::resetMessage()
{
    ui->lineEditMsg->setText("");
}
