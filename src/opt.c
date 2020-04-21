#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opt.h"

#ifdef DEV_BUILD
#define DEF_FS		0
#define DEF_VSYNC	1
#define DEF_SSCR	"game"
#else
#define DEF_FS		1
#define DEF_VSYNC	1
#define DEF_SSCR	"game"
#endif

struct options opt = {
	1280, 800,
	DEF_FS,
	DEF_VSYNC,
	1,				/* multisample */
	1,				/* sRGB */
	DEF_SSCR
};

static void print_usage(const char *argv0);

int parse_args(int argc, char **argv)
{
	int i;

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			if(strcmp(argv[i], "-s") == 0) {
				if(!argv[++i] || sscanf(argv[i], "%dx%d", &opt.width, &opt.height) != 2) {
					fprintf(stderr, "-s must be followed by a resolution of the form WxH\n");
					return -1;
				}
			} else if(strcmp(argv[i], "-fs") == 0) {
				opt.fullscreen = 1;
			} else if(strcmp(argv[i], "-win") == 0) {
				opt.fullscreen = 0;
			} else if(strcmp(argv[i], "-vsync") == 0) {
				opt.vsync = 1;
			} else if(strcmp(argv[i], "-novsync") == 0) {
				opt.vsync = 0;
			} else if(strcmp(argv[i], "-aa") == 0) {
				opt.multisample = 1;
			} else if(strcmp(argv[i], "-noaa") == 0) {
				opt.multisample = 0;
			} else if(strcmp(argv[i], "-nosrgb") == 0) {
				opt.srgb = 0;
			} else if(strcmp(argv[i], "-scr") == 0) {
				if(!argv[++i]) {
					fprintf(stderr, "-scr must be followed by a screen name\n");
					return -1;
				}
				opt.startscr = argv[i];
			} else if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0) {
				print_usage(argv[0]);
				exit(0);
			} else {
				fprintf(stderr, "invalid option: %s\n", argv[i]);
				return -1;
			}
		} else {
			fprintf(stderr, "unexpected argument: %s\n", argv[i]);
			return -1;
		}
	}

	return 0;
}

static void print_usage(const char *argv0)
{
	printf("Usage: %s [options]\n", argv0);
	printf("Options:\n");
	printf(" -s <WxH>         set windowed mode window size\n");
	printf(" -fs/-win         start fullscreen/windowed\n");
	printf(" -vsync/-novsync  enable/disable vsync\n");
	printf(" -aa/-noaa        enable/disbale anti-aliasing\n");
	printf(" -nosrgb          disable sRGB color space (only in case of emergency)\n");
	printf(" -scr <name>      starting screen name\n");
	printf(" -h,-help         print usage and exit\n");
}
