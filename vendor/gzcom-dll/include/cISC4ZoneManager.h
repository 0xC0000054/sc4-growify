#pragma once
#include "cIGZUnknown.h"

class cGZPersistResourceKey;
template <typename T> class cISC4SimGrid;
template <typename T> class SC4CellRegion;

class cISC4ZoneManager : public cIGZUnknown
{
public:

	virtual bool Init() = 0;
	virtual bool Shutdown() = 0;

	virtual cISC4SimGrid<int8_t>* GetZoneGrid() const = 0;
	virtual uint32_t GetZoneCount(int32_t zoneType) = 0;

	virtual uint32_t GetUndevelopedTileCount() = 0;
	virtual uint32_t GetUndevelopedTileCount(int32_t zoneType) = 0;

	virtual intptr_t GetDevelopmentFailureCount(int32_t unknown1, int32_t unknown2) = 0;
	virtual uint32_t GetAbandonedTileCount() = 0;
	virtual uint32_t PlaceZone(SC4CellRegion<long>& cellRegion, int32_t zoneType, bool unknown3, bool unknown4, bool unknown5, bool unknown6, bool unknown7, int64_t* unknown8, int32_t* unknown9, intptr_t unknown10) = 0;

	virtual int32_t GetZoneType(int32_t x, int32_t z) = 0;
	virtual bool IsRCIZone(int32_t zoneType) = 0;

	virtual bool GetTextureForZone(int32_t zoneType, cGZPersistResourceKey& textureKey) = 0;
	virtual uint32_t GetZoneDragColor(int32_t zoneType) const = 0;

	virtual bool GetZoneDisplayAlpha() const = 0;
	virtual bool GetDefaultZoneDisplayAlpha() const = 0;
	virtual void SetZoneDisplayAlpha(bool value) = 0;

	virtual uint32_t GetMinZoneSize(int32_t zoneType) const = 0;
	virtual uint32_t GetMaxZoneSize(int32_t zoneType) const = 0;
};