#pragma once

class BaseModel;
struct RenderData;

class BaseActor {
public:
	BaseActor(BaseModel* const inBaseModel) noexcept;
	BaseActor(BaseModel* const inBaseModel, const RenderData& inRenderData, const bool inIsRender = true) noexcept;
	virtual ~BaseActor();

public:
	virtual void Render(HDC pHDC);
	inline void SetRender(const bool inIsRender) noexcept {	isRender = inIsRender; };

protected:
	BaseModel* renderComponent;
	RenderData* renderData;

	bool isRender;
};