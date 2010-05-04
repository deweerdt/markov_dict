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
	int _graph[ASCII_CHARS][ASCII_CHARS];
	int _graph_total[ASCII_CHARS];
	int _init[ASCII_CHARS];
	int _init_total;
	int token_length;
};

static int hash_graph_elems(struct markov *m)
{
	return ARRAY_SIZE(m->_graph[0]);
}

static void hash_graph_links_set(struct markov *m, unsigned char cur, unsigned char next, int value)
{
	m->_graph[cur][next] = value;
}

static int hash_graph_links_get(struct markov *m, unsigned char cur, unsigned char next)
{
	return m->_graph[cur][next];
}

static void hash_graph_total_set(struct markov *m, unsigned char c, int i)
{
	m->_graph_total[c] = i;
}

static int hash_graph_total_get(struct markov *m, unsigned char c)
{
	return m->_graph_total[c];
}

static unsigned char markov_get_ponderated(struct markov *m, unsigned char cur, int i)
{
	int n;
	int total;

	total = 0;
	for (n = 0; n < hash_graph_elems(m); n++) {
		total += hash_graph_links_get(m, cur, n);
		if (i < total)
			return n;
	}
	return 0;
}

static int hash_init_size(struct markov *m)
{
	return ARRAY_SIZE(m->_init);
}

static void __attribute__((unused)) hash_init_occurs_set(struct markov *m, unsigned char c, int i)
{
	m->_init[c] = i;
}

static int hash_init_occurs_get(struct markov *m, unsigned char c)
{
	return m->_init[c];
}

unsigned char markov_get_next(struct markov *m, unsigned char cur)
{
	int i;

	/* No letters following this one */
	if (hash_graph_total_get(m, cur) == 0)
		return 0;

	i = rand() % hash_graph_total_get(m, cur);

	return markov_get_ponderated(m, cur, i);
}

unsigned char markov_get_init(struct markov *m)
{
	int i;

	do {
		i = rand() % hash_init_size(m);
	} while(hash_init_occurs_get(m, i) == 0);

	return i;
}

void markov_add_cycle(struct markov *m, unsigned char cur, unsigned char next)
{
	int i;

	i = hash_graph_links_get(m, cur, next);
	hash_graph_links_set(m, cur, next, i + 1);
}

void markov_add_init_state(struct markov *m, unsigned char cur)
{
	m->_init[cur]++;
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
	for (i = 0; i < hash_init_size(m); i++) {
		total += hash_init_occurs_get(m, i);
	}
	m->_init_total = total;

	/* Sum up all the graph items */
	for (i = 0; i < ARRAY_SIZE(m->_graph); i++) {
		total = 0;
		for (j = 0; j < hash_graph_elems(m); j++) {
			total += hash_graph_links_get(m, i, j);
		}
		hash_graph_total_set(m, i, total);
	}
}

void markov_free(struct markov *m)
{
	free(m);
}
