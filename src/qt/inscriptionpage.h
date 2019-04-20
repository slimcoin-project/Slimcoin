#ifndef INSCRIPTIONPAGE_H
#define INSCRIPTIONPAGE_H

#include <QDialog>
#include <QThread>

namespace Ui {
    class InscriptionPage;
}
class InscriptionTableModel;
class ClientModel;

QT_BEGIN_NAMESPACE
class QTableView;
class QItemSelection;
class QSortFilterProxyModel;
class QMenu;
class QModelIndex;
QT_END_NAMESPACE

/** Widget that shows a list of inscriptions.
  */
class InscriptionPage : public QDialog
{
    Q_OBJECT

public:

    explicit InscriptionPage(QWidget *parent = 0);
    ~InscriptionPage();

    void setModel(InscriptionTableModel *model);
    void setClientModel(ClientModel *model);

    const QString &getReturnValue() const { return returnValue; }

public Q_SLOTS:
    void refreshInscriptionTable();
private:
    Ui::InscriptionPage *ui;
    InscriptionTableModel *model;
    ClientModel *clientModel;
    QString returnValue;
    QSortFilterProxyModel *proxyModel;
    QMenu *contextMenu;
    QAction *deleteAction;
    QString newAddressToSelect;

Q_SIGNALS:
    void doubleClicked(const QModelIndex&);
    void updateRequest();

private slots:
    void searchButtonClicked();
    void showDetails();
    void selectionChanged();
private:
    

};

#endif // INSCRIPTIONPAGE_H
