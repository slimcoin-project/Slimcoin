#ifndef MININGPAGE_H
#define MININGPAGE_H

#include <QWidget>
#include <memory>
#if BOOST_VERSION > 105600
#include <boost/move/unique_ptr.hpp>
#endif
#include "walletmodel.h"

namespace Ui {
class MiningPage;
}

class ClientModel;
class WalletModel;

class MiningPage : public QWidget
{
    Q_OBJECT

public:
    explicit MiningPage(QWidget *parent = 0);
    ~MiningPage();

    void setModel(WalletModel *model);
    void setClientModel(ClientModel *model);
    void updatePlot();

private:
    Ui::MiningPage *ui;
    ClientModel *clientModel;
    WalletModel *model;
#if BOOST_VERSION > 105600
    boost::movelib::unique_ptr<WalletModel::UnlockContext> unlockContext;
#else
    std::unique_ptr<WalletModel::UnlockContext> unlockContext;
#endif
    bool hasMiningprivkey;

    QVector<double> vX;
    QVector<double> vY;

    QVector<double> vX2;
    QVector<double> vY2;

    void restartMining(bool fGenerate);
    void timerEvent(QTimerEvent *event);
    void updateUI();

private slots:

    void changeNumberOfCores(int i);
    void switchMining();
};


#endif // MININGPAGE_H
