#include "ShadowVolumeObject.h"
#include "direct3d.h"

using namespace DirectX;

struct ShadowScreenVertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
};

void ShadowVolumeObject::Draw()
{
	ID3D11DeviceContext* d3dContext = D3D::Get()->GetContext();

	D3D::ConstBuffer cb;
	XMMATRIX matrixWorld,matrixView,matrixProj;

	// View変換行列作成　＝　カメラ用行列
	// eye = カメラの存在する位置
	XMVECTOR eye = XMVectorSet(0.0f,0.0f,-3.0f,0.0f);
	// focus = カメラが写そうと狙っている位置
	XMVECTOR focus = XMVectorSet(0.0f,0.0f,1.0f,0.0f);
	// up = カメラの上方向を現すベクトル
	XMVECTOR up = XMVectorSet(0.0f,1.0f,0.0f,0.0f);
	// View変換行列作成の関数を呼び出す
	matrixView = XMMatrixLookAtLH(eye,focus,up);
	matrixView = Camera::GetMatrixView();
	//	平行投影の行列作成
	//	引数①②：映し出す面の横縦の長さ
	//	引数③④：映し出す空間の奥行（手前と最奥）
	//	float scaleScreen = 1.0f;
	//	matrixProj = XMMatrixOrthographicLH(
	//  4.0f * scaleScreen, 3.0f * scaleScreen, 0.0f, 3.0f);

	// 透視投影　＝　遠近感が出る
	matrixProj = XMMatrixPerspectiveFovLH(XMConvertToRadians(45),4.0f / 3.0f,0.01f,100.0f);

	// ワールド変換行列の作成
	// 移動行列
	XMMATRIX matrixMove = XMMatrixTranslation(position.x,position.y,position.z);
	// 拡大縮小行列
	XMMATRIX matrixScale = XMMatrixScaling(scaling.x,scaling.y,scaling.z);
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
	d3dContext->UpdateSubresource(D3D::Get()->GetConstantBuffer(),0,NULL,&cb,0,0);

	D3D::Model& mdl = model;

	// 今からDrawする頂点バッファ（モデル）を指定する
	UINT strides = D3D::Get()->GetVertexStride();
	UINT offsets = 0;
	d3dContext->IASetVertexBuffers(0,1,&mdl.vertexBuffer,&strides,&offsets);

	// 描画に使うインデックスバッファを指定する
	d3dContext->IASetIndexBuffer(mdl.indexBuffer,DXGI_FORMAT_R16_UINT,0);

	// ピクセルシェーダーにテクスチャを渡す
	d3dContext->PSSetShaderResources(0,1,&mdl.texture);
	// 第１引数　→　描画する頂点数
	d3dContext->DrawIndexed(mdl.numIndex,0,0); // 描画命令

}

void ShadowVolumeObject::Update() {}

#define	SHADOW_LENGTH	1000.0f					// シャドウボリューム長

struct SHADOWVERTEX
{
	float		x,y,z,rhw;		// 頂点座標（座標変換なし）
	DWORD		diffuse;			// ディフューズ
};


//-----------------------------------------------------------------------------
// エッジリストへの追加
//-----------------------------------------------------------------------------
void AddEdge(WORD* pEdge,DWORD* pdwNumEdge,WORD v0,WORD v1)
{
	// 三角形間の共有エッジの削除（2度目の登録時）
	for(DWORD i = 0; i < *pdwNumEdge; ++i){
		if((pEdge[i * 2 + 0] == v0 && pEdge[i * 2 + 1] == v1) ||
			(pEdge[i * 2 + 0] == v1 && pEdge[i * 2 + 1] == v0)){
			// 末尾のエッジで上書きして消去
			if(*pdwNumEdge > 1){
				pEdge[i * 2 + 0] = pEdge[(*pdwNumEdge - 1) * 2 + 0];
				pEdge[i * 2 + 1] = pEdge[(*pdwNumEdge - 1) * 2 + 1];
			}
			-- * pdwNumEdge;
			return;
		}
	}
	// エッジリストへ追加
	pEdge[*pdwNumEdge * 2 + 0] = v0;
	pEdge[*pdwNumEdge * 2 + 1] = v1;
	++*pdwNumEdge;
}


