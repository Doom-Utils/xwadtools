//
// This program originally was written by Randy Heit <rheit@iastate.edu>.
//
// Please note that this source is modified so that it compiles
// with ANSI C++ compilers on various platforms without a bunch
// of warnings.
//
// 28 November 1999
// Udo Munk <munkudo@aol.com>
//

#include <iostream.h>
#include <iomanip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "listacs.h"
#include "strfunc.h"

WadFile::WadFile ()
{
	File = NULL;
	NumLumps = 0;
	InfoTable = NULL;
	InfoTableOfs = 0;
}

WadFile::WadFile (char *filename)
{
	WadFile ();
	Open (filename);
}

WadFile::~WadFile ()
{
	Close ();
}

bool WadFile::Ok (void)
{
	return !!File && !!NumLumps;
}

bool WadFile::Open (char *filename)
{
	wadinfo info;

	if (File)
		Close ();

	if ((File = fopen (filename, "rb"))) {
		if (fread (&info, sizeof(info), 1, File) == 1) {
			if (!strncmp (info.identification, "IWAD", 4) ||
				!strncmp (info.identification, "PWAD", 4)) {
				NumLumps = info.numlumps;
				InfoTableOfs = info.infotableofs;
				if (RefreshInfoTable ()) {
					return true;
				}
			}
		}
		fclose (File);
		File = NULL;
	}

	return false;
}

void WadFile::Close (void)
{
	if (File) {
		fclose (File);
		File = NULL;
	}

	if (InfoTable) {
		delete[] InfoTable;
		InfoTable = NULL;
	}

	NumLumps = 0;
	InfoTableOfs = 0;
}

bool WadFile::RefreshInfoTable (void)
{
	if (!File || NumLumps == 0)
		return false;

	if (InfoTable)
		delete[] InfoTable;

	if ((InfoTable = new filelump[NumLumps])) {
		if (!fseek (File, InfoTableOfs, SEEK_SET))
			if (fread (InfoTable, sizeof(filelump), NumLumps, File) == NumLumps)
				return true;
		delete[] InfoTable;
	}

	InfoTable = NULL;

	return false;
}

int WadFile::CheckNumForName (char *name)
{
	union {
		char	s[9];
		int	x[2];
	} name8;

	int		v1;
	int		v2;
	filelump*	lump_p;

	strncpy (name8.s,name,8);
	name8.s[8] = 0;
	strupr (name8.s);

	v1 = name8.x[0];
	v2 = name8.x[1];

	lump_p = InfoTable;

	while (lump_p != InfoTable + NumLumps)
	{
		if ( *(int *)lump_p->name == v1
			 && *(int *)&lump_p->name[4] == v2)
		{
			return lump_p - InfoTable;
		}
		lump_p++;
	}

	return -1;
}

int WadFile::LumpLength (int lump)
{
	if (InfoTable && ((unsigned)lump < NumLumps))
		return InfoTable[lump].size;
	else
		return 0;
}

bool WadFile::ReadLump (int lump, void *dest)
{
	if (!InfoTable || ((unsigned)lump >= NumLumps))
		return false;

	if (!fseek (File, InfoTable[lump].filepos, SEEK_SET))
		return fread (dest, 1, InfoTable[lump].size, File) == InfoTable[lump].size;

	return false;
}

char *WadFile::GetLumpName (int lump)
{
	if (InfoTable && ((unsigned)lump < NumLumps))
		return InfoTable[lump].name;
	else
		return NULL;
}

int WadFile::GetNumLumps (void)
{
	return NumLumps;
}


