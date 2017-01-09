#include "inscriptiondialog.h"
#include "ui_inscriptiondialog.h"

// #include "version.h"
#include "init.h"
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

// Value bound in script.h
// int MAX_OP_RETURN_RELAY = 120; 
int DUST_HARD_LIMIT = 500;

/** class "InscriptionDialog" dialog box */
InscriptionDialog::InscriptionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InscriptionDialog),
    clientModel(0)
{
    ui->setupUi(this);
}

void InscriptionDialog::setModel(WalletModel *model)
{
    this->model = model;
    if(model)
    {
    }
}

void InscriptionDialog::setClientModel(ClientModel *model)
{
    this->clientModel = model;
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
    if ( std::string(txmsg.toStdString().c_str()).length() > 120 )
    {
        QMessageBox::question(this, tr("Message error"),
                              tr("Message length exceeds the limit (120 bytes)!"),
              QMessageBox::Cancel,
              QMessageBox::Cancel);
        return;
    }

    // Officially:
    // sendstatus = model->sendCoins(recipients, txmsg);

    const char* msg = txmsg.toStdString().c_str();
    CScript scriptMsg;
    // static unsigned char pchMessageStartTestOld[4] = { 0xbd, 0x1e, 0x2f, 0xa3 };
    const unsigned char msgHeader[5] = { 0xfa, 0xce, 0x01, 0x00, 0x00 };
    std::vector<unsigned char> vMsg;
    int i;
    for ( i = 0; i < 4; ++ i )
        vMsg.push_back(msgHeader[i]);
    for ( i = 0; i < std::strlen(msg); ++ i )
        vMsg.push_back(msg[i]);

    scriptMsg << OP_RETURN << vMsg;


    /* FIXME: just create a 0 SML tx for the wallet default address
    QList<SendCoinsRecipient> recipients;
    SendCoinsRecipient rcptmp;
    // Payment request
    if (rcptmp.paymentRequest.IsInitialized())
        return ;
    rcptmp.address = addrOP;
    rcptmp.label = "inscription";
    rcptmp.amount = DUST_HARD_LIMIT;
    rcptmp.message = textOP;
    recipients.append(rcptmp);

    // Format confirmation message
    QStringList formatted;
    Q_FOREACH(const SendCoinsRecipient &rcp, recipients)
    {
        // generate bold amount string
        QString amount = "<b>" + BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), rcp.amount);
        amount.append("</b>");
        // generate monospace address string
        QString address = "<span style='font-family: monospace;'>" + rcp.address;
        address.append("</span>");

        QString recipientElement;
        if (!rcp.paymentRequest.IsInitialized()) // normal payment
        {
            if(rcp.label.length() > 0) // label with address
            {
                recipientElement = tr("%1 to %2").arg(amount, GUIUtil::HtmlEscape(rcp.label));
                recipientElement.append(QString(" (%1)").arg(address));
            }
            else // just address
            {
                recipientElement = tr("%1 to %2").arg(amount, address);
            }
        }
        else // insecure payment request
        {
            recipientElement = tr("%1 to %2").arg(amount, address);
        }

        formatted.append(recipientElement);
    }

    // prepare transaction for getting txFee earlier
    WalletModelTransaction currentTransaction(recipients);
    WalletModel::SendCoinsReturn prepareStatus;
    prepareStatus = model->prepareTransaction(currentTransaction);

    // process prepareStatus and on error generate message shown to user
    processSendCoinsReturn(prepareStatus,
        BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), currentTransaction.getTransactionFee()));

    if(prepareStatus.status != WalletModel::OK) {
        return;
    }

    QString questionString = tr("Are you sure you want to send?");
    questionString.append("<br /><br />%1");
    qint64 txFee = currentTransaction.getTransactionFee();
    if(txFee > 0)
    {
        // append fee string if a fee is required
        questionString.append("<hr /><span style='color:#aa0000;'>");
        questionString.append(BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), txFee));
        questionString.append("</span> ");
        questionString.append(tr("added as transaction fee"));
    }

    // add total amount in all subdivision units
    questionString.append("<hr />");
    qint64 totalAmount = currentTransaction.getTotalTransactionAmount() + txFee;
    QStringList alternativeUnits;
    Q_FOREACH(BitcoinUnits::Unit u, BitcoinUnits::availableUnits())
    {
        if(u != model->getOptionsModel()->getDisplayUnit())
            alternativeUnits.append(BitcoinUnits::formatWithUnit(u, totalAmount));
    }
    questionString.append(tr("Total Amount %1 (= %2)")
        .arg(BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), totalAmount))
        .arg(alternativeUnits.join(" " + tr("or") + " ")));
    QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Confirm send coins"),
        questionString.arg(formatted.join("<br />")),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);
    if(retval != QMessageBox::Yes)
    {
        return;
    }

    // now send the prepared transaction
    WalletModel::SendCoinsReturn sendStatus = model->sendCoins(currentTransaction);
    if (sendStatus.status == WalletModel::OK)
    {
        QMessageBox::information(NULL, tr("Inscription"), tr("Inscribe on blockchain, Yes"), QMessageBox::Yes , QMessageBox::Yes);
        ui->txtComment->setText("");
    }
    */
    QMessageBox::information(NULL, tr("Inscription"), tr("Inscribe on blockchain, Yes"), QMessageBox::Yes , QMessageBox::Yes);
    ui->lineEditMsg->setText("Message inscribed.");
}

