/*
	*** RunData Class
	*** src/rundata.h
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

#ifndef JOURNALVIEWER_RUNDATA_H
#define JOURNALVIEWER_RUNDATA_H

#include <QString>
#include <QDate>
#include "list.h"
#include "data2d.h"
// #include "instrument.h"
#include "enumeration.h"
#include "isis.h"

// Forward Declarations
class Journal;
class Instrument;

// Single Property Value
class SingleValue : public ListItem<SingleValue>
{
	public:
	// Constructor
	SingleValue();

	private:
	// Group name
	QString groupName_;
	// Block name
	QString blockName_;
	// Value
	QString value_;

	public:
	// Set data
	void set(QString group, QString blockName, QString value);
	// Return group name
	QString groupName();
	// Return block name
	QString blockName();
	// Return value
	QString value();
};

// Run Property (and ListItem)
class RunProperty : public ListItem<RunProperty>
{
	public:
	// Constructor
	RunProperty();

	// Properties
	enum Property { Cycle, Duration, EndDate, EndTime, EndTimeAndDate, GroupNumber, InstrumentName, Name, ProtonCharge, RBNumber, RunNumber, StartDate, StartTime, StartTimeAndDate, Title, TotalMEvents, User, nProperties };
	// Return propery from NXentry text
	static Property propertyFromNX(QString s);
	// Return whether specified property should be enclosed in quotes
	static bool propertyNeedsQuotes(RunProperty::Property p);
	// Return whether specified property is hidden from the user
	static bool propertyIsHidden(RunProperty::Property p);
	// Return property name
	static QString property(Property p);
	// Return property from name
	static Property property(QString s);
	// Return property from indicative character
	static Property property(char c);

	private:
	// Property Type
	Property type_;
	
	public:
	// Set property type
	void setType(Property prop);
	// Return property type
	Property type();
};

// RunData Storage
class RunData : public ListItem<RunData>
{
	public:
	// Constructor / Destructor
	RunData();
	~RunData();


	/*
	 * Properties
	 */
	private:
	// Source instrument
	Instrument* instrument_;
	// Journal source
	Journal* journalSource_;
	// Name
	QString name_;
	// Run run number
	int runNumber_;
	// Title
	QString title_;
	// RB number
	int rbNumber_;
	// User
	QString user_;
	// Proton Charge (uAmps)
	double protonCharge_;
	// Duration (seconds)
	int duration_;
	// Duration string (h/m/s)
	QString durationString_;
	// Start time / date
	QDateTime startDateTime_;
	// End time / date
	QDateTime endDateTime_;
	// Cycle index
	int cycle_;
	// Total MEvents received
	double totalMEvents_;
	// Internal Group number
	int group_;

	public:
	// Set instrument
	void setInstrument(Instrument* inst);
	// Return instrument
	Instrument* instrument();
	// Set journal source
	void setJournalSource(Journal* source);
	// Return journal source
	Journal* journalSource();
	// Set name
	void setName(QString name);
	// Return name
	QString name();
	// Set run number
	void setRunNumber(int id);
	// Return run number
	int runNumber();
	// Set title
	void setTitle(QString title);
	// Return title
	QString title();
	// RB number
	void setRBNumber(int rbno);
	// Return RB number
	int rbNumber();
	// Set user
	void setUser(QString user);
	// Return user
	QString user();
	// Set proton charge
	void setProtonCharge(double pc);
	// Return proton charge
	double protonCharge();
	// Set duration (seconds)
	void setDuration(int runtime);
	// Return duration (seconds)
	int duration();
	// Return duration as formatted string
	QString durationAsString();
	// Return specified duration as a formatted string
	static QString durationAsString(int nSeconds);
	// Set start time and date
	void setStartDateTime(QString s);
	// Return start time and date string
	QString startDateTimeString();
	// Return start time and date
	QDateTime startDateTime();
	// Return start time
	QTime startTime();
	// Return start date
	QDate startDate();
	// Set end time and date
	void setEndDateTime(QString s);
	// Return end time and date string
	QString endDateTimeString();
	// Return end time and date
	QDateTime endDateTime();
	// Return end time
	QTime endTime();
	// Return end date
	QDate endDate();
	// Set cycle index
	void setCycle(int index);
	// Return cycle index
	int cycle();
	// Set total mevents
	void setTotalMEvents(double tmev);
	// Return total mevents
	double totalMEvents();
	// Return specified property as a string
	QString propertyAsString(RunProperty::Property prop);
	// Set internal Group number
	void setGroup(int group);
	// Return internal Group number
	int group();


	/*
	 * List / Display Control
	 */
	private:
	// Whether item is visible
	bool visible_;
	
	public:
	// Set visibility
	void setVisible(bool visible);
	// Return visibility
	bool visible();


	/*
	 * Block Value Enumerations
	 */
	private:
	// List of enumerations
	static List<Enumeration> blockEnumerations_;

	public:
	// Add/retrieve enumeration from list
	static EnumeratedValue* enumeratedBlockValue(QString block, QString value);


	/*
	 * Logfile/Nexus Block Information
	 */
	public:
	// Source block data preference
	enum BlockDataSource { LogOnlySource, NexusOnlySource, LogBeforeNexusSource, NexusBeforeLogSource };

	private:
	// List of extracted Data from logfile
	List<Data2D> blockData_;
	// List of extracted single-values from logfile
	List<SingleValue> singleValues_;

	public:
	// Load block data for this run
	bool loadBlockData(RunData::BlockDataSource source, bool forceReload = false);
	// Add Block Data
	Data2D* addBlockData(QString blockName, QString groupName, QDateTime timeOrigin, QDateTime timeEnd);
	// Add block data value
	void addBlockDataValue(QString blockName, QDateTime dateTime, double value, QString groupName = "No Group");
	// Add block data value (enumerated text value
	void addBlockDataValue(QString blockName, QDateTime dateTime, QString value, QString groupName = "No Group");
	// Return list of blockData_
	Data2D* blockData();
	// Return reference to nth blockData
	Data2D& blockData(int n);
	// Return reference to named blockData
	Data2D& blockData(QString blockName);
	// Return whether specified blockData exists for run
	bool hasBlockData(QString blockName);
	// Add single value data
	void addSingleValue(QString groupName, QString blockName, QString value);
	// Return list of single values
	SingleValue* singleValues();
	// Clear all block data
	void clearBlockData();
};

#endif
