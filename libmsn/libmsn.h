/*
 * libmsn
 *
 * Copyright (C) 1999, Shane P. Brady <shane@jxie.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef __LIB_MSN__
#define __LIB_MSN__

//#include <gdk/gdk.h>
#include "parse_utils.h"

#define DEFAULT_HOST "messenger.hotmail.com"
#define DEFAULT_PORT 1863
#define DEFAULT_PROTOCOL "MSNP2"

#define DISPATCH_CONN 0
#define SWITCHBOARD_CONN 1
#define COMMAND_LENGTH 4
#define LINE_LENGTH 513

/*
 * The following are the allowable states for a user to be in right now
 */

typedef enum USERSTATE {
    USER_NLN, USER_FLN, USER_HDN, USER_BSY, USER_IDL, USER_BRB, USER_AWY, 
    USER_PHN, USER_LUN, NUM_OF_STATES
} UserState;

typedef struct INSTANT_MESSAGE {
    int year;                    /* six fields used for time stamping */
    int month;                   
    int day;                 
    int hour;
    int minute;
    int sec;            
    char *msg;                   /* message itself             */
    char *sender;                /* sender of the message      */
    int fd;                      /* file descriptor of message */
} MSN_InstantMessage, *MSN_InstantMessage_PTR;

/*
** Status change structure
*/

typedef struct STATUS_CHANGE {
    UserState  newStatus;             /* new status value */
    char       *handle;               /* handle of user   */
} MSN_StatusChange, *MSN_StatusChange_PTR;

/*
** Email notification structure
*/

typedef struct MAIL_NOTIFICATION {
	char *from;		/* NULL if initial e-mail notification, only unread will have a value */
	char *subject;		/* The subject of the mail */
	char *destfolder; 	/* Hotmail folder where the mail is */
	char *fromaddr;		/* Full e-mail address */
	int unread;		/* Number of unread messages */
} MSN_MailNotification;

/*
** Callback definition
*/

typedef void (*MSN_CALLBACK)( void *);
typedef void *ERR_CALLBACK( char *, char *);

/*
** Type of server events
*/

typedef enum SERVER_EVENT {
    MSN_RNG, MSN_MSG, MSN_ILN, MSN_NLN, MSN_FLN, MSN_OUT, MSN_AUTH, MSN_MAIL,
    MSN_NUM_EVENTS
} ServerEvent;

/*
** An array of functions to store the callbacks 
*/

void (*msn_event[MSN_NUM_EVENTS])(void *data);

/*
** Error function to use
*/

extern ERR_CALLBACK *MSN_ErrorOut;

/*
** Really cheezy..but this will work
*/

#define MIME_HEADER "MIME-Version: 1.0\r\nContent-Type: text/plain; charset=UTF-8\r\nX-MMS-IM-Format: FN=MS%20Sans%20Serif; EF=; CO=0; CS=0; PF=0\r\n\r\n"

/*
** This apparently does some sort of control thing, but I don't know yet what
** for
*/

#define CONTROL_CONTENT "Content-Type: text/x-msmsgscontrol"

/*
** Name:    The following structureis hold chat group information by holding
**          all the users in the current switchboard session
*/

typedef struct USER
{
    char        userHandle[256];          /* user's handle        */
    char        userFriendlyHandle[256];  /* user's friendly name */
    int         index;                    /* index of user        */
    UserState   state;                    /* state of user        */
} ChatUser;

typedef struct CHATSESSION
{
    int      numOfUsers;                  /* number of users in the session */
    ChatUser users[30];                      /* user list                      */
} ChatSession;

/* 
** The following structure holds the connection information.
** I included the cookie and the common name, because both those things 
** come from the server.  Common name might be moved out later.
**                           - shane
*/

