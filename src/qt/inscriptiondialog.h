#ifndef INSCRIPTIONDIALOG_H
#define INSCRIPTIONDIALOG_H

#include <QDialog>
#include <QObject>
#include "walletmodel.h"
#include "clientmodel.h"

namespace Ui {
    class InscriptionDialog;
}
class ClientModel;
class SendCoinsRecipient;

/** "Inscription" dialog box */
class InscriptionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InscriptionDialog(QWidget *parent);
    ~InscriptionDialog();

    void setWalletModel(WalletModel *model);
    void setClientModel(ClientModel *model);

private:
    Ui::InscriptionDialog *ui;
    WalletModel *walletModel;
    ClientModel *clientModel;
    // void processSendCoinsReturn(const WalletModel::SendCoinsReturn &sendCoinsReturn, const QString &msgArg = QString());

private Q_SLOTS:
    void on_insertButton_clicked();
    void on_pushButton_clicked();
    void resetMessage();

Q_SIGNALS:
    void message(const QString &title, const QString &message, unsigned int style);
};

#endif // INSCRIPTIONDIALOG_H
