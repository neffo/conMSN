#include <curses.h>
#include "display.h"
#include <stdarg.h>
#include <math.h>
#include "input.h"
#include "msn_shiz.h"

extern cdisplay_t display;
extern input_t input;
extern msn_shiz_t MSNshiz;

void screen_setup ( /*cdisplay_t *display*/ )
{
	int i;

#ifdef DYNAMIC_SCROLLBACK
	display.log = malloc(SCROLLBACK * sizeof(char **));

	for (i=0;i<SCROLLBACK;i++)
	{
		display.log[i] = malloc( sizeof ( char *) * MAX_LOG_WIDTH );
	}
#endif

	initscr();
	cbreak();
	noecho();
	nodelay(stdscr,TRUE);
	keypad(stdscr, TRUE);
//	leaveok(stdscr, TRUE);
//	scrollok(stdscr,TRUE);
	start_color();

	init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
	init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
	init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
	init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
	init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(COLOR_BLUE, COLOR_WHITE, COLOR_BLUE);
	init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
	init_pair(COLOR_BLACK, COLOR_BLUE, COLOR_BLACK);
	//init_pair(COLOR_YELLOW_BLUE, COLOR_YELLOW, COLOR_BLUE);
	//init_pair(COLOR_WHITE_BLUE, COLOR_WHITE, COLOR_BLUE);
	//init_pair(COLOR_CYAN_BLUE, COLOR_CYAN, COLOR_BLUE);
	
	getmaxyx(stdscr, display.scrh, display.scrw);
	display.scrw--;

	display.wmain.winh = display.scrh - 5;
	display.wmain.winw = display.scrw - CONT_X - 2;
	display.wmain.sx = 1;
	display.wmain.sy = 1;
	display.wmain.changed = 0;
/*
	move(display.wmain.sy+display.wmain.winh,display.wmain.sx+display.wmain.winw);
	addch(ACS_STERLING);*/
	
	display.wcontacts.winh = display.scrh - 5;
	display.wcontacts.winw = CONT_X-1;
	display.wcontacts.sx = (display.scrw - CONT_X) + 1;
	display.wcontacts.sy = 1;
	display.wcontacts.changed = 0;

	/*
	move(display.wcontacts.sy,display.wcontacts.sx);
	addch(ACS_STERLING);*/

	display.wcmd.winh = 1;
	display.wcmd.winw = display.scrw - 2;
	display.wcmd.sx = 1;
	display.wcmd.sy = display.scrh - 2;
	display.wcmd.changed = 0;

	/*
	move(display.wcmd.sy,display.wcmd.sx);
	addch(ACS_STERLING);*/

	clear_scrollback();
	
	display.logpos=SCROLLBACK; //last line displayed
	display.loglines=0;

}

void screen_end (/* cdisplay_t *display*/ )
{
	endwin();
	printf("Exiting...\n");

	/*
	printf("SX: %d SY: %d\n\n",display.scrw, display.scrh);
	printf("MX: %d MY: %d\n",display.wmain.sx,display.wmain.sy);
	printf("DX: %d DY: %d\n",display.wcontacts.sx,display.wcontacts.sy);
	*/
}

/*void create_windows (  cdisplay_t *display )
{
	
}*/

//#define NEW_STYLE

void draw_windows (/* cdisplay_t *display*/ )
{
#ifndef NEW_STYLE
	box(stdscr,0,0);
	move(0,display.scrw-CONT_X);
	addch(ACS_TTEE);
	move(1,display.scrw-CONT_X);
	vline(ACS_VLINE,display.wcontacts.winh+1);
	move(display.wcmd.sy-1,display.wcmd.sx);
	hline(ACS_HLINE,display.wcmd.winw+1);
	move(display.wcmd.sy-1,0);
	addch(ACS_LTEE);
	move(display.wcmd.sy-1,display.wmain.winw+2);
	addch(ACS_BTEE);
	move(display.wcmd.sy-1,display.wcmd.winw+2);
	addch(ACS_RTEE);
	refresh();
#else
	move(0,0);
	addch(165);
	move(0,1);
//	hline(
#endif
}

void scroll_up (/* cdisplay_t *display*/ )
{
	int i;
	for(i=0;i<SCROLLBACK-1;i++)
	{
		//if (*display.log[i+1])
			strcpy(display.log[i],display.log[i+1]);
	}
	display.log[i][0]=0;
	display.loglines++;
	if ( display.loglines > SCROLLBACK)
		display.loglines = SCROLLBACK;
}

void scroll_down ( /*cdisplay_t *display*/ )
{
	// ditto
}

void clear_scrollback (/* cdisplay_t *display*/ )
{
	int i;

	for (i=0;i<SCROLLBACK;i++)
	{
		display.log[i][0]=0;
	}
}

