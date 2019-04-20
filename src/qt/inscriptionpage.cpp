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
    return 0;
}


InscriptionPage::InscriptionPage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InscriptionPage),
    model(0)
{
    ui->setupUi(this);
}

InscriptionPage::~InscriptionPage()
{
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
    ui->tableView->resizeColumnsToContents();
    connect(ui->tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(selectionChanged()));
    connect(ui->tableView, SIGNAL(doubleClicked(QModelIndex)), this, SIGNAL(doubleClicked(QModelIndex)));
    connect(ui->searchButton, SIGNAL(clicked()), this, SLOT(searchButtonClicked()));
}

void InscriptionPage::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if(!model) {
        printf("No client model.\n");
        return;
    }else{
        printf("Clientmodel initialised.\n");
    }
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
