#include "inscriptiontablemodel.h"
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

struct InscriptionTableEntry
{

    QString inscription;
    QString date;

    InscriptionTableEntry() {}
    InscriptionTableEntry(const QString &inscription, const QString &date):
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

        std::string inscription = "<inscription>";
        std::string date = "<date>";

        cachedInscriptionTable.append(
            InscriptionTableEntry(
                QString::fromStdString(inscription),
                QString::fromStdString(date)));

        /*
        try {
            QSqlQuery query;
            query.exec(QString("select txid from inscription  order by blockindex desc limit 1000"));
            while (query.next())
            {
                std::vector<std::string> args;
                args.push_back("getinscription");
                args.push_back(query.value(0).toString().toStdString());
                Value value = tableRPC.execute(
                    args[0],
                    RPCConvertValues(
                        args[0],
                        std::vector<std::string>(args.begin() + 1, args.end())));
                if (value.type() == obj_type)
                {
                    Object reply = value.get_obj();

                    std::string inscription = find_value(reply, "inscription").get_str();
                    std::string date = find_value(reply, "date").get_str();

                    cachedInscriptionTable.append(
                        InscriptionTableEntry(
                            QString::fromStdString(inscription),
                            QString::fromStdString(date)));
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
        */
    }

    void getInscriptions()
    {
        std::vector<std::pair<std::string, int> > vTxResults;
        this->wallet->GetTxMessages(vTxResults);

        std::string inscription = "<inscription>";
        std::string date = "<date>";

        cachedInscriptionTable.append(
            InscriptionTableEntry(
                QString::fromStdString(inscription),
                QString::fromStdString(date)));

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
        strHTML += "<b>Inscription date:</b> " + rec->date + "<br>";
        return strHTML;


    }
};


InscriptionTableModel::InscriptionTableModel(CWallet *wallet, WalletModel *parent) :
    QAbstractTableModel(parent),
    wallet(wallet),
    priv(0)
{
    columns << tr("Inscription content") ;
    columns << tr("Inscription date") ;
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
            return rec->date;
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
