#ifndef MSN_SHIZ_H

#define MSN_SHIZ_H

#include "libmsn.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>

#define MSG_IM
#define MSG_MAIL

#define MAX_EVENTS 30

#define PROG_NAME "conMSN"
//#define PROG_VERSION "v"

static char * msn_status_strings[] =
{"","OFFLINE","HIDDEN","BUSY","AWAY","BRB","AWAY","PHONE","LUNCH"};

static char * msn_short_status[] =
{ "1","0","H","B","X","S","A","P","L"};

static char * msn_long_status[] =
{ "Online","Offline","Hidden","Busy","ExtAway","BRB","Away","Phone","Lunch"};

typedef enum msn_state_n {
	MSN_ONLINE,
	MSN_OFFLINE,
	MSN_HIDDEN,
	MSN_BUSY,
	MSN_EXTAWAY,
	MSN_BRB,
	MSN_AWAY,
	MSN_PHONE,
	MSN_LUNCH,
	MAX_STATES
} msn_state_t;

static char msn_server[255] = "messenger.hotmail.com";
static char msn_port[10] = "1863";

typedef struct msn_sess_s
{
	int fd;
	MSN_Conn *conn;
	MSN_CALLBACK cb;
} msn_sess_conn_t;

typedef struct msn_conn_s
{
	void *mainconn; // storage for MSNConn struct
	mlist cnx; // list of connections (msn_sess_cont_t types to be precise)
	char username[256];
	char password[256];
	int status; // status of user (ie. me)
} msn_conn_t;

typedef struct msn_cvar_s
{
	char value[30]; // also used to store ints
	char *name;
	char *help;
	int type;
} msn_cvar_t;

typedef struct msn_contact_s
{
	char handle[256];
	char alias[30];
	int status;
//	void *next;
} msn_contact_t;

typedef struct msn_event_s
{
	int type; // new mail or instant message
	int user; // contact #
	char message[1024];
//	void *next;
} msn_event_t;

typedef struct msn_shiz_s
{
	msn_conn_t conn;
	mlist contacts;
	mlist events;
	mlist cvars;
	int nevents;
	MSN_AuthMessage_PTR authdata;
	fd_set fds;
	int maxfds;
	int first_run;
	//FILE *errfile;
	void *errfile;
	struct timeval startup;
	int ready;
} msn_shiz_t;

void MSNInitShiz(void);
void MSNRegisterCallbacks(void);

// call backs (ie called by libmsn)
void MSNInstantMessage(void *data); // called when we receive a new message
void MSNMail(void *data); // called on arrival of new mail
void MSNStatusChange(void *data); // called whenever someones status changes
void MSNAuthRequest(void *data); // called by libmsn when someone asks for auth
void MSNLogout(void *data); // called by libmsn when we get the boot
void MSNError(char *, char *);


// called by client
void cMSN_Authorize(); // called after our user says yes to auth req
void cMSN_Login();
void cMSN_Logout();
void cMSN_SendIM(msn_contact_t *contact, char *message);
void cMSN_ChangeState(int state);
void cMSN_TermChat(char *handle);
void cMSN_AddContact(char *handle);
void cMSN_RemContact(char *handle);

msn_contact_t *GetContactByHandle( char *handle);

void m_input_add (int, MSN_CALLBACK, MSN_Conn * );
void m_input_remove ( MSN_Conn * );
void setup_fds();

msn_contact_t *new_contact(char *, char *);

void init_cvars();
void set_cvar ( char *cvar, char *value, char *help );
int cvar_true ( char *cvar );
int cvar_to_int ( char *cvar );
char *cvar_value ( char *cvar );
msn_cvar_t * find_cvar ( char *cvar );

void err_printf(char *format, ...);

void getfut(char *, struct timeval *, struct timeval *);


#endif
