typedef unsigned int uint;
typedef unsigned char byte;

class WadFile {
	public:
		WadFile ();
		WadFile (char *filename);
		~WadFile ();

		bool Ok (void);

		bool Open (char *filename);
		void Close (void);
		bool RefreshInfoTable (void);

		int CheckNumForName (char *name);
		int LumpLength (int lump);
		bool ReadLump (int lump, void *dest);
		char *GetLumpName (int lump);
		int GetNumLumps (void);

	private:
		struct wadinfo {
			// Should be "IWAD" or "PWAD".
			char	identification[4];
			uint	numlumps;
			uint	infotableofs;
		};

		struct filelump {
			uint	filepos;
			uint	size;
			char	name[8];
		};

		FILE		*File;
		uint		NumLumps;
		filelump	*InfoTable;
		uint		InfoTableOfs;
};

#define NUM_CODES	105

class Behavior {
	public:
		Behavior (WadFile &wad, int lump);
		Behavior (byte *data, uint len);
		~Behavior ();

		void DumpPointers (void);
		void DumpCode (uint script);

	private:
		static struct code {
			char *name;
			uint numparams;
		} Codes[NUM_CODES];

		uint	LumpSize;
		uint	*Lump;
};
