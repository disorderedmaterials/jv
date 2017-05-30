/*
	*** JournalViewer Main
	*** src/main.cpp
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

#include "version.h"
#include "jv.h"
#include "messenger.hui"

int main(int argc, char *argv[])
{
	// Pragma for Windows build - Hides console
	#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

	/* Create the main QApplication */
	QApplication app(argc, argv);
	QCoreApplication::setOrganizationName("ProjectAten");
	QCoreApplication::setOrganizationDomain("www.projectaten.net");
#ifdef LITE
	QCoreApplication::setApplicationName("JournalViewer Lite");
#else
	QCoreApplication::setApplicationName("JournalViewer");
#endif

	/* Create the main object */
	JournalViewer jv;

	/* Check license */
	if (!jv.showLicense()) return -1;

	// Do we have CLI options?
	if (argc > 1)
	{
		// Initialise JV to load default journal etc.
		jv.initialise();
		msg.setToStdout(true);
		msg.print("Current instrument is %s (%s)\n", qPrintable(jv.currentInstrument()->ndxName()), qPrintable(jv.currentInstrument()->capitalisedName()));
		bool missingArg = false;

		int n = 1;
		while (n < argc)
		{
			if (argv[n][0] != '-')
			{
				msg.print("Encountered argument on command-line ('%s') when a switch was expected.\n", argv[n]);
				++n;
				continue;
			}

			// Command-line switch
			switch (argv[n][1])
			{
				case ('a'):
					if (!jv.searchRuns("*", QRegExp::WildcardUnix)) return 1;
					break;
				case ('c'):
					if (((n+1) == argc) || (argv[n+1][0] == '-')) missingArg = true;
					else if (!jv.setVisibleProperties(argv[++n])) return 1;
					break;
				case ('h'):
#ifdef LITE
					printf("JournalViewer Lite version %s\n\nAvailable CLI options are:\n\n", JVVERSION);
#else
					printf("JournalViewer version %s\n\nAvailable CLI options are:\n\n", JVVERSION);
#endif
					printf("\t-c <columns>\tSet the visible columns (properties) to display for matching run entries:\n");
					printf("\t\t\ta  Accumulated current in uAmps\n");
					printf("\t\t\tb  RB number\n");
					printf("\t\t\tc  Cycle\n");
					printf("\t\t\td  Duration of run\n");
					printf("\t\t\te  End date/time\n");
					printf("\t\t\tm  Mevents\n");
					printf("\t\t\tr  Run number\n");
					printf("\t\t\ts  Start date/time\n");
					printf("\t\t\tt  Title of run\n");
					printf("\t\t\tu  User\n");
					printf("\t-h\t\tShow this help\n");
					printf("\t-i <inst>\tChange to specified <instrument> ('CRISP', 'MUSR', 'SANS2D', 'SLS', 'TSC' etc.)\n");
					printf("\t-j <cycle>\tLoad journal for specified cycle ('All', '12/2', '09/1', '13/3' etc.)\n");
					printf("\t-l\t\tList available journals for current instrument\n");
					printf("\t-r <regexp>\tPerform a regular expression search of the current run data, displaying matching entries\n");
					printf("\t-s <text>\tPerform a plaintext search of the current run data, displaying matching entries\n");
					printf("\t-w <wildcard>\tPerform a wildcard search of the current run data, displaying matching entries\n");
					return 1;
					break;
				case ('i'):
					if (((n+1) == argc) || (argv[n+1][0] == '-')) missingArg = true;
					else if (!jv.changeInstrument(argv[++n])) return 1;
					break;
				case ('j'):
					if (((n+1) == argc) || (argv[n+1][0] == '-')) missingArg = true;
					else if (!jv.changeJournal(argv[++n]))
					{
						jv.showJournals();
						return 1;
					}
					break;
				case ('l'):
					jv.showJournals();
					return 0;
					break;
				case ('r'):
					if (((n+1) == argc) || (argv[n+1][0] == '-')) missingArg = true;
					else if (!jv.searchRuns(argv[++n], QRegExp::RegExp)) return 1;
					break;
				case ('s'):
					if (((n+1) == argc) || (argv[n+1][0] == '-')) missingArg = true;
					else if (!jv.searchRuns(argv[++n], QRegExp::FixedString)) return 1;
					break;
				case ('w'):
					if (((n+1) == argc) || (argv[n+1][0] == '-')) missingArg = true;
					else if (!jv.searchRuns(argv[++n], QRegExp::WildcardUnix)) return 1;
					break;
				case ('v'):
					msg.print("Version %s\n", JVVERSION);
					break;
				default:
					msg.print("Unrecognised command-line switch '%s'.\n", argv[n]);
					msg.print("Run with -h to see available switches.\n");
					return 1;
					break;
			}

			// Check for missing argument flag
			if (missingArg)
			{
				msg.print("Error: Argument expected but none was given for switch '%s'\n", argv[n]);
				return 1;
			}

			++n;
		}
	}
	else
	{
		jv.show();
		jv.initialise(true);
		
		/* Enter Qt's main event loop */
		return app.exec();
	}
	
	return 0;
}

