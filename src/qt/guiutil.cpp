#include "guiutil.h"
#include "bitcoinaddressvalidator.h"
#include "walletmodel.h"
#include "bitcoinunits.h"

#include "util.h"
#include "init.h"

#include <QString>
#include <QDateTime>
#include <QDoubleValidator>
#include <QFont>
#include <QLineEdit>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#else
#include <QUrl>
#endif
#include <QTextDocument> // for Qt::mightBeRichText
#include <QAbstractItemView>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QDesktopServices>
#include <QThread>
#include <QTableView>

namespace GUIUtil
{

  QString dateTimeStr(const QDateTime &date)
  {
    return date.date().toString(Qt::SystemLocaleShortDate) + QString(" ") + date.toString("hh:mm");
  }

  QString dateTimeStr(qint64 nTime)
  {
    return dateTimeStr(QDateTime::fromTime_t((qint32)nTime));
  }

  QFont bitcoinAddressFont()
  {
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    return font;
  }

  void setupAddressWidget(QLineEdit *widget, QWidget *parent)
  {
    widget->setMaxLength(BitcoinAddressValidator::MaxAddressLength);
    widget->setValidator(new BitcoinAddressValidator(parent));
    widget->setFont(bitcoinAddressFont());
  }

  void setupAmountWidget(QLineEdit *widget, QWidget *parent)
  {
    QDoubleValidator *amountValidator = new QDoubleValidator(parent);
    amountValidator->setDecimals(8);
    amountValidator->setBottom(0.0);
    widget->setValidator(amountValidator);
    widget->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  }

  bool parseBitcoinURI(const QUrl &uri, SendCoinsRecipient *out)
  {
    if(uri.scheme() != QString("slimcoin"))
      return false;

    // check if the address is valid
    CBitcoinAddress addressFromUri(uri.path().toStdString());
    if (!addressFromUri.IsValid())
        return false;

    SendCoinsRecipient rv;
    rv.address = uri.path();
    rv.amount = 0;
   
#if QT_VERSION < 0x050000
    QList<QPair<QString, QString> > items = uri.queryItems();
#else
    QUrlQuery uriQuery(uri);
    QList<QPair<QString, QString> > items = uriQuery.queryItems();
#endif
    for(QList<QPair<QString, QString> >::iterator i = items.begin(); i != items.end(); i++)
    {
      bool fShouldReturnFalse = false;
      if(i->first.startsWith("req-"))
      {
        i->first.remove(0, 4);
        fShouldReturnFalse = true;
      }

      if(i->first == "label")
      {
        rv.label = i->second;
        fShouldReturnFalse = false;
      }
      else if(i->first == "amount")
      {
        if(!i->second.isEmpty())
        {
          if(!BitcoinUnits::parse(BitcoinUnits::BTC, i->second, &rv.amount))
          {
            return false;
          }
        }
        fShouldReturnFalse = false;
      }

      if(fShouldReturnFalse)
        return false;
    }
    if(out)
    {
      *out = rv;
    }
    return true;
  }

  bool parseBitcoinURI(QString uri, SendCoinsRecipient *out)
  {
    // Convert bitcoin:// to bitcoin:
    //
    //    Cannot handle this later, because bitcoin:// will cause Qt to see the part after // as host,
    //    which will lowercase it (and thus invalidate the address).
    if(uri.startsWith("slimcoin://"))
    {
      uri.replace(0, std::string("slimcoin://").length(), "slimcoin:");
    }
    QUrl uriInstance(uri);
    return parseBitcoinURI(uriInstance, out);
  }

  QString HtmlEscape(const QString& str, bool fMultiLine)
  {
#if QT_VERSION < 0x050000
    QString escaped = Qt::escape(str);
#else
    QString escaped = str.toHtmlEscaped();
#endif
    if(fMultiLine)
    {
      escaped = escaped.replace("\n", "<br>\n");
    }
    return escaped;
  }

  QString HtmlEscape(const std::string& str, bool fMultiLine)
  {
    return HtmlEscape(QString::fromStdString(str), fMultiLine);
  }

  void copyEntryData(QAbstractItemView *view, int column, int role)
  {
    if(!view || !view->selectionModel())
      return;
    QModelIndexList selection = view->selectionModel()->selectedRows(column);

    if(!selection.isEmpty())
    {
      // Copy first item
      QApplication::clipboard()->setText(selection.at(0).data(role).toString());
    }
  }

