#pragma once

#include "BaseActor.h"

class BaseModel;

class Pawn : public BaseActor
{
public:
	Pawn(BaseModel* const inBaseModel, const RenderData& inRenderData,
	const USHORT inXPosition = 0, const USHORT inYPosition = 0);
	virtual ~Pawn() override;

public:
	virtual void Render(HDC pHDC) override;

	//void MoveWithDirection(DIRECTION inDirection); // only Client
	void SetPosition(const std::pair<USHORT, USHORT>, const std::pair<USHORT, USHORT>);

	
	void SetOnlyActorPositionNotUpdateRenderData(const std::pair<USHORT, USHORT>);	// 이름 일부러 길게. (플레이어만 사용하는 함수)
	void UpdateRenderData(const std::pair<USHORT, USHORT>);
	inline std::pair<USHORT, USHORT> GetPosition() noexcept { return std::make_pair(xPosition, yPosition); }

private:
	USHORT xPosition;
	USHORT yPosition;

	//static constexpr UINT8 BLOCK_MIN_POSITION = 0;
	//static constexpr UINT8 BLOCK_MAX_POSITION = 7;
};