#include "MyPG.h"
#include "Task_GameBG.h"
#include "Task_Game.h"
#include "GameStage.h"

#include <cmath>

namespace GameBG
{
	Resource::WP Resource::instance;

	bool Resource::Initialize()
	{
		floorImg = DG::Image::Create("./data/image/yuka.jpg");
		eggPackImg = DG::Image::Create("./data/image/tamagopakku.png");
		stoveImg = DG::Image::Create("./data/image/konnro.png");
		wallImg = DG::Image::Create("./data/image/kabe.png");
		futonImg = DG::Image::Create("./data/image/hutonn.png");
		chairImg = DG::Image::Create("./data/image/isu.png");
		sofaImg = DG::Image::Create("./data/image/sofa.png");
		chestImg = DG::Image::Create("./data/image/tannsu.png");
		tableImg = DG::Image::Create("./data/image/teburu.png");
		deskImg = DG::Image::Create("./data/image/desuku.png");
		pianoImg = DG::Image::Create("./data/image/piano.png");
		// S-rank route targets use the floating star marker supplied for the stage.
		sRankMarkerImg = DG::Image::Create("./data/image/s_rank_star.png");

		return floorImg && eggPackImg && stoveImg && wallImg &&
			futonImg && chairImg && sofaImg && chestImg &&
			tableImg && deskImg && pianoImg && sRankMarkerImg;
	}

	bool Resource::Finalize()
	{
		floorImg.reset();
		eggPackImg.reset();
		stoveImg.reset();
		wallImg.reset();
		futonImg.reset();
		chairImg.reset();
		sofaImg.reset();
		chestImg.reset();
		tableImg.reset();
		deskImg.reset();
		pianoImg.reset();
		sRankMarkerImg.reset();
		return true;
	}

	bool Object::Initialize()
	{
		__super::Initialize(defGroupName, defName, true);
		res = Resource::Create();
		render2D_Priority[1] = 1.0f;
		return res != nullptr;
	}

	bool Object::Finalize()
	{
		res.reset();
		return true;
	}

	void Object::UpDate()
	{
	}

	void Object::Render2D_AF()
	{
		const int cameraOffset = -static_cast<int>(ge->camera2D.x);
		DrawFloor(cameraOffset);
		DrawStartTable(cameraOffset);
		DrawWalls(cameraOffset);
		DrawEggPack(cameraOffset);
		DrawFurniture(cameraOffset);
		DrawSRankMarkers(cameraOffset);
		DrawGoal(cameraOffset);
	}

	void Object::DrawStartTable(int cameraOffset) const
	{
		// Match the Game1 concept art: the starting table sits partly outside
		// the left edge, around the egg pack and player.
		ML::Box2D draw(-145 + cameraOffset, 210, 667, 548);
		const ML::Box2D src(0, 0, 100, 100);
		res->tableImg->Rotation(0.0f, ML::Vec2(333.5f, 274.0f));
		res->tableImg->Draw(draw, src);
	}

	DG::Image::SP Object::FurnitureImage(int kind) const
	{
		switch (static_cast<GameStage::FurnitureKind>(kind)) {
		case GameStage::FurnitureKind::Desk:  return res->deskImg;
		case GameStage::FurnitureKind::Futon: return res->futonImg;
		case GameStage::FurnitureKind::Chair: return res->chairImg;
		case GameStage::FurnitureKind::Piano: return res->pianoImg;
		case GameStage::FurnitureKind::Sofa:  return res->sofaImg;
		case GameStage::FurnitureKind::Chest: return res->chestImg;
		case GameStage::FurnitureKind::Table: return res->tableImg;
		}
		return nullptr;
	}

	void Object::DrawFloor(int cameraOffset) const
	{
		const POINT floorSize = res->floorImg->Size();
		const int floorCount = GameStage::WorldWidth / GameStage::ScreenWidth;
		const int seamOverlap = 1;
		for (int i = 0; i < floorCount; ++i) {
			// Mirror every second image so matching texture edges touch.
			const ML::Box2D src = (i % 2 == 0)
				? ML::Box2D(0, 0, floorSize.x, floorSize.y)
				: ML::Box2D(floorSize.x, 0, -floorSize.x, floorSize.y);
			const int drawWidth = GameStage::ScreenWidth +
				(i + 1 < floorCount ? seamOverlap : 0);
			ML::Box2D draw(GameStage::ScreenWidth * i + cameraOffset, 0,
				drawWidth, GameStage::ScreenHeight);
			res->floorImg->Rotation(0.0f, ML::Vec2(0.0f, 0.0f));
			res->floorImg->Draw(draw, src);
		}
	}

