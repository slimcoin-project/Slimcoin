#include "overviewpage.h"
#include "ui_overviewpage.h"

#include "clientmodel.h"
#include "walletmodel.h"
#include "bitcoinunits.h"
#include "optionsmodel.h"
#include "transactiontablemodel.h"
#include "transactionfilterproxy.h"
#include "guiutil.h"
#include "guiconstants.h"
#include "main.h"
#include "bitcoinrpc.h"
#include "util.h"

#include <QAbstractItemDelegate>
#include <QPainter>

#define DECORATION_SIZE 64
#define NUM_ITEMS 3

class TxViewDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    TxViewDelegate(): QAbstractItemDelegate(), unit(BitcoinUnits::BTC)
    {

    }

    inline void paint(QPainter *painter, const QStyleOptionViewItem &option,
                                        const QModelIndex &index ) const
    {
        painter->save();

        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QRect mainRect = option.rect;
        QRect decorationRect(mainRect.topLeft(), QSize(DECORATION_SIZE, DECORATION_SIZE));
        int xspace = DECORATION_SIZE + 8;
        int ypad = 6;
        int halfheight = (mainRect.height() - 2*ypad)/2;
        QRect amountRect(mainRect.left() + xspace, mainRect.top()+ypad, mainRect.width() - xspace, halfheight);
        QRect addressRect(mainRect.left() + xspace, mainRect.top()+ypad+halfheight, mainRect.width() - xspace, halfheight);
        icon.paint(painter, decorationRect);

        QDateTime date = index.data(TransactionTableModel::DateRole).toDateTime();
        QString address = index.data(Qt::DisplayRole).toString();
        qint64 amount = index.data(TransactionTableModel::AmountRole).toLongLong();
        bool confirmed = index.data(TransactionTableModel::ConfirmedRole).toBool();
        QVariant value = index.data(Qt::ForegroundRole);
        QColor foreground = option.palette.color(QPalette::Text);
        if(value.canConvert<QBrush>())
        {
            QBrush brush = qvariant_cast<QBrush>(value);
            foreground = brush.color();
        }
        painter->setPen(foreground);
        painter->drawText(addressRect, Qt::AlignLeft|Qt::AlignVCenter, address);

        if(amount < 0)
        {
            foreground = COLOR_NEGATIVE;
        }
        else if(!confirmed)
        {
            foreground = COLOR_UNCONFIRMED;
        }
        else
        {
            foreground = option.palette.color(QPalette::Text);
        }
        painter->setPen(foreground);
        QString amountText = BitcoinUnits::formatWithUnit(unit, amount, true);
        if(!confirmed)
        {
            amountText = QString("[") + amountText + QString("]");
        }
        painter->drawText(amountRect, Qt::AlignRight|Qt::AlignVCenter, amountText);

        painter->setPen(option.palette.color(QPalette::Text));
        painter->drawText(amountRect, Qt::AlignLeft|Qt::AlignVCenter, GUIUtil::dateTimeStr(date));

        painter->restore();
    }

    inline QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        return QSize(DECORATION_SIZE, DECORATION_SIZE);
    }

    int unit;

};
#include "overviewpage.moc"

