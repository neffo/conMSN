#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include "input.h"
#include "errno.h"

extern input_t input;
extern cdisplay_t display;
extern msn_shiz_t MSNshiz;

extern int quit;


void input_init (/* input_t *input*/)
{
	input.in_mode = IN_CMD;
	input.cmd[0]=0;
	input.contact=0;
	input.line=0;
	input.len=0;
	input.curspos=0;
	input.history_pos=0;
	input.inputbuf = 0;
	input.input_private = 0;
	input.macros = 0;
	input.ready = 1;
	memset(&history,0,sizeof(char) * CMD_HISTORY * MAX_INPUT);
	if (display.ready)
		do_about(0);
}

void input_end (/* input_t *input*/)
{
//	FD_ZERO(&input.fds);
}

void input_process ( /*input_t *input*/)
{
	int ch;
//	char errstr[256];

	ch = getch();
	if (ch != ERR)
		input_parse(ch);
}

void cmd_complete()
{
	int count;
	int i;
//	int cmds[MAX_CMDS];
	int match; 
	char *space;
	char *cmp;
	char matches[1024];

	char *str;
	
	count = 0;
	match = -1;

	if (input.len == 0)
		return;

	space = strchr(input.cmd,' ');

	*matches = 0;

	if (!space)
	{

/* old method		
		for (i=0; commands[i].cmdn < BADCMD; i++)
		{
			if ( strncasecmp(input.cmd,commands[i].cmd,input.len) == 0)
			{
				match = i;
				count++;
			}
		}
*/

		match = 0;
		
		match = get_cmd_by_string ( complete_word( input.cmd, CMDS, matches) );

		if ( match > -1 )
		{	
			if ( cvar_true("msn_complete_word") )
			{
				sprintf(input.cmd,"%s ",commands[match].cmd);
				input.len = strlen(commands[match].cmd)+1;
				input.curspos = input.len;

				redraw_cmd_line();
			}
		}
		else
		{
				log_println("eNo matches.");
		}
	}
	else
	{
		match = get_cmd_by_string(input.cmd);

		if (match < 0)
			goto end;
		
		cmp = 0;

		cmp = complete_word(space+1,commands[match].comp_t, matches);

		if (cmp)
		{
			if ( cvar_true("msn_complete_word") )
			{
				strcpy(space+1,cmp);

				input.len = strlen(input.cmd);
				input.curspos = input.len;
				redraw_cmd_line();
			}
		}
		else
		{
			log_printf("eNo matches for %s.",space+1);
		}

		end:
		// skip all that crap above
	}

	if ( *matches && strchr(matches,' '))
		log_printf("aMatches: %s",matches);

}

char *complete_word ( char *word, complete_t type, char *matches)
{
	// FIXME: use new method so as to allow completion of 2 or more
	// different types (ie use "if (type & CMDS)" )
	char *ret;
	mlist cur;
	int i;
	int c;
	int p;

	msn_contact_t *cont;
	msn_cvar_t *cvar;
	macro_t *macro;

	ret = 0;
	cont = 0;
	c=0;
	p=0;

//	log_printf("DEBUG: _%s_ %d",word, type);
//
	
	matches[0]=0;
	
	switch (type)
	{
	case CMDS:
//		log_printf("DEBUG: type = CMDS");
		for (i=0; commands[i].cmdn < BADCMD; i++)
		{
			if ( strncasecmp(word,commands[i].cmd,strlen(word)) == 0 )
			{
				if ( p < commands[i].pri )
				{
					ret = commands[i].cmd;
					p = commands[i].pri;
				}
//				matches[c]=commands[i].cmd;
				if (*matches)
					strcat(matches," ");
				strcat(matches, commands[i].cmd);
				c++;
			}
		}
		break;
	case NONE:
	case CONTACTS:
//		log_printf("DEBUG: type = CONTACTS/NONE");
		cur = MSNshiz.contacts;

		for(;cur;cur=cur->next)
		{
			cont = (msn_contact_t *)cur->data;
			if (strncasecmp(word,cont->alias,strlen(word))== 0)
			{
				ret = cont->alias;
				//matches[c]=cont->alias;
				strcat(matches," ");
                                strcat(matches,ret);
				c++;
			}
			else if (strncasecmp(word,cont->handle,strlen(word))== 0)
			{
				ret = cont->handle;
				//matches[c]=cont->handle;
				strcat(matches," ");
                                strcat(matches,ret);
				c++;
			}
		}
		break;
	case STATUS:
//		log_printf("DEBUG: type = STATUS");
		for(i=0;i<MAX_STATES;i++)
		{
			if ( strncasecmp(word,msn_long_status[i],strlen(word))== 0 
					|| strcasecmp(word,msn_short_status[i]) == 0)
			{
				ret = msn_long_status[i];
//				matches[c] = ret;
				strcat(matches," ");
                                strcat(matches,ret);
				c++;
			}
		}
		break;
	case CVARS:

		for (cur = MSNshiz.cvars;cur;cur=cur->next)
		{
			cvar = (msn_cvar_t *)cur->data;
			if (strncasecmp(word,cvar->name,strlen(word))==0)
			{
				ret = cvar->name;
//				matches[c] = ret;
				strcat(matches," ");
                                strcat(matches,ret);
				c++;
			}
		}
		break;
	case MACROS:

		for (cur = input.macros; cur; cur=cur->next)
		{
			macro = cur->data;
			if (strncasecmp(word, macro->cmd, strlen(word))==0)
			{
				ret = macro->cmd;
//				matches[c] = ret;
				strcat(matches," ");
                                strcat(matches,ret);
				c++;
			}

		}
		break;
	}

	return ret;
}

