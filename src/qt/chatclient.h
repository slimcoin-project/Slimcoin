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

#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <QListView>
#include <QScrollBar>
#include <QSystemTrayIcon>
#include <QTextEdit>
#include <QtGui>
#include <QtNetwork>

class ChatClient : public QTcpSocket
{
	Q_OBJECT

	public:
        ChatClient();
		QTextEdit *outstring;
        QListView *userList;
		QString pseudo,chatclient,msgQuit;
		int port;
        QTabWidget *tab;
		QMap<QString,QTextEdit *> conversations;
		QSystemTrayIcon *tray;

		bool updateUsers;

		QString parseCommand(QString comm,bool chatclient=false);

		QWidget *parent;


    Q_SIGNALS:
		void pseudoChanged(QString newPseudo);
		void joinTab();
        void tabJoined();

    public Q_SLOTS:
		void readChatClient();
		void errorSocket(QAbstractSocket::SocketError);
        void connected();
        void joins();
		void sendData(QString txt);
		void join(QString chan);
        void leave(QString chan);
        void writestring(QString txt,QString destChan="",QString msgTray="");
        void updateUsersList(QString chan="",QString message="");

    // void tabChanged(int index);
};

#endif // CHATCLIENT_H
