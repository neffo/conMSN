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

//#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include "libmsn.h"
#include "parse_utils.h"
#include "msn_commands.h"
#include "msn_shiz.h"

/*
 * String representations of commands codes
 */

char CommandString[NUM_OF_COMMANDS][COMMAND_LENGTH] = {
    "ACK", "ADD", "ANS", "BLP", "BYE", "CAL", "CHG", "FLN", "GTC", "INF",
    "ILN", "IRO", "JOI", "LST", "MSG", "NAK", "NLN", "OUT", "REM", "RNG",
    "SYN", "USR", "VER", "XFR"
};

/*
** State strings
*/

char StateStrings[NUM_OF_STATES][4] = {
    "NLN", "FLN", "HDN", "BSY", "IDL", "BRB", "AWY", "PHN", "LUN"
};

/*
** Name:    msn_callback_handler
** Purpose: Passes on the call back information to the parsing function
** Input:   data - data passed to handler ( is actually a MSNconn struct )
** Output:  none
*/

void msn_callback_handler(void * data)
{
    ParseForCommand(data);
}

/*
** Main MSN Connection
*/

//extern MSN_Conn mainMSNConnection;
extern msn_shiz_t MSNshiz;

/*
** Name:    MSN_RegisterErrorOutput
** Purpose: This function sets the error output function
** Input:   func - function pointer
** Output:  none
*/

void MSN_RegisterErrorOutput(ERR_CALLBACK func)
{
    MSN_ErrorOut = func;
}

/*
** Name:    MSN_RegisterCallback
** Purpose: This function regsiters the call backs for the proper event
** Input:   eventType  - type of server event
**          func       - function pointer
** Output:  none
*/

void MSN_RegisterCallback(int eventType, MSN_CALLBACK func)
{
    msn_event[eventType] = func;
}

/*
** Name:    InitializeMSNConnection
** Purpose: This function initializes a MSN_Conn structure to default values
** Input:   conn - msn connection structure
** Output:  0 on success, -1 on failure
*/

int InitializeMSNConnection(MSN_Conn *conn)
{
    conn->serverType = DISPATCH_CONN;
    conn->chatUsers.users = NULL;
    conn->chatUsers.numOfUsers = 0;
    conn->flUsers.users = NULL;
    conn->flUsers.numOfUsers = 0;
    conn->alUsers.users = NULL;
    conn->alUsers.numOfUsers = 0;
    conn->blUsers.users = NULL;
    conn->blUsers.numOfUsers = 0;
    conn->rlUsers.users = NULL;
    conn->rlUsers.numOfUsers = 0;
    conn->cookie[0] = '\0';
    conn->commonName[0] = '\0';
    //conn->listenerID = -1;
    conn->unreadMail = 0;
    return 0;
}

/*
** Name:    MSN_Login
** Purpose: This function encapsulates the login process to MSN
** Input:   handle   - user's handle
**          passwd   - user's password
**          host     - notification server to use
**          port     - port number of notifcation server
** Output:  0 on success, -1 on failure
*/

int MSN_Login(char *handle, char *passwd, char *host, int port)
{
    int  lID;
    char sp[10];
    int retry = 0;

	char test[30];

    MSN_Conn *mainconn;

    mainconn = (MSN_Conn *)malloc(sizeof(MSN_Conn));

    MSNshiz.conn.mainconn = mainconn;

    InitializeMSNConnection(mainconn);

    strcpy(mainconn->passwd, passwd);
    strcpy(mainconn->handle, handle);
#ifdef DEBUG
//snprintf(test,30,"MSNshiz.conn.cnx = %d |",&MSNshiz.conn.cnx);
//    MSN_ErrorOut(test,"debug");
#endif //DEBUG

    err_printf("MSNshiz.conn.cnx = %d\n",&MSNshiz.conn.cnx);
    
LOGIN_RETRY:
    retry++;

    if (ConnectToServer(mainconn, host, port) != 0) {
//        MSN_ErrorOut("Couldn't connect to server", "Server Error");
		err_printf("Unable to connect to server.\n");
        return -1;
    }
    if (SetProtocol(mainconn, DEFAULT_PROTOCOL) != 0) {
	if(retry <= 20)
	{
		goto LOGIN_RETRY;
	}
	else
	{
        	MSN_ErrorOut("Couldn't set protocol", "Protocol Error");
		return -1;
	}
	
    }
    if (GetServerPolicyInfo(mainconn, sp) != 0) {
        MSN_ErrorOut("Couldn't retrieve security policy", "Policy Error");
        return -1;
    }

    if (AuthenticateUserMD5(mainconn, handle, passwd) != 0) {
        MSN_ErrorOut("Unable to authenticate user", "Authentication Error");
        return -1;
    }
   
    Synchronize(mainconn);
    ChangeState(mainconn, "NLN");

    m_input_add(mainconn->fd, msn_callback_handler, mainconn);
    //mainMSNConnection.listenerID = lID;
    return 0;
}

