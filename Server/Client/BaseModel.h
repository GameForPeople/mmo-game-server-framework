#pragma once

struct RenderData
{
	/*UINT16*/ int xPosition;
	/*UINT16*/ int yPosition;
	UINT16 xSize;
	UINT16 ySize;
	UINT32 rgbColor;

	RenderData() noexcept = delete;
	~RenderData() = default;

	RenderData(const int inXPosition = 0, const int inYPosition = 0) noexcept
		: xPosition(inXPosition), yPosition(inYPosition), xSize(), ySize(), rgbColor()
	{};

	RenderData(const int inXPosition, const int inYPosition,
		const UINT16 inXSize, const UINT16 inYSize) noexcept
		: xPosition(inXPosition), yPosition(inYPosition), xSize(inXSize), ySize(inYSize), rgbColor()
	{};

	RenderData(const int inXPosition, const int inYPosition,
		const UINT16 inXSize, const UINT16 inYSize, const UINT32 inRgbColor) noexcept
		: xPosition(inXPosition), yPosition(inYPosition), xSize(inXSize), ySize(inYSize), rgbColor(inRgbColor)
	{};
};

__interface InterfaceBaseModel 
{
public:
	virtual void Render(HDC pHDC, RenderData* inRenderData) = 0;
};

class BaseModel : public InterfaceBaseModel
{
public:
	BaseModel(const std::wstring inResourcePath);
	BaseModel() = delete;
	virtual ~BaseModel();

protected:
	std::unique_ptr<CImage> imageOfModel;
};

#pragma region [Legacy Code]
/*
struct BitData 
{
	UINT16 xPosition;
	UINT16 yPosition;

	BitData(const UINT16 inXPosition, const UINT16 inYPosition) noexcept
		:xPosition(inXPosition), yPosition(inYPosition)
	{}
};

struct StretchData
{
	UINT16 xPosition;
	UINT16 yPosition;
	UINT16 xSize;
	UINT16 ySize;

	StretchData(const UINT16 inXPosition, const UINT16 inYPosition,
		const UINT16 inXSize, const UINT16 inYSize) noexcept
		: xPosition(inXPosition), yPosition(inYPosition)
		, xSize(inXSize), ySize(inYSize)
	{}
};

struct TransparentData
{
	UINT16 xPosition;
	UINT16 yPosition;
	UINT16 xSize;
	UINT16 ySize;
	UINT32 rgbColor;

	TransparentData(const UINT16 inXPosition, const UINT16 inYPosition,
		const UINT16 inXSize, const UINT16 inYSize, const UINT32 inRgbColor) noexcept
		: xPosition(inXPosition), yPosition(inYPosition)
		, xSize(inXSize), ySize(inYSize), rgbColor(inRgbColor)
	{}
};

struct RenderData
{
	union
	{
		BitData bitData;
		StretchData stretchData;
		TransparentData transparentData;
	};

	RenderData() noexcept
		: bitData(0,0)
	{};

	RenderData(const UINT16 inXPosition, const UINT16 inYPosition) noexcept
		: bitData(inXPosition, inYPosition)
	{};

	RenderData(const UINT16 inXPosition, const UINT16 inYPosition,
		const UINT16 inXSize, const UINT16 inYSize) noexcept
		: stretchData(inXPosition, inYPosition, inXSize, inYSize)
	{};

	RenderData(const UINT16 inXPosition, const UINT16 inYPosition,
		const UINT16 inXSize, const UINT16 inYSize, const UINT32 inRgbColor) noexcept
		: transparentData(inXPosition, inYPosition, inXSize, inYSize, inRgbColor)
	{};
};
*/
#pragma endregion