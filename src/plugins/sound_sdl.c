/************************************************************************
 * sound_sdl.c    dopewars sound system (SDL driver)                    *
 * Copyright (C)  1998-2022  Ben Webb                                   *
 *                Email: benwebb@users.sf.net                           *
 *                WWW: https://dopewars.sourceforge.io/                 *
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

#ifdef HAVE_SDL_MIXER
#include <SDL.h>
#include <SDL_mixer.h>
#include <glib.h>
#include "../sound.h"
#include <stdio.h>

static GHashTable *sound_cache;
static gboolean sdl_inited = FALSE;
static gboolean audio_open = FALSE;
  
static gboolean SoundOpen_SDL(void)
{
  int mix_flags = MIX_INIT_OGG | MIX_INIT_MP3;
  int initted;

  if (sdl_inited) {
    return TRUE;
  }

  if (!g_getenv("SDL_AUDIODRIVER")) {
#ifdef __APPLE__
    g_setenv("SDL_AUDIODRIVER", "coreaudio", TRUE);
#elif defined(__linux__)
    g_setenv("SDL_AUDIODRIVER", "pulseaudio", TRUE);
#endif
  }

  if (!(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO)) {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
      fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
      return FALSE;
    }
  }

  initted = Mix_Init(mix_flags);
  if ((initted & mix_flags) != mix_flags) {
    fprintf(stderr, "Mix_Init warning: %s\n", Mix_GetError());
  }

  if (!audio_open) {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
      fprintf(stderr, "Mix_OpenAudio failed: %s\n", Mix_GetError());
      Mix_Quit();
      SDL_QuitSubSystem(SDL_INIT_AUDIO);
      return FALSE;
    }
    audio_open = TRUE;
  }

  Mix_AllocateChannels(32);

  sound_cache = g_hash_table_new(g_str_hash, g_str_equal);
  sdl_inited = TRUE;
  return TRUE;
}

static void SoundClose_SDL(void)
{
  GHashTableIter iter;
  gpointer key, value;

  if (!sdl_inited) {
    return;
  }

  if (sound_cache) {
    g_hash_table_iter_init(&iter, sound_cache);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
      g_free(key);
      Mix_FreeChunk((Mix_Chunk *)value);
    }
    g_hash_table_destroy(sound_cache);
    sound_cache = NULL;
  }

  if (audio_open) {
    Mix_CloseAudio();
    audio_open = FALSE;
  }
  Mix_Quit();
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
  sdl_inited = FALSE;
}

static void SoundPlay_SDL(const gchar *snd)
{
  Mix_Chunk *chunk;
  int chan_num;

  chunk = g_hash_table_lookup(sound_cache, snd);
  if (!chunk) {
    chunk = Mix_LoadWAV(snd);
    if (!chunk) {
      return;
    }
    g_hash_table_insert(sound_cache, g_strdup(snd), chunk);
  }

  chan_num = Mix_PlayChannel(-1, chunk, 0);
  if (chan_num < 0) {
    int total_channels = Mix_AllocateChannels(-1);
    int i;
    for (i = 0; i < total_channels; i++) {
      if (Mix_Playing(i)) {
        Mix_HaltChannel(i);
        chan_num = Mix_PlayChannel(-1, chunk, 0);
        if (chan_num >= 0) {
          g_message("Recovered playback for %s by halting channel %d", snd, i);
          break;
        }
      }
    }
    if (chan_num < 0) {
      fprintf(stderr, "Mix_PlayChannel failed for %s: %s\n", snd, Mix_GetError());
    }
  }
}

SoundDriver *sound_sdl_init(void)
{
  static SoundDriver driver;

  driver.name = "sdl";
  driver.open = SoundOpen_SDL;
  driver.close = SoundClose_SDL;
  driver.play = SoundPlay_SDL;
  return &driver;
}

#endif /* HAVE_SDL_MIXER */
