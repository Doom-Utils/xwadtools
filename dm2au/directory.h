typedef struct dir_entry {
	four_byte res_start;		/* Pointer to start of resource */
	four_byte res_len;		/* Length of resource */
	char name[9];			/* Really only 8 bytes! */
} dir_entry;

#define ENTRY_SIZE	16	
#define WADDIR_END	"F_END"		/* The last entry in the directory */

extern int set_directory(int);
extern int get_entry(int, char *, dir_entry *);
