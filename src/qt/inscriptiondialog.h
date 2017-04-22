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

#define INSCRIPTION_DIALOG_NAME    "Inscribe"

/** "Inscription" dialog box */
class InscriptionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InscriptionDialog(QWidget *parentt = 0);
    ~InscriptionDialog();

    void setWalletModel(WalletModel *model);
    void setClientModel(ClientModel *model);

private:
    Ui::InscriptionDialog *ui;
    WalletModel *walletModel;
    ClientModel *clientModel;

private Q_SLOTS:
    void on_insertButton_clicked();
    void on_pushButton_clicked();
    void resetMessage();

Q_SIGNALS:
    void inscribed();
};

#endif // INSCRIPTIONDIALOG_H
