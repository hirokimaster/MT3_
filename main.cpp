#include <Novice.h>
#include <stdint.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#include <cassert>
#include <imgui.h>
#include "Mathfunction.h"

struct Sphere {
	Vector3 center;
	float radius;
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



const char kWindowTitle[] = "LE2D_18_ニヘイリュウダイ_MT3";

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

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

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

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
