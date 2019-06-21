#pragma once

class StretchModel final : public BaseModel
{
public:
	StretchModel(const std::wstring& inResourcePath);
	virtual ~StretchModel() override final = default;

public:
	virtual void Render(HDC pHDC, RenderData* inPoint) override final;
};