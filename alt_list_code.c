#include "alt_list_code.h"
#include "display.h"

mlist m_list_append ( mlist list, void *data)
{
	mlist new = 0;
	mlist cur = 0;
	void *prev = 0;

	cur = list;
	
	if (!cur)
	{
		cur = (mlist) malloc (sizeof(mlist_t));

		cur->prev = 0;
		cur->next = 0;
		cur->data = data;

		new = cur;

		list = new;

	}
	else
	{
		while (cur)
		{
			prev = cur;
			cur = cur->next;
		}

		new = (mlist) malloc ( sizeof( mlist_t ) );

		new->data = data;

		new->prev = prev;

		new->next = 0;

		cur = prev;

		cur->next = new;

		//new = list;
	}

#ifdef DEBUG
	err_printf("m_list_append: (NEW) ORIG:%d NEW:%d DATA: %d\n",list,new,data);
#endif
	return list;
}

mlist m_list_remove ( mlist list, void *data)
{
	mlist cur;
	mlist prev;
	mlist next;
	mlist first;
	
	cur = list;
	prev = 0;
	first = cur;
	
	// FIXME: use a function to do this
	
	while ( cur && cur->data != data )
	{
		cur = cur->next;
		prev = cur;
	}

	if ( !cur ) // not found
		return 0;

	next = cur->next;

	if ( cur == first)
		if ( next )
			first = next;
		else
			first = 0;


	if (prev)
		prev->next = next;

	if (next)
		next->prev = prev;

//	if (cur->data)
//		free(cur->data);

	// FIXME: i'm not 100% sure we should be doing this
	free(cur);

	return first;
	
}

void m_list_free ( mlist list)
{
	mlist cur;

	cur = list;
	
	if (!cur)
		return;
		
	while (cur)
	{
		if (cur->data)
			free(cur->data);
		free(cur);

		cur=cur->next;
		cur = 0;
	}
}

mlist m_list_find_custom ( mlist list, void *data, mCompareFunc compFunc)
{
	mlist cur;
	cur = list;
	while ( cur && compFunc(data,cur->data) == 0 )
	{
		cur = cur->next;
	}

	return cur;
}

mlist m_list_find ( mlist list, void *data )
{
	mlist cur;

	cur = list;

	while ( cur && data != cur->data )
		cur = cur->next;

	return cur;
}
