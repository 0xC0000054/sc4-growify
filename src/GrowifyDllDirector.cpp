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

#include "version.h"
#include "Growify.h"
#include "GrowifyOccupantFilter.h"
#include "Logger.h"
#include "StringViewUtil.h"
#include "cIGZApp.h"
#include "cIGZCheatCodeManager.h"
#include "cIGZCOM.h"
#include "cIGZFrameWork.h"
#include "cIGZMessageServer2.h"
#include "cIGZMessage2Standard.h"
#include "cIGZVariant.h"
#include "cRZAutoRefCount.h"
#include "cRZBaseString.h"
#include "cRZMessage2COMDirector.h"
#include "cISC4App.h"
#include "cISC4City.h"
#include "cISC4Lot.h"
#include "cISC4LotManager.h"
#include "cISC4Occupant.h"
#include "cISC4OccupantManager.h"
#include "cISC4SimGrid.h"
#include "cISC4ZoneManager.h"
#include "cISCProperty.h"
#include "cISCPropertyHolder.h"
#include "GZServPtrs.h"
#include "SC4Rect.h"
#include "StringResourceKey.h"
#include "StringResourceManager.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include <Windows.h>
#include "wil/resource.h"
#include "wil/win32_helpers.h"

static constexpr uint32_t kSC4MessagePostCityInit = 0x26D31EC1;
static constexpr uint32_t kSC4MessagePreCityShutdown = 0x26D31EC2;
static constexpr uint32_t kMessageCheatIssued = 0x230E27AC;

static constexpr uint32_t kGrowifyCheatID = 0x6AABEBA1;

static constexpr uint32_t kGrowifyDirectorID = 0x8E9901E8;

static const uint32_t kOccupantType_Building = 0x278128A0;


static constexpr std::string_view PluginLogFileName = "SC4Growify.log";

using SC4ZoneType = Growify::SC4ZoneType;

namespace
{
	static cIGZString* GetOccupantName(cISC4Occupant* pOccupant)
	{
		cIGZString* name = nullptr;

		cISCPropertyHolder* propertyHolder = pOccupant->AsPropertyHolder();

		constexpr uint32_t kUserVisibleName = 0x8A416A99;

		cISCProperty* userVisibleName = propertyHolder->GetProperty(kUserVisibleName);

		if (userVisibleName)
		{
			const cIGZVariant* propertyValue = userVisibleName->GetPropertyValue();

			if (propertyValue->GetType() == cIGZVariant::Type::Uint32Array
				&& propertyValue->GetCount() == 3)
			{
				const uint32_t* pTGI = propertyValue->RefUint32();

				uint32_t group = pTGI[1];
				uint32_t instance = pTGI[2];

				StringResourceKey key(group, instance);

				StringResourceManager::GetLocalizedString(key, &name);
			}
		}

		return name;
	}

	void PrintLineToDebugOutput(const char* const line)
	{
		OutputDebugStringA(line);
		OutputDebugStringA("\n");
	}

	struct GrowifyIteratorContext
	{
		cISC4LotManager* pLotManager;
		cISC4ZoneManager* pZoneManager;

		SC4ZoneType requestedZoneType;
		int32_t convertedLotCount;
		bool makeHistorical;
	};

	static bool GrowifyBuildingIterator(cISC4Occupant* pOccupant, void* pData)
	{
		GrowifyIteratorContext* pContext = static_cast<GrowifyIteratorContext*>(pData);

		cISC4Lot* pLot = pContext->pLotManager->GetOccupantLot(pOccupant);

		if (pLot)
		{
			SC4ZoneType existingZoneType = static_cast<SC4ZoneType>(pLot->GetZoneType());

			if (existingZoneType == SC4ZoneType::Plopped)
			{
				cISC4SimGrid<int8_t>* pZoneGrid = pContext->pZoneManager->GetZoneGrid();

				SC4Rect<int32_t> lotBounds{};
				pLot->GetBoundingRect(lotBounds);

				for (int32_t x = lotBounds.topLeftX; x <= lotBounds.bottomRightX; x++)
				{
					for (int32_t z = lotBounds.topLeftY; z <= lotBounds.bottomRightY; z++)
					{
						// Update the zone type for each cell that the lot occupies.
						pZoneGrid->SetTractValue(x, z, static_cast<int8_t>(pContext->requestedZoneType));
					}
				}

				// Update the cached zone data for the lot.
				pLot->UpdateZoneType();

#ifdef _DEBUG
				SC4ZoneType updatedZoneType = static_cast<SC4ZoneType>(pLot->GetZoneType());
#endif // _DEBUG

				if (pContext->makeHistorical)
				{
					pLot->SetHistorical(true);
				}

				pContext->convertedLotCount++;
			}
		}

		return true;
	}

	std::filesystem::path GetDllFolderPath()
	{
		wil::unique_cotaskmem_string modulePath = wil::GetModuleFileNameW(wil::GetModuleInstanceHandle());

		std::filesystem::path temp(modulePath.get());

		return temp.parent_path();
	}
}

