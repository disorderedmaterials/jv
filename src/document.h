/*
	*** Document Class
	*** src/document.h
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

#ifndef JOURNALVIEWER_DOCUMENT_H
#define JOURNALVIEWER_DOCUMENT_H

#include <QString>
#include <QPainter>
#include <QColor>
#include <QtPrintSupport/QPrinter>
#include "documentcommands.h"
#include "list.h"
#include "reflist.h"

// Forward Declarations
/* none */

// Document Column
class DocumentColumn : public ListItem<DocumentColumn>
{
	public:
	// Constructor / Destructor
	DocumentColumn();

	private:
	// Whether column width is adjusted to fit contents
	bool fitContents_;
	// Set / required width for content
	int contentWidth_;
	// Current content to be displayed
	DocumentCommand* content_;
	// Bounding rectangle for content
	QRect contentsRect_;
	// Whether this is the last column in a particular row
	bool lastColumn_;

	public:
	// Set whether column width is adjusted to fit contents
	void setFitContents(bool fit);
	// Return whether column is expanding
	bool fitContents();
	// Set content width
	void setContentWidth(int width);
	// Return content width
	int contentWidth();
	// Pass specified content rectanglt through to get minimum content width required
	void passContentsRect(QRect contentsRect);
	// Set content to be displayed
	void setContent(DocumentCommand* cmd);
	// Return content to be displayed
	DocumentCommand* content();
	// Remove current content from column
	void removeContent();
	// Update contents rect for content
	void setContentsRect(QRect rect);
	// Return textRect for content
	QRect& contentsRect();
	// Flag that this column is the last one in aa particular row
	void setLastColumn();
	// Return whether this column is the last one in a particular row
	bool lastColumn();
};

// Document
class Document
{
	public:
	// Constructor / Destructor
	Document(QPainter& painter, QPrinter* printer = NULL);
	// Document Mode
	enum DocumentMode { Creation, Execution };

	private:
	// Target QPainter
	QPainter& painter_;
	// Target QPrinter (if any)
	QPrinter* printer_;


	/*
	 * Content
	 */
	private:
	// List of Commands
	List<DocumentCommand> commands_;
	// Whether we are caching commands or executing
	DocumentMode mode_;

	public:
	// Clear all commands
	void clearCommands();
	// Add column
	void addColumn(bool fitContents = true, DocumentColumn* column = NULL);
	// Add expanding column
	void addExpandingColumn();
	// Clear columns
	void clearColumns();
	// Set foreground colour
	void setForegroundColour(QColor colour);
	// Set background colour
	void setBackgroundColour(QColor colour);
	// Set both colours simultaneously
	void setColours(QColor fgcolour, QColor bgcolour);
	// Set font
	void setFont(QFont font);
	// Add text
	void addText(QString text, int span = 1, Qt::AlignmentFlag alignment = Qt::AlignLeft, bool bold = false, bool italic = false);
	// Add bold text
	void addBoldText(QString text, int span = 1, Qt::AlignmentFlag alignment = Qt::AlignLeft);
	// Add italic text
	void addItalicText(QString text, int span = 1, Qt::AlignmentFlag alignment = Qt::AlignLeft);
	// Add plot
	void addPlot(PlotWidget& source, int span = 1, double fracHeight = 0.1);
	// End current row
	void endRow();
	// Add content to column
	void addContent(DocumentCommand* content);


	/*
	 * Rendering
	 */
	private:
	// Current column setup
	RefList<DocumentColumn,int> currentColumns_;
	// Current column target
	RefListItem<DocumentColumn,int>* currentColumn_;
	// Current position on page
	int x_, y_;
	// Height of single line of text in current font
	int textHeight_;
	// Current foreground colour
	QColor foregroundColour_;
	// Current background colour
	QColor backgroundColour_;
	// Internal margin for cells
	int cellMargin_;

	private:
	// Recalculate current column widths
	void recalculateColumnWidths();

	public:
	// End document creation
	void end();
};

#endif
