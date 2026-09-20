#pragma warning(disable:4996)
#pragma once

#include "GameEngine_Ver3_83.h"

namespace GameBG
{
	const string defGroupName("Game");
	const string defName("Stage");

	class Resource : public BResource
	{
		bool Initialize() override;
		bool Finalize() override;
		Resource();
	public:
		~Resource();
		using SP = shared_ptr<Resource>;
		using WP = weak_ptr<Resource>;
		static WP instance;
		static SP Create();

		DG::Image::SP floorImg;
		DG::Image::SP eggPackImg;
		DG::Image::SP stoveImg;
		DG::Image::SP wallImg;
		DG::Image::SP futonImg;
		DG::Image::SP chairImg;
		DG::Image::SP sofaImg;
		DG::Image::SP chestImg;
		DG::Image::SP tableImg;
		DG::Image::SP deskImg;
		DG::Image::SP pianoImg;
		DG::Image::SP sRankMarkerImg;
	};

	class Object : public BTask
	{
	public:
		~Object() override;
		using SP = shared_ptr<Object>;
		using WP = weak_ptr<Object>;
		static SP Create(bool flagGameEnginePushBack_);

		Resource::SP res;
	private:
		Object();
		bool B_Initialize();
		bool B_Finalize();
		bool Initialize();
		void UpDate() override;
		void Render2D_AF() override;
		bool Finalize();

		DG::Image::SP FurnitureImage(int kind) const;
		void DrawFloor(int cameraOffset) const;
		void DrawStartTable(int cameraOffset) const;
		void DrawWalls(int cameraOffset) const;
		void DrawEggPack(int cameraOffset) const;
		void DrawFurniture(int cameraOffset) const;
		void DrawSRankMarkers(int cameraOffset) const;
		void DrawGoal(int cameraOffset) const;
	};
}
