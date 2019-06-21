#pragma once

class BitModel final : public BaseModel
{
public:
	BitModel(const std::wstring& inResourcePath);
	virtual ~BitModel() override final = default;

public:
	virtual void Render(HDC pHDC, RenderData* inPoint) override final;
};