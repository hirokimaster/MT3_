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

struct Plane {
	Vector3 normal; // 法線
	float distance; // 距離
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

// 垂直なベクトルを求める
Vector3 Perpendicular(const Vector3& vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return {-vector.y, vector.x, 0.0f};
	}
	return {0.0f, -vector.z, vector.y};
}

// 平面
void DrawPlane(
    const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix,
    uint32_t color) {
	Vector3 center = Vec3Multiply(plane.distance, plane.normal); // 中心点
	Vector3 perpendiculars[4];
	perpendiculars[0] = Normalize(Perpendicular(plane.normal)); // 法線と垂直なベクトル
	perpendiculars[1] = {
	    -perpendiculars[0].x, -perpendiculars[0].y, -perpendiculars[0].z}; // 上の逆ベクトル
	perpendiculars[2] =
	    Cross(plane.normal, perpendiculars[0]); // 法線と垂直なベクトルと法線とのクロス積
	perpendiculars[3] = {
	    -perpendiculars[2].x, -perpendiculars[2].y, -perpendiculars[2].z}; // 上の逆ベクトル
	// 頂点を求める
	Vector3 points[4];
	for (int32_t index = 0; index < 4; ++index) {
		Vector3 extend = Vec3Multiply(2.0f, perpendiculars[index]);
		Vector3 point = Vec3Add(center, extend);
		points[index] = Transform(Transform(point, viewProjectionMatrix), viewportMatrix);
	}
	// pointを結んでDrawLineで描画
	Novice::DrawLine((int)points[0].x, (int)points[0].y, (int)points[2].x, (int)points[2].y, color);
	Novice::DrawLine((int)points[1].x, (int)points[1].y, (int)points[3].x, (int)points[3].y, color);
	Novice::DrawLine((int)points[2].x, (int)points[2].y, (int)points[1].x, (int)points[1].y, color);
	Novice::DrawLine((int)points[3].x, (int)points[3].y, (int)points[0].x, (int)points[0].y, color);
}

// Project
Vector3 Project(const Vector3& v1, const Vector3& v2) {
	Vector3 project;
	// 単位ベクトルを求める
	Vector3 unit;
	unit = Normalize(v2);

	project.x = Dot(v1, Vec3Multiply(unit.x, unit));
	project.y = Dot(v1, Vec3Multiply(unit.y, unit));
	project.z = Dot(v1, Vec3Multiply(unit.z, unit));

	return project;
}

// ClosestPoint
Vector3 ClosestPoint(const Vector3& point, const Segment& segment) {
	Vector3 closestPoint;
	Vector3 project;
	project = Project(Vec3Subtract(point, segment.origin), segment.diff);
	closestPoint = Vec3Add(segment.origin, project);

	return closestPoint;
}

bool isCollision(const Segment& segment, const Plane& plane) {
	// 単位ベクトルを求める
	Vector3 unit;
	unit = Normalize(plane.normal);

	// 垂直の時
	float bn = Dot(unit, segment.diff);
	if (bn == 0.0f) {
		return false;
	}

	Vector3 center = Vec3Multiply(plane.distance, plane.normal); // 任意の点
	float d = Dot(unit, center); 

	float t = (d - Dot(segment.origin, unit)) / bn; // 媒介変数tを求める

	if (t == 1.0f) {
		return true;
	} 
	
}

const char kWindowTitle[] = "LE2D_18_ニヘイリュウダイ_MT3_02_03";

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};


	Vector3 scale{1.0f, 1.0f, 1.0f};
	Vector3 rotate{0.0f, 0.0f, 0.0f};
	Vector3 translate{0.0f, 0.0f, 0.0f};
	Vector3 cameraScale{1.0f, 1.0f, 1.0f};
	Vector3 cameraRotate{0.3f, 0.0f, 0.0f};
	Vector3 cameraTranslate{0.0f, 2.0f, -10.0f};


	// 平面上の点
	Vector3 planePointA = {0.1f, 0.2f, 0.1f};
	Vector3 planePointB = {0.0f, 0.2f, 0.1f};
	Vector3 planePointC = {0.1f, 0.2f, 0.0f};

	Vector3 v1 = Vec3Subtract(planePointB, planePointA); // b-a
	Vector3 v2 = Vec3Subtract(planePointC, planePointB); // c-b
	// 法線
	Vector3 normal = Normalize(Cross(v1, v2));

	Plane plane;
	plane.normal = normal;
	plane.distance = Dot(planePointA, plane.normal);

	uint32_t color = WHITE;

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

		// imgui
		ImGui::Begin("setting");
		ImGui::DragFloat3("CameraTranslate", &cameraTranslate.x, 0.01f);
		ImGui::DragFloat3("CameraRotate", &cameraRotate.x, 0.01f);
		ImGui::DragFloat3("Plane.normal", &plane.normal.x, 0.01f);
		ImGui::DragFloat("Plane.distance", &plane.distance, 0.01f);
		ImGui::End();

		// 変換
		Matrix4x4 worldMatrix = MakeAffineMatrix(scale, rotate, translate);
		Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraScale, cameraRotate, cameraTranslate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(1280) / float(720), 0.1f, 100.0f);
		// WVPMatrix
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
		// ViewPortMatrix
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, 1200.0f, 720.0f, 0.0f, 1.0f);

		Segment segment{
		    {-2.0f, -1.0f, 0.0f},
            {3.0f,  2.0f,  2.0f}
        };
		Vector3 point{-1.5f, 0.6f, 0.6f};

		Vector3 project = Project(Vec3Subtract(point, segment.origin), segment.diff);
		Vector3 closestPoint = ClosestPoint(point, segment);


		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		// 平面
		DrawPlane(plane, worldViewProjectionMatrix, viewportMatrix, WHITE);

		if (isCollision(segment, plane)) {
			color = RED;
		
		} else {
			color = WHITE;
		}

		// 線分
		Vector3 start =
		    Transform(Transform(segment.origin, worldViewProjectionMatrix), viewportMatrix);
		Vector3 end = Transform(
		    Transform(Vec3Add(segment.origin, segment.diff), worldViewProjectionMatrix),
		    viewportMatrix);
		Novice::DrawLine(int(start.x), int(start.y), int(end.x), int(end.y), color);

		

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
