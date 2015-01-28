#include "burncoinsdialog.h"
#include "ui_burncoinsdialog.h"
#include "walletmodel.h"
#include "bitcoinunits.h"
#include "addressbookpage.h"
#include "optionsmodel.h"
#include "burncoinsentry.h"
#include "guiutil.h"
#include "askpassphrasedialog.h"
#include "util.h"
#include "main.h"

#include <QMessageBox>
#include <QLocale>
#include <QTextDocument>
#include <QScrollBar>

BurnCoinsDialog::BurnCoinsDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::BurnCoinsDialog),
  model(0)
{
  ui->setupUi(this);

#ifdef Q_WS_MAC // Icons on push buttons are very uncommon on Mac
  ui->clearButton->setIcon(QIcon());
  ui->sendButton->setIcon(QIcon());
#endif

  addEntry();

  connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clear()));

  fNewRecipientAllowed = true;
}

void BurnCoinsDialog::setModel(WalletModel *model)
{
  this->model = model;

  for(int i = 0; i < ui->entries->count(); ++i)
  {
    BurnCoinsEntry *entry = qobject_cast<BurnCoinsEntry*>(ui->entries->itemAt(i)->widget());
    if(entry)
    {
      entry->setModel(model);
    }
  }
  if(model)
  {
    setBalance(model->getBalance(), model->getStake(), model->getUnconfirmedBalance());
    connect(model, SIGNAL(balanceChanged(qint64, qint64, qint64, BurnCoinsBalances)),
            this, SLOT(setBalance(qint64, qint64, qint64)));
  }
}

BurnCoinsDialog::~BurnCoinsDialog()
{
  delete ui;
}

void BurnCoinsDialog::on_sendButton_clicked()
{
  QList<SendCoinsRecipient> recipients;
  bool valid = true;

  if(!model)
    return;

  for(int i = 0; i < ui->entries->count(); ++i)
  {
    BurnCoinsEntry *entry = qobject_cast<BurnCoinsEntry*>(ui->entries->itemAt(i)->widget());
    if(entry)
    {
      if(entry->validate())
        recipients.append(entry->getValue());
      else
        valid = false;
    }
  }

  if(!valid || recipients.isEmpty())
  {
    return;
  }

  // Format confirmation message
  QStringList formatted;
  foreach(const SendCoinsRecipient &rcp, recipients)
  {
    formatted.append(tr("<b>%1</b> to %2 burn address").arg(
                       BitcoinUnits::formatWithUnit(BitcoinUnits::BTC, rcp.amount), 
                       fTestNet ? "testnet" : ""));
  }

  fNewRecipientAllowed = false;

  QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Confirm burn coins"),
                                                             tr("Are you sure you want to burn %1?").arg(formatted.join(tr(" and "))), QMessageBox::Yes|QMessageBox::Cancel, QMessageBox::Cancel);

  if(retval != QMessageBox::Yes)
  {
    fNewRecipientAllowed = true;
    return;
  }

  WalletModel::UnlockContext ctx(model->requestUnlock());
  if(!ctx.isValid())
  {
    // Unlock wallet was cancelled
    fNewRecipientAllowed = true;
    return;
  }

  //sendCoins will automatically detect that this Tx is a burn Tx
  WalletModel::SendCoinsReturn sendstatus = model->sendCoins(recipients, true);
  switch(sendstatus.status)
  {
  case WalletModel::InvalidAddress:
    QMessageBox::warning(this, tr("Burn Coins"),
                         tr("The recepient address is not valid, please recheck."),
                         QMessageBox::Ok, QMessageBox::Ok);
    break;
  case WalletModel::InvalidAmount:
    QMessageBox::warning(this, tr("Burn Coins"),
                         tr("The amount to pay must be at least one cent (0.01)."),
                         QMessageBox::Ok, QMessageBox::Ok);
    break;
  case WalletModel::AmountExceedsBalance:
    QMessageBox::warning(this, tr("Burn Coins"),
                         tr("Amount exceeds your balance"),
                         QMessageBox::Ok, QMessageBox::Ok);
    break;
  case WalletModel::AmountWithFeeExceedsBalance:
    QMessageBox::warning(this, tr("Burn Coins"),
                         tr("Total exceeds your balance when the %1 transaction fee is included").
                         arg(BitcoinUnits::formatWithUnit(BitcoinUnits::BTC, sendstatus.fee)),
                         QMessageBox::Ok, QMessageBox::Ok);
    break;
  case WalletModel::DuplicateAddress:
    QMessageBox::warning(this, tr("Burn Coins"),
                         tr("Duplicate address found, can only burn to each address once in one burn operation"),
                         QMessageBox::Ok, QMessageBox::Ok);
    break;
  case WalletModel::TransactionCreationFailed:
    QMessageBox::warning(this, tr("Burn Coins"),
                         tr("Error: Transaction creation failed  "),
                         QMessageBox::Ok, QMessageBox::Ok);
    break;
  case WalletModel::TransactionCommitFailed:
    QMessageBox::warning(this, tr("Burn Coins"),
                         tr("Error: The transaction was rejected.  This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here."),
                         QMessageBox::Ok, QMessageBox::Ok);
    break;
  case WalletModel::BadBurningCoins: // there is no such thing as BadBurningCoins when burning coins
    break;
  case WalletModel::Aborted: // User aborted, nothing to do
    break;
  case WalletModel::OK:
    accept();
    break;
  }
  fNewRecipientAllowed = true;
}

void BurnCoinsDialog::clear()
{
  // Remove entries until only one left
  while(ui->entries->count())
  {
    delete ui->entries->takeAt(0)->widget();
  }
  addEntry();

  ui->sendButton->setDefault(true);
}

void BurnCoinsDialog::reject()
{
  clear();
}

void BurnCoinsDialog::accept()
{
  clear();
}

BurnCoinsEntry *BurnCoinsDialog::addEntry()
{
  //there can only be one entry
  if(ui->entries->count() > 0)
    return 0;

  BurnCoinsEntry *entry = new BurnCoinsEntry(this);
  entry->setModel(model);
  ui->entries->addWidget(entry);

  // Focus the field, so that entry can start immediately
  entry->clear();
  entry->setFocus();
  ui->scrollAreaWidgetContents->resize(ui->scrollAreaWidgetContents->sizeHint());
  QCoreApplication::instance()->processEvents();
  QScrollBar* bar = ui->scrollArea->verticalScrollBar();
  if(bar)
    bar->setSliderPosition(bar->maximum());
  return entry;
}

void BurnCoinsDialog::setBalance(qint64 balance, qint64 stake, qint64 unconfirmedBalance)
{
  Q_UNUSED(stake);
  Q_UNUSED(unconfirmedBalance);
  if(!model || !model->getOptionsModel())
    return;

  int unit = model->getOptionsModel()->getDisplayUnit();
  ui->labelBalance->setText(BitcoinUnits::formatWithUnit(unit, balance));
}
