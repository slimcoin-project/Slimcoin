// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ENCRYPTDECRYPTMESSAGEDIALOG_H
#define ENCRYPTDECRYPTMESSAGEDIALOG_H

#include <QDialog>

class WalletModel;

namespace Ui {
    class EncryptDecryptMessageDialog;
}

class EncryptDecryptMessageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EncryptDecryptMessageDialog(QWidget *parent);
    ~EncryptDecryptMessageDialog();

    void setModel(WalletModel *model);
    void setAddress_ED(const QString &address);
    void setAddress_DD(const QString &address);

    void showTab_ED(bool fShow);
    void showTab_DD(bool fShow);

protected:
    bool eventFilter(QObject *object, QEvent *event);

private:
    Ui::EncryptDecryptMessageDialog *ui;
    WalletModel *model;

private slots:
    /* sign message */
    void on_addressBookButton_ED_clicked();
    void on_pasteButton_ED_clicked();
    void on_encryptMessageButton_ED_clicked();
    void on_copySignatureButton_ED_clicked();
    void on_clearButton_ED_clicked();
    /* verify message */
    void on_addressBookButton_DD_clicked();
    void on_decryptMessageButton_DD_clicked();
    void on_clearButton_DD_clicked();
};

#endif // ENCRYPTDECRYPTMESSAGEDIALOG_H
