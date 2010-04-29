#ifndef __MARKOV_H__
#define __MARKOV_H__

struct markov;

void markov_free(struct markov *m);
void markov_freeze(struct markov *m);

struct markov *markov_create(int token_length);
unsigned char markov_get_init(struct markov *m);
void markov_add_init_state(struct markov *m, unsigned char cur);
unsigned char markov_get_next(struct markov *m, unsigned char cur);
void markov_add_cycle(struct markov *m, unsigned char cur, unsigned char next);

#endif /* __MARKOV_H__ */
