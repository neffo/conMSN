#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "msn_shiz.h"
#include "display.h"
#include "input.h"
#include "file.h"

extern msn_shiz_t MSNshiz;
extern input_t input;

extern char **environ;

void first_run ()
{
	char dir[256];
	struct stat fs;

	sprintf(dir,"%s/.msn",getenv("HOME"));

	if ( stat(dir,&fs) != 0 )
	{
		call_ext ( "mkdir", "--mode=0755", dir );
		MSNshiz.first_run = 1;
	}
}

void file_init()
{
	read_contacts();
	read_config();
	read_macros();
}

void file_end()
{
	write_contacts();
	write_config();
	write_macros();
}

void write_contacts()
{
	FILE *cfile;
	mlist cur;
	msn_contact_t *cont;
	struct stat fs;
	char dir[256];

	sprintf(dir,"%s/.msn",getenv("HOME"));

	if ( stat(dir,&fs) == 0 )
	{
		// create dir
		mkdir(dir,0755);
	}

	sprintf(dir,"%s/.msn/contacts",getenv("HOME"));
	
	cfile = fopen(dir,"w+");

	if ( cfile == NULL)
	{
	//	log_printf("eERROR: unable to write contacts file.");
		return;
	}

	cur = MSNshiz.contacts;

	for(;cur;cur=cur->next)
	{
		cont = (msn_contact_t *) cur->data;
		fprintf(cfile,"%s %s\n",cont->handle, cont->alias);
		//log_printf("aDEBUG: %s",cont->handle);
	}

	fclose(cfile);
}
void read_contacts()
{
	FILE *cfile;
	mlist cur;
	msn_contact_t *cont;
	struct stat fs;
	char dir[256];
	char handle[256];
	char alias[30];
	char line[512];
	char *space;

	sprintf(dir,"%s/.msn/contacts",getenv("HOME"));
	
/*	if ( stat(dir,&fs) == 0 )
	{
		log_println("ERROR: contacts file not found.");
		return;
	}
*/
	cfile = fopen(dir,"r");

	if (cfile == NULL)
	{
//		log_println("eERROR: unable to open contacts file");
		return;
	}

	while ( fgets ( line, 512, cfile) != NULL )
	{
		
		if (line[strlen(line)-1] == '\n')
			line[strlen(line)-1] = 0;

		space = (char *)strchr(line,' ');

		if (!space)
			goto end;
		
		*space = 0;

		cont = (msn_contact_t *)malloc(sizeof(msn_contact_t));

		strcpy(cont->handle,line);
		strcpy(cont->alias,space+1);

		cont->status = MSN_OFFLINE;

		MSNshiz.contacts = m_list_append(MSNshiz.contacts, cont);
	}

end:
	
	fclose (cfile);
}

void write_config()
{
	FILE *cfile;

	char file[256];

	mlist cur;

	msn_cvar_t *cvar;
	
	sprintf(file,"%s/.msn/config",getenv("HOME"));

	cfile = fopen(file,"w+");

	if (cfile == NULL)
	{
		//log_printf("eERROR: unable to write config file.");
		return;
	}

	fprintf(cfile,"# MSN config file\n");
	fprintf(cfile,"username: %s\n",MSNshiz.conn.username);
	fprintf(cfile,"password: %s\n",MSNshiz.conn.password);
	for(cur=MSNshiz.cvars;cur;cur=cur->next)
	{
		cvar=(msn_cvar_t *)cur->data;
		fprintf(cfile,"%s: %s\n",cvar->name,cvar->value);
	}
	
	fclose(cfile);
}

void read_config()
{
	FILE *cfile;

	char *space;

	mlist cur;

        msn_cvar_t *cvar;
	
	char line[256];
	char file[256];

	sprintf(file,"%s/.msn/config",getenv("HOME"));
	
	cfile = fopen(file,"r");

	if (cfile == NULL)
	{
		//log_printf("eERROR: unable to read config file");
		return;
	}

	while ( fgets(line,256,cfile) )
	{
		line[strlen(line)-1] = 0;

		space = (char *) strchr(line,' ');

		if (strncasecmp(line,"username",8)==0)
			strcpy(MSNshiz.conn.username,space+1);

		if (strncasecmp(line,"password",8)==0)
			strcpy(MSNshiz.conn.password,space+1);


		for(cur=MSNshiz.cvars;cur;cur=cur->next)
		{
			cvar = (msn_cvar_t *)cur->data;
			if (strncasecmp(cvar->name,line,strlen(cvar->name))== 0)
			{
				strcpy(cvar->value,space+1);
			}
		}

		
	}

	fclose(cfile);
}