  QString getSaveFileName(QWidget *parent, const QString &caption,
                          const QString &dir,
                          const QString &filter,
                          QString *selectedSuffixOut)
  {
    QString selectedFilter;
    QString myDir;
    if(dir.isEmpty()) // Default to user documents location
    {
#if QT_VERSION < 0x050000
        myDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
#else
    QString myDir = GetDataDir().string().c_str();
#endif
    }
    else
    {
      myDir = dir;
    }
    QString result = QFileDialog::getSaveFileName(parent, caption, myDir, filter, &selectedFilter);

    /* Extract first suffix from filter pattern "Description (*.foo)" or "Description (*.foo *.bar ...) */
    QRegExp filter_re(".* \\(\\*\\.(.*)[ \\)]");
    QString selectedSuffix;
    if(filter_re.exactMatch(selectedFilter))
    {
      selectedSuffix = filter_re.cap(1);
    }

    /* Add suffix if needed */
    QFileInfo info(result);
    if(!result.isEmpty())
    {
      if(info.suffix().isEmpty() && !selectedSuffix.isEmpty())
      {
        /* No suffix specified, add selected suffix */
        if(!result.endsWith("."))
          result.append(".");
        result.append(selectedSuffix);
      }
    }

    /* Return selected suffix if asked to */
    if(selectedSuffixOut)
    {
      *selectedSuffixOut = selectedSuffix;
    }
    return result;
  }

  Qt::ConnectionType blockingGUIThreadConnection()
  {
    if(QThread::currentThread() != QCoreApplication::instance()->thread())
    {
      return Qt::BlockingQueuedConnection;
    }
    else
    {
      return Qt::DirectConnection;
    }
  }

  bool checkPoint(const QPoint &p, const QWidget *w)
  {
    QWidget *atW = qApp->widgetAt(w->mapToGlobal(p));
    if(!atW) return false;
    return atW->topLevelWidget() == w;
  }

  bool isObscured(QWidget *w)
  {

    return !(checkPoint(QPoint(0, 0), w)
             && checkPoint(QPoint(w->width() - 1, 0), w)
             && checkPoint(QPoint(0, w->height() - 1), w)
             && checkPoint(QPoint(w->width() - 1, w->height() - 1), w)
             && checkPoint(QPoint(w->width()/2, w->height()/2), w));
  }

  void openDebugLogfile()
  {

    boost::filesystem::path pathDebug = GetDataDir() / "debug.log";

    /* Open debug.log with the associated application */
    if(boost::filesystem::exists(pathDebug))
      QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(pathDebug.string())));
  }

ToolTipToRichTextFilter::ToolTipToRichTextFilter(int size_threshold, QObject *parent) :
    QObject(parent), size_threshold(size_threshold)
{

}

bool ToolTipToRichTextFilter::eventFilter(QObject *obj, QEvent *evt)
{
    if(evt->type() == QEvent::ToolTipChange)
    {
        QWidget *widget = static_cast<QWidget*>(obj);
        QString tooltip = widget->toolTip();
        if(tooltip.size() > size_threshold && !tooltip.startsWith("<qt/>") && !Qt::mightBeRichText(tooltip))
        {
            // Prefix <qt/> to make sure Qt detects this as rich text
            // Escape the current message as HTML and replace \n by <br>
            tooltip = "<qt/>" + HtmlEscape(tooltip, true);
            widget->setToolTip(tooltip);
            return true;
        }
    }
    return QObject::eventFilter(obj, evt);
}

  HelpMessageBox::HelpMessageBox(QWidget *parent) :
    QMessageBox(parent)
  {
    header = tr("Slimcoin-Qt") + " " + tr("version") + " " +
      QString::fromStdString(FormatFullVersion()) + "\n\n" +
      tr("Usage:") + "\n" +
      "  slimcoin-qt [" + tr("command-line options") + "]                     " + "\n";

    coreOptions = QString::fromStdString(HelpMessage());

    uiOptions = tr("UI options") + ":\n" +
      "  -lang=<lang>           " + tr("Set language, for example \"de_DE\" (default: system locale)") + "\n" +
      "  -min                   " + tr("Start minimized") + "\n" +
      "  -splash                " + tr("Show splash screen on startup (default: 1)") + "\n";

    setWindowTitle(tr("Slimcoin-Qt"));
    setTextFormat(Qt::PlainText);
    // setMinimumWidth is ignored for QMessageBox so put in non-breaking spaces to make it wider.
    setText(header + QString(QChar(0x2003)).repeated(50));
    setDetailedText(coreOptions + "\n" + uiOptions);
  }

  void HelpMessageBox::printToConsole()
  {
    // On other operating systems, the expected action is to print the message to the console.
    QString strUsage = header + "\n" + coreOptions + "\n" + uiOptions;
    fprintf(stdout, "%s", strUsage.toStdString().c_str());
  }

  void HelpMessageBox::showOrPrint()
  {
#if defined(WIN32)
    // On Windows, show a message box, as there is no stderr/stdout in windowed applications
    exec();
#else
    // On other operating systems, print help text to console
    printToConsole();
#endif
  }

void TableViewLastColumnResizingFixer::connectViewHeadersSignals()
{
    connect(tableView->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(on_sectionResized(int, int, int)));
    connect(tableView->horizontalHeader(), SIGNAL(geometriesChanged()), this, SLOT(on_geometriesChanged()));
}

