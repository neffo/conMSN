#include <string.h>
#include "libmsn.h"
#include "display.h"
#include "input.h"
#include "msn_shiz.h"

extern msn_shiz_t MSNshiz;

void MSNRegisterCallbacks(void)
{
	MSN_RegisterErrorOutput((ERR_CALLBACK *)&MSNError);
	MSN_RegisterCallback(MSN_MSG, MSNInstantMessage);
	MSN_RegisterCallback(MSN_ILN, MSNStatusChange);
	MSN_RegisterCallback(MSN_NLN, MSNStatusChange);
	MSN_RegisterCallback(MSN_FLN, MSNStatusChange);
	MSN_RegisterCallback(MSN_AUTH, MSNAuthRequest);
	MSN_RegisterCallback(MSN_OUT, MSNLogout);
	MSN_RegisterCallback(MSN_MAIL, MSNMail);
}

void MSNInstantMessage(void *data)
{
	MSN_InstantMessage_PTR im;
	char *newhandle;
	msn_contact_t *contact;
	int en;

	newhandle = NULL;

	im = (MSN_InstantMessage_PTR)data;

	contact = GetContactByHandle(im->sender);

	// FIXME: add event to eventlist instead of just outputting it
	// although this might actually be a better (well less annoying) way
	// (i'm sticking with this method for now)
	//
	if (contact == NULL)
	{
		err_printf("contact == NULL\n");
		contact = new_contact(im->sender,im->sender);
	}
		

	// FIXME: add event to log files (DONE)
	if (cvar_true("msn_log"))
		log_event ( contact->alias, im->msg, im->day, im->month, 
			im->hour, im->minute );
	// print message to screen
	log_printf("m[%02d:%02d] %s> %s", im->hour,im->minute,contact->alias,im->msg);

	// call external program if user wants to
	if (cvar_true("msn_call_ext"))
		ext_msg(contact->alias, im->msg);
}

void MSNMail(void *data)
{
	MSN_MailNotification *newmail;

	newmail = (MSN_MailNotification *) data;

	if (newmail->from != NULL)
	{
		log_printf("lFrom: %s <%s>",newmail->from, newmail->fromaddr);
		log_printf("lSubject: %s",newmail->subject);

		if (cvar_true("msn_call_ext"))
			ext_mail(newmail->from,newmail->subject);
	}
	else
	{
		log_printf("lYou have %d new message%s.",newmail->unread,(newmail->unread > 1) ? "s" : "");
	}

}

void MSNStatusChange(void *data)
{
	MSN_StatusChange_PTR sc;
	msn_contact_t * contact;

	sc = (MSN_StatusChange_PTR)data;
	
	contact = GetContactByHandle(sc->handle);

	if (contact == NULL)
		contact = new_contact(sc->handle,sc->handle);

	contact->status = sc->newStatus;

	if (cvar_true("msn_call_ext"))
		ext_onl(contact->alias, msn_long_status[contact->status]);

	log_printf("a%s is %s.",contact->alias,msn_long_status[contact->status]);
	
	redraw_status();
}

void MSNAuthRequest(void *data)
{
	MSN_AuthMessage_PTR am;

	am = (MSN_AuthMessage_PTR)data;

	//FIXME: just add them anyway?
	
	log_printf("a%s would like to add you to their contact list.",am->handle);
	log_printf("aUse \"auth\" to authorize them.");

	MSNshiz.authdata = am;
}
void MSNLogout(void *data)
{
	log_println("aMSN automagically logged off.");
	MSNshiz.conn.status = MSN_OFFLINE;
}

void MSNError(char *f, char *s)
{
	log_printf("eERROR: (%s) %s",s,f);
	err_printf("ERROR: %s -> %s\n",s,f);
}


void cMSN_Authorize()
{
	if (MSNshiz.authdata)
	{
		if (MSN_AuthorizeContact(MSNshiz.authdata->conn, MSNshiz.authdata->requestor)==0)
			log_printf("aContact authorized.");
		else
			log_printf("eContact authorization failed.");
		
		free(MSNshiz.authdata->requestor);
		free(MSNshiz.authdata);
		MSNshiz.authdata = 0;
	}
	else
	{
		log_println("eERROR: no authorize request pending.");
	}
}

void cMSN_Login()
{
	char server[256];
	int port;
	
	strcpy(server,msn_server);
	port = atoi(msn_port);

	log_println("aLogging in...");
	update_windows();
	if ( MSN_Login(MSNshiz.conn.username, MSNshiz.conn.password, server,  port) != 0 )
	{
		append_ln("failed.");
		return;
	}
	else
	{
		append_ln("success.");
		log_printf("aLogged into MSN as %s\n",MSNshiz.conn.username);
	}

	MSNshiz.conn.status = MSN_ONLINE;

	redraw_status();
}

void cMSN_Logout()
{
	mlist cur;

	msn_contact_t *contact;

	MSN_Logout();
	
	// FIXME: set all user status to OFFLINE (DONE)
	
	for (cur = MSNshiz.contacts;cur;cur=(mlist)cur->next)
	{
		contact = (msn_contact_t *) cur->data;

		contact->status = MSN_OFFLINE;
	}
	
	redraw_status();
}

