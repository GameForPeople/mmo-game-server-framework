#pragma once

#include "InHeaderDefine.hh"

enum class MONSTER_TYPE : int
{
	SLIME = 0,
	GOLEM = 1,
	DRAGON = 2,
	ENUM_SIZE
};

struct MonsterModel {
	const MONSTER_TYPE monsterType;

	const _HpType hpPerLevel;
	const _DamageType damagePerLevel;

	const unsigned char attackRange;

	const _LevelType startLevel;
	const _LevelType levelMaxDifference;

	MonsterModel(MONSTER_TYPE, _HpType , _DamageType
		, unsigned char , _LevelType, _LevelType);
};

class MonsterModelManager {
	std::vector<MonsterModel*> monsterModelCont;

public:
	MonsterModelManager();
	~MonsterModelManager();

public:
	MonsterModel* GetRenderModel(MONSTER_TYPE);
};