#include "stdafx.h"
#include "BaseModel.h"

#include "BitModel.h"

BitModel::BitModel(const std::wstring& inResourcePath)
	: BaseModel(inResourcePath)
{
}

void BitModel::Render(HDC pHDC, RenderData* inRenderData)
{
	imageOfModel->BitBlt(pHDC, 
		inRenderData->/*bitData.*/xPosition,
		inRenderData->/*bitData.*/yPosition
	);
}