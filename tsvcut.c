#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// You'll have to make this smaller if you ever compile this on a PDP-11
// It would totally work though
// If your input line is more than a megabyte, holy cow what are you doing?
#define HUGEBUF (1024*1024)

typedef struct _col {
	char *name; // Name of column (from command line)
	char *start; // Start of value of column (from input)
	int length; // length of value of column (from input)
	int null; // missing
} col;

// step past the next column, retrun start of column and length of column.
char *nextcol(char *p, char **ptr, int *len)
{
	int i = 0;

	// step step step
	while(p[i] && p[i] != '\t' && p[i] != '\n') i++;

	// fill in the blanks
	*ptr = p;
	*len = i;
	return p + i + (p[i] != 0);
}

// Step through the line filling in the output columns structure with matched column names.
void tsvparse(char *line, col *cols, int nCols)
{
	int c;
	char *p;

	// Clear out the output
	for(c = 0; c < nCols; c++) {
		cols[c].start = NULL;
		cols[c].length = 0;
		cols[c].null = 1;
	}

	p = line;

	// While I haven't hit the null byte at the end of the universe
	while(*p) {
		char *key, *val;
		int keylen, vallen;

		// read two columns
		p = nextcol(p, &key, &keylen);
		p = nextcol(p, &val, &vallen);

		// If we read the first column and the neme isn't empty
		if(key && keylen) {
			// Look for the column name (hey, we could use a btree or something)
			for(c = 0; c < nCols; c++) {
				if(strncmp(cols[c].name, key, keylen) == 0 && cols[c].name[keylen] == '\0')
					break;
			}
			// If we found it, fill in the data. Even if the length is zero, that's not a null, it's an empty string
			if(c < nCols) {
				cols[c].start = val;
				cols[c].length = vallen;
				cols[c].null = 0;
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

	// This is more columns than we actually need, always, but they don't eat much.
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
		int first = 1;
		// all the heavy lifting is in tsvparse
		tsvparse(buf, cols, nCols);

		// output what we got
		for(c = 0; c < nCols; c++) {
			// in key-value output, don't output null columns at all
			if(kvOutput && cols[c].null)
				continue;

			// Suppress the first tab
			if(first) {
				first = 0;
			} else {
				putchar('\t');
			}

			// in key-value output, do the name
			if(kvOutput) printf("%s\t", cols[c].name);

			// distinguish between nulls and empty strings, unless you didn't specify -n, and null == ""
			if(cols[c].null)
				printf("%s", null);
			else if(cols[c].length)
				fwrite(cols[c].start, cols[c].length, 1, stdout);
		}
		putchar('\n');
	}
}
