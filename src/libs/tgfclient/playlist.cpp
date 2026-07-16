#include "playlist.h"
#include "musicplayer.h"
#include <tgf.h>
#include <tgfclient.h>
#include <translation.h>

const char *Playlist::tr(const char *key) const
{
    std::string s = std::string("translatable/") + key;

    return GfuiGetTranslatedText(param, s.c_str(), "text", "");
}

void Playlist::halt()
{
    notifications.clear();
}

void Playlist::push()
{
    const musicTrack &track = getMusicTrack();

    if (track.author.empty())
        return;

    std::string artist = tr("artist"),
        title = tr("title"),
        license = tr("license"),
        trackinfo = "\n";

    trackinfo += artist + ": " + track.author + '\n';
    trackinfo += title + ": " + track.name + '\n';
    trackinfo += license + ": " + track.license;
    notifications.set(trackinfo);
}

void Playlist::release(void *screen)
{
    Playlist &p = self();

    p.notifications.release(screen);
}

void Playlist::update(int ms)
{
    Playlist &p = self();

    p.notifications.updateStatus();
    GfuiApp().eventLoop().setTimerCB(ms, update);
}

Playlist &Playlist::self()
{
    static Playlist playlist;

    if (!playlist.init)
    {
        static const char path[] = "playlist.xml";

        if (!(playlist.param = GfuiMenuLoad(path)))
            GfLogFatal("GfuiMenuLoad %s failed\n", path);

        playlist.init = true;
        GfuiApp().eventLoop().setTimerCB(0, update);
    }

    return playlist;
}

Playlist::~Playlist()
{
    if (param)
        GfParmReleaseHandle(param);
}

Playlist::Playlist() :
    init(false),
    param(nullptr)
{
}
