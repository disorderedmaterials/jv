/*
	*** ISIS Helper Functions / Data
	*** src/isis.cpp
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

#include "isis.h"
#include "jv.h"
#include "instrument.h"
#include "messenger.hui"
#include <QXmlStreamReader>
#include <QTextStream>
#include <QRegularExpression>

// Static Members
QStringList ISIS::cycles_;
JournalViewer* ISIS::parent_ = NULL;
QDateTime ISIS::startTime_;
QDateTime ISIS::endTime_;
QString ISIS::currentBlock_;
QString ISIS::currentGroup_;
#ifndef NOHDF
hid_t ISIS::timeDataSet_;
hid_t ISIS::valueDataSet_;
#endif

// Constructor
ISIS::ISIS()
{
}

// Destructor
ISIS::~ISIS()
{
}

/*
 * Parent
 */

// Set parent
void ISIS::setParent(JournalViewer* parent)
{
	parent_ = parent;
}

/*
 * Instrument Information
 */

// Return instrument with long/short name provided
ISIS::ISISInstrument ISIS::instrument(QString name)
{
	QString capitalisedName = name.toUpper();
	for (int n=0; n<ISIS::nInstruments; ++n)
	{
		if (capitalisedName == instruments_[n].name) return (ISIS::ISISInstrument) n;
		if (capitalisedName == instruments_[n].shortName) return (ISIS::ISISInstrument) n;
	}
	return ISIS::nInstruments;
}

// Return capitalised name of instrument
const QString ISIS::capitalisedName(ISIS::ISISInstrument inst)
{
	return instruments_[inst].name;
}

// Return filename prefix (for new format files)
const QString ISIS::fileNamePrefix(ISIS::ISISInstrument inst)
{
	if (!instruments_[inst].fileNamePrefix.isEmpty()) return instruments_[inst].fileNamePrefix;
	else return instruments_[inst].name;
}

// Return ndx-style name for instrument
QString ISIS::ndxName(ISIS::ISISInstrument inst)
{
	if (!instruments_[inst].ndxName.isEmpty()) return instruments_[inst].ndxName;

	QString result = "ndx";
	result += instruments_[inst].name;
	result = result.toLower();
	return result;
}

// Return short name used in files
const QString ISIS::shortName(ISIS::ISISInstrument inst)
{
	return instruments_[inst].shortName;
}

// Return instrument location
const ISIS::Location ISIS::location(ISIS::ISISInstrument inst)
{
	return instruments_[inst].location;
}

/*
 * Global Data
 */

// Return cycle index for specified cycle text
int ISIS::cycleIndex(QString cycleName)
{
	int index = cycles_.indexOf(cycleName);
	if (index == -1)
	{
		cycles_ << cycleName;
		index = cycles_.count()-1;
	}
	return index;
}

// Return cycle index for specified cycle text
QString ISIS::cycleText(int index)
{
	if ((index < 0) || (index >= cycles_.count())) return "UNKNOWN";
	else return cycles_.at(index);
}

// Locate data file with given extension
QString ISIS::locateFile(RunData* runData, QString extension, bool useLocalDirectory, QDir localDir)
{
	// Check RunData
	if (runData == NULL)
	{
		msg.print("ISIS::locateFile() - NULL RunData passed.");
		return QString();
	}

	// Construct both possible filenames
	QString newFileName, oldFileName;
	newFileName.sprintf("%s%08i.", qPrintable(runData->instrument()->fileNamePrefix()), runData->runNumber());
	newFileName += extension;
	oldFileName.sprintf("%s%05i.", qPrintable(runData->instrument()->shortName()), runData->runNumber());
	oldFileName += extension;

	// Check main data directory or local directory?
	if (useLocalDirectory)
	{
		if (!parent_->validUserDataDirectory())
		{
			msg.print("ISIS::locateFile() - Local directory demanded, but no valid user data directory is set.\n");
			return QString();
		}

		if (runData->journalSource() == NULL)
		{
			msg.print("ISIS::locateFile() - Local directory demanded, but Journal source in RunData is not set.\n");
			return QString();
		}
		
		QDir path(runData->journalSource()->localDirectory());
		msg.print("Looking for file %s or %s in local dir %s\n", qPrintable(newFileName), qPrintable(oldFileName), qPrintable(path.absolutePath()));

		if (QFile::exists(path.absoluteFilePath(newFileName))) return path.absoluteFilePath(newFileName);
		if (QFile::exists(path.absoluteFilePath(oldFileName))) return path.absoluteFilePath(oldFileName);
	}
	else
	{
		if (!parent_->validDataDirectory())
		{
			msg.print("ISIS::locateFile() - No valid data directory is set.\n");
			return QString();
		}

		QDir path(parent_->dataDirectory().absoluteFilePath(runData->instrument()->ndxName() + "/Instrument/data/cycle_" + ISIS::cycleText(runData->cycle())));
		msg.print("Looking for file %s or %s in %s\n", qPrintable(newFileName), qPrintable(oldFileName), qPrintable(path.absolutePath()));

		if (QFile::exists(path.absoluteFilePath(newFileName)))
		{
			msg.print("ISIS::locateFile() - File found in data directory: " + path.absoluteFilePath(newFileName));
			return path.absoluteFilePath(newFileName);
		}
		if (QFile::exists(path.absoluteFilePath(oldFileName)))
		{
			msg.print("ISIS::locateFile() - File found in data directory: " + path.absoluteFilePath(oldFileName));
			return path.absoluteFilePath(oldFileName);
		}
	}

	// No match found
	return QString();
}

