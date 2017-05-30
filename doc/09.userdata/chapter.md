---
title: User (Local) Data
brief: Accessing locally-stored data with <strong>JournalViewer</strong>
taxonomy:
  category: docs
docroot: /jv/docs
header_class: alt
---

As well as retrieving journal and run data from network locations (i.e. when connected to the STFC network as an authenticated user) **JournalViewer** is able to probe and manage data stored locally in the [**User Data Directory**](/jv/docs/settings/access). The intended purpose of this is to allow facility users to make use of **JournalViewer**'s functionality on their collected data after having returned to their home institution, where no access to the original ISIS journal data is available.

The layout of the directory should be as follows - assuming that the location is set to `C:/Users/abc12345/MyExperimentData`, then one may organise user data as follows:

```
   C:/Users/abc12345/MyExperimentData
       RB1220486/
           NIMROD00014136.log
           NIMROD00014136.raw
           NIMROD00014137.log
           NIMROD00014137.raw
           ...
       RB1310475/
           NIMROD00018606.nxs
           NIMROD00018606.raw
           NIMROD00018607.nxs
           NIMROD00018607.raw
           ...
       SLS41093.raw
       SLS41094.raw
       SLS41095.raw
       ...
       WaterData/
           NIMROD00026207.nxs
           NIMROD00026207.raw
           NIMROD00026208.nxs
           NIMROD00026208.raw
       ...
```

The basic principle here is to store the files related to each distinct experiment in their own separate directories, the names of which will be displayed as individual entries in the **Data** selector (which replaces the **Cycle** selector when the instrument is set to LOCAL). For the example given above, the entries will be 'RB1220486', 'RB1310475', and 'WaterData'. Note that there are also some 'loose' files in the root of the user directory (`SLS41093.raw` etc.) - any files found in this directory will be added to a journal entry called 'Top'.

Before this data is available within **JournalViewer**, index files must be created by selecting **Tools&#8594;Regenerate Local Journals**. **JournalViewer** will then search through all subfolders in the specified user directory, extracting the necessary information from the `nxs` or `raw` files in order to build journals for them. Since this operation can be quite time consuming, it is never run automatically - as such, if files are changed, moved, or new `raw` or `nxs` files are added, the index files must be updated manually by selecting **Tools&#8594;Regenerate Local Journals**.

