#include "MyPG.h"
#include "Task_Ending.h"
#include "Task_Title.h"
#include "Task_Game.h"
#include "GameConfig.h"

#include <cstdio>

namespace Ending
{
	namespace
	{
		const int ResultEggTextureSize = 2048;
		const int ResultEggCellSize = ResultEggTextureSize / 2;

		// Shift-JIS byte strings keep the existing multibyte font renderer able
		// to draw Japanese independently of the source file's text encoding.
		const char ClearMessage[] =
			"\x83\x4E\x83\x8A\x83\x41\x82\xA8\x82\xDF\x82\xC5\x82\xC6\x82\xA4\x81\x49";
		const char ClearTimeLabel[] =
			"\x83\x4E\x83\x8A\x83\x41\x83\x5E\x83\x43\x83\x80";
		const char WallHitLabel[] =
			"\x95\xC7\x82\xC9\x82\xD4\x82\xC2\x82\xA9\x82\xC1\x82\xBD\x89\xF1\x90\x94";
		const char SRankMessage[] =
			"\x8D\xC5\x8D\x82\x82\xC9\x82\xA8\x82\xA2\x82\xB5\x82\xBB\x82\xA4\x82\xC8\x96\xDA\x8B\xCA\x8F\xC4\x82\xAB\x81\x49";
		const char ARankMessage[] =
			"\x82\xA8\x82\xA2\x82\xB5\x82\xBB\x82\xA4\x82\xC8\x96\xDA\x8B\xCA\x8F\xC4\x82\xAB\x81\x49";
		const char BRankMessage[] =
			"\x82\xDC\x82\xB8\x82\xDC\x82\xB8\x82\xCC\x96\xDA\x8B\xCA\x8F\xC4\x82\xAB\x81\x49";
		const char CRankMessage[] =
			"\x8F\xC5\x82\xB0\x82\xB7\x82\xAC\x82\xBD\x96\xDA\x8B\xCA\x8F\xC4\x82\xAB\x81\x63\x81\x63";
		const char* const RankMessages[] = {
			SRankMessage, ARankMessage, BRankMessage, CRankMessage
		};
		const char ReturnGuide[] =
			"\x5A\x83\x4C\x81\x5B\x82\xC5\x83\x5E\x83\x43\x83\x67\x83\x8B\x82\xC9\x96\xDF\x82\xE9\x82\xE6";
	}

	Resource::WP Resource::instance;

	bool Resource::Initialize()
	{
		backgroundImg = DG::Image::Create("./data/effect/black.png");
		resultEggImg = DG::Image::Create("./data/image/result_eggs.png");
		titleFont = DG::Font::Create("Yu Gothic UI", 43, 94, FW_BOLD, SHIFTJIS_CHARSET);
		rankFont = DG::Font::Create("Arial", 72, 160, FW_BOLD, ANSI_CHARSET);
		detailFont = DG::Font::Create("Yu Gothic UI", 31, 68, FW_NORMAL, SHIFTJIS_CHARSET);
		guideFont = DG::Font::Create("Yu Gothic UI", 22, 46, FW_NORMAL, SHIFTJIS_CHARSET);
		return backgroundImg && resultEggImg && titleFont && rankFont &&
			detailFont && guideFont;
	}

	bool Resource::Finalize()
	{
		guideFont.reset();
		detailFont.reset();
		rankFont.reset();
		titleFont.reset();
		resultEggImg.reset();
		backgroundImg.reset();
		return true;
	}

	bool Object::Initialize()
	{
		__super::Initialize(defGroupName, defName, true);
		res = Resource::Create();
		logoPosY = -1080;

		const Game::Result& result = Game::GetLastResult();
		elapsedTenths = result.elapsedTenths;
		gameOverCount = result.gameOverCount;
		rankText[0] = Game::GetResultRankText(result.rank)[0];
		rankText[1] = '\0';
		switch (result.rank) {
		case Game::ResultRank::S: resultEggIndex = 0; break;
		case Game::ResultRank::A: resultEggIndex = 1; break;
		case Game::ResultRank::B: resultEggIndex = 2; break;
		case Game::ResultRank::C:
		default: resultEggIndex = 3; break;
		}
		return res != nullptr && res->backgroundImg != nullptr &&
			res->resultEggImg != nullptr;
	}

