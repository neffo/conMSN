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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "chat_lists.h"
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
                      UserState userState)
{
    mlist node;
    ChatUser *cu;
    ChatUser *newUser;

    node = m_list_find_custom(chatSession->users, userHandle, CompareUserName);
    if (node != NULL) {
        cu = node->data;
        cu->state = userState;
    }
    
    newUser = (ChatUser *)malloc(sizeof(ChatUser));
    if (newUser == NULL)
        return -1;
    strcpy(newUser->userHandle, userHandle);
    strcpy(newUser->userFriendlyHandle, userFriendlyHandle);
    newUser->index = index;
    newUser->state = userState;
 
    chatSession->users = m_list_append(chatSession->users, newUser); 
    chatSession->numOfUsers += 1;
    err_printf("AddUserToChatList: %s\n",userHandle);
    return 0;
}

/*
** Name:    RemoveUserFromChatList
** Purpose: This function removes a user from the list
** Input:   chatSession    - chat list structure
**          userHandle     - user handle
** Output:  0 on success, -1 on failure
*/

int RemoveUserFromChatList(ChatSession *chatSession, char *userHandle)
{
    mlist     node;

    if (chatSession->users == NULL)
        return -1;

    node = m_list_find_custom(chatSession->users, userHandle, CompareUserName);
    if (node == NULL) 
        return -1; 
    chatSession->users = m_list_remove(chatSession->users, node);
    free(node->data);
    node->data = NULL;
    chatSession->numOfUsers--;
    err_printf("RemoveUserFromChatList: %s\n",userHandle);
    return 0; 
}

/*
** Name:    SearchForUser
** Purpose: This function searches a chat list and returns whether the
**          user is indeed, on a list
** Input:   chatSession - chat list structure
**          handle      - handle to search for
** Output:  0 if found, -1 if not
*/

int SearchForUser(ChatSession chatSession, char *handle)
{
    mlist node;

    node = m_list_find_custom(chatSession.users, handle, CompareUserName);
    if (node == NULL)
        return 0;
    else
        return -1;
}

/*
** Name:    ChangeUserState
** Purpose: This function changes a users state
** Input:   chatSession - chat list structure
**          userHandle  - user handle
**          newState    - new state of user
** Output:  0 on success, -1 on failure
*/

int ChangeUserState(ChatSession *chatSession, char *userHandle,
                    UserState userState)
{
    mlist node;
    ChatUser  *cu;

    if (chatSession->users == NULL)
        return -1;

    node = m_list_find_custom(chatSession->users, userHandle, CompareUserName);
    if (node == NULL)
    {
	    err_printf("ChangeUserState: %s failed. (No User Found.)\n",userHandle);
        return -1;
    }
    cu = node->data;
    cu->state = userState;
    err_printf("ChangeUserState: %s success.\n",userHandle);
    return 0;
}

/*
** Name:    DestroyChatList
** Purpose: This function destroys the list of chat users
** Input:   chatSession - chat list structure
** Output:  0 on success, -1 on failure
*/

int DestroyChatList(ChatSession chatSession)
{
    mlist node;
    ChatUser *cu;

    for (node = chatSession.users; node; node=node->next) {
        cu = node->data;
        free(cu);
	node->data = NULL;
	cu = NULL;
    }
    free(node->data);
    m_list_free(chatSession.users);
    chatSession.users = NULL;
    chatSession.numOfUsers = 0;
    return 0;         
}

/*
** Name:    CompareUserName
** Purpose: This function is used to compare nodes in the list
** Input:   a - node
**          b - name
** Output:  0 if found, something else
*/

int CompareUserName(const void * a, const void * b)
{
    ChatUser *cu;
    char     *handle;

    handle = (char *)b;
    cu = (ChatUser *)a;

    if (!handle || !cu->userHandle)
        return (-1);

    return !strcmp(handle, cu->userHandle);
}
