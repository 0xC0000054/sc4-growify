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

#include "Growify.h"
#include "cRZBaseString.h"
#include "SC4NotificationDialog.h"
#include "StringViewUtil.h"
#include <vector>

using namespace std::string_view_literals;

using SC4ZoneType = Growify::SC4ZoneType;
using GrowifyZoneType = Growify::GrowifyZoneType;

namespace
{
	void ShowMessageBox(const char* const message)
	{
		SC4NotificationDialog::ShowDialog(cRZBaseString(message), cRZBaseString("Growify"));
	}

	void ShowUsage()
	{
		ShowMessageBox("Usage: Growify <zone type> <zone density> [make historical - optional, defaults to true]");
	}

	void ShowInvalidZoneTypeErrorMessage()
	{
		ShowMessageBox("The zone type value must be one of: Residential, Commercial, Agriculture or Industrial.");
	}

	void ShowInvalidZoneDensityErrorMessage()
	{
		ShowMessageBox("The zone density value must be one of: Low, Medium or High.");
	}

	void ShowInvalidMakeHistoricalErrorMessage()
	{
		ShowMessageBox("The make historical value must either true or false.");
	}

	enum class GrowifyZoneDensity
	{
		// A placeholder used for the Agriculture zone.
		Invalid,
		Low,
		Medium,
		High
	};

	SC4ZoneType MapGrowifyZoneToSC4Zone(GrowifyZoneType type, GrowifyZoneDensity density)
	{
		SC4ZoneType sc4ZoneType = SC4ZoneType::None;

		if (type == GrowifyZoneType::Residential)
		{
			switch (density)
			{
			case GrowifyZoneDensity::Low:
				sc4ZoneType = SC4ZoneType::ResidentialLowDensity;
				break;
			case GrowifyZoneDensity::Medium:
				sc4ZoneType = SC4ZoneType::ResidentialMediumDensity;
				break;
			case GrowifyZoneDensity::High:
				sc4ZoneType = SC4ZoneType::ResidentialHighDensity;
				break;
			}
		}
		else if (type == GrowifyZoneType::Commercial)
		{
			switch (density)
			{
			case GrowifyZoneDensity::Low:
				sc4ZoneType = SC4ZoneType::CommercialLowDensity;
				break;
			case GrowifyZoneDensity::Medium:
				sc4ZoneType = SC4ZoneType::CommercialMediumDensity;
				break;
			case GrowifyZoneDensity::High:
				sc4ZoneType = SC4ZoneType::CommercialHighDensity;
				break;
			}
		}
		else if (type == GrowifyZoneType::Agriculture)
		{
			sc4ZoneType = SC4ZoneType::Agriculture;
		}
		else if (type == GrowifyZoneType::Industrial)
		{
			switch (density)
			{
			case GrowifyZoneDensity::Medium:
				sc4ZoneType = SC4ZoneType::IndustrialMediumDensity;
				break;
			case GrowifyZoneDensity::High:
				sc4ZoneType = SC4ZoneType::IndustrialHighDensity;
				break;
			}
		}

		return sc4ZoneType;
	}

	bool ParseGrowifyZoneType(
		const std::string_view& zoneType,
		GrowifyZoneType& growifyZoneType)
	{
		bool result = false;

		// We check the first letter of each zone type name, this
		// allows the user to save on typing.
		if (StringViewUtil::StartsWithIgnoreCase(zoneType, "R"sv))
		{
			growifyZoneType = GrowifyZoneType::Residential;
			result = true;
		}
		else if (StringViewUtil::StartsWithIgnoreCase(zoneType, "C"sv))
		{
			growifyZoneType = GrowifyZoneType::Commercial;
			result = true;
		}
		else if (StringViewUtil::StartsWithIgnoreCase(zoneType, "A"sv))
		{
			growifyZoneType = GrowifyZoneType::Agriculture;
			result = true;
		}
		else if (StringViewUtil::StartsWithIgnoreCase(zoneType, "I"sv))
		{
			growifyZoneType = GrowifyZoneType::Industrial;
			result = true;
		}

		return result;
	}