/*
void InscriptionDialog::processSendCoinsReturn(const WalletModel::SendCoinsReturn &sendCoinsReturn, const QString &msgArg)
{
    QPair<QString, CClientUIInterface::MessageBoxFlags> msgParams;
    // Default to a warning message, override if error message is needed
    msgParams.second = CClientUIInterface::MSG_WARNING;

    // This comment is specific to SendCoinsDialog usage of WalletModel::SendCoinsReturn.
    // WalletModel::TransactionCommitFailed is used only in WalletModel::sendCoins()
    // all others are used only in WalletModel::prepareTransaction()
    switch(sendCoinsReturn.status)
    {
    case WalletModel::InvalidAddress:
        msgParams.first = tr("The recipient address is not valid. Please recheck.");
        break;
    case WalletModel::InvalidAmount:
        msgParams.first = tr("The amount to pay must be larger than 0.");
        break;
    case WalletModel::AmountExceedsBalance:
        msgParams.first = tr("The amount exceeds your balance.");
        break;
    case WalletModel::AmountWithFeeExceedsBalance:
        msgParams.first = tr("The total exceeds your balance when the %1 transaction fee is included.").arg(msgArg);
        break;
    case WalletModel::DuplicateAddress:
        msgParams.first = tr("Duplicate address found: addresses should only be used once each.");
        break;
    case WalletModel::TransactionCreationFailed:
        msgParams.first = tr("Transaction creation failed!");
        msgParams.second = CClientUIInterface::MSG_ERROR;
        break;
    case WalletModel::TransactionCommitFailed:
        msgParams.first = tr("The transaction was rejected! This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here.");
        msgParams.second = CClientUIInterface::MSG_ERROR;
        break;
    case WalletModel::AbsurdFee:
        msgParams.first = tr("A fee higher than %1 is considered an absurdly high fee.").arg(BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), 10000000));
        break;
    case WalletModel::PaymentRequestExpired:
        msgParams.first = tr("Payment request expired.");
        msgParams.second = CClientUIInterface::MSG_ERROR;
        break;
    // included to prevent a compiler warning.
    case WalletModel::OK:
    default:
        return;
    }

    Q_EMIT message(tr("Send Coins"), msgParams.first, msgParams.second);
}
*/

void InscriptionDialog::on_pushButton_clicked()
{
    close();
}

void InscriptionDialog::resetMessage()
{
    ui->lineEditMsg->setText("");
}
