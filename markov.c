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

#define ASCII_CHARS 255

struct markov {
	int graph[ASCII_CHARS][ASCII_CHARS];
	int graph_total[ASCII_CHARS];
	int init[ASCII_CHARS];
	int init_total;
	int token_length;
};

static unsigned char markov_get_ponderated(struct markov *m, unsigned char cur, int i)
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
unsigned char markov_get_next(struct markov *m, unsigned char cur)
{
	int i;

	/* No letters following this one */
	if (m->graph_total[cur] == 0)
		return 0;

	i = rand() % m->graph_total[cur];

	return markov_get_ponderated(m, cur, i);
}

unsigned char markov_get_init(struct markov *m)
{
	int i;

	do {
		i = rand() % ARRAY_SIZE(m->init);
	} while(m->init[i] == 0);

	return i;
}

void markov_add_cycle(struct markov *m, unsigned char cur, unsigned char next)
{
	m->graph[cur][next]++;
}

void markov_add_init_state(struct markov *m, unsigned char cur)
{
	m->init[cur]++;
}

struct markov *markov_create(int token_length)
{
	struct markov *m;

	m = calloc(1, sizeof(*m));
	if (!m)
		return NULL;

	m->token_length = token_length;

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
