#include "MyPG.h"
#include "Task_Player.h"
#include "Task_Game.h"
#include "GameStage.h"
#include "GameConfig.h"

#include <algorithm>
#include <cmath>

namespace Player
{
	namespace
	{
		const float CameraMoveSpeed = 20.0f;
		const int DeathDurationFrames = 30;
		const int EggHalfDrawSize = GameConfig::Egg::DrawSize / 2;

		bool PointHitsFurniture(float pointX, float pointY,
			const GameStage::FurnitureSpec& furniture)
		{
			const float centerX = furniture.x + furniture.w * 0.5f;
			const float centerY = furniture.y + furniture.h * 0.5f;
			const float angle = ML::ToRadian(furniture.angleDeg);
			const float cosine = std::cos(angle);
			const float sine = std::sin(angle);
			const float offsetX = pointX - centerX;
			const float offsetY = pointY - centerY;
			const float localX = offsetX * cosine + offsetY * sine;
			const float localY = -offsetX * sine + offsetY * cosine;

			return std::abs(localX) <= furniture.w * 0.5f &&
				std::abs(localY) <= furniture.h * 0.5f;
		}
	}

	Resource::WP Resource::instance;

	bool Resource::Initialize()
	{
		eggImg = DG::Image::Create("./data/image/tamago.png");
		deadImg = DG::Image::Create("./data/image/broken_egg.png");
		arrowImg = DG::Image::Create("./data/image/yazirusi.png");
		return eggImg && deadImg && arrowImg;
	}

	bool Resource::Finalize()
	{
		eggImg.reset();
		deadImg.reset();
		arrowImg.reset();
		return true;
	}

	bool Object::Initialize()
	{
		__super::Initialize(defGroupName, defName, true);
		res = Resource::Create();
		controller = ge->in1;
		mouse = ge->mouse;
		render2D_Priority[1] = 0.5f;

		state = PlayerState::Stay;
		x = 337.0f;
		y = 475.0f;
		arrowAngle = 0.0f;
		velocityX = 0.0f;
		velocityY = 0.0f;
		checkpointX = x;
		checkpointY = y;
		checkpointFurniture = -1;
		ignoredFurniture = -1;
		ignoreDistanceRemaining = 0.0f;
		deathFrameCount = 0;
		showArrow = true;
		UpdateCamera();
		return res != nullptr && controller != nullptr && mouse != nullptr;
	}

	bool Object::Finalize()
	{
		controller.reset();
		mouse.reset();
		res.reset();
		return true;
	}

	void Object::UpDate()
	{
		if (ge->GameClearFlag) {
			return;
		}

		const auto input = controller->GetState();
		const auto mouseState = mouse->GetState();

		if (state == PlayerState::Dead) {
			// Keep the splatted image still for half a second. Right click (or the
			// existing controller reset button) skips the remaining wait.
			if (mouseState.RB.down || input.B2.down || --deathFrameCount <= 0) {
				Respawn(false);
			}
			return;
		}

		// Middle-clicking furniture moves the egg there as an assist feature.
		if (mouseState.CB.down) {
			WarpToFurniture(mouseState.pos.x, mouseState.pos.y);
		}

		// Right click / X returns to the latest safe landing point.
		if (mouseState.RB.down || input.B2.down) {
			Respawn(state == PlayerState::Go);
		}

		if (state != PlayerState::Go) {
			UpdateManualCamera(input);
		}

		if (state == PlayerState::Stay) {
			UpdateArrow();
			if (mouseState.LB.down || input.B1.down) {
				StartShot();
			}
		}

		if (state == PlayerState::Go) {
			UpdateShot();
		}
	}

	void Object::Render2D_AF()
	{
		DrawEgg();
		if (showArrow) {
			DrawArrow();
		}
	}

	void Object::UpdateArrow()
	{
		arrowAngle += ML::ToRadian(2.0f);
		if (arrowAngle >= GameStage::Pi * 2.0f) {
			arrowAngle -= GameStage::Pi * 2.0f;
		}
	}

