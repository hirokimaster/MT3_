#include <Novice.h>
#include <stdint.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#include <cassert>
#include <imgui.h>
#include "Mathfunction.h"
#include <algorithm>	

struct Sphere {
	Vector3 center;
	float radius;
};

struct AABB {
	Vector3 min; // 最小点
	Vector3 max; // 最大点
};

// 4x4行列表示
static const int kRowHeight = 20;
static const int kColumnWidth = 60;
void MatrixScreenPrintf(int x, int y, const Matrix4x4& matrix) {
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			Novice::ScreenPrintf(
			    x + column * kColumnWidth, y + row * kRowHeight, "%6.02f", matrix.m[row][column]);
		}
	}
}

// 三次元ベクトル表示
void VectorScreenPrintf(int x, int y, const Vector3& vector, const char* label) {
	Novice::ScreenPrintf(x, y, "%.02f", vector.x);
	Novice::ScreenPrintf(x + kColumnWidth, y, "%.02f", vector.y);
	Novice::ScreenPrintf(x + kColumnWidth * 2, y, "%.02f", vector.z);
	Novice::ScreenPrintf(x + kColumnWidth * 3, y, "%s", label);
}

// Grid
void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {

	const float kGridHalfWidth = 2.0f;                                      // Grid半分
	const uint32_t kSubdivision = 10;                                       // 分割数
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision); // 1つ分の長さ

	// 奥から手前への線を順番に引いていく
	for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex) {
		// 始点と終点
		float st = -kGridHalfWidth + (kGridEvery * xIndex);
		Vector3 xSp, xEp;
		xSp = {st, 0.0f, -kGridHalfWidth}; // 始点
		xEp = {st, 0.0f, kGridHalfWidth};  // 終点
		// ndcまで変換
		Vector3 ndcXsp = Transform(xSp, viewProjectionMatrix);
		Vector3 ndcXep = Transform(xEp, viewProjectionMatrix);
		// screenまで変換
		Vector3 screenXsp = Transform(ndcXsp, viewportMatrix);
		Vector3 screenXep = Transform(ndcXep, viewportMatrix);
		// 線を引く
		Novice::DrawLine(
		    (int)screenXsp.x, (int)screenXsp.y, (int)screenXep.x, (int)screenXep.y, WHITE);
	}
	// 左から右に順番に引く
	for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex) {
		// 始点と終点
		float st2 = -kGridHalfWidth + (kGridEvery * zIndex);
		Vector3 zSp, zEp;
		zSp = {-kGridHalfWidth, 0.0f, st2}; // 始点
		zEp = {kGridHalfWidth, 0.0f, st2};  // 終点
		// ndcまで変換
		Vector3 ndcZsp = Transform(zSp, viewProjectionMatrix);
		Vector3 ndcZep = Transform(zEp, viewProjectionMatrix);
		// screenまで変換
		Vector3 screenZsp = Transform(ndcZsp, viewportMatrix);
		Vector3 screenZep = Transform(ndcZep, viewportMatrix);
		// 線を引く
		Novice::DrawLine(
		    (int)screenZsp.x, (int)screenZsp.y, (int)screenZep.x, (int)screenZep.y, WHITE);
	}
}

