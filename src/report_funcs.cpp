/*
	*** ReportGenerator Functinos
	*** src/report_funcs.cpp
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

#include "report.h"
#include "jv.h"
#include "datainterface.h"
#include "document.h"
#include "ttreewidgetitem.h"
#include "plotwidget.hui"
#include "messenger.hui"
#include <QtSvg/QSvgGenerator>
#include <QString>
#include <QSettings>
#include <QInputDialog>
#include <QProgressDialog>

// Constructor
ReportGenerator::ReportGenerator(JournalViewer* parent, const QList<int>& availableRB, QStringList availableRBUsers, RefList<RunData, Journal*>& runData) : QDialog(), runData_(runData)
{
	// Call the main creation function
	ui.setupUi(this);

	parent_ = parent;
	instrument_ = parent_->currentInstrument();
	availableRB_ = availableRB;
	
	// Set initial property availability
	for (int n=0; n<RunProperty::nProperties; ++n) availableProperties_[n] = true;
	
	retrieveSettings();

	refreshing_ = true;

	// Add experiment combo items
	for (int n=0; n<availableRB.count(); ++n) ui.RBCombo->addItem(QString::number(availableRB.at(n)) + "  " + availableRBUsers.at(n));

	refreshing_ = false;
	
	// Add context menu actions
	ui.BlockTree->setContextMenuPolicy(Qt::ActionsContextMenu);
	QMenu* blockTreeMenu = new QMenu(ui.BlockTree);

	QAction* action = new QAction("Set Group", blockTreeMenu);
	ui.BlockTree->addAction(action);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(blockTreeContextMenu_setGroup(bool)));
	action = new QAction("Remove Group", blockTreeMenu);
	ui.BlockTree->addAction(action);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(blockTreeContextMenu_removeGroup(bool)));

	// Update initial experiment details
	updateDetails( ui.RBCombo->currentIndex() == -1 ? -1 : availableRB_.at(ui.RBCombo->currentIndex()) );
	
	// Update initial instrument details
	retrieveInstrumentInformation();
	
	// Update property lists
	updatePropertyLists();
}

// Destructor
ReportGenerator::~ReportGenerator()
{
}

// Store settings
void ReportGenerator::storeSettings()
{
	// Store block settings for instrument
	instrument_->storeBlockSettings();

	// Store Run Property defaults
	QSettings settings;
	settings.remove("ReportProperties");
	settings.beginGroup("ReportProperties");
	int n = 0;
	for (RunProperty* rp = visibleProperties_.first(); rp != NULL; rp = rp->next) settings.setValue(QString::number(n++), RunProperty::property(rp->type()));
	settings.endGroup();

	// Store analysis options
	settings.beginGroup("ReportAnalysis");
	settings.setValue("AnalyseRuns", ui.AnalyseRunsCheck->isChecked());
	settings.setValue("AnalyseBeamCurrent", ui.AnalyseBeamCurrentCheck->isChecked());
	settings.setValue("BeamWarn", ui.BeamRangeWarningCheck->isChecked());
	settings.setValue("BeamOnChange", ui.BeamOnChangeCheck->isChecked());
	settings.setValue("TS1BeamCurrentMin", ui.TS1BeamMinimumSpin->value());
	settings.setValue("TS1BeamCurrentMax", ui.TS1BeamMaximumSpin->value());
	settings.setValue("TS2BeamCurrentMin", ui.TS2BeamMinimumSpin->value());
	settings.setValue("TS2BeamCurrentMax", ui.TS2BeamMaximumSpin->value());
	settings.setValue("AnalyseH2Moderator", ui.AnalyseHModeratorCheck->isChecked());
	settings.setValue("ModeratorH2Warn", ui.ModeratorH2RangeWarningCheck->isChecked());
	settings.setValue("ModeratorH2OnChange", ui.ModeratorH2OnChangeCheck->isChecked());
	settings.setValue("ModeratorH2Min", ui.ModeratorH2MinimumSpin->value());
	settings.setValue("ModeratorH2Max", ui.ModeratorH2MaximumSpin->value());
	settings.setValue("AnalyseCH4Moderator", ui.AnalyseCH4ModeratorCheck->isChecked());
	settings.setValue("ModeratorCH4Warn", ui.ModeratorCH4RangeWarningCheck->isChecked());
	settings.setValue("ModeratorCH4OnChange", ui.ModeratorCH4OnChangeCheck->isChecked());
	settings.setValue("ModeratorCH4Min", ui.ModeratorCH4MinimumSpin->value());
	settings.setValue("ModeratorCH4Max", ui.ModeratorCH4MaximumSpin->value());
	settings.endGroup();

	// Store instrument block/group settings
	instrument_->storeBlockSettings();
}

// Retrieve settings
void ReportGenerator::retrieveSettings()
{
	// Probe settings for store visible property information
	QSettings settings;
	settings.beginGroup("ReportProperties");
	int n = 0;
	while (settings.contains(QString::number(n)))
	{
		// Convert the value of this entry to a RunProperty
		RunProperty::Property prop = RunProperty::property(settings.value(QString::number(n)).toString());
		if (prop == RunProperty::nProperties)
		{
			msg.print("Found unknown property '%s' while looking at settings...", qPrintable(settings.value(QString::number(n)).toString()));
			++n;
			continue;
		}

		// Check it hasn't already been added and, if not, add to visible list
		if (!availableProperties_[prop]) msg.print("Property '%s' has already been added to the visible properties list.", qPrintable(RunProperty::property(prop)));
		else
		{
			visibleProperties_.add()->setType(prop);
			availableProperties_[prop] = false;
		}
		
		++n;
	}
	settings.endGroup();
	
	// Analysis
	settings.beginGroup("ReportAnalysis");
	if (settings.contains("AnalyseRuns")) ui.AnalyseRunsCheck->setChecked(settings.value("AnalyseRuns").toBool());
	if (settings.contains("AnalyseBeamCurrent")) ui.AnalyseBeamCurrentCheck->setChecked(settings.value("AnalyseBeamCurrent").toBool());
	if (settings.contains("BeamWarn")) ui.BeamRangeWarningCheck->setChecked(settings.value("BeamWarn").toBool());
	if (settings.contains("BeamOnChange")) ui.BeamOnChangeCheck->setChecked(settings.value("BeamOnChange").toBool());
	if (settings.contains("TS1BeamCurrentMin")) ui.TS1BeamMinimumSpin->setValue(settings.value("TS1BeamCurrentMin").toDouble());
	if (settings.contains("TS1BeamCurrentMax")) ui.TS1BeamMaximumSpin->setValue(settings.value("TS1BeamCurrentMax").toDouble());
	if (settings.contains("AnalyseHModerator")) ui.AnalyseHModeratorCheck->setChecked(settings.value("AnalyseHModerator").toBool());
	if (settings.contains("ModeratorH2Warn")) ui.ModeratorH2RangeWarningCheck->setChecked(settings.value("ModeratorH2Warn").toBool());
	if (settings.contains("ModeratorH2OnChange")) ui.ModeratorH2OnChangeCheck->setChecked(settings.value("ModeratorH2OnChange").toBool());
	if (settings.contains("ModeratorHMin")) ui.ModeratorH2MinimumSpin->setValue(settings.value("ModeratorHMin").toDouble());
	if (settings.contains("ModeratorHMax")) ui.ModeratorH2MaximumSpin->setValue(settings.value("ModeratorHMax").toDouble());
	if (settings.contains("AnalyseCH4Moderator")) ui.AnalyseCH4ModeratorCheck->setChecked(settings.value("AnalyseCH4Moderator").toBool());
	if (settings.contains("ModeratorCH4Warn")) ui.ModeratorH2RangeWarningCheck->setChecked(settings.value("ModeratorCH4Warn").toBool());
	if (settings.contains("ModeratorCH4OnChange")) ui.ModeratorH2OnChangeCheck->setChecked(settings.value("ModeratorCH4OnChange").toBool());
	if (settings.contains("ModeratorCH4Min")) ui.ModeratorCH4MinimumSpin->setValue(settings.value("ModeratorCH4Min").toDouble());
	if (settings.contains("ModeratorCH4Max")) ui.ModeratorCH4MaximumSpin->setValue(settings.value("ModeratorCH4Max").toDouble());
	settings.endGroup();

	// Retrieve instrument block map settings
	instrument_->retrieveBlockMapSettings();
	instrument_->retrieveBlockSelectedSettings();
}

// Return dialog report result
ReportGenerator::Result ReportGenerator::reportResult()
{
	return result_;
}

// Update style preview
void ReportGenerator::updateDetails(int rbNo)
{
	// Determine List of Users, start / end dates etc.
	QList<QString> userNames;
	beginRunDateTime_ = QDateTime::currentDateTime();
	endRunDateTime_.setDate(QDate(1900,1,1));
	nRuns_ = 0;
	totalCurrent_ = 0.0;
	totalRunTime_ = 0;
	rbNumber_ = rbNo;
	
	// Clear combo box in Instrument Information tab, ready for repopulation
	ui.RunInformationCombo->clear();

	refreshing_ = true;
	
	if (rbNo != -1)
	{
		for (RefListItem<RunData,Journal*>* ri = runData_.first(); ri != NULL; ri = ri->next)
		{
			RunData* rd = ri->item;
			if (rd->rbNumber() != rbNumber_) continue;

			if (!userNames.contains(rd->user())) userNames << rd->user();
			if (rd->startDateTime() < beginRunDateTime_) beginRunDateTime_ = rd->startDateTime();
			if (rd->endDateTime() > endRunDateTime_) endRunDateTime_ = rd->endDateTime();
			totalCurrent_ += rd->protonCharge();
			totalRunTime_ += rd->duration();
			ui.RunInformationCombo->addItem(QString::number(rd->runNumber()));
			++nRuns_;
		}
	}

	// Set controls
	ui.UserCombo->clear();
	foreach(QString name, userNames) ui.UserCombo->addItem(name);
	ui.StartDateLabel->setText(beginRunDateTime_.isValid() ? beginRunDateTime_.date().toString() : "<none>");
	ui.EndDateLabel->setText(endRunDateTime_.isValid() ? endRunDateTime_.date().toString() : "<none>");
	ui.NRunsLabel->setText(QString::number(nRuns_));
	ui.RunInformationCombo->setCurrentIndex(0);
	
	refreshing_ = false;
}

// Retrieve new Instrument Information
void ReportGenerator::retrieveInstrumentInformation()
{
	// From where are we getting the block information?
	if (ui.RunInformationRadio->isChecked())
	{
		// Loop over block values in selected RunData
		int runNumber = atoi(qPrintable(ui.RunInformationCombo->currentText()));
		if (runNumber == 0) return;
		RunData* rd = NULL;
		for (RefListItem<RunData,Journal*>* ri = runData_.first(); ri != NULL; ri = ri->next) if (ri->item->runNumber() == runNumber)
		{
			rd = ri->item;
			break;
		}
		if (rd == NULL) return;

		// Do we need to load rundata for this run?
		if (rd->blockData() == NULL)
		{
			// Search for the nexus file for this run
			QString nxsFile;
#ifndef NOHDF
			nxsFile = ISIS::locateFile(rd, "nxs", false, QDir());
#endif
			if (!nxsFile.isEmpty())
			{
				// Load the logfile
				QByteArray data;
#ifndef NOHDF
				if (ISIS::parseNexusFile(rd, nxsFile)) msg.print("Successfully parsed Nexus file " + nxsFile);
				else
				{
					QMessageBox::warning(this, "Couldn't access Nexus file", QString("Error reading Nexus file") + nxsFile);
					return;
				}
#endif
			}
			else
			{
				msg.print("Nexus file not found for run %i - looking for logfile instead...", rd->runNumber());
				// Search for the logfile for this run
				QString logFile = ISIS::locateFile(rd, "log", false, QDir());
				if (!logFile.isEmpty())
				{
					// Load the logfile
					QByteArray data;
					if (DataInterface::readFile(logFile, data) && ISIS::parseLogInformation(rd, data)) msg.print("Successfully parsed logfile " + logFile);
					else
					{
						QMessageBox::warning(this, "Couldn't access logfile", QString("Error reading logfile ") + logFile);
						return;
					}
				}
				else msg.print("Logfile not found for run %i", rd->runNumber());
			}
		}

		// Loop over block values in runData, creating an entry in the Instrument group/block structure
		instrument_->clearBlocks();
		bool dummy;
		for (Data2D* data = rd->blockData(); data != NULL; data = data->next) instrument_->addBlock(data->groupName(), data->name(), QString::number(data->runAverage(dummy)), "");
		for (SingleValue* sv = rd->singleValues(); sv != NULL; sv = sv->next) instrument_->addBlock(sv->groupName(), sv->blockName(), sv->value(), "");
	}
	else
	{
		instrument_->clearBlocks();

		// Get information from current SECI values online
		DataInterface di(NULL, NULL);
		QByteArray data;

		// Construct URL - will be of the form 'http://dataweb.isis.rl.ac.uk/SeciWeb/xml/XXX.xml' where XXX is the lower-case name of the instrument
		QString infoUrl = QString("http://dataweb.isis.rl.ac.uk/SeciWeb/xml/") + instrument_->capitalisedName().toLower() + ".xml";
		bool result = di.readHttp(QUrl(infoUrl), data);
		if (result)
		{
			// Parse information for instrument
			ISIS::parseInstrumentInformation(instrument_, data);
		}
	}

	// Retrieve settings for block values (if they exist)
	instrument_->retrieveBlockSelectedSettings();

	// Update list
	updateInstrumentInformation();
}

// Update Instrument information
void ReportGenerator::updateInstrumentInformation()
{
	// Clear tree
	ui.BlockTree->clear();
	ui.BlockTree->setHeaderLabels(QStringList() << "Group" << "Block" << "Value");

	TTreeWidgetItem* parent, *item;
	InstrumentBlockGroup* hiddenGroup = NULL;
	// Loop over groups - skip 'Hidden' group in this loop
	for (InstrumentBlockGroup* group = instrument_->blockGroups(); group != NULL; group = group->next)
	{
		if (group->name() == "Hidden")
		{
			hiddenGroup = group;
			continue;
		}

		// Create parent Tree node
		parent = new TTreeWidgetItem(ui.BlockTree, group->name());
		
		// Loop over block values
		for (InstrumentBlockValue* value = group->values(); value != NULL; value = value->next)
		{
			// Create an entry for this block value
			item = new TTreeWidgetItem(parent, value);
		}

		// Set group to be expanded
		parent->setExpanded(true);
	}

	// Do we have a hidden group? Now we add it (at the end)
	if (hiddenGroup)
	{
		// Create parent Tree node
		parent = new TTreeWidgetItem(ui.BlockTree, hiddenGroup->name());
		
		// Loop over block values
		for (InstrumentBlockValue* value = hiddenGroup->values(); value != NULL; value = value->next)
		{
			// Create an entry for this block value
			item = new TTreeWidgetItem(parent, value);
		}

		// Set group to be expanded
		parent->setExpanded(false);
	}

	// Resize columns in BlockTree
	for (int n=0; n<3; ++n) ui.BlockTree->resizeColumnToContents(n);
}

// Update available / visible properties
void ReportGenerator::updatePropertyLists()
{
	// Clear available property list, and add only those which appear in the availableProperties_ array
	ui.AvailablePropertiesList->clear();
	for (int n=0; n<RunProperty::nProperties; ++n) if (availableProperties_[n]) ui.AvailablePropertiesList->addItem(RunProperty::property((RunProperty::Property) n));

	// Clear visible property list, and add the current set in order
	ui.VisiblePropertiesList->clear();
	for (RunProperty* rp = visibleProperties_.first(); rp != NULL; rp = rp->next) ui.VisiblePropertiesList->addItem(RunProperty::property(rp->type()));
}

/*
// Experiment Details
*/

