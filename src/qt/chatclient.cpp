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

#include "chatclient.h"
#include "chatwindow.h"
        QStringList users;
        bool delist = true;
ChatClient::ChatClient()
{
	connect(this, SIGNAL(readyRead()), this, SLOT(readChatClient()));
	connect(this, SIGNAL(connected()), this, SLOT(connected()));
    connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(errorSocket(QAbstractSocket::SocketError)));

	updateUsers=false;
}




void ChatClient::errorSocket(QAbstractSocket::SocketError error)
{
	switch(error)
	{
		case QAbstractSocket::HostNotFoundError:
            outstring->append(tr("<em>ERROR : can't find freenode server.</em>"));
			break;
		case QAbstractSocket::ConnectionRefusedError:
            outstring->append(tr("<em>ERROR : server refused connection</em>"));
			break;
		case QAbstractSocket::RemoteHostClosedError:
            outstring->append(tr("<em>ERROR : server cut connection</em>"));
			break;
		default:
            outstring->append(tr("<em>ERROR : ") + this->errorString() + tr("</em>"));
	}
}

void ChatClient::connected()
{
    outstring->append("Connecting...");

	sendData("USER "+pseudo+" localhost "+chatclient+" :"+pseudo);
    sendData("NICK "+pseudo);
    outstring->append("Connected to freenode.");

}

void ChatClient::joins()
{
    join("#slimcoin");
}

void ChatClient::readChatClient()
{
        QString message=QString::fromUtf8(this->readAll());


	QString currentChan=tab->tabText(tab->currentIndex());

	if(message.startsWith("PING :"))
	{
		QStringList _list=message.split(" ");
		QString msg="PONG "+_list.at(1);
		sendData(msg);
	}
	else if(message.contains("Nickname is already in use."))
	{
        pseudo=pseudo+"_2";
		pseudo.remove("\r\n");
		sendData("NICK "+pseudo);
        Q_EMIT pseudoChanged(pseudo);
        writestring("-> Name changed to "+pseudo);
	}
        else if(updateUsers==true)
	{
		updateUsersList("",message);
	}

    QStringList list=message.split("\r\n");
        Q_FOREACH(QString msg,list)
        {
            if(msg.contains(QRegExp(":([a-zA-Z0-9]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ PRIVMSG ([a-zA-Z0-9\\#]+) :(.+)")))
            {
                QRegExp reg(":([a-zA-Z0-9]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ PRIVMSG ([a-zA-Z0-9\\#]+) :(.+)");
                QString msg2=msg;
                    writestring(msg.replace(reg,"\\2 <b>&lt;\\1&gt;</b> \\3"),"",msg2.replace(reg,"\\2 <\\1> \\3"));
            }
            else if(msg.contains(QRegExp(":([a-zA-Z0-9]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ JOIN ([a-zA-Z0-9\\#]+)")))
            {
                QRegExp reg(":([a-zA-Z0-9]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ JOIN ([a-zA-Z0-9\\#]+)");
                QString msg2=msg;
                writestring(msg.replace(reg,"\\2 <i>-> \\1 join \\2</i><br />"),"",msg2.replace(reg,"-> \\1 join \\2"));
                updateUsersList(msg.replace(reg,"\\2"));
            }
            else if(msg.contains(QRegExp(":([a-zA-Z0-9]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ PART ([a-zA-Z0-9\\#]+)")))
            {
                QRegExp reg(":([a-zA-Z0-9]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ PART ([a-zA-Z0-9\\#]+) :(.+)");
                QString msg2=msg;
                writestring(msg.replace(reg,"\\2 <i>-> \\1 quit \\2 (\\3)</i><br />"),"",msg2.replace(reg,"-> \\1 quit \\2"));
                updateUsersList(msg.replace(reg,"\\2"));
            }
            else if(msg.contains(QRegExp(":([a-zA-Z0-9]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ QUIT (.+)")))
            {
                QRegExp reg(":([a-zA-Z0-9]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ QUIT (.+)");
                QString msg2=msg;
                writestring(msg.replace(reg,"\\2 <i>-> \\1 quit this server (\\2)</i><br />"),"",msg2.replace(reg,"-> \\1 left"));
                updateUsersList(msg.replace(reg,"\\2"));
            }
            else if(msg.contains(QRegExp(":([a-zA-Z0-9]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ NICK :(.+)")))
            {
                QRegExp reg(":([a-zA-Z0-9]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ NICK :(.+)");
                QString msg2=msg;
                writestring(msg.replace(reg,"<i>\\1 is now called \\2</i><br />"),"",msg2.replace(reg,"-> \\1 is now called \\2"));
                updateUsersList(currentChan);
            }
            else if(msg.contains(QRegExp(":([a-zA-Z0-9]+)\\![a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ KICK ([a-zA-Z0-9\\#]+) ([a-zA-Z0-9]+) :(.+)")))
            { 
                QRegExp reg(":([a-zA-Z0-9]+)\\!~[a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ KICK ([a-zA-Z0-9\\#]+) ([a-zA-Z0-9]+) :(.+)");
                QString msg2=msg;
                writestring(msg.replace(reg,"\\2 <i>-> \\1 kicked \\3 (\\4)</i><br />"),"",msg2.replace(reg,"-> \\1 quit \\3"));
                updateUsersList(msg.replace(reg,"\\2"));
            }
            else if(msg.contains(QRegExp(":([a-zA-Z0-9]+)\\![a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ NOTICE ([a-zA-Z0-9]+) :(.+)")))
            {
                if(conversations.contains(currentChan))
                {
                    QRegExp reg(":([a-zA-Z0-9]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ NOTICE [a-zA-Z0-9]+ :(.+)");
                    writestring(msg.replace(reg,"<b>[NOTICE] <i>\\1</i> : \\2 <br />"),currentChan);
                }
                else if(currentChan==chatclient)
                {
                    QRegExp reg(":([a-zA-Z0-9]+)\\![~a-zA-Z0-9]+@[a-zA-Z0-9\\/\\.-]+ NOTICE [a-zA-Z0-9]+ :(.+)");
                    writestring(msg.replace(reg,"<b>[NOTICE] <i>\\1</i> : \\2 <br />"));
                }
            }
            else if(msg.contains("/MOTD command."))
            {
             joins();


            }



        }

        //}
}