// Sphere
void DrawSphere(
    const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix,
    uint32_t color) {
	const uint32_t kSubdivision = 16;
	const float pi = (float)M_PI;
	const float kLonEvery = pi * 2.0f / float(kSubdivision);
	const float kLatEvery = pi / float(kSubdivision);

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		// 緯度の方向に分割
		float lat = -pi / 2.0f + kLatEvery * latIndex;

		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			// 経度の方向に分割
			float lon = lonIndex * kLonEvery;
			// world座標
			Vector3 a, b, c;
			// 緯度、経度
			float latD = pi / kSubdivision;
			float lonD = (2.0f * pi) / kSubdivision;

			a = {
			    sphere.center.x + sphere.radius * (std::cos(lat) * std::cos(lon)),
			    sphere.center.y + sphere.radius * std::sin(lat),
			    sphere.center.z + sphere.radius * (std::cos(lat) * std::sin(lon))};

			b = {
			    sphere.center.x + sphere.radius * (std::cos(lat + latD) * std::cos(lon)),
			    sphere.center.y + sphere.radius * std::sin(lat + latD),	
			    sphere.center.z + sphere.radius * (std::cos(lat + latD) * std::sin(lon))};

			c = {
			    sphere.center.x + sphere.radius * (std::cos(lat) * std::cos(lon + lonD)),
			    sphere.center.y + sphere.radius * std::sin(lat),
			    sphere.center.z + sphere.radius * (std::cos(lat) * std::sin(lon + lonD))};

			// a,b,cをscreen座標に変換
			Vector3 ndcVertexA = Transform(a, viewProjectionMatrix);
			Vector3 ndcVertexB = Transform(b, viewProjectionMatrix);
			Vector3 ndcVertexC = Transform(c, viewProjectionMatrix);
			Vector3 screenA = Transform(ndcVertexA, viewportMatrix);
			Vector3 screenB = Transform(ndcVertexB, viewportMatrix);
			Vector3 screenC = Transform(ndcVertexC, viewportMatrix);
			// ab acで線を引く
			Novice::DrawLine((int)screenA.x, (int)screenA.y, (int)screenB.x, (int)screenB.y, color);
			Novice::DrawLine((int)screenA.x, (int)screenA.y, (int)screenC.x, (int)screenC.y, color);
		}
	}
}

// AABBの描画
void DrawAABB(
    const AABB& aabb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix,
    uint32_t color) {

	Vector3 vertex[8];

	Vector3 ndcVertex[8];

	Vector3 screenVertices[8];

	vertex[0] = {aabb.min.x, aabb.min.y, aabb.min.z};
	vertex[1] = {aabb.min.x, aabb.min.y, aabb.max.z};

	vertex[2] = {aabb.max.x, aabb.min.y, aabb.min.z};
	vertex[3] = {aabb.max.x, aabb.min.y, aabb.max.z};

	vertex[4] = {aabb.min.x, aabb.max.y, aabb.min.z};
	vertex[5] = {aabb.min.x, aabb.max.y, aabb.max.z};

	vertex[6] = {aabb.max.x, aabb.max.y, aabb.min.z};
	vertex[7] = {aabb.max.x, aabb.max.y, aabb.max.z};

	for (uint32_t Index = 0; Index < 8; ++Index) {
		// 正規化デバイス座標系
		ndcVertex[Index] = Transform(vertex[Index], viewProjectionMatrix);
		// スクリーン座標系
		screenVertices[Index] = Transform(ndcVertex[Index], viewportMatrix);
	}

	// 底辺
	Novice::DrawLine(
	    int(screenVertices[0].x), int(screenVertices[0].y), int(screenVertices[1].x),
	    int(screenVertices[1].y), color);
	Novice::DrawLine(
	    int(screenVertices[0].x), int(screenVertices[0].y), int(screenVertices[2].x),
	    int(screenVertices[2].y), color);
	Novice::DrawLine(
	    int(screenVertices[1].x), int(screenVertices[1].y), int(screenVertices[3].x),
	    int(screenVertices[3].y), color);
	Novice::DrawLine(
	    int(screenVertices[2].x), int(screenVertices[2].y), int(screenVertices[3].x),
	    int(screenVertices[3].y), color);

	// 上辺
	Novice::DrawLine(
	    int(screenVertices[4].x), int(screenVertices[4].y), int(screenVertices[5].x),
	    int(screenVertices[5].y), color);
	Novice::DrawLine(
	    int(screenVertices[4].x), int(screenVertices[4].y), int(screenVertices[6].x),
	    int(screenVertices[6].y), color);
	Novice::DrawLine(
	    int(screenVertices[5].x), int(screenVertices[5].y), int(screenVertices[7].x),
	    int(screenVertices[7].y), color);
	Novice::DrawLine(
	    int(screenVertices[6].x), int(screenVertices[6].y), int(screenVertices[7].x),
	    int(screenVertices[7].y), color);

	// 面
	Novice::DrawLine(
	    int(screenVertices[0].x), int(screenVertices[0].y), int(screenVertices[4].x),
	    int(screenVertices[4].y), color);
	Novice::DrawLine(
	    int(screenVertices[1].x), int(screenVertices[1].y), int(screenVertices[5].x),
	    int(screenVertices[5].y), color);
	Novice::DrawLine(
	    int(screenVertices[2].x), int(screenVertices[2].y), int(screenVertices[6].x),
	    int(screenVertices[6].y), color);
	Novice::DrawLine(
	    int(screenVertices[3].x), int(screenVertices[3].y), int(screenVertices[7].x),
	    int(screenVertices[7].y), color);
}


