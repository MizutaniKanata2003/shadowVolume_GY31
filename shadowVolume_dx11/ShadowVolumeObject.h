#pragma once

#include "GameObject.h"
#include <DirectXMath.h>

struct SHADOWVOLUME
{
	DirectX::XMVECTOR Vertex[32768];
	int iNumVertex;
};

HRESULT BuildShadowVolume(
	SHADOWVOLUME* pShadow,
	D3D::Model& model,
	DirectX::XMVECTOR vLight);

// ShadowVolumeObject.h
void DrawShadow(ID3D11Buffer* fullscreenVB);

void DrawShadowVolume(SHADOWVOLUME* shadow);

void RenderStencilShadow(SHADOWVOLUME* pShadow);

extern ID3D11Buffer* g_shadowVolumeVB;

// １つの描画物を表すクラス
class ShadowVolumeObject :public GameObject
{

private:
	// メンバー変数
	//	D3D::Model model; // 描画するべきモデルデータ

public:
	// 機能

	void Update(); // 更新関数（移動などオブジェクトの変化）

	void Draw(); // 描画関数
};

