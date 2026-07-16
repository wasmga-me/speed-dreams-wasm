/*
 * Speed Dreams, a free and open source motorsport simulator.
 * Copyright (C) 2026 Xavier Del Campo Romero
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "musicplayer.h"
#include "playlist.h"
#include <guimenu.h>
#include <portability.h>
#include <sound.h>
#include <tgf.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <cstring>
#include <string>

struct musicplayer
{
    Mix_Music *music;
    bool enabled, opened;
    float volume;
    musicTrack selTrack;
    std::vector<musicTrack> tracks;
    std::string path, defpath, ndefpath;
    int index;
};

static musicplayer mp;

static void onFinished()
{
    mp.path.clear();
    playMusic();
}

static std::vector<musicTrack> getTracks(void *h)
{
    static const char section[] = SND_SCT_MUSIC "/" SND_SCT_TRACKS;
    std::vector<musicTrack> tracks;

    for (int i = 1; i <= GfParmGetEltNb(h, section); i++)
    {
        std::string tracksec = section + std::string("/") + std::to_string(i);
        const char *ctracksec = tracksec.c_str();
        const char *path = GfParmGetStr(h, ctracksec,
            SND_ATT_MUSIC_PATH, nullptr);

        if (!path)
            continue;

        musicTrack track;

        track.path = path;
        track.author = GfParmGetStr(h, ctracksec, SND_ATT_MUSIC_AUTHOR,
            "Unknown artist");
        track.name = GfParmGetStr(h, ctracksec, SND_ATT_MUSIC_NAME,
            "Unknown title");
        track.license = GfParmGetStr(h, ctracksec, SND_ATT_MUSIC_LICENSE,
            "Unknown license");
        tracks.push_back(track);
    }

    if (tracks.empty())
        mp.defpath = GfParmGetStr(h, SND_SCT_MUSIC, SND_ATT_MUSIC_DEFAULT_MUSIC,
            "data/music/main.ogg");

    return tracks;
}

void initMusic()
{
    void *h = nullptr;
    const char *enabled;
    float vol;

    GfLogInfo("Reading music player config\n");

    if (!(h = GfParmReadFileLocal(SND_PARAM_FILE,
        GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT)))
    {
        GfLogError("GfParmReadFileLocal %s failed\n", SND_PARAM_FILE);
        goto end;
    }

    vol = GfParmGetNum(h, SND_SCT_MUSIC, SND_ATT_MUSIC_VOLUME, "%", 100.0f);

    if (vol > 100.0f)
        vol = 100.0f;
    else if (vol < 0)
        vol = 0;

    mp.volume = vol;
    enabled = GfParmGetStr(h, SND_SCT_MUSIC, SND_ATT_MUSIC_STATE, "");
    mp.enabled = !strcmp(enabled, SND_VAL_MUSIC_STATE_ENABLED);
    mp.tracks = getTracks(h);

    if (mp.enabled && Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048))
    {
        GfLogError("Mix_OpenAudio failed: %s\n", SDL_GetError());
        goto end;
    }

    mp.index = -1;

end:
    if (h)
        GfParmReleaseHandle(h);
}

void fadeOutDefaultMusic(unsigned ms)
{
    mp.path.clear();
    Mix_HookMusicFinished(nullptr);
    Mix_FadeOutMusic(ms);
}

void shutdownMusic()
{
    if (mp.music)
        fadeOutDefaultMusic(1000);

    while (Mix_PlayingMusic())
        GfSleep(0.1);

    if (mp.opened)
        Mix_CloseAudio();
}

void enableMusic(bool enable)
{
    if (isMusicEnabled() && !enable)
        shutdownMusic();
    else if (enable)
        initMusic();

    mp.enabled = enable;
}

void fadeInDefaultMusic()
{
    Playlist &playlist = Playlist::self();
    int loop;

    if (!mp.music)
        return;

    loop = mp.path.empty() ? -1 : 0;
    Mix_FadeInMusic(mp.music, loop, 1000);
    Mix_HookMusicFinished(onFinished);
    playlist.push();
}

void setMusicVolume(float volume)
{
    Mix_VolumeMusic(mp.volume = volume);
    GfLogInfo("Music volume set to %.2f\n", volume);
}

float getMusicVolume()
{
    return mp.volume;
}

static const char *getDefault()
{
    const char *ret = nullptr;
    void *h = nullptr;

    h = GfParmReadFileLocal(SND_PARAM_FILE, GFPARM_RMODE_REREAD
        | GFPARM_RMODE_CREAT);

    if (!h)
    {
        GfLogError("GfParmReadFileLocal %s failed\n", SND_PARAM_FILE);
        goto end;
    }

    if (mp.tracks.empty())
        ret = mp.defpath.c_str();
    else
    {
        unsigned index;

        do
        {
            if (portability::rand(&index, sizeof index))
            {
                GfLogError("Failed to calculate random index\n");
                index = 0;
            }

            index %= mp.tracks.size();
        } while (mp.index >= 0 && (unsigned)mp.index == index);

        mp.index = index;
        mp.selTrack = mp.tracks.at(index);
        ret = mp.selTrack.path.c_str();
    }

    mp.path = ret;
    mp.ndefpath.clear();

end:
    if (h)
        GfParmReleaseHandle(h);

    return ret;
}

void playMusic(const char* filename)
{
    if (!mp.enabled)
        return;
    else if (filename)
    {
        mp.path.clear();
        Mix_HookMusicFinished(nullptr);

        if (!strcmp(filename, "None"))
        {
            Playlist &playlist = Playlist::self();

            Mix_HaltMusic();
            playlist.halt();
            return;
        }
        else if (filename == mp.ndefpath)
            return;

        mp.ndefpath = filename;
    }
    else if (mp.path.empty())
        filename = getDefault();
    else
        return;

    if (mp.music)
        Mix_FreeMusic(mp.music);

    if (!(mp.music =  Mix_LoadMUS(filename)))
    {
        GfLogError("Mix_LoadMUS %s failed: %s\n", filename, SDL_GetError());
        return;
    }

    GfLogInfo("Music changing to: %s \n", filename);
    fadeInDefaultMusic();
}

const musicTrack &getMusicTrack()
{
    return mp.selTrack;
}

void setMusicTrack(void *h)
{
    musicTrack track;
    const char *sec = GFMNU_SECT_MUSIC,
        *path = GfParmGetStr(h, sec, GFMNU_ATTR_MUSIC_FILE, nullptr),
        *artist = GfParmGetStr(h, sec, SND_ATT_MUSIC_AUTHOR, nullptr),
        *name = GfParmGetStr(h, sec, SND_ATT_MUSIC_NAME, nullptr),
        *lic = GfParmGetStr(h, sec, SND_ATT_MUSIC_LICENSE, nullptr);

    if (!path || !artist || !name || !lic)
        goto end;

    track.path = path;
    track.author = artist;
    track.name = name;
    track.license = lic;
end:
    mp.selTrack = track;
}

bool isMusicEnabled()
{
    return mp.enabled;
}