void ChatClient::sendData(QString txt)
{
	if(this->state()==QAbstractSocket::ConnectedState)
    {
        this->write((txt+"\r\n").toUtf8());
	}
}

QString ChatClient::parseCommand(QString comm,bool chatclient)
{
    if(comm.startsWith("/"))
    {
        comm.remove(0,1);
        QString pref=comm.split(" ").first();
        QStringList args=comm.split(" ");
        args.removeFirst();
        QString destChan=tab->tabText(tab->currentIndex());
        QString msg=args.join(" ");

        if(pref=="me")
            return "PRIVMSG "+destChan+" ACTION " + msg + "";
        else if(pref=="msg")
            return "MSG "+destChan+" ACTION " + msg + "";
        else if(pref=="join")
        {
            join(msg);
            return " ";
        }
        else if(pref=="quit")
        {
            if(msg == "")
                return "QUIT "+msgQuit;
            else
                return "QUIT "+msg;
        }
        else if(pref=="part")
        {
            tab->removeTab(tab->currentIndex());

            if(msg == "")
            {
                if(msg.startsWith("#"))
                    destChan=msg.split(" ").first();

                if(msgQuit=="")
                    return "PART "+destChan+" using IrcLightClient";
                else
                    return "PART "+destChan+" "+msgQuit;
            }
            else
                return "PART "+destChan+" "+msg;

            conversations.remove(destChan);
        }
        else if(pref=="kick")
        {
            QStringList kicked=msg.split(" ");
            QString c1,c2,c3;
            if(kicked.count() > 0) c1=" "+kicked.first();
            else c1="";
            if(kicked.count() > 1) c2=" "+kicked.at(1);
            else c2="";
            if(kicked.count() > 2) c3=" "+kicked.at(2);
            else c3="";

            if(c1.startsWith("#"))
                return "KICK"+c1+c2+c3;
            else
                return "KICK "+destChan+c1+c2;
        }
        else if(pref=="update")
        {
            updateUsers=true;
            return "WHO "+destChan;
        }
        else if(pref=="ns")
        {
            return "NICKSERV "+msg;
        }
        else if(pref=="nick")
        {
            Q_EMIT pseudoChanged(msg);
            writestring("-> Nickname changed to "+msg);
            return "NICK "+msg;
        }
        else if(pref=="msg")
        {
            return "MSG "+msg;
        }

        else
            return pref+" "+msg;
    }
    else if(!chatclient)
	{
        QString destChan=tab->tabText(tab->currentIndex());
                if(comm.endsWith("<br />"))
                    comm=comm.remove(QRegExp("<br />$"));
                writestring("<b>&lt;"+pseudo+"&gt;</b> "+comm,destChan);

        if(comm.startsWith(":"))
            comm.insert(0,":");

        return "PRIVMSG "+destChan+" "+comm.replace(" ","\t");
    }
	else
	{
		return "";
	}
    return "";
}