typedef struct MSNCONN {
    char   passwd[LINE_LENGTH+1];
    char   handle[LINE_LENGTH+1];
    int    fd;                /* file descriptor used */
    char   cookie[1024];      /* cookie used for switchboard */
    char   commonName[1024];  /* common name */
    int    serverType;        /* what server are we connected to */
    int	   unreadMail;		/* number of unread mails */
    //gint   listenerID;        /* id of what is listening */

    ChatSession chatUsers;

    // don't think these are currently used
    ChatSession flUsers;      /* "forward list" */
    ChatSession alUsers;      /* "allow list "  */
    ChatSession blUsers;      /* "blocked list" */
    ChatSession rlUsers;      /* "reverse list" */
} MSN_Conn;

/*
** Authorization message
*/

typedef struct AUTH_MSG {
    char     *handle;
    char     *requestor;
    MSN_Conn *conn;
} MSN_AuthMessage, *MSN_AuthMessage_PTR;
 

/*
 * The following are the command code.  For a better description of each
 * command, consult the protocol document at: http://www.xxx.com/xxx
 */

typedef enum COMMANDCODE {
    ACK, ADD, ANS, BLP, BYE, CAL, CHG, FLN, GTC, INF, ILN, IRO, JOI, LST,
    MSG, NAK, NLN, OUT, REM, RNG, SYN, USR, VER, XFR, NUM_OF_COMMANDS
} CommandCode;

/*
** Name:    msn_callback_handler
** Purpose: Passes on the call back information to the parsing function
** Input:   data - data passed to handler
** Output:  none
*/

void msn_callback_handler( void *data );

/*
** Name:    InitializeMSNConnection
** Purpose: This function initializes a MSN_Conn structure to default values
** Input:   conn - msn connection structure
** Output:  0 on success, -1 on failure
*/

int InitializeMSNConnection(MSN_Conn *conn);

/*
** Name:    MSN_RegisterErrorOutput
** Purpose: This function sets the error output function
** Input:   func - function pointer
** Output:  none
*/

void MSN_RegisterErrorOutput(ERR_CALLBACK func);

/*
** Name:    MSN_RegisterCallback
** Purpose: This function regsiters the call backs for the proper event
** Input:   eventType  - type of server event
**          func       - function pointer
** Output:  none
*/

void MSN_RegisterCallback(int eventType, MSN_CALLBACK func);

/*
** Name:    MSN_Login
** Purpose: This function encapsulates the login process to MSN
** Input:   handle   - user's handle
**          passwd   - user's password
**          host     - notification server to use
**          port     - port number of notifcation server
** Output:  0 on success, -1 on failure
*/

int MSN_Login(char *handle, char *passwd, char *host, int port);

/*
** Name:    MSN_SendMessage
** Purpose: This function encapuslates the sending of an instant message
** Input:   handle - user's handle who is receiving the message
**          message - the actual message to send
** Output:  0 on success, -1 on failure
*/

int MSN_SendMessage(char *handle, char *message);

/*
** Name:    MSN_AddContact
** Purpose: This function sends a add to forward contact list message to the
**          server
** Input:   handle  - handle to add to the list
** Output:  0 on success, -1 on failure
*/

int MSN_AddContact(char *handle);

/*
** Name:    MSN_RemoveContact
** Purpose: This function sends a remove to forward contact list message to
**          the server
** Input:   handle - handle to remove from the list
** Output:  0 on success, -1 on failure
*/

int MSN_RemoveContact(char *handle);

/*
** Name:    MSN_AuthorizeContact
** Purpose: This function sends an authorize message to the server
** Input:   conn   - MSN connection structure
**          handle - handle to authorize
** Output:  0 on success, -1 on failure
*/

int MSN_AuthorizeContact(MSN_Conn *conn, char *handle);

/*
** Name:    MSN_EndChat
** Purpose: This function sends an OUT mesage to the server to end a
**          chat with a user
** Input:   handle - handle to end chat with
** Output:  0 on success, -1 on failure
*/

int MSN_EndChat(char *handle);

/*
** Name:    MSN_Logout
** Purpose: This function properly logs out of the MSN service
** Input:   none
** Output:  0 on success, -1 on failure
*/

int MSN_Logout(void);

/*
** Name:    MSN_ChangeState
** Purpose: This function changes the current state of the user
** Input:   state - integer representation of the state
** Output:  0 on success, -1 on failure
*/

int MSN_ChangeState(int state);

#endif
