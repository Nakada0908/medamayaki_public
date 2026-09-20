#include "GameStage.h"
#include "GameConfig.h"

#include <algorithm>

namespace GameStage
{
	namespace
	{
		// Positions, sizes, and rotations follow stage furniture.PNG (9600 x 1080).
		// Route colors and walls are intentionally not considered in this layout pass.
		const FurnitureSpec furnitureList[] = {
			{ FurnitureKind::Chair,   739,  609, 240, 223,   0.0f },
			{ FurnitureKind::Desk,   1159,  -81, 440, 610,  90.0f },
			// Centered below the desk landing point.
			{ FurnitureKind::Chair,  1316,  954, 127, 126,   0.0f },
			{ FurnitureKind::Futon,  2124,  438, 305, 386,   0.0f },
			{ FurnitureKind::Chest,  2625,  -74, 304, 300, -90.0f },
			{ FurnitureKind::Sofa,   3201,  484, 469, 439,   0.0f },
			{ FurnitureKind::Chest,  4077,  849, 304, 300,  90.0f },
			{ FurnitureKind::Chair,  4602,   76, 192, 192,   0.0f },
			{ FurnitureKind::Chair,  5163,  791, 184, 186,   0.0f },
			{ FurnitureKind::Piano,  5552,  196, 355, 349,   0.0f },
			{ FurnitureKind::Futon,  6122,  666, 319, 414,   0.0f },
			{ FurnitureKind::Chair,  6186,   11, 128, 124,   0.0f },
			{ FurnitureKind::Chest,  6823,  -88, 220, 286, -90.0f },
			{ FurnitureKind::Chair,  6889,  964, 119, 116,   0.0f },
			{ FurnitureKind::Table,  7424,  363, 341, 221,   0.0f },
			{ FurnitureKind::Chair,  8292,  536, 189, 178,   0.0f },
			{ FurnitureKind::Futon,  8491,    6, 263, 290,   0.0f },
			{ FurnitureKind::Chair,  8572,  974, 137, 106,   0.0f },
			{ FurnitureKind::Chair,  8963,  643, 144, 124,   0.0f },
			// Centered on the frying pan and its final vertical passage.
			{ FurnitureKind::Chair,  9466,  867, 139, 121,   0.0f }
		};

		// Furniture circled in green in the S-rank route reference.
		// The same list drives marker drawing and route-completion checks.
		const int sRankTargetFurnitureIndices[] = { 2, 6, 8, 11, 13, 17 };

