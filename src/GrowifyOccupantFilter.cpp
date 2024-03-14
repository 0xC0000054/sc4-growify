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

#include "GrowifyOccupantFilter.h"
#include "cIGZVariant.h"
#include "cISC4Occupant.h"
#include "cISCProperty.h"
#include "cISCPropertyHolder.h"

static const uint32_t kOccupantType_Building = 0x278128A0;

namespace
{
	enum class SC4BuildingPurposeType : uint8_t
	{
		None = 0,
		Residence = 1,
		Services = 2,
		Office = 3,
		Tourism = 4,
		Agriculture = 5,
		Processing = 6,
		Manufacturing = 7,
		HighTech = 8,
		Other = 9,
	};

	bool GetBuildingPurposeType(cISCPropertyHolder* pProperties, SC4BuildingPurposeType& purposeType)
	{
		bool result = false;

		if (pProperties)
		{
			constexpr uint32_t kBuildingPurpose = 0x27812833;

			const cISCProperty* property = pProperties->GetProperty(kBuildingPurpose);

			if (property)
			{
				const cIGZVariant* data = property->GetPropertyValue();

				if (data)
				{
					uint16_t type = data->GetType();

					if (type == cIGZVariant::Type::Uint8)
					{
						purposeType = static_cast<SC4BuildingPurposeType>(data->GetValUint8());
						result = true;
					}
				}
			}
		}

		return result;
	}
}

GrowifyOccupantFilter::GrowifyOccupantFilter(Growify::GrowifyZoneType zoneType)
	: refCount(0),
	  requestedZoneType(zoneType)
{
}

bool GrowifyOccupantFilter::QueryInterface(uint32_t riid, void** ppvObj)
{
	if (riid == GZIID_cISC4OccupantFilter)
	{
		*ppvObj = static_cast<cISC4OccupantFilter*>(this);
		AddRef();

		return true;
	}
	else if (riid == GZIID_cIGZUnknown)
	{
		*ppvObj = static_cast<cIGZUnknown*>(this);
		AddRef();

		return true;
	}

	return false;
}

uint32_t GrowifyOccupantFilter::AddRef()
{
	return ++refCount;
}

uint32_t GrowifyOccupantFilter::Release()
{
	if (refCount > 0)
	{
		--refCount;
	}

	return refCount;
}

bool GrowifyOccupantFilter::IsOccupantIncluded(cISC4Occupant* pOccupant)
{
	return IsOccupantTypeIncluded(pOccupant->GetType())
		&& IsCompatibleBuildingPurpose(pOccupant->AsPropertyHolder());
}

bool GrowifyOccupantFilter::IsOccupantTypeIncluded(uint32_t dwType)
{
	return dwType == kOccupantType_Building;
}

bool GrowifyOccupantFilter::IsPropertyHolderIncluded(cISCPropertyHolder* pProperties)
{
	return true;
}

bool GrowifyOccupantFilter::IsCompatibleBuildingPurpose(cISCPropertyHolder* pProperties) const
{
	bool result = false;

	SC4BuildingPurposeType purposeType = SC4BuildingPurposeType::None;

	if (GetBuildingPurposeType(pProperties, purposeType))
	{
		switch (purposeType)
		{
		case SC4BuildingPurposeType::Residence:
			result = requestedZoneType == Growify::GrowifyZoneType::Residential;
			break;
		case SC4BuildingPurposeType::Services:
		case SC4BuildingPurposeType::Office:
			result = requestedZoneType == Growify::GrowifyZoneType::Commercial;
			break;
		case SC4BuildingPurposeType::Agriculture:
			result = requestedZoneType == Growify::GrowifyZoneType::Agriculture;
			break;
		case SC4BuildingPurposeType::Processing:
		case SC4BuildingPurposeType::Manufacturing:
		case SC4BuildingPurposeType::HighTech:
			result = requestedZoneType == Growify::GrowifyZoneType::Industrial;
			break;
		}
	}

	return result;
}