//-----------------------------------------------------------------------------
// メッシュと光源へのベクトルよりシャドウボリュームを生成
//-----------------------------------------------------------------------------
HRESULT BuildShadowVolume(SHADOWVOLUME* pShadow,D3D::Model& model,XMVECTOR vLight)
{
	// 頂点バッファ、インデックスバッファの取得
	DWORD dwNumFace = (DWORD)model.indices.size() / 3;

	// ワーク用エッジリスト確保
	WORD* pEdge = new WORD[dwNumFace * 6];
	if(pEdge == NULL){
		delete[] pEdge;
		return S_OK;
	}

	DWORD	dwNumEdge = 0;
	DWORD	i;

	// 光源を向いている面を検索
	for(i = 0; i < dwNumFace; i++){
		WORD wFace0 = model.indices[i * 3 + 0];
		WORD wFace1 = model.indices[i * 3 + 1];
		WORD wFace2 = model.indices[i * 3 + 2];
		XMVECTOR v0 = XMLoadFloat3(&model.vertices[wFace0].position);
		XMVECTOR v1 = XMLoadFloat3(&model.vertices[wFace1].position);
		XMVECTOR v2 = XMLoadFloat3(&model.vertices[wFace2].position);
		// 光源を向いている面か?  
		XMVECTOR	vCross1(v2 - v1);
		XMVECTOR	vCross2(v1 - v0);
		XMVECTOR	vNormal;
		vNormal = XMVector3Cross(vCross1,vCross2);
		float rv{};
		DirectX::XMStoreFloat(&rv,DirectX::XMVector3Dot(vNormal,vLight)
		);
		if(rv <= 0.0f){
			// エッジリストに3辺を追加
			AddEdge(pEdge,&dwNumEdge,wFace0,wFace1);
			AddEdge(pEdge,&dwNumEdge,wFace1,wFace2);
			AddEdge(pEdge,&dwNumEdge,wFace2,wFace0);
		}
	}

	// シャドウボリュームの側面を追加
	pShadow->iNumVertex = 0;
	for(i = 0; i < dwNumEdge; i++){
		WORD index0 = pEdge[i * 2 + 0];
		WORD index1 = pEdge[i * 2 + 1];
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(&model.vertices[index0].position);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat3(&model.vertices[index1].position);
		XMVECTOR v3 = v1 - vLight * SHADOW_LENGTH;
		XMVECTOR v4 = v2 - vLight * SHADOW_LENGTH;

		// 矩形の追加
		if(_countof(pShadow->Vertex) < pShadow->iNumVertex + 6) break;
		pShadow->Vertex[pShadow->iNumVertex++] = v1;
		pShadow->Vertex[pShadow->iNumVertex++] = v2;
		pShadow->Vertex[pShadow->iNumVertex++] = v3;
		pShadow->Vertex[pShadow->iNumVertex++] = v2;
		pShadow->Vertex[pShadow->iNumVertex++] = v4;
		pShadow->Vertex[pShadow->iNumVertex++] = v3;
	}

	// ワーク用エッジリスト解放
	delete[] pEdge;

	return S_OK;
}