// Create experiment report
bool ReportGenerator::createReport(Document& report)
{
	const char* dateFormat = "dddd d MMMM yyyy";
	
	// Setup colours
	QColor headerBG(16, 40, 120);  // ISIS Blue??
	QColor headerText(255, 255, 255);
	
	QString s, valueString;
	QString uA = QString(QChar(0x03BC)) + "A";

	// Extra items to include in report
	bool needLogFileData = false;
	if (ui.AnalyseRunsCheck->isChecked()) needLogFileData = true;
	else if (ui.AnalyseBeamCurrentCheck->isChecked()) needLogFileData = true;
	else if (ui.AnalyseHModeratorCheck->isChecked()) needLogFileData = true;
	else if (ui.AnalyseCH4ModeratorCheck->isChecked()) needLogFileData = true;

	// Load log data for rundata (if required)
	if (needLogFileData)
	{
		int nLoaded = 0, n = 0;

		QProgressDialog progress("Loading data...", "Cancel", 0, nRuns_, this);
		progress.setWindowModality(Qt::WindowModal);

		// Loop over RunData
		for (RefListItem<RunData,Journal*>* ri = runData_.first(); ri != NULL; ri = ri->next)
		{
			RunData* rd = ri->item;

			// Check RB number of run
			if (rd->rbNumber() != rbNumber_) continue;

			// Check for progress dialog being canceled
			QApplication::processEvents();
			if (progress.wasCanceled()) break;

			if (rd->loadBlockData(RunData::LogBeforeNexusSource)) ++nLoaded;
			
			++n;
			progress.setValue(n);
		}
		progress.setValue(nRuns_);

		// Was the progress dialog cancelled
		if (n != nRuns_) return false;

		// Did we load all available data?
		if (nLoaded != nRuns_)
		{
			QMessageBox::StandardButton button = QMessageBox::warning(this, "Error loading data", QString("Error loading logfiles for ") + QString::number(nRuns_-nLoaded) + " of the " + QString::number(nRuns_) + " files needed for this report.\nCheck the data directory locations defined in Settings.");
			return false;
		}

	}

	// Variables
	int n, lastGroup = -1;
	RunData* rd = NULL;

	// Clear old document
	report.clearCommands();
	report.setBackgroundColour(QColor(255,255,255));
	report.setForegroundColour(QColor(0,0,0));
	report.setFont(parent_->documentFont());

	/* First part of report contains two columns only, and displays basic information about the experiment */

	report.clearColumns();
	report.addColumn();
	report.addColumn();
	report.addColumn(false);
	
	// -- 'Experiment' Section
	report.setColours(headerText, headerBG);
	report.addBoldText("Experiment", 99, Qt::AlignHCenter);
	report.endRow();
	report.setColours(QColor(0,0,0), QColor(255,255,255));
	report.addText("Instrument:  ", 1, Qt::AlignRight);
	report.addText(parent_->currentInstrument()->capitalisedName(), 2);
	report.endRow();
	report.addText("RB Number:  ", 1, Qt::AlignRight);
	if (ui.PartCheck->isChecked()) report.addText(QString::number(rbNumber_) + " (Part " + QString::number(ui.PartNSpin->value()) + " of " + QString::number(ui.NPartsSpin->value()) + ")", 2);
	else report.addText(QString::number(rbNumber_), 2);
	report.endRow();
	report.addText("User:  ", 1, Qt::AlignRight);
	report.addText(ui.UserCombo->currentText(), 2);
	report.endRow();
	report.addText("Start Date:  ", 1, Qt::AlignRight);
	report.addText(beginRunDateTime_.date().toString(dateFormat), 2);
	report.endRow();
	report.addText("End Date:  ", 1, Qt::AlignRight);
	report.addText(endRunDateTime_.date().toString(dateFormat), 2);
	report.endRow();
	report.endRow();

	// -- 'Blocks' Section
	if (instrument_->nSelectedBlocks() != 0)
	{
		report.setColours(headerText, headerBG);
		report.addBoldText("Instrument Setup", 99, Qt::AlignHCenter);
		report.endRow();
		report.setColours(QColor(0,0,0), QColor(255,255,255));
		for (InstrumentBlockGroup* group = instrument_->blockGroups(); group != NULL; group = group->next)
		{
			if (group->nSelected() == 0) continue;

			// Loop over values
			int n = 0;
			for (InstrumentBlockValue* value = group->values(); value != NULL; value = value->next)
			{
				if (!value->selected()) continue;

				// Construct the value string, including units
				if (value->units().isEmpty()) valueString = value->value();
				else valueString = value->value() + QString(" ") + value->units();

				// Write block group name, or just the data item
				if (n == 0)
				{
					report.addText(group->name() + ":  ", 1, Qt::AlignRight);
					report.addText(value->name() + "  ", 1, Qt::AlignRight);
					report.addText(valueString);
				}
				else
				{
					report.addText("", 1, Qt::AlignRight);
					report.addText(value->name() + "  ", 1, Qt::AlignRight);
					report.addText(valueString);
				}
				report.endRow();
				++n;
			}
		}
		report.endRow();
	}

	// -- 'Run Information' Section
	if (ui.AnalyseRunsCheck->isChecked())
	{
		report.setColours(headerText, headerBG);
		report.addBoldText("Run Information", 99, Qt::AlignCenter);
		report.endRow();
		report.setColours(QColor(0,0,0), QColor(255,255,255));
		report.addText("NRuns:  ", 1, Qt::AlignRight);
		report.addText(QString::number(nRuns_), 2);
		report.endRow();
		report.addText("Total RunTime:  ", 1, Qt::AlignRight);
		report.addText(RunData::durationAsString(totalRunTime_), 2);
		report.endRow();
		int totalTime = beginRunDateTime_.secsTo(endRunDateTime_);
		s.sprintf("%0.2f %s over %s, averaging %0.2f %s/h", totalCurrent_, qPrintable(uA), qPrintable(RunData::durationAsString(totalTime)), totalCurrent_ / (totalTime / 3600.0), qPrintable(uA));
		report.addText("Beam Usage:  ", 1, Qt::AlignRight);
		report.addText(s, 2);
		report.endRow();
		report.endRow();
	}

	/* Analysis Sections */
	PlotWidget beamCurrentGraph(this);
	if (ui.AnalyseBeamCurrentCheck->isChecked())
	{
		if (ISIS::location(parent_->currentInstrument()->instrument()) == ISIS::TS1)
		{
			if (ui.BeamRangeWarningCheck->isChecked()) setupGraph(beamCurrentGraph, "TS1BeamCurrent", ui.TS1BeamMinimumSpin->value(), Qt::red, ui.TS1BeamMaximumSpin->value(), Qt::white, "TS1Beam");
			else setupGraph(beamCurrentGraph, "TS1BeamCurrent", -1000.0, Qt::white, 1000.0, Qt::white, "TS1Beam");
		}
		else
		{
			if (ui.BeamRangeWarningCheck->isChecked()) setupGraph(beamCurrentGraph, "TS2BeamCurrent", ui.TS2BeamMinimumSpin->value(), Qt::red, ui.TS2BeamMaximumSpin->value(), Qt::white);
			else setupGraph(beamCurrentGraph, "TS2BeamCurrent", -1000.0, Qt::white, 1000.0, Qt::white);
		}
		beamCurrentGraph.setOnChangeData(ui.BeamRangeWarningCheck->isChecked());
		beamCurrentGraph.setFont(parent_->reportPlotFont());
		beamCurrentGraph.setYAxisTitle("Beam Current, " + uA);
		report.setColours(headerText, headerBG);
		report.addBoldText("Beam Current", 99, Qt::AlignCenter);
		report.endRow();
		report.setColours(QColor(0,0,0), QColor(255,255,255));
		report.addPlot(beamCurrentGraph, 99, 0.2);
		report.endRow();
	}

	PlotWidget moderatorH2Graph(this);
	if (ui.AnalyseHModeratorCheck->isChecked())
	{
		if (ui.ModeratorH2RangeWarningCheck->isChecked()) setupGraph(moderatorH2Graph, "Hydrogen_Temp",  ui.ModeratorH2MinimumSpin->value(), Qt::red, ui.ModeratorH2MaximumSpin->value(), Qt::red, "Coupled_Hydrogen");
		else setupGraph(moderatorH2Graph, "Hydrogen_Temp",  -1000.0, Qt::white, 1000.0, Qt::white, "Coupled_Hydrogen");
		moderatorH2Graph.setOnChangeData(ui.ModeratorH2OnChangeCheck->isChecked());
		moderatorH2Graph.setFont(parent_->reportPlotFont());
		moderatorH2Graph.setYAxisTitle("Temperature, K");
		report.setColours(headerText, headerBG);
		report.addBoldText("Hydrogen Moderator", 99, Qt::AlignCenter);
		report.endRow();
		report.setColours(QColor(0,0,0), QColor(255,255,255));
		report.addPlot(moderatorH2Graph, 99, 0.2);
		report.endRow();
	}

	PlotWidget moderatorCH4Graph(this);
	if (ui.AnalyseCH4ModeratorCheck->isChecked())
	{
		if (ui.ModeratorCH4RangeWarningCheck->isChecked()) setupGraph(moderatorCH4Graph, "Coupled_Methane", ui.ModeratorCH4MinimumSpin->value(), Qt::red, ui.ModeratorCH4MaximumSpin->value(), Qt::red);
		else setupGraph(moderatorCH4Graph, "Coupled_Methane", -1000.0, Qt::white, 1000.0, Qt::white);
		moderatorCH4Graph.setOnChangeData(ui.ModeratorCH4OnChangeCheck->isChecked());
		moderatorCH4Graph.setFont(parent_->reportPlotFont());
		moderatorCH4Graph.setYAxisTitle("Temperature, K");
		report.setColours(headerText, headerBG);
		report.addBoldText("Methane Moderator", 99, Qt::AlignCenter);
		report.endRow();
		report.setColours(QColor(0,0,0), QColor(255,255,255));
		report.addPlot(moderatorCH4Graph, 99, 0.2);
		report.endRow();
	}

	// Reset font here, just in case a graph changed it
	report.setFont(parent_->documentFont());

	/* Run Data */
	// Create basic column layout
	report.clearColumns();
	for (RunProperty* rp = visibleProperties_.first(); rp != NULL; rp = rp->next) report.addColumn(rp->type() != RunProperty::Title);

	bool highlighting = true, highlight = false;

	// Setup initial date...
	QDate date(1900,1,1);

	// Loop over RunData
	for (RefListItem<RunData,Journal*>* ri = runData_.first(); ri != NULL; ri = ri->next)
	{
		RunData* rd = ri->item;

		// Check RB number of run
		if (rd->rbNumber() != rbNumber_) continue;

		// Write new date and header rows?
		if (rd->startDate() > date)
		{
			// Leave a gap
			report.endRow();

			// Reset colours and highlighting status
			highlight = true;
			report.setColours(QColor(0,0,0), QColor(255,255,255));
			date = rd->startDate();
			report.addBoldText(date.toString(dateFormat), 99);
			report.endRow();
			
			// Write column headers
			report.setColours(headerText, headerBG);
			for (RunProperty* rp = visibleProperties_.first(); rp != NULL; rp = rp->next) report.addBoldText(RunProperty::property(rp->type()));
			report.endRow();
		}

		// Determine highlight status
		if (highlighting)
		{
			highlight = !highlight;
			report.setBackgroundColour(highlight ? parent_->highlightBGColour() : QColor(255,255,255));
			report.setForegroundColour(highlight ? parent_->highlightTextColour() : QColor(0,0,0));
		}

		// Loop over RunProperties
		for (RunProperty* rp = visibleProperties_.first(); rp != NULL; rp = rp->next) report.addText(rd->propertyAsString(rp->type()));
		
		// End table row
		report.endRow();
	}
	
	report.end();

	return result_;
}

