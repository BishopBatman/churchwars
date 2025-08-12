/************************************************************************
 * newgamedia.c   New game dialog                                       *
 * Copyright (C)  1998-2022  Ben Webb                                   *
 *                Email: benwebb@users.sf.net                           *
 *                WWW: https://churchwars.sourceforge.io/               *
 *                                                                      *
 * This program is free software; you can redistribute it and/or        *
 * modify it under the terms of the GNU General Public License          *
 * as published by the Free Software Foundation; either version 2       *
 * of the License, or (at your option) any later version.               *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, write to the Free Software          *
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,               *
 *                   MA  02111-1307, USA.                               *
 ************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>              /* For atoi */
#include <glib.h>

#include "dopewars.h"
#include "network.h"
#include "message.h"
#include "nls.h"
#include "gtkport/gtkport.h"
#include "gtk_client.h"
#include "newgamedia.h"

struct StartGameStruct {
  GtkWidget *dialog, *name, *antique, *status;
  Player *play;
#ifdef NETWORKING
  CurlConnection *MetaConn;
  GSList *NewMetaList;
  NBCallBack sockstat;
#endif
};

static struct StartGameStruct stgam;

#ifdef NETWORKING
/* Placeholder for networking-specific callback types.  Networking
 * functionality has been removed from this dialog, but some helper
 * functions remain for existing network code. */
#endif /* NETWORKING */


static gboolean GetStartGamePlayerName(gchar **PlayerName)
{
  g_free(*PlayerName);
  *PlayerName = gtk_editable_get_chars(GTK_EDITABLE(stgam.name), 0, -1);
  if (*PlayerName && (*PlayerName)[0])
    return TRUE;
  else {
    GtkMessageBox(stgam.dialog,
                  _("You can't start the game without giving a name first!"),
                  _("New Game"), GTK_MESSAGE_WARNING, MB_OK);
    return FALSE;
  }
}

static void SetStartGameStatus(gchar *msg)
{
  gtk_label_set_text(GTK_LABEL(stgam.status),
                     msg ? msg : _("Status: Waiting for user input"));
}

#ifdef NETWORKING

static void ConnectError(void)
{
  GString *neterr;
  gchar *text;
  LastError *error;

  error = stgam.play->NetBuf.error;

  neterr = g_string_new("");

  if (error) {
    g_string_assign_error(neterr, error);
  } else {
    g_string_assign(neterr, _("Connection closed by remote host"));
  }

  /* Error: GTK+ client could not connect to the given Church Wars server */
  text = g_strdup_printf(_("Status: Could not connect (%s)"), neterr->str);

  SetStartGameStatus(text);
  g_free(text);
  g_string_free(neterr, TRUE);
}

void FinishServerConnect(gboolean ConnectOK)
{
  if (ConnectOK) {
    Client = Network = TRUE;
    gtk_widget_destroy(stgam.dialog);
    GuiStartGame();
  } else {
    ConnectError();
  }
}

void DisplayConnectStatus(NBStatus oldstatus, NBSocksStatus oldsocks)
{
  NBStatus status;
  NBSocksStatus sockstat;
  gchar *text;

  status = stgam.play->NetBuf.status;
  sockstat = stgam.play->NetBuf.sockstat;
  if (oldstatus == status && sockstat == oldsocks)
    return;

  switch (status) {
  case NBS_PRECONNECT:
    break;
  case NBS_SOCKSCONNECT:
    switch (sockstat) {
    case NBSS_METHODS:
      /* Tell the user that we've successfully connected to a SOCKS server,
         and are now ready to tell it to initiate the "real" connection */
      text = g_strdup_printf(_("Status: Connected to SOCKS server %s..."),
                             Socks.name);
      SetStartGameStatus(text);
      g_free(text);
      break;
    case NBSS_USERPASSWD:
      /* Tell the user that the SOCKS server is asking us for a username
         and password */
      SetStartGameStatus(_("Status: Authenticating with SOCKS server"));
      break;
    case NBSS_CONNECT:
      text =
      /* Tell the user that all necessary SOCKS authentication has been
         completed, and now we're going to try to have it connect to
         the final destination */
          g_strdup_printf(_("Status: Asking SOCKS for connect to %s..."),
                          ServerName);
      SetStartGameStatus(text);
      g_free(text);
      break;
    }
    break;
  case NBS_CONNECTED:
    break;
  }
}

