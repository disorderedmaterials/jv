/*
	*** ISIS Helper / Data
	*** src/isis.h
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

#ifndef JOURNALVIEWER_ISIS_H
#define JOURNALVIEWER_ISIS_H

#include "list.h"
#include <QXmlStreamReader>
#include <QDateTime>
#include <QDir>
#include <hdf5.h>

// Forward Declarations
class JournalViewer;
class InstrumentInfo;
class RunData;
class Journal;
class Instrument;
class Data2D;

// ISIS Helper Class
class ISIS
{
	public:
	// Constructor
	ISIS();
	// Destructor
	~ISIS();


	/*
	 * Parent
	 */
	private:
	// Parent JournalViewer
	static JournalViewer* parent_;

	public:
	// Set parent
	static void setParent(JournalViewer* parent);


	/*
	 * Instrument Information
	 */
	public:
	// Available Instruments
	enum ISISInstrument { LOCAL, ALF, ARGUS, CHIPIR, CSP, EMMA, EMU, ENG, GEM, HET, HIFI, HRP, IMAT, INS, INTER, IRS, IRS_SET, LARMOR, LET, LOQ, MAP, MAR, MER, MUS, NIMROD, OFFSPEC, OSI, PRL, POL, POLREF, SLS, SANS2D, SRF, SXD, TSC, VESUVIO, WISH, ZOOM, CHRONUS, nInstruments };
	// Instrument Location
	enum Location { Local, TS1, TS2, Muon };

	private:
	// Array of instrument data
	static InstrumentInfo instruments_[];

	public:
	// Return instrument with long/short name provided
	static ISIS::ISISInstrument instrument(QString capitalisedName);
	// Return capitalised instrument name
	static const QString capitalisedName(ISIS::ISISInstrument inst);
	// Return filename prefix (for new format files)
	static const QString fileNamePrefix(ISIS::ISISInstrument inst);
	// Return local 'ndx***' instrument name
	static QString ndxName(ISIS::ISISInstrument inst);
	// Return short name used in naming
	static const QString shortName(ISIS::ISISInstrument inst);
	// Return instrument location
	static const ISIS::Location location(ISIS::ISISInstrument inst);


	/*
	 * Global Data
	 */
	private:
	// List of Cycles
	static QStringList cycles_;

	public:
	// Return cycle index for specified cycle text
	static int cycleIndex(QString cycleName);
	// Return cycle text for specified cycle index
	static QString cycleText(int index);
	// Locate file for RunData
	static QString locateFile(RunData* runData, QString extension, bool useLocalDirectory, QDir localDir);


	/*
	 * Data Parsing
	 */
	private:
	// Temporary start/end time variables
	static QDateTime startTime_, endTime_;
	// Current block name being investigated by nexusBlockIterator
	static QString currentBlock_;
	// Group name for current block being investigated by nexusBlockIterator
	static QString currentGroup_;
	// Identifiers for time/value data in blocks without a 'value_log' subgroup
#ifndef NOHDF
	static hid_t timeDataSet_, valueDataSet_;
#endif

	public:
#ifndef NOHDF
	// Retrieve string from specified Nexus dataset
	static bool nexusExtractString(hid_t rootLocation, const char* name, QString& dest);
	// Retrieve double from specified Nexus dataset
	static bool nexusExtractDouble(hid_t rootLocation, const char* name, double& dest);
	// Retrieve integer from specified Nexus dataset
	static bool nexusExtractInteger(hid_t rootLocation, const char* name, int& dest);
	// Retrieve time/value data from specified Nexus datasets
	static herr_t nexusExtractTimeValueData(RunData* runData, hid_t time, hid_t value);
#endif
	// Parse journal index from specified QByteArray
	static bool parseJournalIndex(Instrument* inst, QByteArray& data);
	// Parse journal data from specified QByteArray
	static bool parseJournalData(Journal* jrnl, QByteArray& data, bool addUniqueOnly = false, bool forceISOEncoding = false);
	// Parse instrument information (blocks etc.)
	static bool parseInstrumentInformation(Instrument* inst, QByteArray& data);
	// Parse log information (block data etc.) from log dile
	static bool parseLogInformation(RunData* runData, QByteArray& data);
#ifndef NOHDF
	// Parse Nexus file
	static bool parseNexusFile(RunData* runData, QString fileName);
	// Iterator callback for HDF5 (group access)
	static herr_t nexusGroupIterator(hid_t loc_id, const char *name, const H5L_info_t *info, void *operator_data);
	// Iterator for block data
	static herr_t nexusBlockIterator(hid_t loc_id, const char *name, const H5L_info_t *info, void *operator_data);
#endif
};

// ISIS Instrument Information
class InstrumentInfo
{
	public:
	// Instrument name
	const QString name;
	// Short instrument name
	const QString shortName;
	// Instrument Location
	const ISIS::Location location;
	// NDX Machine Name
	const QString ndxName;
	// Datafile prefix
	const QString fileNamePrefix;
};

#endif