void input_parse(int ch)
{
	if (input.in_mode == IN_CMD)
	{
		switch (ch)
		{
			case '\t':
				cmd_complete();
				break;
			case '\n':
				if (input.len)
				{
					process_cmd();
				}
				break;
			case KEY_UP:
				if (input.history_pos == 0)
				{
					add_to_history(input.cmd);
					history_up();
				}
				history_up();
				break;
			case KEY_DOWN:
				if (input.history_pos == 0)
				{
					add_to_history(input.cmd);
					//history_up();
				}
				history_down();
				break;
			case KEY_PPAGE:
				log_scroll_up();
				break; 
			case KEY_NPAGE:
				log_scroll_down();
				break;
			case KEY_BACKSPACE:
			case 127:
				rem_char();
				break;
			case '[':
				input.contact = prev_contact();
				break;
			case ']':
				input.contact = next_contact();
				break;
			case KEY_F(1):
				// send message to selected user
				break;
			case KEY_F(2):
				// view event
				break;
			case KEY_RESIZE:
				log_printf("Terminal resized.");
				break;
			default:
				add_char(ch);
		}
	}
	else if ( input.in_mode == IN_LINE )
	{
		switch (ch) {
			
		case '\t':
		case KEY_F0:
		case KEY_UP:
		case KEY_DOWN:
			break;
			
		case KEY_PPAGE:
			history_up();
			break;
			
		case KEY_NPAGE:
			history_down();
			break;
			
		case KEY_BACKSPACE:
		case 127:
			line_remchar();
			break;
		case '\n':
			line_docmd();
			break;
		default:
			line_addchar(ch);
		}
			
	}
	else if ( input.in_mode == IN_BLOCK )
	{

	}
	else if ( input.in_mode == IN_CHAT )
	{
		switch (ch) {
			
		case '\t':
		case KEY_F0:
		case KEY_UP:
		case KEY_DOWN:
			break;

		case KEY_PPAGE:
			history_up();
			break;

		case KEY_NPAGE:
			history_down();
			break;

		case '[':
			input.contact = prev_contact();
			snprintf(input.prompt,29,"%s>",input.contact->alias);
			log_printf("aNow chatting to %s.",input.contact->alias);
			redraw_cmd_line();
			break;
			
		case ']':
			input.contact = next_contact();
			snprintf(input.prompt,29,"%s>",input.contact->alias);
			log_printf("aNow chatting to %s.",input.contact->alias);
			redraw_cmd_line();
			break;

		case KEY_BACKSPACE:
		case 127:
			line_remchar(); 
			break;
		case '\n':
			if ( input.input_size > 0 )
				chat_send();
			else
				end_chat();
			break;
		default:
			line_addchar(ch);
		}
	}
}

void add_char(int ch)
{
	//char chr[10];
	if (input.len < MAX_INPUT)
	{
		input.cmd[input.len]=ch;
		input.len++;
		input.cmd[input.len]=0;
		input.curspos++;
	}
	else
	{
		log_println("eInput buffer full.");
	}
	/*sprintf(chr,"%d",ch);
	log_println(chr);*/
	redraw_cmd_line();
}

