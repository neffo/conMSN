#include <utmp.h>
#include <signal.h>
#include "msn_shiz.h"
#include "display.h"

#define CHECK_TIME 120

extern msn_shiz_t MSNshiz;

void CheckLoggedIn(int a1, int a2)
{
	int found;
	char *username;
	struct utmp *ut;

	username = (char *)getenv("USER");

	setutent();

	found = 0;
	
	while( (ut = getutent()) )
		if (ut->ut_type == 7) // login?
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