void ChatClient::join(QString chan)
{
    outstring->append("Joining "+ chan +" channel");
    Q_EMIT joinTab();
	QTextEdit *textEdit=new QTextEdit;
	int index=tab->insertTab(tab->currentIndex()+1,textEdit,chan);
	tab->setTabToolTip(index,chatclient);
	tab->setCurrentIndex(index);

	textEdit->setReadOnly(true);

	conversations.insert(chan,textEdit);

        sendData("JOIN "+chan);

    Q_EMIT tabJoined();
}
void ChatClient::leave(QString chan)
{
    sendData(parseCommand("/part "+chan+ " "+msgQuit));
}

void ChatClient::writestring(QString txt,QString destChan,QString msgTray)
{
	if(destChan!="")
        {
            conversations[destChan]->setHtml(conversations[destChan]->toHtml()+txt);
            QScrollBar *sb = conversations[destChan]->verticalScrollBar();
            sb->setValue(sb->maximum());
        }
        else if(txt.startsWith("#"))
        {
            QString dest=txt.split(" ").first();
            QStringList list=txt.split(" ");
            list.removeFirst();
            txt=list.join(" ");
            conversations[dest]->setHtml(conversations[dest]->toHtml()+txt);
            QScrollBar *sb = conversations[dest]->verticalScrollBar();
            sb->setValue(sb->maximum());        }
        else
        {
            txt.replace("\r\n","<br />");
            outstring->setHtml(outstring->toHtml()+txt+"<br />");
            QScrollBar *sb = outstring->verticalScrollBar();
            sb->setValue(sb->maximum());
        }


}

void ChatClient::updateUsersList(QString chan,QString message)
{
    message = message.replace("\r\n","");
    message = message.replace("\r","");
    if(chan!=chatclient)
    {
    if(updateUsers==true || message != "")
    {
        QString __list=message.replace(":","");
        QStringList _list=__list.split(" ");

        if (delist == true) users.clear();

        for(int i=5; i < _list.count(); i++)
        {
            users.append(_list.at(i));
        }
        updateUsers=false;
        if (_list.count() < 53) delist = true;
        else delist = false;
		QStringListModel *model = new QStringListModel(users);
		userList->setModel(model);
		userList->update();
	}
	else
	{
        updateUsers=true;
        sendData("NAMES "+chan);
	}
    }
    else
    {
        QStringListModel model;
        userList->setModel(&model);
        userList->update();
    }
}