void line_addchar(int ch)
{
	if (input.input_size < MAX_MESSAGE)
	{
		input.inputbuf[input.input_size] = ch;
		input.input_size++;
		input.inputbuf[input.input_size] = 0;
	}
	else
	{
		log_println("eMessage buffer full.");
	}

	redraw_cmd_line();
}

void rem_char()
{
	if (input.len > 0)
	{
		input.cmd[input.len-1] = 0;
		input.len--;
		input.curspos--;
	}
	redraw_cmd_line();
//	display.wcmd.changed = 1;
}

void line_remchar()
{
	if (input.input_size > 0)
	{
		input.inputbuf[input.input_size-1] = 0;
		input.input_size--;
	}
	redraw_cmd_line();
}

void line_docmd()
{
	int ignore=0;
	
	switch (input.current_cmd)
	{
	case LOGIN_USER:
		strcpy(MSNshiz.conn.username,input.inputbuf);
		input.current_cmd = LOGIN_PASSWORD;
		strcpy(input.prompt,"Password>");
		input.input_private = 1;
		input.in_mode = IN_LINE;
		input.cmd[0] = 0;
		input.inputbuf = input.cmd;
		input.input_size = 0;
		ignore = 1; 	// hack so it doesn't overwrite some of the above 
				// values
		break;
	case LOGIN_PASSWORD:
		strcpy(MSNshiz.conn.password,input.inputbuf);
		input.input_private = 0;
		break;
	case SNDMSG:
		if (input.input_size)
		{
			log_printf("mSending message to %s ... ",input.contact->handle);
			if (MSN_SendMessage(input.contact->handle,input.inputbuf) == 0)
			{
				append_ln("sent.");
				log_printf("m%s > %s",input.contact->handle,input.inputbuf);
			}
			else
			{
				append_ln("failed.");
			}
		}
		else
		{
			log_printf("aMessage aborted.");
		}
		break;
	case CHALIAS:
		break;
	default:
		log_println("eBad input.current_cmd value.");

	}
	if (!ignore)
	{
		input.in_mode = IN_CMD;
		input.cmd[0] = 0;
	}
	redraw_cmd_line();
}

void history_up ()
{
	input.history_pos--;

	if (input.history_pos < 0)
		input.history_pos = CMD_HISTORY - 1;

	strcpy(input.cmd,history[input.history_pos]);
	input.len = strlen(input.cmd);
	input.curspos = input.len;
	
	redraw_cmd_line();
	
	display.wcmd.changed = 1;
}

void history_down()
{
	input.history_pos++;

	if (input.history_pos == CMD_HISTORY)
		input.history_pos = 0;

	strcpy(input.cmd,history[input.history_pos]);
	input.len = strlen(input.cmd);
	input.curspos = input.len;
	
	redraw_cmd_line();
	
	display.wcmd.changed = 1;
}

void add_to_history(char *command)
{
	int i;

	for (i=0; i<CMD_HISTORY-1; i++)
	{
		if (*history[i+1])
			strcpy(history[i],history[i+1]);
	}

	strcpy(history[CMD_HISTORY-1],command);
}

void process_cmd()
{
	char cmd[1024];
	int i;
	cmd_t *command;
	mlist cur;
	msn_cvar_t *cvar;
	char *arg;
	char *args[2];
	char *eq;
	int match;
	macro_t *macro;
	msn_contact_t *cont;
	
	i=0;
	command=0;
	arg=0;
	cmd[0]=0;
	eq = 0;

	add_to_history(input.cmd);

	//eq = strchr(input.cmd,'=');

	if (!eq)
	{
		//err_printf("before: %s",input.cmd);
		if (string_to_args(input.cmd,args,2) != 2 )
			goto endbit;
		//err_printf("after: %s",args[1]);
		
		command = &commands[get_cmd_by_string( args[0] )];

		if (command == 0)
			goto endbit;

		if ( (command->comp_t & CONTACTS ) && command->argc == 1 )
		{
			cont = get_contact_by_string (args[1]);
	//		args[1] = cont->handle;
		}

		clr_cmdline();
		
		command->do_it(args[1]);
	
		endbit:
		if (!command)
		{
			//log_println("eUnknown command.");

			for ( cur = input.macros; cur; cur=cur->next)
			{
				macro = cur->data;
				if (strcasecmp(macro->cmd,cmd)==0)
				{
					command = &commands[macro->cmdn];
					arg=input.cmd;
					sprintf(arg,macro->string,input.ev_cont->handle);
					clr_cmdline();
					command->do_it(arg);
				}
				
			}
		}
	}
	else
	{
	/*	this should never be called anyway
	
		arg = eq + 1;
		strncpy(cmd,input.cmd,eq-input.cmd);

		if (*arg == 0)
			goto nodata;
		match = -1;
		for(cur = MSNshiz.cvars; cur; cur=cur->next)
		{
			cvar = (msn_cvar_t *) cur->data;

			if ( strncasecmp(cvar->name,cmd,eq-input.cmd) == 0 )
			{
				strncpy(cvar->value,arg,30);
				match = 1;
			}
		}

		if (match < 0)
			log_printf("eERROR: unknown cvar (%s)",cmd);

nodata:
*/
	}
/*
 * handled by clr_cmdline above now
 *
	input.len=0;
	input.cmd[0]=0;
	input.curspos=0;
	input.history_pos=0;
	redraw_cmd_line();
*/
}

