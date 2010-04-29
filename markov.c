#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

struct vector {
	void **v;
	int nb;
	int alloc;
};

static const int default_size = 1024;

struct vector* vector_create(int init_size)
{
	struct vector *v;

	v = calloc(1, sizeof(*v));
	if (!v)
		return NULL;

	if (!init_size)
		init_size = default_size;

	v->v = calloc(init_size, sizeof(char));
	if (!v->v) {
		free(v);
		return NULL;
	}

	v->nb = 0;
	v->alloc = init_size;

	return v;
}

void vector_free(struct vector *v)
{
	free(v->v);
	free(v);
}

void *vector_get(struct vector *v, int index)
{
	if (index < 0 || index >= v->nb)
		return NULL;

	return v->v[index];
}

void *vector_del(struct vector *v, int index)
{
	void *d;
	if (index < 0 || index >= v->nb)
		return NULL;

	d = v->v[index];
	v->v[index] = NULL;

	return d;
}

void *vector_add(struct vector *v, int index, void *data)
{
	if (index < 0)
		return NULL;

	if (index > v->alloc) {
		void *tmp;
		tmp = calloc(index + default_size, sizeof(void *));
		if (!tmp)
			return NULL;

		memcpy(tmp, v->v, v->alloc * sizeof(void *));
		free(v->v);
		v->v = tmp;
	}

	v->v[index] = data;
}

#define ASCII_CHARS 255
struct markov {
	int graph[ASCII_CHARS][ASCII_CHARS];
	int graph_total[ASCII_CHARS];
	int init[ASCII_CHARS];
	int init_total;
};

unsigned char get_ponderated(struct markov *m, unsigned char cur, int i)
{
	int n;
	int total;

	total = 0;
	for (n = 0; n < ARRAY_SIZE(m->graph[0]); n++) {
		total += m->graph[cur][n];
		if (i < total)
			return n;
	}
	return 0;
}
unsigned char get_next(struct markov *m, unsigned char cur)
{
	int i;

	/* No letters following this one */
	if (m->graph_total[cur] == 0)
		return 0;

	i = rand() % m->graph_total[cur];

	return get_ponderated(m, cur, i);
}

unsigned char get_init(struct markov *m)
{
	int i;

	do {
		i = rand() % ARRAY_SIZE(m->init);
	} while(m->init[i] == 0);

	return i;
}

void add_cycle(struct markov *m, unsigned char cur, unsigned char next)
{
	m->graph[cur][next]++;
}

void add_init_state(struct markov *m, unsigned char cur)
{
	m->init[cur]++;
}

struct markov *markov_create(void)
{
	struct markov *m;

	m = calloc(1, sizeof(*m));
	if (!m)
		return NULL;

	return m;
}

void markov_freeze(struct markov *m)
{
	int i, j;
	int total;

	total = 0;
	for (i = 0; i < ARRAY_SIZE(m->init); i++) {
		total += m->init[i];
	}
	m->init_total = total;

	for (i = 0; i < ARRAY_SIZE(m->graph); i++) {
		total = 0;
		for (j = 0; j < ARRAY_SIZE(m->graph[i]); j++) {
			total += m->graph[i][j];
		}
		m->graph_total[i] = total;
	}
}

void markov_free(struct markov *m)
{
	free(m);
}

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

	m = markov_create();

	while ((read = getline(&line, &len, f)) != -1) {
		if (line[0] == '\0' || line[0] == '\n')
			continue;

		add_init_state(m, line[0]);
		for (i = 1; i + 1 < read
			    && line[i + 1] != '\n'; i++) {
			add_cycle(m, line[i], line[i+1]);
		}
		add_cycle(m, line[i], '\0');
	}

	markov_freeze(m);

	if (line)
		free(line);

	i = 0;
	unsucess_full_tries = 0;
	while (i < gen_names) {
		unsigned char c;
		char str[128];

		c = get_init(m);
		str[0] = c;
		j = 1;

		do {
			str[j] = get_next(m, str[j-1]);
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
