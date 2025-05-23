#include <Novice.h>

#include <Matrix4x4.h>
#include <Vector3.h>
#include <cmath>
#include <assert.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdint.h>

//ImGUI
#include <ImGui.h>
#include <ImGuiManager.h>
#include "DirectXCommon.h"
#include "WinApp.h"

const char kWindowTitle[] = "LC1C_14_タカムラシュン_タイトル";

const float kWindowWidth = 1080.0f;
const float kWindowHeight = 720.0f;

typedef struct Segment {
	Vector3 origin;// 始点
	Vector3 diff;// 終点
}Segment;

typedef struct Sphere {
	Vector3 center;// 中心点
	float radius;// 半径
}Sphere;

typedef struct Plane {
	Vector3 normal;// 法線
	float distance;// 距離
}Plane;

// 行列をベクトルに変換する関数
Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);

/// <summary>
/// cotangent(余接)を求める関数
/// </summary>
/// <param name="theta">θ(シータ)</param>
/// <returns>cotangent</returns>
float Cotangent(float theta);

/// <summary>
/// アフィン行列作成関数
/// </summary>
/// <param name="scale">縮尺</param>
/// <param name="rotate">thetaを求めるための数値</param>
/// <param name="translate">三次元座標でのx,y,zの移動量</param>
/// <returns>アフィン行列</returns>
Matrix4x4 MakeAffineMatrix(Vector3 scale, Vector3 rotate, Vector3 translate);

/// <summary>
/// 4x4逆行列を求める関数
/// </summary>
/// <param name="matrix4x4">逆行列を求めたい行列</param>
/// <returns>4x4逆行列</returns>
Matrix4x4 Inverse(Matrix4x4 matrix4x4);

/// <summary>
/// 4x4行列の積を求める関数
/// </summary>
/// <param name="matrix1">1つ目の行列</param>
/// <param name="matrix2">1つ目の行列</param>
/// <returns>4x4行列の積</returns>
Matrix4x4 Multiply(Matrix4x4 matrix1, Matrix4x4 matrix2);

/// <summary>
/// 投視投影行列作成関数
/// </summary>
/// <param name="fovY">縦の画角</param>
/// <param name="aspectRatio">アスペクト比</param>
/// <param name="nearClip">近平面への距離</param>
/// <param name="farClip">遠平面への距離</param>
/// <returns>投視投影行列</returns>
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

Vector3 TransformWithoutW(const Vector3& vector, const Matrix4x4& matrix);

/// <summary>
/// ビューポート行列作成関数
/// </summary>
/// <param name="left">左側の座標</param>
/// <param name="top">上側の座標</param>
/// <param name="width">切り取るスクリーンの幅</param>
/// <param name="height">切り取るスクリーンの高さ</param>
/// <param name="minDepth">最小深度値</param>
/// <param name="maxDepth">最大深度値</param>
/// <returns></returns>
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix);

void DrawSphere(const Vector3& center, float radius, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color);

Vector3 Project(const Vector3& v1, const Vector3& v2);

Vector3 ClosestPoint(const Vector3& point, const Segment& segment);

Vector3 Subtract(const Vector3& v1, const Vector3& v2);

Vector3 Add(const Vector3& v1, const Vector3& v2);

float Dot(const Vector3& v1, const Vector3& v2);

bool IsCollision(const Segment& segment, const Plane& plane);

float GetLength(const Vector3& v1);

Vector3 Perpendicular(const Vector3& vector);

void DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color);

Vector3 Normalize(const Vector3& v);

Vector3 Cross(const Vector3& v1, const Vector3& v2);

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	Vector3 rotate{ 0.0f,0.0f,0.0f };
	Vector3 translate{ 0.0f,0.0f,0.0f };

	Matrix4x4 cameraMatrix = MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, { 0.26f, 0.0f, 0.0f }, { 0, 1.9f, -6.49f });
	Matrix4x4 viewMatrix = Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
	Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
	Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, 1280, 720, 0.0f, 1.0f);

	Segment segment = { {0.0f,1.0f,0.0f},{0.0f,0.0f,0.0f} };
	Plane plane = { {0.0f,1.0f,0.0f}, 1.0f };

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

		segment.diff = { segment.origin.x, segment.origin.y-1.0f, segment.origin.z };

		Segment transformSegment = {
			Transform(Transform(segment.origin,viewProjectionMatrix),viewportMatrix),
			Transform(Transform(segment.diff,viewProjectionMatrix),viewportMatrix)
		};

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		// グリッドの描画
		DrawGrid(viewProjectionMatrix, viewportMatrix);

		// 描画
		if (IsCollision(segment, plane)) {
			Novice::DrawLine(
				static_cast<int>(transformSegment.origin.x),
				static_cast<int>(transformSegment.origin.y),
				static_cast<int>(transformSegment.diff.x),
				static_cast<int>(transformSegment.diff.y),
				0xFF0000FF
			);
		} else {
			Novice::DrawLine(
				static_cast<int>(transformSegment.origin.x),
				static_cast<int>(transformSegment.origin.y),
				static_cast<int>(transformSegment.diff.x),
				static_cast<int>(transformSegment.diff.y),
				0xFFFFFFFF
			);
		}

		DrawPlane(plane, viewProjectionMatrix, viewportMatrix, 0x000000FF);

		ImGui::Begin("Segment Controller");
		ImGui::SetWindowSize(ImVec2(400, 300)); // 幅400, 高さ300
		ImGui::SliderFloat3("segment.origine", &segment.origin.x, -5.0f, 5.0f);
		ImGui::End();

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
void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	const float kGridHalfWidth = 2.0f;
	const uint32_t kSubdivision = 10;
	const float kGridEvery = (kGridHalfWidth * 2.0f) / static_cast<float>(kSubdivision);

	for (uint32_t i = 0; i <= kSubdivision; ++i) {
		float offset = -kGridHalfWidth + i * kGridEvery;

		// 色を決定（中央線だけ黒、それ以外は灰色）
		uint32_t color = (offset == 0.0f) ? 0x000000FF : 0xAAAAAAFF;

		// Z方向（X軸に平行）
		Vector3 start = { -kGridHalfWidth, 0.0f, offset };
		Vector3 end = { kGridHalfWidth, 0.0f, offset };
		start = Transform(Transform(start, viewProjectionMatrix), viewportMatrix);
		end = Transform(Transform(end, viewProjectionMatrix), viewportMatrix);
		Novice::DrawLine(static_cast<int>(start.x), static_cast<int>(start.y), static_cast<int>(end.x), static_cast<int>(end.y), color);

		// X方向（Z軸に平行）
		start = { offset, 0.0f, -kGridHalfWidth };
		end = { offset, 0.0f, kGridHalfWidth };
		start = Transform(Transform(start, viewProjectionMatrix), viewportMatrix);
		end = Transform(Transform(end, viewProjectionMatrix), viewportMatrix);
		Novice::DrawLine(static_cast<int>(start.x), static_cast<int>(start.y), static_cast<int>(end.x), static_cast<int>(end.y), color);
	}
}

