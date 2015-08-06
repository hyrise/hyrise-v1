#################
File organization
#################


Project layout
==============

//TODO


Header Protection Guards
========================

Every header should use the following style of protection guards::

    #ifndef HYRISE_$PART_$FILENAME
    #define HYRISE_$PART_$FILENAME
    // here comes the code
    #endif

Where $PART is the name of the binary or library in uppercase.
And $FILENAME is the filename in uppercase, replace special letters with underscores.

Example::

   src/lib/memory/this_myFile.h => #ifndef HYRISE_MEMORY_THIS_MYFILE_H...

**NOT: #pragma once, since this is sadly not generally supported.**

