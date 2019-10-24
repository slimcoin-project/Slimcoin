#ifndef OVERVIEWPAGE_H
#define OVERVIEWPAGE_H

#include <QWidget>
#include <walletmodel.h>

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

namespace Ui {
    class OverviewPage;
}
class ClientModel;
class WalletModel;
class TxViewDelegate;

/** Overview ("home") page widget */
class OverviewPage : public QWidget
{
    Q_OBJECT

public:
    explicit OverviewPage(QWidget *parent = 0);
    ~OverviewPage();

    void setModel(WalletModel *model);
    void setClientModel(ClientModel *model);
    void updatePlot(int count);

public slots:
    void setBalance(qint64 balance, qint64 stake, qint64 unconfirmedBalance, qint64 immatureBalance, qint64 reserveBalance, BurnCoinsBalances burnBalances);
    void setNumTransactions(int count);
    void setReserveBalance(qint64 nreserveBalance);

signals:
    void transactionClicked(const QModelIndex &index);

private:
    Ui::OverviewPage *ui;
    ClientModel *clientModel;
    WalletModel *model;
    qint64 currentBalance;
    qint64 currentStake;
    qint64 currentReserveBalance;
    qint64 currentUnconfirmedBalance;
    qint64 currentImmatureBalance;

    qint64 currentNetBurnCoins;
    qint64 currentEffectiveBurnCoins;
    qint64 currentImmatureBurnCoins;
    qint64 currentDecayedBurnCoins;
    QVector<double> vX;
    QVector<double> vY;
    QVector<double> vX2;
    QVector<double> vY2;


    TxViewDelegate *txdelegate;

private slots:
    void displayUnitChanged();
    void reserveBalanceChanged();
};

#endif // OVERVIEWPAGE_H
