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

#include "errors.h"
#include <stdio.h>
#include <glib.h>

/*
** Name:    MSN_ErrorString
** Purpose: This function returns the error string associated with a particular
**          code
** Input:   buffer - where to store message
**          code   - error code
** Output:  0 if code found, -1 if not
*/

int MSN_ErrorString(char *buffer, int code)
{

    g_snprintf(buffer, 1024, "Unknown Error");
    return -1;
}