OverviewPage::OverviewPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OverviewPage),
    currentBalance(-1),
    currentStake(0),
    currentReserveBalance(0),
    currentUnconfirmedBalance(-1),
    currentWatchOnlyBalance(-1),
    currentWatchUnconfBalance(-1),
    currentWatchImmatureBalance(-1),
    currentImmatureBalance(-1),
    txdelegate(new TxViewDelegate())
{
    ui->setupUi(this);

    // Balance: <balance>
    ui->labelBalance->setFont(QFont("Monospace", -1, QFont::Bold));
    ui->labelBalance->setToolTip(tr("Your current balance"));
    ui->labelBalance->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);

    // slimcoin: stake: <stake>
    ui->labelStake->setFont(QFont("Monospace", -1, QFont::Bold));
    ui->labelStake->setToolTip(tr("Your current stake"));
    ui->labelStake->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);

    // Unconfirmed balance: <balance>
    ui->labelUnconfirmed->setFont(QFont("Monospace", -1, QFont::Bold));
    ui->labelUnconfirmed->setToolTip(tr("Total of transactions that have yet to be confirmed, and do not yet count toward the current balance"));
    ui->labelUnconfirmed->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);

    // Immature balance: <balance>
    ui->labelImmature->setFont(QFont("Monospace", -1, QFont::Bold));
    ui->labelImmature->setToolTip(tr("Total of transactions that have yet to be confirmed, and do not yet count toward the current balance"));
    ui->labelImmature->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);

    ui->labelNumTransactions->setToolTip(tr("Total number of transactions in wallet"));
    ui->labelReserveBalance->setToolTip(tr("Reserve balance of coins, excluded from staking."));

    // Recent transactions
    ui->listTransactions->setStyleSheet("QListView { background:transparent }");
    ui->listTransactions->setItemDelegate(txdelegate);
    ui->listTransactions->setIconSize(QSize(DECORATION_SIZE, DECORATION_SIZE));
    ui->listTransactions->setSelectionMode(QAbstractItemView::NoSelection);
    ui->listTransactions->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE + 2));
    ui->listTransactions->setAttribute(Qt::WA_MacShowFocusRect, false);

    // Net Burnt Coins: <total burnt coins>
    ui->labelNetBurnCoins->setFont(QFont("Monospace", -1, QFont::Bold));
    ui->labelNetBurnCoins->setToolTip(tr("The total amount of coins burned"));
    ui->labelNetBurnCoins->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);

    // Effective Burnt Coins: <active burnt coins>
    ui->labelEffBurnCoins->setFont(QFont("Monospace", -1, QFont::Bold));
    ui->labelEffBurnCoins->setToolTip(tr("Burnt coins that are currently active"));
    ui->labelEffBurnCoins->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);

    // Immature Burnt Coins: <immature burnt coins>
    ui->labelImmBurnCoins->setFont(QFont("Monospace", -1, QFont::Bold));
    ui->labelImmBurnCoins->setToolTip(tr("Burnt coins that are immature (not yet active)"));
    ui->labelImmBurnCoins->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);

    // Immature Burnt Coins: <immature burnt coins>
    ui->labelDecayBurnCoins->setFont(QFont("Monospace", -1, QFont::Bold));
    ui->labelDecayBurnCoins->setToolTip(tr("Burnt coins are no longer active"));
    ui->labelDecayBurnCoins->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);

    connect(ui->listTransactions, SIGNAL(clicked(QModelIndex)), this, SIGNAL(transactionClicked(QModelIndex)));

    if(GetBoolArg("-chart", true))
    {
        // setup Plot
        // create graph
        ui->diffplot->addGraph();

        // Argh can't get the background to work.
        //QPixmap background = QPixmap(":/images/splash_testnet");
        //ui->diffplot->setBackground(background);
        //ui->diffplot->setBackground(QBrush(QWidget::palette().color(this->backgroundRole())));

        // give the axes some labels:
        ui->diffplot->xAxis->setLabel("Blocks");
        ui->diffplot->yAxis->setLabel("Difficulty");

        // set the pens
        ui->diffplot->graph(0)->setPen(QPen(QColor(255, 165, 18)));
        ui->diffplot->graph(0)->setLineStyle(QCPGraph::lsNone);
        ui->diffplot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, QColor(255, 165, 18), 1));

        // set axes label fonts:
        QFont label = font();
        ui->diffplot->xAxis->setTickLabels(false);
        // ui->diffplot->xAxis->setLabelFont(label);
        // ui->diffplot->xAxis->setTickLabelFont(label);
        // ui->diffplot->xAxis->setTickLabelRotation(15);
        ui->diffplot->yAxis->setLabelFont(label);
        ui->diffplot->yAxis->setTickLabelFont(label);
    }
    else
    {
        ui->diffplot->setVisible(false);
    }
}

OverviewPage::~OverviewPage()
{
    delete ui;
}

