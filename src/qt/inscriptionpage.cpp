#include "inscriptionpage.h"
#include "ui_inscriptionpage.h"
#include "clientmodel.h"
#include "inscriptiontablemodel.h"
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

int getInscriptionBlockCount()
{
    /*
    QSqlQuery query;
    query.exec(QString("select blockindex from blockindex"));
    if (query.next())
    {
        return query.value(0).toInt();
    }
    */
    return 0;
}

void updateInscription(int index)
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
                    args.push_back("getinscription");
                    args.push_back(v.get_str());
                    Value value = tableRPC.execute(
                       args[0],
                       RPCConvertValues(
                       args[0],
                       std::vector<std::string>(args.begin() + 1, args.end())));
                    if (value.type() == obj_type)
                    {
                        Object reply = value.get_obj();
                        /*
                        QSqlQuery query;
                        query.exec(QString("insert into inscription values('%1','%2',%3)").arg(QString::fromStdString(find_value(reply, "title").get_str())).arg(QString::fromStdString(v.get_str())).arg(index));
                        */
                    }
                }
            }
        }
        /*
        QSqlQuery query;
        query.exec(QString("UPDATE blockindex set blockindex = %1").arg(index));
        */
    }catch (json_spirit::Object& objError)
    {
        throw std::runtime_error("updateInscription error");
    }
    catch(std::runtime_error& e) 
    {  
          throw std::runtime_error("updateInscription error");
    }
    catch (std::exception& e)
    {
        throw std::runtime_error("updateInscription error");
    }
}


class TDBExecutor: public QObject
{
    Q_OBJECT

public Q_SLOTS:
   void start();

Q_SIGNALS:
   void refreshInscriptionTable();
};

#include "inscriptionpage.moc"

void TDBExecutor::start()
{
    try{
        int inscriptionblockcount = getInscriptionBlockCount();
        while((inscriptionblockcount < nBestHeight))
        { 
            updateInscription(inscriptionblockcount);
            inscriptionblockcount++;
        } 
    }catch(std::runtime_error& e) 
    {  
          Q_EMIT refreshInscriptionTable();
    }
    Q_EMIT refreshInscriptionTable();
}

InscriptionPage::InscriptionPage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InscriptionPage),
    model(0)
{
    ui->setupUi(this);
    startExecutor();
}

InscriptionPage::~InscriptionPage()
{
    emit stopExecutor();
    delete ui;
}

void InscriptionPage::setModel(InscriptionTableModel *model)
{
    this->model = model;
    if(!model) {
        printf("No Inscriptiontable model.\n");
        return;
    }else{
        printf("Inscriptiontable initialised.\n");
    }

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setDynamicSortFilter(true);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->tableView->setModel(proxyModel);
    ui->tableView->sortByColumn(1, Qt::DescendingOrder);

    // Set column widths
    #if QT_VERSION < 0x050000
    ui->tableView->horizontalHeader()->setResizeMode(InscriptionTableModel::Inscription, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setResizeMode(InscriptionTableModel::Date, QHeaderView::ResizeToContents);
    #else
    ui->tableView->horizontalHeader()->setSectionResizeMode(InscriptionTableModel::Inscription, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(InscriptionTableModel::Date, QHeaderView::ResizeToContents);
    #endif
    connect(ui->tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(selectionChanged()));
    connect(ui->tableView, SIGNAL(doubleClicked(QModelIndex)), this, SIGNAL(doubleClicked(QModelIndex)));
    connect(ui->searchButton, SIGNAL(clicked()), this, SLOT(searchButtonClicked()));
}

void InscriptionPage::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if(model)
    {}
}

void InscriptionPage::startExecutor()
{
    QThread* thread = new QThread;
    TDBExecutor *executor = new TDBExecutor();
    executor->moveToThread(thread);

    connect(executor, SIGNAL(refreshInscriptionTable()), this, SLOT(refreshInscriptionTable()));
    connect(thread, SIGNAL(started()), executor, SLOT(start()));
    connect(this, SIGNAL(stopExecutor()), executor, SLOT(deleteLater()));
    connect(this, SIGNAL(stopExecutor()), thread, SLOT(quit()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    thread->start();
}

void InscriptionPage::refreshInscriptionTable()
{
    model->refreshInscriptionTable();
}

void InscriptionPage::showDetails()
{
    if(!ui->tableView->selectionModel())
        return;
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();
    if(!selection.isEmpty())
    {
        // QString html = selection.at(0).data(InscriptionTableModel::InscriptionRole).toString();
        TransactionDescDialog dlg(selection.at(0));
        dlg.exec();
    }
}

void InscriptionPage::selectionChanged()
{
    if(!ui->tableView->selectionModel())
        return;

    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();
    if(!selection.isEmpty())
    {
        QString html = selection.at(0).data(InscriptionTableModel::InscriptionRole).toString();
        // TransactionDescDialog dlg(selection.at(0));
        // dlg.exec();
    }
}

void InscriptionPage::searchButtonClicked()
{
    /* TODO: What are the semantics of searching through inscriptionhashes?
    */
}
