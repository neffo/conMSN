/*
 * EveryBuddy 
 *
 * Copyright (C) 1999, Torrey Searle <tsearle@uci.edu>
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

/* 11/5/01 - Thomas Stewart - T.Stewart@stewart.umist.ac.uk
		Added mail notification script stuff */

/*
 * msn.c
 */

//#include<gtk/gtk.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
//#if defined( _WIN32 )
//#include "../libmsn/libmsn.h"
//typedef unsigned long u_long;
//typedef unsigned long ulong;
//#else
//#include "libmsn/libmsn.h"
//#endif
#include "libmsn.h"
#include "ebmsn.h"
//#include "contact.h"
//#include "account.h"
//#include "service.h"
//#include "chat_window.h"
//#include "info_window.h"
//#include "gtk_eb_html.h"
//#include "util.h"
//#include "status.h"
//#include "globals.h"
//#include "dialog.h"
//#include "message_parse.h"
//#include "chat_room.h"
//#include "value_pair.h"
//#include "input_list.h"

//#include "pixmaps/msn_online.xpm"
//#include "pixmaps/msn_away.xpm"

GList *msn_contacts = NULL;

enum
{
	MSN_ONLINE,
	MSN_OFFLINE,
    MSN_HIDDEN,
	MSN_BUSY,
	MSN_IDLE,
	MSN_BRB,
	MSN_AWAY,
	MSN_PHONE,
	MSN_LUNCH,
};

static char * msn_status_strings[] =
{ "", "(Offline)", "(Hidden)", "(Busy)", "(Extended Away)", "(BRB)", "(Away)", "(Phone)", "(Lunch)"};

static char msn_server[255] = "messenger.hotmail.com";
static char msn_port[10] = "1863";

static int do_mail_notify = 0;
static int do_mail_notify_show = 0;
static int do_mail_notify_run_script = 0;
static char do_mail_notify_script_name[255];
static input_list * msn_prefs = NULL;

/* Use this struct to hold any service specific information you need
 * about people on your contact list
 */

typedef struct _eb_msn_account_data
{
	gint status;   //the status of the user
} eb_msn_account_data;

typedef struct _msn_info_data
{
    gchar *profile;
} msn_info_data;


/* Use this struct to hold any service specific information you need about
 * local accounts
 * below are just some suggested values
 */

typedef struct _eb_msn_local_account_data
{
	char password[255]; // account password
	int fd;				// the file descriptor
	int status;			// the current status of the user
} eb_msn_local_account_data;

static eb_local_account *msn_local_account;
void eb_msn_terminate_chat( eb_account * account );
void eb_msn_add_user( eb_account * account );
void eb_msn_del_user( eb_account * account );
void eb_msn_login( eb_local_account * account );
void eb_msn_logout( eb_local_account * account );
void msn_info_update(info_window *iw); 
void msn_info_data_cleanup(info_window *iw);
static void msn_authorize_callback( GtkWidget * widget, gpointer data );
void MSNRegisterCallbacks(void);
void MSNInstantMessage(void *data);
void MSNMail(void *data);
void MSNStatusChange(void *data);
void MSNAuthRequest(void *data);
void MSNLogout(void *data);
static int ref_count = 0;
static int is_setting_state = 0;

static void msn_authorize_callback( GtkWidget * widget, gpointer data )
{
    MSN_AuthMessage_PTR am;
    int response = (int)gtk_object_get_user_data(GTK_OBJECT(widget));

    am = (MSN_AuthMessage_PTR)data;
    if(response)
    {
        MSN_AuthorizeContact(am->conn, am->requestor);
    }
    free(am->requestor);
    free(am);
}


gboolean eb_msn_query_connected( eb_account * account )
{
    eb_msn_account_data * aad = account->protocol_account_data;

    if(ref_count <= 0 )
        aad->status = MSN_OFFLINE;
    return aad->status != MSN_OFFLINE;

}