void cMSN_SendIM(msn_contact_t *contact, char *message)
{
	if (MSN_SendMessage(contact->handle, message) == 0)
	{
		err_printf("cMSN_SendIM: %s -> %s success\n",contact->alias);
		log_printf("aMessage sent.");
	}
	else
	{
		err_printf("cMSN_SendIM: %s -> %s failure\n",contact->alias);
		log_printf("eUnable to send message to %s.",contact->alias);
	}
}

void cMSN_ChangeState(int state)
{
	if (MSN_ChangeState(state) == 0 )
		err_printf("cMSN_ChangeState: %s success\n",msn_long_status[state]);
	else
		err_printf("cMSN_ChangeState: %s failure\n",msn_long_status[state]);
}

void cMSN_TermChat(char *handle)
{
	MSN_EndChat(handle);
}

void cMSN_AddContact(char *handle)
{
	msn_contact_t *contact;
	
	if (MSN_AddContact(handle) == 0)
		err_printf("cMSN_AddContact: %s -> %s success\n",handle);
	else
		err_printf("cMSN_AddContact: %s -> %s failure\n",handle);

	// FIXME: this should use the new_contact() function
	
/*
	contact = (msn_contact_t *) malloc ( sizeof(msn_contact_t));

	strcpy(contact->handle, handle);

	strncpy(contact->alias, handle, 29);

	contact->status = MSN_OFFLINE;

	MSNshiz.contacts = m_list_append(MSNshiz.contacts, contact);
*/
	new_contact(handle,handle);

}

void cMSN_RemContact(char *handle)
{
	msn_contact_t *cur;
	
	MSN_RemoveContact(handle);

	cur = GetContactByHandle(handle);

	MSNshiz.contacts = m_list_remove(MSNshiz.contacts,cur);

	free(cur);
}

msn_contact_t *GetContactByHandle( char *handle)
{
	msn_contact_t *contact;
	msn_contact_t *temp;
	mlist cur;
	int len;

	contact = 0;

	len = 0;

	err_printf("GetContactByHandle(%s)\n",handle);

	if (strchr(handle,'@'))
	{
		len = (int)(strchr(handle,'@') - strlen(handle));
	}

	for(cur = MSNshiz.contacts;cur;cur=cur->next)
	{
		temp = (msn_contact_t *) cur->data;

		if (strncasecmp(temp->handle, handle,len) == 0)
			contact = temp;
	}
	
	return contact;	
}

void m_input_add (int fd, MSN_CALLBACK func, MSN_Conn *conn )
{
	msn_sess_conn_t *sconn;
//	log_printf("am_input_add: cnx = %d conn = %d",MSNshiz.conn.cnx,conn);
	sconn = (msn_sess_conn_t *) malloc(sizeof(msn_sess_conn_t));

	sconn->fd = fd;
	sconn->conn = conn;
	sconn->in = 0;
	sconn->out = 0;

	err_printf("m_input_add: fd=%d cnx = %x sconn = %x conn = %x\n",fd,MSNshiz.conn.cnx,sconn,sconn->conn);

	MSNshiz.conn.cnx = m_list_append(MSNshiz.conn.cnx,sconn);
}
void m_input_remove ( MSN_Conn *conn )
{
	mlist cur;
	msn_sess_conn_t *sconn;


	err_printf("m_input_remove( Mainconn = %x, conn = %x, fd = %d)\n",MSNshiz.conn.cnx,conn,conn->fd);
	
	for (cur = MSNshiz.conn.cnx;cur;cur=cur->next)
	{
		sconn = (msn_sess_conn_t *)cur->data;

		err_printf("m_input_rem: cnx = %x sconn = %x conn = %x fd = %d\n",MSNshiz.conn.cnx,sconn,sconn->conn,sconn->fd);

		if ( sconn->conn->fd == conn->fd )
		{
			MSNshiz.conn.cnx = m_list_remove(MSNshiz.conn.cnx,sconn);
			free (sconn);
			return;
		}
	}
	
	// FIXME: free conn as well ?
}

void setup_fds()
{
	mlist cur;
	msn_sess_conn_t *conn;
	
	cur = MSNshiz.conn.cnx;
	conn = 0;
	FD_ZERO(&MSNshiz.fds);
	FD_SET(0, &MSNshiz.fds);
	
	for(;cur;cur=cur->next)
	{
		conn = ( msn_sess_conn_t * ) cur->data;

	//	err_printf("setup_fds() : fd = %d\n",conn->conn->fd);
		
		FD_SET(conn->conn->fd,&MSNshiz.fds);
	}

	if (conn)
		MSNshiz.maxfds = conn->fd;
	else
		MSNshiz.maxfds = 2;
}

