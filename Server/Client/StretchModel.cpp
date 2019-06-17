#include "stdafx.h"
#include "BaseModel.h"
#include "StretchModel.h"

StretchModel::StretchModel(const std::wstring& inResourcePath)
	: BaseModel(inResourcePath)
{
}

void StretchModel::Render(HDC pHDC, RenderData* inRenderData)
{
	imageOfModel->StretchBlt(
		pHDC,
		inRenderData->/*stretchData.*/xPosition,
		inRenderData->/*stretchData.*/yPosition,
		inRenderData->/*stretchData.*/xSize,
		inRenderData->/*stretchData.*/ySize
	);
}