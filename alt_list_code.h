
typedef struct mlist_s {
	void *next;
	void *prev;
	void *data;
} mlist_t, *mlist;

typedef int (*mCompareFunc) ( const void *, const void *);

mlist m_list_append ( mlist, void *);
mlist m_list_remove ( mlist, void *);
void m_list_free ( mlist);
mlist m_list_find_custom ( mlist, void *, mCompareFunc);
mlist m_list_find ( mlist list, void *data );
