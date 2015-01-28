#ifndef BURNCOINSENTRY_H
#define BURNCOINSENTRY_H

#include <QFrame>

namespace Ui {
    class BurnCoinsEntry;
}

class WalletModel;
class SendCoinsRecipient;

/** A single entry in the dialog for sending bitcoins. */
class BurnCoinsEntry : public QFrame
{
    Q_OBJECT

public:
    explicit BurnCoinsEntry(QWidget *parent = 0);
    ~BurnCoinsEntry();

    void setModel(WalletModel *model);
    bool validate();
    SendCoinsRecipient getValue();

    /** Return whether the entry is still empty and unedited */
    bool isClear();

    void setValue(const qint64 &value);

    void setFocus();

public slots:
    void clear();

signals:
    void removeEntry(BurnCoinsEntry *entry);

private:
    Ui::BurnCoinsEntry *ui;
    WalletModel *model;
};

#endif // BURNCOINSENTRY_H
