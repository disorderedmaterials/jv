/*
	*** ISIS Instrument Data
	*** src/isis_data.cpp
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

#include "isis.h"

// Instrument Data
InstrumentInfo ISIS::instruments_[] = {
	// Name		ShortName	Location
	{ "LOCAL",	"LOCAL",	Local },
	{ "ALF",	"ALF",	 	TS1 },
	{ "ARGUS",	"ARGUS", 	Muon },
	{ "CHIPIR",	"CHIPIR",	TS2 },
	{ "CRISP",	"CSP", 		TS1 },
	{ "EMMA",	"EMMA",		TS1,	"ndxemma", "EMMA-A" },
	{ "EMU",	"EMU",		Muon },
	{ "ENGINX",	"ENG", 		TS1 },
	{ "GEM",	"GEM", 		TS1 },
	{ "HET",	"HET", 		TS1 },
	{ "HIFI",	"HIFI",		Muon },
	{ "HRPD", 	"HRP", 		TS1 },
	{ "IMAT",	"IMAT", 	TS2 },
	{ "INES", 	"INS", 		TS1 },
	{ "INTER",	"INTER",	TS2 },
	{ "IRIS", 	"IRS", 		TS1 },
	{ "IRIS_SET", 	"IRIS_SETUP",		TS1,	"ndxiris", "IRIS_SETUP" },
	{ "LARMOR", 	"LARMOR", 	TS2 },
	{ "LET",	"LET", 		TS2 },
	{ "LOQ",	"LOQ", 		TS2 },
	{ "MAPS", 	"MAP", 		TS1 },
	{ "MARI", 	"MAR", 		TS1 },
	{ "MERLIN",	"MER", 		TS1 },
	{ "MUSR",	"MUS",		Muon },
	{ "NIMROD",	"NIMROD",	TS2 },
	{ "OFFSPEC",	"OFFSPEC",	TS2 },
	{ "OSIRIS", 	"OSI",		TS1 },
	{ "PEARL", 	"PRL",		TS1 },
	{ "POLARIS", 	"POL",		TS1 },
	{ "POLREF",	"POLREF",	TS2 },
	{ "SANDALS",	"SLS",		TS1 },
	{ "SANS2D",	"SANS2D",	TS2 },
	{ "SURF",	"SRF",		TS1 },
	{ "SXD",	"SXD",		TS1 },
	{ "TOSCA",	"TSC",		TS1 },
	{ "VESUVIO",	"EVS", 		TS1, 	"ndxevs" },
	{ "WISH",	"WISH",		TS2 },
	{ "ZOOM",	"ZOOM",		TS2 },
	{ "CHRONUS",	"CHRONUS",		TS1 }
};

