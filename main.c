#include <curses.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define MAIN_C

#include "display.h"
#include "input.h"
#include "msn_shiz.h"

cdisplay_t display;
input_t input;
msn_shiz_t MSNshiz;

//MSN_Conn mainMSNConnection;

int quit;

extern int errno;

//#define TEST 

int main( void )
{
	int res;
	mlist cur;
	msn_sess_conn_t *conn;

#ifdef USE_ERR_LOG

	MSNshiz.errfile = fopen("/home/ineffable/.msn/error.log","a+");

	if ( MSNshiz.errfile == NULL )
		MSNshiz.errfile = stderr;

	err_printf("conMSN starting up...\n");

#else
	MSNshiz.errfile = stderr;
#endif	

	screen_setup();
	draw_windows();
	input_init();

	MSNInitShiz();
	redraw_cmd_line();
	init_cvars();

	quit = 0;

	MSNRegisterCallbacks();

	read_contacts_file();
	read_config();
	
	redraw_status();
	update_windows();

	setup_autoaway();

	if (cvar_true("msn_auto_login"))
	{
		//log_printf("aAutologging in.");
		//update_windows();
		cMSN_Login();
		update_windows();
	}

	while (!quit)
	{
		setup_fds();
		res = select(MSNshiz.maxfds+1,&MSNshiz.fds, NULL, NULL, NULL);
		if (res == -1)
		{
			if ( errno != EINTR ) // SIG_ALARM causes this
				log_println("eError: select() failed.");
		}
		else
		{
			if (FD_ISSET(0, &MSNshiz.fds))
				input_process();
			else
			{
				cur = MSNshiz.conn.cnx;

				conn = 0;
				
				for (;cur;cur=cur->next)
				{
					conn = (msn_sess_conn_t *)cur->data;

					if (FD_ISSET(conn->fd, &MSNshiz.fds))
					{
						err_printf("FD_ISSET: %d\n",conn->fd);
						msn_callback_handler(conn->conn);
					}
				}
				
			}
		}
		
		redraw_status();
		update_windows();
	}
	write_contacts_file();
	write_config();
	screen_end();

	if (MSNshiz.errfile != stderr)
		fclose(MSNshiz.errfile);

	return 0;
}
