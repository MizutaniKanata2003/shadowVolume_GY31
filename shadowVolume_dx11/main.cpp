#undef UNICODE  // Unicodeではなく、マルチバイト文字を使う

#include <Windows.h>
#include "direct3d.h"
#include "Game.h"

// マクロ定義
#define CLASS_NAME    "DX21Smpl"// ウインドウクラスの名前
#define WINDOW_NAME  "スケルトンプログラム"// ウィンドウの名前

#define SCREEN_WIDTH (640)	// ウインドウの幅
#define SCREEN_HEIGHT (480)	// ウインドウの高さ

// 関数のプロトタイプ宣言
LRESULT CALLBACK WndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

// グローバル変数の宣言
Game* gGame;


// エントリポイント＝一番最初に実行される関数
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,int nCmdShow)
{
	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = CLASS_NAME;
	wc.hIconSm = NULL;

	RegisterClassEx(&wc);

	HWND hWnd;
	hWnd = CreateWindowEx(0,// 拡張ウィンドウスタイル
		CLASS_NAME,// ウィンドウクラスの名前
		WINDOW_NAME,// ウィンドウの名前
		WS_OVERLAPPEDWINDOW,// ウィンドウスタイル
		CW_USEDEFAULT,// ウィンドウの左上Ｘ座標
		CW_USEDEFAULT,// ウィンドウの左上Ｙ座標 
		SCREEN_WIDTH,// ウィンドウの幅
		SCREEN_HEIGHT,// ウィンドウの高さ
		NULL,// 親ウィンドウのハンドル
		NULL,// メニューハンドルまたは子ウィンドウID
		hInstance,// インスタンスハンドル
		NULL);// ウィンドウ作成データ

	// 指定されたウィンドウの表示状態を設定(ウィンドウを表示)
	ShowWindow(hWnd,nCmdShow);
	// ウィンドウの状態を直ちに反映(ウィンドウのクライアント領域を更新)
	UpdateWindow(hWnd);

	// ゲームループに入る前に
	// DirectXの初期化をする
	D3D::Get()->Create(hWnd);

	// ゲームクラスの初期化処理
	gGame = new Game();

	// FPS表示用変数
	int fpsCounter = 0;
	long long oldTick = GetTickCount64();//現在時間を保存
	long long nowTick = oldTick; // 現在時間取得用

	// FPS固定用変数
	LARGE_INTEGER liWork; // 関数から値を受け取る用
	long long frequency; // 計測精度
	// 計測精度を取得
	QueryPerformanceFrequency(&liWork);
	frequency = liWork.QuadPart; // １秒あたりの解像度が入る
	// １フレームあたりのカウント値を計算
	long long numCount_1frame = frequency / 60; // 60FPSで計算
	// 現在時間（単位：カウント）を取得
	QueryPerformanceCounter(&liWork);
	long long oldCount = liWork.QuadPart;
	long long nowCount = oldCount;

	MSG msg;

	// ゲームループ
	for(;;){
		BOOL isAnyMessage = PeekMessage(&msg,NULL,0,0,PM_REMOVE);

		if(isAnyMessage)// 何かメッセージがあるか判定
		{
			DispatchMessage(&msg);

			if(msg.message == WM_QUIT){
				break;
			}
		}
		else{
			// 1/60秒経過したか？
			QueryPerformanceCounter(&liWork);
			nowCount = liWork.QuadPart; // 現在時間を取得（単位：カウント）
			if(nowCount >= oldCount + numCount_1frame){
				// ゲーム処理実行
				gGame->Update();
				gGame->Draw();
				fpsCounter++; // ゲームループ実行回数をカウント＋１
				oldCount = nowCount;
			}

			nowTick = GetTickCount64(); // 現在時間取得
			// １秒経過したか？
			if(nowTick >= oldTick + 1000){
				// FPSを表示する
				char str[32];
				wsprintfA(str,"FPS=%d",fpsCounter);
				SetWindowTextA(hWnd,str);
				// カウンターをリセット
				fpsCounter = 0;
				oldTick = nowTick;
			}
		}
	} // ゲームループの閉じカッコ

	// ゲームクラスの終了処理
	delete gGame;

	UnregisterClass(CLASS_NAME,hInstance);

	return (int)msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg){
	case WM_DESTROY:// ウィンドウ破棄のメッセージ
	PostQuitMessage(0);// “WM_QUIT”メッセージを送る　→　アプリ終了
	break;

	case WM_CLOSE:  // xボタンが押されたら
	DestroyWindow(hWnd);  // “WM_DESTROY”メッセージを送る
	break;

	// キーが押されたイベント
	case WM_KEYDOWN:
	break;

	// キーが離されたイベント
	case WM_KEYUP:
	break;

	default:
	// 上のcase以外の場合の処理を実行
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
	break;
	}

	return 0;
}








