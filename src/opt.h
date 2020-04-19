#ifndef OPT_H_
#define OPT_H_

struct options {
	int width, height;
	int fullscreen;
	int vsync;
	int multisample;
	char *startscr;
};

extern struct options opt;

#ifdef __cplusplus
extern "C" {
#endif

int parse_args(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif	/* OPT_H_ */