void update_windows ()
{
	if ( display.wmain.changed + display.wcontacts.changed + display.wcmd.changed )
	{
		if ( input.in_mode == IN_CMD)
			move(display.wcmd.sy,display.wcmd.sx+input.curspos+1);
		else 
			move(display.wcmd.sy,display.wcmd.sx+strlen(input.prompt)+input.input_size+1);
		refresh();
	}

	display.wmain.changed = display.wcontacts.changed = display.wcmd.changed=0;
}

void redraw_cmd_line()
{
	int shift;
	if (input.in_mode == IN_CMD)
	{
		shift = 0;
		if (input.len > display.wcmd.winw - 3)
			shift = input.len - display.wcmd.winw + 1;
		
		move(display.wcmd.sy,display.wcmd.sx);
		hline(' ',display.wcmd.winw);
		addstr(">");
		move(display.wcmd.sy,display.wcmd.sx+1);
		if (input.len)
		{
			if (!input.input_private)
				addnstr(input.cmd+shift,input.len-shift);
			else
				hline('*',input.len-shift);
		}
		
		move(display.wcmd.sy,display.wcmd.sx+input.curspos+1-shift);
		wredrawln(stdscr,display.wcmd.sy,1);
		display.wcmd.changed = 1;
	}
	else
	{
		shift = 0;
		move(display.wcmd.sy,display.wcmd.sx);
		hline(' ',display.wcmd.winw); // FIXME: find out why line isn't
		// cleared if its too long (works if i use a '-' character)
		// fixed by wredrawln() below
		
		addstr(input.prompt);
		move(display.wcmd.sy,display.wcmd.sx+1+strlen(input.prompt));
		
		if (input.input_size > display.wcmd.winw -3 - strlen(input.prompt) )
			shift = input.input_size - display.wcmd.winw +1 +strlen(input.prompt);
		
		if (input.input_size)
		{
			if (!input.input_private)
				addnstr(input.cmd+shift,input.input_size-shift);
			else
				hline('*',input.input_size-shift);
		}

		wredrawln(stdscr,display.wcmd.sy,1);
		display.wcmd.changed = 1;
	}
}

void redraw_main()
{

	// FIXME: add scroll bar for scrollback history
	int i=0;
	int first;
	int last;
	int cur;

	double size;
	double offset;
	int h;
	int by1,by2; // values of top segment of the scrollbar by1 should be 0
	int my1,my2; // values of middle section (the actual moving bit)
	int by3,by4; // values of space at the bottom (by4 should = winh)
	//float scale;

	char *line;

	move(0,0+(display.wmain.winw/2)-4);
	color_set(COLOR_BLUE,0);
	addstr("[conMSN]");
	
	color_set(COLOR_WHITE,0);
	for(i=0;i<(display.wmain.winh+1);i++)
	{
		move(i+1,1);
		hline(' ',display.wmain.winw);
		cur = display.logpos-(display.wmain.winh+1-i);
		line = display.log[cur];
		
		/*
		if (strncmp(display.log[cur],"ERROR:",6)==0)
			color_set(COLOR_RED,0);
		else if (strchr(display.log[cur],':'))
			color_set(COLOR_GREEN,0);
		else
			color_set(COLOR_WHITE,0);
		*/

		switch (*line){
		case ST_MSG:
			color_set(COLOR_GREEN,0);
			line++;
			break;
		case ST_ERR:
			color_set(COLOR_RED,0);
			line++;
			break;
		case ST_MAIL:
			color_set(COLOR_CYAN,0);
			line++;
			break;
		case ST_ATTN:
			color_set(COLOR_YELLOW,0); // white on blue
			line++;
			break;
		default:
			color_set(COLOR_WHITE,0);
		}

		if ( *line == '-')
		{
			hline(ACS_HLINE,display.wmain.winw);
			line++;
			//color_set(COLOR_BLUE,0);
		}
		// added ability to have text in horizontal line
		if ( *line != '!')
		{
			//color_set(COLOR_BLUE,0);
			addnstr(line,display.wmain.winw);
			//color_set(COLOR_WHITE,0);
		}
	}
// FIXME: what the FUCK IS WRONG WITH THIS CODE?!@#!@#
//	scale = ((float)display.loglines/(float)SCROLLBACK) * (float) display.wmain.winh;
	h = display.wmain.winh + 1;
	size =( ((float)h/(float)display.loglines) * (float) h);
//	size =(float) ( ( display.wmain.winh/ display.loglines) * display.wmain.winh);
	offset =( ( ((float)SCROLLBACK-(float)display.logpos)/(float)display.loglines ) * (float)h);

	//size += 1;

	// FIXME: we need to properly round these values (size, offset)
	
	size = (int) size;
	
	if (size > h)
		size = (float) h;

	offset = (int) offset;


	// hack to get scroll bar to hit the top
	if (display.logpos == (SCROLLBACK - (display.loglines - display.wmain.winh) + 1))
		offset = h-size;
	
//	err_printf("offset = %6.3f size = %6.3f (%6.3f)\nlines = %d height = %d logpos = %d\n",offset,size,size,display.loglines,display.wmain.winh,display.logpos);

	//size +=1;
	by1 = 0;
	by4 = h;
	by3 = by4-offset;
	by2 = by4-size-offset;

//	if (by2>by1)
//		by2-=1;

//	if (by3<by4)
//		by3+=1;

//	my1=by2+1;
//	my2=by3-1;

	standend();
	
	draw_scrollbar(display.wmain.sy,display.wcontacts.sx-1,by1,by2,by3,by4);


	// this fixes errors when using the gnome-terminal
	// (defaults to 'n' because it wastes bandwidth redrawing each line)
	if (cvar_true("msn_correct_graphics"))
		wredrawln(stdscr,display.wmain.sy,display.wmain.winh);
	
	display.wmain.changed = 1;
	standend();
}

