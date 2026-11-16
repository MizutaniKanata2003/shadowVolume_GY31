
#include "Game.h"
//#include "GameObject.h"
#include "ShadowVolumeObject.h"

using namespace DirectX;

DirectX::XMFLOAT3 base_cycle;
DirectX::XMFLOAT3 base_speed;
DirectX::XMFLOAT3 donuts_rot_speed[16];

Game::Game()
{
	//カメラオブジェクト
	cam = new Camera();

	// GameObjectを生成する
	land = new GameObject();
	for(int i = 0; i < 16; i++)donuts[i] = new GameObject();
	for(int i = 0; i < 16; i++)donuts_shadow[i] = new ShadowVolumeObject();
	pointLight = new GameObject();

	// モデル読み込み
	D3D::Model model = D3D::LoadObjModel(L"asset/land3.obj");
	land->SetModel(model);
	model = D3D::LoadObjModel(L"asset/donuts2.obj");
	for(int i = 0; i < 16; i++)donuts[i]->SetModel(model);
	for(int i = 0; i < 16; i++)donuts_shadow[i]->SetModel(model);
	model = D3D::LoadObjModel(L"asset/donuts2.obj");
	pointLight->SetModel(model);

	// 初期位置設定
	land->position = {0.0f,-5.0f,15.0f};
	land->scaling = {0.01,0.01,0.01};

	for(int i = 0; i < 16; i++){
		//	donuts[i]->position.x = 0.0f;
		donuts[i]->scaling = {0.001,0.001,0.001};
		donuts[i]->position.x = ((rand() % 20) - 10) * 0.1f;		// 座標
		donuts[i]->position.y = ((rand() % 10) - 25) * 0.1f;
		donuts[i]->position.z = ((rand() % 20) - 10) * 0.1f;
		donuts[i]->rotation.x = 0.0f;
		donuts[i]->rotation.y = 0.0f;
		donuts[i]->rotation.z = 0.0f;
		donuts_rot_speed[i].x = ((rand() % 100) - 50) / 100.0f;		// 回転速度
		donuts_rot_speed[i].y = ((rand() % 100) - 50) / 100.0f;
		donuts_rot_speed[i].z = ((rand() % 100) - 50) / 100.0f;
		donuts_shadow[i]->scaling = {0.001,0.001,0.001};
	}
	pointLight->scaling = {0.01,0.01,0.01};
	base_cycle.x = rand() / 10.0f;			// 基準座標の回転値
	base_cycle.y = rand() / 10.0f;			// 基準座標の回転値
	base_cycle.z = rand() / 10.0f;			// 基準座標の回転値
	base_speed.x = rand() / 4000000.0f;		// 基準座標の回転速度値
	base_speed.y = rand() / 4000000.0f;		// 基準座標の回転速度値
	base_speed.z = rand() / 4000000.0f;		// 基準座標の回転速度値

}

void Game::Update()
{
	for(int i = 0; i < 16; i++){
		donuts[i]->rotation.x += donuts_rot_speed[i].x;
		donuts[i]->rotation.y += donuts_rot_speed[i].y;
		donuts[i]->rotation.z += donuts_rot_speed[i].z;
	}
	pointLight->position.x = sinf(base_cycle.x) * 1.0f;
	pointLight->position.y = sinf(base_cycle.y) * 2.0f - 1.5f;
	pointLight->position.z = sinf(base_cycle.z) * 1.5f;
	base_cycle.x += base_speed.x;		// 基準座標を移動（回転）させる
	base_cycle.y += base_speed.y;		// 基準座標を移動（回転）させる
	base_cycle.z += base_speed.z;		// 基準座標を移動（回転）させる

	for(int i = 0; i < 16; i++)donuts_shadow[i]->Update();

	cam->Update();
}

void Game::Draw()
{
	D3D::Model shadowScreen;//NEW
	shadowScreen = D3D::CreateSquare({0.0f,0.0f},{2.0f,2.0f},{1.0f,1.0f});

	ID3D11DeviceContext* d3dContext = D3D::Get()->GetContext();

	D3D::Get()->ClearScreen();

	// 各GameObjectの描画
	land->Draw();
	for(int i = 0; i < 16; i++)donuts[i]->Draw();
	for(int i = 0; i < 16; i++){ donuts_shadow[i]->Draw(); }
	pointLight->Draw();

	for(int i = 0; i < 16; i++){
		SHADOWVOLUME shadow;

		XMVECTOR lightDir = XMVector3Normalize(XMVectorSet(pointLight->position.x,pointLight->position.y,pointLight->position.z,0.0f));

		BuildShadowVolume(&shadow,donuts[i]->GetModel(),lightDir);

		RenderStencilShadow(&shadow);

		//DrawShadowVolume(&shadow);
	}

	DrawShadow(shadowScreen.vertexBuffer);

	D3D::Get()->UpdateScreen();
}

Game::~Game()
{
	SAFE_RELEASE(land->GetModel().texture);
	SAFE_RELEASE(land->GetModel().vertexBuffer);
	SAFE_RELEASE(land->GetModel().indexBuffer);

	for(int i = 0; i < 16; i++){
		SAFE_RELEASE(donuts[i]->GetModel().texture);
		SAFE_RELEASE(donuts[i]->GetModel().vertexBuffer);
		SAFE_RELEASE(donuts[i]->GetModel().indexBuffer);
	}
	for(int i = 0; i < 16; i++){
		SAFE_RELEASE(donuts_shadow[i]->GetModel().texture);
		SAFE_RELEASE(donuts_shadow[i]->GetModel().vertexBuffer);
		SAFE_RELEASE(donuts_shadow[i]->GetModel().indexBuffer);
	}
	SAFE_RELEASE(pointLight->GetModel().texture);
	SAFE_RELEASE(pointLight->GetModel().vertexBuffer);
	SAFE_RELEASE(pointLight->GetModel().indexBuffer);

	// 上のGetModel呼び出しが終わってからdeleteする
	delete land;
	for(int i = 0; i < 16; i++)delete donuts[i];
	for(int i = 0; i < 16; i++)delete donuts_shadow[i];
	delete pointLight;
}