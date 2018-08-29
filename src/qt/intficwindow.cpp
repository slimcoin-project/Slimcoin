/*Copyright (C) 2009 Cleriot Simon
* Copyright (C) 2018 Graham Higgins <gjh@bel-epa.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA*/

#include "intficwindow.h"
#include "ui_intficwindow.h"
#include "util.h"
#include <string.h>

#include <QProcess>

class QProcess;

bool fZDebug = false;

QStringList bank_moves = {
    "brief",
    "open box",
    "read leaflet",
    "north",
    "north",
    "up",
    "get egg",
    "open egg",
    "down",
    "west",
    "east",
    "open window",
    "west",
    "get sack",
    "open sack",
    "get lunch",
    "get garlic",
    "get bottle",
    "west",
    "drop all",
    "get lamp",
    "east",
    "turn on lamp",
    "up",
    "get all",
    "down",
    "turn off lamp",
    "west",
    "drop all",
    "read news",
    "drop news",
    "move rug",
    "open trap door",
    "turn on lamp",
    "down",
    "south",
    "south",
    "get painting",
    "south",
    "up",
    "turn off lamp",
    "west",
    "open case",
    "put painting in case",
    "open trap door",
    "turn on lamp",
    "down",
    "south",
    "south",
    "west",
    "ne",
    "east",
    "south",
    "examine portrait",
    "get portrait",
    "north",
    "walk through curtain",
    "walk through south wall",
    "examine lettering",
    "walk through curtain",
    "examine bills",
    "get bills",
    "walk through north wall",
    "examine lettering",
    "drop bills",
    "drop portrait",
    "east",
    "east",
    "get all",
    "walk through curtain",
    "south",
    "south",
    "south",
    "drop bills",
    "up",
    "turn off lamp",
    "west",
    "put portrait in case",
    "open trap door",
    "turn on lamp",
    "down",
    "south",
    "south",
    "south",
    "get bills",
    "up",
    "turn off lamp",
    "west",
    "put bills in case"
};

QStringList fcd_moves = {
    "get sword",
    "open trap door",
    "get egg",
    "get rope",
    "turn on lamp",
    "down",
    "east",
    "kill troll with sword",
    "drop sword",
    "north",
    "north",
    "west",
    "east",
    "tie rope to rail",
    "down",
    "turn off lamp",
    "get torch",
    "down",
    "east",
    "north",
    "north",
    "down",
    "up",
    "east",
    "north",
    "read match",
    "read book",
    "north",
    "get screwdriver",
    "get wrench",
    "press yellow button",
    "west",
    "south",
    "turn bolt with wrench",
    "drop wrench",
    "south",
    "nw",
    "north",
    "get trunk",
    "north",
    "get pump",
    "north",
    "up",
    "north",
    "rub mirror",
    "north",
    "north",
    "up",
    "treasure",
    "drop egg",
    "get chalice",
    "temple",
    "drop all",
    "get lamp",
    "get torch",
    "west",
    "get grail",
    "east",
    "sw",
    "rub mirror",
    "east",
    "down",
    "get trident",
    "up",
    "north",
    "rub mirror",
    "north",
    "north",
    "up",
    "drop trident",
    "east",
    "douse candles",
    "read book",
    "west",
    "get match",
    "get bell",
    "west",
    "east",
    "south",
    "down",
    "light match",
    "light candles with match",
    "exorcise",
    "drop book",
    "drop bell",
    "drop candles",
    "up",
    "north",
    "north",
    "up",
    "get chalice",
    "get trident",
    "east",
    "pray",
    "east",
    "west",
    "east",
    "west",
    "west",
    "put grail in case",
    "drop match",
    "put trident in case",
    "get garlic",
    "put chalice in case",
    "open trap door",
    "down",
    "east",
    "north",
    "north",
    "down",
    "north",
    "north",
    "north",
    "up",
    "north",
    "rub mirror",
    "north",
    "north",
    "up",
    "get all",
    "drop book",
    "west",
    "east",
    "sw",
    "rub mirror",
    "west",
    "west",
    "north",
    "nw",
    "west",
    "drop garlic",
    "get figurine",
    "east",
    "south",
    "ne",
    "north",
    "west",
    "drop torch",
    "turn on lamp",
    "down",
    "get bracelet",
    "up",
    "get torch",
    "turn off lamp",
    "east",
    "south",
    "west",
    "south",
    "down",
    "up",
    "put bracelet in case",
    "put figurine in case",
    "put trunk in case",
    "drop all"
};

