/*
	*** Printing Classes
	*** src/printing.cpp
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

#include "document.h"
#include <QtPrintSupport/QPrinter>

/*
 * Document Column
 */

// Constructor
DocumentColumn::DocumentColumn() : ListItem<DocumentColumn>()
{
	fitContents_ = false;
	contentWidth_ = 0;
	content_ = NULL;
	lastColumn_ = false;
};

// Set whether column adjusts widthto fit contents
void DocumentColumn::setFitContents(bool fit)
{
	fitContents_ = fit;
}

// Return whether column adjusts size to fit contents
bool DocumentColumn::fitContents()
{
	return fitContents_;
}

// Set content width
void DocumentColumn::setContentWidth(int width)
{
	contentWidth_ = width;
}

// Return column width
int DocumentColumn::contentWidth()
{
	return contentWidth_;
}

// Pass specified text through to get minimum width required
void DocumentColumn::passContentsRect(QRect contentsRect)
{
	if (contentsRect.width() > contentWidth_) contentWidth_ = contentsRect.width();
}

// Set content to be displayed
void DocumentColumn::setContent(DocumentCommand* cmd)
{
	content_ = cmd;
}

// Return content to be displayed
DocumentCommand* DocumentColumn::content()
{
	return content_;
}

// Remove current content from column
void DocumentColumn::removeContent()
{
	content_ = NULL;
}

// Update contents rect for content
void DocumentColumn::setContentsRect(QRect rect)
{
	contentsRect_ = rect;
}

// Return textRect for content
QRect& DocumentColumn::contentsRect()
{
	return contentsRect_;
}

// Flag that this column is the last one in aa particular row
void DocumentColumn::setLastColumn()
{
	lastColumn_ = true;
}

// Return whether this column is the last one in a particular row
bool DocumentColumn::lastColumn()
{
	return lastColumn_;
}

/*
 * Document
 */

// Constructor
Document::Document(QPainter& painter, QPrinter* printer) : painter_(painter)
{
	cellMargin_ = 2;
	printer_ = printer;
	mode_ = Document::Creation;
}

// Clear all commands
void Document::clearCommands()
{
	commands_.clear();
	currentColumns_.clear();
	currentColumn_ = NULL;
}

// Add column
void Document::addColumn(bool fitContents, DocumentColumn* column)
{
	if (mode_ == Document::Creation)
	{
		// Create command
		AddColumnCommand* cmd = new AddColumnCommand(fitContents);
		commands_.own(cmd);
		
		// Add new column to currentColumns_
		currentColumns_.add(cmd->column());
		if (currentColumn_ == NULL) currentColumn_ = currentColumns_.first();
	}
	else
	{
		// Add column to current list
		currentColumns_.add(column);
		if (currentColumn_ == NULL) currentColumn_ = currentColumns_.first();
		if (column->lastColumn()) recalculateColumnWidths();
	}
}

// Add expanding column
void Document::addExpandingColumn()
{
	addColumn(true);
}

// Clear columns
void Document::clearColumns()
{
	if (mode_ == Document::Creation)
	{
		// Add command
		commands_.own(new ClearColumnsCommand());
	}

	// Clear currentColumns_
	currentColumn_ = NULL;
	currentColumns_.clear();
}

// Set foreground colour
void Document::setForegroundColour(QColor colour)
{
	if (mode_ == Document::Creation)
	{
		// Create command
		commands_.own(new ForegroundColourCommand(colour));
	}
	else foregroundColour_ = colour;
}

// Set background colour
void Document::setBackgroundColour(QColor colour)
{
	if (mode_ == Document::Creation)
	{
		// Create command
		commands_.own(new BackgroundColourCommand(colour));
	}
	else backgroundColour_ = colour;
}

// Set both colours simultaneously
void Document::setColours(QColor fgcolour, QColor bgcolour)
{
	if (mode_ == Document::Creation)
	{
		// Create commands
		commands_.own(new ForegroundColourCommand(fgcolour));
		commands_.own(new BackgroundColourCommand(bgcolour));
	}
	else
	{
		foregroundColour_ = fgcolour;
		backgroundColour_ = bgcolour;
	}
}