// We need to disconnect these while handling the resize events, otherwise we can enter infinite loops.
void TableViewLastColumnResizingFixer::disconnectViewHeadersSignals()
{
    disconnect(tableView->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(on_sectionResized(int, int, int)));
    disconnect(tableView->horizontalHeader(), SIGNAL(geometriesChanged()), this, SLOT(on_geometriesChanged()));
}

// Setup the resize mode, handles compatibility for Qt5 and below as the method signatures changed.
// Refactored here for readability.
void TableViewLastColumnResizingFixer::setViewHeaderResizeMode(int logicalIndex, QHeaderView::ResizeMode resizeMode)
{
#if QT_VERSION < 0x050000
    tableView->horizontalHeader()->setResizeMode(logicalIndex, resizeMode);
#else
    tableView->horizontalHeader()->setSectionResizeMode(logicalIndex, resizeMode);
#endif
}

void TableViewLastColumnResizingFixer::resizeColumn(int nColumnIndex, int width)
{
    tableView->setColumnWidth(nColumnIndex, width);
    tableView->horizontalHeader()->resizeSection(nColumnIndex, width);
}

int TableViewLastColumnResizingFixer::getColumnsWidth()
{
    int nColumnsWidthSum = 0;
    for (int i = 0; i < columnCount; i++) {
        nColumnsWidthSum += tableView->horizontalHeader()->sectionSize(i);
    }
    return nColumnsWidthSum;
}

int TableViewLastColumnResizingFixer::getAvailableWidthForColumn(int column)
{
    int nResult = lastColumnMinimumWidth;
    int nTableWidth = tableView->horizontalHeader()->width();

    if (nTableWidth > 0) {
        int nOtherColsWidth = getColumnsWidth() - tableView->horizontalHeader()->sectionSize(column);
        nResult = std::max(nResult, nTableWidth - nOtherColsWidth);
    }

    return nResult;
}

// Make sure we don't make the columns wider than the tables viewport width.
void TableViewLastColumnResizingFixer::adjustTableColumnsWidth()
{
    disconnectViewHeadersSignals();
    resizeColumn(lastColumnIndex, getAvailableWidthForColumn(lastColumnIndex));
    connectViewHeadersSignals();

    int nTableWidth = tableView->horizontalHeader()->width();
    int nColsWidth = getColumnsWidth();
    if (nColsWidth > nTableWidth) {
        resizeColumn(secondToLastColumnIndex, getAvailableWidthForColumn(secondToLastColumnIndex));
    }
}

// Make column use all the space available, useful during window resizing.
void TableViewLastColumnResizingFixer::stretchColumnWidth(int column)
{
    disconnectViewHeadersSignals();
    resizeColumn(column, getAvailableWidthForColumn(column));
    connectViewHeadersSignals();
}

// When a section is resized this is a slot-proxy for ajustAmountColumnWidth().
void TableViewLastColumnResizingFixer::on_sectionResized(int logicalIndex, int oldSize, int newSize)
{
    adjustTableColumnsWidth();
    int remainingWidth = getAvailableWidthForColumn(logicalIndex);
    if (newSize > remainingWidth) {
        resizeColumn(logicalIndex, remainingWidth);
    }
}

// When the tabless geometry is ready, we manually perform the stretch of the "Message" column,
// as the "Stretch" resize mode does not allow for interactive resizing.
void TableViewLastColumnResizingFixer::on_geometriesChanged()
{
    if ((getColumnsWidth() - this->tableView->horizontalHeader()->width()) != 0) {
        disconnectViewHeadersSignals();
        resizeColumn(secondToLastColumnIndex, getAvailableWidthForColumn(secondToLastColumnIndex));
        connectViewHeadersSignals();
    }
}

/**
 * Initializes all internal variables and prepares the
 * the resize modes of the last 2 columns of the table and
 */
    TableViewLastColumnResizingFixer::TableViewLastColumnResizingFixer(QTableView* table, int lastColMinimumWidth, int allColsMinimumWidth, QObject *parent, int columnStretch) :
            QObject(parent),
            tableView(table),
            lastColumnMinimumWidth(lastColMinimumWidth),
            allColumnsMinimumWidth(allColsMinimumWidth)
    {
        columnCount = tableView->horizontalHeader()->count();
        lastColumnIndex = columnCount - 1;
        secondToLastColumnIndex = columnCount - columnStretch;
        tableView->horizontalHeader()->setMinimumSectionSize(allColumnsMinimumWidth);
        setViewHeaderResizeMode(secondToLastColumnIndex, QHeaderView::Interactive);
        setViewHeaderResizeMode(lastColumnIndex, QHeaderView::Interactive);
    }


} // namespace GUIUtil

