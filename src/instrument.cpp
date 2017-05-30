/*
	*** Instrument Class
	*** src/instrument.cpp
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

#include "instrument.h"
#include "messenger.hui"
#include <QFile>
#include <QXmlStreamReader>
#include <QRegExp>
#include <QSettings>

/*
 * Instrument Block Value
 */

// Constructor
InstrumentBlockValue::InstrumentBlockValue() : ListItem<InstrumentBlockValue>()
{
	name_ = "Unnamed block";
	selected_ = false;
	group_ = NULL;
}

// Set block name and value
void InstrumentBlockValue::set(QString name, InstrumentBlockGroup* group, QString value, QString units, QString setpoint)
{
	name_ = name;
	value_ = value;
	units_ = units;
	group_ = group;
	setPoint_ = setpoint;
}

// Return block name
QString& InstrumentBlockValue::name()
{
	return name_;
}

// Set block value
void InstrumentBlockValue::setValue(QString value)
{
	value_ = value;
}

// Return block value
QString& InstrumentBlockValue::value()
{
	return value_;
}

// Return block units
QString& InstrumentBlockValue::units()
{
	return units_;
}

// Return block setPoint
QString& InstrumentBlockValue::setPoint()
{
	return setPoint_;
}

// Set selection status
void InstrumentBlockValue::setSelected(bool selected)
{
	selected_ = selected;
}

// Return selected status
bool InstrumentBlockValue::selected()
{
	return selected_;
}

// Set parent group
void InstrumentBlockValue::setGroup(InstrumentBlockGroup* group)
{
	group_ = group;
}

// Return parent group (if any)
InstrumentBlockGroup* InstrumentBlockValue::group()
{
	return group_;
}

/*
 * Instrument Block Group
 */

// Constructor
InstrumentBlockGroup::InstrumentBlockGroup() : ListItem<InstrumentBlockGroup>()
{
	name_ = "Unnamed Group";
}

// Set group name
void InstrumentBlockGroup::setName(QString name)
{
	name_ = name;
}

// Return group name
QString& InstrumentBlockGroup::name()
{
	return name_;
}

// Add unique block value
void InstrumentBlockGroup::addValue(QString name, QString value, QString units)
{
	// Search for existing value...
	for (InstrumentBlockValue* val = values_.first(); val != NULL; val = val->next)
	{
		if (val->name() == name)
		{
			val->setValue(value);
			return;
		}
	}

	// No pre-existing value, so create new entry
	InstrumentBlockValue* val = values_.add();
	val->set(name, this, value, units);
}

// Return list of values
InstrumentBlockValue* InstrumentBlockGroup::values()
{
	return values_.first();
}

// Return number of selected block values in this group
int InstrumentBlockGroup::nSelected()
{
	int nSel = 0;
	for (InstrumentBlockValue* value = values_.first(); value != NULL; value = value->next) if (value->selected()) ++nSel;
	return nSel;
}

// Disown specified value
void InstrumentBlockGroup::disownValue(InstrumentBlockValue* value)
{
	values_.cut(value);
}

// Own specified value
void InstrumentBlockGroup::ownValue(InstrumentBlockValue* value)
{
	// First, remove block from its existing group (if it has one)
	if (value->group() != NULL) value->group()->disownValue(value);

	// Add into our list...
	values_.own(value);
	value->setGroup(this);
}

/*
 * Instrument
 */

// Constructor
Instrument::Instrument(): ListItem<Instrument>()
{
	instrument_ = ISIS::nInstruments;
}

// Destructor
Instrument::~Instrument()
{
}

// Set instrument
void Instrument::set(ISIS::ISISInstrument instrmnt)
{
	instrument_ = instrmnt;
	retrieveBlockMapSettings();
	retrieveBlockSelectedSettings();
}

// Return instrument
ISIS::ISISInstrument Instrument::instrument()
{
	return instrument_;
}

// Return name
QString Instrument::capitalisedName()
{
	return ISIS::capitalisedName(instrument_);
}

// Return data file prefix
QString Instrument::fileNamePrefix()
{
	return ISIS::fileNamePrefix(instrument_);
}

// Return short name
QString Instrument::shortName()
{
	return ISIS::shortName(instrument_);
}

// Return ndx name
QString Instrument::ndxName()
{
	return ISIS::ndxName(instrument_);
}

