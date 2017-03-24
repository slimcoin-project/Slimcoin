#ifndef TORRENTTABLEMODEL_H
#define TORRENTTABLEMODEL_H

#include <QAbstractTableModel>
#include <QStringList>
#include <QtSql>

class CWallet;
class TorrentTablePriv;
class WalletModel;

/**
   Qt model of the torrent. This allows views to access
   and modify the torrent records.
 */

class TorrentTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit TorrentTableModel(CWallet* wallet, WalletModel *parent = 0);
    ~TorrentTableModel();

    enum ColumnIndex {
        Title = 0,
        Address,
        Amount
    };

    enum RoleIndex {
        /** Type of transaction */
        TypeRole = Qt::UserRole,
        /** Date and time this transaction was created */
        TorrentRole
    };

    /** @name Methods overridden from QAbstractTableModel
        @{*/
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QModelIndex index(int row, int column, const QModelIndex & parent) const;
    void refreshTorrentTable();

private:
    CWallet* wallet;
    WalletModel *walletModel;
    TorrentTablePriv *priv;
    QStringList columns;
    QSqlDatabase db;

public Q_SLOTS:
    friend class TorrentTablePriv;
};

#endif // TORRENTTABLEMODEL_H
