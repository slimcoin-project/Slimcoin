/*Copyright (C) 2009 Cleriot Simon
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

class QProcess;

IntficWindow::IntficWindow(QWidget *parent):
    QMainWindow(parent), ui(new Ui::IntficWindow)
{
    ui->setupUi(this);

    // QProcess *process = new QProcess(this);
    // QString program = "Zork";
    // QString folder = "/tmp";
    // process->start(program, QStringList() << folder);

    QProcess *process = new QProcess(this);
    process->start("/tmp/Zork");

    connect(process, SIGNAL(stateChanged(QProcess::ProcessState)), SLOT(printProcessStatus()));
    connect(process, SIGNAL(error(QProcess::ProcessError)), SLOT(printProcessError()));
    connect(process, SIGNAL(readyRead()), SLOT(printProcessOutput()));
}

IntficWindow::~IntficWindow()
{
    delete ui;
}

void IntficWindow::setModel(ClientModel *model)
{
    this->model = model;
}

void IntficWindow::textChanged(int index)
{
    // if(index != 0)
    //     currentTab()->updateUsersList(ui->tab->tabText(index));
}

void IntficWindow::printProcessStatus()
{
    QProcess *process = this->process;
    qDebug() << process->state();
}

void IntficWindow::printProcessError()
{
    QProcess *process = this->process;
    qDebug() << process->errorString();
}

void IntficWindow::printProcessOutput()
{
    QProcess *process = this->process;
    qDebug() << process->readAll();
}