// Set location of journal index file
void Instrument::setIndex(QDir localDir, QUrl httpDir, QString fileName)
{
	indexLocalDir_ = localDir;
	indexHttpDir_ = httpDir;
	indexFileName_ = fileName;
}

// Return local journal index directory
QDir Instrument::indexLocalDir()
{
	return indexLocalDir_;
}

// Return journal index http directory
QUrl Instrument::indexHttpDir()
{
	return indexHttpDir_;
}

// Return journal index local file path
QString Instrument::indexLocalFile()
{
	return indexLocalDir_.absoluteFilePath(indexFileName_);
}

// Return journal index http file path
QUrl Instrument::indexHttpFile()
{
	if (indexHttpDir_.isEmpty()) return QUrl();
	else return QUrl(indexHttpDir_.toString()+indexFileName_);
}

// Return journal filename
QString Instrument::indexFileName()
{
	return indexFileName_;
}

// Add journal to instrument
void Instrument::addJournal(QString journalFile)
{
	Journal* journal = journals_.add();
	journal->setParent(this);
	journal->setFileName(journalFile);
	if (instrument_ == ISIS::LOCAL)
	{
		// Since this is a LOCAL journal, its data path will be localDir_/journalName/
		// Need to check for files in the root of the dir (labelled 'Top') and remove this from the path
		if (journalFile == "Top.xml") journalFile = "";
		QFileInfo info = journalFile;
		journal->setLocal(indexLocalDir_.absoluteFilePath(info.completeBaseName()));
	}

	// Extract cycle information from filename, and look for special cases
	if (journalFile == "All") journal->setName("All", "All");
	else
	{
		if (instrument_ == ISIS::LOCAL)
		{
			QRegExp regexp("(.*).xml");
			regexp.setPatternSyntax(QRegExp::RegExp);
			if (regexp.exactMatch(journal->fileName())) journal->setName(regexp.cap(1), regexp.cap(1));
			else 
			{
				msg.print("Found odd local journal file '%s'. Not added to list.", qPrintable(journal->fileName()));
				journals_.remove(journal);
			}
		}
		else
		{
			// Filename template is 'journal_YR_C.xml' where YR = 2-digit year and C = single digit cycle number
			QRegExp regexp("journal_[0-9][0-9]_[0-9].xml");
			regexp.setPatternSyntax(QRegExp::RegExp);
			if (regexp.exactMatch(journal->fileName()))
			{
				QString year = journalFile.mid(8,2);
				QString cycle = journalFile.mid(11,1);
				
				// Did we get some valid info?
				QString name = "Cycle ";
				name += year;
				name += "/";
				name += cycle;
				journal->setName(name, year + "/" + cycle);
			}
			else 
			{
				msg.print("Found journal file '%s' - assuming it is a journal file preceding per-cycle journal archiving.", qPrintable(journal->fileName()));
				QString name = "Cycle (Multiple)";
				journal->setName(name, "?/?");
			}
		}
	}
}

// Return list of available journals
Journal* Instrument::journals()
{
	return journals_.first();
}

// Clear list of available journals
void Instrument::clearJournals()
{
	journals_.clear();
}

// Return nth journal
Journal* Instrument::journal(int n)
{
	return journals_[n];
}

// Return journal with name specified
Journal* Instrument::journal(QString journalName)
{
	for (Journal* journal = journals_.first(); journal != NULL; journal = journal->next)
	{
		if (journal->name() == journalName) return journal;
		if (journal->cycle() == journalName) return journal;
	}

	return NULL;
}

// Return index of specified journal
int Instrument::journalIndex(Journal* jrnl)
{
	return journals_.indexOf(jrnl);
}

// Return journal for current (i.e. most recent) cycle
Journal* Instrument::currentCycleJournal()
{
	if (journals_.nItems() == 0) return NULL;
	if (journals_.nItems() == 1) return journals_.first();
	else return journals_[journals_.nItems()-2];
}

// Return location of instrument
ISIS::Location Instrument::location()
{
	return ISIS::location(instrument_);
}

// Set modification time for index file
void Instrument::setIndexModificationTime(QDateTime modTime)
{
	indexModificationTime_ = modTime;
}

// Return modification time for index file
QDateTime Instrument::indexModificationTime()
{
	return indexModificationTime_;
}

