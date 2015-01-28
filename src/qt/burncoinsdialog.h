#ifndef BURNCOINSDIALOG_H
#define BURNCOINSDIALOG_H

#include <QDialog>

namespace Ui {
    class BurnCoinsDialog;
}

class WalletModel;
class BurnCoinsEntry;

QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE

#define BURN_COINS_DIALOG_NAME    "Burn coins"

/** Dialog for burning bitcoins */
class  BurnCoinsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BurnCoinsDialog(QWidget *parent = 0);
    ~BurnCoinsDialog();

    void setModel(WalletModel *model);

public slots:
    void clear();
    void reject();
    void accept();
    void setBalance(qint64 balance, qint64 stake, qint64 unconfirmedBalance);

private:
    Ui::BurnCoinsDialog *ui;
    WalletModel *model;
    bool fNewRecipientAllowed;

private slots:
    void on_sendButton_clicked();
    BurnCoinsEntry *addEntry();

};

#endif // BURNCOINSDIALOG_H