void clr_cmdline()
{
	input.len=0;
	input.cmd[0]=0;
	input.curspos=0;
	input.history_pos=0;
	redraw_cmd_line();
}
		
void do_login(char *string)
{
	cMSN_Login();
}
void do_chstatus(char *mode)
{
	int status;	

	status = get_status_by_string(mode);

	if (status > 0)
	{
		if ( MSNshiz.conn.status == MSN_OFFLINE )
			cMSN_Login(); //log on first

		if ( MSNshiz.conn.status != MSN_OFFLINE )
		{
			cMSN_ChangeState(status);
			MSNshiz.conn.status = status;
		}
	}
}

void do_sndmsg(char *user)
{
	if (MSNshiz.conn.status == MSN_OFFLINE)
		log_printf("eNot connected.");
	else
	{

		if (*user)
			select_contact_by_alias ( user );
		
		if (!input.contact)
		{
			log_printf("eNo contact selected.");
			return;
		}
	
		log_printf("aSending message to %s:",input.contact->handle);
		log_println("a<ENTER> to send, empty line to abort");

		//strcpy(input.prompt,"MSG:>");
		snprintf(input.prompt,29,"%s>",input.contact->alias);
	
		input.in_mode = IN_LINE;
		input.cmd[0] = 0;
		input.inputbuf = input.cmd;
		input.input_size = 0;
		input.current_cmd = SNDMSG;
	}
}
void do_readmsg(char *user)
{
}
void do_sndmsgto(char *user)
{
}
void do_readmsgfrom(char *user)
{
}
void do_help(char *command)
{
	int i;
	int match;

	if (*command)
		match = get_cmd_by_string(command);
	else
		match = -1;

	if (match == -1 )
	{
		log_println("--[HELP]");

		log_println("aKeys :");
		log_println("  [ & ] : select previous/next contact.");
		log_println("  PageUp & PageDn : scroll log up/down.");
		log_println("  Up & Down Arrows : command history.");
		
		for (i=0;commands[i].cmdn<BADCMD;i++)
		{
			log_printf("a%s %s:",commands[i].cmd,commands[i].args);
			log_printf("  %s",commands[i].help);
		}
	}
	else
	{
		//log_printf("Help on %s",commands[match].cmd);
		log_printf("a%s %s:",commands[match].cmd,commands[match].args);
		log_printf("  %s",commands[match].help);
	}
	log_println("-!");
}

void do_quit(char *string)
{
	quit=1;
}

void do_chat (char *handle)
{
	if (*handle)
		select_contact_by_alias ( handle);

	if (!input.contact)
	{
		log_printf("eNo contact selected.");
		return;
	}
	
	log_printf("aChatting to %s (%s):",input.contact->alias,input.contact->handle);
	log_println("aPress <ENTER> to send. Empty line ends chat.");
	log_println("aUse [ & ] to select user.");
			
	snprintf(input.prompt,29,"%s>",input.contact->alias);
	input.in_mode = IN_CHAT;
	input.cmd[0] = 0;
	input.inputbuf = input.cmd;
	input.input_size = 0;

	redraw_cmd_line();
}

void do_about (char *data)
{
	struct timeval tv;
	char fut[30];

	gettimeofday(&tv,0);

	getfut(fut,&tv,&MSNshiz.startup);
	
	log_printf("--[ABOUT]");
	log_printf("a%s",PROG_NAME);
	log_printf("a%s",PROG_VERSION);
	log_printf("Type \"help\" for a list of commands.");
	if (MSNshiz.ready == 1)
	{
		log_printf("  ");
		log_printf("Uptime: %s", fut);
	}
	log_printf("-");
}