// Set font
void Document::setFont(QFont font)
{
	if (mode_ == Document::Creation)
	{
		// Create commands
		commands_.own(new FontCommand(font));
	}
	else
	{
		painter_.setFont(font);
	}
}

// Add text
void Document::addText(QString text, int span, Qt::AlignmentFlag alignment, bool bold, bool italic)
{
	if (mode_ == Document::Creation)
	{
		// Create command
		TextCommand* cmd = new TextCommand(text, span, alignment, bold, italic);
		commands_.own(cmd);

		// Pass content to column for size determination
		if (currentColumn_ == NULL) printf("Warning - Added text '%s' has no column.\n", qPrintable(text));
		else
		{
			if (span == 1) currentColumn_->item->passContentsRect( cmd->contentsRect(painter_, QRect()) );
			currentColumn_ = currentColumn_->next;
		}
	}
	else printf("Called Document::addText() with a string when executing document.\n");
}

// Add bold text
void Document::addBoldText(QString text, int span, Qt::AlignmentFlag alignment)
{
	addText(text, span, alignment, true, false);
}

// Add italic text
void Document::addItalicText(QString text, int span, Qt::AlignmentFlag alignment)
{
	addText(text, span, alignment, false, true);
}

// Add graph
void Document::addPlot(PlotWidget& source, int span, double fracHeight)
{
	if (mode_ == Document::Creation)
	{
		// Create command
		commands_.own(new PlotCommand(source, span, fracHeight));

		// Pass content to column for size determination
		if (currentColumn_ == NULL) printf("Warning - Added graph has no column.\n");
		else currentColumn_ = currentColumn_->next;
	}
	else printf("Called Document::addPlot() when executing document.\n");
}

// End current row
void Document::endRow()
{
	if (mode_ == Document::Creation)
	{
		// Create command
		commands_.own(new EndRowCommand());

		// Set current last column
		if (currentColumns_.last() != NULL) currentColumns_.last()->item->setLastColumn();
	}
	else
	{
		// Print row contents to page.

		// Primary Loop
		x_ = 0;
		int maxHeight = 0, columnWidth;
		QRect rect, contentsRect;
		// -- Loop over column content and find the one requiring the biggest vertical space
		for (RefListItem<DocumentColumn,int>* ri = currentColumns_.first(); ri != NULL; ri = ri->next)
		{
			DocumentColumn* col = ri->item;
			DocumentCommand* cmd = col->content();
			
			// If no content is associated to the column, we're done with the loop
			if (cmd == NULL) break;
			
			// Get initial bounding rectangle for column (or spanned column)
			columnWidth = 0;
			if (cmd->span() == 1) columnWidth = col->contentWidth();
			else if (cmd->span() == -1) for (RefListItem<DocumentColumn,int>* rj = ri->next; rj != NULL; rj = rj->next) columnWidth += rj->item->contentWidth();
			else
			{
				int n = cmd->span();
				for (RefListItem<DocumentColumn,int>* rj = ri; rj != NULL; rj = rj->next, --n)
				{
					if (n == 0) break;
					if (rj != ri) columnWidth += 2*cellMargin_;
					columnWidth += rj->item->contentWidth();
				}
			}
			
			rect.setRect(x_+cellMargin_, y_+cellMargin_, columnWidth, textHeight_);
			contentsRect = cmd->contentsRect(painter_, rect, true);
			col->setContentsRect(contentsRect);

			if (contentsRect.height() > maxHeight) maxHeight = contentsRect.height();
			x_ += columnWidth + 2*cellMargin_;
		}

		// If we're using a QPrinter, check maxHeight - if there isn't enough space on the page, must start a new page...
		if (printer_ && (maxHeight > (painter_.device()->height() - y_)))
		{
			if (!printer_->newPage()) printf("Failed to start new page when printing.\n");
			y_ = 0;

			// Reset contentsRects of columns to be at y = 0
			for (RefListItem<DocumentColumn,int>* ri = currentColumns_.first(); ri != NULL; ri = ri->next)
			{
				DocumentColumn* col = ri->item;
				col->contentsRect().moveTop(0);
			}
		}

		// Draw row background
		painter_.setBrush(backgroundColour_);
		painter_.setPen(Qt::NoPen);
		rect.setRect(0, y_, x_, maxHeight + 2*cellMargin_);
		painter_.drawRect(rect);
		painter_.setPen(foregroundColour_);

		// Second  Loop
		// -- Print actual content
		for (RefListItem<DocumentColumn,int>* ri = currentColumns_.first(); ri != NULL; ri = ri->next)
		{
			DocumentColumn* col = ri->item;
			DocumentCommand* cmd = col->content();

			// If no content is associated to the column, we're done with the loop
			if (cmd == NULL) break;

			cmd->drawContent(painter_, col->contentsRect());
			x_ += col->contentWidth() + 2*cellMargin_;

			// Cleanup - unset content pointer for next row
			col->removeContent();
		}

		// Move to start next row
		y_ += maxHeight + 2*cellMargin_;
		currentColumn_ = currentColumns_.first();
	}

	// Reset column pointer
	currentColumn_ = currentColumns_.first();
}