	void Object::DrawWalls(int cameraOffset) const
	{
		const ML::Box2D src(0, 0, 100, 100);

		for (int i = 0; i < GameStage::WallCount(); ++i) {
			const GameStage::WallSpec& wall = GameStage::Wall(i);
			const int drawHeight = static_cast<int>(wall.thickness * 12.5f);
			const float dx = wall.x2 - wall.x1;
			const float dy = wall.y2 - wall.y1;
			const float length = std::sqrt(dx * dx + dy * dy);
			const float centerX = (wall.x1 + wall.x2) * 0.5f;
			const float centerY = (wall.y1 + wall.y2) * 0.5f;
			const float angle = std::atan2(dy, dx);

			ML::Box2D draw(
				static_cast<int>(centerX - length * 0.5f) + cameraOffset,
				static_cast<int>(centerY - drawHeight * 0.5f),
				static_cast<int>(length), drawHeight);
			res->wallImg->Rotation(angle, ML::Vec2(length * 0.5f, drawHeight * 0.5f));
			res->wallImg->Draw(draw, src);
		}
	}

	void Object::DrawEggPack(int cameraOffset) const
	{
		ML::Box2D draw(cameraOffset, 310, 260, 280);
		const ML::Box2D src(0, 0, 100, 100);
		res->eggPackImg->Rotation(0.0f, ML::Vec2(130.0f, 140.0f));
		res->eggPackImg->Draw(draw, src);
	}

	void Object::DrawFurniture(int cameraOffset) const
	{
		const ML::Box2D src(0, 0, 100, 100);
		for (int i = 0; i < GameStage::FurnitureCount(); ++i) {
			const GameStage::FurnitureSpec& furniture = GameStage::Furniture(i);
			DG::Image::SP image = FurnitureImage(static_cast<int>(furniture.kind));
			if (!image) {
				continue;
			}

			ML::Box2D draw(furniture.x + cameraOffset, furniture.y,
				furniture.w, furniture.h);
			image->Rotation(ML::ToRadian(furniture.angleDeg),
				ML::Vec2(furniture.w * 0.5f, furniture.h * 0.5f));
			image->Draw(draw, src);
		}
	}

	void Object::DrawSRankMarkers(int cameraOffset) const
	{
		const int markerSize = 86;
		const ML::Box2D src(0, 0, 256, 256);
		const float elapsed = Game::GetElapsedTimeSeconds();

		for (int i = 0; i < GameStage::SRankTargetCount(); ++i) {
			if (Game::IsSRankTargetVisited(i)) {
				continue;
			}

			const GameStage::FurnitureSpec& furniture = GameStage::Furniture(
				GameStage::SRankTargetFurnitureIndex(i));
			const int markerX = furniture.x + furniture.w / 2 - markerSize / 2;
			const int baseY = furniture.y + furniture.h / 2 - markerSize / 2;
			const int floatOffset = static_cast<int>(
				std::sin(elapsed * 2.5f + i * 0.75f) * 12.0f);
			ML::Box2D draw(markerX + cameraOffset, baseY + floatOffset,
				markerSize, markerSize);
			res->sRankMarkerImg->Rotation(0.0f,
				ML::Vec2(markerSize * 0.5f, markerSize * 0.5f));
			res->sRankMarkerImg->Draw(draw, src);
		}
	}

	void Object::DrawGoal(int cameraOffset) const
	{
		ML::Box2D draw(9200 + cameraOffset, -42, 450, 320);
		const ML::Box2D src(0, 0, 100, 100);
		res->stoveImg->Rotation(0.0f, ML::Vec2(225.0f, 160.0f));
		res->stoveImg->Draw(draw, src);
	}

	Object::SP Object::Create(bool flagGameEnginePushBack_)
	{
		Object::SP object(new Object());
		if (object) {
			object->me = object;
			if (flagGameEnginePushBack_) {
				ge->PushBack(object);
			}
			if (!object->B_Initialize()) {
				object->Kill(false);
			}
			return object;
		}
		return nullptr;
	}

	bool Object::B_Initialize()
	{
		return Initialize();
	}

	Object::~Object()
	{
		B_Finalize();
	}

	bool Object::B_Finalize()
	{
		return Finalize();
	}

	Object::Object()
	{
	}

	Resource::SP Resource::Create()
	{
		if (auto resource = instance.lock()) {
			return resource;
		}

		Resource::SP resource(new Resource());
		if (resource && resource->Initialize()) {
			instance = resource;
			return resource;
		}
		return nullptr;
	}

	Resource::Resource()
	{
	}

	Resource::~Resource()
	{
		Finalize();
	}
}
