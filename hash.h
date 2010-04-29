#ifndef __HASH_H__
#define __HASH_H__

#include "list.h"
typedef struct list_head hash_t;

hash_t *hash_init(void);
void hash_free(hash_t *hash);
void *hash_get(hash_t *hash, const char *key);
int hash_put(hash_t *hash, char *key, void *value);

#endif /* __HASH_H__ */
