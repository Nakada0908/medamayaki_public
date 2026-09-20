#pragma warning(disable:4996)
#pragma once

#include "GameEngine_Ver3_83.h"
#include "GameConfig.h"

namespace Game
{
	const string defGroupName("Scene");
	const string defName("Game");
	using ResultRank = GameConfig::Ranking::ResultRank;

	struct Result
	{
		int elapsedTenths;
		int gameOverCount;
		ResultRank rank;
	};

	void AddGameOver();
	int GetGameOverCount();
	float GetElapsedTimeSeconds();
	void VisitSRankTargetFurniture(int furnitureIndex);
	bool IsSRankTargetVisited(int targetIndex);
	int GetVisitedSRankTargetCount();
	bool IsSRankRouteComplete();
	ResultRank EvaluateResult(int elapsedTenths, int gameOverCount,
		bool sRankRouteComplete);
	const Result& GetLastResult();
	const char* GetResultRankText(ResultRank rank);

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

		DG::Font::SP hudFont;
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
	};
}
