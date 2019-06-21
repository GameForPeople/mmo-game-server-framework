#include "stdafx.h"

#include "BaseModel.h"
#include "BaseActor.h"

BaseActor::BaseActor(BaseModel* const inBaseModel) noexcept
	: renderComponent(inBaseModel), renderData(), isRender(true)
{
	assert(inBaseModel != nullptr, "객체 초기화에서, nullptr인 RenderModel이 전달되었습니다.");
}

BaseActor::BaseActor(BaseModel* const inBaseModel, const RenderData& inRenderData, bool inIsRender /* = true*/) noexcept
	: renderComponent(inBaseModel), renderData(new RenderData(inRenderData)), isRender(inIsRender)
{
	assert(inBaseModel != nullptr, "객체 초기화에서, nullptr인 RenderModel이 전달되었습니다.");
}

BaseActor::~BaseActor()
{
}

void BaseActor::Render(HDC pHDC)
{
	if(isRender) renderComponent->Render(pHDC, renderData);
}