void MSNInitShiz ()
{
	MSNshiz.contacts = 0;
	MSNshiz.events = 0;
	MSNshiz.cvars = 0;
	MSNshiz.authdata = 0;
	MSNshiz.maxfds = 0;
	MSNshiz.conn.mainconn = 0;
	MSNshiz.conn.cnx = 0;
	MSNshiz.conn.username[0] = 0;
	MSNshiz.conn.password[0] = 0;
	MSNshiz.conn.status = MSN_OFFLINE;
	gettimeofday(&MSNshiz.startup,0);
	MSNshiz.ready = 1;
}

void init_cvars ()
{	
	set_cvar("msn_auto_select","n","Change currently selected contact on event.");
	set_cvar("msn_auto_away","y","Change status to away when user logs out. (Useful when using screen.)");
	set_cvar("msn_log","y","Log instant messages to a file.");
	set_cvar("msn_debug","n","Prints various debugging data to the screen.");
	set_cvar("msn_call_ext","y","Call external programs on events.");
	set_cvar("msn_correct_graphics","n","Force rewritting to prevent graphical errors. (Needed by gnome-terminal.)");
	set_cvar("msn_auto_login","n","Automatically logon when program starts.");
	set_cvar("msn_scrollback","200","Number of lines in scrollback buffer. (IGNORED)");
	set_cvar("msn_complete_word","y","Complete a word or just display possibles.");
	set_cvar("msn_show_typing","y","Display typing user notification.");
}

void set_cvar ( char *cvar, char *value, char *help )
{
	mlist cur;
	mlist new;
	msn_cvar_t *cv;

	new = 0;

	for ( cur = MSNshiz.cvars; cur; cur=cur->next )
	{
		cv = (msn_cvar_t *)cur->data;
		if ( strcasecmp(cv->name,cvar)==0)
			new = cur;
	}

	if (new == 0)
	{
		cv = (msn_cvar_t *)malloc(sizeof(msn_cvar_t));
		cv->name = strdup(cvar);
		MSNshiz.cvars = m_list_append(MSNshiz.cvars, cv);
		cv->help = help;
	}
	else
	{
		cv = (msn_cvar_t *)new->data;
	}

	strncpy(cv->value,value,30);
}

int cvar_true ( char *cvar )
{
	mlist cur;
	int state;

	msn_cvar_t *cv;

	state = 0;

	for (cur = MSNshiz.cvars; cur; cur=cur->next)
	{
		cv = (msn_cvar_t *) cur->data;

		if (strcasecmp(cv->name,cvar)==0)
			state = (toupper(cv->value[0])=='Y');
	}

	return state;
}

int cvar_to_int ( char *cvar )
{
        mlist cur;
        int state;

        msn_cvar_t *cv;

        state = 0;

        for (cur = MSNshiz.cvars; cur; cur=cur->next)
        {
                cv = (msn_cvar_t *) cur->data;

                if (strcasecmp(cv->name,cvar)==0)
                        state = atoi(cv->value);
        }

        return state;
}

char *cvar_value ( char *cvar )
{
	mlist cur;
	msn_cvar_t *cv;

	char *value;

	for (cur = MSNshiz.cvars; cur; cur=cur->next)
        {
                cv = (msn_cvar_t *) cur->data;

                if (strcasecmp(cv->name,cvar)==0)
                        value = cv->value;
        }

	return value;
}

msn_cvar_t *find_cvar ( char *cvar )
{
	mlist cur;
	msn_cvar_t *cv;

	for (cur = MSNshiz.cvars; cur; cur=cur->next)
	{
		cv = (msn_cvar_t *) cur->data;
		if (strcasecmp(cv->name,cvar)==0)
			return cv;
	}

	return 0;
}

msn_contact_t *new_contact(char *handle, char *alias)
{
	msn_contact_t *cont;
	char *fullhandle;

	cont = (msn_contact_t *) malloc (sizeof(msn_contact_t));

	AddHotmail(handle,&fullhandle);

	strcpy(cont->handle,fullhandle);
	strncpy(cont->alias,alias,29);

	cont->status = MSN_OFFLINE; // should change straight away anyway

	MSNshiz.contacts = m_list_append(MSNshiz.contacts, cont);

	err_printf("new_contact: %s %d\n",handle,cont);

	free(fullhandle);
	
	return cont;
}

void err_printf(char *format, ...)
{
	va_list argp;
	char out[1024];

	va_start(argp,format);

	vsnprintf(out,1024,format,argp);

//	if (cvar_true("msn_debug"))
		fprintf(MSNshiz.errfile,out);

	fflush(stderr);
	
	va_end(argp);
}

// calculates uptime
void getfut(char *fut, struct timeval *tv1, struct timeval *tv2)
{
	int diff;
	int days;
	int hours;
	int minutes;
	int seconds;

	diff = tv1->tv_sec - tv2->tv_sec;

	if (diff < 0 )
		diff = -diff;

	seconds = diff % 60;
	diff /= 60;
	minutes = diff % 60;
	diff /= 60;
	hours = diff % 24;
	diff /= 24;
	days = diff;

	snprintf(fut,30,"%3d days %02d:%02d:%02d",days,hours,minutes,seconds);

}
