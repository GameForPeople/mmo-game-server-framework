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
	const unsigned short expPerLevel;

	const unsigned char attackRange;

	const _LevelType startLevel;
	const _LevelType levelMaxDifference;

	MonsterModel(MONSTER_TYPE, unsigned short , _DamageType,
		unsigned short, unsigned char , _LevelType_T, _LevelType_T);
};

class MonsterModelManager {
	std::vector<MonsterModel*> monsterModelCont;

public:
	MonsterModelManager();
	~MonsterModelManager();

public:
	MonsterModel* GetMonsterModel(MONSTER_TYPE);
};