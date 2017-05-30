/*
	*** Instrument Class
	*** src/instrument.h
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

#ifndef JOURNALVIEWER_INSTRUMENT_H
#define JOURNALVIEWER_INSTRUMENT_H

#include <QDir>
#include <QUrl>
#include <QMap>
#include "journal.h"
#include "list.h"
#include "isis.h"

// Forward Declarations
class InstrumentBlockGroup;

// Instrument Block Value
class InstrumentBlockValue : public ListItem<InstrumentBlockValue>
{
	public:
	// Constructor
	InstrumentBlockValue();

	private:
	// Name of block value
	QString name_;
	// Value of block value
	QString value_;
	// Units of block value, if any
	QString units_;
	// Setpoint of block value, if any
	QString setPoint_;
	// Whether block value is selected
	bool selected_;
	// Parent group (if any)
	InstrumentBlockGroup* group_;

	public:
	// Set block name and value
	void set(QString name, InstrumentBlockGroup* group, QString value, QString units = QString(), QString setpoint = QString());
	// Return block name
	QString& name();
	// Set block value
	void setValue(QString value);
	// Return block value
	QString& value();
	// Return block units
	QString& units();
	// Return block setPoint
	QString& setPoint();
	// Set selection status
	void setSelected(bool selected);
	// Return selected status
	bool selected();
	// Set parent group
	void setGroup(InstrumentBlockGroup* group);
	// Return parent group (if any)
	InstrumentBlockGroup* group();
};

// Instrument Block Group
class InstrumentBlockGroup : public ListItem<InstrumentBlockGroup>
{
	public:
	// Constructor
	InstrumentBlockGroup();

	private:
	// Name of group
	QString name_;
	// List of block values
	List<InstrumentBlockValue> values_;

	public:
	// Set group name
	void setName(QString name);
	// Return group name
	QString& name();
	// Add unique block value
	void addValue(QString name, QString value, QString units);
	// Return list of values
	InstrumentBlockValue* values();
	// Return number of selected block values in this group
	int nSelected();
	// Disown specified value
	void disownValue(InstrumentBlockValue* value);
	// Own specified value
	void ownValue(InstrumentBlockValue* value);
};

// Instrument Class
class Instrument : public ListItem<Instrument>
{
	public:
	// Constructor / Destructor
	Instrument();
	~Instrument();


	/*
	 * Properties
	 */
	private:
	// Instrument
	ISIS::ISISInstrument instrument_;
	// Local directory for journal index
	QDir indexLocalDir_;
	// Http directory for journal index
	QUrl indexHttpDir_;
	// Journal index filename
	QString indexFileName_;
	// List of available journals
	List<Journal> journals_;
	// Modification time for index file
	QDateTime indexModificationTime_;

	public:
	// Set instrument
	void set(ISIS::ISISInstrument instrmnt);
	// Return instrument
	ISIS::ISISInstrument instrument();
	// Return name
	QString capitalisedName();
	// Return data file prefix
	QString fileNamePrefix();
	// Return short name
	QString shortName();
	// Return ndx name
	QString ndxName();
	// Set location of journal index file
	void setIndex(QDir localDir, QUrl httpDir, QString fileName);
	// Return local journal index directory
	QDir indexLocalDir();
	// Return journal index http directory
	QUrl indexHttpDir();
	// Return journal index local file path
	QString indexLocalFile();
	// Return journal index http file path
	QUrl indexHttpFile();
	// Return journal filename
	QString indexFileName();
	// Add journal to instrument
	void addJournal(QString journalFile);
	// Return list of available journals
	Journal* journals();
	// Clear list of available journals
	void clearJournals();
	// Return nth journal
	Journal* journal(int n);
	// Return journal with name specified
	Journal* journal(QString journalName);
	// Return index of specified journal
	int journalIndex(Journal* jrnl);
	// Return journal for current (i.e. most recent) cycle
	Journal *currentCycleJournal();
	// Return location of instrument
	ISIS::Location location();
	// Set modification time for index file
	void setIndexModificationTime(QDateTime modTime);
	// Return modification time for index file
	QDateTime indexModificationTime();


	/*
	 * Information (block values)
	 */
	private:
	// Block to group map
	QMap<QString, QString> blockGroupMap_;
	// List of block groups
	List<InstrumentBlockGroup> blockGroups_;

	public:
	// Add block to group mapping
	void addBlockGroupMapping(QString blockName, QString groupName);
	// Remove block to group mapping
	void removeBlockGroupMapping(QString blockName);
	// Return mapped group for specified block (if any)
	QString groupForBlock(QString blockName);
	// Return block groups
	InstrumentBlockGroup* blockGroups();
	// Clear all block groups
	void clearBlocks();
	// Add info to specified block
	void addBlock(QString groupName, QString blockName, QString value, QString units);
	// Move block to specified group
	void moveBlock(InstrumentBlockValue* block, QString groupName);
	// Return number of selected blocks
	int nSelectedBlocks();
	// Retrieve existing block selection settings
	void retrieveBlockSelectedSettings();
	// Retrieve existing block->group mappings
	void retrieveBlockMapSettings();
	// Store block settings for instrument
	void storeBlockSettings();
};

#endif