QStringList coffin_moves {    
    "get torch",
    "get lamp",
    "down",
    "east",
    "north",
    "north",
    "west",
    "nw",
    "get coffin",
    "up",
    "north",
    "get wire",
    "east",
    "north",
    "north",
    "north",
    "up",
    "north",
    "rub mirror",
    "north",
    "north",
    "up",
    "east",
    "pray",
    "east",
    "west",
    "east",
    "west",
    "west",
    "get bottle",
    "put coffin in case",
    "drop wire"
};

QStringList robot_moves = {
    "down",
    "east",
    "north",
    "east",
    "north",
    "read engravings",
    "se",
    "well",
    "east",
    "get necklace",
    "east",
    "read wall",
    "board",
    "open bottle",
    "put water in bucket",
    "disembark",
    "read wall",
    "east",
    "get all",
    "eat eatme cake",
    "east",
    "get flask",
    "read blue cake with flask",
    "read red cake with flask",
    "read orange cake with flask",
    "throw red cake",
    "get spices",
    "west",
    "eat blue cake",
    "drop orange cake",
    "nw",
    "read paper",
    "drop paper",
    "tell robot, east, east",
    "tell robot, push triangle button",
    "tell robot, south, south",
    "get sphere",
    "tell robot, lift cage",
    "get sphere",
    "north",
    "west",
    "south",
    "west",
    "board",
    "put water in bottle",
    "close bottle",
    "disembark",
    "drop bottle",
    "west",
    "west",
    "down",
    "north",
    "open box",
    "get violin",
    "west",
    "west",
    "west",
    "up",
    "put spices in case",
    "put necklace in case",
    "put sphere in case",
    "put violin in case",
    "drop all"
};

QStringList boat_moves = {
    "get torch",
    "get pump",
    "down",
    "east",
    "north",
    "east",
    "ne",
    "ne",
    "echo",
    "get bar",
    "east",
    "east",
    "get shovel",
    "nw",
    "south",
    "west",
    "south",
    "nw",
    "east",
    "down",
    "inflate boat with pump",
    "drop pump",
    "put stick in boat",
    "board",
    "launch",
    "down",
    "down",
    "down",
    "get buoy",
    "west",
    "disembark",
    "get stick",
    "open buoy",
    "get emerald",
    "dig with shovel",
    "dig with shovel",
    "dig with shovel",
    "dig with shovel",
    "get statue",
    "drop shovel",
    "drop buoy",
    "south",
    "south",
    "wave stick",
    "drop stick",
    "east",
    "east",
    "get pot",
    "se",
    "up",
    "up",
    "west",
    "west",
    "north",
    "east",
    "west",
    "west",
    "put pot in case",
    "put statue in case",
    "put emerald in case",
    "put bar in case",
    "drop all"
};

