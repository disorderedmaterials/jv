---
title: CLI Usage
brief: <strong>JournalViewer</strong>'s command-line interface
taxonomy:
  category: docs
docroot: /jv/docs
header_class: alt
---

Occasionally it is useful to perform a quick search of journal data without loading the user interface, and for this purpose **JournalViewer** has a basic command-line interface.  Run without any command-line arguments, **JournalViewer** will always start up with the GUI. Available command-line options are:

| Option | Description |
|--------|-------------|
| -a | List all run data for the current journal |
| -c columns | Set the run property columns to display for search output. If the option is omitted the current columns visible in the main GUI table are used. Otherwise, a string of characters representing the required columns can be given (listed in the table below). |
| -h | Display help for all available command-line options (i.e. the contents of this table) |
| -i instrument | Change target instrument to that provided. Long (e.g. SANDALS) or short (e.g. SLS) names may be provided.  If the option is omitted the default instrument is used. |
| -j cycle | Change target journal to the ISIS operating cycle provided. Note that only the YY/N part should be given (e.g. '15/4'). The string 'All' is also accepted, and targets all available cycles in the subsequent search.  If the option is omitted the journal for the most recent cycle is used. |
| -l | Lists all available journals for the current instrument and exit |
| -r "regular expression" | Perform a regular expression search of the run titles in the current journal, printing those that match. |
| -s "plaintext string" | Perform a plain text search of the run titles in the current journal, printing those that match. |
| -w "wildcard string" | Perform a wildcard text search of the run titles in the current journal, printing those that match. |
| -v | Display version information and exit | 

| Character | Column |
|-----------|--------|
| a | Accumulated current in &mu;Amps |
| b | RB number |
| c | Cycle |
| d | Duration of run |
| e | End date/time of run |
| m | Accumulated Mevents |
| r | Run number |
| s | Start date/time of run |
| t | Title of run |
| u | User |

## Examples

To search the current (most recent) journal for the current instrument, looking for the word 'Vanadium' in the title:
```
bob@pc:~>  jv -s Vanadium
```

To search for any 'Empty' run in a cryostat (for instance):
```
bob@pc:~>  jv -w "Empty*cryostat"
```

For a set of sample cells labelled N1 through to N10, display all empty cell runs for the current instrument / journal, excluding those for N4 through to N9, using a regular expression search:
```
bob@pc:~>  jv -r "Empty N[^4-9]"
```

Change the current instrument to IRIS, the current cycle to 11/2, and display all runs for that journal:
```bob@pc:~>  jv -i IRIS -j 11/2 -a
```

For the current instrument, change to cycle 09/5, and display the run number, title, start/end date/times, and accumulated current, for all runs in that journal:
```
bob@pc:~>  jv -j 09/5 -c rtse -a
```