void eb_msn_login( eb_local_account * account )
{
    eb_msn_local_account_data * alad;
    char server[256];
    int  port;

    msn_local_account = account;
    MSNRegisterCallbacks(); 
    account->connected = 1; 
    alad = (eb_msn_local_account_data *)account->protocol_local_account_data;

    if(!g_strncasecmp(msn_server, "msgr-ns3.hotmail.com", 
			    strlen("msgr-ns3.hotmail.com")+1))
    { 
	    strcpy(msn_server, "messenger.hotmail.com"); 
    }

    port = atoi(msn_port);
    strcpy(server, msn_server);

    if (MSN_Login(account->handle, alad->password, server,  port) != 0) {
        fprintf(stderr, "Unable to login to MSN\n");
        account->connected = 0;
        return;
    }

    ref_count++;

    is_setting_state = 1;

    if(account->status_menu)
    {
        gtk_check_menu_item_set_active
        (
                GTK_CHECK_MENU_ITEM
                (
                    g_slist_nth(account->status_menu, MSN_ONLINE)->data
                ), TRUE
        );

    }

    is_setting_state = 0;

}

void eb_msn_logout( eb_local_account * account )
{
  GList *l;
  MSN_Logout();
  
  for (l = msn_contacts; l; l = l->next) {
    eb_account * ea = find_account_by_handle(l->data, MSN_SERVICE_ID);
    buddy_logoff(ea);
    buddy_update_status(ea);
  }
 account->connected = 0;
}

void eb_msn_send_im( eb_local_account * from, eb_account * account_to, 
					 gchar * message)
{
    MSN_SendMessage(account_to->handle, message);
}

eb_local_account * eb_msn_read_local_account_config( GList * values )
{
	char buff[255];
	char * c;

	eb_local_account * ela = g_new0(eb_local_account,1);
	eb_msn_local_account_data *  mlad = g_new0( eb_msn_local_account_data, 1);

	ela->handle = strdup( value_pair_get_value( values, "SCREEN_NAME" ));
	strcpy( mlad->password, value_pair_get_value( values, "PASSWORD") );

	/*the alias will be the persons login minus the @hotmail.com */
	strcpy( buff, ela->handle );
	c = strtok( buff, "@" );
	strcpy(ela->alias, buff );

	mlad->status = MSN_OFFLINE;
	ela->protocol_local_account_data = mlad;
	
	ela->service_id = MSN_SERVICE_ID;

	return ela;
}

GList * eb_msn_write_local_config( eb_local_account * account )
{
	value_pair * val;
	GList * vals = NULL;
	eb_msn_local_account_data * mlad = account->protocol_local_account_data;

	val = g_new0( value_pair, 1 );
	strcpy(val->key, "SCREEN_NAME" );
	strcpy(val->value, account->handle );
	vals = g_list_append( vals, val );

	val = g_new0( value_pair, 1 );
	strcpy(val->key, "PASSWORD");
	strcpy(val->value, mlad->password );
	vals = g_list_append( vals, val );

	return vals;
}

eb_account * eb_msn_read_account_config( GList * config, struct contact * contact)
{
	eb_account * ea = g_new0(eb_account, 1);
	eb_msn_account_data * mad = g_new0( eb_msn_account_data, 1 );

    mad->status = MSN_OFFLINE;
	strncpy(ea->handle, value_pair_get_value( config, "NAME"), 255 );
	
	ea->service_id = MSN_SERVICE_ID;
	ea->protocol_account_data = mad;
	ea->account_contact = contact;
	ea->list_item = NULL;
	ea->online = 0;
	ea->status = NULL;
	ea->pix = NULL;
	ea->icon_handler = -1;
	ea->status_handler = -1;

	eb_msn_add_user(ea);

	return ea;
}

GList * eb_msn_get_states()
{
	GList * list = NULL;
	list = g_list_append( list, "Online" );
	list = g_list_append( list, "Offline" );
    list = g_list_append( list, "Hidden" );
	list = g_list_append( list, "Busy" );
	list = g_list_append( list, "Idle" );
	list = g_list_append( list, "Be Right Back" );
	list = g_list_append( list, "Away" );
	list = g_list_append( list, "On Phone" );
	list = g_list_append( list, "Out To Lunch" );

	return list;

}

gint eb_msn_get_current_state( eb_local_account * account )
{
	eb_msn_local_account_data * mlad = account->protocol_local_account_data;
	return mlad->status;
}

