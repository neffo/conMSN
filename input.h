#ifndef INPUT_H
#define INPUT_H

#include <ncurses.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include "display.h"
#include "file.h"

#define MAX_INPUT 1024
#define MAX_MESSAGE 256

#define CMD_HISTORY 20

#define LOGIN_USER 100
#define LOGIN_PASSWORD 101
#define CHALIAS 102

typedef enum enum_inputtype
{
	IN_CMD,
	IN_BLOCK,
	IN_LINE,
	IN_CHAT // continous input, end with | on line by itself
} inputtype_t;

typedef struct input_s
{
	inputtype_t in_mode;
	fd_set fds;
	int len; // length of current cmd/line
	int line;
	int curspos;
//	int contact; // current contact selected, change with [ and ] and with message command
	msn_contact_t *contact; // currently selected contact
	msn_contact_t *ev_cont; // last contact to cause event
	mlist macros;
	int history_pos;
	char cmd[MAX_INPUT];
	char prompt[30]; // prompt in IN_LINE input mode
	char *inputbuf; // location of string for IN_LINE & IN_BLOCK input modes
	int input_size; // size of buffer above
	int current_cmd; // used for IN_LINE/IN_BLOCK
	int input_private; // use * instead of real characters when printing
} input_t;

typedef enum cmd_enum
{
	LOGIN,
	CHSTATUS,
	ADDCNT, // add contact
	REMCNT, // remove contact
	SNDMSG, // send message to currently selected contact
	HELP, // displays help on a command
	WRITECONT, 
	READCONT, 
	AUTH, // authorise user who last requested it
//	LISTCVAR,
	SET, // set cvar && now lists cvars as well
	CHAT,
	ABOUT,
	MACRO,
	ALIAS,
	SETUP,
	QUIT,
	MAX_CMDS
} cmdnum_t;

typedef enum complete_n
{
	CONTACTS 	= 0x0001,
	CMDS 		= 0x0002,
	STATUS		= 0x0004,
	CVARS		= 0x0010,
	MACROS		= 0x0020,
	NONE 		= 0x0040,
	ALL		= 0x0077 // i think
} complete_t;

typedef struct cmd_s
{
	cmdnum_t cmdn;
	char *cmd;
	char *args;
	char *help;
	void (*do_it)(char *);
	complete_t comp_t;
	int argc;
} cmd_t;

typedef struct macro_s
{
	char cmd[30];
	char string[256];
	cmdnum_t cmdn;
} macro_t;

#define BADCMD MAX_CMDS

void do_login(char *);
void do_chstatus(char *);
void do_sndmsg(char *);
void do_readmsg(char *);
void do_sndmsgto(char *);
void do_readmsgfrom(char *);
void do_help(char *);
void do_quit(char *);
void do_addcnt( char *);
void do_remcnt( char *);
void cvar_list( char *);
void do_chat(char *);
void do_about (char *);
void do_cvar_set (char *);
void do_setup ( char *);
void do_macro ( char *);
void do_set_alias ( char *);
msn_contact_t *prev_contact ();
msn_contact_t *next_contact ();

static cmd_t commands[MAX_CMDS+1] =
// { command #, command name, args, description, func, complete type }
{
	LOGIN, "login","", "Initiates the MSN Login process.",do_login,NONE,0,
	CHSTATUS, "status","<status>", "Changes the visible user state.",do_chstatus,STATUS,1,
	ADDCNT, "add","<handle>","Adds a new contact to your contact list.", do_addcnt,NONE,1,
	REMCNT,"rem","<handle>","Removes a contact from your list.",do_remcnt,CONTACTS,1,
	SNDMSG,"message","[<handle>]","Sends a message to currently selected contact.",do_sndmsg,CONTACTS,1,
	HELP,"help","[<command>]","Displays help associated with a command.",do_help,CMDS,1,
	WRITECONT,"writecontacts","","Saves config file.",write_contacts_file,NONE,0,
	READCONT,"readcontacts","","Parses the config file.",read_contacts_file,NONE,0,
	AUTH,"auth","","Authorise last user who requested it.",cMSN_Authorize,NONE,0,
	SET,"set","<cvar> <value>","Set cvar value.",do_cvar_set,CVARS,2,
	CHAT,"chat","[<handle>]","Enter chat with user.",do_chat,CONTACTS,1,
	MACRO,"macro","[<macro name>] [<command string>]","Create a user defined macro.",do_macro,MACROS,2,
	ALIAS,"alias","<user/alias> <new alias>","Set friendly name for user.",do_set_alias,CONTACTS,2,
	SETUP,"initial_setup","","Rerun initial setup.",do_setup,NONE,0,
	ABOUT,"about","","Display information about program.",do_about,NONE,0,
	QUIT,"quit","","Exits the program.",do_quit,NONE,0,
	MAX_CMDS,0,0,0,0,0
};

static char history[CMD_HISTORY][MAX_INPUT];
	
void input_init ();
void input_end ();
void input_process ();
void input_parse(int ch);
void cmd_complete();
void add_char(int ch);
void rem_char();
void history_up();
void history_down();
void process_cmd();
void add_to_history(char *command);
void line_addchar(int ch);
void line_remchar();
void line_docmd();
void chat_send();
void end_chat();
void process_username();
void process_password();
int get_cmd_by_string( char *string);
char *complete_word ( char *line, complete_t type);
int get_status_by_string (char *string);
void select_contact_by_alias ( char *string);
int string_to_args ( char *string, char **arg, int args );
msn_contact_t *get_contact_by_string ( char *string);
/*
#ifndef MAIN_C
#ifndef EXTERN_INPUT
#define EXTERN_INPUT
extern input_t input;
#endif
#endif
*/
#endif