QStringList thief_moves = {
    "get torch",
    "get lamp",
    "get knife",
    "down",
    "east",
    "south",
    "east",
    "west",
    "up",
    "get coins",
    "get keys",
    "sw",
    "up",
    "east",
    "ne",
    "unlock grate with keys",
    "drop keys",
    "open grate",
    "up",
    "sw",
    "west",
    "west",
    "put coins in case",
    "inventory",
    "east",
    "east",
    "east",
    "down",
    "sw",
    "down",
    "east",
    "north",
    "east",
    "south",
    "ne",
    "odysseus",
    "up",
    "kill thief with knife",
    "kill thief with knife",
    "kill thief with knife",
    "kill thief with knife",
    "get egg",
    "get canary",
    "temple",
    "get book",
    "treasure",
    "east",
    "read paper",
    "drop paper",
    "down",
    "push east wall",
    "south",
    "south",
    "west",
    "push south wall",
    "east",
    "north",
    "north",
    "north",
    "west",
    "push south wall",
    "push south wall",
    "east",
    "east",
    "east",
    "push south wall",
    "get card",
    "push south wall",
    "push west wall",
    "east",
    "east",
    "east",
    "north",
    "push west wall",
    "south",
    "west",
    "west",
    "north",
    "north",
    "east",
    "push south wall",
    "west",
    "south",
    "push east wall",
    "north",
    "east",
    "push south wall",
    "west",
    "north",
    "north",
    "north",
    "north",
    "push east wall",
    "west",
    "south",
    "south",
    "south",
    "east",
    "east",
    "north",
    "north",
    "push west wall",
    "north",
    "west",
    "push south wall",
    "push south wall",
    "west",
    "south",
    "south",
    "east",
    "east",
    "north",
    "push west wall",
    "south",
    "west",
    "push north wall",
    "east",
    "north",
    "push west wall",
    "south",
    "west",
    "push north wall",
    "push north wall",
    "west",
    "north",
    "up",
    "west",
    "down",
    "north",
    "east",
    "put card in case",
    "put egg in case",
    "drop book",
    "east",
    "east",
    "north",
    "north",
    "wind canary",
    "get bauble",
    "west",
    "east",
    "west",
    "west",
    "put canary in case",
    "put bauble in case",
    "drop all"
};

QStringList mine_moves = {
    "get lamp",
    "get torch",
    "get screwdriver",
    "down",
    "east",
    "north",
    "north",
    "down",
    "north",
    "north",
    "north",
    "up",
    "north",
    "west",
    "west",
    "north",
    "ne",
    "put screwdriver in basket",
    "north",
    "ne",
    "north",
    "ne",
    "nw",
    "down",
    "down",
    "ne",
    "get coal",
    "south",
    "up",
    "up",
    "east",
    "east",
    "south",
    "put coal in basket",
    "turn on lamp",
    "put lamp in basket",
    "lower basket",
    "north",
    "ne",
    "north",
    "ne",
    "nw",
    "down",
    "down",
    "south",
    "drop all",
    "sw",
    "get lamp",
    "get screwdrive",
    "get coal",
    "east",
    "open lid",
    "put coal in lid",
    "close lid",
    "turn switch with screwdriver",
    "open lid",
    "get diamond",
    "drop screwdriver",
    "nw",
    "put diamond in basket",
    "put lamp in basket",
    "ne",
    "get torch",
    "north",
    "up",
    "up",
    "east",
    "east",
    "south",
    "raise basket",
    "get lamp",
    "turn lamp off",
    "get diamond",
    "west",
    "south",
    "down",
    "up",
    "put diamond in case",
    "drop all"
};

QStringList volcano_moves = { 
    "get torch",
    "get lamp",
    "get brick",
    "get wire",
    "get match",
    "get book",
    "down",
    "east",
    "north",
    "north",
    "west",
    "nw",
    "up",
    "turn on lamp",
    "throw torch at glacier",
    "west",
    "get ruby",
    "west",
    "south",
    "board",
    "open recept",
    "put book in recept",
    "light match",
    "light book with match",
    "wait",
    "wait",
    "land",
    "disembark",
    "tie rope to hook",
    "get coin",
    "south",
    "open purple book",
    "get stamp",
    "north",
    "board",
    "untie rope",
    "wait",
    "wait",
    "wait",
    "land",
    "disembark",
    "tie rope to hook",
    "south",
    "put brick in hole",
    "put wire in brick",
    "light match",
    "light wire with match",
    "north",
    "south",
    "read card",
    "get crown",
    "north",
    "board",
    "untie rope",
    "close recept",
    "wait",
    "wait",
    "wait",
    "wait",
    "wait",
    "disembark",
    "drop card",
    "north",
    "west",
    "south",
    "north",
    "get torch",
    "east",
    "south",
    "south",
    "west",
    "west",
    "up",
    "turn off lamp",
    "put crown in case",
    "put coin in case",
    "put ruby in case",
    "put stamp in case",
    "put torch in case",
    "close case",
    "drop match"
};