/*
 * Data Parsing
 */

// Parse Instrument's journal index from supplied QByteArray
bool ISIS::parseJournalIndex(Instrument* inst, QByteArray& data)
{
	// Create a stream reader
	QXmlStreamReader stream(data);

	// Parse XML until end of file
	while(!stream.atEnd() && !stream.hasError())
	{
		// Read next element - skip if StartDocument
		QXmlStreamReader::TokenType token = stream.readNext();
		if (token == QXmlStreamReader::StartDocument) continue;

		// If the token is StartElement, is it 'journal'?
		if ((token == QXmlStreamReader::StartElement) && (stream.name() == "journal"))
		{
			msg.print("--> Found journal element.");
			continue;
		}

		// If it is a journal entry ('file') then pull data from it
		if (stream.name() == "file")
		{
			// Grab NXentry attributes (should just be 'name')
			QXmlStreamAttributes attributes = stream.attributes();
			if (attributes.hasAttribute("name"))
			{
				inst->addJournal(attributes.value("name").toString());
				msg.print("--> Found entry for journal file %s", qPrintable(attributes.value("name").toString()));
			}
			else msg.print("Warning - journal file entry has no 'name' attribute.");

			// Go to next element
			stream.readNext();
		}
	}

	// Succeeded without error?
	if (stream.hasError())
	{
		stream.clear();
		msg.print("Error occurred at end of journal index parsing.");
		return false;
	}

	stream.clear();
	return true;
}

