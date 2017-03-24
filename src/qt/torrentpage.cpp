#include "torrentpage.h"
#include "ui_torrentpage.h"
#include "clientmodel.h"
#include "torrenttablemodel.h"
#include "bitcoingui.h"
#include "guiutil.h"
#include "util.h"
#include "main.h"
#include "transactiondescdialog.h"
#include "bitcoinrpc.h"
#include <algorithm>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/foreach.hpp>


#include "ui_interface.h"
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_utils.h"


#include <QSortFilterProxyModel>
#include <QClipboard>
#include <QMessageBox>
#include <QMenu>
#include <QPainter>

using namespace boost;
using namespace json_spirit;

int getTorrentBlockCount()
{
    QSqlQuery query;
    query.exec(QString("select blockindex from blockindex"));
    if (query.next())
    {
        return query.value(0).toInt();
    }
    return 0;
}

void updateTorrent(int index)
{
    try{
        QString s = QString::number(index, 10);  
        std::vector<std::string> args;
        args.push_back("getblockhash");
        args.push_back(s.toStdString());
        Value result = tableRPC.execute(
            args[0],
            RPCConvertValues(
               args[0],
               std::vector<std::string>(args.begin() + 1, args.end())));
        if (result.type() == str_type)
        {
            args.clear();
            args.push_back("getblock");
            args.push_back(result.get_str());
            result = tableRPC.execute(
                args[0],
                RPCConvertValues(
                    args[0],
                    std::vector<std::string>(args.begin() + 1, args.end())));
            if (result.type() == obj_type)
            {
                Object reply = result.get_obj();
                Array  txid  = find_value(reply, "tx").get_array();
                BOOST_FOREACH(Value v, txid)
                {

                    args.clear();
                    args.push_back("gettorrent");
                    args.push_back(v.get_str());
                    Value value = tableRPC.execute(
                       args[0],
                       RPCConvertValues(
                       args[0],
                       std::vector<std::string>(args.begin() + 1, args.end())));
                    if (value.type() == obj_type)
                    {
                        Object reply = value.get_obj();
                        QSqlQuery query;
                        query.exec(QString("insert into torrent values('%1','%2',%3)").arg(QString::fromStdString(find_value(reply, "title").get_str())).arg(QString::fromStdString(v.get_str())).arg(index));
                    }
                }
            }
        }

        QSqlQuery query;
        query.exec(QString("UPDATE blockindex set blockindex = %1").arg(index));
    }catch (json_spirit::Object& objError)
    {
        throw std::runtime_error("updateTorrent error");
    }
    catch(std::runtime_error& e) 
    {  
          throw std::runtime_error("updateTorrent error");
    }
    catch (std::exception& e)
    {
        throw std::runtime_error("updateTorrent error");
    }
}


class TDBExecutor: public QObject
{
    Q_OBJECT

public Q_SLOTS:
   void start();

Q_SIGNALS:
   void refreshTorrentTable();
};

#include "torrentpage.moc"

void TDBExecutor::start()
{
    try{
        int torrentblockcount = getTorrentBlockCount();
        while((torrentblockcount < nBestHeight))
        { 
            updateTorrent(torrentblockcount);
            torrentblockcount++;
        } 
    }catch(std::runtime_error& e) 
    {  
          Q_EMIT refreshTorrentTable();
    }
    Q_EMIT refreshTorrentTable();
}

TorrentPage::TorrentPage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TorrentPage),
    model(0)
{
    ui->setupUi(this);
    startExecutor();

}

TorrentPage::~TorrentPage()
{
    emit stopExecutor();
    delete ui;
}

void TorrentPage::setModel(TorrentTableModel *model)
{
    this->model = model;
    if(!model) {
        printf("No Torrenttable model.\n");
        return;
    }else{
        printf("Torrenttable initialised.\n");
    }

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setDynamicSortFilter(true);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->tableView->setModel(model);
    ui->tableView->sortByColumn(0, Qt::AscendingOrder);

    // Set column widths
    #if QT_VERSION < 0x050000
    ui->tableView->horizontalHeader()->setResizeMode(TorrentTableModel::Title, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setResizeMode(TorrentTableModel::Address, QHeaderView::ResizeToContents);
    #else
    ui->tableView->horizontalHeader()->setSectionResizeMode(TorrentTableModel::Title, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(TorrentTableModel::Address, QHeaderView::ResizeToContents);
    #endif
    connect(ui->tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(selectionChanged()));
    connect(ui->tableView, SIGNAL(doubleClicked(QModelIndex)), this, SIGNAL(doubleClicked(QModelIndex)));
    connect(ui->searchButton, SIGNAL(clicked()), this, SLOT(searchButtonClicked()));
}

void TorrentPage::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if(model)
    {}
}

void TorrentPage::startExecutor()
{
    QThread* thread = new QThread;
    TDBExecutor *executor = new TDBExecutor();
    executor->moveToThread(thread);

    connect(executor, SIGNAL(refreshTorrentTable()), this, SLOT(refreshTorrentTable()));
    connect(thread, SIGNAL(started()), executor, SLOT(start()));
    connect(this, SIGNAL(stopExecutor()), executor, SLOT(deleteLater()));
    connect(this, SIGNAL(stopExecutor()), thread, SLOT(quit()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    thread->start();
}

void TorrentPage::refreshTorrentTable()
{
    model->refreshTorrentTable();
}

void TorrentPage::showDetails()
{
    if(!ui->tableView->selectionModel())
        return;
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();
    if(!selection.isEmpty())
    {
        // QString html = selection.at(0).data(TorrentTableModel::TorrentRole).toString();
        TransactionDescDialog dlg(selection.at(0));
        dlg.exec();
    }
}

void TorrentPage::selectionChanged()
{
    if(!ui->tableView->selectionModel())
        return;

    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();
    if(!selection.isEmpty())
    {
        QString html = selection.at(0).data(TorrentTableModel::TorrentRole).toString();
        // TransactionDescDialog dlg(selection.at(0));
        // dlg.exec();
    }
}

void TorrentPage::searchButtonClicked()
{
    /* TODO: What are the semantics of searching through torrenthashes?
    */
}
