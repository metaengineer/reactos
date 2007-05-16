#ifndef _DDRAWTESTLIST_H
#define _DDRAWTESTLIST_H

#include "ddrawtest.h"
#include "debug.cpp"

/* include the tests */
#include "tests/CreateDDraw.cpp"
#include "tests/DisplayModes.cpp"
#include "tests/CreateSurface.cpp"

/* The List of tests */
TEST TestList[] =
{
	{ "DirectDrawCreate(Ex)", Test_CreateDDraw },
	{ "IDirectDraw::SetCooperativeLevel", Test_SetCooperativeLevel },
	{ "IDirectDraw::EnumDisplayModes/SetDisplayMode", Test_DisplayModes },
	{ "IDirectDraw::CreateSurface", Test_CreateSurface },
	{ "IDirectDraw::GetMonitorFrequency", Test_GetMonitorFrequency },
};

/* The function that gives us the number of tests */
extern "C" INT NumTests(void)
{
	return sizeof(TestList) / sizeof(TEST);
}

#endif /* _DDRAWTESTLIST_H */

/* EOF */