// Parse journal data from supplied QByteArray
bool ISIS::parseJournalData(Journal* jrnl, QByteArray& data, bool updateOnly, bool forceISOEncoding)
{
	// Check pointer and grab the RunData array
	if (jrnl == NULL)
	{
		msg.print("Internal Error: NULL Journal pointer given to ISIS::parseJournalData().\n");
		return false;
	}
	Instrument* inst = jrnl->parent();
	List<RunData>& targetList = jrnl->runData();
	
	RunData* rd, dummyData, *currentSearchPoint = targetList.first(), *oldrd;
	RunProperty::Property prop;
	bool userExperiment = false;

	if (forceISOEncoding) data.replace("UTF-8", "iso-8859-1");

	// Clear list if not updating it
	if (!updateOnly) targetList.clear();

	// Create a stream reader
	QXmlStreamReader stream(data);

	// Parse XML until end of stream
	while  (!stream.atEnd() && !stream.hasError())
	{
		// Read next element - skip if StartDocument
		QXmlStreamReader::TokenType token = stream.readNext();
		if (token == QXmlStreamReader::StartDocument) continue;

		// If the token is StartElement, is it NXroot?
		if ((token == QXmlStreamReader::StartElement) && (stream.name() == "NXroot"))
		{
			//msg.print("--> Found NXroot.");
			continue;
		}

		// If it is a journal entry ('NXentry') then pull data from it
		if (stream.name() == "NXentry")
		{
			// Add a new data entry, and set target instrument
			rd = new RunData;
			if (inst->shortName() == "LOCAL") userExperiment = true;
			rd->setInstrument(inst);
			rd->setJournalSource(jrnl);

			// Grab NXentry attributes (should just be 'name')
			QXmlStreamAttributes attributes = stream.attributes();
			if (attributes.hasAttribute("name")) rd->setName(attributes.value("name").toString());
			else msg.print("Warning - NXentry has no 'name' attribute");

			// Go to next element
			stream.readNext();

			// Loop over xml entries until we find the element end or the start of another NXentry
			while (!(stream.tokenType() == QXmlStreamReader::EndElement && stream.name() == "NXentry"))
			{
				if (stream.tokenType() == QXmlStreamReader::StartElement)
				{
					// Extract run data
					prop = RunProperty::propertyFromNX(qPrintable(stream.name().toString()));
					
					// Skip to actual element data and search for properties of interest
					stream.readNext();

					switch (prop)
					{
						case (RunProperty::Cycle):
							rd->setCycle(cycleIndex(stream.text().toString()));
							break;
						case (RunProperty::Duration):
							rd->setDuration(stream.text().toInt());
							break;
						case (RunProperty::EndTimeAndDate):
							rd->setEndDateTime(stream.text().toString());
							break;
						case (RunProperty::InstrumentName):
							if (inst->instrument() != ISIS::LOCAL) continue;
							rd->setInstrument(parent_->instrument(ISIS::instrument(stream.text().toString())));
							break;
						case (RunProperty::Name):
							rd->setName(stream.text().toString());
							break;
						case (RunProperty::ProtonCharge):
							rd->setProtonCharge(stream.text().toDouble());
							break;
						case (RunProperty::RBNumber):
							rd->setRBNumber(stream.text().toInt());
							break;
						case (RunProperty::RunNumber):
							rd->setRunNumber(stream.text().toInt());
							// Now we know the run number, if we are only updating we search the current list to see if it exists already.
							// If it does, we'll ditch this 'rd' and redirect to the dummy one
							if (updateOnly)
							{
								// Since the run numbers we encounter are highliy likely to be in sequential order, we will do the search in two parts.
								// 1) From 'currentSearchPoint' to the end of the list
								// 2) From the beginning of the list to 'currentSearchPoint'
								for (oldrd = currentSearchPoint; oldrd != NULL; oldrd = oldrd->next) if (oldrd->runNumber() == rd->runNumber()) break;
								if (oldrd == NULL)
								{
									// Second part of search
									for (oldrd = targetList.first(); oldrd != currentSearchPoint; oldrd = oldrd->next) if (oldrd->runNumber() == rd->runNumber()) break;
								}
								// Did we find a match?
								if (oldrd)
								{
									delete rd;
									rd = &dummyData;
									currentSearchPoint = oldrd->next;
								}
								else targetList.own(rd);
							}
							else targetList.own(rd);
							break;
						case (RunProperty::StartTimeAndDate):
							rd->setStartDateTime(stream.text().toString());
							break;
						case (RunProperty::Title):
							rd->setTitle(stream.text().toString());
							break;
						case (RunProperty::TotalMEvents):
							rd->setTotalMEvents(stream.text().toDouble());
							break;
						case (RunProperty::User):
							rd->setUser(stream.text().toString());
							break;
					}
					
					// Move to next entry
					stream.readNext();
				}
				// Move to next entry
				stream.readNext();
			}
		}
	}

	// Succeeded without error?
	if (stream.hasError())
	{
		stream.clear();
		msg.print("Error occurred at end of journal data.");
		return false;
	}

	stream.clear();
	return true;
}

