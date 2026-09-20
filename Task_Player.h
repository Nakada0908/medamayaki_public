#pragma warning(disable:4996)
#pragma once

#include "GameEngine_Ver3_83.h"

namespace Player
{
	const string defGroupName("Game");
	const string defName("Player");

	enum class PlayerState
	{
		Stay,
		Go,
		Dead,
		Medamayaki
	};

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

		DG::Image::SP eggImg;
		DG::Image::SP deadImg;
		DG::Image::SP arrowImg;
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

		void UpdateArrow();
		void UpdateManualCamera(const XI::VGamePad& input);
		void WarpToFurniture(int mouseX, int mouseY);
		void StartShot();
		void UpdateShot();
		void StartDeath();
		void Respawn(bool countAsGameOver);
		void UpdateCamera();
		void DrawEgg() const;
		void DrawArrow() const;

		XI::GamePad::SP controller;
		XI::Mouse::SP mouse;
		PlayerState state;
		float x;
		float y;
		float arrowAngle;
		float velocityX;
		float velocityY;
		float checkpointX;
		float checkpointY;
		int checkpointFurniture;
		int ignoredFurniture;
		float ignoreDistanceRemaining;
		int deathFrameCount;
		bool showArrow;
	};
}
