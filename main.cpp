#include <Novice.h>
#include <imgui.h>
#include "MathFunction.h"
#include <string>

MathFunction mathFunc;

static const int kWindowWidth = 1280;
static const int kWindowHeight = 720;

// Catmull-rom曲線を描く
void DrawCatmullRom(const Vector3& controlPoint0, const Vector3& controlPoint1, const Vector3& controlPoint2, const Vector3& controlPoint3, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewPortMatrix, uint32_t color)
{
	const int segments = 100; // 曲線を分割するセグメント数
	for (int i = 0; i < segments; ++i) {
		float t1 = float(i) / segments;
		float t2 = float(i + 1) / segments;

		Vector3 point1 = mathFunc.CatmullRom(controlPoint0, controlPoint1, controlPoint2, controlPoint3, t1);
		Vector3 point2 = mathFunc.CatmullRom(controlPoint0, controlPoint1, controlPoint2, controlPoint3, t2);

		Vector3 screenPoint1 = mathFunc.Transform(point1, mathFunc.Multiply(viewProjectionMatrix, viewPortMatrix));
		Vector3 screenPoint2 = mathFunc.Transform(point2, mathFunc.Multiply(viewProjectionMatrix, viewPortMatrix));

		Novice::DrawLine(static_cast<int>(screenPoint1.x), static_cast<int>(screenPoint1.y), static_cast<int>(screenPoint2.x), static_cast<int>(screenPoint2.y), color);
	}
}

const char kWindowTitle[] = "提出用課題";

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	int prevMouseX = 0;
	int prevMouseY = 0;
	bool isDragging = false;

	Vector3 rotate = {};
	Vector3 translate = {};
	Vector3 cameraTranslate = { 0.0f, 1.9f, -6.49f };
	Vector3 cameraRotate = { 0.26f, 0.0f, 0.0f };

	// コントロールポイント初期化
	Vector3 controllPoints[4] =
	{
		{ -0.8f, 0.58f, 1.0f },
		{ 1.76f, 1.0f, -0.3f },
		{ 0.94f, -0.7f, 2.3f },
		{ -0.53f, -0.26f, -0.15f }
	};

	// 透視投影行列を作成
	Matrix4x4 projectionMatrix = mathFunc.MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
	// ビューポート変換行列を作成
	Matrix4x4 viewportMatrix = mathFunc.MakeViewportMatrix(0.0f, 0.0f, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0)
	{
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		// マウス入力を取得
		POINT mousePosition;
		GetCursorPos(&mousePosition);

		///
		/// ↓更新処理ここから
		///

		// マウスドラッグによる回転制御
		if (Novice::IsPressMouse(1))
		{
			if (!isDragging)
			{
				isDragging = true;
				prevMouseX = mousePosition.x;
				prevMouseY = mousePosition.y;
			}
			else
			{
				int deltaX = mousePosition.x - prevMouseX;
				int deltaY = mousePosition.y - prevMouseY;
				rotate.y += deltaX * 0.01f; // 水平方向の回転
				rotate.x += deltaY * 0.01f; // 垂直方向の回転
				prevMouseX = mousePosition.x;
				prevMouseY = mousePosition.y;
			}
		}
		else
		{
			isDragging = false;
		}

		// マウスホイールで前後移動
		int wheel = Novice::GetWheel();
		if (wheel != 0)
		{
			cameraTranslate.z += wheel * 0.01f; // ホイールの回転方向に応じて前後移動
		}

		//各種行列の計算
		Matrix4x4 worldMatrix = mathFunc.MakeAffineMatrix({ 1.0f,1.0f,1.0f }, rotate, translate);
		Matrix4x4 cameraMatrix = mathFunc.MakeAffineMatrix({ 1.0f,1.0f,1.0f }, cameraRotate, cameraTranslate);
		Matrix4x4 viewWorldMatrix = mathFunc.Inverse(worldMatrix);
		Matrix4x4 viewCameraMatrix = mathFunc.Inverse(cameraMatrix);
		//ビュー座標変換行列を作成
		Matrix4x4 viewProjectionMatrix = mathFunc.Multiply(viewWorldMatrix, mathFunc.Multiply(viewCameraMatrix, projectionMatrix));

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		// Gridを描画
		mathFunc.DrawGrid(viewProjectionMatrix, viewportMatrix);

		// コントロールポイントのImGui調整
		ImGui::Begin("Control Points");
		for (int i = 0; i < 4; ++i)
		{
			ImGui::DragFloat3(("Control Point " + std::to_string(i)).c_str(), &controllPoints[i].x, 0.01f);
		}
		ImGui::End();

		// コントロールポイントを球で描画
		for (int i = 0; i < 4; ++i)
		{
			Sphere controlSphere = { controllPoints[i], 0.01f };
			mathFunc.DrawSphere(controlSphere, viewProjectionMatrix, viewportMatrix, 0x000000FF);
		}

		// Catmull-Rom曲線を描画
		DrawCatmullRom(controllPoints[0], controllPoints[1], controllPoints[2], controllPoints[3], viewProjectionMatrix, viewportMatrix, 0xFFFFFFFF);
		DrawCatmullRom(controllPoints[3], controllPoints[1], controllPoints[0], controllPoints[2], viewProjectionMatrix, viewportMatrix, 0xFFFFFFFF);
		DrawCatmullRom(controllPoints[1], controllPoints[2], controllPoints[3], controllPoints[2], viewProjectionMatrix, viewportMatrix, 0xFFFFFFFF);

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0)
		{
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}
