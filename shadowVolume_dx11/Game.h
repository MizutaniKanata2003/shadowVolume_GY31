#pragma once

#include "direct3d.h"

// クラス型の前方宣言
// そのクラス型のポインタ変数を宣言したい場合に使える
class Camera;
class GameObject;





// ゲーム全体を表すクラス
class Game
{

public:
	// 列挙型
	// 画面の状態
	enum SCENE_STATE
	{
		NO_SCENE, // どの画面でもない状態
		TITLE, STAGE, RESULT,
	};

	// フェード状態
	enum FADE_STATE
	{
		NO_FADE, FADE_IN, FADE_OUT,
	};

private:
	SCENE_STATE scene = TITLE; // 現在の画面
	SCENE_STATE newScene = NO_SCENE;// フェード時に次の画面を持っておく

	FADE_STATE fadeState = NO_FADE; // 現在のフェード状態

	// メンバー変数
	// ゲームで使うモデル
//	D3D::Model model[2];
	GameObject* land;
	GameObject* donuts[16];
	GameObject* donuts_shadow[16];
	GameObject* pointLight;

	Camera* cam;

public:
	// メンバー関数
	// 初期化処理（コンストラクタ）
	Game();
	
	// ゲームループ毎に実行する処理を書く関数
	void Update();

	// 描画処理を担当
	void Draw();

	// 終了処理（デストラクタ）
	~Game();

	// 発射準備ができている弾を１つ返す関数
//	Shot* GetShotReady();

	// 現在の画面を変更する
	void SetScene(SCENE_STATE nextScene);

	// カメラ位置を持つ変数
	DirectX::XMFLOAT2 cameraPos = {0,0};

private:
	// キャラクター同士の当たり判定を行う関数
	void TestCollision();

	// 円と円の当たり判定結果を返す関数
//	bool IsHit(BOUNDING_CIRCLE bc1, BOUNDING_CIRCLE bc2);

	//各画面の更新処理関数
	void UpdateTitle(); // タイトル画面
	void UpdateStage(); // ステージ画面
	void UpdateResult(); // リザルト画面

};

// このクラスの変数が１つだけの場合は、
// ヘッダーにexternを書いてしまうと楽。
extern Game* gGame;
