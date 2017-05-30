/*
	*** Two-Dimensional Data Storage
	*** src/data2d.h
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

#ifndef JOURNALVIEWER_DATA2D_H
#define JOURNALVIEWER_DATA2D_H

#include "array.h"
#include "enumeration.h"
#include "reflist.h"
#include <QStringList>
#include <QDateTime>

#define OPTOLERANCE 1.0e-6

// Forward Declarations
/* none */

// Data2DValue
class Data2DValue
{
	public:
	// Constructors
	Data2DValue(double y = 0.0, EnumeratedValue* enumeratedY = NULL);
	// Copy constructor
	Data2DValue(const Data2DValue& source);
	// Assignment Operaor
	void operator=(const Data2DValue& source);
	// Assignment Operaor
	void operator=(double y);
	// Assignment Operaor
	void operator=(EnumeratedValue* value);

	private:
	// Real value
	double y_;
	// Enumerated index (or -1 for none)
	EnumeratedValue* enumeratedY_;

	public:
	// Return value of y, from whichever is the valid source
	double y() const;
	// Return real value
	double constY() const;
	// Return enumerated index
	EnumeratedValue* constEnumeratedY() const;
};

// Data2D
class Data2D : public ListItem<Data2D>
{
	public:
	// Constructor
	Data2D();
	// Destructor
	~Data2D();
	// Copy Constructor
	Data2D(const Data2D& source);
	// Clear data
	void clear();


	/*!
	 * \name Data
	 */
	///@{
	private:
	// Run start time (i.e. time origin) for stored data (x values are relative to this point)
	QDateTime runTimeStart_;
	// Run time end
	QDateTime runTimeEnd_;
	// Abcissa
	Array<int> x_;
	// Data values
	Array<Data2DValue> y_;
	// Name
	QString name_;
	// Name of parent group
	QString groupName_;
	// Reference list of enumerated Y values used
	RefList<EnumeratedValue,int> enumeratedY_;

	private:
	// Resize arrays
	void resize(int size);
	// Reset arrays to zero
	void reset();
	
	public:
	// Initialise arrays to specified size
	void initialise(int size);
	// Return current array size
	int arraySize();
	// Return number of defined datapoints
	int nPoints() const;
	// Return time value specified
	int x(int index) const;
	// Return x Array
	Array<int>& arrayX();
	// Set y value
	void setY(int index, double y);
	// Return y value specified
	Data2DValue y(int index) const;
	// Return y Array
	Array<Data2DValue>& arrayY();
	// Add relative data point
	void addRelativePoint(QDateTime time, double y);
	// Add relative data point (enumerated value)
	void addRelativePoint(QDateTime time, EnumeratedValue* enumy);
	// Add normal data point
	void addPoint(int x, double y);
	// Set time origin and endpoint for run
	void setRunTimeSpan(QDateTime origin, QDateTime endTime);
	// Return run time start (time origin) for data
	QDateTime runTimeStart();
	// Return run time end as relative x offset from runTimeStart_
	int runTimeEndAsX();
	// Set name of data
	void setName(QString name);
	// Return name of data
	QString name() const;
	// Set name of parent group
	void setGroupName(QString name);
	// Return name of parent group
	QString groupName() const;
	// Return list of referenced enumerated values
	const RefList<EnumeratedValue,int>& enumeratedY();
	///@}


	/*!
	 * \name Operators
	 */
	///@{
	public:
	// Assignment Operator
	void operator=(const Data2D& source);
	///@}


	/*!
	 * \name General Functions
	 */
	///@{
	public:
	// Calculate and return average of all data points
	double totalAverage();
	// Determine number of datapoints within run begin/end
	int runNPoints();
	// Calculate time covered by datapoints within run begin/end
	double runTimeCoverage();
	// Calculate and return average over run begin/end
	double runAverage(bool& noValue);
	// Return minimum value over run begin/end
	double runMinimum(bool& noValue);
	// Return maximum value over run begin/end
	double runMaximum(bool& noValue);
	// Return standard deviation over run begin/end
	double runStandardDeviation();
	// Calculate and return average over all points
	double average(bool& noValue);
	// Return minimum value over all points
	double minimum(bool& noValue);
	// Return maximum value over all points
	double maximum(bool& noValue);
	///@}
};

#endif
