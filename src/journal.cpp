/*
	*** Journal Class
	*** src/journal.cpp
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

#include "journal.h"
#include "instrument.h"

// Constructor
Journal::Journal(): ListItem<Journal>()
{
	name_ = "Unnamed Journal";
	fileName_ = "filename.abc";
	parent_ = NULL;
	local_ = false;
}

// Destructor
Journal::~Journal()
{
}

// Set parent instrument
void Journal::setParent(Instrument* inst)
{
	parent_ = inst;
}

// Return parent instrument
Instrument* Journal::parent()
{
	return parent_;
}

// Set name
void Journal::setName(QString name, QString cycle)
{
	name_ = name;
	cycle_ = cycle;
}

// Return name
QString Journal::name()
{
	return name_;
}

// Return cycle
QString Journal::cycle()
{
	return cycle_;
}

// Set fileName
void Journal::setFileName(QString fileName)
{
	fileName_ = fileName;
}

// Return fileName
QString Journal::fileName()
{
	return fileName_;
}

// Set modification time of journal
void Journal::setModificationTime(QDateTime modTime)
{
	modificationTime_ = modTime;
}

// Return modification time of journal
QDateTime Journal::modificationTime()
{
	return modificationTime_;
}

// Return full local file path of Journal
QString Journal::filePath()
{
	if (parent_ == NULL) return fileName_;
	else return parent_->indexLocalDir().absoluteFilePath(fileName_);
}

// Return full http file path of Journal
QUrl Journal::httpPath()
{
	if (parent_ == NULL) return QUrl(fileName_);
	else if (parent_->indexHttpDir().isEmpty()) return QUrl();
	return QUrl(parent_->indexHttpDir().toString()+fileName_);
}

// Return RunData list
List<RunData>& Journal::runData()
{
	return runData_;
}

// Set this as a local journal
void Journal::setLocal(QDir dir)
{
	local_ = true;
	localDirectory_ = dir;
}

// Return whether this journal is a local user journal
bool Journal::local()
{
	return local_;
}

// Return local directory containing files listed in Journal
QDir Journal::localDirectory()
{
	return localDirectory_;
}
	