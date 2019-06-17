#include "pch.h"

#include "MonsterModelManager.h"

//--------------------------------------------------------
// MonsterModel
//--------------------------------------------------------

MonsterModel::MonsterModel(	MONSTER_TYPE inMonsterType
	, unsigned short inHpPerLevel
	, _DamageType inDamagePerLevel
	, unsigned short expPerLevel
	, _MoneyType_T moneyPerLevel
	, unsigned char inAttackRange
	, _LevelType_T inStartLevel
	, _LevelType_T inLevelMaxDifference)
	: monsterType(inMonsterType)
	, hpPerLevel(inHpPerLevel)
	, damagePerLevel(inDamagePerLevel)
	, expPerLevel(expPerLevel)
	, moneyPerLevel(moneyPerLevel)
	, attackRange(inAttackRange)
	, startLevel(inStartLevel)
	, levelMaxDifference(inLevelMaxDifference)
{
}

//--------------------------------------------------------
// MonsterModelManager
//--------------------------------------------------------
MonsterModelManager::MonsterModelManager()
{
	monsterModelCont.reserve(static_cast<int>(MONSTER_TYPE::ENUM_SIZE));

	for (int i = 0; i < static_cast<int>(MONSTER_TYPE::ENUM_SIZE); ++i)
	{
		monsterModelCont.emplace_back(nullptr);
	}

	monsterModelCont[static_cast<int>(MONSTER_TYPE::SLIME)] = new MonsterModel(MONSTER_TYPE::SLIME, 10, 1, 20, 1, 1, 1, 20);
	monsterModelCont[static_cast<int>(MONSTER_TYPE::GOLEM)] = new MonsterModel(MONSTER_TYPE::GOLEM, 20, 2, 40, 2, 5, 30, 10);
	monsterModelCont[static_cast<int>(MONSTER_TYPE::DRAGON)] = new MonsterModel(MONSTER_TYPE::DRAGON, 100, 3, 60, 5, 10, 50, 0);
}

MonsterModelManager::~MonsterModelManager()
{
	for (auto& iter : monsterModelCont)
	{
		delete iter;
	}
}

MonsterModel* MonsterModelManager::GetMonsterModel(MONSTER_TYPE inMonsterType)
{
	return monsterModelCont[static_cast<int>(inMonsterType)];
}