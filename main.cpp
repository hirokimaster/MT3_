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

// 直線
struct Line {
	Vector3 origin; // !< 始点
	Vector3 diff;   // !< 終点への差分ベクトル
};

// 半直線
struct Ray {
	Vector3 origin; // !< 始点
	Vector3 diff;   // !< 終点への差分ベクトル
};

// 線分
struct Segment {
	Vector3 origin; // !< 始点
	Vector3 diff;   // !< 終点への差分ベクトル
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

		if (xIndex == 5) {
			// 線を引く
			Novice::DrawLine(
			    (int)screenXsp.x, (int)screenXsp.y, (int)screenXep.x, (int)screenXep.y, BLACK);
		}
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
		if (zIndex == 5) {
			// 線を引く
			Novice::DrawLine(
			    (int)screenZsp.x, (int)screenZsp.y, (int)screenZep.x, (int)screenZep.y, BLACK);
		}
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

// 減算
Vector3 Subtract(const Vector3& v1, const Vector3& v2) {
	Vector3 result;
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	result.z = v1.z - v2.z;
	return result;
}

// 加算
Vector3 Add(const Vector3& v1, const Vector3& v2) {
	Vector3 result;
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	result.z = v1.z + v2.z;
	return result;
}

// 内積
float Dot(const Vector3& v1, const Vector3& v2) {
	float result;
	result = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	return result;
}

// 長さ（ノルム）
float Length(const Vector3& v) {
	float result;
	result = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	return result;
}

// 正規化
Vector3 Normalize(const Vector3& v) {
	Vector3 result;
	float norm = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	result.x = v.x / norm;
	result.y = v.y / norm;
	result.z = v.z / norm;
	return result;
}

// スカラー倍
Vector3 Multiply(float scalar, const Vector3& v) {
	Vector3 result;
	result.x = scalar * v.x;
	result.y = scalar * v.y;
	result.z = scalar * v.z;
	return result;
}

// Project
Vector3 Project(const Vector3& v1, const Vector3& v2) { 
	Vector3 project;
	// 単位ベクトルを求める
	Vector3 unit;
	unit = Normalize(v2);

	project.x = Dot(v1, Multiply(unit.x, unit));
	project.y = Dot(v1, Multiply(unit.y, unit));
	project.z = Dot(v1, Multiply(unit.z, unit));

	return project;

}

// ClosestPoint
Vector3 ClosestPoint(const Vector3& point, const Segment& segment) { 
	Vector3 closestPoint;
	Vector3 project;
	project = Project(Subtract(point, segment.origin), segment.diff);
	closestPoint = Add(segment.origin, project);

	return closestPoint;
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
		
	    Vector3 scale{1.0f, 1.0f, 1.0f};
		Vector3 rotate{0.0f, 0.0f, 0.0f};
		Vector3 translate{0.0f, 0.0f, 0.0f};
		Vector3 cameraScale{1.0f, 1.0f, 1.0f};
		Vector3 cameraRotate{0.3f, 0.0f, 0.0f};
		Vector3 cameraTranslate{0.0f, 2.0f, -6.0f};

		Matrix4x4 worldMatrix = MakeAffineMatrix(scale, rotate, translate);
		Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraScale, cameraRotate, cameraTranslate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix =
		    MakePerspectiveFovMatrix(0.45f, float(1280) / float(720), 0.1f, 100.0f);
		// WVPMatrix
		Matrix4x4 worldViewProjectionMatrix =
		    Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
		// ViewPortMatrix
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, 1200.0f, 720.0f, 0.0f, 1.0f);

		Segment segment{
		    {-2.0f, -1.0f, 0.0f},
            {3.0f,  2.0f,  2.0f}
        };
		Vector3 point{-1.5f, 0.6f, 0.6f};

		Vector3 project = Project(Subtract(point, segment.origin), segment.diff);
		Vector3 closestPoint = ClosestPoint(point, segment);

		Sphere pointSphere{point, 0.01f};
		Sphere closestPointSphere{closestPoint, 0.01f};

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///
		
		ImGui::Begin("Window");
		ImGui::DragFloat3("point", &point.x, 0.01f);
		ImGui::DragFloat3("segment origin", &segment.origin.x, 0.01f);
		ImGui::DragFloat3("segment diff", &segment.diff.x, 0.01f);
		ImGui::InputFloat3("Project", &project.x, "%.3f", ImGuiInputTextFlags_ReadOnly);
		ImGui::End();

		// グリッド
		DrawGrid(worldViewProjectionMatrix, viewportMatrix);

		// 点の描画
		DrawSphere(pointSphere, worldViewProjectionMatrix, viewportMatrix, RED);
		DrawSphere(closestPointSphere, worldViewProjectionMatrix, viewportMatrix, BLACK);

		// 線分
		Vector3 start = Transform(Transform(segment.origin, worldViewProjectionMatrix), viewportMatrix);
		Vector3 end = Transform(Transform(Add(segment.origin, segment.diff), worldViewProjectionMatrix), viewportMatrix);
		Novice::DrawLine(int(start.x), int(start.y), int(end.x), int(end.y), WHITE);


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
