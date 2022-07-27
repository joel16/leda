# LEDA Reverse Engineering Project

This is a reverse engineered version of LEDA - Legacy Software Loader, version 0.1, by Dark_AleX.


# How you can help:
1. Get a copy of leda.prx from the [latest ME version 2.3](http://www.mediafire.com/download/6cz8ofj44a42wme/release_661me2.3+%28OFW+Version%29.zip)
2. Extract leda.prx and load it into your favourite hex editor.
3. Find the gzip magic (0x1F 0x8B 08 00 ...) and copy everything from the magic to the end of the file and save it as a new PRX file (leda_dec.prx).
4. Disassemble the new prx with prxtool ``prxtool -w leda_dec.prx -o leda_dec.txt``
5. Start Reversing :P


# Credits:
- Valantin/leda: https://github.com/Valantin/leda/blob/master/leda.c
