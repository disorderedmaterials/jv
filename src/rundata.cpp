/*
	*** RunData Class
	*** src/rundata.cpp
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

#include "rundata.h"
#include "datainterface.h"
#include "messenger.hui"

// Static Members
List<Enumeration> RunData::blockEnumerations_;

/*
 * Single Property Value
 */

// Constructor
SingleValue::SingleValue()
{
}

// Set data
void SingleValue::set(QString group, QString blockName, QString value)
{
	groupName_ = group;
	blockName_ = blockName;
	value_ = value;
}

// Return group name
QString SingleValue::groupName()
{
	return groupName_;
}

// Return block name
QString SingleValue::blockName()
{
	return blockName_;
}

// Return value
QString SingleValue::value()
{
	return value_;
}

/*
 * Run Property
 */

// Return propery from NXentry text
QString propertyNXentries[] = { "isis_cycle", "duration", "__enddate__", "__endtime__", "end_time", "__group__", "instrument_name", "__name__", "proton_charge", "experiment_identifier", "run_number", "__startdate__", "__starttime__", "start_time", "title", "total_mevents", "user_name" };
QString propertyNames[] = { "Cycle", "Duration", "End Date", "End Time", "End Date/Time", "Group", "Instrument", "Name", "uAmps", "RB No.", "Run No.", "Start Date", "Start Time", "Start Date/Time", "Title", "Mev", "User" };
const char propertyCharacters[] = { 'c', 'd', '?', '?', 'e', '\0', '\0', 'n', 'a', 'b', 'r', '?', '?', 's', 't', 'm', 'u' };
bool propertyQuoteFlags[] = { false, true, false, false, false, false, false, false, true, false, false, false, false, false, false, true, false, true };
RunProperty::Property RunProperty::propertyFromNX(QString s)
{
	for (int n=0; n<nProperties; ++n) if (propertyNXentries[n] == s) return (RunProperty::Property) n;
	return RunProperty::nProperties;
}

// Return property name
QString RunProperty::property(RunProperty::Property p)
{
	return propertyNames[p];
}

// Return property from name
RunProperty::Property RunProperty::property(QString s)
{
	for (int n=0; n<nProperties; ++n) if (propertyNames[n] == s) return (RunProperty::Property) n;
	return RunProperty::nProperties;
}

// Return property from indicative character
RunProperty::Property RunProperty::property(char c)
{
	for (int n=0; n<nProperties; ++n) if (propertyCharacters[n] == c) return (RunProperty::Property) n;
	return RunProperty::nProperties;
}

// Return whether specified property should be enclosed in quotes
bool RunProperty::propertyNeedsQuotes(RunProperty::Property p)
{
	return propertyQuoteFlags[p];
}

// Return whether specified property is hidden from the user
bool RunProperty::propertyIsHidden(RunProperty::Property p)
{
	return propertyNXentries[p][0] == '_';
}

// Constructor
RunProperty::RunProperty(): ListItem<RunProperty>()
{
	type_ = nProperties;
}

// Set type
void RunProperty::setType(RunProperty::Property prop)
{
	type_ = prop;
}

// Return type
RunProperty::Property RunProperty::type()
{
	return type_;
}


/*
 * RunData
 */

// Constructor
RunData::RunData(): ListItem<RunData>()
{
	name_ = "Unnamed Run";
	journalSource_ = NULL;
	instrument_ = NULL;
	runNumber_ = -1;
	rbNumber_ = -1;
	duration_ = -1;
	title_ = "";
	user_ = "";
	visible_ = true;
	group_ = -1;
}

// Destructor
RunData::~RunData()
{
}

// Set instrument
void RunData::setInstrument(Instrument* inst)
{
	instrument_ = inst;
}

// Return instrument
Instrument* RunData::instrument()
{
	return instrument_;
}

// Set journal source
void RunData::setJournalSource(Journal* source)
{
	journalSource_ = source;
}

// Return journal source
Journal* RunData::journalSource()
{
	return journalSource_;
}

// Set name
void RunData::setName(QString name)
{
	name_ = name;
}

// Return name
QString RunData::name()
{
	return name_;
}

