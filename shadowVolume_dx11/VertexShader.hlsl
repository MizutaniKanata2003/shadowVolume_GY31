// 頂点シェーダー

// 頂点のデータを表す構造体（受け取り用）
struct VS_IN
{
    float4 pos : POSITION; // 位置座標が入る
    float2 tex : TEX;       // UV座標が入る
    float4 normal : NORMAL; // 頂点の法線ベクトル
};

// 頂点のデータを表す構造体（送信用） 
struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD;
    float4 normal : NORMAL; // 頂点の法線ベクトル
};

// グローバル変数の宣言
// 定数バッファ受け取り用
cbuffer ConstBuffer : register(b0)
{
   // ワールド変換行列
    matrix matrixWorld;
    // マテリアル色
    float4 materialDiffuse;
}
 

// 頂点シェーダーのエントリポイント 
VS_OUT vs_main( VS_IN input )
{
    VS_OUT output;
 
    // ワールド変換行列を頂点に掛ける
    output.pos = mul(input.pos, matrixWorld);
        
    output.tex = input.tex;
    
    output.normal = input.normal;
        
    return output;
}