#define _REENTRANT
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
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include "libmsn.h"
#include "parse_utils.h"
#include "msn_commands.h"

extern char CommandString[NUM_OF_COMMANDS][COMMAND_LENGTH];

/* DecodeMime auxiliary funcs */
static void base64_decode(char **dest, char **src);
static void quoted_printable_decode(char **dest, char **src);
#ifndef HAVE_STRSEP
char *strsep (char **stringp, const char *delim);
#endif

/*
** Name:    ParseHostPort
** Purpose: This function parses out the host and port for a switchboard
**          server
** Input:   address - host and port
**          host    - host
**          port    - port number
** Output:  0 on success, -1 on failure
*/

int ParseHostPort(char *address, char **host, int *port)
{
    *host = strsep(&address, ":");
    if (*host == NULL)
        return -1;
    if (address == NULL)
        *port = 1863;
    else
        *port = atoi(address);

    return 0;
}

/*
** Name:    ParseMimeHeaders
** Purpose: This function returns the mime parts out of the message
** Input:   message - the whole message
**          mimeInfo    - mime info
**          im          - instant message type
** Output:  0 on success, -1 on failure
*/

int ParseMimeHeaders(char *message, char **mimeInfo, char **im)
{
    char *place;
    
    *mimeInfo = strstr(message, "MIME-Version: ");
    place = strstr(message, "\r\n\r\n");
    if (place == NULL)
        return -1;

    *place = '\0';
    *im = place + 4;
    if (strcasecmp(*im, "\r\n") == 0)
        *im += 2;
    return 0;
}

/*
** Name:    DecodeMime
** Purpose: decode a mime encoded message. the original string
**          IS MODIFIED
** Input:   message
** Output:  decoded message
*/
void DecodeMime(char *s)
{
	char *buf = NULL;	
	
    if(!s) return;
	
	buf = s;	
	
	while(*buf) {		
		if ((*buf == '=') && (*++buf == '?')) {
			char tmp;
			while(*++buf != '?');
			buf++;
			tmp = *buf; buf += 2;
			if ((tmp == 'Q') || (tmp == 'q')) quoted_printable_decode(&s, &buf);
			else if ((tmp == 'B') || (tmp == 'b')) base64_decode(&s, &buf);
		}
		else {*s++ = *buf++;}
    }
    *s=0;
}

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

int ParseArguments(char *string, char *delimiter, char ***args, int *numOfArgs)
{

    char *place,*temp;

    if ((string == NULL) || (delimiter == NULL))
        return -1;

    *args = NULL;
    *numOfArgs = 0;

#ifdef HAVE_STRTOK_R
    temp = strtok_r(string, delimiter, &place);
#else
#warning You do not have a re-entrant strtok_r - MSN MAY CRASH if run multithreaded
    temp = strtok(string, delimiter);
#endif

    while (temp) {
        *args = (char **)realloc(*args, sizeof(char *) * (*numOfArgs+1));
        (*args)[*numOfArgs] = strdup(temp);
        *numOfArgs += 1;
#ifdef HAVE_STRTOK_R
        temp = strtok_r(place, delimiter, &place);  
#else
        temp = strtok(NULL, delimiter);  
#endif
    } 
    return 0;

} 

/*
** Name:    DestroyArguments
** Purpose: This function destroys a set of arguments
** Input:   args      - arguments to be destroyed
**          numOfArgs - number of arguments
** Output:  0 on success, -1 on failure
*/

int DestroyArguments(char ***args, int numOfArgs)
{
    int count = 0;

    if ((args == NULL) || (!numOfArgs))
        return -1;

    while (count < numOfArgs) {
        free((*args)[count++]);
    }
    free(*args);
    (*args) = NULL;
    return 0;
}

/*
** Name:    AddHotmail
** Purpose: This function adds @hotmail.com to a username that needs it
**          If @hotmail.com is already present, string is not modified
** Input:   src - string to look at
**          dst - destination string
** Output:  0 on success, -1 on failure
*/

int AddHotmail(const char *src, char **dst)
{
    *dst = NULL;
    if (src == NULL)
        return -1;

	if(strchr(src, '@') == NULL) {
        *dst = (char *)malloc(strlen(src) + strlen("@hotmail.com") + 1);  
        sprintf(*dst, "%s@hotmail.com", src);
    }
    else
        *dst = strdup(src); 

    return 0;
}

