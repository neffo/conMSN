/*
 * these functions use the utmp file to check if a user is currently logged in.
 * they work best if you are logging in remotely, and running this program in
 * "screen". when you log in your status will switch to online, when you log out
 * your status will switch to away. it doesn't like local logins (tty or X).
 *
 */

#include <utmp.h>
#include <signal.h>
#include "msn_shiz.h"
#include "display.h"

#define CHECK_TIME 120

extern msn_shiz_t MSNshiz;

void CheckLoggedIn(int a1, int a2)
{
	// FIXME : check idle time as well, use "busy" mode or something

	int found;
	char *username;
	struct utmp *ut;

	username = (char *)getenv("USER");

	setutent();

	found = 0;
	
	while( (ut = getutent()) )
		if (ut->ut_type == 7) // login pty? ignore screen, X and locals
		{
			if (strncasecmp(username,ut->ut_user,8)==0)
				found++;
		}

	endutent();

	if (cvar_true("msn_auto_away") && found)
	{
		// do online stuff
		if (MSNshiz.conn.status == MSN_EXTAWAY)
		{
			cMSN_ChangeState(MSN_ONLINE);
			log_printf("aAutomagically set as online.");
		}
	}
	else
	{
		if (MSNshiz.conn.status == MSN_ONLINE)
		{
                        cMSN_ChangeState(MSN_EXTAWAY);
                	log_printf("aAutomagically set as away.");
		}
	}

	alarm(CHECK_TIME);
}

void setup_autoaway()
{
	signal(SIGALRM,(void *)CheckLoggedIn);

	alarm(CHECK_TIME);
}
