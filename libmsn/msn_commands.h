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
#ifndef __LIB_MSNCOMMANDS__
#define __LIB_MSNCOMMANDS__

#include "libmsn.h"

/*
** Name:    ParseForCommand
** Purpose: This function parses the line of input from a message sent to
**          the client
** Input:   msn_conn   - a MSN connection structure
** Output:  none
*/

void ParseForCommand(MSN_Conn *msn_connection);

/*
** Name:    ConnectToServer
** Purpose: This function connects a socket to the server
** Input:   conn   - connection structure 
**          host   - hostname of dispatch server
**          port   - port number of dispatch server
** Output:  0 on success, -1 on failure
*/

int ConnectToServer(MSN_Conn *conn, char *host, int port);

/*
** Name:    SetProtocol
** Purpose: This function sets up the appropiate protocol, right now MSNP2
** Input:   conn       - connection structure
**          protocol   - protocol to use
** Output:  0 on success, -1 on failure
*/

int SetProtocol(MSN_Conn *conn, char *protocol);

/*
** Name:    GetServerPolicyInfo
** Purpose: This function queries the server for the policy.  Initially, this
**          is just the authentication package
** Input:   conn       - connection structure
**          sp         - security policy returned
** Output:  0 on success, -1 on failure
*/

int GetServerPolicyInfo(MSN_Conn *conn, char *sp);

/*
** Name:    AuthenticateUserMD5
** Purpose: This function authenticates a user and completes the login process
**          MD5 hashing is used
** Input:   conn       - connection structure
**          handle     - user's handle
**          passwd     - user's password
** Output:  0 on success, -1 on failure
*/

int AuthenticateUserMD5(MSN_Conn *conn, char *handle, char *passwd);

/*
** Name:    ChangeState
** Purpose: This function changes the state of the user
** Input:   conn       - connection structure
**          state      - new state
** Output:  0 on success, -1 on failure
*/

int ChangeState(MSN_Conn *conn, char *state);

/*
** Name:    Synchronize
** Purpose: This function syncronizes the server and the client for
**          contact information.  Does nothing right now. :)
** Input:   conn       - connection structure
** Output:  0 on success, -1 on failure
*/

int Synchronize(MSN_Conn *conn);

/*
** Name:    HandleRing
** Purpose: This function accepts a ring and establishes the connection
**          to the switchboard server
** Input:   conn       - connection structure
**          args       - rest of RNG command to be parsed and dealt with
**          numOfArgs  - number of arguments
** Output:  0 on success, -1 on failure
*/

int HandleRing(MSN_Conn *conn, char **args, int numOfArgs);

/*
** Name:    SendMessage
** Purpose: This function sends an instant message to th switchboard
**          server connection
** Input:   conn       - connection structure
**          message    - message
** Output:  0 on success, -1 on failure
*/

int SendMessage(MSN_Conn *conn, char *message);

/*
** Name:    HandleMessage
** Purpose: This function handles an instant message from either the server
**          or another client
** Input:   conn   - connection structure
**          args   - list of arguments
**          numOfargs - number of arguments
** Output:  0 on success, -1 on failure
*/

int HandleMessage(MSN_Conn *conn, char **args, int numOfArgs);

/*
** Name:    HandleAcknowledge
** Purpose: This function simply handles an ackknowledgement from the server
**          Right now this will prolly just do a read and ignore
** Input:   conn     - connection structure
** Output:  0 on success, -1 on failure
*/

int HandleAcknowledge(MSN_Conn *conn);

/*
** Name:    SendBYE
** Purpose: This function sends a BYE command to any chat connection
** Input:   conn - MSN connection structure
** Output:  0 on success, -1 on failure
*/

int SendBYE(MSN_Conn *conn);

/*
** Name:    SendSBYE
** Purpose: This function sends a BYE command to a switchboard connection
**          and propely disposes of the connection
** Input:   conn - MSN connection structure
** Output:  0 on success, -1 on failure
*/

int SendSBYE(MSN_Conn *conn);

/*
** Name:    HandleBye
** Purpose: This function simply handles the BYE command from when a user
**          ends a switchboard session.  Could end a connection to the server
**          too
** Input:   conn      - connection structure
**          args      - arguments
**          numOfArgs - number of Arguments
** Output:  0 on success, -1 on failure
*/

int HandleBye(MSN_Conn *conn, char **args, int numOfArgs);

