A simple MorphOS shell tool dumping meta data from a sound file. Handles all formats supported by Reggae.
Requirements: MorphOS 3.13 (probably older ones too)
License: MIT

Usage:
	multimediatags "ram disk:foobar.mp3"
	multimediatags "ram disk:foobar.mp3" quiet - dump raw data without types, silence errors
	multimediatags "ram disk:foobar.mp3" quiet notitle noperformer noauthor notrack - dump just the album name

Building:
	Open the xprj file in Flow Studio
	or just ppc-morphos-gcc-9 -noixemul -O3 multimediatags.c

