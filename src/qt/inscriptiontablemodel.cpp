#include "inscriptiontablemodel.h"
#include "guiutil.h"
#include "bitcoinrpc.h"
#include "base58.h"
#include "walletmodel.h"
#include "util.h"
#include "ui_interface.h"

#include <QFont>
#include <QColor>
#include <QDateTime>

/*
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_utils.h"
*/

#include <boost/algorithm/string.hpp>

#include <stdlib.h>

using namespace boost;
// using namespace json_spirit;

struct InscriptionTableEntry
{

    QString inscription;
    QDateTime date;

    InscriptionTableEntry() {}
    InscriptionTableEntry(const QString &inscription, const QDateTime &date):
        inscription(inscription),date(date) {}
};

class InscriptionTablePriv
{
public:
    InscriptionTablePriv(CWallet *wallet, InscriptionTableModel *parent):
        wallet(wallet),
        parent(parent)
        {
        }
    CWallet *wallet;
    InscriptionTableModel *parent;
    QList<InscriptionTableEntry> cachedInscriptionTable;

    void getLastInscription()
    {
        std::vector<std::pair<std::string, int> > vTxResults;
        this->wallet->GetTxMessages(vTxResults);

        QDateTime inscription_date = QDateTime::currentDateTime();

        std::string inscription = vTxResults[0].first;
#if QT_VERSION < 0x058000
        inscription_date.setMSecsSinceEpoch((qint64)vTxResults[0].second*1000);
#else
        inscription_date.setSecsSinceEpoch((qint64)vTxResults[0].second);
#endif
        cachedInscriptionTable.append(
            InscriptionTableEntry(
                QString::fromStdString(inscription),
                inscription_date
                ));
    }

    void getAllInscriptions()
    {
        std::vector<std::pair<std::string, int> > vTxResults;
        this->wallet->GetTxMessages(vTxResults);
        QDateTime inscription_date = QDateTime::currentDateTime();

        for(std::vector<std::pair<std::string, int> >::size_type i = 0; i != vTxResults.size(); i++) {
            std::string inscription = vTxResults[i].first;
#if QT_VERSION < 0x058000
            inscription_date.setMSecsSinceEpoch((qint64)vTxResults[i].second*1000);
#else
            inscription_date.setSecsSinceEpoch((qint64)vTxResults[i].second);
#endif
            cachedInscriptionTable.append(
                InscriptionTableEntry(
                    QString::fromStdString(inscription),
                    inscription_date
            ));
        }
    }

    void getInscriptions()
    {
        std::vector<std::pair<std::string, int> > vTxResults;
        this->wallet->GetMyTxMessages(vTxResults);
        QDateTime inscription_date = QDateTime::currentDateTime();

        for(std::vector<std::pair<std::string, int> >::size_type i = 0; i != vTxResults.size(); i++) {
            std::string inscription = vTxResults[i].first;
#if QT_VERSION < 0x058000
            inscription_date.setMSecsSinceEpoch((qint64)vTxResults[i].second*1000);
#else
            inscription_date.setSecsSinceEpoch((qint64)vTxResults[i].second);
#endif
            cachedInscriptionTable.append(
                InscriptionTableEntry(
                    QString::fromStdString(inscription),
                    inscription_date
                ));
        }
    }

    void refreshTable()
    {
        cachedInscriptionTable.clear();
        getInscriptions();
    }

    int size()
    {
        return cachedInscriptionTable.size();
    }

    InscriptionTableEntry *index(int idx)
    {
        if(idx >= 0 && idx < cachedInscriptionTable.size())
        {
            return &cachedInscriptionTable[idx];
        }
        else
        {
            return 0;
        }
    }

    QString describe(InscriptionTableEntry *rec)
    {

        QString strHTML;

        strHTML.reserve(4000);
        strHTML += "<html><font face='verdana, arial, helvetica, sans-serif'>";
        strHTML += "<b>Inscription content:</b> " + rec->inscription + "<br>";
        strHTML += "<b>Inscription date:</b> " + rec->date.toString() + "<br>";
        return strHTML;


    }
};


InscriptionTableModel::InscriptionTableModel(CWallet *wallet, WalletModel *parent) :
    QAbstractTableModel(parent),
    wallet(wallet),
    priv(0)
{
    columns << tr("Inscription content      ") ;
    columns << tr("Inscription date         ") ;
    priv = new InscriptionTablePriv(wallet, this);
    priv->refreshTable();
}

InscriptionTableModel::~InscriptionTableModel()
{
    delete priv;
}

int InscriptionTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return priv->size();
}

int InscriptionTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return columns.length();
}

void InscriptionTableModel::refreshInscriptionTable()
{
    /* FIXME: Use explicit check for NULL */
    if (this) {
        printf("Refreshing inscription table");
        priv->refreshTable();
        Q_EMIT layoutChanged();
    } else {
        printf("Inscriptiontable uninitialised.");
    }
}

QVariant InscriptionTableModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    InscriptionTableEntry *rec = static_cast<InscriptionTableEntry*>(index.internalPointer());

    if(role == Qt::DisplayRole)
    {
        switch(index.column())
        {
        case Inscription:
            return rec->inscription;
        case Date:
            return GUIUtil::dateTimeStr(rec->date);
        }
    }
    else if(role == InscriptionRole)
    {
        return priv->describe(rec);
    }
        
    return QVariant();
}

QVariant InscriptionTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal)
    {
        if(role == Qt::DisplayRole)
        {
            return columns[section];
        }
    }
    return QVariant();
}

QModelIndex InscriptionTableModel::index(int row, int column, const QModelIndex & parent) const
{
   Q_UNUSED(parent);
    InscriptionTableEntry *data = priv->index(row);
    if(data)
    {
        return createIndex(row, column, priv->index(row));
    }
    else
    {
        return QModelIndex();
    }
}