		const WallSpec wallList[] = {
			// Screen 1
			{  678.0f,    7.0f,  883.0f,  461.0f, 23.0f },
			{ 1197.0f,  608.0f,  915.0f, 1078.0f, 24.0f },
			// Egg-width chute centered on the desk and chair landing points.
			{ 1329.0f,  534.0f, 1329.0f, 1068.0f, 24.0f },
			{ 1429.0f,  527.0f, 1429.0f,  950.0f, 24.0f },

			// Screens 2-3
			{ 4488.0f,    1.0f, 3645.0f,  291.0f, 24.0f },
			{ 3608.0f,  945.0f, 4078.0f,  942.0f, 24.0f },
			{ 4078.0f,  942.0f, 4483.0f,  377.0f, 24.0f },
			{ 4785.0f,  461.0f, 4988.0f,  794.0f, 14.0f },
			{ 5273.0f,  339.0f, 5273.0f,  547.0f, 11.0f },
			{ 5274.0f,    2.0f, 5278.0f,  134.0f,  5.0f },
			{ 5828.0f,  558.0f, 5393.0f,  948.0f, 20.0f },
			{ 5393.0f,  948.0f, 5393.0f, 1074.0f, 20.0f },
			{ 6017.0f,    0.0f, 5624.0f,  219.0f, 18.0f },

			// Screen 4
			// Four shared corners form a clean rotated rectangle.
			{ 5946.0f,  302.0f, 6160.0f,  156.0f,  8.0f },
			{ 5946.0f,  302.0f, 6222.0f,  654.0f, 22.0f },
			{ 6160.0f,  156.0f, 6436.0f,  508.0f, 16.0f },
			// Pull the V away from the futon-to-chest line for the easy route.
			{ 6333.0f,    3.0f, 6550.0f,  395.0f, 20.0f },
			{ 6550.0f,  395.0f, 6789.0f,   59.0f, 20.0f },
			{ 6436.0f,  508.0f, 6222.0f,  654.0f,  7.0f },
			// A slightly wider parallel chute that opens before the chair.
			{ 6703.0f,  783.0f, 6535.0f, 1077.0f, 12.0f },
			{ 6703.0f,  783.0f, 6832.0f,  958.0f, 12.0f },
			// Shorten the upper end so the egg can leave the sideways chest.
			{ 7020.0f,  207.0f, 6796.0f,  733.0f, 20.0f },
			{ 6791.0f,  717.0f, 6920.0f,  892.0f, 12.0f },

			// Screen 5
			{ 7731.0f,  582.0f, 8268.0f,  755.0f, 22.0f },
			// Leave one visible egg-height between this hard-route wall and
			// the bottom of the screen. Move the joined wall endpoint with it.
			{ 8227.0f,  978.0f, 8582.0f,  980.0f, 14.0f },
			{ 8582.0f,  980.0f, 8578.0f,  476.0f, 18.0f },
			{ 8413.0f,    6.0f, 8280.0f,  477.0f, 17.0f },
			{ 8683.0f,  504.0f, 8964.0f,  749.0f, 18.0f },
			{ 8683.0f,  504.0f, 8686.0f,  997.0f, 22.0f },
			{ 8711.0f,   24.0f, 9052.0f,  623.0f, 30.0f },
			// Open the diagonal entrance enough for the egg plus a small margin.
			{ 9018.0f,  767.0f, 9485.0f, 1000.0f, 20.0f },
			{ 9110.0f,  653.0f, 9485.0f,  850.0f, 20.0f },
			// Final passage: its visible gap matches the egg and its center
			// lines up with the center of the frying pan.
			{ 9485.0f,  850.0f, 9485.0f,  164.0f, 20.0f },
			{ 9585.0f,  170.0f, 9585.0f,  887.0f, 19.0f }
		};
		const bool wallsEnabled = true;

		float PointSegmentDistanceSquared(float px, float py, const WallSpec& wall)
		{
			const float dx = wall.x2 - wall.x1;
			const float dy = wall.y2 - wall.y1;
			const float lengthSquared = dx * dx + dy * dy;
			float ratio = 0.0f;

			if (lengthSquared > 0.0f) {
				ratio = ((px - wall.x1) * dx + (py - wall.y1) * dy) / lengthSquared;
				ratio = (std::max)(0.0f, (std::min)(1.0f, ratio));
			}

			const float closestX = wall.x1 + dx * ratio;
			const float closestY = wall.y1 + dy * ratio;
			const float distanceX = px - closestX;
			const float distanceY = py - closestY;
			return distanceX * distanceX + distanceY * distanceY;
		}
	}

	int FurnitureCount()
	{
		return static_cast<int>(sizeof(furnitureList) / sizeof(furnitureList[0]));
	}

	const FurnitureSpec& Furniture(int index)
	{
		return furnitureList[index];
	}

	int SRankTargetCount()
	{
		return static_cast<int>(sizeof(sRankTargetFurnitureIndices) /
			sizeof(sRankTargetFurnitureIndices[0]));
	}

	int SRankTargetFurnitureIndex(int targetIndex)
	{
		return sRankTargetFurnitureIndices[targetIndex];
	}

	int SRankTargetIndexForFurniture(int furnitureIndex)
	{
		for (int i = 0; i < SRankTargetCount(); ++i) {
			if (SRankTargetFurnitureIndex(i) == furnitureIndex) {
				return i;
			}
		}
		return -1;
	}

	int WallCount()
	{
		if (!wallsEnabled) {
			return 0;
		}
		return static_cast<int>(sizeof(wallList) / sizeof(wallList[0]));
	}