void chat_send ()
{
	if (MSNshiz.conn.status == MSN_OFFLINE)
	{
		log_printf("eNot connected.");
		goto end;
	}
	
	if (MSN_SendMessage(input.contact->handle,input.inputbuf)==0)
	{
		// succeess
		log_printf("m%s > %s",input.contact->alias, input.inputbuf);
		err_printf("chat_send (success): %s > %s \n",input.contact->alias,input.inputbuf);
	}
	else
	{
		// failure
		log_printf("eERROR: unable to send message.");
		err_printf("chat_send (failure): %s > %s \n",input.contact->alias,input.inputbuf);
	}

//	log_printf("m%s > %s",input.contact->alias, input.inputbuf);

end:
	
	*input.inputbuf = 0;
	input.input_size = 0;

	redraw_cmd_line();
}

void end_chat ()
{
	log_println("aEnding chat.");
	input.in_mode = IN_CMD;
	input.cmd[0] = 0;
	input.len = 0;
	input.curspos = 0;

	redraw_cmd_line();
}

void do_addcnt( char *handle)
{

}

void do_remcnt( char *handle)
{

}

int get_cmd_by_string( char *string)
{
	int cmdn;
	int i;

	char *space;

	cmdn = -1;
	i = 0;

	if (!string)
		return -1;

	if (*string)
		space = strchr(string,' ');
	else
		space = string+strlen(string);

	while ( i < MAX_CMDS )
	{
		if (strncasecmp(string,commands[i].cmd,space-string)==0)
			cmdn = i;
		i++;
	}

	return cmdn;
}

msn_contact_t * prev_contact ()
{
	mlist cur;

	if (!input.contact)
	{
		if (MSNshiz.contacts)
			return MSNshiz.contacts->data;
		else
			return NULL;
	}
	
	cur =  m_list_find ( MSNshiz.contacts, input.contact );

	if ( cur && cur->prev)
		cur =  cur->prev;
	if (cur)
		return cur->data;
	else
		return NULL;
}

msn_contact_t * next_contact ()
{
	mlist cur;

	if (!input.contact)
	{
		if (MSNshiz.contacts)
			return MSNshiz.contacts->data; // return the first one
		else
			return NULL;
	}
	
	cur = m_list_find ( MSNshiz.contacts, input.contact );

	if ( cur && cur->next )
		cur = cur->next;

	if (cur)
		return cur->data;
	else
		return NULL;
}

void cvar_list ( char *string)
{
	mlist cur;
	msn_cvar_t *cvar;
	
	log_println("--[CVARS]");
	for(cur=MSNshiz.cvars;cur;cur=cur->next)
	{
		cvar = (msn_cvar_t *) cur->data;
		log_printf("a%s = %s",cvar->name,cvar->value);
		log_printf(" %s",cvar->help);
	}
	log_println("-!");
}

int get_status_by_string (char *string)
{
	int i;
	int status;

	status = -1;
	
	for (i=0;i<MAX_STATES;i++)
	{
		if ( strncasecmp(string,msn_short_status[i],strlen(string))==0 || strncasecmp(string,msn_long_status[i],strlen(string))==0 )
		{
			status = i;
		}
	}

	return status;
}

void select_contact_by_alias ( char *string)
{
//	mlist cur;
	msn_contact_t *cont;

	cont = get_contact_by_string ( string );

	if (cont)
		input.contact=cont;
}

void do_cvar_set ( char *string )
{
	char *space; // value
	char cvarn[128];
	msn_cvar_t *cv; 

	space = strchr(string,' ');

	cv = find_cvar ( string );

	if (!cv && !space)
	{
		cvar_list(0);
		return;
	}

	if (space != 0)
	{
		*space=0;
		space++;
		set_cvar ( string, space, 0);
		log_printf("a%s set to %s",string,space);
	}
	else
	{
		space = cvar_value( string );

		log_printf("a%s = %s",string, space);
	}
}