/*
// Widget Slots
*/

// Experiment combo changed
void ReportGenerator::on_RBCombo_currentIndexChanged(int index)
{
	if (refreshing_) return;

	// If no expts available, disable some controls
	ui.ExperimentDetailsGroup->setEnabled(index != -1);
	ui.SaveAsPDFButton->setEnabled(index != -1);
	ui.PrintButton->setEnabled(index != -1);
	if (index == -1) return;

	updateDetails(availableRB_.at(index));
}

// Experiment Part check clicked
void ReportGenerator::on_PartCheck_clicked(bool checked)
{
	ui.NPartsSpin->setEnabled(checked);
	ui.PartNSpin->setEnabled(checked);
	ui.PartLabel->setEnabled(checked);
}

// Experiment NParts changed
void ReportGenerator::on_NPartsSpin_valueChanged(int value)
{
	ui.PartNSpin->setMaximum(value);
}

// RunInformation radio clicked
void ReportGenerator::on_RunInformationRadio_clicked(bool checked)
{
	retrieveInstrumentInformation();
}

// RunInformation combo changed
void ReportGenerator::on_RunInformationCombo_currentIndexChanged(int index)
{
	if (refreshing_) return;
	retrieveInstrumentInformation();
}

// CurrentSECI values radio clicked
void ReportGenerator::on_CurrentSECIRadio_clicked(bool checked)
{
	retrieveInstrumentInformation();
}