void eb_msn_set_current_state( eb_local_account * account, gint state )
{
    MSN_ChangeState(state);
}

void eb_msn_terminate_chat(eb_account * account )
{
    char *newHandle;
    AddHotmail(account->handle, &newHandle);
    MSN_EndChat(account->handle);
    free(newHandle);
}

void eb_msn_add_user(eb_account * account )
{
    msn_contacts = g_list_append(msn_contacts, account->handle);
    MSN_AddContact(account->handle);
}

void eb_msn_del_user(eb_account * account )
{
    msn_contacts = g_list_remove(msn_contacts, account->handle);
    MSN_RemoveContact(account->handle);
}

eb_account * eb_msn_new_account( gchar * account )
{
	eb_account * ea = g_new0(eb_account, 1);
	eb_msn_account_data * mad = g_new0( eb_msn_account_data, 1 );
	
	ea->protocol_account_data = mad;
	strncpy(ea->handle, account, 255 );
	ea->service_id = MSN_SERVICE_ID;
	mad->status = MSN_OFFLINE;

	return ea;
}

static gint pixmaps = 0;
static GdkPixmap * eb_msn_pixmap[MSN_LUNCH+1];
static GdkBitmap * eb_msn_bitmap[MSN_LUNCH+1];

void eb_msn_init_pixmaps()
{
	gint i;
	gchar ** xpm;
	
	for (i=MSN_ONLINE; i<=MSN_LUNCH; i++) {
		switch(i) 
		{
		case MSN_ONLINE:
			xpm = msn_online_xpm;	
			break;
		default:
			xpm = msn_away_xpm;
			break;
		}
		eb_msn_pixmap[i] = gdk_pixmap_create_from_xpm_d(statuswindow->window,
			&eb_msn_bitmap[i], NULL, xpm);
	}
	pixmaps = 1;
}

void eb_msn_get_status_pixmap( eb_account * account, GdkPixmap ** pm, GdkBitmap ** bm )
{
	eb_msn_account_data * mad;
	
	if (!pixmaps)
		eb_msn_init_pixmaps();
	
	mad = account->protocol_account_data;
	
	*pm = eb_msn_pixmap[mad->status];
	*bm = eb_msn_bitmap[mad->status];
}


gchar * eb_msn_get_status_string( eb_account * account )
{
	eb_msn_account_data * mad = account->protocol_account_data;
	return msn_status_strings[mad->status];
}

void eb_msn_set_idle( eb_local_account * account, gint idle )
{
    if ((idle == 0) && eb_msn_get_current_state(account) == MSN_IDLE)
    {
        if(account->status_menu)
        {
            gtk_check_menu_item_set_active
            (
                GTK_CHECK_MENU_ITEM
                (
                    g_slist_nth(account->status_menu, MSN_ONLINE)->data
                ), TRUE
            );

        }

    }
    if( idle >= 600 && eb_msn_get_current_state(account) == MSN_ONLINE )
    {
        if(account->status_menu)
        {
            gtk_check_menu_item_set_active
            (
                GTK_CHECK_MENU_ITEM
                (
                    g_slist_nth(account->status_menu, MSN_IDLE)->data
                ), TRUE
            );

        }
    }

}

void eb_msn_set_away( eb_local_account * account, char * message )
{
    if(message)
    {
        if(account->status_menu)
        {
            gtk_check_menu_item_set_active
            (
                GTK_CHECK_MENU_ITEM
                (
                    g_slist_nth(account->status_menu, MSN_AWAY)->data
                ), TRUE
            );

        }
    }
    else
    {
        if(account->status_menu)
        {
            gtk_check_menu_item_set_active
            (
                GTK_CHECK_MENU_ITEM
                (
                    g_slist_nth(account->status_menu, MSN_ONLINE)->data
                ), TRUE
            );

        }

    }

}

void eb_msn_send_chat_room_message( eb_chat_room * room, gchar * message )
{
}

void eb_msn_join_chat_room( eb_chat_room * room )
{
}

void eb_msn_leave_chat_room( eb_chat_room * room )
{
}

eb_chat_room * eb_msn_make_chat_room( gchar * name, eb_local_account * account )
{
	return NULL;
}

void eb_msn_send_invite( eb_local_account * account, eb_chat_room * room,
						  char * user, char * message )
{
}