class GrowifyDllDirector : public cRZMessage2COMDirector
{
public:

	GrowifyDllDirector()
	{
		std::filesystem::path dllFolderPath = GetDllFolderPath();

		std::filesystem::path logFilePath = dllFolderPath;
		logFilePath /= PluginLogFileName;

		Logger& logger = Logger::GetInstance();
		logger.Init(logFilePath, LogLevel::Error);
		logger.WriteLogFileHeader("SC4Growify v" PLUGIN_VERSION_STR);
	}

	uint32_t GetDirectorID() const
	{
		return kGrowifyDirectorID;
	}

	void PostCityInit()
	{
		Logger& logger = Logger::GetInstance();

		cISC4AppPtr pSC4App;

		if (pSC4App)
		{
			cIGZCheatCodeManager* pCheatMgr = pSC4App->GetCheatCodeManager();

			if (pCheatMgr)
			{
				pCheatMgr->AddNotification2(this, 0);
				pCheatMgr->RegisterCheatCode(kGrowifyCheatID, cRZBaseString("Growify"));
			}
			else
			{
				logger.WriteLine(LogLevel::Error, "The cheat manager pointer was null.");
			}
		}
	}

	void PreCityShutdown()
	{
		cISC4AppPtr pSC4App;

		if (pSC4App)
		{
			cIGZCheatCodeManager* pCheatMgr = pSC4App->GetCheatCodeManager();

			if (pCheatMgr)
			{
				pCheatMgr->UnregisterCheatCode(kGrowifyCheatID);
				pCheatMgr->RemoveNotification2(this, 0);
			}
		}
	}

	void ProcessCheat(cIGZMessage2Standard* pStandardMsg)
	{
		uint32_t cheatID = static_cast<uint32_t>(pStandardMsg->GetData1());

		if (cheatID == kGrowifyCheatID)
		{
			cIGZString* cheatStr = static_cast<cIGZString*>(pStandardMsg->GetVoid2());

			Growify::GrowifyData data{};

			if (Growify::ParseCheatString(cheatStr->ToChar(), data))
			{
				cISC4AppPtr pSC4App;

				if (pSC4App)
				{
					cISC4City* pCity = pSC4App->GetCity();

					if (pCity)
					{
						cISC4LotManager* pLotManager = pCity->GetLotManager();
						cISC4OccupantManager* pOccupantManager = pCity->GetOccupantManager();
						cISC4ZoneManager* pZoneManager = pCity->GetZoneManager();

						if (pLotManager && pOccupantManager && pZoneManager)
						{
							GrowifyIteratorContext context{};
							context.pLotManager = pLotManager;
							context.pZoneManager = pZoneManager;
							context.requestedZoneType = data.targetSC4ZoneType;
							context.makeHistorical = data.makeLotHistorical;
							context.convertedLotCount = 0;

							GrowifyOccupantFilter filter(data.zoneType);

							pOccupantManager->IterateOccupants(&GrowifyBuildingIterator, &context, nullptr, nullptr, &filter);

							Growify::ShowConvertedLotCount(data.zoneType, context.convertedLotCount);
						}
					}
				}
			}

		}
	}

	bool DoMessage(cIGZMessage2* pMessage)
	{
		cIGZMessage2Standard* pStandardMsg = static_cast<cIGZMessage2Standard*>(pMessage);

		uint32_t dwType = pMessage->GetType();

		switch (dwType)
		{
		case kMessageCheatIssued:
			ProcessCheat(pStandardMsg);
			break;
		case kSC4MessagePostCityInit:
			PostCityInit();
			break;
		case kSC4MessagePreCityShutdown:
			PreCityShutdown();
			break;
		}

		return true;
	}

	bool OnStart(cIGZCOM* pCOM)
	{
		cIGZFrameWork* const pFramework = pCOM->FrameWork();

		if (pFramework->GetState() < cIGZFrameWork::kStatePreAppInit)
		{
			pFramework->AddHook(this);
		}
		else
		{
			PreAppInit();
		}

		return true;
	}

	bool PostAppInit()
	{
		Logger& logger = Logger::GetInstance();

		cIGZMessageServer2Ptr pMsgServ;

		if (pMsgServ)
		{
			std::vector<uint32_t> requiredNotifications;
			requiredNotifications.push_back(kSC4MessagePostCityInit);
			requiredNotifications.push_back(kSC4MessagePreCityShutdown);

			for (uint32_t messageID : requiredNotifications)
			{
				if (!pMsgServ->AddNotification(this, messageID))
				{
					logger.WriteLine(LogLevel::Error, "Failed to subscribe to the required notifications.");
					return false;
				}
			}
		}
		else
		{
			logger.WriteLine(LogLevel::Error, "Failed to subscribe to the required notifications.");
			return false;
		}

		return true;
	}
};

cRZCOMDllDirector* RZGetCOMDllDirector() {
	static GrowifyDllDirector sDirector;
	return &sDirector;
}