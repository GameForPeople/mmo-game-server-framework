#include "stdafx.h"
#include "ClientDefine.h"

#include "BaseModel.h"
#include "BitModel.h"
#include "StretchModel.h"
#include "TransparentModel.h"

#include "RenderModelManager.h"

RenderModelManager::RenderModelManager()
{
	const BYTE renderModelCount = static_cast<BYTE>(RENDER_MODEL_TYPE::ENUM_SIZE);
	renderModelCont.reserve(renderModelCount);

	// 조금 느려도 안전하게...
	for (int i = 0; i < renderModelCount; ++i) { renderModelCont.emplace_back(nullptr); }
	
	renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::PLAYER_KNIGHT)] = new TransparentModel(L"Resource/Image/Image_PlayerCharacter_MyKnight.png");
	renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::PLAYER_ARCHER)] = new TransparentModel(L"Resource/Image/Image_PlayerCharacter_MyArcher.png");
	renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::PLAYER_WITCH)] = new TransparentModel(L"Resource/Image/Image_PlayerCharacter_MyWitcher.png");

	renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::OTHER_PLAYER_KNIGHT)] = new TransparentModel(L"Resource/Image/Image_PlayerCharacter_Knight.png");
	renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::OTHER_PLAYER_ARCHER)] = new TransparentModel(L"Resource/Image/Image_PlayerCharacter_Archer.png");
	renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::OTHER_PLAYER_WITCH)] = new TransparentModel(L"Resource/Image/Image_PlayerCharacter_Witch.png");

	renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::MONSTER_SLIME)] = new TransparentModel(L"Resource/Image/Image_Monster_Slime.png");
	renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::MONSTER_GOLME)] = new TransparentModel(L"Resource/Image/Image_Monster_Golem.png");
	renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::MONSTER_DRAGON)] = new TransparentModel(L"Resource/Image/Image_Monster_Dragon.png");

	renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::BACKGROUND_0)] = new StretchModel(L"Resource/Image/30X30_Background_0.png");
	renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::BACKGROUND_1)] = new StretchModel(L"Resource/Image/30X30_Background_1.png");
	renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::BACKGROUND_2)] = new StretchModel(L"Resource/Image/30X30_Background_2.png");
	renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::BACKGROUND_3)] = new StretchModel(L"Resource/Image/30X30_Background_3.png");
	renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::BACKGROUND_4)] = new StretchModel(L"Resource/Image/30X30_Background_4.png");

	//renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::NUMBER_0)] = new TransparentModel(L"Resource/Image/Combo_Orange_Yellow_0.png");
	//renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::NUMBER_1)] = new TransparentModel(L"Resource/Image/Combo_Orange_Yellow_1.png");
	//renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::NUMBER_2)] = new TransparentModel(L"Resource/Image/Combo_Orange_Yellow_2.png");
	//renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::NUMBER_3)] = new TransparentModel(L"Resource/Image/Combo_Orange_Yellow_3.png");
	//renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::NUMBER_4)] = new TransparentModel(L"Resource/Image/Combo_Orange_Yellow_4.png");
	//renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::NUMBER_5)] = new TransparentModel(L"Resource/Image/Combo_Orange_Yellow_5.png");
	//renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::NUMBER_6)] = new TransparentModel(L"Resource/Image/Combo_Orange_Yellow_6.png");
	//renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::NUMBER_7)] = new TransparentModel(L"Resource/Image/Combo_Orange_Yellow_7.png");
	//renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::NUMBER_8)] = new TransparentModel(L"Resource/Image/Combo_Orange_Yellow_8.png");
	//renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::NUMBER_9)] = new TransparentModel(L"Resource/Image/Combo_Orange_Yellow_9.png");

	renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::COVER_UI)] = new StretchModel(L"Resource/Image/Image_Cover_New.png");
	renderModelCont[static_cast<BYTE>(RENDER_MODEL_TYPE::BROADCAST_UI)] = new TransparentModel(L"Resource/Image/Image_BroadcastArea_800X800.png");

	assert(
		[/* void */](const std::vector<BaseModel*>& inRenderModelCont) noexcept -> bool 
		{
			if (inRenderModelCont.size() != static_cast<BYTE>(RENDER_MODEL_TYPE::ENUM_SIZE))
				return false;

			for (const auto& pRenderModel : inRenderModelCont)
			{
				if (pRenderModel == nullptr)
					return false;
			}

			return true;
		}(renderModelCont),
		"생성되지 않은 RenderModel이 있습니다. 확인해주세요. 클라이언트를 종료합니다."
	);
}

RenderModelManager::~RenderModelManager()
{
	for (auto pRenderModel : renderModelCont)
	{
		delete pRenderModel;
	}
}

BaseModel* RenderModelManager::GetRenderModel(RENDER_MODEL_TYPE inRenderModelType)
{
	return renderModelCont[static_cast<BYTE>(inRenderModelType)];
}