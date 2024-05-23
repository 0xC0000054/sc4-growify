////////////////////////////////////////////////////////////////////////////
//
// This file is part of sc4-growify, a DLL Plugin for SimCity 4 that adds
// a cheat code to convert plopped buildings to a growable RCI zone type.
//
// Copyright (c) 2024 Nicholas Hayes
//
// This file is licensed under terms of the MIT License.
// See LICENSE.txt for more information.
//
////////////////////////////////////////////////////////////////////////////

#pragma once

class cISC4Occupant;

namespace DebugUtil
{
	void PrintLineToDebugOutput(const char* const line);
	void PrintLineToDebugOutputFormatted(const char* const format, ...);

	void PrintOccupantNameToDebugOutput(cISC4Occupant* pOccupant);
}