/* Last fragment of a deterministic sequence */
QStringList last_moves = {
    "turn on lamp",
    "down",
    "east",
    "north",
    "east",
    "east",
    "east",
    "south",
    "down",
    "east",
    "east",
    "wait",
    "wait",
    "wait",
    "wait",
    "wait",
    "wait",
    "open crypt",
    "north",
    "close crypt",
    "turn off lamp",
    "wait",
    "wait",
    "wait",
    "down",
    "north",
    "drop sword",
    "south",
    "push button",
    "north",
    "get sword",
    "north",
    "enter",
    "raise short pole",
    "push red panel",
    "push red panel",
    "lower short pole",
    "push mahoga panel",
    "push mahoga panel",
    "push mahoga panel",
    "raise short pole",
    "push red panel",
    "push red panel",
    "push red panel",
    "push red panel",
    "push pine panel",
    "north",
    "knock on door",
    "answer, temple",
    "answer, flask",
    "answer, rusty knife",
    "north",
    "tell master, follow",
    "north",
    "east",
    "north",
    "north",
    "tell master, stay",
    "turn dial to 4",
    "press button",
    "south",
    "open door",
    "south",
    "tell master, push button",
    "open bronze door",
    "south"
};

QStringList moves = bank_moves + fcd_moves + coffin_moves + robot_moves + boat_moves + thief_moves + mine_moves + volcano_moves + last_moves;

QStringList save = {"save"};

IntficWindow::IntficWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::IntficWindow)
{
    ui->setupUi(this);

    connect(ui->quitButton, SIGNAL(clicked()), this, SLOT(quit()));
    connect(ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(sendCommand()));
    connect(ui->demoButton, SIGNAL(clicked()), this, SLOT(demo()));
    connect(ui->startButton, SIGNAL(clicked()), this, SLOT(restart()));
    connect(ui->boatButton, SIGNAL(clicked()), this, SLOT(boat()));
    connect(ui->thiefButton, SIGNAL(clicked()), this, SLOT(thief()));
    connect(ui->mineButton, SIGNAL(clicked()), this, SLOT(mine()));
    connect(ui->balloonButton, SIGNAL(clicked()), this, SLOT(balloon()));

    process = new QProcess(this);
    QString datadir = GetDataDir(true).c_str();
    process->setWorkingDirectory(datadir);

    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finish(int, QProcess::ExitStatus)));
    connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(printProcessError()));
    connect(process, SIGNAL(readyRead()), this, SLOT(updateText()));
}

IntficWindow::~IntficWindow()
{
    QProcess *process = this->process;
    if (process->state() == QProcess::Running) {
        process->terminate();
        process->waitForFinished(3000);
    }
    delete ui;
}

void IntficWindow::render() {
    // qDebug() << "cwd is " << process->workingDirectory();
    process->start("zork");

    if (!process->waitForStarted())
        qDebug() << "failed to start";
    show();
}

void IntficWindow::setModel(ClientModel *model)
{
    this->model = model;
}

void IntficWindow::sendCommand()
{
    if (process->state() != QProcess::Running)
        return;
    QString command = ui->lineEdit->text() + '\n';
    process->write(command.toLocal8Bit());
    // qDebug() << "wrote: [" << command << "]\n";
    ui->lineEdit->clear();
    ui->lineEdit->setFocus();
}

void IntficWindow::sendCommand(const QString &command)
{
    if (process->state() != QProcess::Running)
        return;
    process->write(command.toLocal8Bit() + '\n');
    ui->lineEdit->clear();
    ui->lineEdit->setFocus();
    // qDebug() << "forwarded: " << command.toLocal8Bit() + '\n';
}

void IntficWindow::updateText()
{
    if (process->state() != QProcess::Running)
        return;
    response = QString(process->readAll());
    ui->lineEdit->clear();
    ui->textBrowser->setPlainText(response);
    ui->intficmain->repaint();
    // qDebug() << "read: [" << response << "]\n";
}