// Set run number
void RunData::setRunNumber(int rno)
{
	runNumber_ = rno;
}

// Return run number
int RunData::runNumber()
{
	return runNumber_;
}

// Set title
void RunData::setTitle(QString title)
{
	title_ = title;
}

// Return title
QString RunData::title()
{
	return title_;
}

// Set RB number
void RunData::setRBNumber(int rbno)
{
	rbNumber_ = rbno;
}

// Return RB number
int RunData::rbNumber()
{
	return rbNumber_;
}

// Set user
void RunData::setUser(QString user)
{
	user_ = user;
}

// Return user
QString RunData::user()
{
	return user_;
}

// Set proton charge
void RunData::setProtonCharge(double pc)
{
	protonCharge_ = pc;
}

// Return proton charge
double RunData::protonCharge()
{
	return protonCharge_;
}

// Set duration (seconds)
void RunData::setDuration(int duration)
{
	duration_ = duration;
	durationString_ = "";
	int hours = duration_/3600;
	int minutes = (duration_-hours*3600)/60;
	int seconds = duration_ - hours*3600 - minutes*60;
	int days = hours / 24;
	hours -= days*24;
	if (days > 0) durationString_.sprintf("%id %ih %im %is", days, hours, minutes, seconds);
	else if (hours > 0) durationString_.sprintf("%ih %im %is", hours, minutes, seconds);
	else if (minutes > 0) durationString_.sprintf("%im %is", minutes, seconds);
	else durationString_.sprintf("%is", seconds);
}

// Return duration (seconds)
int RunData::duration()
{
	return duration_;
}

// Set start time and date
void RunData::setStartDateTime(QString s)
{
	// Take NXentry style date string
	startDateTime_ = QDateTime::fromString(s, "yyyy-MM-ddTHH:mm:ss");
}

// Return start time and date string
QString RunData::startDateTimeString()
{
	return startDateTime_.toString();
}

// Return start time and date string
QDateTime RunData::startDateTime()
{
	return startDateTime_;
}

// Return start time
QTime RunData::startTime()
{
	return startDateTime_.time();
}

// Return start date
QDate RunData::startDate()
{
	return startDateTime_.date();
}

// Set end time and date
void RunData::setEndDateTime(QString s)
{
	// Take NXentry style date string
	endDateTime_ = QDateTime::fromString(s, "yyyy-MM-ddTHH:mm:ss");
}

// Return end time and date string
QString RunData::endDateTimeString()
{
	return endDateTime_.toString();
}

// Return end time and date string
QDateTime RunData::endDateTime()
{
	return endDateTime_;
}

// Return end time
QTime RunData::endTime()
{
	return endDateTime_.time();
}

// Return end date
QDate RunData::endDate()
{
	return endDateTime_.date();
}

// Set cycle
void RunData::setCycle(int index)
{
	cycle_ = index;
}

// Return cycle
int RunData::cycle()
{
	return cycle_;
}

// Set total mevents
void RunData::setTotalMEvents(double tmev)
{
	totalMEvents_ = tmev;
}

// Return total mevents
double RunData::totalMEvents()
{
	return totalMEvents_;
}

// Set internal Group number
void RunData::setGroup(int group)
{
	group_ = group;
}

// Return internal Group number
int RunData::group()
{
	return group_;
}

// Return duration as formatted string
QString RunData::durationAsString()
{
	return durationString_;
}

// Return specified duration as a formatted string
QString RunData::durationAsString(int nSeconds)
{
	RunData temp;
	temp.setDuration(nSeconds);
	return temp.durationAsString();
}

