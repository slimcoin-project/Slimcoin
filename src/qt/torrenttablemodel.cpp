#include "torrenttablemodel.h"
#include "guiutil.h"
#include "bitcoinrpc.h"
#include "base58.h"
#include "walletmodel.h"
#include "util.h"


#include <QFont>
#include <QColor>




#include "ui_interface.h"
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_utils.h"

#include <boost/algorithm/string.hpp>


#include <stdlib.h>

using namespace boost;
using namespace json_spirit;

struct TorrentTableEntry
{

    QString title;
    QString link;
    QString address;
    QString amount;

    TorrentTableEntry() {}
    TorrentTableEntry(const QString &title, const QString &link, const QString &address, const QString &amount):
        title(title),link(link),address(address),amount(amount) {}
};

class TorrentTablePriv
{
public:
    TorrentTablePriv(CWallet *wallet, TorrentTableModel *parent):
        wallet(wallet),
        parent(parent)
        {
        }
    CWallet *wallet;
    TorrentTableModel *parent;
    QList<TorrentTableEntry> cachedTorrentTable;

    void getLastTorrent()
    {
        try {
            QSqlQuery query;
            query.exec(QString("select txid from torrent  order by blockindex desc limit 1000"));
            while (query.next())
            {
                std::vector<std::string> args;
                args.push_back("gettorrent");
                args.push_back(query.value(0).toString().toStdString());
                Value value = tableRPC.execute(
                    args[0],
                    RPCConvertValues(
                        args[0],
                        std::vector<std::string>(args.begin() + 1, args.end())));
                if (value.type() == obj_type)
                {
                    Object reply = value.get_obj();

                    std::string title = find_value(reply, "title").get_str();
                    std::string link = find_value(reply, "link").get_str();
                    std::string address = find_value(reply, "address").get_str();
                    std::string amount = find_value(reply, "amount").get_str();

                    cachedTorrentTable.append(
                        TorrentTableEntry(
                            QString::fromStdString(title),
                            QString::fromStdString(link),
                            QString::fromStdString(address),
                            QString::fromStdString(amount)));
                }

            }
        }catch (json_spirit::Object& objError)
        {
            printf("Spirit error");
        }
        catch(std::runtime_error &) 
        {  
            printf("Runtime error");
        }
        catch (std::exception& e)
        {
            printf("General exception");
        }
    }

    void refreshTable()
    {
        cachedTorrentTable.clear();
        getLastTorrent();
    }

    int size()
    {
        return cachedTorrentTable.size();
    }

    TorrentTableEntry *index(int idx)
    {
        if(idx >= 0 && idx < cachedTorrentTable.size())
        {
            return &cachedTorrentTable[idx];
        }
        else
        {
            return 0;
        }
    }

    QString describe(TorrentTableEntry *rec)
    {

        QString strHTML;

        strHTML.reserve(4000);
        strHTML += "<html><font face='verdana, arial, helvetica, sans-serif'>";
        strHTML += "<b>Title:</b> " + rec->title + "<br>";
        strHTML += "<b>Send Address:</b> " + rec->address + "<br>";
        strHTML += "<b>Amount:</b> " + rec->amount + "<br>";
        strHTML += "<b>Link:</b> " + rec->link + "<br>";
        return strHTML;


    }
};


TorrentTableModel::TorrentTableModel(CWallet *wallet, WalletModel *parent) :
    QAbstractTableModel(parent),
    wallet(wallet),
    priv(0)
{
    columns << tr("Title") ;
    columns << tr("Address") ;
    columns << tr("Amount") ;
    priv = new TorrentTablePriv(wallet, this);
}

TorrentTableModel::~TorrentTableModel()
{
    delete priv;
}

int TorrentTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return priv->size();
}

int TorrentTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return columns.length();
}

void TorrentTableModel::refreshTorrentTable()
{
    if (this) {
        printf("Refreshing torrent table");
        priv->refreshTable();
        Q_EMIT layoutChanged();
    } else {
        printf("Torrenttable uninitialised.");
    }
}

QVariant TorrentTableModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    TorrentTableEntry *rec = static_cast<TorrentTableEntry*>(index.internalPointer());

    if(role == Qt::DisplayRole)
    {
        switch(index.column())
        {
        case Title:
            return rec->title;
        case Address:
            return rec->address;
        case Amount:
            return rec->amount;
        }
    }
    else if(role == TorrentRole)
    {
        return priv->describe(rec);
    }
        
    return QVariant();
}

QVariant TorrentTableModel::headerData(int section, Qt::Orientation orientation, int role) const
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

QModelIndex TorrentTableModel::index(int row, int column, const QModelIndex & parent) const
{
   Q_UNUSED(parent);
    TorrentTableEntry *data = priv->index(row);
    if(data)
    {
        return createIndex(row, column, priv->index(row));
    }
    else
    {
        return QModelIndex();
    }
}
