/*
	*** GetRoutines Interface Data
	*** src/get/interface_data.cpp
	Copyright T. Youngs 2012-2014

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

#include "get/interface.h"

SectionParameter GetInterface::parameters_[] = {
	{ "HDR",	3,	SectionParameter::Character,		80,	0 },	// HEADER BLOCK
	{ "VER1",	4,	SectionParameter::Integer,		1,	0 },	// FORMAT version number
	{ "ADD",	3,	SectionParameter::Integer,		10,	0 },	// SECTION addresses
	// RUN section
	{ "VER2",	4,	SectionParameter::Integer,		1,	0 },	// RUN section version #
	{ "RUN",	3,	SectionParameter::Integer,		1,	0 },	// RUN #
	{ "TITL",	4,	SectionParameter::Character,		80,	0 },	// RUN title
	{ "USER",	4,	SectionParameter::Character,		20,	8 },	// USER information
	{ "RPB",	3,	SectionParameter::Integer,		32,	0 },	// RUN parameter block
	{ "RRPB",	4,	SectionParameter::Float,		32,	0 },	// RUN parameter block (as reals)
	// INSTRUMENT section
	{ "VER3",	4,	SectionParameter::Integer,		1,	0 },	// INSTRUMENT section version #
	{ "NAME",	4,	SectionParameter::Character,		8,	0 },	// INSTRUMENT name
	{ "IVPB",	4,	SectionParameter::Integer,		64,	0 },	// INSTRUMENT parameter block  (REAL OR INTEGER???)
	{ "NDET",	4,	SectionParameter::Integer,		1,	0 },	// No. of detectors
	{ "NMON",	4,	SectionParameter::Integer,		1,	0 },	// No. of monitors
	{ "NUSE",	4,	SectionParameter::Integer,		1,	0 },	// No. of USER defined (UTn) tables  ***B
	{ "MDET",	4,	SectionParameter::Integer,		-1,	0 },	// spectrum no. for each monitor
	{ "MONP",	4,	SectionParameter::Integer,		-1,	0 },	// prescale value for each monitor
	{ "SPEC",	4,	SectionParameter::Integer,		-2,	0 },	// SPECTRUM # table
	{ "DELT",	4,	SectionParameter::Float,		-2,	0 },	// HOLD OFF table                    ***B
	{ "LEN2",	4,	SectionParameter::Float,		-2,	0 },	// L2 table
	{ "CODE",	4,	SectionParameter::Integer,		-2,	0 },	// CODE for UTn tables               ***B
	{ "TTHE",	4,	SectionParameter::Float,		-2,	0 },	// 2Theta table (scattering ang)
	{ "UT1",	3,	SectionParameter::Float,		-2,	0 },	// USER defined table 1              ***B
	// SE section
	{ "VER4",	4,	SectionParameter::Integer,		1,	0 },	// SE section version #
	{ "SPB",	3,	SectionParameter::Integer,		32,	0 },	// SAMPLE parameter block
	{ "NSEP",	4,	SectionParameter::Integer,		1,	0 },	// number of controlled SEPs
	{ "SE01",	4,	SectionParameter::Integer,		24,	0 },	// SE parameter block #1
	// ..cont. to SEnn" },
	// DAE section
	{ "VER5",	4,	SectionParameter::Integer,		1,	0 },	// DAE section version #
	{ "DAEP",	4,	SectionParameter::Integer,		64,	0 },	// DAE parameter block
	{ "CRAT",	4,	SectionParameter::Integer,		-2,	0 },	// crate no.for each detector
	{ "MODN",	4,	SectionParameter::Integer,		-2,	0 },	// module no. for each detector
	{ "MPOS",	4,	SectionParameter::Integer,		-2,	0 },	// position in module for each detector
	{ "TIMR",	4,	SectionParameter::Integer,		-2,	0 },	// TIME REGIME # table
	{ "UDET",	4,	SectionParameter::Integer,		-2,	0 },	// 'USER detector #' for each detector
	// Time channel boundaries section
	{ "VER6",	4,	SectionParameter::Integer,		1,	0 },	// TCB secton version #
	{ "NTRG",	4,	SectionParameter::Integer,		1,	0 },	// # of time regimes (normally =1)
	{ "NFPP",	4,	SectionParameter::Integer,		1,	0 },	// No. of frames per period
	{ "NPER",	4,	SectionParameter::Integer,		1,	0 },	// No. of periods
	{ "PMAP",	4,	SectionParameter::Integer,		1,	0 },	// period # for each basic period
	{ "NSP1",	4,	SectionParameter::Integer,		1,	0 },	// No. of spectra for time reg.= 1(tr=1)
	{ "NTC1",	4,	SectionParameter::Integer,		1,	0 },	// No. of time chan.              (tr=1)
	{ "TCM1",	4,	SectionParameter::Integer,		5,	0 },	// time channel mode (0,1,2, ..) (tr=1) *A
	{ "TCP1",	4,	SectionParameter::Float,		20,	0 },	// time channel parameters       (tr=1) *A
	{ "PRE1",	4,	SectionParameter::Integer,		1,	0 },	// prescale value for 32MHz clock (tr=1)
	// NSPn to PREn repeated for each time regime
	{ "TCB1",	4,	SectionParameter::Integer,		-3,	0 },	// Time channel boundaries        (tr=1), NTC == ntc1+1
	// TCBn repeated for each time regime
	// USER section
// 	{ "USER",	4,	SectionParameter::Float,		0,	0 },	// User defined data, max 400 R*4 words
	// DATA section
	{ "VER7",	4,	SectionParameter::Integer,		1,	0 },	// DATA version #
	{ "DAT1",	4,	SectionParameter::Integer,		-4,	0 }	// raw data for first time regime,  NTCSP == (ntc1.nsp1)
};

// Return string corresponding to parameter name
char* GetInterface::parameterName(ParameterName pname)
{
	return parameters_[pname].name;
}

// Return length of parameter name
int GetInterface::parameterNameLength(ParameterName pname)
{
	return parameters_[pname].nameLength;
}
