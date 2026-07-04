/*
 * Speed Dreams, a free and open source motorsport simulator.
 * Copyright (C) 2025 Xavier Del Campo Romero
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
#include <vector>

void GfuiGallery()
{
	if (!GfuiScreen || !GfuiScreen->gallery)
		return;

	tGfuiGallery *gallery = GfuiScreen->gallery;

	gallery->update();
}

std::vector<const tGfBgImg *> tGfuiGallery::bgs() const
{
	std::vector<const tGfBgImg *> ret;

	for (const auto &elm : elms)
		ret.push_back(&elm.getbg());

	return ret;
}

int tGfuiGallery::update()
{
	for (unsigned i = 0; i < 2; i++)
	{
		tGfuiGalleryElm &elm = elms[i], &other = elms[i ^ 1];

		if (elm.update())
			return -1;
		else if (elm.state == tGfuiGalleryElm::FADEOUT
			&& other.state == tGfuiGalleryElm::WAIT)
		{
			idx ^= 1;
			other.start(elm.file());
		}
	}

	return 0;
}

const std::string &tGfuiGallery::file() const
{
	return elms[idx].file();
}

const std::string &tGfuiGallery::dir() const
{
	return elms[idx].dir();
}

int tGfuiGallery::open()
{
	for (auto &elm : elms)
		if (elm.open())
			return -1;

	return elms->start();
}

int tGfuiGallery::active_idx() const
{
	return idx;
}

tGfuiGallery::tGfuiGallery(const char *dirname) :
	elms{{dirname}, {dirname}},
	idx(0)
{
}