	bool ParseGrowifyZoneDensity(
		const std::string_view& zoneDensity,
		GrowifyZoneDensity& growifyZoneDensity)
	{
		bool result = false;

		// We check the first letter of each density type name, this
		// allows the user to save on typing.
		if (StringViewUtil::StartsWithIgnoreCase(zoneDensity, "L"sv))
		{
			growifyZoneDensity = GrowifyZoneDensity::Low;
			result = true;
		}
		else if (StringViewUtil::StartsWithIgnoreCase(zoneDensity, "M"sv))
		{
			growifyZoneDensity = GrowifyZoneDensity::Medium;
			result = true;
		}
		else if (StringViewUtil::StartsWithIgnoreCase(zoneDensity, "H"sv))
		{
			growifyZoneDensity = GrowifyZoneDensity::High;
			result = true;
		}

		return result;
	}

	bool ParseMakeHistoricalString(const std::string_view& makeHistorical, bool& value)
	{
		bool result = false;

		if (StringViewUtil::EqualsIgnoreCase(makeHistorical, "true"sv))
		{
			value = true;
			result = true;
		}
		else if (StringViewUtil::EqualsIgnoreCase(makeHistorical, "false"sv))
		{
			value = false;
			result = true;
		}
		else if (makeHistorical.size() == 1)
		{
			switch (makeHistorical[0])
			{
			case '0':
				value = false;
				result = true;
				break;
			case '1':
				value = true;
				result = true;
				break;
			}
		}

		return result;
	}
}

bool Growify::ParseCheatString(const std::string_view& cheatString, GrowifyData& data)
{
	data.zoneType = GrowifyZoneType::Invalid;
	data.targetSC4ZoneType = SC4ZoneType::None;
	data.makeLotHistorical = true;

	std::vector<std::string_view> arguments;
	arguments.reserve(4); // A valid cheat will have at most 4 parameters.

	StringViewUtil::Split(cheatString, ' ', arguments);

	if (arguments.size() >= 2 && arguments.size() <= 4)
	{
		if (!ParseGrowifyZoneType(arguments[1], data.zoneType))
		{
			ShowInvalidZoneTypeErrorMessage();
			return false;
		}

		if (arguments.size() >= 3)
		{
			GrowifyZoneDensity zoneDensity = GrowifyZoneDensity::Invalid;

			if (!ParseGrowifyZoneDensity(arguments[2], zoneDensity))
			{
				ShowInvalidZoneDensityErrorMessage();
				return false;
			}

			if (arguments.size() == 4)
			{
				if (!ParseMakeHistoricalString(arguments[3], data.makeLotHistorical))
				{
					ShowInvalidMakeHistoricalErrorMessage();
					return false;
				}
			}

			data.targetSC4ZoneType = MapGrowifyZoneToSC4Zone(data.zoneType, zoneDensity);

			// This should never happen, but check anyway.
			if (data.targetSC4ZoneType == SC4ZoneType::None)
			{
				ShowMessageBox("The zone type and density combination was invalid.");
				return false;
			}

			return true;
		}
		else
		{
			// The Agriculture zone only has 1 density level, so we allow a
			// 2 argument version of the command in that case: Growify Agriculture.
			if (data.zoneType == GrowifyZoneType::Agriculture)
			{
				return true;
			}
		}
	}

	ShowUsage();
	return false;
}

void Growify::ShowConvertedLotCount(GrowifyZoneType zoneType, int32_t count)
{
	const char* zoneTypeStr = nullptr;

	switch (zoneType)
	{
	case Growify::GrowifyZoneType::Residential:
		zoneTypeStr = "residential";
		break;
	case Growify::GrowifyZoneType::Commercial:
		zoneTypeStr = "commercial";
		break;
	case Growify::GrowifyZoneType::Agriculture:
		zoneTypeStr = "agricultural";
		break;
	case Growify::GrowifyZoneType::Industrial:
		zoneTypeStr = "industrial";
		break;
	}

	if (zoneTypeStr)
	{
		char buffer[1024]{};

		std::snprintf(
			buffer,
			sizeof(buffer),
			"Growified %d %s lot(s).",
			count,
			zoneTypeStr);

		ShowMessageBox(buffer);
	}
}
