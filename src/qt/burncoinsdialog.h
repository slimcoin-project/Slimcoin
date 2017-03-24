#ifndef BURNCOINSDIALOG_H
#define BURNCOINSDIALOG_H

#include <QDialog>
#include <QObject>
#include "walletmodel.h"
#include "clientmodel.h"

namespace Ui {
    class BurnCoinsDialog;
}

class WalletModel;
class BurnCoinsEntry;

#define BURN_COINS_DIALOG_NAME    "Burn coins"

/** Dialog for burning bitcoins */
class  BurnCoinsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BurnCoinsDialog(QWidget *parentt = 0);
    ~BurnCoinsDialog();

    void setModel(WalletModel *model);

public slots:
    void clear();
    /*
    void reject();
    void accept();
    */
    void setBalance(qint64 balance, qint64 stake, qint64 unconfirmedBalance, qint64 reserveBalance);

private:
    Ui::BurnCoinsDialog *ui;
    WalletModel *model;
    bool fNewRecipientAllowed;

private Q_SLOTS:
    void on_sendButton_clicked();
    BurnCoinsEntry *addEntry();

Q_SIGNALS:
    void message(const QString &title, const QString &message, unsigned int style);
};

#endif // BURNCOINSDIALOG_H
