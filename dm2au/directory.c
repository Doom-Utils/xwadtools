#include <unistd.h>
#include <string.h>

#include "doomdefs.h"
#include "directory.h"

#include "strfunc.h"

/* Find the first byte of the "directory" and move 'fd' to it. */
int set_directory(int fd)
{
	four_byte	dir_start;
	char		magic[4];

	/* Check for IWAD or PWAD magic */
	if (lseek(fd, 1, SEEK_SET) < 0)
		return(-1);
	if (read(fd, magic, 3) != 3)
		return(-1);
	magic[3] = '\0';
	if (strcmp(magic, "WAD") != 0)
		return(-1);
	
	/* Get the start of the directory from the WAD header */
	if (lseek(fd, 8, SEEK_SET) < 0) 
		return(-1);
	read(fd, (char *)&dir_start, 4);

	/* Go to it! */
	if (lseek(fd, dir_start, SEEK_SET) < 0)
		return(-1);
	return(0);
}

int get_entry(int fd, char *name, dir_entry *entry)
{
	while (read(fd, (char *)entry, ENTRY_SIZE) == ENTRY_SIZE) {
		/* Null terminate the name */
		entry->name[8] = '\0';
		/* Is it the entry we want? */
		if (strcasecmp(entry->name, name) == 0)
			return(0);
		/* Have we reached the end of the entries? */
		if (strcasecmp(WADDIR_END, entry->name) == 0)
			break;
	}
	/* We didn't find the proper entry */
	return(-1);
}