// Parser instrument information (blocks etc.)
bool ISIS::parseInstrumentInformation(Instrument* inst, QByteArray& data)
{
	// Clear existing instrument block information
	inst->clearBlocks();

	// Create a stream reader
	QXmlStreamReader stream(data);

	// Temporary strings, and regexps for block group
	QString text, name, value, group, setPoint, units;
	QRegularExpression valueRE("([\\S]+)\\s*(\\S*).*"), setpointRE("\\[Setpoint = (.*)\\]"), groupRE("<Group=(.*)>");
	QRegularExpressionMatch groupMatch, setpointMatch, valueMatch;

	// Parse XML until end of file
	while(!stream.atEnd() && !stream.hasError())
	{
		// Read next element - skip if StartDocument
		QXmlStreamReader::TokenType token = stream.readNext();
		if (token == QXmlStreamReader::StartDocument) continue;

		// If the token is StartElement, is it NXroot?
		if ((token == QXmlStreamReader::StartElement) && (stream.name() == "SeciData"))
		{
			msg.print("--> Found SeciData");
			continue;
		}

		// If it is a 'Field' entry then pull data from it
		if (stream.name() == "Field")
		{
			// Each 'Field' has two elements:
			// -- Name: The name of the field
			// -- Value: The value of the field, including optional units, setpoint, and Block Group (which we want) surrounded by angle brackets (as &lt; and &gt; in the file)
			
			// Go to next element
			stream.readNext();

			// Loop over xml entries until we find the element end or the start of another 'Field'
			while (!(stream.tokenType() == QXmlStreamReader::EndElement && stream.name() == "Field"))
			{
				// Start element found?
				if (stream.tokenType() != QXmlStreamReader::StartElement)
				{
					stream.readNext();
					continue;
				}
				
				// What have we found - 'Name' or 'Value'?
				if (stream.name() == "Name")
				{
					stream.readNext();
					name = stream.text().toString();
				}
				else if (stream.name() == "Value")
				{
					stream.readNext();
					text = stream.text().toString();

					// Try to extract Group information...
					groupMatch = groupRE.match(text);
					if (groupMatch.hasMatch())
					{
						// Take capture, and remove (replace) match
						group = groupMatch.captured(1);
						text.replace(groupMatch.captured(0), "");
					}
					else group = "General";

					// Try to extract setpoint information...
					setpointMatch = setpointRE.match(text);
					if (setpointMatch.hasMatch())
					{
						// Take capture, and remove (replace) match
						setPoint = setpointMatch.captured(1);
						text.replace(setpointMatch.captured(0), "");
					}
					else setPoint = "";

					// Get value and units (if present)
					valueMatch = valueRE.match(text);
					if (valueMatch.hasMatch()) value = valueMatch.captured(1);
					else msg.print("Error extracting value from SeciData '%s'.", qPrintable(text));
					units = valueMatch.captured(2);

					// Add the block value to the instrument
					inst->addBlock(group, name, value, units);
				}

				// Move to next entry
				stream.readNext();
			}
		}
	}

	// Succeeded without error?
	if (stream.hasError())
	{
		stream.clear();
		msg.print("Error occurred at end of instrument information.");
		return false;
	}

	stream.clear();
	return true;
}

// Parse log information (block data etc.)
bool ISIS::parseLogInformation(RunData* runData, QByteArray& data)
{
	// Create a stream reader
	QTextStream stream(data);
	QString line, tempDateTime, tempValue, block;
	double value;
	QDateTime dateTime;

	// Parse until end of stream
	while (!stream.atEnd())
	{
		// Read next line - time/data, followed by block name, followed by value
		line = stream.readLine();
		QTextStream textStream(&line);
		textStream >> tempDateTime;
		textStream >> block;
		if (block.length() == 0)
		{
			msg.print("Warning - No block defined at time %s in logfile.\n", qPrintable(tempDateTime));
			continue;
		}
		textStream >> tempValue;
		if (tempValue.length() == 0)
		{
			msg.print("Warning - Block %s at time %s in logfile has no value.\n", qPrintable(block), qPrintable(tempDateTime));
			continue;
		}
		
		// Convert temp string into proper QDateTime
		dateTime = QDateTime::fromString(tempDateTime, "yyyy-MM-ddTHH:mm:ss");
		
		// Add datapoint to RunData
		bool isNumber;
		value = tempValue.toDouble(&isNumber);
		if (isNumber) runData->addBlockDataValue(block, dateTime, value);
		else runData->addBlockDataValue(block, dateTime, tempValue);
	}

	return true;
}

