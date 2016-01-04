#define _GNU_SOURCE
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "markov.h"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

int main(int argc, char **argv)
{
	FILE *f;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	struct markov *m;
	int i, j;
        int opt, gen_names, min_size, max_size;
	int unsucess_full_tries;

	min_size = 0;
	gen_names = 10;
	max_size = INT_MAX;
	while ((opt = getopt(argc, argv, "M:m:n:")) != -1) {
		switch (opt) {
			case 'n':
				gen_names = atoi(optarg);
				break;
			case 'm':
				min_size = atoi(optarg);
				break;
			case 'M':
				max_size = atoi(optarg);
				break;
			default:
				fprintf(stderr, "Usage: %s"
						"[-m min name length]"
						"[-M max name length]"
						"[-n number of gen names]"
						"[-N token length]"
						"<dict file>\n",
						argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	srand(time(NULL));

	f = fopen(argv[optind], "r");

	if (!f) {
		fprintf(stderr, "Cannot open file %s: %s", argv[optind], strerror(errno));
		exit(EXIT_FAILURE);
	}

	m = markov_create(1);

	while ((read = getline(&line, &len, f)) != -1) {
		if (line[0] == '\0' || line[0] == '\n')
			continue;

		markov_add_init_state(m, line[0]);
		for (i = 1; i + 1 < read
			    && line[i + 1] != '\n'; i++) {
			markov_add_cycle(m, line[i], line[i+1]);
		}
		markov_add_cycle(m, line[i], '\0');
	}

	markov_freeze(m);

	if (line)
		free(line);

	i = 0;
	unsucess_full_tries = 0;
	while (i < gen_names) {
		unsigned char c;
		char str[128];

		c = markov_get_init(m);
		str[0] = c;
		j = 1;

		do {
			str[j] = markov_get_next(m, str[j-1]);
		} while (str[j++] != '\0' && j < 128);

		str[j] = '\0';

		if (strlen(str) < min_size || strlen(str) > max_size) {
			unsucess_full_tries++;
			if (unsucess_full_tries > 10000) {
				fprintf(stderr,
					"Cannot generate names"
					"within given constraints\n");
				exit(EXIT_FAILURE);
			}
			continue;
		}

		printf("[%s]\n", str);
		i++;
	}

	markov_free(m);

	fclose(f);

	exit(EXIT_SUCCESS);
	return 0;
}
