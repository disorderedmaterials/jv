/*
	*** JournalViewer Settings Functions
	*** src/jv_settings.cpp
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
#include <QSettings>

// Set default settings
void JournalViewer::setDefaultSettings()
{
	// Visible Properties
	visibleProperties_.add()->setType(RunProperty::RunNumber);
	visibleProperties_.add()->setType(RunProperty::Title);
	visibleProperties_.add()->setType(RunProperty::StartTimeAndDate);
	visibleProperties_.add()->setType(RunProperty::Duration);
	visibleProperties_.add()->setType(RunProperty::ProtonCharge);
	visibleProperties_.add()->setType(RunProperty::User);
	
	// Journal Access / Instrument
	journalDirectory_ = QDir::currentPath();
	journalUrl_ = "http://data.isis.rl.ac.uk/journals/";
#ifdef _WIN32
	dataDirectory_ = "\\\\isis\\inst$\\";
	validDataDirectory_ = true;
#else
	dataDirectory_ = "";
	validDataDirectory_ = false;
#endif
	forceISOEncoding_ = false;
	userDataDirectory_ = "";
	validUserDataDirectory_ = false;
	journalAccessType_ = JournalViewer::NetOnlyAccess;
	defaultInstrument_ = ISIS::nInstruments;
	blockDataSource_ = RunData::LogBeforeNexusSource;
	autoReload_ = false;
	autoReloadFrequency_ = 5;
	
	// Document Style
	documentFont_.setPointSize(8);
	plotFont_.setPointSize(8);
	reportPlotFont_.setPointSize(8);
	headerTextColour_= Qt::white;
	headerBGColour_ = Qt::black;
	highlightTextColour_ = Qt::black;
	highlightBGColour_.setRgb(230,230,230);
	highlightDocument_ = true;
	documentLayout_ = JournalViewer::ListLayout;
	
	// Margins
	setDocumentMargins(20, 20, 20, 20);
	setReportMargins(40, 20, 20, 20);
}

// Store settings
void JournalViewer::storeSettings()
{
	QSettings settings;

	// Window Settings
	settings.setValue("MainWinPositions", saveState());
	settings.setValue("MainWinGeometries", saveGeometry());
	settings.value("MainWinSize", size());
	settings.value("MainWinPosition", pos());

	// Program Settings
	settings.setValue("JournalDirectory", journalDirectory_.path());
	settings.setValue("JournalUrl", journalUrl_);
	settings.setValue("DataDirectory", dataDirectory_.path());
	settings.setValue("UserDataDirectory", userDataDirectory_.path());
	settings.setValue("JournalAccessType", journalAccessType_);
	if (defaultInstrument_ != ISIS::nInstruments) settings.setValue("DefaultInstrument", ISIS::capitalisedName(defaultInstrument_));
	settings.setValue("AutoReload", autoReload_);
	settings.setValue("AutoReloadFrequency", autoReloadFrequency_);
	settings.setValue("BlockDataSource", blockDataSource_);
	settings.setValue("ForceISOEncoding", forceISOEncoding_);

	// Visible properties
	QStringList visProps;
	for (RunProperty* rp = visibleProperties_.first(); rp != NULL; rp = rp->next) visProps << RunProperty::property(rp->type());
	settings.setValue("VisibleProperties", visProps);

	// Style Settings
	settings.setValue("HeaderTextColour", headerTextColour_.name());
	settings.setValue("HeaderBGColour", headerBGColour_.name());
	settings.setValue("HighlightTextColour", highlightTextColour_.name());
	settings.setValue("HighlightBGColour", highlightBGColour_.name());
	settings.setValue("DocumentFont", documentFont_.family());
	settings.setValue("DocumentFontSize", documentFont_.pointSizeF());
	settings.setValue("PlotFont", plotFont_.family());
	settings.setValue("PlotFontSize", plotFont_.pointSizeF());
	settings.setValue("ReportPlotFont", reportPlotFont_.family());
	settings.setValue("ReportPlotFontSize", reportPlotFont_.pointSizeF());
	settings.setValue("DocumentLayout", documentLayout_);

	// Margins
	for (int n=0; n<4; ++n)
	{
		settings.setValue("DocumentMargins"+QString::number(n), documentMargins_[n]);
		settings.setValue("ReportMargins"+QString::number(n), reportMargins_[n]);
	}

	settings.sync();
}

// Retrieve settings
void JournalViewer::retrieveSettings()
{
	QSettings settings;

	// Window Settings
	if (settings.contains("MainWinPositions")) restoreState( settings.value("MainWinPositions").toByteArray());
	if (settings.contains("MainWinGeometries")) restoreGeometry( settings.value("MainWinGeometries").toByteArray());
	if (settings.contains("MainWinSize")) resize(settings.value("mainwin_size", QSize(400, 400)).toSize());
	if (settings.contains("MainWinPosition")) move(settings.value("mainwin_pos", QPoint(200, 200)).toPoint());

	// Program Settings
	if (settings.contains("JournalDirectory")) journalDirectory_ = settings.value("JournalDirectory").toString();
	if (settings.contains("JournalUrl")) journalUrl_ = settings.value("JournalUrl").toString();
	if (settings.contains("DataDirectory")) setDataDirectory(settings.value("DataDirectory").toString());
	if (settings.contains("UserDataDirectory")) setUserDataDirectory(settings.value("UserDataDirectory").toString());
	if (settings.contains("JournalAccessType")) journalAccessType_ = (JournalViewer::JournalAccess) settings.value("JournalAccessType").toInt();
	if (settings.contains("DefaultInstrument"))
	{
		ISIS::ISISInstrument inst = ISIS::instrument(settings.value("DefaultInstrument").toString());
		if (inst == ISIS::nInstruments) msg.print("JournalViewer::retrieveSettings() - Unrecognised default instrument " + settings.value("DefaultInstrument").toString());
		else defaultInstrument_ = inst;
	}
	if (settings.contains("AutoReloadFrequency")) setAutoReloadFrequency(settings.value("AutoReloadFrequency").toInt());
	if (settings.contains("AutoReload")) setAutoReload(settings.value("AutoReload").toBool());
	if (settings.contains("BlockDataSource")) blockDataSource_ = (RunData::BlockDataSource) settings.value("BlockDataSource").toInt();
	if (settings.contains("ForceISOEncoding")) forceISOEncoding_ = settings.value("ForceISOEncoding").toBool();

	// Visible properties
	if (settings.contains("VisibleProperties"))
	{
		QStringList visProps = settings.value("VisibleProperties").toStringList();
		visibleProperties_.clear();
		for (int n=0; n<visProps.size(); ++n)
		{
			  RunProperty::Property prop = RunProperty::property(visProps.at(n));
			  if (prop != RunProperty::nProperties) visibleProperties_.add()->setType(prop);
		}
	}

	// Style Settings
	if (settings.contains("HeaderTextColour")) headerTextColour_ = settings.value("HeaderTextColour").toString();
	if (settings.contains("HeaderBGColour")) headerBGColour_ = settings.value("HeaderBGColour").toString();
	if (settings.contains("HighlightTextColour")) highlightTextColour_ = settings.value("HighlightTextColour").toString();
	if (settings.contains("HighlightBGColour")) highlightBGColour_ = settings.value("HighlightBGColour").toString();
	if (settings.contains("DocumentFont")) documentFont_.setFamily(settings.value("DocumentFont").toString());
	if (settings.contains("DocumentFontSize")) documentFont_.setPointSizeF(settings.value("DocumentFontSize").toDouble());
	if (settings.contains("PlotFont")) plotFont_.setFamily(settings.value("PlotFont").toString());
	if (settings.contains("PlotFontSize")) plotFont_.setPointSizeF(settings.value("PlotFontSize").toDouble());
	if (settings.contains("ReportPlotFont")) reportPlotFont_.setFamily(settings.value("ReportPlotFont").toString());
	if (settings.contains("ReportPlotFontSize")) reportPlotFont_.setPointSizeF(settings.value("ReportPlotFontSize").toDouble());
	if (settings.contains("DocumentLayout")) documentLayout_ = (JournalViewer::DocumentLayout) settings.value("DocumentLayout").toInt();

	// Margins
	for (int n=0; n<4; ++n)
	{
		if (settings.contains("DocumentMargins"+QString::number(n))) documentMargins_[n] = settings.value("DocumentMargins"+QString::number(n)).toDouble();
		if (settings.contains("ReportMargins"+QString::number(n))) reportMargins_[n] = settings.value("ReportMargins"+QString::number(n)).toDouble();
	}

	/* LITE version - enforce some values */
