// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "encryptdecryptmessagedialog.h"
#include "ui_encryptdecryptmessagedialog.h"

#include "addressbookpage.h"
#include "guiutil.h"
#include "walletmodel.h"

#include "base58.h"
#include "init.h"
#include "util.h"
#include "wallet.h"

#include <string>
#include <vector>

#include <QClipboard>

EncryptDecryptMessageDialog::EncryptDecryptMessageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EncryptDecryptMessageDialog),
    model(0)
{
    ui->setupUi(this);

#if QT_VERSION >= 0x040700
    ui->signatureOut_ED->setPlaceholderText(tr("Click \"Encrypt Message\" to encrypt the message"));
    ui->signatureIn_DD->setPlaceholderText(tr("Click \"Decrypt Message\" to decrypt the message"));
    ui->addressIn_ED->setPlaceholderText(tr("Enter a Slimcoin address"));
    ui->addressIn_DD->setPlaceholderText(tr("Enter a Slimcoin address"));
#endif

    GUIUtil::setupAddressWidget(ui->addressIn_ED, this);
    GUIUtil::setupAddressWidget(ui->addressIn_DD, this);

    ui->addressIn_ED->installEventFilter(this);
    ui->messageIn_ED->installEventFilter(this);
    ui->signatureOut_ED->installEventFilter(this);
    ui->addressIn_DD->installEventFilter(this);
    ui->messageIn_DD->installEventFilter(this);
    ui->signatureIn_DD->installEventFilter(this);

    ui->signatureOut_ED->setFont(GUIUtil::bitcoinAddressFont());
    ui->signatureIn_DD->setFont(GUIUtil::bitcoinAddressFont());
}

EncryptDecryptMessageDialog::~EncryptDecryptMessageDialog()
{
    delete ui;
}

void EncryptDecryptMessageDialog::setModel(WalletModel *model)
{
    this->model = model;
}

void EncryptDecryptMessageDialog::setAddress_ED(const QString &address)
{
    ui->addressIn_ED->setText(address);
    ui->messageIn_ED->setFocus();
}

void EncryptDecryptMessageDialog::setAddress_DD(const QString &address)
{
    ui->addressIn_DD->setText(address);
    ui->messageIn_DD->setFocus();
}

void EncryptDecryptMessageDialog::showTab_ED(bool fShow)
{
    ui->tabWidget->setCurrentIndex(0);
    if (fShow)
        this->show();
}

void EncryptDecryptMessageDialog::showTab_DD(bool fShow)
{
    ui->tabWidget->setCurrentIndex(1);
    if (fShow)
        this->show();
}

void EncryptDecryptMessageDialog::on_addressBookButton_ED_clicked()
{
    if (model && model->getAddressTableModel())
    {
        AddressBookPage dlg(AddressBookPage::ForSending, AddressBookPage::ReceivingTab, this);
        dlg.setModel(model->getAddressTableModel());
        if (dlg.exec())
        {
            setAddress_ED(dlg.getReturnValue());
        }
    }
}

void EncryptDecryptMessageDialog::on_pasteButton_ED_clicked()
{
    setAddress_ED(QApplication::clipboard()->text());
}

void EncryptDecryptMessageDialog::on_encryptMessageButton_ED_clicked()
{
    if (!model)
        return;

    /* Clear old signature to ensure users don't get confused on error with an old signature displayed */
    ui->signatureOut_ED->clear();

    CBitcoinAddress addr(ui->addressIn_ED->text().toStdString());
    if (!addr.IsValid())
    {
        ui->statusLabel_ED->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_ED->setText(tr("The entered address is invalid.") + QString(" ") + tr("Please check the address and try again."));
        return;
    }
    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
    {
        ui->addressIn_ED->setValid(false);
        ui->statusLabel_ED->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_ED->setText(tr("The entered address does not refer to a key.") + QString(" ") + tr("Please check the address and try again."));
        return;
    }

    WalletModel::UnlockContext ctx(model->requestUnlock());
    if (!ctx.isValid())
    {
        ui->statusLabel_ED->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_ED->setText(tr("Wallet unlock was cancelled."));
        return;
    }

    CKey key;
    if (!pwalletMain->GetKey(keyID, key))
    {
        ui->statusLabel_ED->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_ED->setText(tr("Private key for the entered address is not available."));
        return;
    }

    // void CKey::EncryptData(const std::vector<unsigned char> &plaindata, std::vector<unsigned char> &encdata) {
    std::vector<unsigned char> vchEncrypted;
    std::string strData = ui->messageIn_ED->document()->toPlainText().toStdString();
    key.EncryptData(std::vector<unsigned char>(strData.begin(), strData.end()), vchEncrypted);
    QString encrypted_message = QString::fromStdString(EncodeBase58Check(vchEncrypted));

    ui->messageIn_ED->setPlainText(encrypted_message);
    //ui->messageIn_ED->setReadOnly();

    ui->statusLabel_ED->setStyleSheet("QLabel { color: green; }");
    ui->statusLabel_ED->setText(QString("<nobr>") + tr("Message encrypted.") + QString("</nobr>"));
    ui->signatureOut_ED->setText(encrypted_message);
}

