#pragma once

// ゲームバランスの調整項目をまとめたファイルです。
// レベルデザイン時は、各タスクの処理ではなく基本的にこのファイルを変更してください。
namespace GameConfig
{
	namespace Egg
	{
		// 卵の見た目の大きさ（幅・高さ）です。
		// 画面上の卵だけを大きく／小さくしたい場合は DrawSize を変更してください。
		const int DrawSize = 100;

		// tamago.png の元画像サイズです。
		// 画像ファイル自体を別サイズのものへ差し替えたときだけ変更してください。
		const int TextureSize = 100;

		// 卵の当たり判定の半径です。
		// DrawSize の半分より小さくすると、壁際を通り抜けやすくなります。
		// 現在は見た目より判定を小さくして、狭い通路を遊びやすくしています。
		const float CollisionRadius = 20.0f;

		// 発射後の卵の移動速度です。大きいほど速く飛びます。
		const float ShotSpeed = 15.0f;

		// 1回の当たり判定で進める最大距離です。
		// 小さくすると高速移動時の判定が正確になりますが、処理回数が増えます。
		// ShotSpeed 以下の正の値を指定してください。
		const float MoveStep = 3.0f;
	}

	namespace Time
	{
		// ゲーム内時間の計算単位です。通常のレベル調整では変更しません。
		// 更新頻度を変えた場合は UpdatesPerSecond を実際の更新回数に合わせます。
		const int UpdatesPerSecond = 60;
		const int TenthsPerSecond = 10;
		const int SecondsPerMinute = 60;
	}

	namespace Ranking
	{
		enum class ResultRank
		{
			S,
			A,
			B,
			C
		};

		// timeLimitTenths : 制限時間（0.1秒単位）
		// gameOverLimit   : 許容するゲームオーバー回数
		// needsFullSRoute : Sルートの全家具を通る必要があるか
		// text            : リザルト画面に表示する文字
		// 必要な条件をすべて満たしたとき、そのランクになります。
		struct Rule
		{
			ResultRank rank;
			int timeLimitTenths;
			int gameOverLimit;
			bool needsFullSRoute;
			const char* text;
		};

		// 制限を設けない項目に指定する値です。
		const int NoLimit = -1;

		// ランク評価は上から順番に行います。Sは、緑丸を付けた家具を
		// すべて通り、1分30秒以内・ゲームオーバー10回以内でクリアした場合です。
		// 条件を調整するときは、下記の「分数」と「回数」を変更してください。
		// 例: Aを4分・ゲームオーバー5回以下にする場合は、Aの行を
		//     4 * Time::SecondsPerMinute * Time::TenthsPerSecond, 5
		// に変更します。
		// Cはどの条件にも該当しなかった場合の評価なので、最後のままにしてください。
		// ランクを追加する場合は ResultRank に項目を追加し、この表にも評価順で追加します。
		const Rule Rules[] = {
			{ ResultRank::S,
				(1 * Time::SecondsPerMinute + 30) * Time::TenthsPerSecond,
				10, true, "S" },
			{ ResultRank::A,
				2 * Time::SecondsPerMinute * Time::TenthsPerSecond,
				20, false, "A" },
			{ ResultRank::B,
				3 * Time::SecondsPerMinute * Time::TenthsPerSecond,
				30, false, "B" },
			{ ResultRank::C, NoLimit, NoLimit, false, "C" }
		};
		const int RuleCount = static_cast<int>(sizeof(Rules) / sizeof(Rules[0]));
	}
}