#ifdef LITE
	journalAccessType_ = JournalViewer::DiskOnlyAccess;
	journalDirectory_ = QDir();
	journalUrl_ = QUrl();
	defaultInstrument_ = ISIS::LOCAL;
#endif
}

// Return local directory for storing journal copies
QDir JournalViewer::journalDirectory()
{
	return journalDirectory_;
}

// Set local directory for storing journal copies
void JournalViewer::setJournalDirectory(QString dir)
{
	journalDirectory_ = dir;
}

// Return http directory for storing journal copies
QUrl JournalViewer::journalUrl()
{
	return journalUrl_;
}

// Set http directory for storing journal copies
void JournalViewer::setJournalUrl(QString url)
{
	journalUrl_ = url;
}

// Return data directory
QDir JournalViewer::dataDirectory()
{
	return dataDirectory_;
}

// Set data directory
void JournalViewer::setDataDirectory(QString dir)
{
	dataDirectory_ = dir;
	validDataDirectory_ = dataDirectory_.exists();
	msg.print("Data directory '%s' %s.\n", qPrintable(dataDirectory_.path()), validDataDirectory_ ? "exists" : "does not exist.");
}

// Return whether data directory is valid
bool JournalViewer::validDataDirectory()
{
	return validDataDirectory_;
}

// Return user data directory
QDir JournalViewer::userDataDirectory()
{
	return userDataDirectory_;
}

// Set user data directory
void JournalViewer::setUserDataDirectory(QString dir)
{
	userDataDirectory_ = dir;
	validUserDataDirectory_ = userDataDirectory_.exists();
	msg.print("Data directory '%s' %s.\n", qPrintable(userDataDirectory_.path()), validUserDataDirectory_ ? "exists" : "does not exist.");
}