//void draw_scrollbar_alt (int sy, int sx, int h, int visible, int firstvis, int total);

void draw_scrollbar(int sy,int sx, int by1, int by2, int by3, int by4)
{

	// FIXME: add colours too
	int x,y;
	int len;

	x = sx;
	y = sy;
	len = by2;

	//color_set(COLOR_BLACK,0);
	
	if (len > 0)
	{	
		mvvline(y,x,ACS_VLINE,len);
//		err_printf("x = %d y = %d len = %d\n",x,y,len);
		y+=len;
	}

	//y++;
//	move(y,x);
//	color_set(COLOR_WHITE,0); 
	len = by3-by2;
	mvvline(y,x,ACS_CKBOARD,len);
//	err_printf("x = %d y = %d len = %d\n",x,y,len);
	y+=len;

	len = by4-by3;
	if (len>0)
	{
//		move(y,x);
//		color_set(COLOR_BLACK,0); 
		mvvline(y,x,ACS_VLINE,len);
//		err_printf("x = %d y = %d len = %d\n",x,y,len);
	}

	standout();

//	err_printf("draw_scrollbar: (%d,%d) %d %d %d %d\n",sy,sx,by1,by2,by3,by4);
		
}

#define MAXVISCONTS 15
#define BOTVIS 3

void redraw_status ()
{
	// FIXME: make the contact list scroll (DONE)

	// FIXME: add scroll bar for contact list
	int x,y;
	char status[40];
	mlist cur;
	msn_contact_t *cont;
	int nc,nco,cc;

	int scp,skip,i;


	//scrollbar shiznits
	double size;
	double offset;
	int h;
	int by1,by2; // values of top segment of the scrollbar by1 should be 0
	int my1,my2; // values of middle section (the actual moving bit)
	int by3,by4; // values of space at the bottom (by4 should = winh)

	x = display.wcontacts.sx;
	y = display.wcontacts.sy;

	move(y-1,x+(display.wcontacts.winw/2)-5);
	color_set(COLOR_BLUE,0);
	addstr("[Contacts]");
	//y++;
	standend();

	nc = 0;
	nco = 0;
	scp = 0;
	skip= 0;
	cc = conn_count();
	

	i = 0;

	for (cur = MSNshiz.contacts; cur; cur=cur->next)
	{
		if (cur && input.contact == cur->data)
		{
			scp = i;
		}
		i++;
	}

	if (i < MAXVISCONTS-BOTVIS)
		skip = 0;
	else
		skip = scp - MAXVISCONTS+BOTVIS;

	if (skip + MAXVISCONTS  > i-1)
		skip = i - MAXVISCONTS -1 ;
	
	i = 0;
	
	for (cur = MSNshiz.contacts; cur; cur=cur->next)
	{
		cont = (msn_contact_t *) cur->data;
		if (i>skip && y < MAXVISCONTS+1 )
		{
			move(y,x);
			if (input.contact != cont)
				attroff(A_BOLD);
//				color_set(COLOR_CYAN,0);
			else 
				attron(A_BOLD);
//				color_set(COLOR_BLUE,0);
			snprintf(status,display.wcontacts.winw+1,"[%s]%s               ",msn_short_status[cont->status],cont->alias);
			addstr(status);
			y++;
		}

		nc++;

		if (cont->status != MSN_OFFLINE )
			nco++;

		i++;
	}
	
//	standout();
	attroff(A_BOLD);

	color_set(COLOR_WHITE,0);
	
	while (y < (display.wcontacts.sy + display.wcontacts.winh-3) )
	{
		move(y,x);
		hline(' ',display.wcontacts.winw);
		y++;
	}
	
	x = display.wcontacts.sx;
	y = display.wcontacts.sy + display.wcontacts.winh-3;

	move(y,x);
	color_set(COLOR_BLUE,0);
	snprintf(status,display.wcontacts.winw+1,"%s", 
			MSNshiz.conn.username);
	addstr(status);

	move(++y,x);
	snprintf(status,display.wcontacts.winw+1,"[L: %d/%d C: %d]",nco,nc,cc);
	addstr(status);
	move(++y,x);
	snprintf(status,display.wcontacts.winw+1,"[St: %s]",
			msn_long_status[MSNshiz.conn.status]);
	addstr(status);

	move(++y,x);
	
	snprintf(status,display.wcontacts.winw+1,"[Input :");
	if ( input.in_mode == IN_CMD )
		strcat(status," CMD ]");
	else if ( input.in_mode == IN_LINE )
		strcat (status," LINE]");
	else if ( input.in_mode == IN_CHAT )
		strcat (status, " CHAT]");

	addstr(status);
	
	standend();

	// draw_scrollbar
	
	h = MAXVISCONTS + 1;
	
	size =( ((float)h/(float)nc) * (float) h);
	
	offset =( ( ((float)skip)/(float)nc ) * (float)h);

	size = (int) size;

	// bleh
	
	display.wcontacts.changed = 1;
}