	void Object::UpdateManualCamera(const XI::VGamePad& input)
	{
		float cameraX = ge->camera2D.x + input.RStick.axis.x * CameraMoveSpeed;

		cameraX = (std::max)(0.0f, cameraX);
		cameraX = (std::min)(static_cast<float>(
			GameStage::WorldWidth - GameStage::ScreenWidth), cameraX);

		ge->camera2D.x = static_cast<int>(cameraX);
		ge->camera2D.y = 0;
	}

	void Object::WarpToFurniture(int mouseX, int mouseY)
	{
		const int worldX = mouseX + ge->camera2D.x;
		const int worldY = mouseY + ge->camera2D.y;

		for (int furnitureIndex = 0;
			furnitureIndex < GameStage::FurnitureCount(); ++furnitureIndex) {
			const GameStage::FurnitureSpec& furniture =
				GameStage::Furniture(furnitureIndex);
			if (!PointHitsFurniture(static_cast<float>(worldX),
				static_cast<float>(worldY), furniture)) {
				continue;
			}

			int nearestHitbox = 0;
			float nearestDistanceSquared = -1.0f;
			for (int part = 0;
				part < GameStage::FurnitureHitBoxCount(furniture); ++part) {
				const ML::Box2D hitbox = GameStage::FurnitureHitBox(furniture, part);
				const float hitboxCenterX = hitbox.x + hitbox.w * 0.5f;
				const float hitboxCenterY = hitbox.y + hitbox.h * 0.5f;
				const float distanceX = worldX - hitboxCenterX;
				const float distanceY = worldY - hitboxCenterY;
				const float distanceSquared =
					distanceX * distanceX + distanceY * distanceY;
				if (nearestDistanceSquared < 0.0f ||
					distanceSquared < nearestDistanceSquared) {
					nearestHitbox = part;
					nearestDistanceSquared = distanceSquared;
				}
			}

			const ML::Box2D landingBox =
				GameStage::FurnitureHitBox(furniture, nearestHitbox);
			x = landingBox.x + landingBox.w * 0.5f;
			y = landingBox.y + landingBox.h * 0.5f;
			state = PlayerState::Stay;
			checkpointX = x;
			checkpointY = y;
			checkpointFurniture = furnitureIndex;
			Game::VisitSRankTargetFurniture(furnitureIndex);
			ignoredFurniture = -1;
			ignoreDistanceRemaining = 0.0f;
			velocityX = 0.0f;
			velocityY = 0.0f;
			showArrow = true;
			return;
		}
	}

	void Object::StartShot()
	{
		if (state != PlayerState::Stay) {
			return;
		}

		state = PlayerState::Go;
		velocityX = std::cos(arrowAngle) * GameConfig::Egg::ShotSpeed;
		velocityY = std::sin(arrowAngle) * GameConfig::Egg::ShotSpeed;
		ignoredFurniture = checkpointFurniture;
		if (ignoredFurniture >= 0) {
			const GameStage::FurnitureSpec& furniture =
				GameStage::Furniture(ignoredFurniture);
			ignoreDistanceRemaining = static_cast<float>(
				(std::max)(furniture.w, furniture.h)) +
				GameConfig::Egg::CollisionRadius * 2.0f;
		}
		showArrow = false;
		// Manual camera movement ends as soon as the egg is fired.
		UpdateCamera();
	}