// Return whether user data directory is valid
bool JournalViewer::validUserDataDirectory()
{
	return validUserDataDirectory_;
}

// Return current journal access approach
JournalViewer::JournalAccess JournalViewer::journalAccessType()
{
	return journalAccessType_;
}

// Return current journal access approach
void JournalViewer::setJournalAccessType(JournalViewer::JournalAccess type)
{
	journalAccessType_ = type;
}

// Return default Instrument to display
ISIS::ISISInstrument JournalViewer::defaultInstrument()
{
	return defaultInstrument_;
}

// Set default Instrument to display
void JournalViewer::setDefaultInstrument(ISIS::ISISInstrument inst)
{
	defaultInstrument_ = inst;
}

// Set preferred source for loading SE block data
void JournalViewer::setBlockDataSource(RunData::BlockDataSource source)
{
	blockDataSource_ = source;
}

// Return preferred source for loading SE block data
RunData::BlockDataSource JournalViewer::blockDataSource()
{
	return blockDataSource_;
}

// Return header text colour
QColor JournalViewer::headerTextColour()
{
	return headerTextColour_;
}

// Set header text colour
void JournalViewer::setHeaderTextColour(QColor colour)
{
	headerTextColour_ = colour;
}

// Return header cell colour
QColor JournalViewer::headerBGColour()
{
	return headerBGColour_;
}

// Set header cell colour
void JournalViewer::setHeaderBGColour(QColor colour)
{
	headerBGColour_ = colour;
}

// Return highlight text colour
QColor JournalViewer::highlightTextColour()
{
	return highlightTextColour_;
}

// Set highlight text colour
void JournalViewer::setHighlightTextColour(QColor colour)
{
	highlightTextColour_ = colour;
}

// Return highlight cell colour
QColor JournalViewer::highlightBGColour()
{
	return highlightBGColour_;
}

// Set highlight cell colour
void JournalViewer::setHighlightBGColour(QColor colour)
{
	highlightBGColour_ = colour;
}

// Return document font
QFont JournalViewer::documentFont()
{
	return documentFont_;
}

// Set document font
void JournalViewer::setDocumentFont(QFont font)
{
	documentFont_ = font;
}

// Return plot font
QFont JournalViewer::plotFont()
{
	return plotFont_;
}

// Set plot font
void JournalViewer::setPlotFont(QFont font)
{
	plotFont_ = font;
}

// Return report plot font
QFont JournalViewer::reportPlotFont()
{
	return reportPlotFont_;
}

// Set report plot font
void JournalViewer::setReportPlotFont(QFont font)
{
	reportPlotFont_ = font;
}

// Return whether to highlight rows/groups in document
bool JournalViewer::highlightDocument()
{
	return highlightDocument_;
}

// Return whether to highlight rows/groups in document
void JournalViewer::setHighlightDocument(bool highlight)
{
	highlightDocument_ = highlight;
}

// Return document layout style
JournalViewer::DocumentLayout JournalViewer::documentLayout()
{
	return documentLayout_;
}

// Document layout style
void JournalViewer::setDocumentLayout(JournalViewer::DocumentLayout layout)
{
	documentLayout_ = layout;
}

// Return general document margins (mm)
int* JournalViewer::documentMargins()
{
	return documentMargins_;
}

// Set general document margins (mm)
void JournalViewer::setDocumentMargins(int left, int top, int right, int bottom)
{
	documentMargins_[0] = left;
	documentMargins_[1] = top;
	documentMargins_[2] = right;
	documentMargins_[3] = bottom;
}

// Return report document margins (mm)
int* JournalViewer::reportMargins()
{
	return reportMargins_;
}

// Set report document margins (mm)
void JournalViewer::setReportMargins(int left, int top, int right, int bottom)
{
	reportMargins_[0] = left;
	reportMargins_[1] = top;
	reportMargins_[2] = right;
	reportMargins_[3] = bottom;
}

// Return whether to auto-refresh current journal data
bool JournalViewer::autoReload()
{
	return autoReload_;
}

// Set whether to auto-refresh current journal data
void JournalViewer::setAutoReload(bool reload)
{
	autoReload_ = reload;
	if (autoReload_) autoReloadTimer_.start();
	else autoReloadTimer_.stop();
}

// Return frequency (in minutes) of auto-refresh
int JournalViewer::autoReloadFrequency()
{
	return autoReloadFrequency_;
}

// Set frequency (in minutes) of auto-refresh
void JournalViewer::setAutoReloadFrequency(int mins)
{
	autoReloadFrequency_ = mins;
	autoReloadTimer_.setInterval(autoReloadFrequency_*1000*60);
}

// Set whether to force ISO-8859-1 encoding when reading XML files
void JournalViewer::setForceISOEncoding(bool b)
{
	forceISOEncoding_ = b;
}

// Return whether to force ISO-8859-1 encoding when reading XML files
bool JournalViewer::forceISOEncoding()
{
	return forceISOEncoding_;
}