Behavior::code Behavior::Codes[NUM_CODES] = {
	{ "NOP",				0 },
	{ "TERMINATE",				0 },
	{ "SUSPEND",				0 },
	{ "PUSHNUMBER",				1 },
	{ "LSPEC1",				1 },
	{ "LSPEC2",				1 },
	{ "LSPEC3",				1 },
	{ "LSPEC4",				1 },
	{ "LSPEC5",				1 },
	{ "LSPEC1DIRECT",			2 },
	{ "LSPEC2DIRECT",			3 },
	{ "LSPEC3DIRECT",			4 },
	{ "LSPEC4DIRECT",			5 },
	{ "LSPEC5DIRECT",			6 },
	{ "ADD",				0 },
	{ "SUBTRACT",				0 },
	{ "MULTIPLY",				0 },
	{ "DIVIDE",				0 },
	{ "MODULUS",				0 },
	{ "EQ",					0 },
	{ "NE",					0 },
	{ "LT",					0 },
	{ "GT",					0 },
	{ "LE",					0 },
	{ "GE",					0 },
	{ "ASSIGNSCRIPTVAR",			1 },
	{ "ASSIGNMAPVAR",			1 },
	{ "ASSIGNWORLDVAR",			1 },
	{ "PUSHSCRIPTVAR",			1 },
	{ "PUSHMAPVAR",				1 },
	{ "PUSHWORLDVAR",			1 },
	{ "ADDSCRIPTVAR",			1 },
	{ "ADDMAPVAR",				1 },
	{ "ADDWORLDVAR",			1 },
	{ "SUBSCRIPTVAR",			1 },
	{ "SUBMAPVAR",				1 },
	{ "SUBWORLDVAR",			1 },
	{ "MULSCRIPTVAR",			1 },
	{ "MULMAPVAR",				1 },
	{ "MULWORLDVAR",			1 },
	{ "DIVSCRIPTVAR",			1 },
	{ "DIVMAPVAR",				1 },
	{ "DIVWORLDVAR",			1 },
	{ "MODSCRIPTVAR",			1 },
	{ "MODMAPVAR",				1 },
	{ "MODWORLDVAR",			1 },
	{ "INCSCRIPTVAR",			1 },
	{ "INCMAPVAR",				1 },
	{ "INCWORLDVAR",			1 },
	{ "DECSCRIPTVAR",			1 },
	{ "DECMAPVAR",				1 },
	{ "DECWORLDVAR",			1 },
	{ "GOTO",				1 },
	{ "IFGOTO",				1 },
	{ "DROP",				0 },
	{ "DELAY",				0 },
	{ "DELAYDIRECT",			1 },
	{ "RANDOM",				0 },
	{ "RANDOMDIRECT",			2 },
	{ "THINGCOUNT",				0 },
	{ "THINGCOUNTDIRECT",			2 },
	{ "TAGWAIT",				0 },
	{ "TAGWAITDIRECT",			1 },
	{ "POLYWAIT",				0 },
	{ "POLYWAITDIRECT",			1 },
	{ "CHANGEFLOOR",			0 },
	{ "CHANGEFLOORDIRECT",			2 },
	{ "CHANGECEILING",			0 },
	{ "CHANGECEILINGDIRECT",		2 },
	{ "RESTART",				0 },
	{ "ANDLOGICAL",				0 },
	{ "ORLOGICAL",				0 },
	{ "ANDBITWISE",				0 },
	{ "ORBITWISE",				0 },
	{ "EORBITWISE",				0 },
	{ "NEGATELOGICAL",			0 },
	{ "LSHIFT",				0 },
	{ "RSHIFT",				0 },
	{ "UNARYMINUS",				0 },
	{ "IFNOTGOTO",				1 },
	{ "LINESIDE",				0 },
	{ "SCRIPTWAIT",				0 },
	{ "SCRIPTWAITDIRECT",			1 },
	{ "CLEARLINESPECIAL",			0 },
	{ "CASEGOTO",				2 },
	{ "BEGINPRINT",				0 },
	{ "ENDPRINT",				0 },
	{ "PRINTSTRING",			0 },
	{ "PRINTNUMBER",			0 },
	{ "PRINTCHARACTER",			0 },
	{ "PLAYERCOUNT",			0 },
	{ "GAMETYPE",				0 },
	{ "GAMESKILL",				0 },
	{ "TIMER",				0 },
	{ "SECTORSOUND",			0 },
	{ "AMBIENTSOUND",			0 },
	{ "SOUNDSEQUENCE",			0 },
	{ "SETLINETEXTURE",			0 },
	{ "SETLINEBLOCKING",			0 },
	{ "SETLINESPECIAL",			0 },
	{ "THINGSOUND",				0 },
	{ "ENDPRINTBOLD",			0 },
	{ "ACTIVATORSOUND",			0 },
	{ "LOCALAMBIENTSOUND",			0 },
	{ "SETLINEMONSTERBLOCKING",		0 }
};

Behavior::Behavior (WadFile &wad, int lump)
{
	Lump = NULL;

	if ((LumpSize = wad.LumpLength (lump)))
		if ((Lump = new uint[LumpSize >> 2])) {
			if (!wad.ReadLump (lump, Lump)) {
				delete[] Lump;
				Lump = NULL;
			}
		}
}

Behavior::Behavior (byte *data, uint len)
{
	if ((Lump = new uint[len >> 2])) {
		memcpy (Lump, data, len);
		LumpSize = len;
	}
}

