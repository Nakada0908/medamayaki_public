#include "MyPG.h"
#include "Task_Game.h"
#include "Task_GameBG.h"
#include "Task_Player.h"
#include "Task_Ending.h"
#include "GameStage.h"

#include <cstdio>

namespace Game
{
	namespace
	{
		const int UpdatesPerTenthSecond =
			GameConfig::Time::UpdatesPerSecond / GameConfig::Time::TenthsPerSecond;
		int gameOverCount = 0;
		int elapsedUpdateCount = 0;
		unsigned int sRankVisitedMask = 0;
		Result lastResult = { 0, 0, ResultRank::C };
	}

	void AddGameOver()
	{
		++gameOverCount;
	}

	int GetGameOverCount()
	{
		return gameOverCount;
	}

	float GetElapsedTimeSeconds()
	{
		return elapsedUpdateCount /
			static_cast<float>(GameConfig::Time::UpdatesPerSecond);
	}

	void VisitSRankTargetFurniture(int furnitureIndex)
	{
		const int targetIndex =
			GameStage::SRankTargetIndexForFurniture(furnitureIndex);
		if (targetIndex >= 0) {
			sRankVisitedMask |= 1u << targetIndex;
		}
	}

	bool IsSRankTargetVisited(int targetIndex)
	{
		if (targetIndex < 0 || targetIndex >= GameStage::SRankTargetCount()) {
			return false;
		}
		return (sRankVisitedMask & (1u << targetIndex)) != 0;
	}

	int GetVisitedSRankTargetCount()
	{
		int visitedCount = 0;
		for (int i = 0; i < GameStage::SRankTargetCount(); ++i) {
			if (IsSRankTargetVisited(i)) {
				++visitedCount;
			}
		}
		return visitedCount;
	}

	bool IsSRankRouteComplete()
	{
		return GetVisitedSRankTargetCount() == GameStage::SRankTargetCount();
	}

	ResultRank EvaluateResult(int elapsedTenths, int gameOverCount,
		bool sRankRouteComplete)
	{
		for (int i = 0; i < GameConfig::Ranking::RuleCount; ++i) {
			const GameConfig::Ranking::Rule& rule = GameConfig::Ranking::Rules[i];
			const bool withinTime =
				rule.timeLimitTenths == GameConfig::Ranking::NoLimit ||
				elapsedTenths <= rule.timeLimitTenths;
			const bool withinGameOverCount =
				rule.gameOverLimit == GameConfig::Ranking::NoLimit ||
				gameOverCount <= rule.gameOverLimit;
			const bool completedRequiredRoute =
				!rule.needsFullSRoute || sRankRouteComplete;
			if (withinTime && withinGameOverCount && completedRequiredRoute) {
				return rule.rank;
			}
		}
		return ResultRank::C;
	}

	const Result& GetLastResult()
	{
		return lastResult;
	}

	const char* GetResultRankText(ResultRank rank)
	{
		for (int i = 0; i < GameConfig::Ranking::RuleCount; ++i) {
			const GameConfig::Ranking::Rule& rule = GameConfig::Ranking::Rules[i];
			if (rule.rank == rank) {
				return rule.text;
			}
		}
		return GameConfig::Ranking::Rules[
			GameConfig::Ranking::RuleCount - 1].text;
	}

	Resource::WP Resource::instance;

	bool Resource::Initialize()
	{
		hudFont = DG::Font::Create("Arial", 18, 42, FW_BOLD, ANSI_CHARSET);
		return hudFont != nullptr;
	}

	bool Resource::Finalize()
	{
		hudFont.reset();
		return true;
	}

	bool Object::Initialize()
	{
		__super::Initialize(defGroupName, defName, true);
		res = Resource::Create();

		ge->GameClearFlag = false;
		gameOverCount = 0;
		elapsedUpdateCount = 0;
		sRankVisitedMask = 0;
		lastResult = { 0, 0, ResultRank::C };
		ge->camera2D = ML::Box2D(0, 0, GameStage::ScreenWidth, GameStage::ScreenHeight);

		GameBG::Object::Create(true);
		Player::Object::Create(true);
		return res != nullptr;
	}

	bool Object::Finalize()
	{
		// Background and player share the Game group.
		ge->KillAll_G(GameBG::defGroupName, false);
		ge->camera2D = ML::Box2D(0, 0, GameStage::ScreenWidth, GameStage::ScreenHeight);
		res.reset();

		if (!ge->QuitFlag() && nextTaskCreate) {
			Ending::Object::Create(true);
		}
		return true;
	}

	void Object::UpDate()
	{
		if (ge->GameClearFlag) {
			lastResult.elapsedTenths = elapsedUpdateCount / UpdatesPerTenthSecond;
			lastResult.gameOverCount = gameOverCount;
			lastResult.rank = EvaluateResult(
				lastResult.elapsedTenths, lastResult.gameOverCount,
				IsSRankRouteComplete());
			Kill();
			return;
		}

		++elapsedUpdateCount;
	}

	void Object::Render2D_AF()
	{
		const int totalTenths = elapsedUpdateCount / UpdatesPerTenthSecond;
		const int tenthsPerMinute = GameConfig::Time::SecondsPerMinute *
			GameConfig::Time::TenthsPerSecond;
		const int minutes = totalTenths / tenthsPerMinute;
		const int seconds =
			(totalTenths / GameConfig::Time::TenthsPerSecond) %
			GameConfig::Time::SecondsPerMinute;
		const int tenths = totalTenths % GameConfig::Time::TenthsPerSecond;

		char timeText[32];
		char gameOverText[32];
		sprintf_s(timeText, sizeof(timeText), "TIME  %02d:%02d.%d",
			minutes, seconds, tenths);
		sprintf_s(gameOverText, sizeof(gameOverText), "GAME OVER  %d",
			gameOverCount);

		const ML::Box2D timeDraw(40, 30, 600, 50);
		const ML::Box2D gameOverDraw(40, 80, 600, 50);
		res->hudFont->DrawF(timeDraw, timeText, DG::Font::x4,
			ML::Color(1, 1, 1, 1), ML::Color(1, 0, 0, 0));
		res->hudFont->DrawF(gameOverDraw, gameOverText, DG::Font::x4,
			ML::Color(1, 1, 1, 1), ML::Color(1, 0, 0, 0));
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