void DrawSphere(const Vector3& center, float radius, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	const uint32_t kSubdivision = 20; //分割数
	const float kLatEvery = static_cast<float>(M_PI) / static_cast<float>(kSubdivision); // 緯度分割1つ分の角度 θd
	const float kLonEvery = static_cast<float>(2.0f * M_PI) / static_cast<float>(kSubdivision); // 経度分割1つ分の角度 φd

	// 緯度の方向に分割 -π/2~π/2
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -static_cast<float>(M_PI) / 2.0f + kLatEvery * latIndex; // θ

		// 経度の方向に分割 θ~2π
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = kLonEvery * lonIndex; // φ

			// 緯線
			Vector3 a = {
				center.x + radius * cosf(lat) * cosf(lon),
				center.y + radius * sinf(lat),
				center.z + radius * cosf(lat) * sinf(lon)
			};

			Vector3 b = {
				center.x + radius * cosf(lat + kLatEvery) * cosf(lon),
				center.y + radius * sinf(lat + kLatEvery),
				center.z + radius * cosf(lat + kLatEvery) * sinf(lon)
			};

			// 経線
			Vector3 c = {
				center.x + radius * cosf(lat) * cosf(lon + kLonEvery),
				center.y + radius * sinf(lat),
				center.z + radius * cosf(lat) * sinf(lon + kLonEvery)
			};

			a = Transform(Transform(a, viewProjectionMatrix), viewportMatrix);
			b = Transform(Transform(b, viewProjectionMatrix), viewportMatrix);
			c = Transform(Transform(c, viewProjectionMatrix), viewportMatrix);

			Novice::DrawLine((int)a.x, (int)a.y, (int)b.x, (int)b.y, color);
			Novice::DrawLine((int)a.x, (int)a.y, (int)c.x, (int)c.y, color);
		}
	}
}

Vector3 Project(const Vector3& v1, const Vector3& v2) {
	float dot = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; // v1・v2
	float normSq = v2.x * v2.x + v2.y * v2.y + v2.z * v2.z; // ||v2||^2

	if (normSq == 0.0f) {
		return Vector3{ 0.0f, 0.0f, 0.0f }; // ゼロベクトルへの射影はゼロ
	}

	float scale = dot / normSq;
	return Vector3{
		scale * v2.x,
		scale * v2.y,
		scale * v2.z
	};
}

Vector3 Subtract(const Vector3& v1, const Vector3& v2)
{
	Vector3 result;

	result = { v1.x - v2.x,v1.y - v2.y ,v1.z - v2.z };

	return result;
}

Vector3 Add(const Vector3& v1, const Vector3& v2)
{
	Vector3 result;

	result = { v1.x + v2.x,v1.y + v2.y ,v1.z + v2.z };

	return result;
}

float Dot(const Vector3& v1, const Vector3& v2)
{
	float dot =
		v1.x * v2.x +
		v1.y * v2.y +
		v1.z * v2.z;

	return dot;
}

bool IsCollision(const Segment& segment, const Plane& plane)
{
	float dot = Dot(plane.normal,segment.diff);

	if (dot == 0.0f) {
		return false;
	}

	float t = (plane.distance - Dot(segment.origin, segment.diff)) / dot;

	if (t > 0.0f && t < 1.0f) {
		return true;
	} else {
		return false;
	}

}

float GetLength(const Vector3& v1)
{
	float length = sqrtf(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z);

	return length;
}

Vector3 Perpendicular(const Vector3& vector)
{
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return{ -vector.y,vector.x,0.0f };
	}

	return { 0.0f,-vector.z,vector.y };
}

void DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color)
{
	// 1.中心点を決める
	Vector3 center = {
		plane.distance * plane.normal.x,
		plane.distance * plane.normal.y,
		plane.distance * plane.normal.z,
	};

	// 2.法線と垂直なベクトルを1つ求める
	Vector3 perpendiculars[4];
	perpendiculars[0] = Normalize(Perpendicular(plane.normal));

	// 3.2の逆ベクトルを求める
	perpendiculars[1] = { -perpendiculars[0].x,-perpendiculars[0].y,-perpendiculars[0].z };

	// 4.2と法線のクロス積を求める
	perpendiculars[2] = Cross(plane.normal, perpendiculars[0]);

	// 5.4の逆ベクトルを求める
	perpendiculars[3] = { -perpendiculars[2].x,-perpendiculars[2].y,-perpendiculars[2].z };

	// 6.2~5のベクトルを中心にそれぞれ定数売して足すと4頂点が出来上がる
	Vector3 points[4];
	for (int32_t index = 0; index < 4; ++index) {
		Vector3 extend = {
		2.0f * perpendiculars[index].x,
		2.0f * perpendiculars[index].y,
		2.0f * perpendiculars[index].z,
		};

		Vector3 point = Add(center, extend);
		points[index] = Transform(Transform(point, viewProjectionMatrix), viewportMatrix);

	}

	Novice::DrawLine(
		static_cast<int>(points[0].x),
		static_cast<int>(points[0].y),
		static_cast<int>(points[2].x),
		static_cast<int>(points[2].y), color);

	Novice::DrawLine(
		static_cast<int>(points[2].x),
		static_cast<int>(points[2].y),
		static_cast<int>(points[1].x),
		static_cast<int>(points[1].y), color);

	Novice::DrawLine(
		static_cast<int>(points[1].x),
		static_cast<int>(points[1].y),
		static_cast<int>(points[3].x),
		static_cast<int>(points[3].y), color);

	Novice::DrawLine(
		static_cast<int>(points[3].x),
		static_cast<int>(points[3].y),
		static_cast<int>(points[0].x),
		static_cast<int>(points[0].y), color);

}

Vector3 Normalize(const Vector3& v)
{
	Vector3 normalised;
	float nolm;

	nolm = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);

	normalised = { v.x / nolm,v.y / nolm, v.z / nolm };

	return normalised;
}

Vector3 Cross(const Vector3& v1, const Vector3& v2)
{
	Vector3 result;

	result.x = (v1.y * v2.z) - (v1.z * v2.y);
	result.y = (v1.z * v2.x) - (v1.x * v2.z);
	result.z = (v1.x * v2.y) - (v1.y * v2.x);

	return result;
}

Vector3 ClosestPoint(const Vector3& point, const Segment& segment)
{
	Vector3 closestPoint;
	Vector3 project = Project(Subtract(point, segment.origin), segment.diff);

	closestPoint = { project.x + segment.origin.x,project.y + segment.origin.y ,project.z + segment.origin.z };

	return closestPoint;
}

Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix)
{
	Vector3 resultVector3;

	resultVector3.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0];
	resultVector3.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1];
	resultVector3.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2];

	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];

	assert(w != 0.0f);

	resultVector3.x /= w;
	resultVector3.y /= w;
	resultVector3.z /= w;

	return resultVector3;
}


float Cotangent(float theta)
{
	float cotngent;

	cotngent = 1.0f / std::tanf(theta);

	return cotngent;
}

