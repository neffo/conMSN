#ifndef DISPLAY_H
#define DISPLAY_H
#include <ncurses.h>
/*
 * ***********************************
 */

#define CONT_X 15 // width of contacts window
//#define SCROLLBACK 200
#define SCROLLBACK cvar_to_int ("msn_scrollback")

#define MAX_LOG_WIDTH 120
/*
typedef struct display_s
{
	struct dispwin_s wmain;
	struct dispwin_s wcontacts;
	struct dispwin_s wcmd;
	int scrw;
	int scrh;
} display_t;*/

#define ST_MSG 'm'
#define ST_ERR 'e'
#define ST_LINE '-'
#define ST_MAIL 'l'
#define ST_BLANK 0
#define ST_ATTN 'a'
#define ST_NORM ' '
#define ST_BLUE 'b'

#define SPECIAL_CHARS "me-abl"

typedef struct dispwin_s
{
	WINDOW *win; 
	int winh; // height
	int winw; // width
	int sx; // top left corner
	int sy; // top left corner
	int changed; // window must be refreshed
} dispwin_t;

typedef struct display_s
{
	struct dispwin_s wmain;
	struct dispwin_s wcontacts;
	struct dispwin_s wcmd;
	int scrw;
	int scrh;
	char **log;
	int logpos;
	int loglines;
} cdisplay_t;

void screen_setup (/* cdisplay_t *display*/ );
void screen_end (/* cdisplay_t *display*/ );
void draw_windows (/* cdisplay_t *display*/ );
void scroll_up (/* cdisplay_t *display*/); // physically move data
void scroll_down (/* cdisplay_t *display*/); // ditto
void log_scroll_up(); // scroll visible display up
void log_scroll_down(); // scroll visible display down
void clear_scrollback();
void redraw_cmd_line();
void redraw_main();
void redraw_status ();
void log_println(char *string);
void log_printf(char *format, ...);
void append_ln ( char *string );
void draw_scrollbar(int sy,int sx, int by1, int by2, int by3, int by4);

int conn_count ();
/*
#ifndef MAIN_C
#ifndef EXTERN_DISPLAY
#define EXTERN_DISPLAY
//extern cdisplay_t display;
#endif
#endif
*/
#endif
