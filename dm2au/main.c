#include	<sys/types.h>
#include	<fcntl.h>
#include	<stdio.h>

#define DOOM_WAD	"/usr/local/games/xdoom/doom2.wad"

extern int convert_rawsnd(int, char *);

int main(int argc, char *argv[])
{
	int	fd;
	char	*wad = DOOM_WAD;

	if (argc == 4 && strcmp(argv[1], "-f") == 0) {
		wad = argv[2];
		argc -= 2;
		argv += 2;
	}
	if ((fd = open(wad, O_RDONLY, 0)) < 0) {
		perror(wad);
		exit(2);
	}
	if (argc != 2) {
		fprintf(stderr, "Usage: %s [-f pwad] sndname\n", argv[0]);
		exit(1);
	}
	if (convert_rawsnd(fd, argv[1]) < 0)
		exit(2);
	exit(0);
}
