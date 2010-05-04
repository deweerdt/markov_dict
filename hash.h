#ifndef __HASH_H__
#define __HASH_H__

#include "list.h"

struct hash {
	struct list_head *hash;
	int key_size;
	int nb_elems;
};

int hash_size(struct hash *hash);
struct hash *hash_init(int size);
void hash_free(struct hash *hash);
void *hash_get(struct hash *hash, const char *key);
int hash_put(struct hash *hash, char *key, void *value);

#endif /* __HASH_H__ */
