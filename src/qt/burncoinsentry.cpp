#include "burncoinsentry.h"
#include "ui_burncoinsentry.h"
#include "guiutil.h"
#include "bitcoinunits.h"
#include "addressbookpage.h"
#include "walletmodel.h"
#include "optionsmodel.h"
#include "addresstablemodel.h"

//for fTestNet and burn addresses
#include "util.h"
#include "main.h"

#include <QApplication>
#include <QClipboard>

BurnCoinsEntry::BurnCoinsEntry(QWidget *parent) :
  QFrame(parent),
  ui(new Ui::BurnCoinsEntry),
  model(0)
{
  ui->setupUi(this);

  setFocusPolicy(Qt::TabFocus);
  setFocusProxy(ui->payAmount);

}

BurnCoinsEntry::~BurnCoinsEntry()
{
  delete ui;
}

void BurnCoinsEntry::setModel(WalletModel *model)
{
  this->model = model;
  clear();
}

void BurnCoinsEntry::clear()
{
  ui->payAmount->clear();
  ui->payAmount->setFocus();
  if(model && model->getOptionsModel())
  {
    ui->payAmount->setDisplayUnit(model->getOptionsModel()->getDisplayUnit());
  }
}

bool BurnCoinsEntry::validate()
{
  // Check input validity
  bool retval = true;

  if(!ui->payAmount->validate())
    retval = false;
  else
  {
    if(ui->payAmount->value() <= 0)
    {
      // Cannot send 0 coins or less
      ui->payAmount->setValid(false);
      retval = false;
    }
  }

  return retval;
}

SendCoinsRecipient BurnCoinsEntry::getValue()
{
  SendCoinsRecipient rv;

  CBurnAddress address;   

  rv.address = address.ToString().c_str();
  rv.label = "";
  rv.amount = ui->payAmount->value();

  return rv;
}

void BurnCoinsEntry::setValue(const qint64 &value)
{
  ui->payAmount->setValue(value);
}

bool BurnCoinsEntry::isClear()
{
  return ui->payAmount->validate();
}

void BurnCoinsEntry::setFocus()
{
  ui->payAmount->setFocus();
}