void log_println(char *string)
{
	int diff;
	int len;
	int count;

	char special;
	
	//char *ptr;

	len = strlen(string);
	
	count = 0;

	//ptr = string;

	if (strchr(SPECIAL_CHARS,*string))
	{
		special = *string;
		string++;
		len--;
	}
	else
		special = ' ';
	
loop_top:
	
	scroll_up();

	diff = display.wmain.winw;

	if (diff > strlen(string))
		diff = strlen(string);
	else
		while (diff && string[diff] != ' ')
			diff--;

	if (diff == 0)
		diff = display.wmain.winw;

	strncpy(display.log[SCROLLBACK-1]+1,string,diff);
	display.log[SCROLLBACK-1][0] = special;

	display.log[SCROLLBACK-1][diff+1] = 0;

	diff++; // remove that space we skipped
	
	string += diff;

	count += diff;

	//string++;

	if (count < len)
		goto loop_top;
		
	redraw_main();
	
//	display.wmain.changed = 1;
}

void append_ln ( char *string )
{
	// FIXME: add word wrapping
	char *line;
	int diff;

	line = display.log[SCROLLBACK-1];

	strcat(line,string);

	redraw_main();
}

void add_int(char *string, int *val)
{
}

void add_float(char *string, float *val)
{
}

void log_printf(char *format, ...)
{
	va_list argp;
	char out[1024];

	va_start(argp,format);
	
	vsnprintf(out,1024,format,argp);

	log_println(out);

	va_end(argp);
}
/*
void log_printf(char *format, ...)
{
	va_list argp;
	char out[1024];
	char *sout;
	int count;
	int integer;
	
	va_start(argp, format);

	out[0] = 0;
	sout = out;
	
	while (*format)
	{
		if (*format == '%')
			switch(*(++format))
			{
				case 'd':
					integer = va_arg(argp,int);
					add_int(out,&integer);
					break;
				case 's':
					strcat(out,va_arg(argp,char *));
					break;
				case 'c':
					*sout++ = (char) va_arg(argp,int);
					break;
			}
		else
		{
			*sout++ = *format;
		}
		format++;	
	}
	*sout = 0;

	log_println(out);
	
	va_end(argp);
}
*/

void log_scroll_up()
{
	int maxlogpos;
	
	display.logpos-=5;
	if (display.logpos < display.wmain.winh)
		display.logpos = display.wmain.winh;

	maxlogpos = SCROLLBACK - (display.loglines - display.wmain.winh) + 1;

	if (maxlogpos > SCROLLBACK)
		maxlogpos = SCROLLBACK;
	
	if ( display.logpos < maxlogpos )
		display.logpos = maxlogpos;

//	log_printf("maxlogpos = %d\n",maxlogpos);
	
	redraw_main();
}

void log_scroll_down()
{
	display.logpos+=5;
	if (display.logpos > SCROLLBACK)
		display.logpos = SCROLLBACK;
	redraw_main();
}

int conn_count ()
{
	mlist cur;
	int cc=0;

	for (cur=MSNshiz.conn.cnx;cur;cur=cur->next)
	{
		cc++;
	}

	return cc;
}
