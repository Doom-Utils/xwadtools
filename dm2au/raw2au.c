/* This module extracts raw sound information from a wadfile */

#include	<stdio.h>
#include	<unistd.h>
#include	"doomdefs.h"
#include	"directory.h"

#define ULAW_CONVERT		/* Convert the sound into sun .au format */

#define SND_MAGIC	3
struct snd_header {
	two_byte magic;			/* The magic number 3 */
	two_byte sample_rate;		/* The sample rate (11025) */
	two_byte num_samples;		/* The number of samples */
	two_byte zero;			/* Null integer */
};

extern int sample_cvt(int);

int convert_rawsnd(int fd, char *name)
{
	struct dir_entry	entry;
	struct snd_header	header;
	char			outfile[BUFSIZ];
	FILE			*output;
	int			i, samplei;
	unsigned char		sample, ulaw;
	float			sum = 0, increment;

	/* Set the fd to the start of the directory entries */
	if (set_directory(fd) < 0) {
		fprintf(stderr, "Can't find start of directories.\n");
		return(-1);
	}

	/* Now cycle through the entries, looking for the sound  */
	if (get_entry(fd, name, &entry) < 0) {
		fprintf(stderr, "Couldn't find directory entry for %s\n", name);
		return(-1);
	}

	/* Go to the entry and retrieve the sound */
	if (lseek(fd, entry.res_start, SEEK_SET) < 0) {
		perror("lseek()");
		return(-1);
	}
	if (read(fd, (char *)&header, sizeof(header)) != sizeof(header)) {
		perror("header read error");
		return(-1);
	}
	if (header.magic != SND_MAGIC) {
		fprintf(stderr, "%s is not a sound resource (%d).\n",name,header.magic);
		return(-1);
	}
	/* Print out the header for now */
	printf("Sound: sample rate = %d, holds %d samples.\n",
				header.sample_rate, header.num_samples);

	/* increment is the number of bytes to read each time */
	increment = ((float)header.sample_rate) / 8000;

	/* Make sure we can write the output file */
#ifdef ULAW_CONVERT
	sprintf(outfile, "%s.au", name);
#else
	sprintf(outfile, "%s.snd", name);
#endif
	if ((output = fopen(outfile, "w")) == NULL) {
		fprintf(stderr, "Can't open %s for writing.\n", outfile);
		return(-1);
	}
	/* Write the output file */
	for (i = 1, read(fd, &sample, 1); i < header.num_samples;) {
#ifdef ULAW_CONVERT
		/* convert the excess 128 to two's complement */
		samplei = 0x80 - sample;
		/* increase the volume, convert to uLAW */
		ulaw = (unsigned char)sample_cvt(samplei * 16);
		/* Output the converted sample */
		fputc((char)ulaw, output);

		/* skip input to compensate for sampling frequency diff */
		sum += increment;
		while(sum > 0) {
			if (read(fd, &sample, 1) != 1) {
				perror("read()");
				return(-1);
			}
			++i; --sum;
		}
#else /* Raw snd format */
		fputc(sample, output);
		if (read(fd, &sample, 1) != 1) {
			perror("read()");
			return(-1);
		}
#endif
	}
	fclose(output);

	/* Done! */
	return(0);
}