void do_macro ( char *string )
{

	// string = "reply message %s"
	//           ^str  ^space  ^space2
	char *space; // start of macro
	char *space2; // macro bit just after command
	int cmd;
	macro_t *macro;
	macro_t *found;
	mlist cur;

	found = 0;
	
	space = strchr(string,' ');

	if (space)
	{
		*space=0;
		space++;
	}

	for (cur = input.macros; cur; cur=cur->next)
	{
		macro = cur->data;

		if (strncasecmp(macro->cmd,string,strlen(macro->cmd))==0)
			found = macro;
	}

	if (!found && space) // create new macro
	{
		space2 = strchr(space,' ');

		if (space2)
		{
			*space2 = 0;
			space2++;
		}

		cmd = get_cmd_by_string( space );

		if (cmd)
		{
			macro = (macro_t *) malloc (sizeof(macro_t));
			macro->cmdn = cmd;
			strcpy(macro->string,space2);
			strcpy(macro->cmd,string);

			input.macros = m_list_append(input.macros,macro);
		}
		else
		{
			log_printf("eInvalid command name.");
		}
	}
	else if (found && space) // set macro to value of space
	{
		space2 = strchr(space,' ');

		if (space2)
		{
			*space2 = 0;
			space2++;
		}

		cmd = get_cmd_by_string( space );

		if (cmd)
		{
			macro = found;
			macro->cmdn = cmd;
			strcpy(macro->string,space2);
			strcpy(macro->cmd, string);
		}
		else
			log_printf("eInvalid command name.");

	}
	else if (found) // print cvar value
	{
		log_printf("a%s -> \"%s %s\"",found->cmd, commands[found->cmdn].cmd, found->string);
	}
	else
	{
		for (cur = input.macros;cur; cur=cur->next)
		{
			found = cur->data;

			log_printf("a%s -> \"%s %s\"",found->cmd, commands[found->cmdn].cmd, found->string);
		}
	}
			
}

void do_setup ( char *string )
{
	struct stat fs;
	char dir[256];

	log_println("aPlease enter your MSN details.");
	log_println("Include \"@hotmail\" in username.");

	input.in_mode = IN_LINE;
	input.current_cmd = LOGIN_USER;
	input.cmd[0] = 0;
	input.inputbuf = input.cmd;
	input.input_size = 0;
	strcpy(input.prompt,"Username>");
}


// if user = '-' then user = currently selected user
// that way you can rename someone if they have a dodgy alias (with spaces or
// something)
void do_set_alias ( char *string)
{
	msn_contact_t *cont;
	char *args[2]; // 0 = user 1 = new alias

	cont = 0;
	
	err_printf("do_set_alias %s\n",string);
	
	if ( string_to_args(string, args, 2) == 2)
	{
		if ( strcmp(args[0],"-")== 0 )
		{
			cont = input.contact;
		}
		else
		{
			cont = get_contact_by_string ( args[0] );
		}

		if (cont)
		{
			strcpy(cont->alias,args[1]);

			redraw_status();
		}
		else
		{
			log_printf("eUnknown contact.");
		}
	}
	else
	{
		log_printf("eInvalid number of args.");
	}
	
}


// alters string (adds NULLs to divide it up into words)
// if important strdup() it before hand
int string_to_args ( char *string, char **arg, int args )
{
	char *space;
	int i;

	i=0;
	
	for (space = string; i < args && space; i++)
	{
		if (space && i)
		{
			*space = 0;
			space++;
		}
		
		arg[i] = space;

		space = strchr(space,' ');

		/*if (space && i)
		{
			*space=0;
			space++;
		}*/
	}

	err_printf("string_to_args() %d -> %d\n",args,i);
	
	return i;
}

msn_contact_t *get_contact_by_string ( char *string)
{
	mlist cur;
	msn_contact_t *cont;

	for (cur=MSNshiz.contacts;cur;cur=cur->next)
	{
		cont = cur->data;
		if (strncasecmp(string,cont->handle,strlen(string))==0)
			return cont;
		if (strncasecmp(string,cont->alias,strlen(string))==0)
			return cont;
	}

	return 0;
}

/*mlist find_matches( char *string, char *matching, complete_t cmp_t)
{
	mlist cur;
	mlist cur2;
	mlist matches;
	char *match;
	cvar_t cv;
	
	
	*matches=0;

	if (cmp_t == CVARS)
	{
		cur = MSNshiz.cvars;

		while (; cur;cur=cur->next)
		{
			match = 0;
			cv = cur->data;
			if (strncasecmp(string,cv->name,strlen(string)) == 0)
			{
				match = strdup(cv->name);
				matches = m_list_append(matches,match);
			}
		}

		cur = matches;
		cur2 = cur->next;

		while (;cur;cur=cur->next)
		{
			while (;cur2;cur2=cur2->next)
			{
				if
			}
			
		}
	}

	
}
*/