	const WallSpec& Wall(int index)
	{
		return wallList[index];
	}

	ML::Box2D EggHitBox(float x, float y)
	{
		return ML::Box2D(
			static_cast<int>(x - GameConfig::Egg::CollisionRadius),
			static_cast<int>(y - GameConfig::Egg::CollisionRadius),
			static_cast<int>(GameConfig::Egg::CollisionRadius * 2.0f),
			static_cast<int>(GameConfig::Egg::CollisionRadius * 2.0f));
	}

	int FurnitureHitBoxCount(const FurnitureSpec& furniture)
	{
		return furniture.kind == FurnitureKind::Desk ? 2 : 1;
	}

	ML::Box2D FurnitureHitBox(const FurnitureSpec& furniture, int hitboxIndex)
	{
		// The desk in the reference is rotated 90 degrees: one horizontal arm at
		// the top and one vertical arm at the right.
		if (furniture.kind == FurnitureKind::Desk) {
			const int centerX = furniture.x + furniture.w / 2;
			const int centerY = furniture.y + furniture.h / 2;
			if (hitboxIndex == 0) {
				return ML::Box2D(
					centerX - furniture.h * 46 / 100,
					centerY - furniture.w * 46 / 100,
					furniture.h * 92 / 100,
					furniture.w * 34 / 100);
			}
			return ML::Box2D(
				centerX + furniture.h * 16 / 100,
				centerY - furniture.w * 46 / 100,
				furniture.h * 30 / 100,
				furniture.w * 92 / 100);
		}

		// Chests are placed sideways; the S-route chest faces the opposite way.
		if (furniture.kind == FurnitureKind::Chest) {
			const int centerX = furniture.x + furniture.w / 2;
			const int centerY = furniture.y + furniture.h / 2;
			return ML::Box2D(
				centerX - furniture.h / 2,
				centerY - furniture.w * 25 / 100,
				furniture.h,
				furniture.w * 51 / 100);
		}

		int left = 10;
		int top = 0;
		int width = 79;
		int height = 100;
		switch (furniture.kind) {
		case FurnitureKind::Futon:
			left = 17; width = 64; break;
		case FurnitureKind::Piano:
			left = 16; width = 69; height = 99; break;
		case FurnitureKind::Sofa:
			left = 0; top = 28; width = 100; height = 44; break;
		case FurnitureKind::Table:
			left = 2; width = 94; break;
		case FurnitureKind::Chair:
		default:
			break;
		}
		return ML::Box2D(
			furniture.x + furniture.w * left / 100,
			furniture.y + furniture.h * top / 100,
			furniture.w * width / 100,
			furniture.h * height / 100);
	}

	bool HitAnyWall(float x, float y)
	{
		for (int i = 0; i < WallCount(); ++i) {
			const WallSpec& wall = Wall(i);
			const float hitDistance =
				GameConfig::Egg::CollisionRadius + wall.thickness * 0.5f;
			if (PointSegmentDistanceSquared(x, y, wall) <= hitDistance * hitDistance) {
				return true;
			}
		}
		return false;
	}

	int HitFurniture(float x, float y, int ignoredIndex, int* hitboxIndex)
	{
		if (hitboxIndex) {
			*hitboxIndex = -1;
		}
		const ML::Box2D eggHit = EggHitBox(x, y);
		for (int i = 0; i < FurnitureCount(); ++i) {
			if (i == ignoredIndex) {
				continue;
			}
			const FurnitureSpec& furniture = Furniture(i);
			for (int part = 0; part < FurnitureHitBoxCount(furniture); ++part) {
				if (eggHit.Hit(FurnitureHitBox(furniture, part))) {
					if (hitboxIndex) {
						*hitboxIndex = part;
					}
					return i;
				}
			}
		}
		return -1;
	}

	bool HitGoal(float x, float y)
	{
		return ML::Box2D(9220, 0, 380, 276).Hit(EggHitBox(x, y));
	}
}
