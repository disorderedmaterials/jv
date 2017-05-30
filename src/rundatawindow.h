/*
	*** RunData Window
	*** src/rundatawindow.h
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

#ifndef JOURNALVIEWER_RUNDATAWINDOW_H
#define JOURNALVIEWER_RUNDATAWINDOW_H

#include "ui_rundatawindow.h"

// Forward Declarations
class RunData;

class RunDataWindow : public QDialog
{
	// All Qt declarations must include this macro
	Q_OBJECT


	/*
	// Window Functions
	*/
	public:
	// Constructor / Destructor
	RunDataWindow(QWidget* parent, QFont& font);
	~RunDataWindow();
	// Main form declaration
	Ui::RunDataWindow ui;

	private:
	// Whether window is currently refreshing
	bool refreshing_;

	private:
	// Update analysis tree
	void updateAnalysisTree();
	// Export run property data
	void exportRunPropertyData(QString property = "");

	public:
	// Add RunData to GraphWidget
	void addRunData(RunData* rd);
	// Finalise and show GraphWidget
	void finaliseAndShow();


	/*
	 * Widget Slots
	 */
	private slots:
	// SECI/RunLog Graph
	// -- Run List item changed
	void on_RunList_itemChanged(QListWidgetItem* item);
	// -- Run Property item changed
	void on_RunPropertyList_itemChanged(QListWidgetItem* item);
	// -- Absolute time check clicked
	void on_AbsoluteTimeCheck_clicked(bool checked);
	// -- Begin/End markers check clicked
	void on_ShowBeginEndCheck_clicked(bool checked);
	// -- Show Legend check clicked
	void on_ShowLegendCheck_clicked(bool checked);
	// -- On Change check clicked
	void on_OnChangeCheck_clicked(bool checked);
	// -- Export button clicked
	void on_ExportButton_clicked(bool checked);
	// Context menu event
	void runPropertyList_contextMenuEvent(const QPoint& pos);
	// Analysis Tab
	// -- Tree item collapsed
	void on_AnalysisTree_itemCollapsed(QTreeWidgetItem* item);
	// -- Tree item expanded
	void on_AnalysisTree_itemExpanded(QTreeWidgetItem* item);
	// -- Copy to clipboard button clicked
	void on_AnalysisCopyToClipboardButton_clicked(bool checked);
	// Dialog
	// -- Close button
	void on_CloseButton_clicked(bool checked);
};

#endif
