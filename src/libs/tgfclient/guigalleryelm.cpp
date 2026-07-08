/*
 * Speed Dreams, a free and open source motorsport simulator.
 * Copyright (C) 2026 Xavier Del Campo Romero
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "gui.h"
#include "tgfclient.h"
#include <portability.h>
#include <tgfclient.h>
#include <tgf.h>
#include <string>

const std::string &tGfuiGalleryElm::dir() const
{
	return dirname;
}

const std::string &tGfuiGalleryElm::file() const
{
	return fullpath;
}

const tGfBgImg &tGfuiGalleryElm::getbg() const
{
	return bg;
}

int tGfuiGalleryElm::open()
{
	const char *cdirname = dirname.c_str();
	tFList *list = GfDirGetList(cdirname);
	const tFList *entry = list;

	if (!list)
	{
		GfLogError("%s: GfDirGetList failed\n", cdirname);
		return -1;
	}

	do
	{
		if (entry->type != FList::file)
			continue;

		std::string fullpath = dirname + "/" + entry->name;
		const char *cfullpath = fullpath.c_str();

		if (GfTexTryPNGImage(cfullpath) && GfTexTryJPEGImage(cfullpath))
			continue;

		files.push_back(entry->name);
	} while ((entry = entry->next) != list);

	GfDirFreeList(list, NULL, true, true);

	if (files.empty())
	{
		GfLogWarning("%s contains no images\n", cdirname);
		return -1;
	}

	return 0;
}

int tGfuiGalleryElm::init_direction()
{
	unsigned direction;

	if (portability::rand(&direction, sizeof direction))
	{
		GfLogError("portability::rand failed\n");
		return -1;
	}

	direction %= tGfuiGalleryElm::N_DIRECTIONS;

	switch (direction)
	{
		case DIR_RIGHT:
			xoffset = overscan;
			yoffset = overscan / 2.0f;
			break;

		case DIR_LEFT:
			xoffset = 0;
			yoffset = overscan / 2.0f;
			break;

		case DIR_DOWN:
			xoffset = overscan / 2.0f;
			yoffset = 0;
			break;

		case DIR_UP:
			xoffset = overscan / 2.0f;
			yoffset = overscan;
			break;
	}

	this->direction = direction;
	return 0;
}

int tGfuiGalleryElm::wait()
{
	return 0;
}

int tGfuiGalleryElm::init()
{
	size_t index;
	std::string newpath;

	do
	{
		if (portability::rand(&index, sizeof index))
		{
			GfLogError("portability::rand failed\n");
			return -1;
		}

		index %= files.size();
		newpath = dirname + "/" + files.at(index);
	} while (newpath == fullpath || newpath == prev);

	fullpath = newpath;
	GfuiSetBgImg(bg, fullpath.c_str());
	bg.colors[0] = bg.colors[1] = bg.colors[2] = 0.6;
	bg.colors[3] = alpha = 0;
	state = FADEIN;

	if (init_direction())
		return -1;

	return 0;
}

int tGfuiGalleryElm::fadein()
{
	if ((alpha += 0.01f) >= 1.0f)
	{
		time = GfTimeClock();
		alpha = 1.0f;
		state = SHOW;
	}

	return 0;
}

int tGfuiGalleryElm::show()
{
	const double seconds = 8.0f;

	if (GfTimeClock() - time >= seconds)
		state = FADEOUT;

	return 0;
}

int tGfuiGalleryElm::fadeout()
{
	if ((alpha -= 0.01f) <= 0)
	{
		alpha = 0;
		state = WAIT;
	}

	return 0;
}

void tGfuiGalleryElm::move()
{
	const float speed = 0.0375f;

	switch (direction)
	{
		case DIR_LEFT:
			if ((xoffset += speed) >= overscan)
				xoffset = overscan;
			break;

		case DIR_RIGHT:
			if ((xoffset -= speed) <= 0)
				xoffset = 0;
			break;

		case DIR_UP:
			if ((yoffset -= speed) <= 0)
				yoffset = 0;
			break;

		case DIR_DOWN:
			if ((yoffset += speed) >= overscan)
				yoffset = overscan;
			break;
	}

	bg.xoffset = xoffset;
	bg.yoffset = yoffset;
	bg.overscan = overscan;
}

int tGfuiGalleryElm::start(const std::string &prev)
{
	state = INIT;
	this->prev = prev;
	return 0;
}

int tGfuiGalleryElm::update()
{
#define INVOKE(x, y)  ((*x).*(y))
	typedef int (tGfuiGalleryElm::*fn)();

	static const fn functions[] =
	{
		&tGfuiGalleryElm::wait,
		&tGfuiGalleryElm::init,
		&tGfuiGalleryElm::fadein,
		&tGfuiGalleryElm::show,
		&tGfuiGalleryElm::fadeout
	};

	if (INVOKE(this, functions[state])())
		return -1;

	move();
	bg.colors[3] = alpha;
	return 0;
}

tGfuiGalleryElm::tGfuiGalleryElm(const char *dirname) :
	state(WAIT),
	bg{{0}},
	direction(0),
	dirname(dirname),
	overscan(30.0f),
	alpha(0),
	xoffset(0),
	yoffset(0),
	time(0)
{
}
