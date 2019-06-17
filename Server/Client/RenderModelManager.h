#pragma once

class BaseModel;

class RenderModelManager
{
	std::vector<BaseModel*> renderModelCont;

public:
	RenderModelManager();
	~RenderModelManager();

public:
	BaseModel* GetRenderModel(RENDER_MODEL_TYPE);
};