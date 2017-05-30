---
title: Operation
brief: Basic method of operation
taxonomy:
  category: docs
docroot: /jv/docs
header_class: alt
---

While local to each 'NDH' instrument computer, the journal files for every beamline are also mirrored to a network filestore location (http://data.isis.rl.ac.uk/journals). It is these files that **JournalViewer** reads from the network, parses, and displays, as its primary function. This means that, while on the local network (potentially via VPN) every stored journal file for every beamline is available within **JournalViewer** from any computer. Moreover, if the location of the ISIS data archive is set, **JournalViewer** will also be able to plot block value data from any run file detailed within the journal. The journal files accessed can optionally be saved to a specified local directory, meaning basic run information can be viewed even when off-site (without VPN) or offline.

**JournalViewer** is also able to traverse through directories of run data (i.e. `raw`, `log`, and/or `nxs` files) stored on a local machine, permitting block value interrogation to be performed offline as well. This is of particular use to external users who wish to use the software to process / view the run data associated with their experiments, and a cut-down version of the software exists for distribution outside of ISIS.  This version removes any ability to access journals via the network, and only allows selection / interrogation of local data stored on the user's machine. Otherwise the functionality remains the same.

