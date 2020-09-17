/*

Copyright 2020 Jacek Piszczek

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

*/

#include <proto/multimedia.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/charsets.h>
#include <proto/intuition.h>
#include <proto/multimedia.h>

#include <libraries/charsets.h>
#include <dos/dos.h>
#include <dos/rdargs.h>

#include <classes/multimedia/multimedia.h>
#include <classes/multimedia/sound.h>
#include <classes/multimedia/metadata.h>

// ReadArgs template and a structure representing the data
#define ARGSSTR "FILE,QUIET/S,NOTITLE/S,NOALBUM/S,NOPERFORMER/S,NOAUTHOR/S,NOTRACK/S"
struct Args {
	CONST_STRPTR file;
	LONG quiet;
	LONG notitle;
	LONG noalbum;
	LONG noperformer;
	LONG noauthor;
	LONG notrack;
};
static struct Args args;

// Dumps a single tag
static void printTag(struct MetaItem *mti, CONST_STRPTR header)
{
	// Obtain the byte needed to store a converted string
	// We use MIBENUM_SYSTEM since that's somethign that'll look best in a shell
	ULONG bytes = GetByteSize(mti->mi_Data, mti->mi_Length, MIBENUM_UTF_32BE, MIBENUM_SYSTEM);
	UBYTE *tag = NULL;

	if (bytes > 0)
	{
		// Allocate memory for converted string...
		tag = (UBYTE *)AllocVec(bytes, MEMF_ANY);
		if (tag)
		{
			// Convert the string
			if (-1 == ConvertTagList((APTR)mti->mi_Data, mti->mi_Length, (APTR)tag, bytes, MIBENUM_UTF_32BE, MIBENUM_SYSTEM, NULL))
			{
				// And account for errors...
				FreeVec(tag);
				tag = NULL;
			}
		}
	}

	// Print an empty string if conversion has failed (or it really was empty)
	if (args.quiet)
		Printf("%s\n", tag ? (const char *)tag : "");
	else
		Printf("%s: %s\n", header, tag ? (const char *)tag : "");

	// Free the converted string
	if (tag)
	{
		FreeVec(tag);
	}
}

// Dumps an integer tag
static void printIntTag(struct MetaItem *mti, CONST_STRPTR header)
{
	if (args.quiet)
		Printf("%ld\n", *(LONG *)mti->mi_Data);
	else
		Printf("%s: %ld\n", header, *(LONG *)mti->mi_Data);
}

static BOOL printTags(CONST_STRPTR file)
{
	// Reggae file open tags
	struct TagItem tags[] = {
		// Could just as easily handle a networked asset here, but let's make it simple
		{ MMA_StreamType, (IPTR)"file.stream" },
		// Pass the file path...
		{ MMA_StreamName, (IPTR)file }, 
		// Limit the accepted types to sound
		{ MMA_MediaType, MMT_SOUND },
		// Force enough processing to make sure metadata is decoded
		{ MMA_Decode, TRUE },
		{ TAG_DONE, 0 }
	};

	// Open the file / create the Media object
	Object *mmObject = MediaNewObjectTagList(tags);
	if (mmObject)
	{
		// Obtain the meta data structure
		struct MetaItem *mti = (struct MetaItem *)MediaGetPort(mmObject, 0, MMA_MetaData); 

		// Go through the meta data
		if (mti)
		{
			while(mti->mi_Id)
			{
				switch(mti->mi_Id)
				{
				case MMETA_Title:
					if (!args.notitle)
						printTag(mti, "Title");
					break;
				case MMETA_Performer:
					if (!args.noperformer)
						printTag(mti, "Performer");
					break;
				case MMETA_Album:
					if (!args.noalbum)
						printTag(mti, "Album");
					break;
				case MMETA_Author:
					if (!args.noauthor)
						printTag(mti, "Author");
					break;
				case MMETA_TrackNum:
					if (!args.notrack)
						printIntTag(mti, "Track");
					break;
				}
				mti++;
			}
		}
		else
		{
			if (!args.quiet)
				Printf("Metadata not found\n");
		}

		// Dispose the Reggae object		
		DisposeObject(mmObject);

		// Report success if meta data was found
		return mti != NULL;
	}
	else
	{
		if (!args.quiet)
			Printf("Failed opening %s\n", file);

		return FALSE;
	}
}

int main(void)
{
	// Check the input attributes...
	struct RDArgs *rdargs = ReadArgs((STRPTR)ARGSSTR, (LONG *)&args, 0);
	ULONG rc = 0;

	if (rdargs)
	{
		if (args.file)
		{
			if (!printTags(args.file))
				rc = 20;
		}
		else if (!args.quiet)
		{
			Printf("No file specified\n");
			rc = 20;
		}

		// Free rdargs
		FreeArgs(rdargs);
	}
	
	return rc;
}
