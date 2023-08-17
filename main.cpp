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

struct Triangle {
	Vector3 vertices[3]; // !< 頂点
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

void DrawTriangle(
	const Triangle& triangle, const Matrix4x4& viewProjectionMatrix,
	const Matrix4x4& viewportMatrix, uint32_t color) 
{

	// Screen空間へと頂点を変換する
	Vector3 screenVertices[3];

	for (uint32_t i = 0; i < 3; ++i) {
		Vector3 ndcVertex = Transform(triangle.vertices[i], viewProjectionMatrix);
		screenVertices[i] = Transform(ndcVertex, viewportMatrix);
	}

	// 描画
	Novice::DrawTriangle(
	    int(screenVertices[0].x), int(screenVertices[0].y), int(screenVertices[1].x),
	    int(screenVertices[1].y), int(screenVertices[2].x), int(screenVertices[2].y), color,
	    kFillModeWireFrame);

}

bool IsCollision(const Triangle& triangle, const Segment& segment) {

	// 法線
	Vector3 v1 = Vec3Subtract(triangle.vertices[0], triangle.vertices[1]);
	Vector3 v2 = Vec3Subtract(triangle.vertices[1], triangle.vertices[2]);
	Vector3 v3 = Vec3Subtract(triangle.vertices[2], triangle.vertices[0]);
	Vector3 normal = Normalize(Cross(v1, v2));
	// 距離
	float dist = Dot(triangle.vertices[1], normal);
	

	// 垂直の時
	float bn = Dot(normal, segment.diff);
	if (bn == 0.0f) {
		return false;
	}

	float t = (dist - Dot(segment.origin, normal)) / bn; // 媒介変数tを求める
	Vector3 p = Vec3Add(segment.origin, Vec3Multiply(t, segment.diff));
	Vector3 v1p = Vec3Subtract(p, triangle.vertices[1]);
	Vector3 v2p = Vec3Subtract(p, triangle.vertices[2]);
	Vector3 v0p = Vec3Subtract(p, triangle.vertices[0]);
	Vector3 cross01 = Cross(v1, v1p);
	Vector3 cross12 = Cross(v2, v2p);
	Vector3 cross20 = Cross(v3, v0p);

	// 線分の長さ(範囲)
	float norm = Length(Normalize(segment.diff));

	if (t > 0.0f && t < norm) {
		if (Dot(cross01, normal) >= 0.0f &&
			Dot(cross12, normal) >= 0.0f &&
		    Dot(cross20, normal) >= 0.0f) 
		{
			return true;
		
		}
	}
	return false;

}



const char kWindowTitle[] = "LE2D_18_ニヘイリュウダイ_MT3_02_04";

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

	// 三角形の頂点
	Triangle triangle;
	triangle.vertices[0] = {-0.5f, -0.5f, 2.0f};
	triangle.vertices[1] = {0.0f, 0.5f, 2.0f};
	triangle.vertices[2] = {0.5f, -0.5f, 2.0f};
		
	
	// 線分
	Segment segment{
	    {-2.0f, -1.0f, 0.0f},
        {3.0f,  2.0f,  2.0f}
    };
	Vector3 point{-1.5f, 0.6f, 0.6f};

	Vector3 project = Project(Vec3Subtract(point, segment.origin), segment.diff);
	Vector3 closestPoint = ClosestPoint(point, segment);

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
		ImGui::DragFloat3("Vertices[0]", &triangle.vertices[0].x, 0.01f);
		ImGui::DragFloat3("Vertices[1]", &triangle.vertices[1].x, 0.01f);
		ImGui::DragFloat3("Vertices[2]", &triangle.vertices[2].x, 0.01f);
		ImGui::DragFloat3("Segment.orgin", &segment.origin.x, 0.01f);
		ImGui::DragFloat3("Segment.diff", &segment.diff.x, 0.01f);
		ImGui::End();

		// 変換
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

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		DrawGrid(worldViewProjectionMatrix, viewportMatrix);

		DrawTriangle(triangle, worldViewProjectionMatrix, viewportMatrix, WHITE);


		if (IsCollision(triangle, segment)) {
			color = RED;
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