// Reload info button pressed
void ReportGenerator::on_ReloadButton_clicked(bool checked)
{
	retrieveInstrumentInformation();
}

// Set as Default button pressed
void ReportGenerator::on_SetAsDefaultButton_clicked(bool checked)
{
	storeSettings();
}

// Block tree item clicked
void ReportGenerator::on_BlockTree_itemClicked(QTreeWidgetItem* item, int column)
{
	// Convert originating item to a TTreeWidgetItem
	TTreeWidgetItem* titem = (TTreeWidgetItem*) item;
	if (titem == NULL) return;

	// Was a child item clicked?
	if (titem->block() == NULL) return;

	// We are only interested in certain columns...
	if (column == 0) titem->block()->setSelected(item->checkState(0) == Qt::Checked);
}

// Block Tree context menu - Set Group
void ReportGenerator::blockTreeContextMenu_setGroup(bool checked)
{
	// Get new group name from user
	bool ok;
	QString groupName = QInputDialog::getText(this, "Assign selected blocks to new group", "New Group Name", QLineEdit::Normal, "", &ok);
	if (!ok) return;
	
	// Assign group name to selected items (blocks) - blockName is in column 1
	foreach(QTreeWidgetItem* qitem, ui.BlockTree->selectedItems())
	{
		// Cast into TTreeWidgetItem
		TTreeWidgetItem* item = (TTreeWidgetItem*) qitem;

		if (item->block())
		{
			instrument_->addBlockGroupMapping(item->block()->name(), groupName);
			instrument_->moveBlock(item->block(), groupName);
		}
	}
	
	updateInstrumentInformation();
}

