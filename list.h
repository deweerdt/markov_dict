#ifndef _LIST_H_
#define _LIST_H_

#define LINE_POISON ((void *)((0x6b6b << 16 ) | __LINE__))
#if defined(TEST)
#  define DEBUG(x) x
#else
#  define DEBUG(x)
#endif

struct list_head {
	struct list_head *next, *prev;
};

static inline struct list_head *list_tail(struct list_head *h)
{
	return h->prev;
}

#define list_for_each_safe(__l, __n, head)  \
	for (__l = (head)->next, __n = __l->next; \
	     __l != (head); \
	     __l = __n, __n =__l->next)

#define list_for_each_reverse(__l, head)  \
	for (__l = (head)->prev; __l != (head); __l = __l->prev)

#define list_for_each(__l, head)  \
	for (__l = (head)->next; __l != (head); __l = __l->next)

#define DEFINE_HEAD(head) struct list_head head = { &head, &head }
static inline void INIT_LIST(struct list_head *h)
{
	h->next = h;
	h->prev = h;
}

static inline void __list_add(struct list_head *new_elem, struct list_head *prev, struct list_head *next)
{
	new_elem->next = next;
	new_elem->prev = prev;
	prev->next = new_elem;
	next->prev = new_elem;
}

static inline void list_add_tail(struct list_head *new_elem, struct list_head *head)
{
	__list_add(new_elem, head->prev, head);
}

static inline void list_add(struct list_head *new_elem, struct list_head *h)
{
	__list_add(new_elem, h->prev, h->next);
}

static inline void __list_del(struct list_head *prev, struct list_head *next)
{
	prev->next = next;
	next->prev = prev;
}

static inline void list_del(struct list_head *elem)
{
	__list_del(elem->prev, elem->next);
	DEBUG(elem->next = LINE_POISON);
	DEBUG(elem->prev = LINE_POISON);
}

static inline int list_empty(const struct list_head *h)
{
	return h->next == h;
}

#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE,MEMBER) __compiler_offsetof(TYPE,MEMBER)
#else
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif


#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})



#endif /* _LIST_H_ */
