#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "list.h"
#include "hash.h"

/*
#define KEY_SIZE 4
#define HASH_SIZE (1 << KEY_SIZE)
*/

struct key_val {
	char *key;
	void *value;
	struct list_head list;
};

unsigned long compute_hash(const char *s)
{
	unsigned long h = *s;
	if (!h)
		return 0;

	s++;
	while (*s) {
		h = (h << 5) - h + *s;
		s++;
	}
	return h;
}

int hash_put(struct hash *hash, char *key, void *value)
{
	struct key_val *kv;
	unsigned long h = compute_hash(key) % hash->key_size;

	kv = malloc(sizeof(*kv));
	if (!kv)
		return 0;

	kv->key = key;
	kv->value = value;

	list_add_tail(&kv->list, &hash->hash[h]);

	hash->nb_elems++;

	return 1;
}

void *hash_get(struct hash *hash, const char *key)
{
	struct list_head *l;
	unsigned long h = compute_hash(key) % hash->key_size;

	if (list_empty(&hash->hash[h]))
		return NULL;
	list_for_each(l, &hash->hash[h]) {
		struct key_val *kv = container_of(l, struct key_val, list);
		if (!strcmp(key, kv->key)) {
			return kv->value;
		}
	}
	return NULL;
}
void hash_free(struct hash *hash)
{
	int i;
	struct list_head *l, *n;
	for (i = 0; i < hash->key_size; i++) {
		list_for_each_safe(l, n, &hash->hash[i]) {
			struct key_val *kv = container_of(l, struct key_val, list);
			list_del(l);
			free(kv->value);
			free(kv->key);
			free(kv);
		}
	}
}

struct hash *hash_init(int key_bits)
{
	int i;
	struct hash *hash;
	int key_size = 1 << key_bits;

	hash = calloc(1, sizeof(*hash));
	if (!hash)
		return NULL;

	hash->hash = calloc(key_size, sizeof(hash->hash[0]));
	if (!hash->hash) {
		free(hash);
		return NULL;
	}

	for (i = 0; i < key_size; i++) {
		INIT_LIST(&hash->hash[i]);
	}

	hash->key_size = key_size;

	return hash;
}

int hash_size(struct hash *hash)
{
	return hash->nb_elems;
}

