#ifndef WALLETMODEL_H
#define WALLETMODEL_H

#include <QObject>

#include "allocators.h" /* for SecureString */

class OptionsModel;
class AddressTableModel;
class TransactionTableModel;
class CWallet;

class BurnCoinsBalances
{
public:
  qint64 netBurnCoins;
  qint64 nEffectiveBurnCoins;
  qint64 nImmatureBurnCoins;
  qint64 nDecayedBurnCoins;

  BurnCoinsBalances()
  {
    netBurnCoins = nEffectiveBurnCoins = nImmatureBurnCoins = nDecayedBurnCoins = 0;
  }

  BurnCoinsBalances(qint64 netBurn, qint64 effBurn, qint64 immBurn)
  {
    netBurnCoins = netBurn;
    nEffectiveBurnCoins = effBurn;
    nImmatureBurnCoins = immBurn;
    nDecayedBurnCoins = netBurnCoins - nImmatureBurnCoins - nEffectiveBurnCoins;
  }

  bool operator==(const BurnCoinsBalances &other)
  {
    return netBurnCoins == other.netBurnCoins &&
    nEffectiveBurnCoins == other.nEffectiveBurnCoins &&
    nImmatureBurnCoins == other.nImmatureBurnCoins &&
    nDecayedBurnCoins == other.nDecayedBurnCoins;
  }

  bool operator!=(const BurnCoinsBalances &other)
  {
    return !(*this == other);
  }

};

class SendCoinsRecipient
{
public:
  QString address;
  QString label;
  qint64 amount;
};

/** Interface to Bitcoin wallet from Qt view code. */
class WalletModel : public QObject
{
  Q_OBJECT
public:
  explicit WalletModel(CWallet *wallet, OptionsModel *optionsModel, QObject *parent = 0);

  enum StatusCode // Returned by sendCoins
  {
    OK,
    InvalidAmount,
    InvalidAddress,
    AmountExceedsBalance,
    AmountWithFeeExceedsBalance,
    DuplicateAddress,
    TransactionCreationFailed, // Error returned when wallet is still locked
    TransactionCommitFailed,
    BadBurningCoins, //if the user sends coins to a burn address from the send coins dialog
    Aborted
  };

  enum EncryptionStatus
  {
    Unencrypted,  // !wallet->IsCrypted()
    Locked,       // wallet->IsCrypted() && wallet->IsLocked()
    Unlocked      // wallet->IsCrypted() && !wallet->IsLocked()
  };

  OptionsModel *getOptionsModel();
  AddressTableModel *getAddressTableModel();
  TransactionTableModel *getTransactionTableModel();

  qint64 getBalance() const;
  qint64 getStake() const;
  qint64 getUnconfirmedBalance() const;
  int getNumTransactions() const;
  EncryptionStatus getEncryptionStatus() const;
  BurnCoinsBalances getBurnCoinBalances() const;

  // Check address for validity
  bool validateAddress(const QString &address);

  // Return status record for SendCoins, contains error id + information
  struct SendCoinsReturn
  {
  SendCoinsReturn(StatusCode status,
                  qint64 fee=0,
                  QString hex=QString()):
    status(status), fee(fee), hex(hex) {}

    StatusCode status;
    qint64 fee; // is used in case status is "AmountWithFeeExceedsBalance"
    QString hex; // is filled with the transaction hash if status is "OK"
  };

  // Send coins to a list of recipients
  SendCoinsReturn sendCoins(const QList<SendCoinsRecipient> &recipients, bool fBurnTx);

  // Wallet encryption
  bool setWalletEncrypted(bool encrypted, const SecureString &passphrase);
  // Passphrase only needed when unlocking
  bool setWalletLocked(bool locked, const SecureString &passPhrase=SecureString());
  bool changePassphrase(const SecureString &oldPass, const SecureString &newPass);
  // Wallet backup
  bool backupWallet(const QString &filename);

  // RAI object for unlocking wallet, returned by requestUnlock()
  class UnlockContext
  {
  public:
    UnlockContext(WalletModel *wallet, bool valid, bool relock);
    ~UnlockContext();

    bool isValid() const { return valid; }

    // Copy operator and constructor transfer the context
    UnlockContext(const UnlockContext& obj) { CopyFrom(obj); }
    UnlockContext& operator=(const UnlockContext& rhs) { CopyFrom(rhs); return *this; }
  private:
    WalletModel *wallet;
    bool valid;
    mutable bool relock; // mutable, as it can be set to false by copying

    void CopyFrom(const UnlockContext& rhs);
  };

  UnlockContext requestUnlock();

private:
  CWallet *wallet;

  // Wallet has an options model for wallet-specific options
  // (transaction fee, for example)
  OptionsModel *optionsModel;

  AddressTableModel *addressTableModel;
  TransactionTableModel *transactionTableModel;

  // Cache some values to be able to detect changes
  qint64 cachedBalance;
  qint64 cachedUnconfirmedBalance;
  qint64 cachedNumTransactions;
  EncryptionStatus cachedEncryptionStatus;
  BurnCoinsBalances cachedBurnCoinsBalances;

signals:
  // Signal that balance in wallet changed
  void balanceChanged(qint64 balance, qint64 stake, qint64 unconfirmedBalance, 
                      BurnCoinsBalances cachedBurnCoinsBalances);

  // Number of transactions in wallet changed
  void numTransactionsChanged(int count);

  // Encryption status of wallet changed
  void encryptionStatusChanged(int status);

  // Signal emitted when wallet needs to be unlocked
  // It is valid behaviour for listeners to keep the wallet locked after this signal;
  // this means that the unlocking failed or was cancelled.
  void requireUnlock();

  // Asynchronous error notification
  void error(const QString &title, const QString &message, bool modal);

  public slots:
    void update();
    void updateAddressList();
};


#endif // WALLETMODEL_H