#ifndef NOHDF
// Parse log information from Nexus file
bool ISIS::parseNexusFile(RunData* runData, QString fileName)
{
	// Open HDF5 file
	hid_t file = H5Fopen(qPrintable(fileName), H5F_ACC_RDONLY, H5P_DEFAULT);

	// Get start/end times
	QString tempString;
	if (nexusExtractString(file, "/raw_data_1/start_time", tempString)) startTime_ = QDateTime::fromString(tempString, "yyyy-MM-ddTHH:mm:ss");
	else
	{
		msg.print("Warning - Failed to get start time from NEXUS file. Using value from RunData instead.\n");
		startTime_ = runData->startDateTime();
	}
	if (nexusExtractString(file, "/raw_data_1/end_time", tempString)) endTime_ = QDateTime::fromString(tempString, "yyyy-MM-ddTHH:mm:ss");
	else
	{
		msg.print("Warning - Failed to get end time from NEXUS file. Using value from RunData instead.\n");
		endTime_ = runData->endDateTime();
	}
	
	// Specify path to sample environment group and open it
	hid_t selog = H5Gopen1(file, "/raw_data_1/selog");
	if (selog > 0)
	{
		// Iterate over items in SE log using H5Literate - HORRIBLE WAY TO DO THINGS!
		H5Literate(selog, H5_INDEX_NAME, H5_ITER_NATIVE, NULL, &ISIS::nexusGroupIterator, runData);
		H5Gclose(selog);
	}

	// Specify path to run data group and open it
	hid_t runlog = H5Gopen1(file, "/raw_data_1/runlog");
	if (runlog > 0)
	{
		// Iterate over items in run log using H5Literate - DITTO!
		H5Literate(runlog, H5_INDEX_NAME, H5_ITER_NATIVE, NULL, &ISIS::nexusGroupIterator, runData);
		H5Gclose(runlog);
	}

	// Try to extract some specific values...
	if (nexusExtractString(file, "/raw_data_1/seci_config", tempString)) runData->addSingleValue("Instrument", "SECI Config", tempString);
	if (nexusExtractString(file, "/raw_data_1/instrument/dae/detector_table_file", tempString)) runData->addSingleValue("Instrument", "Detector Table File", tempString);
	if (nexusExtractString(file, "/raw_data_1/instrument/dae/spectra_table_file", tempString)) runData->addSingleValue("Instrument", "Spectra Table File", tempString);
	if (nexusExtractString(file, "/raw_data_1/instrument/dae/wiring_table_file", tempString)) runData->addSingleValue("Instrument", "Wiring Table File", tempString);

	H5Fclose(file);

	return true;
}

// Iterator callback for HDF5 (group access)
herr_t ISIS::nexusGroupIterator(hid_t loc_id, const char *name, const H5L_info_t *info, void *operator_data)
{
	// Get type of object - if it is not a Group then I don't care
	H5O_info_t infobuf;
	herr_t status = H5Oget_info_by_name(loc_id, name, &infobuf, H5P_DEFAULT);
	if (infobuf.type == H5O_TYPE_GROUP)
	{
		// It's a group, which means we want to extract some useful data from it.
		// Each group potentially has within it several control variables etc., and a 'value_log' group containing the time/value data
		RunData* rd = (RunData*) operator_data;
		currentBlock_ = name;
		currentGroup_ = (rd == NULL ? "No Group" : (rd->instrument() ? rd->instrument()->groupForBlock(currentBlock_) : "No Group"));
		timeDataSet_ = -1;
		valueDataSet_ = -1;
		hid_t block = H5Gopen2(loc_id, name, H5P_DEFAULT);
		H5Literate(block, H5_INDEX_NAME, H5_ITER_NATIVE, NULL, &ISIS::nexusBlockIterator, operator_data);
	}

	return 0;
}

// Iterator for block data
herr_t ISIS::nexusBlockIterator(hid_t loc_id, const char *name, const H5L_info_t *info, void *operator_data)
{
	H5O_info_t infobuf;
	herr_t status = H5Oget_info_by_name(loc_id, name, &infobuf, H5P_DEFAULT);
	if (infobuf.type == H5O_TYPE_GROUP)
	{
		if (strcmp("value_log",name) == 0)
		{
			// Found the value log!
			hid_t valueLog = H5Gopen2(loc_id, "value_log", H5P_DEFAULT);
			if (valueLog < 0) msg.print("Error opening value_log for block %s.\n", qPrintable(currentBlock_));
			else
			{
				// Get time/value identifiers for this group
				hid_t time = H5Dopen2(valueLog, "time", H5P_DEFAULT);
				if (time < 0)
				{
					msg.print("Warning - value_log for NEXUS group '%s' did not contain a 'time' dataset.\n", qPrintable(currentBlock_));
					return -1;
				}
				hid_t value = H5Dopen2(valueLog, "value", H5P_DEFAULT);
				if (value < 0)
				{
					msg.print("Warning - value_log for NEXUS group '%s' did not contain a 'value' dataset.\n", qPrintable(currentBlock_));
					return -1;
				}

				// Get data
				nexusExtractTimeValueData((RunData*) operator_data, time, value);
				
				// Cleanup
				H5Dclose(time);
				H5Dclose(value);
			}
		}
	}
	else if (infobuf.type == H5O_TYPE_DATASET)
	{
		// Some blocks (especially those in the 'runlog' group) don't have time/value datasets in a value_log subgroup.
		// Check here to see if we get both for a given blockName
		if (strcmp("time", name) == 0) timeDataSet_ = loc_id;
		else if (strcmp("value", name) == 0) valueDataSet_= loc_id;

		// Do we now have both time and value datasets?
		if ((timeDataSet_ > 0) && (valueDataSet_ > 0))
		{
			hid_t time = H5Dopen2(timeDataSet_, "time", H5P_DEFAULT);
			hid_t value= H5Dopen2(valueDataSet_, "value", H5P_DEFAULT);
			nexusExtractTimeValueData((RunData*) operator_data, time, value);
			timeDataSet_ = -1;
			valueDataSet_ = -1;
			
			// Cleanup
			H5Dclose(time);
			H5Dclose(value);
		}
	}
	return 0;
}

