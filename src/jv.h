/*
	*** Main Window
	*** src/jv.h
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

#ifndef JOURNALVIEWER_MAINWINDOW_H
#define JOURNALVIEWER_MAINWINDOW_H

#include "ui_jv.h"
#include "list.h"
#include "rundata.h"
#include "instrument.h"
#include "logwindow.h"
#include <QDir>
#include <QTimer>
#include <QtPrintSupport/QPrinter>

// Forward Declarations
class Journal;
class QTextEdit;
class QPushButton;
class QProgressBar;
class LogWindow;
class FindWindow;
class PrintSetup;
class Document;
class DataInterface;

class JournalViewer : public QMainWindow
{
	// All Qt declarations must include this macro
	Q_OBJECT


	/*
	// Window Functions
	*/
	public:
	// Constructor / Destructor
	JournalViewer(QMainWindow *parent = 0);
	~JournalViewer();
	// Main form declaration
	Ui::JournalViewer ui;

	protected:
	// Window close event
	void closeEvent(QCloseEvent *event);
	
	public:
	// Clear all loaded data
	void clear();
	// Initialise JournalViewer
	bool initialise(bool startTimers = false);
	// Show license if necessary, returning whether it was accepted
	bool showLicense(bool force = false);


	/*
	 * Widget Slots
	 */
	private slots:
	// -- Instrument / Journal Selection
	// Current instrument changed
	void on_InstrumentCombo_currentIndexChanged(int index);
	// Current journal changed
	void on_JournalCombo_currentIndexChanged(int index);
	// Reload journal button clicked
	void on_ReloadJournalButton_clicked(bool checked);

	// -- Search Panel
	// Search text entered
	void on_SearchEdit_returnPressed();
	// Search style changed
	void on_SearchStyleCombo_currentIndexChanged(int index);
	// Search box clear button pressed
	void on_SearchClearButton_clicked(bool checked);
	// Case Sensitivity checkbox clicked
	void on_SearchCaseSensitiveCheck_clicked(bool checked);

	// -- Filter Group
	// Filter user combo changed
	void on_FilterUserCombo_currentIndexChanged(int index);
	// User search box clear button pressed
	void on_UserSearchClearButton_clicked(bool checked);
	// Filter RB combo changed
	void on_FilterRBCombo_currentIndexChanged(int index);
	// RB search box clear button pressed
	void on_RBSearchClearButton_clicked(bool checked);
	// Filter date type changed
	void on_FilterDateTypeCombo_currentIndexChanged(int index);
	// From date filter changed
	void on_FilterFromDateTimeEdit_dateTimeChanged(const QDateTime& datetime);
	// To date filter changed
	void on_FilterToDateTimeEdit_dateTimeChanged(const QDateTime& datetime);
	// From run filter changed
	void on_FilterFromRunSpin_valueChanged(int value);
	// To run filter changed
	void on_FilterToRunSpin_valueChanged(int value);
	// From date filter lock status changed
	void on_FilterFromDateTimeLockButton_clicked(bool checked);
	// To date filter lock status changed
	void on_FilterToDateTimeLockButton_clicked(bool checked);
	// From run filter lock status changed
	void on_FilterFromRunLockButton_clicked(bool checked);
	// To run filter lock status changed
	void on_FilterToRunLockButton_clicked(bool checked);

	// -- Data Table
	// Header of data table clicked
	void dataTable_headerClicked(int section);
	// Table cell single-clicked
	void on_DataTable_itemClicked(QTableWidgetItem* item);
	// Table cell double-clicked
	void on_DataTable_itemDoubleClicked(QTableWidgetItem* item);
	// Context menu event
	void dataTable_contextMenuEvent(const QPoint& pos);
	
	// -- File Menu
	// File->SaveAsText selected
	void on_actionFileSaveAsText_triggered(bool checked);
	// File->SaveAsPDF selected
	void on_actionFileSaveAsPDF_triggered(bool checked);
	// File->Print selected
	void on_actionFilePrint_triggered(bool checked);
	// File->PrintSelection selected
	void on_actionFilePrintSelection_triggered(bool checked);
	// File->Quit selected
	void on_actionFileQuit_triggered(bool checked);

	// -- View Menu
	// Item selected in View menu
	void viewMenuActionTriggered(bool checked);

	// -- Selection Menu
	// Clear current selection
	void on_actionSelectionClear_triggered(bool checked);
	// Plot data selected
	void on_actionSelectionPlotData_triggered(bool checked);
	// Plot logfile data selected
	void on_actionSelectionPlotLogfileData_triggered(bool checked);
	// Plot Nexus file data selected
	void on_actionSelectionPlotNexusData_triggered(bool checked);
	// Copy selection to clipboard
	void on_actionSelectionCopy_triggered(bool checked);

	// -- Tools Menu
	// Tools->Find... selected
	void on_actionToolsFind_triggered(bool checked);
	// Tools->FindNext selected
	void on_actionToolsFindNext_triggered(bool checked);
	// Tools->FindPrevious selected
	void on_actionToolsFindPrevious_triggered(bool checked);
	// Tools->Reload Data selected
	void on_actionToolsReloadData_triggered(bool checked);
	// Tools->GroupData selected
	void on_actionToolsGroupData_triggered(bool checked);
	// Tools->Reset Filters selected
	void on_actionToolsResetFilters_triggered(bool checked);
	// Tools->CreateExperimentReport selected
	void on_actionToolsCreateExperimentReport_triggered(bool checked);
	// Tools->CreateQuickReport selected
	void on_actionToolsCreateQuickReport_triggered(bool checked);
	// Tools->Settings selected
	void on_actionToolsSettings_triggered(bool checked);
	// Tools->Log Window selected
	void on_actionToolsLogWindow_triggered(bool checked);
	// Tools->Regenerate Local Journals selected
	void on_actionToolsRegenerateLocalJournals_triggered(bool checked);
	// Tools->Show License selected
	void on_actionToolsShowLicense_triggered(bool checked);

	// -- Hidden Actions
	void on_actionCancelDownload_triggered(bool checked);

	private:
	// Flag to indicate widgets are currently being refreshed
	bool refreshing_;
	// List of actions in View menu associated to Properties
	QAction* viewPropertyAction[RunProperty::nProperties];


	/*
	 * Windows / Widgets
	 */
	private:
	// LogWindow Dialog
	LogWindow* logWindow_;
	// FindWindow Dialog
	FindWindow* findWindow_;
	// Label for displaying date / source information in status bar
	QLabel* statusSourceLabel_;
	// Label for displaying search / filter data
	QLabel* statusSearchLabel_;
	// Label for displaying group mode
	QLabel* statusGroupLabel_;
	// Progress bar for http operations
	QProgressBar* statusHttpProgress_;
	// Label for http progress bar
	QLabel* statusHttpProgressLabel_;
	// Data container for update check
	QByteArray updateCheckData_;

	private:
	// Set enabled state of instrument / journal combo boxes
	void setJournalControlsEnabled(bool enabled);
	// Update status bar permanent widgets
	void updateStatusBarPermanentWidgets();

	private slots:
	// Hide progress bar
	void hideProgressBar();


	/*
	 * Settings
	 */
	public:
	// Journal Access types
	enum JournalAccess { DiskOnlyAccess, NetOnlyAccess, DiskAndNetAccess, NoAccess };
	// Document Layout styles
	enum DocumentLayout { ListLayout, IndentedLayout };
	
	private:
	// Local directory for storing journal copies
	QDir journalDirectory_;
	// Base URL for journals
	QUrl journalUrl_;
	// Base data directory
	QDir dataDirectory_;
	// Whether a valid data directory has been specified
	bool validDataDirectory_;
	// User data directory
	QDir userDataDirectory_;
	// Whether a valid user data directory has been specified
	bool validUserDataDirectory_;
	// Current journal access approach
	JournalAccess journalAccessType_;
	// Preferred source for loading SE block data
	RunData::BlockDataSource blockDataSource_;
	// Default Instrument to display
	ISIS::ISISInstrument defaultInstrument_;
	// Colours for CSS
	QColor headerTextColour_, headerBGColour_, highlightTextColour_, highlightBGColour_;
	// Font to use for printing
	QFont documentFont_;
	// Font to use for plots
	QFont plotFont_;
	// Font to use for plots in reports
	QFont reportPlotFont_;
	// Whether to highlight rows/groups in document
	bool highlightDocument_;
	// Document layout style
	JournalViewer::DocumentLayout documentLayout_;
	// General document margins
	int documentMargins_[4];
	// Report document margins
	int reportMargins_[4];
	// Whether to auto-refresh current journal data
	bool autoReload_;
	// Frequency (minutes) of auto-refresh
	int autoReloadFrequency_;
	// Timer for auto-refresh
	QTimer autoReloadTimer_;
	// Timer for hiding progress bar/label
	QTimer hideProgressTimer_;
	// Whether to force ISO-8859-1 encoding when reading XML files
	bool forceISOEncoding_;

	private:
	// Set default settings
	void setDefaultSettings();
	// Store settings
	void storeSettings();
	// Retrieve settings
	void retrieveSettings();

	public:
	// Return local directory for storing journal copies
	QDir journalDirectory();
	// Set local directory for storing journal copies
	void setJournalDirectory(QString dir);
	// Return user journal directory
	QDir userJournalDirectory();
	// Return http directory for storing journal copies
	QUrl journalUrl();
	// Set http directory for storing journal copies
	void setJournalUrl(QString dir);
	// Return data directory
	QDir dataDirectory();
	// Set data directory
	void setDataDirectory(QString dir);
	// Return whether data directory is valid
	bool validDataDirectory();
	// Return user data directory
	QDir userDataDirectory();
	// Return whether user data directory is valid
	bool validUserDataDirectory();
	// Set user data directory
	void setUserDataDirectory(QString dir);
	// Return current journal access approach
	JournalAccess journalAccessType();
	// Return current journal access approach
	void setJournalAccessType(JournalAccess type);
	// Set preferred source for loading SE block data
	void setBlockDataSource(RunData::BlockDataSource source);
	// Return preferred source for loading SE block data
	RunData::BlockDataSource blockDataSource();
	// Return default Instrument to display
	ISIS::ISISInstrument defaultInstrument();
	// Set default Instrument to display
	void setDefaultInstrument(ISIS::ISISInstrument inst);
	// Return header text colour
	QColor headerTextColour();
	// Set header text colour
	void setHeaderTextColour(QColor colour);
	// Return header cell colour
	QColor headerBGColour();
	// Set header cell colour
	void setHeaderBGColour(QColor colour);
	// Return highlight text colour
	QColor highlightTextColour();
	// Set highlight text colour
	void setHighlightTextColour(QColor colour);
	// Return highlight cell colour
	QColor highlightBGColour();
	// Set highlight cell colour
	void setHighlightBGColour(QColor colour);
	// Return document font
	QFont documentFont();
	// Set document font
	void setDocumentFont(QFont font);
	// Return plot font
	QFont plotFont();
	// Set plot font
	void setPlotFont(QFont font);
	// Return report plot font
	QFont reportPlotFont();
	// Set report plot font
	void setReportPlotFont(QFont font);
	// Return whether to highlight rows/groups in document
	bool highlightDocument();
	// Return whether to highlight rows/groups in document
	void setHighlightDocument(bool highlight);
	// Return document layout style
	JournalViewer::DocumentLayout documentLayout();
	// Set document layout style
	void setDocumentLayout(JournalViewer::DocumentLayout layout);
	// Return general document margins (mm)
	int* documentMargins();
	// Set general document margins (mm)
	void setDocumentMargins(int left, int top, int right, int bottom);
	// Return report document margins (mm)
	int* reportMargins();
	// Set report document margins (mm)
	void setReportMargins(int left, int top, int right, int bottom);
	// Return whether to auto-refresh current journal data
	bool autoReload();
	// Set whether to auto-refresh current journal data
	void setAutoReload(bool reload);
	// Return frequency (in minutes) of auto-refresh
	int autoReloadFrequency();
	// Set frequency (in minutes) of auto-refresh
	void setAutoReloadFrequency(int mins);
	// Return whether to force ISO-8859-1 encoding when reading XML files
	bool forceISOEncoding();
	// Set whether to force ISO-8859-1 encoding when reading XML files
	void setForceISOEncoding(bool b);


	/*
	 * Printing
	 */
	private:
	// Target print device
	QPrinter printer_;

	public:
	// Create document
	void createDocument(Document& document, const RefList<RunData,int>& data);
	// Create text document
	void createText(QString& string, const RefList<RunData,int>& data, QString separator = "\t");


	/*
	 * Instrument Definitions
	 */
	private:
	// List of available instruments
	List<Instrument> instruments_;
	// Currently-selected instrument
	Instrument* currentInstrument_;
	// Currently-selected journal entry
	Journal* currentJournal_;

	public:
	// Add new instrument
	bool addInstrument(ISIS::ISISInstrument instrmnt);
	// Set current Instrument
	void setInstrument(Instrument* inst);
	// Return current instrument
	Instrument* currentInstrument();
	// Return Instrument requested
	Instrument* instrument(ISIS::ISISInstrument instrmnt);
	// Set current Journal (loads data)
	void setJournal(Journal* jrnl);
	// Load specified journal data
	bool loadJournalData(Journal* jrnl, bool addUniqueOnly);

	public slots:
	// Update current journal data
	bool updateJournalData();


	/*
	 * Local Data
	 */
	private:

	public:
	// Update local journals
	void updateLocalJournals();


	/*
	 * Run Data
	 */
	public:
	// Search styles
	enum SearchStyle { TextStyle, WildStyle, RegExpStyle };

	private:
	// DataInterface for journal acquisition only
	DataInterface* dataInterface_;
	// RefList of run data
	RefList<RunData, Journal*> runData_;
	// List of unique users determined from runData_
	QList<QString> availableUsers_;
	// List of unique RB numbers determined from runData_
	QList<int> availableRB_;
	// List of user names associated to RB numbers in availableRB_ list
	QStringList availableRBUsers_;
	// Starting DateTime of earliest run
	QDateTime earliestRunStart_;
	// Starting DateTime of latest run
	QDateTime latestRunStart_;
	// First run number available
	int firstRunNumber_;
	// Last run number available
	int lastRunNumber_;
	// List of visible items, in order
	List<RunProperty> visibleProperties_;
	// Number of items in runData that are visible
	int nRunDataVisible_;
	// Whether view by group is enabled
	bool viewByGroup_;
	// Stored filter limits
	QString storedUserFilter_, storedRBFilter_;
	int storedFromRunFilter_, storedToRunFilter_;
	QDateTime storedFromDateFilter_, storedToDateFilter_;
	// Find text to which findMatches_ relates
	QString findText_;
	// Expression match style for findMatches_
	JournalViewer::SearchStyle findSearchStyle_;
	// Case sensitivity used for findMatches_
	bool findCaseSensitive_;
	// List of row indices that match the current Find search
	Array<int> findMatches_;
	// Integer index of last findMatches_ row selected
	int lastFindMatchIndex_;

	private:
	// Determine limits, including construction of unique lists
	void findFilterLimits();
	// Store current filter values
	void storeFilters();
	// Retrieve stored filter values
	void retrieveFilters();
	// Reset filters
	void resetFilters();
	// Create groups over visible RunData
	void createGroups();
	// Update data table
	void updateDataTable();
	// Filter run data
	void filterRunData();
	// Update data table highlighting
	void updateDataTableHighlighting();
	// Plot info from selected run data
	void plotSelectedRunData(RunData::BlockDataSource source, bool forceReload = false);
	// Create list of RunData from table
	RefList<RunData,int> getTableContents(bool selectionOnly);
	// Set visible flags of all current rundata
	void setAllRunDataVisible(bool state);
	// Return column containing specified RunProperty (or -1 if not visible)
	int runPropertyColumn(RunProperty::Property property);

	public:
	// Return List of RunData
	RefList<RunData,Journal*>& runData();
	// Return list of visible RunProperties
	List<RunProperty>& visibleRunProperties();
	// Set visible properties from column characters
	bool setVisibleProperties(const char* propertyString);
	// Set current Find, returning number of matches
	int find(QString text, JournalViewer::SearchStyle style, bool caseSensitive);
	// Return current number of find matches
	int nFindMatches();
	// Select (exclusively) next item in the current findMatches_
	bool findNext();
	// Select (exclusively) previous item in the current findMatches_
	bool findPrevious();


	/*
	 * CLI Control Interface
	 */
	public:
	// Change current journal
	bool changeJournal(const char* journalName);
	// Change current instrument
	bool changeInstrument(const char* instrumentName);
	// Show available journals for current instrument
	void showJournals();
	// Search for and display runs matching supplied string / search style
	bool searchRuns(QString searchString, QRegExp::PatternSyntax searchType);
};

#endif