#endif /* NETWORKING */

static void StartSinglePlayer(GtkWidget *widget, gpointer data)
{
  WantAntique =
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(stgam.antique));
  if (!GetStartGamePlayerName(&stgam.play->Name))
    return;
  GuiStartGame();
  gtk_widget_destroy(stgam.dialog);
}

static void CloseNewGameDia(GtkWidget *widget, gpointer data)
{
#ifdef NETWORKING
  /* Terminate any existing connection attempts */
  if (stgam.play->NetBuf.status != NBS_CONNECTED) {
    ShutdownNetworkBuffer(&stgam.play->NetBuf);
  }
  if (stgam.MetaConn) {
    CloseCurlConnection(stgam.MetaConn);
    stgam.MetaConn = NULL;
  }
  ClearServerList(&stgam.NewMetaList);
#endif
}

static void set_initial_player_name(GtkEntry *entry, Player *play)
{
  char *name = GetPlayerName(play);
  if (*name) {
    gtk_entry_set_text(entry, name);
  } else {
    /* If name is blank, use the first word from the user's full login name */
    char *firstspace;
    name = g_strdup(g_get_real_name());
    g_strstrip(name);
    firstspace = strchr(name, ' ');
    if (firstspace) {
      *firstspace = '\0';
    }
    /* "Unknown" is returned from g_get_real_name() on error */
    gtk_entry_set_text(entry, strcmp(name, "Unknown") == 0 ? "" : name);
    g_free(name);
  }
}

#ifdef NETWORKING
void NewGameDialog(Player *play, NBCallBack sockstat, CurlConnection *MetaConn)
#else
void NewGameDialog(Player *play)
#endif
{
  GtkWidget *vbox, *vbox2, *hbox, *label, *entry;
  GtkWidget *button, *dialog;
  GtkAccelGroup *accel_group;
#if GTK_MAJOR_VERSION == 2
  guint AccelKey;
#endif

#ifdef NETWORKING
  /* Parameters are currently unused but kept for API compatibility */
  (void)sockstat;
  (void)MetaConn;
#endif

  stgam.play = play;
  stgam.dialog = dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect(G_OBJECT(dialog), "destroy",
                   G_CALLBACK(CloseNewGameDia), NULL);

  gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(MainWindow));
  gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 300);
  accel_group = gtk_accel_group_new();

  /* Title of 'New Game' dialog */
  gtk_window_set_title(GTK_WINDOW(dialog), _("New Game"));
  my_set_dialog_position(GTK_WINDOW(dialog));
  gtk_container_set_border_width(GTK_CONTAINER(dialog), 7);
  gtk_window_add_accel_group(GTK_WINDOW(dialog), accel_group);

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 7);
  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 7);

  label = gtk_label_new("");
#if GTK_MAJOR_VERSION == 2
  AccelKey = gtk_label_parse_uline(GTK_LABEL(label),
#else
  gtk_label_set_text_with_mnemonic(GTK_LABEL(label),
#endif
                                   _("Greetings Trader, what's your _name?"));
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

  entry = stgam.name = gtk_entry_new();
#if GTK_MAJOR_VERSION == 2
  gtk_widget_add_accelerator(entry, "grab-focus", accel_group, AccelKey,
                             GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
#else
  gtk_label_set_mnemonic_widget(GTK_LABEL(label), entry);
#endif
  set_initial_player_name(GTK_ENTRY(entry), stgam.play);
  gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);

  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

  vbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 7);
  gtk_container_set_border_width(GTK_CONTAINER(vbox2), 8);
  stgam.antique = gtk_check_button_new_with_label("");
  button = gtk_button_new_with_label("");

  /* Button to start a new single-player (standalone, non-network) game */
  SetAccelerator(button, _("_Start single-player game"), button,
                 "clicked", accel_group, TRUE);
  g_signal_connect(G_OBJECT(button), "clicked",
                   G_CALLBACK(StartSinglePlayer), NULL);
  gtk_box_pack_start(GTK_BOX(vbox2), button, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(vbox), vbox2, TRUE, TRUE, 0);

  /* Caption of status label in New Game dialog before anything has
   * happened */
  label = stgam.status = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

  gtk_container_add(GTK_CONTAINER(stgam.dialog), vbox);

  gtk_widget_grab_focus(stgam.name);

  SetStartGameStatus(NULL);
  gtk_widget_show_all(dialog);
}