// Block Tree context menu - Remove Group
void ReportGenerator::blockTreeContextMenu_removeGroup(bool checked)
{
	// Assign group name to selected items (blocks) - blockName is in column 1
	foreach(QTreeWidgetItem* qitem, ui.BlockTree->selectedItems())
	{
		// Cast into TTreeWidgetItem
		TTreeWidgetItem* item = (TTreeWidgetItem*) qitem;

		if (item->block())
		{
			instrument_->removeBlockGroupMapping(item->block()->name());
			instrument_->moveBlock(item->block(), "");
		}
	}
	
	updateInstrumentInformation();
}

// Add property to visible list
void ReportGenerator::on_AddPropertyButton_clicked(bool checked)
{
	if (ui.AvailablePropertiesList->currentItem() == NULL) return;

	// Find property in list
	RunProperty::Property prop = RunProperty::property(ui.AvailablePropertiesList->currentItem()->text());

	// Remove property from available list, and add to visible list
	availableProperties_[prop] = false;
	visibleProperties_.add()->setType(prop);

	// Refresh widgets
	updatePropertyLists();
}

// Remove property from visible list
void ReportGenerator::on_RemovePropertyButton_clicked(bool checked)
{
	if (ui.VisiblePropertiesList->currentItem() == NULL) return;

	// Find property in list
	RunProperty::Property prop = RunProperty::property(ui.VisiblePropertiesList->currentItem()->text());

	// Remove property from visible list, and add to available list
	availableProperties_[prop] = true;
	for (RunProperty* rp = visibleProperties_.first(); rp != NULL; rp = rp->next)
	{
		if (rp->type() == prop)
		{
			visibleProperties_.remove(rp);
			break;
		}
	}

	// Refresh widgets
	updatePropertyLists();
}