void OverviewPage::setBalance(qint64 balance, qint64 stake, qint64 unconfirmedBalance, qint64 immatureBalance, qint64 reserveBalance, BurnCoinsBalances burnBalances, qint64 watchOnlyBalance, qint64 watchUnconfBalance, qint64 watchImmatureBalance)
{
    int unit = model->getOptionsModel()->getDisplayUnit();
    currentBalance = balance;
    currentStake = stake;
    currentUnconfirmedBalance = unconfirmedBalance;
    currentImmatureBalance = immatureBalance;
    currentReserveBalance = reserveBalance;
    currentWatchOnlyBalance = watchOnlyBalance;
    currentWatchUnconfBalance = watchUnconfBalance;
    currentWatchImmatureBalance = watchImmatureBalance;

    ui->labelBalance->setText(BitcoinUnits::formatWithUnit(unit, balance));
    ui->labelStake->setText(BitcoinUnits::formatWithUnit(unit, stake));
    ui->labelUnconfirmed->setText(BitcoinUnits::formatWithUnit(unit, unconfirmedBalance));
    ui->labelReserveBalance->setText(BitcoinUnits::formatWithUnit(unit, reserveBalance));
    ui->labelImmature->setText(BitcoinUnits::formatWithUnit(unit, immatureBalance));

    //burn information
    currentNetBurnCoins = burnBalances.netBurnCoins;
    currentEffectiveBurnCoins = burnBalances.nEffectiveBurnCoins;
    currentImmatureBurnCoins = burnBalances.nImmatureBurnCoins;
    currentDecayedBurnCoins = burnBalances.nDecayedBurnCoins;

    ui->labelNetBurnCoins->setText(BitcoinUnits::formatWithUnit(unit, burnBalances.netBurnCoins));
    ui->labelEffBurnCoins->setText(BitcoinUnits::formatWithUnit(unit, burnBalances.nEffectiveBurnCoins));
    ui->labelImmBurnCoins->setText(BitcoinUnits::formatWithUnit(unit, burnBalances.nImmatureBurnCoins));
    ui->labelDecayBurnCoins->setText(BitcoinUnits::formatWithUnit(unit, burnBalances.nDecayedBurnCoins));

    ui->labelWatchAvailable->setText(BitcoinUnits::formatWithUnit(unit, watchOnlyBalance));
    ui->labelWatchPending->setText(BitcoinUnits::formatWithUnit(unit, watchUnconfBalance));
    ui->labelWatchImmature->setText(BitcoinUnits::formatWithUnit(unit, watchImmatureBalance));
    ui->labelWatchTotal->setText(BitcoinUnits::formatWithUnit(unit, watchOnlyBalance + watchUnconfBalance + watchImmatureBalance));


    bool showWatchOnlyImmature = watchImmatureBalance != 0;
    bool showWatchOnly = (watchOnlyBalance != 0 || watchUnconfBalance != 0 || showWatchOnlyImmature);

    // for symmetry reasons also show immature label when the watchonly one is shown
    // ui->labelImmature->setVisible(showImmature || showWatchOnlyImmature);
    // ui->labelImmatureText->setVisible(showImmature || showWatchOnlyImmature);
    ui->labelWatchonly->setVisible(showWatchOnly);              // show Watchonly label
    ui->lineWatchBalance->setVisible(showWatchOnly);            // show watchonly balance separator line
    ui->labelWatchAvailable->setVisible(showWatchOnly);         // show watchonly available balance
    ui->labelWatchImmature->setVisible(showWatchOnlyImmature);  // show watchonly immature balance
    ui->labelWatchPending->setVisible(showWatchOnly);           // show watchonly pending balance
    ui->labelWatchTotal->setVisible(showWatchOnly);             // show watchonly total balance

    // only show immature (newly mined) balance if it's non-zero, so as not to complicate things
    // for the non-mining users
    bool showImmature = immatureBalance != 0;
    ui->labelImmature->setVisible(showImmature);
    ui->labelImmatureText->setVisible(showImmature);
}

void OverviewPage::setNumTransactions(int count)
{
    ui->labelNumTransactions->setText(QLocale::system().toString(count));
}

void OverviewPage::setReserveBalance(qint64 nreserveBalance)
{
    int unit = model->getOptionsModel()->getDisplayUnit();
    ui->labelReserveBalance->setText(BitcoinUnits::formatWithUnit(unit, nreserveBalance));
}

void OverviewPage::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if(model)
    {
    }
};

