#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "list.h"
#include "hash.h"

#define KEY_SIZE 4
#define HASH_SIZE (1 << KEY_SIZE)

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

int hash_put(hash_t *hash, char *key, void *value)
{
	struct key_val *kv;
	unsigned long h = compute_hash(key) % HASH_SIZE;

	kv = malloc(sizeof(*kv));
	if (!kv)
		return 0;

	kv->key = key;
	kv->value = value;

	list_add_tail(&kv->list, &hash[h]);
	return 1;
}

void *hash_get(hash_t *hash, const char *key)
{
	struct list_head *l;
	unsigned long h = compute_hash(key) % HASH_SIZE;

	if (list_empty(&hash[h]))
		return NULL;
	list_for_each(l, &hash[h]) {
		struct key_val *kv = container_of(l, struct key_val, list);
		if (!strcmp(key, kv->key)) {
			return kv->value;
		}
	}
	return NULL;
}
void hash_free(hash_t *hash)
{
	int i;
	struct list_head *l, *n;
	for (i = 0; i < HASH_SIZE; i++) {
		list_for_each_safe(l, n, &hash[i]) {
			struct key_val *kv = container_of(l, struct key_val, list);
			list_del(l);
			free(kv->value);
			free(kv->key);
			free(kv);
		}
	}
}

hash_t *hash_init(void)
{
	int i;
	hash_t *hash;

	hash = calloc(HASH_SIZE, sizeof(hash[0]));
	if (!hash)
		return NULL;

	for (i = 0; i < HASH_SIZE; i++) {
		INIT_LIST(&hash[i]);
	}
	return hash;
}