// Move property up
void ReportGenerator::on_MovePropertyUpButton_clicked(bool checked)
{
	if (ui.VisiblePropertiesList->currentItem() == NULL) return;

	// Find property in list
	RunProperty::Property prop = RunProperty::property(ui.VisiblePropertiesList->currentItem()->text());
	RunProperty* rp = NULL;
	for (rp = visibleProperties_.first(); rp != NULL; rp = rp->next) if (rp->type() == prop) break;
	if (rp == NULL) return;
	
	visibleProperties_.shiftUp(rp);

	// Refresh widgets
	updatePropertyLists();

	// Reselect item in list
	ui.VisiblePropertiesList->setCurrentRow(visibleProperties_.indexOf(rp));
}

// Move property down
void ReportGenerator::on_MovePropertyDownButton_clicked(bool checked)
{
	if (ui.VisiblePropertiesList->currentItem() == NULL) return;

	// Find property in list
	RunProperty::Property prop = RunProperty::property(ui.VisiblePropertiesList->currentItem()->text());
	RunProperty* rp = NULL;
	for (rp = visibleProperties_.first(); rp != NULL; rp = rp->next) if (rp->type() == prop) break;
	if (rp == NULL) return;
	
	visibleProperties_.shiftDown(rp);

	// Refresh widgets
	updatePropertyLists();

	// Reselect item in list
	ui.VisiblePropertiesList->setCurrentRow(visibleProperties_.indexOf(rp));
}