/*
** Name:    MSN_SendMessage
** Purpose: This function encapuslates the sending of an instant message
** Input:   handle - user's handle who is receiving the message
**          message - the actual message to send
** Output:  0 on success, -1 on failure
*/

int MSN_SendMessage(char *handle, char *message)
{
    char     *newHandle;
    MSN_Conn *conn;

    AddHotmail(handle, &newHandle);
    conn = FindMSNConnectionByHandle(newHandle);
    if (conn == NULL) {
    /* if a connection isn't found try to establish a new connection */
        if (RequestSwitchboardSession(MSNshiz.conn.mainconn, newHandle) == -1) {
            free(newHandle);
            return -1;
        }
        conn = FindMSNConnectionByHandle(newHandle);
        if (conn == NULL) {
            free(newHandle);
	    err_printf("MSN_SendMessage: unable to create new connection.\n");
            return -1;
        }
    } 
    SendMessage(conn, message);
    free(newHandle);
    return 0;

}

/*
** Name:    MSN_AddContact
** Purpose: This function sends a add to forward contact list message to the
**          server
** Input:   handle  - handle to add to the list
** Output:  0 on success, -1 on failure
*/

int MSN_AddContact(char *handle)
{
    if (handle == NULL)
        return -1;

    AddContact(MSNshiz.conn.mainconn, handle);
    return 0;    
}

/*
** Name:    MSN_RemoveContact
** Purpose: This function sends a remove to forward contact list message to
**          the server
** Input:   handle - handle to remove from the list
** Output:  0 on success, -1 on failure
*/

int MSN_RemoveContact(char *handle)
{
    if (handle == NULL)
        return -1;

    RemoveContact(MSNshiz.conn.mainconn, handle);
    return 0;
}

/*
** Name:    MSN_AuthorizeContact
** Purpose: This function sends an authorize message to the server
** Input:   conn   - MSN connection structure
**          handle - handle to authorize
** Output:  0 on success, -1 on failure
*/

int MSN_AuthorizeContact(MSN_Conn *conn, char *handle)
{
    return AuthorizeContact(conn, handle);
}

/*
** Name:    MSN_EndChat
** Purpose: This function sends an OUT mesage to the server to end a 
**          chat with a user
** Input:   handle - handle to end chat with 
** Output:  0 on success, -1 on failure
*/

int MSN_EndChat(char *handle)
{
    MSN_Conn *conn;

    conn = FindMSNConnectionByHandle(handle);
    if (conn == NULL)
        return -1;
    return SendSBYE(conn);
}

/*
** Name:    MSN_Logout
** Purpose: This function properly logs out of the MSN service
** Input:   none
** Output:  0 on success, -1 on failure
*/

int MSN_Logout(void)
{
    return SendBYE(MSNshiz.conn.mainconn);
}

/*
** Name:    MSN_ChangeState
** Purpose: This function changes the current state of the user
** Input:   state - integer representation of the state
** Output:  0 on success, -1 on failure
*/

int MSN_ChangeState(int state)
{
    if (state > (NUM_OF_STATES-1))
        return -1;

    return ChangeState(MSNshiz.conn.mainconn, StateStrings[state]);
}

