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
#include <string_view>

namespace Growify
{
	enum class GrowifyZoneType
	{
		Invalid,
		// Supports Low, Medium and High density.
		Residential,
		// Supports Low, Medium and High density.
		Commercial,
		// Shown as industrial resource (IR) or I-Ag in-game.
		// Density is ignored.
		Agriculture,
		// Supports Medium and High density.
		// Low density is used for agriculture.
		Industrial
	};

	enum class SC4ZoneType : uint32_t
	{
		None = 0,
		ResidentialLowDensity = 1,
		ResidentialMediumDensity = 2,
		ResidentialHighDensity = 3,
		CommercialLowDensity = 4,
		CommercialMediumDensity = 5,
		CommercialHighDensity = 6,
		// Agriculture is the low density industrial zone.
		Agriculture = 7,
		IndustrialMediumDensity = 8,
		IndustrialHighDensity = 9,
		Military = 10,
		Airport = 11,
		Seaport = 12,
		Spaceport = 13,
		Landfill = 14,
		Plopped = 15,
	};

	struct GrowifyData
	{
		GrowifyZoneType zoneType;
		SC4ZoneType targetSC4ZoneType;
		bool makeLotHistorical;
	};

	bool ParseCheatString(const std::string_view& cheatString, GrowifyData& data);

	void ShowConvertedLotCount(GrowifyZoneType zoneType, int32_t count);
}