void write_macros()
{
	FILE *cfile;
	char file[256];
	mlist cur;
	macro_t *macro;

	sprintf(file,"%s/.msn/macros",getenv("HOME"));
	cfile = fopen(file,"w+");

	if (cfile == NULL)
	{
		log_printf("eERROR: unable to write macro file.");
		return;
	}

	for(cur=input.macros;cur;cur=cur->next)
	{
		macro=cur->data;
		fprintf(cfile,"%s %s %s\n",macro->cmd,commands[macro->cmdn].cmd,macro->string);
	}
	fclose(cfile);
}

void read_macros()
{
	FILE *cfile;
	char file[256];
	char line[256];
	char *space;
	char *space2;
	int cmd;
	int c;
	
	mlist cur;
	macro_t *macro;

	sprintf(file,"%s/.msn/macros",getenv("HOME"));
	cfile = fopen(file,"r");

	if (cfile == NULL)
	{
	//	log_printf("eERROR: unable to read macro file.");
		return;
	}
	c = 0;
	while ( fgets(line,256,cfile) )
	{
		// "reply message %s"
		//  ^file ^space  ^space2
		//       0       0  0
		line[strlen(line)-1] = 0;
		space = (char *) strchr(line,' ');
		*space = 0;
		space++;
		space2 = (char *) strchr(space,' ');
		*space2 = 0;
		space2++;
		cmd = get_cmd_by_string( space );
		//err_printf("read_macros: %s %s %s\n",line,space,space2);
		if (cmd)
		{
			macro = (macro_t *) malloc (sizeof(macro_t));
			macro->cmdn = cmd;
			strcpy(macro->string,space2);
			strcpy(macro->cmd,line);
			input.macros = m_list_append(input.macros,macro);
			c++;
		}
	}

	err_printf("%d macros read.\n",c);

	fclose(cfile);
}

void log_event ( char *alias, char *message, int day, int month, int hour, int min )
{
	FILE *log;

	char filename[256];

	// FIXME: should probably have a function to ensure alias is a 
	// valid filename

	snprintf(filename,256,"%s/.msn/%s.log",getenv("HOME"),alias);

	log = fopen(filename,"a");

	if (log == NULL)
	{
		log_println("eERROR: unable to open log file. (perhaps disk is full)");
		return;
	}

	fprintf(log,"[ A: %s D: %d/%d T: %d:%d ]\n",alias,month,day,hour,min);
	fprintf(log,"%s\n",message);
	fprintf(log,"~\n");

	fclose(log);
}

/* secure way to call external programs (system can't be interupted) */

// these *SHOULD* check if the program executed correctly
void ext_msg (char *user, char *msg )
{
	if (call_ext(EXT_MSG,user,msg))
		log_printf("eERROR: ext_msg() unable to execute external command.");		
}

void ext_onl (char *user, char *status)
{
	call_ext(EXT_ONL,user,status);
}

void ext_mail (char *mail, char *mail2)
{
	call_ext(EXT_MAIL,mail,mail2);
}

int call_ext ( char *cmd, char *d1, char *d2 )
{
	int pid, status;
	char *argv[4];
	char command[1024];
	char file[128];
	struct stat fs;

	/*
	pid = fork();

	if (pid == -1)
		return -1;
	*/

	snprintf(file,128,"%s/.msn/%s",getenv("HOME"),cmd);

        if ( stat(file,&fs) == 0 || fs.st_mode & S_IXUSR )
        {
		log_printf("eERROR: external command (%s) not found.",file);
		return -1;
	}

	snprintf(command,1024,"%s %s \"%s\"",cmd,d1,d2);

//	log_printf("eERROR: command = %s",command);
/*
	if (pid == 0) {
		argv[0] = "sh";
		argv[1] = "-c";
		argv[2] = command;
		argv[3] = 0;

		execve(command,argv, environ);
		exit(127);
	}
	do {
		if (waitpid(pid, &status, 0) == -1 ) {
			if (errno != EINTR)
				return -1;
		} else
			return status;
	} while (1);
*/

	system(command);

	return 0;
}	

