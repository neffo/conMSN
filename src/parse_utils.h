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

#ifndef __PARSE_UTILS__
#define __PARSE_UTILS__

#include "libmsn.h"

/*
** Name:    ParseHostPort
** Purpose: This function parses out the host and port for a switchboard
**          server
** Input:   address - host and port
**          host    - host
**          port    - port number
** Output:  0 on success, -1 on failure
*/

int ParseHostPort(char *address, char **host, int *port);

/*
** Name:    ParseMimeHeaders
** Purpose: This function returns the mime parts out of the message
** Input:   message - the whole message
**          mimeInfo    - mime info
**          im          - instant message type
** Output:  0 on success, -1 on failure
*/

int ParseMimeHeaders(char *message, char **mimeInfo, char **im);

/*
** Name:    DecodeMime
** Purpose: decode a mime encoded message. the original string
**          IS MODIFIED
** Input:   message
** Output:  decoded message
*/
void DecodeMime(char *message);

/*
** Name:    ParseArguments
** Purpose: This function parses a line of input, stores the arguments in an
**          array, and returns the count
** Input:   string    -  string to be parsed
**          delimiter - string delimiter
**          args      - argument array
**          numOfArgs - number of arguments
** Ouput:   0 if arguments are parsed, -1 if not
*/

int ParseArguments(char *string, char *delimiter, char ***args, int *numOfArgs);

/*
** Name:    DestroyArguments
** Purpose: This function destroys a set of arguments
** Input:   args      - arguments to be destroyed
**          numOfArgs - number of arguments 
** Output:  0 on success, -1 on failure
*/

int DestroyArguments(char ***args, int numOfArgs);

/*
** Name:    AddHotmail
** Purpose: This function adds @hotmail.com to a username that needs it
**          If @hotmail.com is already present, string is not modified
** Input:   src - string to look at
**          dst - destination string
** Output:  0 on success, -1 on failure
*/

int AddHotmail(const char *src, char **dst);

/*
** Name:    RemoveHotmail
** Purpose: This function removes @hotmail.com from any string
**          If @hotmail.com is already missing, string is not modifed
** Input:   src - string to look at
**          dst - destination string
** Output:  0 on success, -1 on failure
*/

int RemoveHotmail(const char *src, char **dst);

#endif