// Retrieve double from specified Nexus dataset
bool ISIS::nexusExtractDouble(hid_t rootLocation, const char* name, double& dest)
{
	// Try to check existence of dataset in a 'nice' way first....
	H5O_info_t objectInfo;
	if (H5Oget_info_by_name(rootLocation, name, &objectInfo, H5P_DEFAULT) < 0) return false;

	hid_t dataSet = H5Dopen1(rootLocation, name);
	if (dataSet < 0)
	{
		msg.print("Warning - Failed to open '%s' dataset in nexus file.\n", name);
		return false;
	}

	// Get the dataspaces for each dataset
	hid_t space = H5Dget_space(dataSet);

	// Get data properties
	int nDims = H5Sget_simple_extent_ndims(space);
	hsize_t* nValues = new hsize_t[nDims];
	H5Sget_simple_extent_dims(space, nValues, NULL);

	// Make some checks...
	if (nDims != 1)
	{
		msg.print("Warning - Tried to extract a single value from a multi-arrayed dataset '%s'.\n", name);
		delete[] nValues;
		H5Dclose(dataSet);
		return false;
	}
	if (nValues[0] > 1)
	{
		msg.print("Warning - Tried to extract a single value from a multi-valued dataset '%s'.\n", name);
		delete[] nValues;
		H5Dclose(dataSet);
		return false;
	}
	delete[] nValues;

	// Check type of value data
	hid_t valueType = H5Dget_type(dataSet);
	int intBuffer;
	char charBuffer[256];
	herr_t status;
	hid_t memType, charSpace;
	switch (H5Tget_class(valueType))
	{
		case (H5T_INTEGER):
			status = H5Dread(dataSet, H5T_NATIVE_INT_g, H5S_ALL, H5S_ALL, H5P_DEFAULT, &intBuffer);
			dest = intBuffer;
			break;
		case (H5T_FLOAT):
			status = H5Dread(dataSet, H5T_NATIVE_DOUBLE_g, H5S_ALL, H5S_ALL, H5P_DEFAULT, &dest);
			break;
		case (H5T_STRING):
			memType = H5Tcopy(H5T_C_S1);
			status = H5Tset_size(memType, 256);
			status = H5Dread(dataSet, memType, H5S_ALL, H5S_ALL, H5P_DEFAULT, &charBuffer);
			dest = QString(charBuffer).toDouble();
			break;
	}

	// Cleanup
	H5Sclose(space);
	
	return true;
}

