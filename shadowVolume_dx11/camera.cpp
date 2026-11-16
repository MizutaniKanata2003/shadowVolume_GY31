
#include "Game.h"
#include "GameObject.h"

//using namespace DirectX;


XMMATRIX Camera::matrixView;

Camera::Camera() {
	eyeX = 0;
	eyeY = 0;
	eyeZ = -3;
	focusX = 0;
	focusY = 0;
	focusZ = 1;
	upX = 0;
	upY = 1;
	upZ = 0;
}
Camera::~Camera() {

}
void Camera::Update() {

	if (GetAsyncKeyState(VK_UP) & 0x8000) { eyeY += 0.05f;  focusY += 0.05f; }// カーソル（上）
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) { eyeY -= 0.05f;  focusY -= 0.05f; }	// カーソル（下）
	if (GetAsyncKeyState(VK_LEFT) & 0x8000) focusX += 0.05f;	// カーソル（左）
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) focusX -= 0.05f;	// カーソル（右）

	// View変換行列作成　＝　カメラ用行列
	// eye = カメラの存在する位置
	XMVECTOR eye = XMVectorSet(eyeX, eyeY, eyeZ, 0);
	// focus = カメラが写そうと狙っている位置
	XMVECTOR focus = XMVectorSet(focusX, focusY, focusZ, 0);
	// up = カメラの上方向を現すベクトル
	XMVECTOR up = XMVectorSet(upX, upY, upZ, 0);
	// View変換行列作成の関数を呼び出す
	matrixView = XMMatrixLookAtLH(eye, focus, up);
}

XMMATRIX Camera::GetMatrixView() {
	return matrixView;
}