void IntficWindow::finish(int i, QProcess::ExitStatus e)
{
    QProcess *process = this->process;
    if (process->state() == QProcess::Running) {
        qDebug() << "terminating running process\n";
        process->terminate();
        process->waitForFinished(3000);
    }
    close();
}

void IntficWindow::quit()
{
    QProcess *process = this->process;
    if (process->state() == QProcess::Running) {
        process->write(QString("QUIT\n").toLocal8Bit());
        process->waitForReadyRead();
        process->write(QString("YES\n").toLocal8Bit());
        process->waitForFinished(3000);
    }
    close();
}

void IntficWindow::printProcessError()
{
    QProcess *process = this->process;
    qDebug() << "error " << process->errorString();
}

void IntficWindow::restart()
{
    QProcess *process = this->process;
    if (process->state() == QProcess::Running) {
        process->write(QString("QUIT\n").toLocal8Bit());
        process->waitForReadyRead();
        process->write(QString("YES\n").toLocal8Bit());
        process->waitForFinished(3000);
    }
    close();
    process->start("zork");

    if (!process->waitForStarted())
        qDebug() << "failed to start";
    show();
}

void IntficWindow::boat()
{
    restart();
    runMoves(bank_moves + fcd_moves + coffin_moves + robot_moves);
}

void IntficWindow::thief()
{
    restart();
    runMoves(bank_moves + fcd_moves + coffin_moves + robot_moves + boat_moves);
}

void IntficWindow::mine()
{
    restart();
    runMoves(bank_moves + fcd_moves + coffin_moves + robot_moves + boat_moves + thief_moves);
}

void IntficWindow::balloon()
{
    restart();
    runMoves(bank_moves + fcd_moves + coffin_moves + robot_moves + boat_moves + thief_moves + mine_moves);
}

void IntficWindow::runMoves(QStringList moves) {
    if (process->state() != QProcess::Running)
        return;
    for (int i = 0; i < moves.size(); ++i) {
        QString move = moves[i] + '\n';
        process->write(move.toLocal8Bit());
        if (process->state() != QProcess::Running)
            return;
        else
            process->waitForReadyRead();
    }
}

void IntficWindow::demo()
{
    QProcess *process = this->process;
    if (process->state() != QProcess::Running)
        return;

    for (int i = 0; i < moves.size(); ++i) {
        QString move = moves[i];
        QString charstyped = "";
        QTextCursor textcursor = ui->textBrowser->textCursor();
        textcursor.setPosition(response.length());
        ui->textBrowser->setTextCursor(textcursor);
        ui->textBrowser->insertPlainText(charstyped);
        for(int j = 0; j < move.size(); ++j) {
            charstyped.append(move[j]);
            ui->lineEdit->clear();
            ui->lineEdit->setFocus();
            ui->lineEdit->setText(charstyped);
            ui->textBrowser->insertPlainText(QString(move[j]));
            ui->intficmain->repaint();
            if (!fZDebug)
                usleep(60000);
        }
        ui->lineEdit->setText(charstyped);
        ui->intficmain->repaint();
        if (!fZDebug)
            sleep(1);
        emit ui->lineEdit->returnPressed();
        if (process->state() != QProcess::Running)
            return;
        else
            process->waitForReadyRead();
        int readingtime = 1;
        int words = response.count(QString(" "));
        if (words > 4)
             readingtime = std::min(8, std::max(2, words / 2));
        if (response.length() < 300)
            readingtime = std::min(8, std::max(2, words / 2));
        else if (response.length() > 590)
            readingtime = 8;
        else if (response.length() < 400)
            readingtime = 12;
        else
            readingtime = 20;
        // qDebug() << "words: " << words << " / 2 = " << readingtime << " seconds";
        if (response.startsWith(QString("Living Room")) || response.startsWith(QString("There is a rumble")))
            readingtime = 2;
        if (!fZDebug)
            sleep(readingtime);
    }
}