// Return specified property as a string
QString RunData::propertyAsString(RunProperty::Property prop)
{
	switch (prop)
	{
		case (RunProperty::Cycle):
			return ISIS::cycleText(cycle_);
			break;
		case (RunProperty::Duration):
			return durationString_;
			break;
		case (RunProperty::EndDate):
			return endDateTime_.date().toString();
			break;
		case (RunProperty::EndTime):
			return endDateTime_.time().toString();
			break;
		case (RunProperty::EndTimeAndDate):
			return endDateTime_.toString();
			break;
		case (RunProperty::RBNumber):
			return QString::number(rbNumber_);
			break;
		case (RunProperty::RunNumber):
			return QString::number(runNumber_);
			break;
		case (RunProperty::GroupNumber):
			return QString::number(group_);
			break;
		case (RunProperty::Name):
			return name_;
			break;
		case (RunProperty::ProtonCharge):
			return QString::number(protonCharge_, 'g', 9);
			break;
		case (RunProperty::StartDate):
			return startDateTime_.date().toString();
			break;
		case (RunProperty::StartTime):
			return startDateTime_.time().toString();
			break;
		case (RunProperty::StartTimeAndDate):
			return startDateTime_.toString();
			break;
		case (RunProperty::Title):
			return title_;
			break;
		case (RunProperty::TotalMEvents):
			return QString::number(totalMEvents_);
			break;
		case (RunProperty::User):
			return user_;
			break;
		default:
			printf("Error - case %i (%s) not accounted for in propertyAsString.\n", prop, qPrintable(RunProperty::property(prop)));
			break;
	}
	return "NULL";
}

/*
 * List / Display Control
 */

// Set visibility of the item
void RunData::setVisible(bool visible)
{
	visible_ = visible;
}

// Return visibility
bool RunData::visible()
{
	return visible_;
}

/*
 * Block Value Enumerations
 */

// Add/retrieve enumeration from list
EnumeratedValue* RunData::enumeratedBlockValue(QString block, QString value)
{
	// Does an enumeration for this block already exist?
	Enumeration* en = NULL;
	for (en = blockEnumerations_.first(); en != NULL; en = en->next) if (en->name() == block) break;
	if (en == NULL)
	{
		en = new Enumeration(block);
		blockEnumerations_.own(en);
	}
	
	// Create/retrieve value
	return en->value(value);
}

/*
 * Nexus/Logfile Block Information
 */

// Load block data for this run
bool RunData::loadBlockData(RunData::BlockDataSource source, bool forceReload)
{
	// Has the RunData already had its data loaded?
	if ((!forceReload) && (blockData_.nItems() != 0)) return true;

	// Clear old data (if it exists)
	clearBlockData();

	int sourceOrder[2];
	if (source == RunData::LogBeforeNexusSource)
	{
		sourceOrder[0] = RunData::LogOnlySource;
		sourceOrder[1] = RunData::NexusOnlySource;
	}
	else if (source == RunData::NexusBeforeLogSource)
	{
		sourceOrder[0] = RunData::NexusOnlySource;
		sourceOrder[1] = RunData::LogOnlySource;
	}
	else if (source == RunData::LogOnlySource)
	{
		sourceOrder[0] = RunData::LogOnlySource;
		sourceOrder[1] = -1;
	}
	else
	{
		sourceOrder[0] = RunData::NexusOnlySource;
		sourceOrder[1] = -1;
	}

	for (int n=0; n<2; ++n)
	{
		if (sourceOrder[n] == -1) continue;

		if (sourceOrder[n] == RunData::LogOnlySource)
		{
			msg.print("Looking for log file for run %i...", runNumber_);

			// Search for the logfile for this run
			QString logFile = ISIS::locateFile(this, "log", journalSource_->local(), journalSource_->localDirectory());
			if (!logFile.isEmpty())
			{
				// Load the logfile
				QByteArray data;
				if (DataInterface::readFile(logFile, data) && ISIS::parseLogInformation(this, data))
				{
					msg.print("Successfully parsed logfile " + logFile);
					return true;
				}
				else
				{
					msg.print("Failed to parse logfile " + logFile);
					return false;
				}
			}
			else msg.print("Logfile not found for run %i", runNumber_);
		}
		else if (sourceOrder[n] == RunData::NexusOnlySource)
		{
#ifdef NOHDF
			if (source == RunData::NexusOnlySource)
			{
				msg.print("Warning: Nexus file specifically requested in RunData::loadBlockData(), but no HDF file support has been built in (run number %i).", runNumber_);
				return false;
			}
			return false;
#else
			// If it is a muon instrument, search for nxs_v2 (which is the HDF5 version) rather than nxs (which is HDF4)
			QString nxsFile;
			if (instrument_ && (instrument_->location() == ISIS::Muon)) nxsFile = ISIS::locateFile(this, "nxs_v2", journalSource_->local(), journalSource_->localDirectory());
			else nxsFile = ISIS::locateFile(this, "nxs", journalSource_->local(), journalSource_->localDirectory());

			if (nxsFile.isEmpty()) msg.print("Nexus file not found for run %i", runNumber_);
			else if (ISIS::parseNexusFile(this, nxsFile))
			{
				msg.print("Successfully parsed Nexus file " + nxsFile);
				return true;
			}
			else
			{
				msg.print("Failed to parse Nexus file " + nxsFile);
				return false;
			}
#endif
		}
	}

	return false;
}