	bool Object::Finalize()
	{
		res.reset();
		if (!ge->QuitFlag() && nextTaskCreate) {
			Title::Object::Create(true);
		}
		return true;
	}

	void Object::UpDate()
	{
		logoPosY += 27;
		if (logoPosY > 0) {
			logoPosY = 0;
		}

		const auto input = ge->in1->GetState();
		if (input.B1.down) {
			Kill();
		}
	}

	void Object::Render2D_AF()
	{
		ML::Box2D backgroundDraw(0, logoPosY, 1920, 1080);
		const ML::Box2D backgroundSrc(0, 0, 256, 256);
		res->backgroundImg->Rotation(0.0f, ML::Vec2(128.0f, 128.0f));
		res->backgroundImg->Draw(backgroundDraw, backgroundSrc);

		const int eggColumn = resultEggIndex % 2;
		const int eggRow = resultEggIndex / 2;
		const ML::Box2D eggSrc(
			eggColumn * ResultEggCellSize,
			eggRow * ResultEggCellSize,
			ResultEggCellSize, ResultEggCellSize);
		ML::Box2D eggDraw(990, 80 + logoPosY, 850, 850);
		res->resultEggImg->Rotation(0.0f,
			ML::Vec2(ResultEggCellSize * 0.5f, ResultEggCellSize * 0.5f));
		res->resultEggImg->Draw(eggDraw, eggSrc);

		const int totalSeconds =
			elapsedTenths / GameConfig::Time::TenthsPerSecond;
		const int minutes = totalSeconds / GameConfig::Time::SecondsPerMinute;
		const int seconds = totalSeconds % GameConfig::Time::SecondsPerMinute;
		const int tenths = elapsedTenths % GameConfig::Time::TenthsPerSecond;
		char timeText[64];
		char gameOverText[64];
		sprintf_s(timeText, sizeof(timeText), "%s    %d:%02d.%d",
			ClearTimeLabel,
			minutes, seconds, tenths);
		sprintf_s(gameOverText, sizeof(gameOverText), "%s    %d\x89\xF1",
			WallHitLabel, gameOverCount);

		const ML::Color white(1.0f, 1.0f, 1.0f, 1.0f);
		const ML::Color black(1.0f, 0.0f, 0.0f, 0.0f);
		const UINT left = DT_LEFT | DT_VCENTER | DT_SINGLELINE;
		const UINT centered = DT_CENTER | DT_VCENTER | DT_SINGLELINE;

		ML::Box2D titleDraw(50, 55 + logoPosY, 900, 145);
		ML::Box2D timeDraw(50, 295 + logoPosY, 900, 105);
		ML::Box2D gameOverDraw(50, 405 + logoPosY, 900, 105);
		ML::Box2D messageDraw(50, 630 + logoPosY, 930, 130);
		ML::Box2D guideDraw(50, 880 + logoPosY, 900, 80);
		ML::Box2D rankLabelDraw(50, 520 + logoPosY, 200, 75);
		ML::Box2D rankDraw(255, 475 + logoPosY, 180, 170);

		res->titleFont->DrawF(titleDraw, ClearMessage, DG::Font::x2,
			white, black, left);
		res->detailFont->DrawF(timeDraw, timeText, DG::Font::x4,
			white, black, left);
		res->detailFont->DrawF(gameOverDraw, gameOverText, DG::Font::x4,
			white, black, left);
		res->titleFont->DrawF(messageDraw, RankMessages[resultEggIndex], DG::Font::x2,
			white, black, left);
		res->guideFont->DrawF(guideDraw, ReturnGuide,
			DG::Font::x2, white, black, left);
		res->guideFont->DrawF(rankLabelDraw, "RANK", DG::Font::x2,
			white, black, left);
		res->rankFont->DrawF(rankDraw, rankText, DG::Font::x4,
			white, black, centered);
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
