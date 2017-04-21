#ifndef INSCRIPTIONTABLEMODEL_H
#define INSCRIPTIONTABLEMODEL_H

#include <QAbstractTableModel>
#include <QStringList>

class CWallet;
class InscriptionTablePriv;
class WalletModel;

/**
   Qt model of the inscription. This allows views to access
   and modify the inscription records.
 */

class InscriptionTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit InscriptionTableModel(CWallet* wallet, WalletModel *parent = 0);
    ~InscriptionTableModel();

    enum ColumnIndex {
        Inscription = 0,
        Date
    };

    enum RoleIndex {
        /** Type of transaction */
        TypeRole = Qt::UserRole,
        /** Date and time this transaction was created */
        InscriptionRole
    };

    /** @name Methods overridden from QAbstractTableModel
        @{*/
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QModelIndex index(int row, int column, const QModelIndex & parent) const;
    void getLastInscription();
    void getInscriptions();

private:
    CWallet* wallet;
    WalletModel *walletModel;
    InscriptionTablePriv *priv;
    QStringList columns;

public Q_SLOTS:
    friend class InscriptionTablePriv;
    void refreshInscriptionTable();
};

#endif // INSCRIPTIONTABLEMODEL_H