/*

struct TLVERTEX {
	float		x, y, z;			// 頂点座標（座標変換あり）
	DWORD		diffuse;			// ディフューズ
};

const int	LIGHT_DETAIL = 32;				// 点光源のディテールレベル

struct POINTLIGHT {							//----- 点光源
	int				status = 0;						// ステータス
	DirectX::XMFLOAT3 position;					// 座標
	DirectX::XMFLOAT3 base_cycle;					// 中心移動用回転値
	DirectX::XMFLOAT3 base_speed;					// 中心移動用回転速度値
	TLVERTEX		vertex[LIGHT_DETAIL + 2];	// 頂点データ
	float			cycle[LIGHT_DETAIL + 2];	// 頂点変化サインカーブ角度値
	float			speed[LIGHT_DETAIL + 2];	// 頂点変化サインカーブ角速度値
	float			range1, range2;				// 頂点変化サインカーブ変化値
};

POINTLIGHT	g_PointLight;				// 点光源オブジェクト



//-----------------------------------------------------------------------------
// 点光源オブジェクト
//-----------------------------------------------------------------------------
//----- メイン
void ExecPointLight()
{
	POINTLIGHT* pt;
	int			i;
	float		x;
	BYTE		col_r, col_g, col_b;

	pt = &g_PointLight;
	switch (pt->status) {
	case 0:			// 初期化
		// 頂点データを設定
		pt->base_cycle.x = rand() / 10.0f;			// 基準座標の回転値
		pt->base_cycle.y = rand() / 10.0f;			// 基準座標の回転値
		pt->base_cycle.z = rand() / 10.0f;			// 基準座標の回転値
		pt->base_speed.x = rand() / 4000000.0f;		// 基準座標の回転速度値
		pt->base_speed.y = rand() / 4000000.0f;		// 基準座標の回転速度値
		pt->base_speed.z = rand() / 4000000.0f;		// 基準座標の回転速度値
		pt->vertex[0].x = 0;		// 最初の頂点（＝中心）の座標
		pt->vertex[0].y = 0;
		pt->vertex[0].z = 0;
		pt->vertex[0].diffuse = D3DCOLOR_RGBA(0xff, 0xff, 0xff, 0xff);	// 中心は「白・不透明」
		col_r = rand() & 0xff;		// 光源全体をカラーを指定
		col_g = rand() & 0xff;
		col_b = rand() & 0xff;
		for (i = 0; i <= LIGHT_DETAIL; i++) {
			pt->vertex[i + 1].z = pt->vertex[0].z;
			pt->vertex[i + 1].diffuse = D3DCOLOR_RGBA(col_r, col_g, col_b, 0x00);
			pt->cycle[i + 1] = rand() / 1000.0f;
			pt->speed[i + 1] = rand() / 1000000.0f;
			pt->range1 = 150.0f + (rand() % 100);
			pt->range2 = 50.0f + (rand() % 50);
		}
		pt->cycle[i] = pt->cycle[1];	// 最後の頂点は最初の頂点と同じ
		pt->speed[i] = pt->speed[1];
		pt->status = 1;
		// THRU
	case 1:			// メイン
		for (i = 0; i <= LIGHT_DETAIL; i++) {
			x = cosf(pt->cycle[i + 1]);
			pt->vertex[i + 1].x = pt->vertex[0].x
				+ sinf(DirectX::XMConvertToRadians(i * 360 / LIGHT_DETAIL)) * (pt->range1 + x * pt->range2);
			pt->vertex[i + 1].y = pt->vertex[0].y
				+ cosf(DirectX::XMConvertToRadians(i * 360 / LIGHT_DETAIL)) * (pt->range1 + x * pt->range2);
			pt->cycle[i + 1] += pt->speed[i + 1];
		}
		pt->position.x = sinf(pt->base_cycle[0]) * 1500.0f;
		pt->position.y = sinf(pt->base_cycle[1]) * 200.0f + 1500.0f;
		pt->position.z = sinf(pt->base_cycle[2]) * 1500.0f;
		pt->base_cycle += pt->base_speed;		// 基準座標を移動（回転）させる
		break;
	default:
		break;
	}
}

//----- 描画
void DrawPointLight()
{
	DirectX::XMMATRIX	matWorld;										// ワールドマトリックス
	g_pD3DDevice->SetFVF(FVF_TLVERTEX);							// 頂点フォーマット設定
	g_pD3DDevice->SetTexture(0, NULL);							// テクスチャ設定
	// ビルボード用に回転部分はビューマトリックスの逆行列を設定（転置行列でＯＫ）
	matWorld._11 = g_MatView._11;		matWorld._12 = g_MatView._21;		matWorld._13 = g_MatView._31;
	matWorld._21 = g_MatView._12;		matWorld._22 = g_MatView._22;		matWorld._23 = g_MatView._32;
	matWorld._31 = g_MatView._13;		matWorld._32 = g_MatView._23;		matWorld._33 = g_MatView._33;
	matWorld._14 = matWorld._24 = matWorld._34 = 0.0f;
	matWorld._41 = g_PointLight.position.x;
	matWorld._42 = g_PointLight.position.y;
	matWorld._43 = g_PointLight.position.z;
	matWorld._44 = 1.0f;
	g_pD3DDevice->SetTransform(D3DTS_WORLD, &matWorld);						// ワールドマトリックスをセット
	g_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);				// αブレンディング設定
	g_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	// 描画
	g_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, LIGHT_DETAIL, &g_PointLight.vertex[0], sizeof(TLVERTEX));
}

*/