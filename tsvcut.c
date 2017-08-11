#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define HUGEBUF (1024*1024)

typedef struct _col {
	char *name; // Name of column (from command line)
	char *start; // Start of value of column (from input)
	int length; // length of value of column (from input)
} col;


char *nextcol(char *p, char **ptr, int *len)
{
	int i = 0;

	while(p[i] && p[i] != '\t' && p[i] != '\n') i++;
	*ptr = p;
	*len = i;
	return p + i + (p[i] != 0);
}

void tsvparse(char *line, col *cols, int nCols)
{
	int c;
	char *p;

	for(c = 0; c < nCols; c++) {
		cols[c].start = NULL;
		cols[c].length = 0;
	}

	p = line;

	while(*p) {
		char *key, *val;
		int keylen, vallen;

		p = nextcol(p, &key, &keylen);
		p = nextcol(p, &val, &vallen);
		if(key && keylen) {
			for(c = 0; c < nCols; c++) {
				if(strncmp(cols[c].name, key, keylen) == 0 && cols[c].name[keylen] == '\0')
					break;
			}
			if(c < nCols) {
				cols[c].start = val;
				cols[c].length = vallen;
			}
		}
	}
}

void usage (char *prog)
{
	fprintf(stderr, "Usage: %s [-h|-t] [-n NULL] [--] columns\n", prog);
}

int main(int ac, char **av)
{
	col *cols;
	int nCols = 0;
	int options = 1;
	char *argv0 = *av;
	char *buf;
	int kvOutput = 0;
	int headers = 0;
	char *null = "";

	if (!(cols = malloc(ac * (sizeof *cols)))) {
		perror("malloc(cols)");
		return -1;
	}
	if (!(buf = malloc(HUGEBUF))) {
		perror("malloc(buf)");
		return -1;
	}

	while(*++av) {
		if(options && **av == '-') {
			if(strcmp(*av, "-h") == 0) {
				headers = 1;
			} else if(strcmp(*av, "-t") == 0) {
				kvOutput = 1;
			} else if(strcmp(*av, "-n") == 0) {
				if(!av[1]) {
					fprintf(stderr, "%s: No argument to -n\n", argv0);
					usage(argv0);
					return -1;
				}
				null = *++av;
			} else if(strcmp(*av, "--") == 0) {
				options = 0;
			} else {
				fprintf(stderr, "%s: Unknown option %s\n", argv0, *av);
				usage(argv0);
				return -1;
			}
		} else {
			cols[nCols].name = *av;
			nCols++;
		}
	}

	if(headers && kvOutput) {
		fprintf(stderr, "%s: Can't use both -h and -t\n", argv0);
		usage(argv0);
		return -1;
	}
	if(headers) {
		int c;
		for(c = 0; c < nCols; c++) {
			if(c) putchar('\t');
			printf("%s", cols[c].name);
		}
		putchar('\n');
	}
	while(fgets(buf, HUGEBUF, stdin)) {
		int c;
		tsvparse(buf, cols, nCols);
		for(c = 0; c < nCols; c++) {
			if(c) putchar('\t');
			if(kvOutput) printf("%s\t", cols[c].name);
			if(cols[c].length)
				fwrite(cols[c].start, cols[c].length, 1, stdout);
			else
				printf("%s", null);
		}
		putchar('\n');
	}
}
