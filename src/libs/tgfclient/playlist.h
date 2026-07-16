#ifndef PLAYLIST_H
#define PLAYLIST_H

#include "notification.h"
#include <tgfclient.h>
#include <SDL_thread.h>

class TGFCLIENT_API Playlist
{
public:
    static Playlist &self();
    static void update(int ms);
    static void release(void *screen);
    void push();
    void halt();

private:
    Playlist();
    ~Playlist();
    const char *tr(const char *key) const;

    NotificationManager notifications;
    bool init;
    void *param;
};

#endif