// Retrieve integer from specified Nexus dataset
bool ISIS::nexusExtractInteger(hid_t rootLocation, const char* name, int& dest)
{
	// Try to check existence of dataset in a 'nice' way first....
	H5O_info_t objectInfo;
	if (H5Oget_info_by_name(rootLocation, name, &objectInfo, H5P_DEFAULT) < 0) return false;

	hid_t dataSet = H5Dopen1(rootLocation, name);
	if (dataSet < 0)
	{
		msg.print("Warning - Failed to open '%s' dataset in nexus file.\n", name);
		return false;
	}

	// Get the dataspaces for each dataset
	hid_t space = H5Dget_space(dataSet);

	// Get data properties
	int nDims = H5Sget_simple_extent_ndims(space);
	hsize_t* nValues = new hsize_t[nDims];
	H5Sget_simple_extent_dims(space, nValues, NULL);

	// Make some checks...
	if (nDims != 1)
	{
		msg.print("Warning - Tried to extract a single value from a multi-arrayed dataset '%s'.\n", name);
		delete[] nValues;
		H5Dclose(dataSet);
		return false;
	}
	if (nValues[0] > 1)
	{
		msg.print("Warning - Tried to extract a single value from a multi-valued dataset '%s'.\n", name);
		delete[] nValues;
		H5Dclose(dataSet);
		return false;
	}
	delete[] nValues;

	// Check type of value data
	hid_t valueType = H5Dget_type(dataSet);
	int intBuffer;
	double doubleBuffer;
	char charBuffer[256];
	herr_t status;
	hid_t memType, charSpace;
	switch (H5Tget_class(valueType))
	{
		case (H5T_INTEGER):
			status = H5Dread(dataSet, H5T_NATIVE_INT_g, H5S_ALL, H5S_ALL, H5P_DEFAULT, &dest);
			break;
		case (H5T_FLOAT):
			status = H5Dread(dataSet, H5T_NATIVE_DOUBLE_g, H5S_ALL, H5S_ALL, H5P_DEFAULT, &doubleBuffer);
			dest = doubleBuffer;
			break;
		case (H5T_STRING):
			memType = H5Tcopy(H5T_C_S1);
			status = H5Tset_size(memType, 256);
			status = H5Dread(dataSet, memType, H5S_ALL, H5S_ALL, H5P_DEFAULT, &charBuffer);
			dest = QString(charBuffer).toInt();
			break;
	}

	// Cleanup
	H5Sclose(space);
	
	return true;
}

// Retrieve string from specified Nexut dataset
bool ISIS::nexusExtractString(hid_t rootLocation, const char* name, QString& dest)
{
	// Try to check existence of dataset in a 'nice' way first....
	H5O_info_t objectInfo;
	if (H5Oget_info_by_name(rootLocation, name, &objectInfo, H5P_DEFAULT) < 0) return false;

	hid_t dataSet = H5Dopen1(rootLocation, name);
	if (dataSet < 0)
	{
		msg.print("Warning - Failed to open '%s' dataset in nexus file.\n", name);
		return false;
	}

	// Get the dataspaces for each dataset
	hid_t space = H5Dget_space(dataSet);

	// Get data properties
	int nDims = H5Sget_simple_extent_ndims(space);
	hsize_t* nValues = new hsize_t[nDims];
	H5Sget_simple_extent_dims(space, nValues, NULL);

	// Make some checks...
	if (nDims != 1)
	{
		msg.print("Warning - Tried to extract a single value from a multi-arrayed dataset '%s'.\n", name);
		delete[] nValues;
		H5Dclose(dataSet);
		return false;
	}
	if (nValues[0] > 1)
	{
		msg.print("Warning - Tried to extract a single value from a multi-valued dataset '%s'.\n", name);
		delete[] nValues;
		H5Dclose(dataSet);
		return false;
	}

	// Check type of value data
	hid_t valueType = H5Dget_type(dataSet);
	int intBuffer;
	double doubleBuffer;
	char charBuffer[256];
	herr_t status;
	hid_t memType, charSpace;
	switch (H5Tget_class(valueType))
	{
		case (H5T_INTEGER):
			status = H5Dread(dataSet, H5T_NATIVE_INT_g, H5S_ALL, H5S_ALL, H5P_DEFAULT, &intBuffer);
			dest = QString::number(intBuffer);
			break;
		case (H5T_FLOAT):
			status = H5Dread(dataSet, H5T_NATIVE_DOUBLE_g, H5S_ALL, H5S_ALL, H5P_DEFAULT, &doubleBuffer);
			dest = QString::number(doubleBuffer);
			break;
		case (H5T_STRING):
			memType = H5Tcopy(H5T_C_S1);
			status = H5Tset_size(memType, 256);
			status = H5Dread(dataSet, memType, H5S_ALL, H5S_ALL, H5P_DEFAULT, &charBuffer);
			dest = charBuffer;
			break;
	}

	// Cleanup
	delete[] nValues;
	H5Sclose(space);
	
	return true;
}