Behavior::~Behavior ()
{
	if (Lump) {
		delete[] Lump;
		Lump = NULL;
	}
}

void Behavior::DumpPointers (void)
{
	int i, c;
	uint *table;

	if (!Lump) {
		cout << "No behavior lump loaded" << endl;
		return;
	}

	cout << "Pointer table begins at offset " << Lump[1] << ':' << endl << endl;

	table = (uint *)((byte *)Lump + Lump[1]);

	i = *table++;
	cout << i << " scripts" << endl;
	cout << "---------------------------------------------" << endl << setfill ('0');

	for (; i; i--, table += 3) {
		cout << "0x" << hex << setw (8) << table[1] << dec;
		cout << "  script ";
		if ((table[0] >= 1000)) {
			cout << table[0] - 1000 << " OPEN" << endl;
		} else {
			cout << table[0] << " (";
			if (table[2]) {
				for (c = table[2]; c; c--) {
					cout << "int arg" << table[2] - c;
					if (c > 1)
						cout << ", ";
				}
			} else {
				cout << "void";
			}
			cout << ")" << endl;
		}
	}

	i = c = *table++;
	cout << endl << i << " strings" << endl;
	cout << "---------------------------------------------" << endl;
	for (; i; i--, table++) {
		cout << "0x" << hex << setw (8) << setfill ('0') << *table << dec;
		cout << dec << setw(3) << setfill (' ') << c - i;
		cout << "  \"" << ((char *)Lump + *table) << '\"' << endl;
	}
}

void Behavior::DumpCode (uint script)
{
	uint *ptr, *end;
	uint i;
	uint code;
	uint numscripts;
	
	struct scriptheader {
		uint script;
		uint offset;
		uint numparam;
	} *header;

	ptr = (uint *)((byte *)Lump + Lump[1]);
	numscripts = *ptr;
	header = (scriptheader *)(ptr + 1);

	for (i = 0; i < numscripts; i++)
		if (header[i].script % 1000 == script)
			break;

	if (i == numscripts) {
		cout << "No script " << script << endl;
		return;
	}

	if (i == numscripts - 1) {
		if (ptr[numscripts*3+1])
			end = (uint *)((byte *)Lump + ptr[numscripts*3+2]);
		else
			end = ptr;
	} else {
		end = (uint *)((byte *)Lump + header[i+1].offset);
	}

	ptr = (uint *)((byte *)Lump + header[i].offset);

	while (ptr < end) {
		cout << setw(6) << (byte *)ptr - (byte *)Lump << ": ";
		code = *ptr++;
		if (code >= NUM_CODES) {
			cout << "**UNKWOWN " << code << "**";
		} else {
			cout << Codes[code].name;
			for (i = 0; i < Codes[code].numparams; i++)
				cout << ' ' << *ptr++;
		}
		cout << endl;
	}
}

void usage (void)
{
	cout << "Usage: listacs <wadfile> <map> [script]" << endl
		 << "   or: listacs <binaryfile> [script]" << endl;
	exit(5);
}

int main (int argc, char **argv)
{
	WadFile wad;
	int lump;

	if (argc < 2 || argc > 4)
		usage();

	if (wad.Open (argv[1])) {
		if (argc < 3)
			usage();

		cout << wad.GetNumLumps () << " lumps in wad." << endl;
		if ((lump = wad.CheckNumForName (argv[2])) >= 0) {
			if (!strnicmp (wad.GetLumpName (lump + 11), "BEHAVIOR", 8)) {
				Behavior acs (wad, lump + 11);

				if (argc == 3)
					acs.DumpPointers ();
				else
					acs.DumpCode (atoi (argv[3]));
			}
		} else {
			cout << "Could not find " << argv[2] << " lump.\n";
		}
	} else {
		FILE *f = fopen (argv[1], "rb");

		if (f) {
			size_t len;
			byte *data;

			fseek (f, 0, SEEK_END);
			len = ftell (f);
			fseek (f, 0, SEEK_SET);

			if (len) {
				data = new byte[len];

				if (fread (data, len, 1, f) == 1) {
					Behavior acs (data, len);

					if (argc == 2)
						acs.DumpPointers ();
					else
						acs.DumpCode (atoi (argv[2]));
				} else
					cout << "Could not read " << argv[1] << endl;

				delete data;
			} else
				cout << argv[1] << " is empty" << endl;
		} else
			cout << "Could not open " << argv[1] << endl;
	}

	return 0;
}