bool IsCollision(const AABB& a, const Sphere& s1) {
	Vector3 closesetPoint{
	    std::clamp(s1.center.x, a.min.x, a.max.x), std::clamp(s1.center.y, a.min.y, a.max.y),
	    std::clamp(s1.center.z, a.min.z, a.max.z)};
	float distance = Length(Vec3Subtract(closesetPoint, s1.center));

	if (distance <= s1.radius) {

		return true;
	}

	return false;
}


const char kWindowTitle[] = "LE2D_18_ニヘイリュウダイ_MT3_02_06";

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

	AABB aabb1{
	    .min{-0.5f, -0.5f, -0.5f},
	    .max{0.0f,  0.0f,  0.0f },
	};

	Sphere sphere = {
	    {0.0f, 0.0f, 0.0f},
        0.5f
    };

	uint32_t color = WHITE;

	Vector3 rotate{0, 0, 0};
	Vector3 translate{0, 0, 0};
	Vector3 cameraTranslate{0.0f, 1.9f, -5.49f};
	Vector3 cameraRotate{0.26f, 0.0f, 0.0f};

	int mouseMovePosX = 0;
	int mouseMovePosY = 0;

	const float move = 0.01f;

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		// カメラ
		if (keys[DIK_SPACE]) {
			Novice::GetMousePosition(&mouseMovePosX, &mouseMovePosY);

			if (mouseMovePosX > 1280 / 2) {
				cameraRotate.y += move;
			} else if (mouseMovePosX < 1280 / 2) {
				cameraRotate.y -= move;
			}
			
		}

		if (keys[DIK_W]) {
			cameraTranslate.y += move;
		} else if (keys[DIK_S]) {
			cameraTranslate.y -= move;
		} else if (keys[DIK_D]) {
			cameraTranslate.x += move;
		} else if (keys[DIK_A]) {
			cameraTranslate.x -= move;
		}

		uint32_t value = Novice::GetWheel();

		if (value > 0) {
			cameraTranslate.z += move * 10;
		} else if (value < 0) {
			cameraTranslate.z -= move * 10;
		}

		// 各行列の計算
		Matrix4x4 worldMatrix = MakeAffineMatrix({1.0f, 1.0f, 1.0f}, rotate, translate);

		Matrix4x4 cameraMatrix =
		    MakeAffineMatrix({1.0f, 1.0f, 1.0f}, cameraRotate, cameraTranslate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);

		Matrix4x4 projecttionMatrix = MakePerspectiveFovMatrix(
		    0.45f, float(1280) / float(720), 0.1f, 100.0f);
		// VPMatrixを作る。同次クリップ空間
		Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projecttionMatrix);
		// ViewportMatrixを作る
		Matrix4x4 viewportMatrix =
		    MakeViewportMatrix(0, 0, float(1280), float(720), 0.0f, 1.0f);

		aabb1.min.x = (std::min)(aabb1.min.x, aabb1.max.x);
		aabb1.max.x = (std::max)(aabb1.min.x, aabb1.max.x);
		aabb1.min.y = (std::min)(aabb1.min.y, aabb1.max.y);
		aabb1.max.y = (std::max)(aabb1.min.y, aabb1.max.y);

		ImGui::Begin("Window");
		ImGui::DragFloat3("CamerRotate", &cameraRotate.x, 0.01f);

		ImGui::DragFloat3("aabb1.min", &aabb1.min.x, 0.01f);
		ImGui::DragFloat3("aabb1.max", &aabb1.max.x, 0.01f);
	
		ImGui::End();

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		if (IsCollision(aabb1, sphere)) {
			color = RED;
		} else {
			color = WHITE;
		}

		DrawGrid(viewProjectionMatrix, viewportMatrix);

		DrawAABB(aabb1, viewProjectionMatrix, viewportMatrix, color);
		DrawSphere(sphere, viewProjectionMatrix, viewportMatrix, WHITE);


		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}
