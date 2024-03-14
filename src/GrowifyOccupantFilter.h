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
#include "cISC4OccupantFilter.h"
#include "Growify.h"

class GrowifyOccupantFilter final : public cISC4OccupantFilter
{
public:

	GrowifyOccupantFilter(Growify::GrowifyZoneType zoneType);

private:

	bool QueryInterface(uint32_t riid, void** ppvObj) override;

	uint32_t AddRef() override;

	uint32_t Release() override;

	bool IsOccupantIncluded(cISC4Occupant* pOccupant) override;

	bool IsOccupantTypeIncluded(uint32_t dwType) override;

	bool IsPropertyHolderIncluded(cISCPropertyHolder* pProperties) override;

	bool IsCompatibleBuildingPurpose(cISCPropertyHolder* pProperties) const;

	uint32_t refCount;
	Growify::GrowifyZoneType requestedZoneType;
};