Matrix4x4 MakeAffineMatrix(Vector3 scale, Vector3 rotate, Vector3 translate)
{
	//====================
	// 拡縮の行列の作成
	//====================
	Matrix4x4 scaleMatrix4x4;
	scaleMatrix4x4.m[0][0] = scale.x;
	scaleMatrix4x4.m[0][1] = 0.0f;
	scaleMatrix4x4.m[0][2] = 0.0f;
	scaleMatrix4x4.m[0][3] = 0.0f;

	scaleMatrix4x4.m[1][0] = 0.0f;
	scaleMatrix4x4.m[1][1] = scale.y;
	scaleMatrix4x4.m[1][2] = 0.0f;
	scaleMatrix4x4.m[1][3] = 0.0f;

	scaleMatrix4x4.m[2][0] = 0.0f;
	scaleMatrix4x4.m[2][1] = 0.0f;
	scaleMatrix4x4.m[2][2] = scale.z;
	scaleMatrix4x4.m[2][3] = 0.0f;

	scaleMatrix4x4.m[3][0] = 0.0f;
	scaleMatrix4x4.m[3][1] = 0.0f;
	scaleMatrix4x4.m[3][2] = 0.0f;
	scaleMatrix4x4.m[3][3] = 1.0f;

	//===================
	// 回転の行列の作成
	//===================
	// Xの回転行列
	Matrix4x4 rotateMatrixX;
	rotateMatrixX.m[0][0] = 1.0f;
	rotateMatrixX.m[0][1] = 0.0f;
	rotateMatrixX.m[0][2] = 0.0f;
	rotateMatrixX.m[0][3] = 0.0f;

	rotateMatrixX.m[1][0] = 0.0f;
	rotateMatrixX.m[1][1] = cosf(rotate.x);
	rotateMatrixX.m[1][2] = sinf(rotate.x);
	rotateMatrixX.m[1][3] = 0.0f;

	rotateMatrixX.m[2][0] = 0.0f;
	rotateMatrixX.m[2][1] = -sinf(rotate.x);
	rotateMatrixX.m[2][2] = cosf(rotate.x);
	rotateMatrixX.m[2][3] = 0.0f;

	rotateMatrixX.m[3][0] = 0.0f;
	rotateMatrixX.m[3][1] = 0.0f;
	rotateMatrixX.m[3][2] = 0.0f;
	rotateMatrixX.m[3][3] = 1.0f;

	// Yの回転行列
	Matrix4x4 rotateMatrixY;
	rotateMatrixY.m[0][0] = cosf(rotate.y);
	rotateMatrixY.m[0][1] = 0.0f;
	rotateMatrixY.m[0][2] = -sinf(rotate.y);
	rotateMatrixY.m[0][3] = 0.0f;

	rotateMatrixY.m[1][0] = 0.0f;
	rotateMatrixY.m[1][1] = 1.0f;
	rotateMatrixY.m[1][2] = 0.0f;
	rotateMatrixY.m[1][3] = 0.0f;

	rotateMatrixY.m[2][0] = sinf(rotate.y);
	rotateMatrixY.m[2][1] = 0.0f;
	rotateMatrixY.m[2][2] = cosf(rotate.y);
	rotateMatrixY.m[2][3] = 0.0f;

	rotateMatrixY.m[3][0] = 0.0f;
	rotateMatrixY.m[3][1] = 0.0f;
	rotateMatrixY.m[3][2] = 0.0f;
	rotateMatrixY.m[3][3] = 1.0f;

	// Zの回転行列
	Matrix4x4 rotateMatrixZ;
	rotateMatrixZ.m[0][0] = cosf(rotate.z);
	rotateMatrixZ.m[0][1] = sinf(rotate.z);
	rotateMatrixZ.m[0][2] = 0.0f;
	rotateMatrixZ.m[0][3] = 0.0f;

	rotateMatrixZ.m[1][0] = -sinf(rotate.z);
	rotateMatrixZ.m[1][1] = cosf(rotate.z);
	rotateMatrixZ.m[1][2] = 0.0f;
	rotateMatrixZ.m[1][3] = 0.0f;

	rotateMatrixZ.m[2][0] = 0.0f;
	rotateMatrixZ.m[2][1] = 0.0f;
	rotateMatrixZ.m[2][2] = 1.0f;
	rotateMatrixZ.m[2][3] = 0.0f;

	rotateMatrixZ.m[3][0] = 0.0f;
	rotateMatrixZ.m[3][1] = 0.0f;
	rotateMatrixZ.m[3][2] = 0.0f;
	rotateMatrixZ.m[3][3] = 1.0f;

	// 回転行列の作成
	Matrix4x4 rotateMatrix4x4;

	rotateMatrix4x4 = Multiply(rotateMatrixX, Multiply(rotateMatrixY, rotateMatrixZ));

	//==================
	// 移動の行列の作成
	//==================
	Matrix4x4 translateMatrix4x4;
	translateMatrix4x4.m[0][0] = 1.0f;
	translateMatrix4x4.m[0][1] = 0.0f;
	translateMatrix4x4.m[0][2] = 0.0f;
	translateMatrix4x4.m[0][3] = 0.0f;

	translateMatrix4x4.m[1][0] = 0.0f;
	translateMatrix4x4.m[1][1] = 1.0f;
	translateMatrix4x4.m[1][2] = 0.0f;
	translateMatrix4x4.m[1][3] = 0.0f;

	translateMatrix4x4.m[2][0] = 0.0f;
	translateMatrix4x4.m[2][1] = 0.0f;
	translateMatrix4x4.m[2][2] = 1.0f;
	translateMatrix4x4.m[2][3] = 0.0f;

	translateMatrix4x4.m[3][0] = translate.x;
	translateMatrix4x4.m[3][1] = translate.y;
	translateMatrix4x4.m[3][2] = translate.z;
	translateMatrix4x4.m[3][3] = 1.0f;

	//====================
	// アフィン行列の作成
	//====================
	// 上で作った行列からアフィン行列を作る
	// アフィン行列の作成（スケール→回転→移動の順）
	Matrix4x4 affineMatrix4x4;
	affineMatrix4x4 = Multiply(scaleMatrix4x4, Multiply(rotateMatrix4x4, translateMatrix4x4));

	return  affineMatrix4x4;
}