/*
** Name:    HandleOUT
** Purpose: This function handles any OUT call from the server. Clears
**          all the information too
** Input:   conn      - connection structure
**          args      - arguments
**          numOfArgs - number of Arguments
** Output:  0 on success, -1 on failure
*/

int HandleOUT(MSN_Conn *conn, char **args, int numOfArgs);

/*
** Name:    HandleILN
** Purpose: This function handles the initial online status of people
** Input:   conn      - connection structure
**          args      - arguments
**          numOfArgs - number of arguments
** Output:  0 on success, -1 on failure
*/

int HandleILN(MSN_Conn *conn, char **args, int numOfArgs);

/*
** Name:    HandleFLN
** Purpose: This function handles when users come offline
** Input:   conn      - connection structure
**          args      - arguments
**          numOfArgs - number of arguments
** Output:  0 on success, -1 on failure
*/

int HandleFLN(MSN_Conn *conn, char **args, int numOfArgs);

/*
** Name:    HandleNLN
** Purpose: This function handles when users come online 
** Input:   conn      - connection structure
**          args      - arguments
**          numOfArgs - number of arguments
** Output:  0 on success, -1 on failure
*/

int HandleNLN(MSN_Conn *conn, char **args, int numOfArgs);

/*
** Name:    HandleADD
** Purpose: This function handles the ADD commands from the server that can
**          happen asyncronously
** Input:   conn      - connection structure
**          args      - arguments
**          numOfArgs - number of arguments
** Output:  0 on success, -1 on failure
*/

int HandleAdd(MSN_Conn *conn, char **args, int numOfArgs);

/*
** Name:    HandleRemove
** Purpose: This function handles the REM commands from the server that can
**          happen asyncronously
** Input:   conn      - connection structure
**          args      - arguments
**          numOfArgs - number of argyments
** Output:  0 on success, -1 on failure
*/

int HandleRemove(MSN_Conn *conn, char **args, int numOfArgs);

/*
** Name:    HandleLST
** Purpose: This function handles the LST command, which deals with user
**          lists
** Input:   conn - MSN connection structure
**          args      - arguments
**          numOfArgs - number of arguments
** Output:  0 on success, -1 on failure
*/

int HandleLST(MSN_Conn *conn, char **args, int numOfArgs);

/*
** Name:    HandleXFR
** Purpose: This function handles an XFR event, which transfers to a new server
** Input:   conn      - connection structure
**          args      - arguments
**          numOfArgs - number of arguments
**          listener  - whether or not to set up a listener
** Output:  0 on success, -1 on failure
*/

int HandleXFR(MSN_Conn *conn, char **args, int numOfArgs, int listener);

/*
** Name:    AddContact
** Purpose: This function adds a contact to the forward list
** Input:   conn - MSN connection structure
**          handle - handle of person to use
** Output:  0 on success, -1 on failure
*/

int AddContact(MSN_Conn *conn, char *handle);

/*
** Name:    AuthorizeContact
** Purpose: This function authorizes a contact to be on the allow list
** Input:   conn - MSN connection structure
**          handle - handle of the person to authorize
** Output:  0 on succuess, -1 in failure
*/

int AuthorizeContact(MSN_Conn *conn, char *handle);



/*
** Name:    RemoveContact
** Purpose: This function removes a contact to the forward list
** Input:   conn - MSN connection structure
**          handle - handle of person to use
** Output:  0 on success, -1 on failure
*/

int RemoveContact(MSN_Conn *conn, char *handle);

/*
** Name:    Logout
** Purpose: This function logs out from the server
** Input:   conn - MSN connection structure
** Output:  0 on success, -1 on failure
*/

int Logout(MSN_Conn *conn);

/*
** Name:    RequestSwitchboardSession
** Purpose: This function requests a connection to a Switchboard server for
**          the intent of establishing an instant message session
** Input:   conn - MSN connection structure
**          handle - handle of person
** Output:  0 on success, -1 on failure
*/

int RequestSwitchboardSession(MSN_Conn *conn, char *handle);

/*
** Name:    FindMSNConnectionByHandle
** Purpose: This function searches the list of connections and made and looks
**          for the first connection containing a handle
** Input:   handle - handle of the user having a message sent to
** Output:  0 on success, -1 on failure
*/

MSN_Conn *FindMSNConnectionByHandle(char *handle);

/*
** Name:    KillConnection
** Purpose: This function destroys a connection that has been unceremoniously
**          bet cut
** Input:   conn - MSN connection structure
** Output:  0 on success, -1 by failure
*/

int KillConnection(MSN_Conn *conn);
 
#endif