// Retrieve time/value data from specified NEXUS group
herr_t ISIS::nexusExtractTimeValueData(RunData* runData, hid_t time, hid_t value)
{
	// Get the dataspaces for each dataset
	hid_t timeSpace = H5Dget_space(time);
	hid_t valueSpace = H5Dget_space(value);

	// Get data properties
	int timeNDims = H5Sget_simple_extent_ndims(timeSpace);
	int valueNDims = H5Sget_simple_extent_ndims(valueSpace);
	hsize_t* nTime = new hsize_t[timeNDims];
	hsize_t* nValue = new hsize_t[valueNDims];
	H5Sget_simple_extent_dims(timeSpace, nTime, NULL);
	H5Sget_simple_extent_dims(valueSpace, nValue, NULL);

	// Get type (and size) of value data
	hid_t valueType = H5Dget_type(value);
	int valueSize = H5Tget_size(valueType);

	// Make some checks...
	if (timeNDims != 1)
	{
		msg.print("Error - time array in value_log for '%s' does not consist of exactly one dimension.\n", qPrintable(currentBlock_));
		H5Sclose(timeSpace);
		H5Sclose(valueSpace);
		return -1;
	}
	if (valueNDims != 1)
	{
		// Might be ok, provided it's of string type....
		if ((H5Tget_class(valueType) == H5T_STRING) && (valueNDims == 2)) msg.print("Found string array with dimension 2.\n");
		else
		{
			msg.print("Warning - value array in the value_log for '%s' does not consist of exactly one dimension (or two if of type H5T_STRING).\n", qPrintable(currentBlock_));
			H5Sclose(timeSpace);
			H5Sclose(valueSpace);
			return -1;
		}
	}

	if (nTime[0] != nValue[0])
	{
		msg.print("Warning - Array sizes of time/value domains do not match (%i / %i).\n", nTime[0], nValue[0]);
		H5Sclose(timeSpace);
		H5Sclose(valueSpace);
		return -1;
	}

	// Create temporary arrays to store time data, and retrieve it
	double* timeData = new double[(long int) nTime[0]];
	herr_t status = H5Dread(time, H5T_NATIVE_DOUBLE_g, H5S_ALL, H5S_ALL, H5P_DEFAULT, timeData);

	// Read in data
	int* intBuffer;
	double* doubleBuffer;
	char** charBuffer;
	hid_t memType, charSpace;
	Data2D* data = runData->addBlockData(currentBlock_, currentGroup_, startTime_, endTime_);
	switch (H5Tget_class(valueType))
	{
		case (H5T_INTEGER):
			intBuffer = new int[(long int) nValue[0]];
			status = H5Dread(value, H5T_NATIVE_INT_g, H5S_ALL, H5S_ALL, H5P_DEFAULT, intBuffer);
			for (int n=0; n<nValue[0]; ++n) data->addRelativePoint(startTime_.addSecs(timeData[n]), (double) intBuffer[n]);
			delete[] intBuffer;
			break;
		case (H5T_FLOAT):
			doubleBuffer = new double[(long int) nValue[0]];
			status = H5Dread(value, H5T_NATIVE_DOUBLE_g, H5S_ALL, H5S_ALL, H5P_DEFAULT, doubleBuffer);
			for (int n=0; n<nValue[0]; ++n) data->addRelativePoint(startTime_.addSecs(timeData[n]), doubleBuffer[n]);
			delete[] doubleBuffer;
			break;
		case (H5T_STRING):
			// Make room for null terminators of strings
			++valueSize;
			charBuffer = new char*[(long int) nValue[0]];
// 			for (int n=0; n<(long int) nValue[0]; ++n) charBuffer[n] = new char[valueSize];
			charBuffer[0] = new char[(long int) nValue[0] * valueSize];
			for (int n=1; n<(long int) nValue[0]; ++n) charBuffer[n] = charBuffer[n-1] + valueSize;
// 			charSpace = H5Dget_space(value);
			memType = H5Tcopy(H5T_C_S1);
			status = H5Tset_size(memType, valueSize);
			status = H5Dread(value, memType, H5S_ALL, H5S_ALL, H5P_DEFAULT, charBuffer[0]);
			for (int n=0; n<nValue[0]; ++n) data->addRelativePoint(startTime_.addSecs(timeData[n]), runData->enumeratedBlockValue(currentBlock_, QString(charBuffer[n])));
// 			for (int n=0; n<(long int) nValue[0]; ++n) delete charBuffer[n];
			delete[] charBuffer[0];
			delete[] charBuffer;
			break;
	}
	
	// Cleanup
	delete[] nTime;
	delete[] nValue;
	H5Sclose(timeSpace);
	H5Sclose(valueSpace);

	return 0;
}

#endif