/*
** Name:    RemoveHotmail
** Purpose: This function removes @hotmail.com from any string
**          If @hotmail.com is already missing, string is not modifed
** Input:   src - string to look at
**          dst - destination string
** Output:  0 on success, -1 on failure
*/

int RemoveHotmail(const char *src, char **dst)
{
    char *temp;
    char *ptr;
    *dst = NULL;

    if (src == NULL)
        return -1;

    temp = strdup(src);
    ptr = strstr(temp, "@hotmail.com");
    if (ptr == NULL) 
        *dst = strdup(src);
    else {
        *ptr = '\0';
        *dst = strdup(temp);
    }

    free(temp);
    return 0;
}

static void
base64_decode(char **dest, char **src)
{
	static unsigned char tab[256];
	static unsigned char initialized = 0;
	char *buf = *src, *s = *dest;
	unsigned int pr[6];
	register int n;
	register int doit=1;

	if (!initialized) {
		int i, n = 0;
		initialized++;
		memset(tab, 100, 256);
		tab[0] = tab[255] = 255;
		for (i = 'A'; i <= 'Z'; i++) tab[i] = n++;
		for (i = 'a'; i <= 'z'; i++) tab[i] = n++;
		for (i = '0'; i <= '9'; i++) tab[i] = n++;
		tab['+'] = n++;
		tab['/'] = n++;
		tab['='] = 255;
	}
		
	while(doit) {
		n=0;
		while(n < 4) {
			register unsigned char ch;
			ch=tab[*(unsigned char *)buf];
			if(ch < 100) { pr[n]=ch; n++; buf++; continue; }
			if(ch == 100) { buf++; continue; }
			while (*buf == '=') buf++;
			doit = 0; break;
		}
		if(!doit && n < 4) {pr[n+2] = pr[n+1] = pr[n] = 0;}

		*(char *)s = (pr[0] << 2) | (pr[1] >> 4);
		*(char *)(s+1) = (pr[1] << 4) | (pr[2] >> 2);
		*(char *)(s+2) = (pr[2] << 6) | pr[3];

		s+=(n * 3) >> 2 ;
	}
	if ((*buf == '?') && *(buf+1) == '=') buf++;
	*src = buf;
	*dest = s;
}


static void
quoted_printable_decode(char **dest, char **src)
{
	char *buf = *src;
	char *s = *dest;
	register int n = 0;

	buf--;
	while(*++buf) {
		if(*buf == '_') {
			*s++ = ' ';
			continue;
		};

		if (*buf == '?') {
			buf++;
			if (!*buf) {*src = --buf; break;}
			else if(*buf == '=') {*src = ++buf; break;}
		}
		
        if(*buf == '=') {						
            n = *++buf;
            if(!n) {buf--; break; }
			if(!*(buf+1)) break;

			if((n >= '0') && (n <= '9')) *s = (n - '0');
			else if(((n >= 'A') && (n <= 'F')) || ((n >= 'a') && (n <= 'f')) )
				*s = 10 + n - ((n>='a')?'a':'A');
			else {
				if(n == '\n' || n == '\r') continue;
				*s++ = '='; *s++ = n; continue;
			};

			*s <<= 4;
			n = *++buf;

			if((n >= '0') && (n <= '9')) *s += (n - '0');
			else if(((n >= 'A') && (n <= 'F')) || ((n >= 'a') && (n <= 'f')))
				*s += 10 + n - ((n>='a')?'a':'A');
			else {
				if(n == '\n' || n == '\r') { *s++ = n; continue; };
				*s = ' '; continue;
			};

			s++;
			continue;
        };		
		*s++ = *buf;
    }
    *dest = s;
}


/**
 * We don't have strsep() on DEC OSF or SunOS (maybe not on any
 * non-Linux system -- ? ).  So we'll take it from glibc-2.0.6.
 * 
 * David Sweet <dsweet@chaos.umd.edu>
 **/

#ifndef HAVE_STRSEP

char *strsep (char **stringp, const char *delim) {

  char *begin, *end;

  begin = *stringp;
  if (! begin || *begin == '\0')
    return NULL;

  /* Find the end of the token.  */
  end = strpbrk (begin, delim);
  if (end)
    {
      /* Terminate the token and set *STRINGP past NUL character.  */
      *end++ = '\0';
      *stringp = end;
    }
  else
    /* No more delimiters; this is the last token.  */
    *stringp = 0;

  return begin;
}                      
#endif