/*
 * Information (block values)
*/

// Add block to group mapping
void Instrument::addBlockGroupMapping(QString blockName, QString groupName)
{
	blockGroupMap_[blockName] = groupName;
}

// Remove block to group mapping
void Instrument::removeBlockGroupMapping(QString blockName)
{
	if (blockGroupMap_.contains(blockName)) blockGroupMap_.remove(blockName);
}

// Return mapped group for specified block (if any)
QString Instrument::groupForBlock(QString blockName)
{
	if (blockGroupMap_.contains(blockName)) return blockGroupMap_.value(blockName);
	else return "No Group";
}

// Return block groups
InstrumentBlockGroup* Instrument::blockGroups()
{
	return blockGroups_.first();
}

// Clear all block groups
void Instrument::clearBlocks()
{
	blockGroups_.clear();
}

// Add info to specified block
void Instrument::addBlock(QString groupName, QString blockName, QString value, QString units)
{
	// Search for an existing group called 'groupName'
	InstrumentBlockGroup* group = NULL;
	for (group = blockGroups_.first(); group != NULL; group = group->next) if (group->name() == groupName) break;
	
	// If no existing group was found, create a new one
	if (group == NULL)
	{
		group = blockGroups_.add();
		group->setName(groupName);
	}

	// Add block to group
	group->addValue(blockName, value, units);
}

// Move block to specified group
void Instrument::moveBlock(InstrumentBlockValue* block, QString groupName)
{
	if (groupName.isEmpty())
	{
		if (block->group()) block->group()->disownValue(block);
		block->setGroup(NULL);
	}
	else
	{
		// Find (or create) group
		InstrumentBlockGroup* group = NULL;
		for (group = blockGroups_.first(); group != NULL; group = group->next) if (group->name() == groupName) break;
		if (group == NULL)
		{
			group = blockGroups_.add();
			group->setName(groupName);
		}

		group->ownValue(block);
	}
}

// Return number of selected blocks
int Instrument::nSelectedBlocks()
{
	int nSelected = 0;
	for (InstrumentBlockGroup* group = blockGroups_.first(); group != NULL; group = group->next) nSelected += group->nSelected();
	return nSelected;
}

// Retrieve existing block selection settings
void Instrument::retrieveBlockSelectedSettings()
{
	QSettings settings;
	QString s;

	// Enter general instrument blocks settings group
	settings.beginGroup("Blocks_" + capitalisedName());

	// Loop over block groups
	for (InstrumentBlockGroup* group = blockGroups_.first(); group != NULL; group = group->next)
	{
		// Loop over blocks in group
		for (InstrumentBlockValue* value = group->values(); value != NULL; value = value->next)
		{
			// Do we have the value's 'Selected' state?
			s = value->name()+"_Selected";
			if (settings.contains(s)) value->setSelected(settings.value(s).toBool());
		}
	}
	// End block groups group
	settings.endGroup();
}

// Retrieve existing block->group mappings
void Instrument::retrieveBlockMapSettings()
{
	QSettings settings;

	// Read block->group map
	settings.beginGroup("BlockMap_" + capitalisedName());
	foreach(QString key, settings.childKeys()) blockGroupMap_[key] = settings.value(key).toString();
	settings.endGroup();
}

// Store block settings for instrument
void Instrument::storeBlockSettings()
{
	QSettings settings;
	QString s;

	// Enter general instrument blocks settings group
	settings.beginGroup("Blocks_" + capitalisedName());

	// Loop over block groups
	for (InstrumentBlockGroup* group = blockGroups_.first(); group != NULL; group = group->next)
	{
		// Loop over blocks in group
		for (InstrumentBlockValue* value = group->values(); value != NULL; value = value->next)
		{
			// Store 'selected' value
			s = value->name()+"_Selected";
			settings.setValue(s, value->selected());
		}
	}

	// End subgroup
	settings.endGroup();

	// Write block->group map
	settings.beginGroup("BlockMap_" + capitalisedName());
	QMap<QString, QString>::const_iterator i = blockGroupMap_.constBegin();
	while (i != blockGroupMap_.constEnd())
	{
		settings.setValue(i.key(), i.value());
		++i;
	}
	settings.endGroup();

	settings.sync();
}
