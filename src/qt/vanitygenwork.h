#ifndef VANITYGENWORK_H
#define VANITYGENWORK_H

#include <QObject>
#include <QDebug>
#include <QThread>

//#include "vanitygenpage.h"


//extern QStringList VanityGenPatternList;


//extern QStringList VanityGenPatternList;

#include "vanity_pattern.h"
#include "vanity_util.h"

//static QString vanity_result;


static vg_context_t *vcp = NULL;

extern double VanityGenHashrate;
extern double VanityGenKeysChecked;
extern int VanityGenNThreads;
extern int VanityGenMatchCase;

struct VanGenStruct{
    int id;
    QString pattern;
    QString privkey;
    QString pubkey;
    QString difficulty;

    //state 0 nothing
    //state 1 difficulty was recalculated
    //state 2 match was found
    //state 3 import was triggered (pattern can now be deleted from worklist)
    int state;
    int notification;
};

extern QList<VanGenStruct> VanityGenWorkList;

extern bool VanityGenRunning;

extern QString VanityGenPassphrase;

extern bool AddressIsMine;

class VanityGenWork : public QObject
{
    Q_OBJECT
public:
    explicit VanityGenWork(QObject *parent = 0);

    void vanityGenSetup(QThread *cThread);

    char **pattern;
    int threads;
    int caseInsensitive;

signals:

public slots:

    int setup();
    void doVanityGenWork();

    void stop_threads();

private:
    int start_threads(vg_context_t *vcp, int nthreads);
};

#endif // VANITYGENWORK_H
