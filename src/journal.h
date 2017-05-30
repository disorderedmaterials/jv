/*
	*** Journal Class
	*** src/journal.h
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

#ifndef JOURNALVIEWER_JOURNAL_H
#define JOURNALVIEWER_JOURNAL_H

#include <QString>
#include <QDate>
#include <QUrl>
#include <QDir>
#include "list.h"
#include "rundata.h"

// Forward Declarations
class Instrument;

// Journal Storage
class Journal : public ListItem<Journal>
{
	public:
	// Constructor / Destructor
	Journal();
	~Journal();


	/*
	 * Properties
	 */
	private:
	// Parent Instrument
	Instrument* parent_;
	// Name (including 'Cycle' prefix)
	QString name_;
	// Cycle
	QString cycle_;
	// Journal filename
	QString fileName_;
	// Modification time of journal file on loading
	QDateTime modificationTime_;
	// RunData contained in this journal
	List<RunData> runData_;
	// Flag indicating that this journal is a local user journal
	bool local_;
	// Local directory containing files listed in this journal (if local_)
	QDir localDirectory_;

	public:
	// Set parent instrument
	void setParent(Instrument* inst);
	// Return parent instrument
	Instrument* parent();
	// Set name
	void setName(QString name, QString cycle);
	// Return name
	QString name();
	// Return cycle
	QString cycle();
	// Set fileName
	void setFileName(QString fileName);
	// Return fileName
	QString fileName();
	// Set modification time of journal
	void setModificationTime(QDateTime modTime);
	// Return modification time of journal
	QDateTime modificationTime();
	// Return full local file path of Journal
	QString filePath();
	// Return full http file path of Journal
	QUrl httpPath();
	// Return RunData list
	List<RunData>& runData();
	// Set this as a local journal
	void setLocal(QDir dir);
	// Return whether this journal is a local user journal
	bool local();
	// Return directory containing files listed in Journal
	QDir localDirectory();
};

#endif
