/*
	*** Messaging Routines
	*** src/messenger_funcs.cpp
	Copyright T. Youngs 2012-2016

	This file is part of JournalViewer.

	JournalViewer is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	JournalViewer is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with JournalViewer.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "messenger.hui"
#include <QTextBrowser>
#include <QTextStream>
#include <QDateTime>
#include <stdarg.h>
#include <stdio.h>

// Static Singletons
Messenger msg;

// Constructor
Messenger::Messenger()
{
	// Private variables
	textBrowser_ = NULL;
	toStdout_ = false;
}

// Set target QTextBrowser
void Messenger::setTextBrowser(QTextBrowser* tb)
{
	textBrowser_ = tb;

	// Connect logText signal to LogWindow
	connect(this, SIGNAL(displayText(QString)), textBrowser_, SLOT(append(QString)), Qt::QueuedConnection);
}

// Set whether to redirect all output to stdout
void Messenger::setToStdout(bool b)
{
	toStdout_ = b;
}

// Display message
void Messenger::print(QString text)
{
	if (toStdout_) printf("%s\n", qPrintable(text));
	else emit(displayText(QDateTime::currentDateTime().toString(Qt::ISODate) + "  " + text));
}

// Print standard message
void Messenger::print(const char* fmt, ...)
{
	static char text[1024];

	text[0] = '\0';

	va_list arguments;
	va_start(arguments,fmt);
	vsprintf(text, fmt, arguments);
	va_end(arguments);

	if (toStdout_) printf("%s\n", qPrintable(text));
	else emit(displayText(QDateTime::currentDateTime().toString(Qt::ISODate) + "  " + text));
}