void OverviewPage::setModel(WalletModel *model)
{
    this->model = model;
    if(model)
    {
        // Set up transaction list
        TransactionFilterProxy *filter = new TransactionFilterProxy();
        filter->setSourceModel(model->getTransactionTableModel());
        filter->setLimit(NUM_ITEMS);
        filter->setDynamicSortFilter(true);
        filter->setSortRole(Qt::EditRole);
        filter->sort(TransactionTableModel::Date, Qt::DescendingOrder);

        ui->listTransactions->setModel(filter);
        ui->listTransactions->setModelColumn(TransactionTableModel::ToAddress);

        // Keep up to date with wallet
        setBalance(model->getBalance(), model->getStake(), model->getUnconfirmedBalance(), model->getImmatureBalance(), model->getReserveBalance(), model->getBurnCoinBalances(), model->getWatchBalance(), model->getWatchUnconfirmedBalance(), model->getWatchImmatureBalance());
        connect(model, SIGNAL(balanceChanged(qint64, qint64, qint64, qint64, qint64, BurnCoinsBalances)), this, SLOT(setBalance(qint64, qint64, qint64, qint64, qint64, BurnCoinsBalances, qint64, qint64, qint64)));

        setNumTransactions(model->getNumTransactions());
        connect(model, SIGNAL(numTransactionsChanged(int)), this, SLOT(setNumTransactions(int)));

        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(displayUnitChanged()));

        setReserveBalance(model->getReserveBalance());
        connect(model->getOptionsModel(), SIGNAL(reserveBalanceChanged(qint64)), this, SLOT(reserveBalanceChanged()));
    }
}

void OverviewPage::displayUnitChanged()
{
    if(!model || !model->getOptionsModel())
        return;
    if(currentBalance != -1)
        setBalance(currentBalance, currentStake, currentUnconfirmedBalance, currentImmatureBalance, currentReserveBalance, BurnCoinsBalances(currentNetBurnCoins, currentEffectiveBurnCoins, currentImmatureBurnCoins, currentWatchOnlyBalance, currentWatchUnconfBalance, currentWatchImmatureBalance));
    txdelegate->unit = model->getOptionsModel()->getDisplayUnit();
    ui->listTransactions->update();
}

void OverviewPage::reserveBalanceChanged()
{
    if(!model || !model->getOptionsModel())
        return;
    if(currentBalance != -1)
        setBalance(currentBalance, currentStake, currentUnconfirmedBalance, currentImmatureBalance, currentReserveBalance, BurnCoinsBalances(currentNetBurnCoins, currentEffectiveBurnCoins, currentImmatureBurnCoins));

    /*
    int unit = model->getOptionsModel()->getDisplayUnit();
    qint64 nreservebalance = model->getOptionsModel()->getReserveBalance();
    ui->labelReserveBalance->setText(BitcoinUnits::formatWithUnit(unit, nreservebalance));
    ui->labelReserveBalance->update();
    */
}

void OverviewPage::updatePlot(int count)
{
    // Double Check to make sure we don't try to update the plot when it is disabled
    if (!GetBoolArg("-chart", true)) { return; }

    // if(fDebug) { printf("Plot: Getting Ready: pidnexBest: %p\n", pindexBest); }

    int numLookBack = 2000;
    double diffMax = 0;
    CBlockIndex* pindex = pindexBest;
    int height = nBestHeight;
    int xStart = std::max<int>(height-numLookBack, 0) + 1;
    int xEnd = height;

    // Start at the end and walk backwards
    int i = numLookBack-1;
    int x = xEnd;

    // This should be a noop if the size is already 2000
    vX.resize(numLookBack);
    vY.resize(numLookBack);

    /*
    if(fDebug) {
        if(height != pindex->nHeight) {
            printf("Plot: Warning: nBestHeight and pindexBest->nHeight don't match: %d:%d:\n", height, pindex->nHeight);
        }
    }

    if(fDebug) { printf("Plot: Reading blockchain\n"); }
    */
    CBlockIndex* itr = pindex;
    while(i >= 0 && itr != NULL)
    {
        // if(fDebug) { printf("Plot: Processing block: %d - pprev: %p\n", itr->nHeight, itr->pprev); }
        vX[i] = itr->nHeight;
        vY[i] = GetDifficulty(itr);
        diffMax = std::max<double>(diffMax, vY[i]);

        itr = itr->pprev;
        i--;
        x--;
    }

    // if(fDebug) { printf("Plot: Drawing plot\n"); }

    ui->diffplot->graph(0)->setData(vX, vY);

    // set axes ranges, so we see all data:
    ui->diffplot->xAxis->setRange((double)xStart, (double)xEnd);
    ui->diffplot->yAxis->setRange(0, diffMax+(diffMax/10));

    ui->diffplot->xAxis->setAutoSubTicks(false);
    ui->diffplot->yAxis->setAutoSubTicks(false);
    ui->diffplot->xAxis->setSubTickCount(0);
    ui->diffplot->yAxis->setSubTickCount(0);

    ui->diffplot->replot();

    // if(fDebug) { printf("Plot: Done!\n"); }
 }
 
