#include "msn_shiz.h"

#define EXT_MSG "xmsg"
#define EXT_ONL "xstat"
#define EXT_MAIL "xmail"

void first_run (); // test if msn has be run before

void file_init();
void file_end();

void write_contacts();
void read_contacts();

void write_config();
void read_config();

void write_macros();
void read_macros();

void log_event ( char *alias, char *message, int day, int month, int hour, int min );

int call_ext ( char *cmd, char *user, char *data );
void ext_msg (char *user, char *msg );
void ext_onl (char *user, char *status);
void ext_mail (char *mail, char *mail2);