// Add Block Data
Data2D* RunData::addBlockData(QString blockName, QString groupName, QDateTime timeOrigin, QDateTime timeEnd)
{
	// Search for existing data with this blockName
	for (Data2D* bd = blockData_.first(); bd != NULL; bd = bd->next)
	{
		if (bd->name() == blockName)
		{
			msg.print("Warning - Tried to add block data '%s' to run number %i but it already exists.\n", qPrintable(blockName), runNumber_);
			return bd;
		}
	}
	Data2D* bd = blockData_.add();
	bd->setName(blockName);
	bd->setGroupName(groupName);
	bd->setRunTimeSpan(timeOrigin, timeEnd);
	return bd;
}

// Add block data value
void RunData::addBlockDataValue(QString blockName, QDateTime dateTime, double value, QString groupName)
{
	// Search for existing block with this name....
	Data2D* data;
	for (data = blockData_.first(); data != NULL; data = data->next) if (data->name() == blockName) break;
	
	if (!data) 
	{
		data = blockData_.add();
		data->setName(blockName);
		data->setRunTimeSpan(startDateTime_, endDateTime_);
		data->setGroupName(groupName);
	}

	data->addRelativePoint(dateTime, value);
}

// Add block data value (enumerated string)
void RunData::addBlockDataValue(QString blockName, QDateTime dateTime, QString value, QString groupName)
{
	// Search for existing block with this name....
	Data2D* data;
	for (data = blockData_.first(); data != NULL; data = data->next) if (data->name() == blockName) break;
	
	if (!data) 
	{
		data = blockData_.add();
		data->setName(blockName);
		data->setRunTimeSpan(startDateTime_, endDateTime_);
		data->setGroupName(groupName);
	}
	
	// Find/create enumerated value
	EnumeratedValue* ev = enumeratedBlockValue(blockName, value);

	data->addRelativePoint(dateTime, ev);
}

// Return list of blockData_
Data2D* RunData::blockData()
{
	return blockData_.first();
}

// Return reference to nth blockData
Data2D& RunData::blockData(int n)
{
	static Data2D dummyData;
	if ((n < 0) || (n >= blockData_.nItems()))
	{
		printf("BlockData index '%i' is out of range.\n", n);
		return dummyData;
	}

	return (*blockData_[n]);
}

// Return reference to named blockData
Data2D& RunData::blockData(QString blockName)
{
	static Data2D dummyData;
	for (Data2D* bd = blockData_.first(); bd != NULL; bd = bd->next) if (bd->name() == blockName) return (*bd);
	printf("BlockData named '%s' doesn't exist in this RunData.\n", qPrintable(blockName));

	return dummyData;
}

// Return whether specified blockData exists for run
bool RunData::hasBlockData(QString blockName)
{
	for (Data2D* bd = blockData_.first(); bd != NULL; bd = bd->next) if (bd->name() == blockName) return true;
	return false;
}

// Add single value data
void RunData::addSingleValue(QString groupName, QString blockName, QString value)
{
	SingleValue* sv = singleValues_.add();
	sv->set(groupName, blockName, value);
}

// Return list of single values
SingleValue* RunData::singleValues()
{
	return singleValues_.first();
}

// Clear all block data
void RunData::clearBlockData()
{
	blockData_.clear();
	singleValues_.clear();
}
