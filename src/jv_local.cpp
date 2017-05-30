/*
	*** JournalViewer Local Journal Functions
	*** src/jv_local.cpp
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

#include "jv.h"
#include "messenger.hui"
#include "datainterface.h"
#include "get/interface.h"
#include <QMessageBox>
#include <QSettings>

// Update local journals
void JournalViewer::updateLocalJournals()
{
	// First, check user data directory specified to see if it exists...
	if (!userDataDirectory_.exists())
	{
		QMessageBox::critical(this, "Error", "User data directory '" + userDataDirectory_.absolutePath() + "' does not exist.\nCheck the directory defined in Settings.");
		return;
	}

	// Open the journal file ready for writing
	QString indexFileName = userDataDirectory_.filePath("local_main.xml");
	QFile indexFile(indexFileName);
	if (!indexFile.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QMessageBox::critical(this, "Error", "Could not open the root 'local_main.xml' journal file for writing.\nCheck the directory defined in Settings and your permissions to write to it.");
		return;
	}

	// Create an xml writer, and write basic information to journal file
	QXmlStreamWriter indexXml(&indexFile);
	indexXml.writeStartDocument();
	indexXml.writeStartElement("journal");

	// First step - loop over directories in userDataDirectory_
	QStringList dirs = userDataDirectory_.entryList(QDir::AllDirs| QDir::NoDotAndDotDot);
	for (int n=0; n<dirs.size(); ++n)
	{
		indexXml.writeStartElement("file");
		indexXml.writeAttribute("name", dirs[n] + ".xml");
		indexXml.writeEndElement();
	}
	// -- Add on root of the user data directory
	indexXml.writeStartElement("file");
	indexXml.writeAttribute("name", "Top.xml");
	indexXml.writeEndElement();
	dirs << ".";

	indexXml.writeEndElement();
	indexXml.writeEndDocument();
	indexFile.close();

	// Write modtime into settings
	QSettings settings;
	settings.setValue(QString("modtime/")+indexFileName, QDateTime::currentDateTime());

	// Now, for each of the directories, construct a local journal file for it
	for (int n=0; n<dirs.size(); ++n)
	{
		// Get list of all raw and nxs files in the directory, and create a list of files to work from
		QDir experimentDir = userDataDirectory_.absoluteFilePath(dirs[n]);
		QFileInfoList rawNxsFiles = experimentDir.entryInfoList(QStringList() << "*.raw" << "*.nxs", QDir::Files, QDir::Name);
		QFileInfoList baseFiles;
		QFileInfo lastFileInfo;
		for (int i = 0; i<rawNxsFiles.size(); ++i)
		{
			// If it is different to the last, then store the last one before overwriting it
			// If it is the same as the last, overwrite the l...
			if (rawNxsFiles[i].completeBaseName() != lastFileInfo.completeBaseName())
			{
				baseFiles << rawNxsFiles[i];
				lastFileInfo = rawNxsFiles[i];
			}
		}

		// Open a journal file, create an xml writer, and write basic information to journal file
		QString journalFileName = userDataDirectory_.absoluteFilePath(dirs[n] == "." ? "Top.xml" : dirs[n] + ".xml");
		QFile journalFile(journalFileName);
		if (!journalFile.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			QMessageBox::warning(this, "Warning", "Could not write the journal file '" + journalFileName + "'.\nCheck the directory defined in Settings and your permissions to write to it.");
			continue;
		}
		QXmlStreamWriter journalXml(&journalFile);
		journalXml.writeStartDocument();
		journalXml.writeStartElement("NXroot");

		for (int i = 0; i < baseFiles.size(); ++i)
		{
			// Begin NXentry for this file
			journalXml.writeStartElement("NXentry");
			journalXml.writeAttribute("name", baseFiles[i].baseName());

			// If the nexus file is available, use that for info.
			// Otherwise, get as much from the raw file as we can
			if (baseFiles[i].suffix().toLower() == "nxs")
			{
				// Open HDF5 file
				hid_t file = H5Fopen(qPrintable(baseFiles[i].absoluteFilePath()), H5F_ACC_RDONLY, H5P_DEFAULT);

				// Extract information...
				QString tempString;
				int tempInt;
				double tempDouble;
				if (ISIS::nexusExtractString(file, "/raw_data_1/beamline", tempString)) journalXml.writeTextElement("instrument_name", tempString);
				else msg.print("Warning - Failed to get start time from NEXUS file.\n");
				if (ISIS::nexusExtractString(file, "/raw_data_1/title", tempString)) journalXml.writeTextElement("title", tempString);
				else msg.print("Warning - Failed to get run title from NEXUS file.\n");
				if (ISIS::nexusExtractString(file, "/raw_data_1/user_1/name", tempString)) journalXml.writeTextElement("user_name", tempString);
				else msg.print("Warning - Failed to get user name from NEXUS file.\n");
				if (ISIS::nexusExtractString(file, "/raw_data_1/experiment_identifier", tempString)) journalXml.writeTextElement("experiment_identifier", tempString);
				else msg.print("Warning - Failed to get experiment identifier from NEXUS file.\n");
				
				if (ISIS::nexusExtractString(file, "/raw_data_1/good_frames", tempString)) journalXml.writeTextElement("good_frames", tempString);
				else msg.print("Warning - Failed to get start time from NEXUS file.\n");
				if (ISIS::nexusExtractInteger(file, "/raw_data_1/raw_frames", tempInt)) journalXml.writeTextElement("raw_frames", QString::number(tempInt));
				else msg.print("Warning - Failed to get start time from NEXUS file.\n");
				if (ISIS::nexusExtractInteger(file, "/raw_data_1/run_number", tempInt)) journalXml.writeTextElement("run_number", QString::number(tempInt));
				else msg.print("Warning - Failed to get run number from NEXUS file.\n");
				if (ISIS::nexusExtractDouble(file, "/raw_data_1/duration", tempDouble)) journalXml.writeTextElement("duration", QString::number(tempDouble));
				else msg.print("Warning - Failed to get duration from NEXUS file.\n");
				if (ISIS::nexusExtractDouble(file, "/raw_data_1/proton_charge", tempDouble)) journalXml.writeTextElement("proton_charge", QString::number(tempDouble));
				else msg.print("Warning - Failed to get proton charge from NEXUS file.\n");

				if (ISIS::nexusExtractString(file, "/raw_data_1/start_time", tempString)) journalXml.writeTextElement("start_time", tempString);
				else msg.print("Warning - Failed to get start time from NEXUS file.\n");
				if (ISIS::nexusExtractString(file, "/raw_data_1/end_time", tempString)) journalXml.writeTextElement("end_time", tempString);
				else msg.print("Warning - Failed to get end time from NEXUS file.\n");

				H5Fclose(file);
			}
			else
			{
				// Open RAW file
				GetInterface rawFile(qPrintable(baseFiles[i].absoluteFilePath()));

				// Grab header
				QString header = rawFile.getCharParameter(GetInterface::HDR);

				// Write information...
				journalXml.writeTextElement("title", rawFile.getCharParameter(GetInterface::TITL));
				journalXml.writeTextElement("user_name", header.mid(8,20));
				journalXml.writeTextElement("experiment_identifier", QString::number(rawFile.runParameterBlock()[21]));
				journalXml.writeTextElement("instrument_name", QString(rawFile.getCharParameter(GetInterface::NAME)).simplified());
				journalXml.writeTextElement("good_frames", QString::number(rawFile.runParameterBlock()[9]));
				journalXml.writeTextElement("raw_frames", QString::number(rawFile.runParameterBlock()[10]));
				journalXml.writeTextElement("run_number", QString::number(rawFile.getIntegerParameter(GetInterface::RUN)));
				journalXml.writeTextElement("duration", QString::number(rawFile.runParameterBlock()[12]));
				QDateTime dateTime;
				dateTime.setDate(QDate::fromString(header.mid(52,12), "dd-MMM-yyyy "));
				dateTime.setTime(QTime::fromString(header.mid(64,8), "HH:mm:ss"));
				journalXml.writeTextElement("start_time", dateTime.toString("yyyy-MM-ddTHH:mm:ss"));
				dateTime.setDate(QDate::fromString(GetInterface::intToChar(&rawFile.runParameterBlock()[16], 3), "dd-MMM-yyyy "));
				dateTime.setTime(QTime::fromString(GetInterface::intToChar(&rawFile.runParameterBlock()[19], 2), "HH:mm:ss"));
				journalXml.writeTextElement("end_time", dateTime.toString("yyyy-MM-ddTHH:mm:ss"));
				journalXml.writeTextElement("proton_charge", QString::number(GetInterface::intToDouble(rawFile.runParameterBlock()[7])));
// 				"total_mevents"??
			}

			// End NXentry
			journalXml.writeEndElement();
		}

		journalXml.writeEndElement();
		journalXml.writeEndDocument();
		journalFile.close();

		// Write modtime into settings
		QSettings settings;
		settings.setValue(QString("modtime/")+journalFileName, QDateTime::currentDateTime());
	}
}