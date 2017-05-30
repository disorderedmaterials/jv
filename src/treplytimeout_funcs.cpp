/*
	*** TReplyTimeout - Network reply timeout handler
	*** src/treplytimeout_funcs.cpp
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

#include "treplytimeout.hui"

#include "messenger.hui"
#include <QNetworkReply>
#include <QTimer>

// Constructor
TReplyTimeout::TReplyTimeout(QNetworkReply* reply, const int timeout) : QObject(reply)
{
	Q_ASSERT(reply);
	if (reply)
	{
		QTimer::singleShot(timeout, this, SLOT(timeout()));
	}
}

/*
 * Slots
 */

void TReplyTimeout::timeout()
{
	QNetworkReply* reply = static_cast<QNetworkReply*>(parent());
	if (reply->isRunning())
	{
		msg.print("Network request timed out.\n");
		reply->close();
	}
}