//-----------------------------------------------------------------------------
// 影をステンシルバッファに描画
//-----------------------------------------------------------------------------
void RenderStencilShadow(SHADOWVOLUME* pShadow)
{
	ID3D11RasterizerState* rsCullFront = nullptr;

	D3D11_RASTERIZER_DESC rsDesc = {};

	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_FRONT;
	rsDesc.DepthClipEnable = TRUE;

	D3D::Get()->GetDevice()->CreateRasterizerState(&rsDesc,&rsCullFront);

	ID3D11RasterizerState* rsCullBack = nullptr;

	rsDesc.CullMode = D3D11_CULL_BACK;

	D3D::Get()->GetDevice()->CreateRasterizerState(&rsDesc,&rsCullBack);

	auto context = D3D::Get()->GetContext();

	D3D11_MAPPED_SUBRESOURCE mapped;

	context->Map(g_shadowVolumeVB,0,D3D11_MAP_WRITE_DISCARD,0,&mapped);

	memcpy(mapped.pData,pShadow->Vertex,sizeof(DirectX::XMVECTOR) * pShadow->iNumVertex);

	context->Unmap(g_shadowVolumeVB,0);

	UINT stride = sizeof(XMVECTOR);
	UINT offset = 0;

	context->IASetVertexBuffers(0,1,&g_shadowVolumeVB,&stride,&offset);

	ID3D11DepthStencilState* dsState;

	D3D11_DEPTH_STENCIL_DESC dsDesc = {};

	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

	dsDesc.StencilEnable = TRUE;

	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;

	D3D::Get()->GetDevice()->CreateDepthStencilState(&dsDesc,&dsState);

	context->OMSetDepthStencilState(dsState,0);

	ID3D11BlendState* blendState = nullptr;

	D3D11_BLEND_DESC blendDesc = {};

	blendDesc.RenderTarget[0].RenderTargetWriteMask = 0;

	D3D::Get()->GetDevice()->CreateBlendState(&blendDesc,&blendState);

	float factor[4]{};

	context->OMSetBlendState(blendState,factor,0xffffffff);

	// FrontFace
	context->RSSetState(rsCullFront);

	context->Draw(pShadow->iNumVertex,0);

	// BackFace
	context->RSSetState(rsCullBack);

	context->Draw(pShadow->iNumVertex,0);

	context->RSSetState(nullptr);

	context->OMSetBlendState(nullptr,nullptr,0xffffffff);

	context->OMSetDepthStencilState(nullptr,0);
}

//-----------------------------------------------------------------------------
// 影を描画
//-----------------------------------------------------------------------------


void DrawShadow(ID3D11Buffer* fullscreenVB)
{
	if(!fullscreenVB)return;

	auto context = D3D::Get()->GetContext();

	ID3D11DepthStencilState* dsState = nullptr;

	D3D11_DEPTH_STENCIL_DESC dsDesc = {};

	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

	dsDesc.StencilEnable = TRUE;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;

	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;

	dsDesc.BackFace = dsDesc.FrontFace;

	D3D::Get()->GetDevice()->CreateDepthStencilState(&dsDesc,&dsState);

	context->OMSetDepthStencilState(dsState,0);

	UINT stride = D3D::Get()->GetVertexStride();

	UINT offset = 0;

	context->IASetVertexBuffers(0,1,&fullscreenVB,&stride,&offset);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11BlendState* blend = nullptr;

	D3D11_BLEND_DESC bd = {};
	bd.RenderTarget[0].BlendEnable = FALSE;
	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	D3D::Get()->GetDevice()->CreateBlendState(&bd,&blend);

	context->OMSetBlendState(blend,nullptr,0xffffffff);

	D3D::ConstBuffer cb{};

	/* 黒塗りできるかチェック
	cb.matrixWorld = XMMatrixIdentity();
	cb.matrixWorld = XMMatrixTranspose(cb.matrixWorld);

	context->UpdateSubresource(
		D3D::Get()->GetConstantBuffer(),
		0,
		NULL,
		&cb,
		0,
		0);
	*/

	context->Draw(6,0);

	context->OMSetDepthStencilState(nullptr,0);
}

//-----------------------------------------------------------------------------
//シャドウボリュームを描画（シャドウボリューム見るようなので実際は使わなくてよい）
//-----------------------------------------------------------------------------
void DrawShadowVolume(SHADOWVOLUME* shadow)
{
	if(!g_shadowVolumeVB)return;

	auto context = D3D::Get()->GetContext();
	D3D11_MAPPED_SUBRESOURCE mapped;

	context->Map(g_shadowVolumeVB,0,D3D11_MAP_WRITE_DISCARD,0,&mapped);

	memcpy(mapped.pData,shadow->Vertex,sizeof(DirectX::XMVECTOR) * shadow->iNumVertex);

	context->Unmap(g_shadowVolumeVB,0);

	UINT stride = sizeof(DirectX::XMVECTOR);
	UINT offset = 0;

	context->IASetVertexBuffers(0,1,&g_shadowVolumeVB,&stride,&offset);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	context->Draw(shadow->iNumVertex,0);
}