Matrix4x4 Inverse(Matrix4x4 matrix4x4)
{
	// 行列式|A|を求める
	float bottom =
		(matrix4x4.m[0][0] * matrix4x4.m[1][1] * matrix4x4.m[2][2] * matrix4x4.m[3][3])
		+ (matrix4x4.m[0][0] * matrix4x4.m[1][2] * matrix4x4.m[2][3] * matrix4x4.m[3][1])
		+ (matrix4x4.m[0][0] * matrix4x4.m[1][3] * matrix4x4.m[2][1] * matrix4x4.m[3][2])
		- (matrix4x4.m[0][0] * matrix4x4.m[1][3] * matrix4x4.m[2][2] * matrix4x4.m[3][1])
		- (matrix4x4.m[0][0] * matrix4x4.m[1][2] * matrix4x4.m[2][1] * matrix4x4.m[3][3])
		- (matrix4x4.m[0][0] * matrix4x4.m[1][1] * matrix4x4.m[2][3] * matrix4x4.m[3][2])
		- (matrix4x4.m[0][1] * matrix4x4.m[1][0] * matrix4x4.m[2][2] * matrix4x4.m[3][3])
		- (matrix4x4.m[0][2] * matrix4x4.m[1][0] * matrix4x4.m[2][3] * matrix4x4.m[3][1])
		- (matrix4x4.m[0][3] * matrix4x4.m[1][0] * matrix4x4.m[2][1] * matrix4x4.m[3][2])
		+ (matrix4x4.m[0][3] * matrix4x4.m[1][0] * matrix4x4.m[2][2] * matrix4x4.m[3][1])
		+ (matrix4x4.m[0][2] * matrix4x4.m[1][0] * matrix4x4.m[2][1] * matrix4x4.m[3][3])
		+ (matrix4x4.m[0][1] * matrix4x4.m[1][0] * matrix4x4.m[2][3] * matrix4x4.m[3][2])
		+ (matrix4x4.m[0][1] * matrix4x4.m[1][2] * matrix4x4.m[2][0] * matrix4x4.m[3][3])
		+ (matrix4x4.m[0][2] * matrix4x4.m[1][3] * matrix4x4.m[2][0] * matrix4x4.m[3][1])
		+ (matrix4x4.m[0][3] * matrix4x4.m[1][1] * matrix4x4.m[2][0] * matrix4x4.m[3][2])
		- (matrix4x4.m[0][3] * matrix4x4.m[1][2] * matrix4x4.m[2][0] * matrix4x4.m[3][1])
		- (matrix4x4.m[0][2] * matrix4x4.m[1][1] * matrix4x4.m[2][0] * matrix4x4.m[3][3])
		- (matrix4x4.m[0][1] * matrix4x4.m[1][3] * matrix4x4.m[2][0] * matrix4x4.m[3][2])
		- (matrix4x4.m[0][1] * matrix4x4.m[1][2] * matrix4x4.m[2][3] * matrix4x4.m[3][0])
		- (matrix4x4.m[0][2] * matrix4x4.m[1][3] * matrix4x4.m[2][1] * matrix4x4.m[3][0])
		- (matrix4x4.m[0][3] * matrix4x4.m[1][1] * matrix4x4.m[2][2] * matrix4x4.m[3][0])
		+ (matrix4x4.m[0][3] * matrix4x4.m[1][2] * matrix4x4.m[2][1] * matrix4x4.m[3][0])
		+ (matrix4x4.m[0][2] * matrix4x4.m[1][1] * matrix4x4.m[2][3] * matrix4x4.m[3][0])
		+ (matrix4x4.m[0][1] * matrix4x4.m[1][3] * matrix4x4.m[2][2] * matrix4x4.m[3][0]);

	Matrix4x4 resoultMatrix;

	// 1行目
	resoultMatrix.m[0][0] = 1.0f / bottom * (
		(matrix4x4.m[1][1] * matrix4x4.m[2][2] * matrix4x4.m[3][3])
		+ (matrix4x4.m[1][2] * matrix4x4.m[2][3] * matrix4x4.m[3][1])
		+ (matrix4x4.m[1][3] * matrix4x4.m[2][1] * matrix4x4.m[3][2])
		- (matrix4x4.m[1][3] * matrix4x4.m[2][2] * matrix4x4.m[3][1])
		- (matrix4x4.m[1][2] * matrix4x4.m[2][1] * matrix4x4.m[3][3])
		- (matrix4x4.m[1][1] * matrix4x4.m[2][3] * matrix4x4.m[3][2]));

	resoultMatrix.m[0][1] = 1.0f / bottom * (
		-(matrix4x4.m[0][1] * matrix4x4.m[2][2] * matrix4x4.m[3][3])
		- (matrix4x4.m[0][2] * matrix4x4.m[2][3] * matrix4x4.m[3][1])
		- (matrix4x4.m[0][3] * matrix4x4.m[2][1] * matrix4x4.m[3][2])
		+ (matrix4x4.m[0][3] * matrix4x4.m[2][2] * matrix4x4.m[3][1])
		+ (matrix4x4.m[0][2] * matrix4x4.m[2][1] * matrix4x4.m[3][3])
		+ (matrix4x4.m[0][1] * matrix4x4.m[2][3] * matrix4x4.m[3][2]));

	resoultMatrix.m[0][2] = 1.0f / bottom * (
		(matrix4x4.m[0][1] * matrix4x4.m[1][2] * matrix4x4.m[3][3])
		+ (matrix4x4.m[0][2] * matrix4x4.m[1][3] * matrix4x4.m[3][1])
		+ (matrix4x4.m[0][3] * matrix4x4.m[1][1] * matrix4x4.m[3][2])
		- (matrix4x4.m[0][3] * matrix4x4.m[1][2] * matrix4x4.m[3][1])
		- (matrix4x4.m[0][2] * matrix4x4.m[1][1] * matrix4x4.m[3][3])
		- (matrix4x4.m[0][1] * matrix4x4.m[1][3] * matrix4x4.m[3][2]));

	resoultMatrix.m[0][3] = 1.0f / bottom * (
		-(matrix4x4.m[0][1] * matrix4x4.m[1][2] * matrix4x4.m[2][3])
		- (matrix4x4.m[0][2] * matrix4x4.m[1][3] * matrix4x4.m[2][1])
		- (matrix4x4.m[0][3] * matrix4x4.m[1][1] * matrix4x4.m[2][2])
		+ (matrix4x4.m[0][3] * matrix4x4.m[1][2] * matrix4x4.m[2][1])
		+ (matrix4x4.m[0][2] * matrix4x4.m[1][1] * matrix4x4.m[2][3])
		+ (matrix4x4.m[0][1] * matrix4x4.m[1][3] * matrix4x4.m[2][2]));

	// 2行目
	resoultMatrix.m[1][0] = 1.0f / bottom * (
		-(matrix4x4.m[1][0] * matrix4x4.m[2][2] * matrix4x4.m[3][3])
		- (matrix4x4.m[1][2] * matrix4x4.m[2][3] * matrix4x4.m[3][0])
		- (matrix4x4.m[1][3] * matrix4x4.m[2][0] * matrix4x4.m[3][2])
		+ (matrix4x4.m[1][3] * matrix4x4.m[2][2] * matrix4x4.m[3][0])
		+ (matrix4x4.m[1][2] * matrix4x4.m[2][0] * matrix4x4.m[3][3])
		+ (matrix4x4.m[1][0] * matrix4x4.m[2][3] * matrix4x4.m[3][2]));

	resoultMatrix.m[1][1] = 1.0f / bottom * (
		(matrix4x4.m[0][0] * matrix4x4.m[2][2] * matrix4x4.m[3][3])
		+ (matrix4x4.m[0][2] * matrix4x4.m[2][3] * matrix4x4.m[3][0])
		+ (matrix4x4.m[0][3] * matrix4x4.m[2][0] * matrix4x4.m[3][2])
		- (matrix4x4.m[0][3] * matrix4x4.m[2][2] * matrix4x4.m[3][0])
		- (matrix4x4.m[0][2] * matrix4x4.m[2][0] * matrix4x4.m[3][3])
		- (matrix4x4.m[0][0] * matrix4x4.m[2][3] * matrix4x4.m[3][2]));

	resoultMatrix.m[1][2] = 1.0f / bottom * (
		-(matrix4x4.m[0][0] * matrix4x4.m[1][2] * matrix4x4.m[3][3])
		- (matrix4x4.m[0][2] * matrix4x4.m[1][3] * matrix4x4.m[3][0])
		- (matrix4x4.m[0][3] * matrix4x4.m[1][0] * matrix4x4.m[3][2])
		+ (matrix4x4.m[0][3] * matrix4x4.m[1][2] * matrix4x4.m[3][0])
		+ (matrix4x4.m[0][2] * matrix4x4.m[1][0] * matrix4x4.m[3][3])
		+ (matrix4x4.m[0][0] * matrix4x4.m[1][3] * matrix4x4.m[3][2]));

	resoultMatrix.m[1][3] = 1.0f / bottom * (
		(matrix4x4.m[0][0] * matrix4x4.m[1][2] * matrix4x4.m[2][3])
		+ (matrix4x4.m[0][2] * matrix4x4.m[1][3] * matrix4x4.m[2][0])
		+ (matrix4x4.m[0][3] * matrix4x4.m[1][0] * matrix4x4.m[2][2])
		- (matrix4x4.m[0][3] * matrix4x4.m[1][2] * matrix4x4.m[2][0])
		- (matrix4x4.m[0][2] * matrix4x4.m[1][0] * matrix4x4.m[2][3])
		- (matrix4x4.m[0][0] * matrix4x4.m[1][3] * matrix4x4.m[2][2]));

	// 3行目
	resoultMatrix.m[2][0] = 1.0f / bottom * (
		(matrix4x4.m[1][0] * matrix4x4.m[2][1] * matrix4x4.m[3][3])
		+ (matrix4x4.m[1][1] * matrix4x4.m[2][3] * matrix4x4.m[3][0])
		+ (matrix4x4.m[1][3] * matrix4x4.m[2][0] * matrix4x4.m[3][1])
		- (matrix4x4.m[1][3] * matrix4x4.m[2][1] * matrix4x4.m[3][0])
		- (matrix4x4.m[1][1] * matrix4x4.m[2][0] * matrix4x4.m[3][3])
		- (matrix4x4.m[1][0] * matrix4x4.m[2][3] * matrix4x4.m[3][1]));

	resoultMatrix.m[2][1] = 1.0f / bottom * (
		-(matrix4x4.m[0][0] * matrix4x4.m[2][1] * matrix4x4.m[3][3])
		- (matrix4x4.m[0][1] * matrix4x4.m[2][3] * matrix4x4.m[3][0])
		- (matrix4x4.m[0][3] * matrix4x4.m[2][0] * matrix4x4.m[3][1])
		+ (matrix4x4.m[0][3] * matrix4x4.m[2][1] * matrix4x4.m[3][0])
		+ (matrix4x4.m[0][1] * matrix4x4.m[2][0] * matrix4x4.m[3][3])
		+ (matrix4x4.m[0][0] * matrix4x4.m[2][3] * matrix4x4.m[3][1]));

	resoultMatrix.m[2][2] = 1.0f / bottom * (
		(matrix4x4.m[0][0] * matrix4x4.m[1][1] * matrix4x4.m[3][3])
		+ (matrix4x4.m[0][1] * matrix4x4.m[1][3] * matrix4x4.m[3][0])
		+ (matrix4x4.m[0][3] * matrix4x4.m[1][0] * matrix4x4.m[3][1])
		- (matrix4x4.m[0][3] * matrix4x4.m[1][1] * matrix4x4.m[3][0])
		- (matrix4x4.m[0][1] * matrix4x4.m[1][0] * matrix4x4.m[3][3])
		- (matrix4x4.m[0][0] * matrix4x4.m[1][3] * matrix4x4.m[3][1]));

	resoultMatrix.m[2][3] = 1.0f / bottom * (
		-(matrix4x4.m[0][0] * matrix4x4.m[1][1] * matrix4x4.m[2][3])
		- (matrix4x4.m[0][1] * matrix4x4.m[1][3] * matrix4x4.m[2][0])
		- (matrix4x4.m[0][3] * matrix4x4.m[1][0] * matrix4x4.m[2][1])
		+ (matrix4x4.m[0][3] * matrix4x4.m[1][1] * matrix4x4.m[2][0])
		+ (matrix4x4.m[0][1] * matrix4x4.m[1][0] * matrix4x4.m[2][3])
		+ (matrix4x4.m[0][0] * matrix4x4.m[1][3] * matrix4x4.m[2][1]));

	// 4行目
	resoultMatrix.m[3][0] = 1.0f / bottom * (
		-(matrix4x4.m[1][0] * matrix4x4.m[2][1] * matrix4x4.m[3][2])
		- (matrix4x4.m[1][1] * matrix4x4.m[2][2] * matrix4x4.m[3][0])
		- (matrix4x4.m[1][2] * matrix4x4.m[2][0] * matrix4x4.m[3][1])
		+ (matrix4x4.m[1][2] * matrix4x4.m[2][1] * matrix4x4.m[3][0])
		+ (matrix4x4.m[1][1] * matrix4x4.m[2][0] * matrix4x4.m[3][2])
		+ (matrix4x4.m[1][0] * matrix4x4.m[2][2] * matrix4x4.m[3][1]));

	resoultMatrix.m[3][1] = 1.0f / bottom * (
		(matrix4x4.m[0][0] * matrix4x4.m[2][1] * matrix4x4.m[3][2])
		+ (matrix4x4.m[0][1] * matrix4x4.m[2][2] * matrix4x4.m[3][0])
		+ (matrix4x4.m[0][2] * matrix4x4.m[2][0] * matrix4x4.m[3][1])
		- (matrix4x4.m[0][2] * matrix4x4.m[2][1] * matrix4x4.m[3][0])
		- (matrix4x4.m[0][1] * matrix4x4.m[2][0] * matrix4x4.m[3][2])
		- (matrix4x4.m[0][0] * matrix4x4.m[2][2] * matrix4x4.m[3][1]));

	resoultMatrix.m[3][2] = 1.0f / bottom * (
		-(matrix4x4.m[0][0] * matrix4x4.m[1][1] * matrix4x4.m[3][2])
		- (matrix4x4.m[0][1] * matrix4x4.m[1][2] * matrix4x4.m[3][0])
		- (matrix4x4.m[0][2] * matrix4x4.m[1][0] * matrix4x4.m[3][1])
		+ (matrix4x4.m[0][2] * matrix4x4.m[1][1] * matrix4x4.m[3][0])
		+ (matrix4x4.m[0][1] * matrix4x4.m[1][0] * matrix4x4.m[3][2])
		+ (matrix4x4.m[0][0] * matrix4x4.m[1][2] * matrix4x4.m[3][1]));

	resoultMatrix.m[3][3] = 1.0f / bottom * (
		(matrix4x4.m[0][0] * matrix4x4.m[1][1] * matrix4x4.m[2][2])
		+ (matrix4x4.m[0][1] * matrix4x4.m[1][2] * matrix4x4.m[2][0])
		+ (matrix4x4.m[0][2] * matrix4x4.m[1][0] * matrix4x4.m[2][1])
		- (matrix4x4.m[0][2] * matrix4x4.m[1][1] * matrix4x4.m[2][0])
		- (matrix4x4.m[0][1] * matrix4x4.m[1][0] * matrix4x4.m[2][2])
		- (matrix4x4.m[0][0] * matrix4x4.m[1][2] * matrix4x4.m[2][1]));

	return resoultMatrix;
}