void eb_msn_get_info( eb_local_account * reciever, eb_account * sender)
{
   gchar buff[1024];

   if(sender->infowindow == NULL){
     sender->infowindow = eb_info_window_new(reciever, sender);
     gtk_widget_show(sender->infowindow->window);
   }

   if(sender->infowindow->info_type == -1 || sender->infowindow->info_data == NULL){
      if(sender->infowindow->info_data == NULL) {
        sender->infowindow->info_data = malloc(sizeof(msn_info_data));
        ((msn_info_data *)sender->infowindow->info_data)->profile = NULL;
        sender->infowindow->cleanup = msn_info_data_cleanup;
      }
      sender->infowindow->info_type = MSN_SERVICE_ID;
    }
    if(sender->infowindow->info_type != MSN_SERVICE_ID) {
       /*hmm, I wonder what should really be done here*/
       return; 
    }
    sprintf(buff,"http://profiles.passport.com/%s?lc=1033&cbid=MSMSGS",sender->handle);
    if( ((msn_info_data *)sender->infowindow->info_data)->profile != NULL)
      free(((msn_info_data *)sender->infowindow->info_data)->profile);
    ((msn_info_data *)sender->infowindow->info_data)->profile = malloc(strlen(buff)+1);
    strcpy(((msn_info_data *)sender->infowindow->info_data)->profile,buff);

    msn_info_update(sender->infowindow);
}


void msn_info_update(info_window *iw) {
  gchar buff[1024];
  msn_info_data * mid = (msn_info_data *)iw->info_data;

  clear_info_window(iw);
  sprintf(buff,"Profile for <B>%s</B><BR><HR>",iw->remote_account->handle);
  gtk_eb_html_add(GTK_SCTEXT(iw->info),buff,0,0,0);
  sprintf(buff,"<a href=\"%s\">%s</a>",mid->profile,mid->profile);
  gtk_eb_html_add(GTK_SCTEXT(iw->info),buff,0,0,0);
}

void msn_info_data_cleanup(info_window *iw){
  msn_info_data * mid = (msn_info_data *)iw->info_data;
  if(mid->profile != NULL) free(mid->profile);
}

input_list * eb_msn_get_prefs()
{
	return msn_prefs;
}

void eb_msn_read_prefs_config(GList * values)
{
	char * c;
	c = value_pair_get_value(values, "server");
	if(c)
	{
		strcpy(msn_server, c);
	}
	c = value_pair_get_value(values, "port");
	if(c)
	{
		strcpy(msn_port, c);
	}
	c = value_pair_get_value(values, "do_mail_notify");
	if(c)
	{
		do_mail_notify = atoi(c);
	}
	c = value_pair_get_value(values, "do_mail_notify_show");
	if(c)
        {
                do_mail_notify_show = atoi(c);
        }
	c = value_pair_get_value(values, "do_mail_notify_run_script");
        if(c)
        {
                do_mail_notify_run_script = atoi(c);
        }
	c = value_pair_get_value(values, "do_mail_notify_script_name");
        if(c)
        {
		strcpy(do_mail_notify_script_name, c); 
        }
}

GList * eb_msn_write_prefs_config()
{
	char buffer[5];
	GList * config = NULL;

	config = value_pair_add(config, "server", msn_server);
	config = value_pair_add(config, "port", msn_port);
	
	sprintf(buffer, "%d", do_mail_notify);
	config = value_pair_add(config, "do_mail_notify", buffer);

        sprintf(buffer, "%d", do_mail_notify_show);
        config = value_pair_add(config, "do_mail_notify_show", buffer);

        sprintf(buffer, "%d", do_mail_notify_run_script);
        config = value_pair_add(config, "do_mail_notify_run_script", buffer);

        config = value_pair_add(config, "do_mail_notify_script_name", do_mail_notify_script_name);

	return config;
}



struct service_callbacks * eb_msn_query_callbacks()
{
	struct service_callbacks * sc;

	sc = g_new0( struct service_callbacks, 1 );
	
