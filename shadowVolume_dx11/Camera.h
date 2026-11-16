#pragma once

#include "direct3d.h"
using namespace DirectX;

class Camera
{
	float eyeX, eyeY, eyeZ;
	float focusX, focusY, focusZ;
	float upX, upY, upZ;
	static XMMATRIX matrixView;

public:
	Camera();
	~Camera();
	void Update();
	static XMMATRIX GetMatrixView();
};
