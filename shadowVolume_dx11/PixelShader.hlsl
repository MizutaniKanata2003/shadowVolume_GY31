// ピクセルシェーダー

// ピクセルの情報の構造体（受け取り用）
struct PS_IN
{
    // float4型　→　float型が４つの構造体
    float4 pos : SV_POSITION; // ピクセルの画面上の座標
    float2 tex : TEXCOORD;  // ピクセルに対応するテクスチャ座標
    float4 normal : NORMAL; // ピクセル面の法線ベクトル
};

// グローバル変数の宣言
// ※シェーダーのグローバル変数は、C言語プログラムから渡された
// 　データを受け取るために使う。
Texture2D myTexture : register(t0); //テクスチャー
SamplerState mySampler : register(s0); //サンプラー

// 定数バッファ受け取り用
cbuffer ConstBuffer : register(b0)
{
    // ワールド変換行列
    matrix matrixWorld;
    // マテリアル色
    float4 materialDiffuse;
}
 

// ピクセルシェーダーのエントリポイント
float4 ps_main(PS_IN input) : SV_Target
{
    // Sample関数→テクスチャから指定したUVを元にピクセル色を取って来る
//    float4 color = myTexture.Sample(mySampler, input.tex);    
    // テクスチャ色とマテリアル色を混ぜる
//    return color * materialDiffuse;
    
    // 光の向きベクトル
    float3 lightVector = float3(1, -1, 1);
    // 環境光（アンビエント）
    float4 ambientColor = float4(0.1f, 0.1f, 0.1f, 0.0f);
    // ピクセル面の法線ベクトル
    float3 pixelNormal = input.normal.xyz;
    
    // ２つのベクトルを単位ベクトル化する
    lightVector = normalize(lightVector);
    pixelNormal = normalize(pixelNormal);
    
    // 内積を求める関数
    float diffusePower = dot(lightVector, pixelNormal);
    // 一定範囲に値を収める
    diffusePower = saturate(diffusePower);
    
    // 一定以下の光になってしまうのを防ぐ
    diffusePower = max(diffusePower, 0.2f);
    
    // テクスチャから取ってきたピクセル色
    float4 textureColor = myTexture.Sample(mySampler, input.tex);
    
    // テクスチャ色に光の当たる強さを掛ける
    float4 pixelColor = textureColor * diffusePower;
    // アルファ値が変化してしまうので戻す
    pixelColor.a = textureColor.a;
    // 環境光を加える
    pixelColor = pixelColor + ambientColor;
    
    return pixelColor;
}