	sc->query_connected = eb_msn_query_connected;
	sc->login = eb_msn_login;
	sc->logout = eb_msn_logout;
	sc->send_im = eb_msn_send_im;
	sc->read_local_account_config = eb_msn_read_local_account_config;
	sc->write_local_config = eb_msn_write_local_config;
	sc->read_account_config = eb_msn_read_account_config;
	sc->get_states = eb_msn_get_states;
	sc->get_current_state = eb_msn_get_current_state;
	sc->set_current_state = eb_msn_set_current_state;
	sc->add_user = eb_msn_add_user;
	sc->del_user = eb_msn_del_user;
	sc->new_account = eb_msn_new_account;
	sc->get_status_string = eb_msn_get_status_string;
	sc->get_status_pixmap = eb_msn_get_status_pixmap;
	sc->set_idle = eb_msn_set_idle;
	sc->set_away = eb_msn_set_away;
	sc->send_chat_room_message = eb_msn_send_chat_room_message;
	sc->join_chat_room = eb_msn_join_chat_room;
	sc->leave_chat_room = eb_msn_leave_chat_room;
	sc->make_chat_room = eb_msn_make_chat_room;
	sc->send_invite = eb_msn_send_invite;
    sc->terminate_chat = eb_msn_terminate_chat;
        sc->get_info = eb_msn_get_info;

	sc->get_prefs = eb_msn_get_prefs;
	sc->read_prefs_config = eb_msn_read_prefs_config;
	sc->write_prefs_config = eb_msn_write_prefs_config;

	{
		input_list * il = g_new0(input_list, 1);
		msn_prefs = il;
		il->widget.entry.value = msn_server;
		il->widget.entry.name = "Server:";
		il->type = EB_INPUT_ENTRY;

		il->next = g_new0(input_list, 1);
		il = il->next;
		il->widget.entry.value = msn_port;
		il->widget.entry.name = "Port:";
		il->type = EB_INPUT_ENTRY;
		
		il->next = g_new0(input_list, 1);
		il = il->next;
		il->widget.checkbox.value = &do_mail_notify;
		il->widget.checkbox.name = "MSN Mail Notification";
		il->type = EB_INPUT_CHECKBOX;

		il->next = g_new0(input_list, 1);
                il = il->next;
                il->widget.checkbox.value = &do_mail_notify_show;
                il->widget.checkbox.name = "Show Mail Notification";
                il->type = EB_INPUT_CHECKBOX;

		il->next = g_new0(input_list, 1);
                il = il->next;
                il->widget.checkbox.value = &do_mail_notify_run_script;
                il->widget.checkbox.name = "Run Script on Mail Notification";
                il->type = EB_INPUT_CHECKBOX;

		il->next = g_new0(input_list, 1);
                il = il->next;
		il->widget.entry.value = do_mail_notify_script_name;
                il->widget.entry.name = "Script Name:";
                il->type = EB_INPUT_ENTRY;
	}

	return sc;
}

/*
** Name:    RegisterCallbacks
** Purpose: This function registers the call backs for the libmsn library to
**          use to interact with an outside library
** Input:   none
** Output:  none
*/

void MSNRegisterCallbacks(void)
{
    MSN_RegisterErrorOutput((ERR_CALLBACK *)&do_error_dialog); 
    MSN_RegisterCallback(MSN_MSG, MSNInstantMessage);
    MSN_RegisterCallback(MSN_ILN, MSNStatusChange);
    MSN_RegisterCallback(MSN_NLN, MSNStatusChange);
    MSN_RegisterCallback(MSN_FLN, MSNStatusChange);
    MSN_RegisterCallback(MSN_AUTH, MSNAuthRequest);
    MSN_RegisterCallback(MSN_OUT, MSNLogout);
    MSN_RegisterCallback(MSN_MAIL, MSNMail);
}

/*
** Name:     MSNInstantMessage
** Purpose: This function acts as the gateway between the libmsn and the 
**           gtk interface
** Input:    data    - data needed for instant message
** Output:   none
*/