// Cancel Button Pressed
void ReportGenerator::on_CancelButton_clicked(bool checked)
{
	result_ = ReportGenerator::CancelResult;
	reject();
}

// SaveAsPDF Button Pressed
void ReportGenerator::on_SaveAsPDFButton_clicked(bool checked)
{
	result_ = ReportGenerator::SaveResult;
	accept();
}

// Print Button pressed
void ReportGenerator::on_PrintButton_clicked(bool checked)
{
	result_ = ReportGenerator::PrintResult;
	accept();
}

/*
 * Graph Generation
 */

// Setup generic property graph
void ReportGenerator::setupGraph(PlotWidget& target, QString blockName, double minY, QColor minColor, double maxY, QColor maxColor, QString altBlockName)
{
	// Loop over RunData for the experiment, and add data to our plotter
	for (RefListItem<RunData,Journal*>* ri = runData_.first(); ri != NULL; ri = ri->next)
	{
		RunData* rd = ri->item;

		// Check RB number of run
		if (rd->rbNumber() != rbNumber_) continue;

		// If an alternative block name was provided, need to check both
		if (rd->hasBlockData(blockName))
		{
			Data2D& dataSource = rd->blockData(blockName);
			target.addDataSet(NULL, dataSource, blockName, rd->endDateTime(), blockName);
		}
		else if (rd->hasBlockData(altBlockName))
		{
			Data2D& dataSource = rd->blockData(altBlockName);
			target.addDataSet(NULL, dataSource, altBlockName, rd->endDateTime(), altBlockName);
		}
		else
		{
			if (altBlockName.isEmpty()) msg.print("Warning - Run %i did not contain a block named '%s'.\n", rd->runNumber(), qPrintable(blockName));
			else msg.print("Warning - Run %i did not contain a block named '%s' or '%s'.\n", rd->runNumber(), qPrintable(blockName), qPrintable(altBlockName));
		}
	}

	target.setAbsoluteTime(true);
	target.setSingleProperty(true);
	target.setShadedBackground(true);
	target.setShadedBackgroundMinimum(minY, minColor);
	target.setShadedBackgroundMaximum(maxY, maxColor);
	target.setShowLegend(false);
	target.setBlockVisible(blockName, true);
	if (!altBlockName.isEmpty()) target.setBlockVisible(altBlockName, true);
	target.fitData(true);
}
