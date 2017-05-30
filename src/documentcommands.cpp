/*
	*** Document Commands
	*** src/documentcommands.cpp
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
#include "plotwidget.hui"
#include <QtPrintSupport/QPrinter>
#include <QFont>

/*
 * Document Command
 */

// Constructor
DocumentCommand::DocumentCommand() : ListItem<DocumentCommand>()
{
	command_ = DocumentCommand::nCommands;
	span_ = 0;
}

// Destructor
DocumentCommand::~DocumentCommand()
{
}

// Return command
DocumentCommand::Command DocumentCommand::command()
{
	return command_;
}

// Return span
int DocumentCommand::span()
{
	return span_;
}

// Return contents rectangle for display
QRect DocumentCommand::contentsRect(QPainter& painter, QRect initialRect, bool wordWrap)
{
	return initialRect;
}

// Draw content within specified bounding rectangle
void DocumentCommand::drawContent(QPainter& painter, QRect boundingRect)
{
	return;
}

/*
 * Add Column Command
 */

// Constructor
AddColumnCommand::AddColumnCommand(bool fitContents) : DocumentCommand()
{
	command_ = DocumentCommand::AddColumnCommand;
	column_ = new DocumentColumn();
	column_->setFitContents(fitContents);
}

// Destructor
AddColumnCommand::~AddColumnCommand()
{
	if (column_ != NULL) delete column_;
}

// Return column
DocumentColumn* AddColumnCommand::column()
{
	return column_;
}

// Execute command on target Document
void AddColumnCommand::execute(Document& target)
{
	// Call the column creation function - the fitContents parameter is not important in this context
	target.addColumn(false, column_);
}

/*
 * Background Colour Command
 */

// Constructor
BackgroundColourCommand::BackgroundColourCommand(QColor& colour)
{
	command_ = DocumentCommand::BackgroundColourCommand;
	colour_ = colour;
}

// Destructor
BackgroundColourCommand::~BackgroundColourCommand()
{
}

// Execute command on target Document
void BackgroundColourCommand::execute(Document& target)
{
	target.setBackgroundColour(colour_);
}

/*
 * Clear Columns Command
 */

// Constructor
ClearColumnsCommand::ClearColumnsCommand()
{
	command_ = DocumentCommand::ClearColumnsCommand;
}

// Destructor
ClearColumnsCommand::~ClearColumnsCommand()
{
}

// Execute command on target Document
void ClearColumnsCommand::execute(Document& target)
{
	target.clearColumns();
}

/*
 * End Row Command
 */

// Constructor
EndRowCommand::EndRowCommand()
{
	command_ = DocumentCommand::EndRowCommand;
}

// Destructor
EndRowCommand::~EndRowCommand()
{
}

// Execute command on target Document
void EndRowCommand::execute(Document& target)
{
	target.endRow();
}

/*
 * Font Command
 */

// Constructor
FontCommand::FontCommand(QFont& font)
{
	command_ = DocumentCommand::FontCommand;
	font_ = font;
}

// Destructor
FontCommand::~FontCommand()
{
}

// Execute command on target Document
void FontCommand::execute(Document& target)
{
	target.setFont(font_);
}

/*
 * Foreground Colour Command
 */

// Constructor
ForegroundColourCommand::ForegroundColourCommand(QColor& colour)
{
	command_ = DocumentCommand::ForegroundColourCommand;
	colour_ = colour;
}

// Destructor
ForegroundColourCommand::~ForegroundColourCommand()
{
}

// Execute command on target Document
void ForegroundColourCommand::execute(Document& target)
{
	target.setForegroundColour(colour_);
}

/*
 * Text Command
 */

// Constructor
TextCommand::TextCommand(QString& text, int span, Qt::AlignmentFlag alignment, bool bold, bool italic)
{
	command_ = DocumentCommand::TextCommand;
	text_ = text;
	span_ = span;
	alignment_ = alignment;
	bold_ = bold;
	italic_ = italic;
}

// Destructor
TextCommand::~TextCommand()
{
}

// Execute command on target Document
void TextCommand::execute(Document& target)
{
	target.addContent(this);
}

// Return contents rectangle for display
QRect TextCommand::contentsRect(QPainter& painter, QRect initialRect, bool wordWrap)
{
	QFont font = painter.font();
	font.setBold(bold_);
	font.setItalic(italic_);
	painter.setFont(font);
	int width = initialRect.width();
	QRect rect = painter.boundingRect(initialRect, wordWrap ? Qt::TextWordWrap : 0, text_);
// 	printf("Width of text '%s' (bold=%i) = %i\n", qPrintable(text_), bold_, rect.width());
	
	// Must 'reset' width of bounding rectangle, since the function returns the minimum width required, and will affect alignment of the text
	if (rect.width() < width) rect.setWidth(width);
	return rect;
}

// Draw content within specified bounding rectangle
void TextCommand::drawContent(QPainter& painter, QRect boundingRect)
{
// 	printf("Drawing text %s in region %f %f %f %f\n", qPrintable(text_), boundingRect.top(), boundingRect.left(), boundingRect.width(), boundingRect.height());
	QFont font = painter.font();
	font.setBold(bold_);
	font.setItalic(italic_);
	painter.setFont(font);
	painter.drawText(boundingRect, Qt::TextWordWrap | alignment_, text_);
}

/*
 * Plot Command
 */

// Constructor
PlotCommand::PlotCommand(PlotWidget& source, int span, double fractionalHeight) : plot_(source)
{
	command_ = DocumentCommand::PlotCommand;
	span_ = span;
	fractionalHeight_ = fractionalHeight;
}

// Destructor
PlotCommand::~PlotCommand()
{
}

// Execute command on target Document
void PlotCommand::execute(Document& target)
{
	target.addContent(this);
}

// Return contents rectangle for display
QRect PlotCommand::contentsRect(QPainter& painter, QRect initialRect, bool wordWrap)
{
	QRect rect = initialRect;
	rect.setHeight(fractionalHeight_*painter.device()->height());
	return rect;
}

// Draw content within specified bounding rectangle
void PlotCommand::drawContent(QPainter& painter, QRect boundingRect)
{
	// Draw direct on to QPainter
// 	plot_.draw(painter, boundingRect);

	// Create bitmap image of plot and then print image to QPainter
	const double inchesPerMetre = 39.3700787;
	QImage image(boundingRect.width()*5, boundingRect.height()*5, QImage::Format_RGB32);
	image.setDotsPerMeterX(inchesPerMetre*300);
	image.setDotsPerMeterY(inchesPerMetre*300);
	plot_.draw(image);
	painter.drawImage(boundingRect, image);
}