void MSNInstantMessage(void *data)
{
    MSN_InstantMessage_PTR im;
    eb_account *sender = NULL;
    eb_account *ea;
    eb_msn_account_data *aad;
    char *newHandle = NULL;

    im = (MSN_InstantMessage_PTR)data;
    sender = find_account_by_handle(im->sender, MSN_SERVICE_ID);
    if (sender == NULL) {
        AddHotmail(im->sender, &newHandle);
        sender = find_account_by_handle(newHandle, MSN_SERVICE_ID);
        free(newHandle);
    } 
    if (sender == NULL) {
        ea = (eb_account *)malloc(sizeof(eb_account));
        aad = g_new0(eb_msn_account_data, 1);
        strcpy(ea->handle, im->sender);
        ea->service_id = MSN_SERVICE_ID;
        aad->status = MSN_ONLINE;
        ea->protocol_account_data = aad;

        add_unknown(ea);
        sender = ea;
    }

    eb_parse_incomming_message(msn_local_account, sender, im->msg);
}

/*
** Name:	MSNMail
** Purpose:	The function notifys the user of unread e-mails in 
**		users hotmail account
** Input:	Mail Notification
** Output:	none
*/

void MSNMail(void *data)
{
    if (do_mail_notify !=0) {
	MSN_MailNotification *newmail = (MSN_MailNotification *) data;
	if (newmail->unread != 0) {
	    char buffer[1024];
	    
	    if (newmail->from != NULL) {
		snprintf(buffer, 1024, "From: %s(%s)\nSubject: %s", newmail->from,
				    newmail->fromaddr, newmail->subject);
	    }
	    else {
		    snprintf(buffer, 1024, "You have %d new message%s.", newmail->unread,
				    (newmail->unread > 1) ? "s" : "");
				    
	    }
	    if(do_mail_notify_show != 0) {
	    	//printf("do_mail_notify_show(test)\n");
		do_error_dialog(buffer, "MSN email notification");
	    }
	    if(do_mail_notify_run_script != 0) {
		//printf("do_mail_notify_run_script(test)\n");
	    	system(do_mail_notify_script_name);
	    }
	}
    }
}

/*
** Name:    MSNStatusChange
** Purpose: This function is a callback that is called to update the
**          status of a contact in the forward list
** Input:   data - data passed to contact
** Output:  none
*/

void MSNStatusChange(void *data)
{
    MSN_StatusChange_PTR sc;
    eb_account *ea;
    eb_msn_account_data *aad;
    char *newHandle = NULL;

    sc = (MSN_StatusChange_PTR)data;

    ea = find_account_by_handle(sc->handle, MSN_SERVICE_ID);
    if (ea)
        aad = ea->protocol_account_data;
    else {
        AddHotmail(sc->handle, &newHandle);
        ea = find_account_by_handle(newHandle, MSN_SERVICE_ID);
        free(newHandle);
        if (ea)
            aad = ea->protocol_account_data;
        else  
            return;
    }

    if ((sc->newStatus != USER_FLN) && (aad->status == MSN_OFFLINE))
		buddy_login(ea);
    else if ((sc->newStatus == USER_FLN) && (aad->status != MSN_OFFLINE))
		buddy_logoff(ea);

	aad->status = sc->newStatus;
	buddy_update_status(ea);
}

/*
** Name:    MSNAuthRequest
** Purpose: This function is a callback that is called to ask for authorization
** Input:   data - data passed to contact
** Output:  none
*/

void MSNAuthRequest(void *data)
{
    MSN_AuthMessage_PTR am;
    char dialog_message[1025];

    am = (MSN_AuthMessage_PTR)data;
    sprintf(dialog_message, "%s\n, the MSN user %s would like to add you to their contact list.\nWould you like to authorize them?", 
            am->handle, am->requestor);
    do_dialog(dialog_message, "Authorize MSN User", msn_authorize_callback, 
              (gpointer)am);
              
    return;
}

/*
** Name:    MSNLogout
** Purpose: This function is called when a user is logged out automatically
**          by the server
** Input:   data - date to be passed
** Output:  none
*/

void MSNLogout(void *data)
{
    ref_count--;
    is_setting_state = 1;

    msn_local_account->connected = 0;
    if(msn_local_account->status_menu) {
        gtk_check_menu_item_set_active( 
             GTK_CHECK_MENU_ITEM (
                  g_slist_nth(msn_local_account->status_menu, 
                  MSN_OFFLINE)->data), TRUE);
    }

    is_setting_state = 0;
}