	void Object::UpdateShot()
	{
		const int steps = (std::max)(1,
			static_cast<int>(GameConfig::Egg::ShotSpeed /
				GameConfig::Egg::MoveStep));
		const float stepX = velocityX / static_cast<float>(steps);
		const float stepY = velocityY / static_cast<float>(steps);

		for (int i = 0; i < steps; ++i) {
			x += stepX;
			y += stepY;

			if (GameStage::HitGoal(x, y)) {
				state = PlayerState::Medamayaki;
				ge->GameClearFlag = true;
				break;
			}

			if (GameStage::HitAnyWall(x, y)) {
				StartDeath();
				break;
			}

			int furnitureHitbox = -1;
			const int furnitureHit = GameStage::HitFurniture(
				x, y, ignoredFurniture, &furnitureHitbox);
			if (furnitureHit >= 0) {
				const GameStage::FurnitureSpec& landed = GameStage::Furniture(furnitureHit);
				const ML::Box2D landingBox =
					GameStage::FurnitureHitBox(landed, furnitureHitbox);
				x = landingBox.x + landingBox.w * 0.5f;
				y = landingBox.y + landingBox.h * 0.5f;
				state = PlayerState::Stay;
				checkpointX = x;
				checkpointY = y;
				checkpointFurniture = furnitureHit;
				Game::VisitSRankTargetFurniture(furnitureHit);
				ignoredFurniture = -1;
				ignoreDistanceRemaining = 0.0f;
				velocityX = 0.0f;
				velocityY = 0.0f;
				showArrow = true;
				break;
			}

			if (ignoredFurniture >= 0) {
				ignoreDistanceRemaining -= GameConfig::Egg::MoveStep;
				if (ignoreDistanceRemaining <= 0.0f) {
					ignoredFurniture = -1;
				}
			}

			if (x < -100.0f || x > GameStage::WorldWidth + 100.0f ||
				y < -100.0f || y > GameStage::ScreenHeight + 100.0f) {
				StartDeath();
				break;
			}
		}

		UpdateCamera();
	}

	void Object::StartDeath()
	{
		state = PlayerState::Dead;
		velocityX = 0.0f;
		velocityY = 0.0f;
		deathFrameCount = DeathDurationFrames;
		showArrow = false;
		Game::AddGameOver();
	}

	void Object::Respawn(bool countAsGameOver)
	{
		if (countAsGameOver) {
			Game::AddGameOver();
		}

		state = PlayerState::Stay;
		x = checkpointX;
		y = checkpointY;
		velocityX = 0.0f;
		velocityY = 0.0f;
		ignoredFurniture = -1;
		ignoreDistanceRemaining = 0.0f;
		deathFrameCount = 0;
		showArrow = true;
		UpdateCamera();
	}

	void Object::UpdateCamera()
	{
		float cameraX = x - 420.0f;
		cameraX = (std::max)(0.0f, cameraX);
		cameraX = (std::min)(static_cast<float>(
			GameStage::WorldWidth - GameStage::ScreenWidth), cameraX);
		ge->camera2D.x = static_cast<int>(cameraX);
		ge->camera2D.y = 0;
	}

	void Object::DrawEgg() const
	{
		if (state == PlayerState::Dead) {
			ML::Box2D draw(-120, -66, 240, 131);
			draw.Offset(x - ge->camera2D.x, y);
			const ML::Box2D src(0, 0, 2816, 1536);
			res->deadImg->Rotation(0.0f, ML::Vec2(1408.0f, 768.0f));
			res->deadImg->Draw(draw, src);
			return;
		}

		ML::Box2D draw(-EggHalfDrawSize, -EggHalfDrawSize,
			GameConfig::Egg::DrawSize, GameConfig::Egg::DrawSize);
		draw.Offset(x - ge->camera2D.x, y);
		const ML::Box2D src(0, 0,
			GameConfig::Egg::TextureSize, GameConfig::Egg::TextureSize);
		res->eggImg->Rotation(0.0f,
			ML::Vec2(GameConfig::Egg::TextureSize * 0.5f,
				GameConfig::Egg::TextureSize * 0.5f));
		res->eggImg->Draw(draw, src);
	}

	void Object::DrawArrow() const
	{
		ML::Box2D draw(0, -50, 100, 100);
		draw.Offset(x - ge->camera2D.x, y);
		const ML::Box2D src(0, 0, 100, 100);
		res->arrowImg->Rotation(arrowAngle, ML::Vec2(0.0f, 50.0f));
		res->arrowImg->Draw(draw, src);
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