Matrix4x4 Multiply(Matrix4x4 matrix1, Matrix4x4 matrix2)
{
	Matrix4x4 resoultMatrix4x4;

	resoultMatrix4x4.m[0][0] = matrix1.m[0][0] * matrix2.m[0][0] + matrix1.m[0][1] * matrix2.m[1][0] + matrix1.m[0][2] * matrix2.m[2][0] + matrix1.m[0][3] * matrix2.m[3][0];
	resoultMatrix4x4.m[0][1] = matrix1.m[0][0] * matrix2.m[0][1] + matrix1.m[0][1] * matrix2.m[1][1] + matrix1.m[0][2] * matrix2.m[2][1] + matrix1.m[0][3] * matrix2.m[3][1];
	resoultMatrix4x4.m[0][2] = matrix1.m[0][0] * matrix2.m[0][2] + matrix1.m[0][1] * matrix2.m[1][2] + matrix1.m[0][2] * matrix2.m[2][2] + matrix1.m[0][3] * matrix2.m[3][2];
	resoultMatrix4x4.m[0][3] = matrix1.m[0][0] * matrix2.m[0][3] + matrix1.m[0][1] * matrix2.m[1][3] + matrix1.m[0][2] * matrix2.m[2][3] + matrix1.m[0][3] * matrix2.m[3][3];

	resoultMatrix4x4.m[1][0] = matrix1.m[1][0] * matrix2.m[0][0] + matrix1.m[1][1] * matrix2.m[1][0] + matrix1.m[1][2] * matrix2.m[2][0] + matrix1.m[1][3] * matrix2.m[3][0];
	resoultMatrix4x4.m[1][1] = matrix1.m[1][0] * matrix2.m[0][1] + matrix1.m[1][1] * matrix2.m[1][1] + matrix1.m[1][2] * matrix2.m[2][1] + matrix1.m[1][3] * matrix2.m[3][1];
	resoultMatrix4x4.m[1][2] = matrix1.m[1][0] * matrix2.m[0][2] + matrix1.m[1][1] * matrix2.m[1][2] + matrix1.m[1][2] * matrix2.m[2][2] + matrix1.m[1][3] * matrix2.m[3][2];
	resoultMatrix4x4.m[1][3] = matrix1.m[1][0] * matrix2.m[0][3] + matrix1.m[1][1] * matrix2.m[1][3] + matrix1.m[1][2] * matrix2.m[2][3] + matrix1.m[1][3] * matrix2.m[3][3];

	resoultMatrix4x4.m[2][0] = matrix1.m[2][0] * matrix2.m[0][0] + matrix1.m[2][1] * matrix2.m[1][0] + matrix1.m[2][2] * matrix2.m[2][0] + matrix1.m[2][3] * matrix2.m[3][0];
	resoultMatrix4x4.m[2][1] = matrix1.m[2][0] * matrix2.m[0][1] + matrix1.m[2][1] * matrix2.m[1][1] + matrix1.m[2][2] * matrix2.m[2][1] + matrix1.m[2][3] * matrix2.m[3][1];
	resoultMatrix4x4.m[2][2] = matrix1.m[2][0] * matrix2.m[0][2] + matrix1.m[2][1] * matrix2.m[1][2] + matrix1.m[2][2] * matrix2.m[2][2] + matrix1.m[2][3] * matrix2.m[3][2];
	resoultMatrix4x4.m[2][3] = matrix1.m[2][0] * matrix2.m[0][3] + matrix1.m[2][1] * matrix2.m[1][3] + matrix1.m[2][2] * matrix2.m[2][3] + matrix1.m[2][3] * matrix2.m[3][3];

	resoultMatrix4x4.m[3][0] = matrix1.m[3][0] * matrix2.m[0][0] + matrix1.m[3][1] * matrix2.m[1][0] + matrix1.m[3][2] * matrix2.m[2][0] + matrix1.m[3][3] * matrix2.m[3][0];
	resoultMatrix4x4.m[3][1] = matrix1.m[3][0] * matrix2.m[0][1] + matrix1.m[3][1] * matrix2.m[1][1] + matrix1.m[3][2] * matrix2.m[2][1] + matrix1.m[3][3] * matrix2.m[3][1];
	resoultMatrix4x4.m[3][2] = matrix1.m[3][0] * matrix2.m[0][2] + matrix1.m[3][1] * matrix2.m[1][2] + matrix1.m[3][2] * matrix2.m[2][2] + matrix1.m[3][3] * matrix2.m[3][2];
	resoultMatrix4x4.m[3][3] = matrix1.m[3][0] * matrix2.m[0][3] + matrix1.m[3][1] * matrix2.m[1][3] + matrix1.m[3][2] * matrix2.m[2][3] + matrix1.m[3][3] * matrix2.m[3][3];

	return resoultMatrix4x4;
}

Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth)
{
	Matrix4x4 viewportMatrix4x4;

	viewportMatrix4x4.m[0][0] = width / 2.0f;
	viewportMatrix4x4.m[0][1] = 0.0f;
	viewportMatrix4x4.m[0][2] = 0.0f;
	viewportMatrix4x4.m[0][3] = 0.0f;

	viewportMatrix4x4.m[1][0] = 0.0f;
	viewportMatrix4x4.m[1][1] = -height / 2.0f;
	viewportMatrix4x4.m[1][2] = 0.0f;
	viewportMatrix4x4.m[1][3] = 0.0f;

	viewportMatrix4x4.m[2][0] = 0.0f;
	viewportMatrix4x4.m[2][1] = 0.0f;
	viewportMatrix4x4.m[2][2] = maxDepth - minDepth;
	viewportMatrix4x4.m[2][3] = 0.0f;

	viewportMatrix4x4.m[3][0] = left + width / 2.0f;
	viewportMatrix4x4.m[3][1] = top + height / 2.0f;
	viewportMatrix4x4.m[3][2] = minDepth;
	viewportMatrix4x4.m[3][3] = 1.0f;

	return viewportMatrix4x4;
}

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip)
{
	Matrix4x4 perspectiveFovMatrix;

	perspectiveFovMatrix.m[0][0] = 1.0f / aspectRatio * Cotangent(fovY / 2.0f);
	perspectiveFovMatrix.m[0][1] = 0.0f;
	perspectiveFovMatrix.m[0][2] = 0.0f;
	perspectiveFovMatrix.m[0][3] = 0.0f;

	perspectiveFovMatrix.m[1][0] = 0.0f;
	perspectiveFovMatrix.m[1][1] = Cotangent(fovY / 2.0f);
	perspectiveFovMatrix.m[1][2] = 0.0f;
	perspectiveFovMatrix.m[1][3] = 0.0f;

	perspectiveFovMatrix.m[2][0] = 0.0f;
	perspectiveFovMatrix.m[2][1] = 0.0f;
	perspectiveFovMatrix.m[2][2] = farClip / (farClip - nearClip);
	perspectiveFovMatrix.m[2][3] = 1.0f;

	perspectiveFovMatrix.m[3][0] = 0.0f;
	perspectiveFovMatrix.m[3][1] = 0.0f;
	perspectiveFovMatrix.m[3][2] = (-nearClip * farClip) / (farClip - nearClip);
	perspectiveFovMatrix.m[3][3] = 0.0f;

	return perspectiveFovMatrix;
}

Vector3 TransformWithoutW(const Vector3& vector, const Matrix4x4& matrix)
{
	Vector3 result;
	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + matrix.m[3][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + matrix.m[3][1];
	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + matrix.m[3][2];
	return result;
}