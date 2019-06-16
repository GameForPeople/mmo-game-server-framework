#include "pch.h"
#include "../Define.h"

#include "ObjectInfo.h"
#include "BaseMonster.h"
#include "MonsterModelManager.h"

BaseMonster::BaseMonster(_KeyType inKey, _PosType inX, _PosType inY, const MonsterModel* const inMonsterModel)
	: objectInfo(nullptr)
	, key(inKey)
	, monsterModel(inMonsterModel)
	//, level()
	, spawnPosX(inX)
	, spawnPosY(inY)
	, isDead(false)
	, isSleep(false)
	, noDamageTick(0)
	, faintTick(0)
	, freezeTick(0)
	, electricTick(0)
	, burnTick(0)
{
	objectInfo = new ObjectInfo(inX, inY);
	objectInfo->level = ( rand() % monsterModel->levelMaxDifference ) + monsterModel->startLevel;
	objectInfo->hp = monsterModel->hpPerLevel * objectInfo->level;
	objectInfo->damage = monsterModel->damagePerLevel * objectInfo->level;
	
	if(monsterModel->monsterType == MONSTER_TYPE::SLIME) objectInfo->job = JOB_TYPE::SLIME;
	else if (monsterModel->monsterType == MONSTER_TYPE::GOLEM) objectInfo->job = JOB_TYPE::GOLEM;
	else if (monsterModel->monsterType == MONSTER_TYPE::DRAGON) objectInfo->job = JOB_TYPE::DRAGON;

	// objectInfo->exp = 0;
	// objectInfo-> mp = 0;
	// objectInfo-> money = 0;
	//	_RedCountType redCount;
	// _BlueCountType blueCount;
	// _TreeCountType treeCount;
}

BaseMonster::~BaseMonster()
{
	// 몬스터 모델은 여기서 삭제되서는 안됩니다.
	// monsterModel = nullptr;
	delete objectInfo;
}