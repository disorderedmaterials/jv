/*
	*** PlotWidget PaintEvent
	*** src/plotwidget_paintevent.cpp
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

#include <QPainter>
#include <QVBoxLayout>
#include <QMouseEvent>
#include "plotwidget.hui"
#include <math.h>
#include <algorithm>

// Widget PaintEvent
void PlotWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);

	// Setup global transform for widget
	globalTransform_.reset();
	areaHeight_ = painter.device()->height();
	areaWidth_ = painter.device()->width();
	renderPlot(painter);

	// Done.
	painter.end();
}

// Create image of current graph
void PlotWidget::draw(QPainter& painter, QRect boundingRect)
{
	// Set up transform to shift image to correct place on graph
	globalTransform_.reset();
	globalTransform_.translate(boundingRect.left(), boundingRect.top());
	areaWidth_ = boundingRect.width();
	areaHeight_ = boundingRect.height();
	renderPlot(painter);
	painter.setTransform(QTransform());
}

// Draw plot on specified image
void PlotWidget::draw(QImage& image)
{
	QPainter painter(&image);
	globalTransform_.reset();
	areaWidth_ = image.width();
	areaHeight_ = image.height();
	renderPlot(painter);
	painter.end();
}

// Render plot
void PlotWidget::renderPlot(QPainter& painter)
{
	// Setup plot variables etc.
	plotSetup(painter);

	// Draw axes and titles
	drawAxes(painter);
	drawTitles(painter);

	// Draw on shaded background
	if (shadeBackground_) drawShadedBackground(painter);

	// Draw begin / end shading
	if (showBeginEnd_) drawGroupPeriods(painter);
	
	// Draw data
	drawData(painter);

	// Draw data legend
	if (showLegend_) drawLegend(painter);

	// Draw selection box
	drawSelection(painter);
}

// Setup plot for drawing
void PlotWidget::plotSetup(QPainter& painter)
{
	// Get text line height
	QRectF tempRect = painter.boundingRect(QRectF(), Qt::AlignLeft, "xyz");
	textHeight_ = tempRect.height();

	// Set font point size
	font_.setBold(false);
	font_.setItalic(false);
	painter.setFont(font_);

	// Determine maximal label widths for each axis.
	QString testText;
	// --- X
	xValueRect_ = painter.boundingRect(QRectF(), Qt::AlignHCenter, "000:00:00");
	// --- Y
	testText = QString::number(yLogarithmic_ ? log10(yMin_) : yMin_*1.01, 'g', 4);
	yValueRect_ = painter.boundingRect(QRectF(), Qt::AlignRight | Qt::AlignVCenter, testText);
	testText = QString::number(yLogarithmic_ ? log10(yMax_) : yMax_*1.01, 'g', 4);
	tempRect = painter.boundingRect(QRectF(), Qt::AlignRight | Qt::AlignVCenter, testText);
	if (tempRect.width() > yValueRect_.width()) { yValueRect_.setLeft(tempRect.left()); yValueRect_.setRight(tempRect.right()); }

	// Determine if we are to display enumerated Y values and, if so, work out the longest text string we'll display
	// TODO Don't do this every time - only do it when the yMin_ or yMax_ has changed?
	numericalYAxis_ = false;
	enumeratedYAxis_ = false;
	enumeratedYValues_.clear();
	for (PlotData* pd = dataSets_.first(); pd != NULL; pd = pd->next)
	{
		// Is this dataset visible?
		if (!pd->visible()) continue;

		// Loop over enumerated values used by this dataset
		const RefList<EnumeratedValue,int>& enums = pd->enumeratedY();
		for (RefListItem<EnumeratedValue,int>* ri = enums.first(); ri != NULL; ri = ri->next)
		{
			EnumeratedValue* ev = ri->item;
			if ((ev->value() < yMin_) || (ev->value() > yMax_)) continue;
			tempRect = painter.boundingRect(QRectF(), Qt::AlignRight, ev->name());
			if (tempRect.width() > yValueRect_.width()) { yValueRect_.setLeft(tempRect.left()); yValueRect_.setRight(tempRect.right()); }
			enumeratedYAxis_ = true;
			enumeratedYValues_.add(ev);
		}

		if (pd->enumeratedY().nItems() == 0) numericalYAxis_ = true;
	}

	// Work out areas
	if (absoluteTime_) xAxisArea_.setRect(spacing_, areaHeight_ - 3*spacing_ - 3*textHeight_, areaWidth_-2*spacing_, 3*textHeight_ + 3*spacing_);
	else xAxisArea_.setRect(spacing_, areaHeight_ - 2*spacing_ - 2*textHeight_, areaWidth_-2*spacing_, 2*textHeight_ + 2*spacing_);
	yAxisArea_.setRect(spacing_, spacing_, yValueRect_.width() + 2*spacing_ + textHeight_, areaHeight_-2*spacing_);
	graphArea_.setRect(yAxisArea_.right(), 2*spacing_+textHeight_, areaWidth_-2*spacing_-yAxisArea_.width(), areaHeight_-2*spacing_-textHeight_-xAxisArea_.height());
	xScale_ = graphArea_.width()/(xMax_-xMin_);
	yScale_ = graphArea_.height()/(yMax_-yMin_);

	// If graph area has changed since last redraw then the scales will also have changed, so must recalculate tick deltas
	if ((graphArea_ != lastGraphArea_) || (fabs(xScale_-lastXScale_) > 1e-5) || (fabs(yScale_-lastYScale_) > 1e-5)) calculateTickDeltas(10);
	
	// Loop over all datasets and recreate painterpaths (if necessary)
	for (PlotData* pd = dataSets_.first(); pd != NULL; pd = pd->next)
	{
		if (!pd->visible()) continue;
		pd->generatePainterPaths(xScale_, xLogarithmic_, yScale_, yLogarithmic_, onChangeData_);
	}
	lastGraphArea_ = graphArea_;
	lastXScale_ = xScale_;
	lastYScale_ = yScale_;

	// Setup a 'data space' transform to give 0,0 at llh corner of graphArea_
	dataTransform_ = globalTransform_;
	dataTransform_.translate(graphArea_.left(), graphArea_.bottom());
	dataTransform_.scale(xScale_, -yScale_);
	dataTransform_.translate(-xMin_, -yMin_);

	// Setup a 'local' transformation matrix which will use local widget coordinates
	localTransform_ = globalTransform_;
	localTransform_.translate(graphArea_.left(), graphArea_.bottom());
	localTransform_.scale(1.0,-1.0);

	// Calculate axis start values, based on current tick deltas
	if (absoluteTime_)
	{
		QDateTime tempRef(absoluteTimeOrigin_.date().addDays(-1));
		xAxisTickStart_ = int(tempRef.secsTo(absoluteTimeOrigin_.addSecs(xMin_)) / xAxisTickDelta_);
		xAxisTickStart_ *= xAxisTickDelta_;
		xAxisTickStart_ -= tempRef.secsTo(absoluteTimeOrigin_);
		if (xAxisTickStart_ < 0.0) xAxisTickStart_ += xAxisTickDelta_;
	}
	else
	{
		xAxisTickStart_ = int(xMin_ / xAxisTickDelta_) + (xMin_ > 0.0 ? 1 : 0);
		xAxisTickStart_ *= xAxisTickDelta_;
	}
	yAxisTickStart_ = int(yMin_ / yAxisTickDelta_) + (yMin_ > 0.0 ? 1 : 0);
	yAxisTickStart_ *= yAxisTickDelta_;

	// Setup brushes
	backgroundBrush_ = QBrush(backgroundColour_, Qt::SolidPattern);
	
	// Setup painter
	painter.setBackground(backgroundBrush_);
	painter.setRenderHint(QPainter::TextAntialiasing);

	// Clear background
	painter.setTransform(globalTransform_);
	painter.setBrush(backgroundBrush_);
	painter.drawRect(0, 0, areaWidth_-1, areaHeight_-1);
	
	// Draw simple outline around graph area
	painter.drawRect(graphArea_);
}

// Draw shaded background
void PlotWidget::drawShadedBackground(QPainter& painter)
{
	if (!singleProperty_) return;

	// Setup global transform and no clipping
	painter.setTransform(globalTransform_);
	painter.setClipping(false);
	painter.setPen(foregroundColour_);
	painter.setRenderHint(QPainter::Antialiasing, false);

	// Move to data space
	painter.setTransform(dataTransform_);

	// Setup initial QRect for background
	QRectF shadeRect(xMin_, yMin_, 0, yMax_-yMin_);
	painter.setPen(Qt::NoPen);
	QColor colour;
	double yavg;

	// Loop over data/datasets
	for (PlotData* pd = dataSets_.first(); pd != NULL; pd = pd->next)
	{
		// Is dataset visible?
		if (!pd->visible()) continue;

		QTransform modifiedTransform = dataTransform_;
		// -- Set correct x-origin for data
		if (absoluteTime_) modifiedTransform.translate( (absoluteTimeOrigin_.secsTo(pd->runTimeStart())-xMin_), 0.0);
		painter.setTransform(modifiedTransform);

		// Loop over datapoints in this dataset
		Data2D& data = pd->data();
		for (int n=0; n<data.nPoints()-1; ++n)
		{
			// If the current X value is less than the graph minimum, skip on
			if (data.x(n) < xMin_) continue;
			if (data.x(n) > xMax_) break;

			// Set right-hand side of shadeRect to the current X value
			shadeRect.setLeft(data.x(n));
			shadeRect.setRight(data.x(n+1));

			// Determine colour to use
			yavg = (data.y(n).y() + data.y(n+1).y())*0.5;
			if (yavg <= shadedBackgroundMinimumY_) colour = shadedBackgroundMinimumColour_;
			else if (yavg >= shadedBackgroundMaximumY_) colour = shadedBackgroundMaximumColour_;
			else colour = backgroundColour_;
			colour.setAlpha(50);

			// Draw rect
			painter.setBrush(colour);
			painter.drawRect(shadeRect);
		}
	}
}

// Draw gridlines
void PlotWidget::drawGridLines(QPainter& painter) 
{
	// Move to data space
	painter.setTransform(dataTransform_);

	// Draw axis gridlines
	QPen grayPen(Qt::gray);
	grayPen.setStyle(Qt::DashLine);
	grayPen.setWidth(1.5f);
	painter.setPen(grayPen); 
	double xpos, ypos, absxpos;
	QLineF line;
	int n;

	// --- X
	if (xLogarithmic_)
	{
		// Start at lowest integer log value equal to or below xMin_
		double xBase = floor(xMin_);
		xpos = xBase;
		int count = 0;
		while (xpos <= xMax_)
		{
			if (xpos >= xMin_)
			{
				line.setLine(xpos, yMin_, xpos, yMax_);
				painter.drawLine(line);
			}
			// Increase tick counter, value, and power if necessary
			++count;
			if (count == 10)
			{
				count = 0;
				xBase += 1.0;
			}
			xpos = xBase + log10(count);
		}
	}
	else
	{
		xpos = xAxisTickStart_;
		do
		{
			line.setLine(xpos, yMin_, xpos, yMax_);
			painter.drawLine(line);
			xpos += xAxisTickDelta_;
			printf("XPOS = %f\n", xpos);
		} while (xpos <= xMax_);
	}

	// --- Y
	if (yLogarithmic_)
	{
		// Start at lowest integer log value equal to or below yMin_
		double yBase = floor(yMin_);
		ypos = yBase;
		int count = 0;
		while (ypos <= yMax_)
		{
			if (ypos >= yMin_)
			{
				line.setLine(xMin_, ypos, xMax_, ypos);
				painter.drawLine(line);
			}
			// Increase tick counter, value, and power if necessary
			++count;
			if (count == 10)
			{
				count = 0;
				yBase += 1.0;
			}
			ypos = yBase + log10(count);
		}
	}
	else
	{
		ypos = yAxisTickStart_;
		do
		{
			line.setLine(xMin_, ypos, xMax_, ypos);
			painter.drawLine(line);
			ypos += yAxisTickDelta_;
			printf("YPOS = %f\n", ypos);
		} while (ypos <= yMax_);
	}

	// Draw 'zero' axis gridlines
	painter.setPen(Qt::black);
	painter.setBrush(Qt::NoBrush);
	// -- Zero Y
	line.setLine(xMin_, 0.0, xMax_, 0.0);
	painter.drawLine(line);
	// -- Zero X (only if in relative time)
	if (! absoluteTime_)
	{
		line.setLine(0.0, yMin_, 0.0, yMax_);
		painter.drawLine(line);
	}
}

// Draw run group period
void PlotWidget::drawGroupPeriods(QPainter& painter)
{
	// Setup global transform and no clipping
	painter.setTransform(globalTransform_);
	painter.setClipping(false);
	painter.setPen(foregroundColour_);
	painter.setRenderHint(QPainter::Antialiasing, false);

	// Draw highlights for run begin/end, before any gridlines or data
	// We will store the x limits in the PlotDataGroup for label plotting later...
	QColor colour;
	QRectF tempRect;
	QString label;
	double xStart, xEnd;
	for (PlotDataGroup* pg = dataSetGroups_.first(); pg != NULL; pg = pg->next)
	{
		// Is dataset visible?
		if (!pg->visible()) continue;
		if (pg->nDataSetsVisible() == 0) continue;

		// Setup basic rectangle
		if (absoluteTime_)
		{
			xStart = absoluteTimeOrigin_.secsTo(pg->runBeginDateTime());
			xEnd = absoluteTimeOrigin_.secsTo(pg->runEndDateTime());
		}
		else
		{
			xStart = 0.0;
// 			xEnd = pd->timeOrigin().secsTo(pg->endDateTime());
		}

		// Check visibility
		if (xStart > xMax_) continue;
		if (xEnd < xMin_) continue;
		
		// Clamp to graph min/max
		if (xStart < xMin_)
		{
			xStart = xMin_;
			pg->setRunXMin(xStart, false);
		}
		else pg->setRunXMin(xStart, true);
		if (xEnd > xMax_)
		{
			xEnd = xMax_;
			pg->setRunXMax(xEnd, false);
		}
		else pg->setRunXMax(xEnd, true);

		// Create rect, color, and draw it
		colour = pg->colour();
		colour.setAlpha(50);
		painter.setBrush(colour);
		tempRect.setRect(graphArea_.left() + (xStart-xMin_)*xScale_, graphArea_.top(), (xEnd - xStart)*xScale_, graphArea_.height());
		painter.drawRect(tempRect);

		// Draw on label
		label.sprintf("%c%s%c", pg->runTrueXMin() ? '|' : '<', qPrintable(pg->name()), pg->runTrueXMax() ? '|' : '>');
		tempRect = painter.boundingRect(QRectF(), Qt::AlignCenter, label);
		tempRect.translate(((pg->runXMax()-pg->runXMin())*0.5 + pg->runXMin() - xMin_)*xScale_ + graphArea_.left(), graphArea_.top() - textHeight_);

		painter.drawText(tempRect, Qt::AlignCenter, label);
	}	
}

// Draw data
void PlotWidget::drawData(QPainter& painter)
{
	// Setup clipping to the main graph area, and turn on antialiasing
	painter.setTransform(globalTransform_);
	painter.setClipping(true);
	painter.setClipRect(graphArea_);
	painter.setRenderHint(QPainter::Antialiasing, true);

	QTransform modifiedTransform;
	QPen dataPen;
	dataPen.setWidthF(1.5);
	int count = 0;
	for (PlotData* pd = dataSets_.first(); pd != NULL; pd = pd->next)
	{
		// Is dataset visible?
		if (!pd->visible()) continue;
		
		// Take copy of the current transformation matrix, and modify it as necessary
		modifiedTransform = localTransform_;
// 		dataTransform.translate(0.0, verticalSpacing_*pd->verticalOffset()); TODO
		// -- Set correct x-origin for data
		if (absoluteTime_) modifiedTransform.translate( (absoluteTimeOrigin_.secsTo(pd->runTimeStart())-xMin_)*xScale_, -yMin_*yScale_);
		else modifiedTransform.translate(-xMin_*xScale_, -yMin_*yScale_);

		// Update Painter transform, pen, and draw the dataset path
		painter.setTransform(modifiedTransform);
		pd->stylePen(dataPen);
		painter.setPen(dataPen);
		painter.setBrush(Qt::NoBrush);
		painter.drawPath(pd->linePath());
		painter.setBrush(Qt::SolidPattern);
		painter.drawPath(pd->symbolPath());
	}
}

// Draw selection box
void PlotWidget::drawSelection(QPainter& painter)
{
	if (buttons_&Qt::LeftButton)
	{
		// Setup unclipped drawing in globalTransform_
		painter.setTransform(globalTransform_);
		painter.setClipping(false);
		painter.setPen(foregroundColour_);
		painter.setRenderHint(QPainter::Antialiasing, false);

		QRectF box(clickedWidgetPosition_, currentWidgetPosition_);

		QPen pen;
		pen.setColor(Qt::gray);
		pen.setStyle(Qt::DotLine);

		painter.setBrush(Qt::NoBrush);
		painter.setPen(pen);
		painter.drawRect(box);
	}
}

// Draw axis tick marks / labels
void PlotWidget::drawAxes(QPainter& painter)
{
	int count;
	double ypos, xpos, lastPos, deviceXpos, deviceYpos, yBase, xBase;
	QRectF labelRect, labelRect2;
	QLineF line;
	int hh, mm, ss;
	QString timeString;
	QDateTime xDateTime;
	QPen labelPen, gridLinePen;
	gridLinePen.setColor(Qt::gray);
	gridLinePen.setStyle(Qt::DashLine);
	painter.setPen(labelPen);

	// Setup global transform and no clipping
	painter.setTransform(globalTransform_);
	painter.setClipping(false);
	painter.setPen(foregroundColour_);
	painter.setRenderHint(QPainter::Antialiasing, true);

	// -- Draw X tick marks and labels
	count = 0;
	xpos = xLogarithmic_ ? floor(xMin_) : xAxisTickStart_;
	xBase = xpos;
	deviceYpos = xAxisArea_.top();
	labelRect = xValueRect_;
	labelRect.moveTop(deviceYpos+spacing_);
	labelRect2 = labelRect.translated(0.0, textHeight_);
	lastPos = 0;
	do
	{
		// Set device position
		deviceXpos = graphArea_.left() + (xpos - xMin_)*xScale_;
		labelRect.moveLeft(deviceXpos - xValueRect_.width()*0.5);
		labelRect2.moveLeft(deviceXpos - xValueRect_.width()*0.5);

		// If we are above xMin_, draw tick mark / gridline / value
		if (xpos >= xMin_)
		{
			// Draw tick mark (always)
			line.setLine(deviceXpos, deviceYpos, deviceXpos, deviceYpos+2.0);
			painter.drawLine(line);

			// Set time
			xDateTime = absoluteTimeOrigin_.addSecs( xLogarithmic_ ? pow(10.0,xpos) : xpos);

			// Draw text label
			if (count == 0)
			{
				// Check for overlap between this label and the last one
				if ((labelRect.left()-5) > lastPos)
				{
					// Draw larger tick mark since there is a value here
					line.setLine(deviceXpos, deviceYpos, deviceXpos, deviceYpos+spacing_);
					painter.drawLine(line);
					if (absoluteTime_)
					{
						painter.drawText(labelRect, Qt::AlignHCenter, xDateTime.time().toString("hh:mm:ss"));
						painter.drawText(labelRect2, Qt::AlignHCenter, xDateTime.date().toString("dd/MM/yy"));
					}
					else
					{
						hh = fabs(xpos) / 3600;
						mm = (fabs(xpos) - hh*3600) / 60;
						ss = fabs(xpos) - hh*3600 - mm*60;
						if (xpos < 0.0) painter.drawText(labelRect, Qt::AlignHCenter, timeString.sprintf("-%i:%02i:%02i", hh, mm, ss));
						else painter.drawText(labelRect, Qt::AlignHCenter, timeString.sprintf("%i:%02i:%02i", hh, mm, ss));
					}
					
					lastPos = labelRect.right();
				}

				// This is a major tick line (with label) so draw a grid line
				painter.setPen(gridLinePen);
				line.setLine(deviceXpos, deviceYpos, deviceXpos, deviceYpos - graphArea_.height());
				painter.drawLine(line);
				painter.setPen(labelPen);
			}
		}

		// Update ypos and counter
		if (xLogarithmic_)
		{
			++count;
			if (count == 10)
			{
				count = 0;
				xBase += 1.0;
				xpos = xBase;
			}
			else xpos = xBase + log10(count);
		}
		else xpos += xAxisTickDelta_;
	} while (xpos <= xMax_);

	// -- Draw Y tick marks and labels
	if (numericalYAxis_)
	{
		count = 0;
		ypos = yLogarithmic_ ? floor(yMin_) : yAxisTickStart_;
		yBase = ypos;
		deviceXpos = yAxisArea_.right() - 2.0;
		labelRect = yValueRect_;
		labelRect.moveRight(deviceXpos);
		lastPos = graphArea_.bottom() - 1;
		do
		{
			// Set device position
			deviceYpos = graphArea_.bottom() - (ypos - yMin_)*yScale_;
			labelRect.moveTop(deviceYpos - yValueRect_.height()*0.5);

			// If we are above yMin_, draw tick mark / gridline / value
			if (ypos >= yMin_)
			{
				// Draw tick mark (always)
				line.setLine(deviceXpos, deviceYpos, deviceXpos+2.0, deviceYpos);
				painter.drawLine(line);

				// Draw text label
				if (count == 0)
				{
					// -- Check for overlap between this label and the last one
					if (labelRect.bottom() < lastPos)
					{
						if (yLogarithmic_) painter.drawText(labelRect, Qt::AlignRight, QString::number(pow(10,ypos), 'g', 4));
						else painter.drawText(labelRect, Qt::AlignRight, QString::number(ypos, 'g', 4));
						lastPos = labelRect.top();
					}
				}
			}

			// Update ypos and counter
			if (yLogarithmic_)
			{
				++count;
				if (count == 10)
				{
					count = 0;
					yBase += 1.0;
					ypos = yBase;
				}
				else ypos = yBase + log10(count);
			}
			else ypos += yAxisTickDelta_;
		} while (ypos <= yMax_);
	}
	if (enumeratedYAxis_)
	{
		deviceXpos = yAxisArea_.right() - 2.0;

		// Loop over RefList of enumerated values we created earlier...
		for (RefListItem<EnumeratedValue,int>* ri = enumeratedYValues_.first(); ri != NULL; ri = ri->next)
		{
			EnumeratedValue* ev = ri->item;
			// Calculate ypos for this enum
			deviceYpos = graphArea_.bottom() - (ev->value() - yMin_)*yScale_;
			         labelRect = yValueRect_;
			         labelRect.translate(deviceXpos, deviceYpos);
			painter.drawText(labelRect, Qt::AlignRight, ev->name());
		}
	}
}

// Draw axis titles
void PlotWidget::drawTitles(QPainter& painter)
{
	// Setup global transform and no clipping
	painter.setTransform(globalTransform_);
	painter.setClipping(false);
	painter.setPen(foregroundColour_);
	painter.setRenderHint(QPainter::Antialiasing, true);

	// Draw time period labels
	QString label;
	QRectF tempRect;

	// Draw X-axis title
	xAxisArea_.setLeft( graphArea_.left() );
	xAxisArea_.moveTop( xAxisArea_.top() + (spacing_ + textHeight_) * (absoluteTime_ ? 2.0 : 1.0));
	painter.drawText(xAxisArea_, Qt::AlignHCenter | Qt::AlignTop, "Time (hh:mm:ss)");
	
	// Draw Y-axis title(s)
	QTransform transform = globalTransform_;
	transform.translate(0, graphArea_.center().y());
	transform.rotate(-90);
	painter.setTransform(transform);
	tempRect.setRect(-0.5*graphArea_.height(), 0.5*(textHeight_+spacing_), graphArea_.height(), textHeight_);
	// First, loop over group data to work out maximum rect width required
	yAxisTitle_ = "";
	for (PlotDataBlock* pdb = dataSetBlocks_.first(); pdb != NULL; pdb = pdb->next)
	{
		if (pdb->visible())
		{
			if (!yAxisTitle_.isEmpty()) yAxisTitle_ += " / ";
			yAxisTitle_ += pdb->blockName();
		}
	}

	painter.drawText(tempRect, Qt::AlignHCenter | Qt::AlignVCenter, yAxisTitle_);
}

// Draw legend
void PlotWidget::drawLegend(QPainter& painter)
{
	// Setup unclipped drawing in globalTransform_
	painter.setTransform(globalTransform_);
	painter.setClipping(false);
	painter.setPen(foregroundColour_);
	painter.setRenderHint(QPainter::Antialiasing, true);

	QPen legendBoxPen, linePen;
	legendBoxPen.setWidth(1.0f);
	legendBoxPen.setColor(Qt::black);
	linePen.setWidth(2.0f);

	// Draw legend (if visible, and more than one dataset block is visible)
	if (nDataSetBlocksVisible_ > 0)
	{
		// Loop over datasets and determine maximum extent needed for titles
		QRectF textRect, tempRect;
		int nDataSets = 0;
		for (PlotDataBlock* pdb = dataSetBlocks_.first(); pdb != NULL; pdb = pdb->next)
		{
			// Is dataset to be drawn?
			if (!pdb->visible()) continue;

			tempRect = painter.boundingRect(QRectF(), Qt::AlignLeft, pdb->blockName());
			if (tempRect.width() > textRect.width()) textRect = tempRect;
			++nDataSets;
		}

		// Create rectangle for legend area
		int legendSpacing = 2, legendLineLength = 20;
		QRectF legendRect = textRect;
		legendRect.setWidth( textRect.width() + 3 * legendSpacing + legendLineLength );
		legendRect.moveRight(width()-spacing_-1);
		legendRect.moveTop(graphArea_.top());
		legendRect.setHeight( textRect.height() * nDataSets + (nDataSets+1) * legendSpacing );
		textRect.moveRight(width() - legendSpacing - spacing_);
		textRect.moveTop(legendRect.top() + legendSpacing);

		// Draw semi-transparent box to underpin legend
		painter.setRenderHint(QPainter::Antialiasing, false);
		QBrush legendBrush(backgroundColour_, Qt::SolidPattern);
		painter.setBrush(legendBrush);
		painter.setPen(legendBoxPen);
		painter.drawRect(legendRect);
		QLine legendLine(legendRect.left() + legendSpacing, legendRect.top()+legendSpacing+textRect.height()/2, legendRect.left() + legendSpacing + legendLineLength, legendRect.top()+legendSpacing+textRect.height()/2);

		// Draw on dataSet names and representative lines...
		for (PlotDataBlock* pdb = dataSetBlocks_.first(); pdb != NULL; pdb = pdb->next)
		{
			// Is dataset visible?
			if (!pdb->visible()) continue;

			painter.drawText(textRect, Qt::AlignLeft, pdb->blockName());
			textRect.moveTop( textRect.top() + textRect.height() + legendSpacing);
			
			linePen.setDashPattern(pdb->dashes());
			painter.setPen(linePen);
			painter.drawLine(legendLine);
			legendLine.translate(0, textRect.height()+legendSpacing);
		}
	}
}
