/*
 * Speed Dreams, a free and open source motorsport simulator.
 * Copyright (C) 2026 Anri Dellal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "playerconfig.h"
#include "serverconfig.h"
#include <tgfclient.h>
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

static const char path[] = "config/webserver.xml",
	section[] = "WebServer Settings";

static void *ScrHandle = NULL;
static void *PrevMenu = NULL;

static std::vector<std::string> servers;

static int defaultServer, defaultServerLabelId, list, url;

static void
onActivate(void * /* dummy */)
{

}

static void
onQuit(void *prevMenu)
{
	servers_set();
	servers.clear();
	GfuiScreenRelease(ScrHandle);
	GfuiScreenActivate(prevMenu);
}

static void
onAdd(void *args)
{
	const char *s = GfuiEditboxGetString(ScrHandle, url);
	char *dup = NULL;
	size_t n;
	int el;

	if (!s)
	{
		GfLogError("GfuiEditboxGetString failed\n");
		goto failure;
	}
	else if (!*s)
		goto failure;

	for (const auto &server : servers)
		if (server == s)
		{
			GfLogWarning("Server %s already added\n", server.c_str());
			goto failure;
		}

	n = strlen(s) + 1;

	if (!(dup = static_cast<char *>(malloc(n))))
	{
		GfLogError("malloc(3): %s\n", strerror(errno));
		goto failure;
	}

	memcpy(dup, s, n);

	if ((el = GfuiScrollListGetNumberOfElements(ScrHandle, list)) < 0)
	{
		GfLogError("GfuiScrollListGetNumberOfElements failed\n");
		return;
	}
	else if (GfuiScrollListInsertElement(ScrHandle, list, dup, el, NULL))
	{
		GfLogError("GfuiScrollListInsertElement %s failed\n", dup);
		return;
	}

	GfuiEditboxSetString(ScrHandle, url, "");
	servers.push_back(dup);
	return;

failure:
	free(dup);
}

static void
onDelete(void * /*dummy*/)
{
	void *args;
	const char *s = GfuiScrollListGetSelectedElement(ScrHandle, list, &args);

	if (!s)
	{
		GfLogError("GfuiScrollListExtractSelectedElement failed\n");
		return;
	}

	int selected = GfuiScrollListGetSelectedElementIndex(ScrHandle, list);
	if (selected == (defaultServer - 1))
		return;  /* do not remove default server */
	else if (selected < (defaultServer - 1))
		defaultServer--;

	GfuiScrollListExtractSelectedElement(ScrHandle, list, &args);

	for (auto server = servers.begin(); server != servers.end(); server++)
	{
		if (*server == s)
		{
			servers.erase(server);
			break;
		}
	}
}

static void
onSetDefault(void * /*dummy*/)
{
	void *args;
	const char *s = GfuiScrollListGetSelectedElement(ScrHandle, list, &args);

	if (!s) {
		return;
	}
	
	defaultServer = GfuiScrollListGetSelectedElementIndex(ScrHandle, list) + 1;
	GfuiLabelSetText(ScrHandle, defaultServerLabelId, s);
}

static int servers_get()
{
	char buf[4];
	int ret = -1;
	void *f = GfParmReadFileLocal(path, GFPARM_RMODE_REREAD);

	if (!f)
	{
		GfLogError("%s: GfParmReadFileLocal failed\n", path);
		goto end;
	}

	if (!GfParmExistsSection(f, "WebServer Settings/default"))
	{
		GfLogError("%s: default section is missing\n", path);
		goto end;
	}

	defaultServer = std::atoi(GfParmGetStr(f, "WebServer Settings/default", "index", NULL));

	sprintf(buf, "%d", defaultServer);

	GfParmListSeekFirst(f, section);
	do
	{
		const char *url = GfParmGetCurStr(f, section, "url", NULL);
		if (!url)
		{
			GfLogError("%s: GfParmGetStr url failed at section %s\n", path,
				GfParmListGetCurEltName(f, section));
			goto end;
		}

		if (strcmp(buf, GfParmListGetCurEltName(f, section)) == 0)
			GfuiLabelSetText(ScrHandle, defaultServerLabelId, url);

		for (const auto &server : servers)
			if (server == url)
			{
				GfLogError("%s: Found duplicate URL %s failed\n", path, url);
				goto end;
			}

		servers.push_back(url);
	}
	while (GfParmListSeekNext(f, section) == 0
			&& strcmp(GfParmListGetCurEltName(f, section), "default") != 0);

	ret = 0;
end:
	if (f)
		GfParmReleaseHandle(f);

	return ret;
}

