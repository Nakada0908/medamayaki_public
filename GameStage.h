#pragma once

#include "GameEngine_Ver3_83.h"

namespace GameStage
{
	const int ScreenWidth = 1920;
	const int ScreenHeight = 1080;
	const int WorldWidth = 9600;
	const float Pi = 3.14159265358979323846f;

	enum class FurnitureKind
	{
		Desk,
		Futon,
		Chair,
		Piano,
		Sofa,
		Chest,
		Table
	};

	struct FurnitureSpec
	{
		FurnitureKind kind;
		int x;
		int y;
		int w;
		int h;
		float angleDeg;
	};

	struct WallSpec
	{
		float x1;
		float y1;
		float x2;
		float y2;
		float thickness;
	};

	int FurnitureCount();
	const FurnitureSpec& Furniture(int index);
	int SRankTargetCount();
	int SRankTargetFurnitureIndex(int targetIndex);
	int SRankTargetIndexForFurniture(int furnitureIndex);
	int WallCount();
	const WallSpec& Wall(int index);

	ML::Box2D EggHitBox(float x, float y);
	int FurnitureHitBoxCount(const FurnitureSpec& furniture);
	ML::Box2D FurnitureHitBox(const FurnitureSpec& furniture, int hitboxIndex = 0);
	bool HitAnyWall(float x, float y);
	int HitFurniture(float x, float y, int ignoredIndex = -1,
		int* hitboxIndex = nullptr);
	bool HitGoal(float x, float y);
}