void EncryptDecryptMessageDialog::on_copyEncryptedDataButton_ED_clicked()
{
    QApplication::clipboard()->setText(ui->signatureOut_ED->text());
}

void EncryptDecryptMessageDialog::on_clearButton_ED_clicked()
{
    ui->addressIn_ED->clear();
    ui->messageIn_ED->clear();
    ui->signatureOut_ED->clear();
    ui->statusLabel_ED->clear();

    ui->addressIn_ED->setFocus();
}

void EncryptDecryptMessageDialog::on_addressBookButton_DD_clicked()
{
    if (model && model->getAddressTableModel())
    {
        AddressBookPage dlg(AddressBookPage::ForSending, AddressBookPage::SendingTab, this);
        dlg.setModel(model->getAddressTableModel());
        if (dlg.exec())
        {
            setAddress_DD(dlg.getReturnValue());
        }
    }
}

void EncryptDecryptMessageDialog::on_decryptMessageButton_DD_clicked()
{
    CBitcoinAddress addr(ui->addressIn_DD->text().toStdString());
    if (!addr.IsValid())
    {
        ui->statusLabel_DD->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_DD->setText(tr("The entered address is invalid.") + QString(" ") + tr("Please check the address and try again."));
        return;
    }
    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
    {
        ui->addressIn_DD->setValid(false);
        ui->statusLabel_DD->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_DD->setText(tr("The entered address does not refer to a key.") + QString(" ") + tr("Please check the address and try again."));
        return;
    }

    CKey key;
    if (!pwalletMain->GetKey(keyID, key))
    {
        ui->statusLabel_ED->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_ED->setText(tr("Private key for the entered address is not available."));
        return;
    }

    std::string strData = ui->messageIn_ED->document()->toPlainText().toStdString();
    std::vector<unsigned char> vchEncrypted;
    if(!DecodeBase58Check(strData, vchEncrypted)) {
        ui->statusLabel_DD->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_DD->setText(tr("The message could not be decrypted.") + QString(" ") + tr("Please check the encrypted data and try again."));
        return;
    };

    std::vector<unsigned char> vchDecrypted;
    key.DecryptData(vchEncrypted, vchDecrypted);
    QString message = QString::fromStdString(std::string((const char *) &vchDecrypted[0], vchDecrypted.size()));
    ui->signatureIn_DD->setText(message);

    // CPubKey pubkey;
    // if (!pubkey.RecoverCompact(Hash(ss.begin(), ss.end()), vchSig))
    // {
    //     ui->signatureIn_DD->setValid(false);
    //     ui->statusLabel_DD->setStyleSheet("QLabel { color: red; }");
    //     ui->statusLabel_DD->setText(tr("The address did not decrypt a message.") + QString(" ") + tr("Please check the encrypted data and try again."));
    //     return;
    // }

    // if (!(CBitcoinAddress(pubkey.GetID()) == addr))
    // {
    //     ui->statusLabel_DD->setStyleSheet("QLabel { color: red; }");
    //     ui->statusLabel_DD->setText(QString("<nobr>") + tr("Message decryption failed.") + QString("</nobr>"));
    //     return;
    // }

    ui->statusLabel_DD->setStyleSheet("QLabel { color: green; }");
    ui->statusLabel_DD->setText(QString("<nobr>") + tr("Message decrypted.") + QString("</nobr>"));
}

void EncryptDecryptMessageDialog::on_clearButton_DD_clicked()
{
    ui->addressIn_DD->clear();
    ui->signatureIn_DD->clear();
    ui->messageIn_DD->clear();
    ui->statusLabel_DD->clear();

    ui->addressIn_DD->setFocus();
}

bool EncryptDecryptMessageDialog::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::FocusIn)
    {
        if (ui->tabWidget->currentIndex() == 0)
        {
            /* Clear status message on focus change */
            ui->statusLabel_ED->clear();

            /* Select generated signature */
            if (object == ui->signatureOut_ED)
            {
                ui->signatureOut_ED->selectAll();
                return true;
            }
        }
        else if (ui->tabWidget->currentIndex() == 1)
        {
            /* Clear status message on focus change */
            ui->statusLabel_DD->clear();
        }
    }
    return QDialog::eventFilter(object, event);
}