// Add content to column
void Document::addContent(DocumentCommand* content)
{
	if (mode_ == Document::Execution)
	{
		if (currentColumn_ == NULL) printf("Warning - ran out of columns when trying to add content.\n");
		else
		{
			// Set this item to be the content of the current column
			currentColumn_->item->setContent(content);
			currentColumn_ = currentColumn_->next;
		}
	}
	else printf("Called Document::addContent() with a DocumentCommand pointer when still creating document.\n");
}

/*
 * Rendering
 */

// Recalculate current column widths
void Document::recalculateColumnWidths()
{
	// First, get total column usage by non-expanding columns
	int nExpanding = 0, width = 0;
	for (RefListItem<DocumentColumn,int>* ri = currentColumns_.first(); ri != NULL; ri = ri->next)
	{
		DocumentColumn* col = ri->item;
		if (col->fitContents()) width += col->contentWidth() + 2*cellMargin_;
		else ++nExpanding;
	}

	// Get space remaining horizontally
	double remainder = painter_.device()->width() - width - 2*cellMargin_*nExpanding;

	// Check for excess or lack of space
	if (remainder > 0)
	{
		// If there are no expanding columns, divide any remaining space amongst the other columns
		// Otherwise, increase expanding columns
		for (RefListItem<DocumentColumn,int>* ri = currentColumns_.first(); ri != NULL; ri = ri->next)
		{
			DocumentColumn* col = ri->item;
			if (nExpanding > 0)
			{
				if (col->fitContents()) continue;
				col->setContentWidth(remainder / nExpanding);
			}
			else col->setContentWidth( col->contentWidth() + remainder / currentColumns_.nItems());
		}
	}
	else if (remainder < 0)
	{
		// TODO
	}
}

// Render document to specified device
void Document::end()
{
	// Clear lists ready for use
	currentColumns_.clear();
	currentColumn_ = NULL;
	DocumentColumn* column;
	
	// Determine line height of text
	QRectF contentsRect = painter_.boundingRect(QRectF(), Qt::AlignCenter, "AQjyqg");
	textHeight_ = contentsRect.height();

	// Set initial position on page
	x_ = 0;
	y_ = 0;

	// Draw printable area border as a test
// 	contentsRect.setRect(0.0, 0.0, painter_.device()->width(), painter_.device()->height());
// 	painter_.setBrush(Qt::red);
// 	painter_.drawRect(contentsRect);
// 	painter_.setPen(QPen());

	// Set state to Execution, and loop over commands
	mode_ = Document::Execution;
	
	// Loop over defined commands...
	for (DocumentCommand* cmd = commands_.first(); cmd != NULL; cmd = cmd->next) cmd->execute(*this);
}