int servers_set()
{
	int ret = -1;
	char buf[256];
	char buf2[4];
	enum {flags = GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT};
	void *f = GfParmReadFileLocal(path, flags);

	if (!f)
	{
		GfLogError("%s: GfParmReadFileLocal failed\n", path);
		goto end;
	}
	GfParmListClean(f, section);
	for (size_t i = 1; i <= servers.size(); i++)
	{
		const std::string &u = servers.at(i-1);
		sprintf(buf, "%s/%ld", section, i);

		if (GfParmSetStr(f, buf, "url", u.c_str()))
		{
			GfLogError("%s: GfParmSetStr %zu failed\n", path, i);
			goto end;
		}

		if (defaultServer >= 0 && i == (size_t)defaultServer)
			if (GfParmSetStr(f, section, "url", u.c_str()))
			{
				GfLogError("%s: GfParmSetStr %s failed\n", path, u.c_str());
				goto end;
			}
	}


	sprintf(buf, "%s/%s", section, "default");
	sprintf(buf2, "%d", defaultServer);
	if (GfParmSetStr(f, buf, "index", buf2))
	{
		GfLogError("%s: GfParmSetStr default failed\n", path);
		goto end;
	}

	GfParmWriteFile(NULL, f, "webserver");
	ret = 0;

end:
	if (f)
		GfParmReleaseHandle(f);

	return ret;
}

void *
ServerConfigInit(void *prevMenu)
{
	PrevMenu = prevMenu;

	ScrHandle = GfuiScreenCreate((float*)NULL, NULL, onActivate, NULL, (tfuiCallback)NULL, 1);

	void *param = GfuiMenuLoad("serverconfig.xml");
	GfuiMenuCreateStaticControls(ScrHandle, param);

	GfuiMenuDefaultKeysAdd(ScrHandle);

	GfuiMenuCreateButtonControl(ScrHandle,param,"back",PrevMenu,onQuit);
	GfuiAddKey(ScrHandle, GFUIK_ESCAPE, "Cancel", PrevMenu, onQuit, NULL);

	list = GfuiMenuCreateScrollListControl(ScrHandle, param, "servers", NULL, NULL);
	url = GfuiMenuCreateEditControl(ScrHandle, param, "url", NULL, NULL, NULL);

	GfuiMenuCreateButtonControl(ScrHandle, param, "add", NULL, onAdd);
	GfuiMenuCreateButtonControl(ScrHandle, param, "delete", NULL, onDelete);
	GfuiMenuCreateButtonControl(ScrHandle, param, "setdefault", NULL, onSetDefault);
	defaultServerLabelId = GfuiMenuCreateLabelControl(ScrHandle, param, "default");

	GfParmReleaseHandle(param);

	if (servers_get())
		throw std::runtime_error("servers_get failed");

	for (const auto &server : servers)
	{
		int n = GfuiScrollListGetNumberOfElements(ScrHandle, list);

		if (n < 0)
			throw std::runtime_error("GfuiScrollListGetNumberOfElements failed");
		else if (GfuiScrollListInsertElement(ScrHandle, list, server.c_str(), n, NULL))
			throw std::runtime_error("GfuiScrollListInsertElement failed");
	}

	return ScrHandle;
}
