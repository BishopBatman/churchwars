/* win32_client.h  dopewars client using native Win32 user interface    */
/* Copyright (C)  1998-2000  Ben Webb                                   */
/*                Email: ben@bellatrix.pcl.ox.ac.uk                     */
/*                WWW: http://bellatrix.pcl.ox.ac.uk/~ben/dopewars/     */

/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU General Public License          */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */

/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU General Public License for more details.                         */

/* You should have received a copy of the GNU General Public License    */
/* along with this program; if not, write to the Free Software          */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston,               */
/*                   MA  02111-1307, USA.                               */


#ifndef __WIN32_CLIENT_H__
#define __WIN32_CLIENT_H__

#ifdef CYGWIN
#include <windows.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

int APIENTRY Win32Loop(HINSTANCE hInstance,HINSTANCE hPrevInstance,
                       LPSTR lpCmdLine,int nCmdShow);

#endif
#endif