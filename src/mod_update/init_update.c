/*-
 * Copyright (c) 2000 SoG Development Team
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: init_update.c,v 1.20 2003-10-10 16:15:07 fjoe Exp $
 */

#include <stdio.h>
#include <dlfcn.h>

#include <merc.h>
#include <module.h>

#include <sog.h>

DECLARE_MODINIT_FUN(_module_load);
DECLARE_MODINIT_FUN(_module_unload);
DECLARE_MODINIT_FUN(_module_boot);

MODINIT_FUN(_module_load, m)
{
	uhandler_mod_load(m);
	return 0;
}

MODINIT_FUN(_module_unload, m)
{
	uhandler_mod_unload(m);
	return 0;
}

MODINIT_FUN(_module_boot, m)
{
	int hour, day, month;

	/*
	 * Set time and weather.
	 */
	hour = (current_time - 650336715) / (get_pulse("char") / PULSE_PER_SCD);
	time_info.hour	= hour  % 24;
	day		= hour  / 24;
	time_info.day	= day   % 35;
	month		= day   / 35;
	time_info.month	= month % 17;
	time_info.year	= month / 17;

	     if (time_info.hour <  5) weather_info.sunlight = SUN_DARK;
	else if (time_info.hour <  6) weather_info.sunlight = SUN_RISE;
	else if (time_info.hour < 19) weather_info.sunlight = SUN_LIGHT;
	else if (time_info.hour < 20) weather_info.sunlight = SUN_SET;
	else                          weather_info.sunlight = SUN_DARK;

	weather_info.change	= 0;
	weather_info.mmhg	= 960;
	if (time_info.month >= 7 && time_info.month <= 12)
		weather_info.mmhg += number_range(1, 50);
	else
		weather_info.mmhg += number_range(1, 80);

	     if (weather_info.mmhg <=  980) weather_info.sky = SKY_LIGHTNING;
	else if (weather_info.mmhg <= 1000) weather_info.sky = SKY_RAINING;
	else if (weather_info.mmhg <= 1020) weather_info.sky = SKY_CLOUDY;
	else                                weather_info.sky = SKY_CLOUDLESS;

	scan_pfiles();
	update_one_handler("area");

	return 0;
}
