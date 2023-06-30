#pragma once

typedef struct Vector3 {
	float x;
	float y;
	float z;
};

typedef struct Matrix4x4 {
	float m[4][4];
};

// ��
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);
// �g��k���s��
Matrix4x4 MakeScaleMatrix(const Vector3& scale);
// ���s�ړ�
Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

// �r���[�|�[�g�ϊ��s��
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float heght, float minDepth, float maxDepth);


/*---------------------------------
 ��]�s��
------------------------------------*/

// X��
Matrix4x4 MakeRotateXMatrix(float radian);
// Y��
Matrix4x4 MakeRotateYMatrix(float radian);
// Z��
Matrix4x4 MakeRotateZMatrix(float radian);

// �A�t�B���ϊ�
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rot, const Vector3& translate);

// �������e�s��
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

//���ˉe�s��
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

// �t�s��
Matrix4x4 Inverse(const Matrix4x4& m);

// �P�ʍs��
Matrix4x4 MakeIdentityMatrix();
