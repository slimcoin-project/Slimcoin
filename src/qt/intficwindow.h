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

#ifndef INTFICWINDOW_H
#define INTFICWINDOW_H

#include <QtGui>
#include <QProcess>
#include <QtNetwork>
#include "clientmodel.h"

#include <QMainWindow>
namespace Ui
{
    class IntficWindow;
}

class IntficWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit IntficWindow(QWidget *parent = 0);
    ~IntficWindow();
    void setModel(ClientModel *model);

    Q_SIGNALS:
		void changeText();

    public Q_SLOTS:
        void printProcessStatus();
        void printProcessError();
        void printProcessOutput();
        void textChanged(int index);

private:
	Ui::IntficWindow *ui;
    ClientModel *model;
    QProcess *process;
};

#endif // INTFICWINDOW_H
