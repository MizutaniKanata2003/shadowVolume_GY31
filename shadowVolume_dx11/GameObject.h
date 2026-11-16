#pragma once

#include "Camera.h"
#include "direct3d.h"

// １つの描画物を表すクラス
class GameObject
{

protected:
	// メンバー変数
	D3D::Model model; // 描画するべきモデルデータ

public:
	// 機能

	virtual void Update(); // 更新関数（移動などオブジェクトの変化）

	virtual void Draw(); // 描画関数

	void SetModel(D3D::Model& m);
	D3D::Model& GetModel() { return model; }

	// このGameObjectの仮想世界での位置（ワールド座標）
	DirectX::XMFLOAT3 position = {};

	// オイラー角
	DirectX::XMFLOAT3 rotation = {};

	// 拡大縮小率
	DirectX::XMFLOAT3 scaling = { 1, 1, 1 };

};

