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
#ifndef __CHAT_LISTS__
#define __CHAT_LISTS__

#include "libmsn.h"

/*
** Name:    AddUserToChatList
** Purpose: This function adds a user to the current chat list
** Input:   chatSession   - chat list structure
**          userHandle    - user's handle
**          userFriendlyHandle - user's friendly handle
**          index         - index of user
**          userState     - user state
** Output:  0 on success, -1 on failure
*/

int AddUserToChatList(ChatSession *chatSession, char *userHandle, 
                      char *userFriendlyHandle, int index, 
                      UserState userState);

/*
** Name:    RemoveUserFromChatList
** Purpose: This function removes a user from the list
** Input:   chatSession    - chat list structure
**          userHandle     - user handle
** Output:  0 on success, -1 on failure
*/

int RemoveUserFromChatList(ChatSession *chatSession, char *userHandle);

/*
** Name:    SearchForUser
** Purpose: This function searches a chat list and returns whether the 
**          user is indeed, on a list
** Input:   chatSession - chat list structure
**          handle      - handle to search for
** Output:  0 if found, -1 if not
*/

int SearchForUser(ChatSession chatSession, char *handle);

/*
** Name:    ChangeUserState
** Purpose: This function changes a users state
** Input:   chatSession - chat list structure
**          userHandle  - user handle
**          newState    - new state of user
** Output:  0 on success, -1 on failure
*/

int ChangeUserState(ChatSession *chatSession, char *userHandle, 
                    UserState userState);

/*
** Name:    CompareUserName
** Purpose: This function is used to compare nodes in the list
** Input:   a - node
**          b - name
** Output:  0 if found, something else
*/

int CompareUserName(const void * a, const void *b);

/*
** Name:    DestroyChatList
** Purpose: This function destroys the list of chat users
** Input:   chatSession - chat list structure
** Output:  0 on success, -1 on failure
*/

int DestroyChatList(ChatSession chatSession);
#endif
