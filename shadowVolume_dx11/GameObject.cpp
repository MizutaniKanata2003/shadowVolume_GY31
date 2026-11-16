#include "GameObject.h"
#include "direct3d.h"

using namespace DirectX;

void GameObject::Draw()
{
	ID3D11DeviceContext* d3dContext = D3D::Get()->GetContext();

	D3D::ConstBuffer cb;
	XMMATRIX matrixWorld, matrixView, matrixProj;

	// View変換行列作成　＝　カメラ用行列
	// eye = カメラの存在する位置
	XMVECTOR eye = XMVectorSet(0, 0, -3, 0);
	// focus = カメラが写そうと狙っている位置
	XMVECTOR focus = XMVectorSet(0, 0, 1, 0);
	// up = カメラの上方向を現すベクトル
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	// View変換行列作成の関数を呼び出す
	matrixView = XMMatrixLookAtLH(eye, focus, up);
	matrixView = Camera::GetMatrixView();
	// 平行投影の行列作成
// 
	// 引数①②：映し出す面の横縦の長さ
	// 引数③④：映し出す空間の奥行（手前と最奥）
//	float scaleScreen = 1.0f;
//	matrixProj = XMMatrixOrthographicLH(
//		4.0f * scaleScreen, 3.0f * scaleScreen, 0.0f, 3.0f);

	// 透視投影　＝　遠近感が出る
	matrixProj = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(45), 4.0f / 3.0f, 0.01f, 100.0f);

	// ワールド変換行列の作成
	// 移動行列
	XMMATRIX matrixMove = XMMatrixTranslation(
		position.x, position.y, position.z);
	// 拡大縮小行列
	XMMATRIX matrixScale = XMMatrixScaling(scaling.x, scaling.y, scaling.z);
	// 回転行列
	// XYZの三軸の回転角度を指定して回転させる方法　＝　オイラー角
	XMMATRIX matrixRotateY = XMMatrixRotationY(XMConvertToRadians(rotation.y));
	XMMATRIX matrixRotateX = XMMatrixRotationX(XMConvertToRadians(rotation.x));
	XMMATRIX matrixRotateZ = XMMatrixRotationZ(XMConvertToRadians(rotation.z));

	// 行列は掛け算で合成できる （注）掛ける順番が影響する
	matrixWorld =
		matrixScale *   // 拡大縮小行列
		matrixRotateX * matrixRotateY * matrixRotateZ *  // 回転行列
		matrixMove;  // 移動行列

	cb.matrixWorld = matrixWorld * matrixView * matrixProj;
	// 合成した行列の転置行列を作成する ※シェーダーとC++でメモリの並びが異なるため
	cb.matrixWorld = XMMatrixTranspose(cb.matrixWorld);

	// 行列をシェーダーに渡す
	d3dContext->UpdateSubresource(D3D::Get()->GetConstantBuffer(), 0, NULL,
		&cb, 0, 0);

	D3D::Model& mdl = model;

	// 今からDrawする頂点バッファ（モデル）を指定する
	UINT strides = D3D::Get()->GetVertexStride();
	UINT offsets = 0;
	d3dContext->IASetVertexBuffers(0, 1, &mdl.vertexBuffer,
		&strides, &offsets);

	// 描画に使うインデックスバッファを指定する
	d3dContext->IASetIndexBuffer(mdl.indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// ピクセルシェーダーにテクスチャを渡す
	d3dContext->PSSetShaderResources(0, 1, &mdl.texture);
	// 第１引数　→　描画する頂点数
	d3dContext->DrawIndexed(mdl.numIndex, 0, 0); // 描画命令
}


void GameObject::Update()
{

}
void GameObject::SetModel(D3D::Model& m)
{
	model = m;
}
