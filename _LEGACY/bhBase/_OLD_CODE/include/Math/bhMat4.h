#ifndef BH_MAT4_H
#define BH_MAT4_H

#include "bhVec3.h"
#include "bhVec4.h"

#ifdef __cplusplus
extern "C"
{
#endif

	////////////////////////////////////////////////////////////////////////////////
	
	typedef struct
	{
		union
		{
			bhVec4 cols[4];
			float m[4][4];
		};
	}
	bhMat4;

	bhMat4 bhMat4_Zero()
	{
		bhMat4 out;
		for (int c = 0; c < 4; ++c)
		{
			for (int r = 0; r < 4; ++r)
			{
				out.m[c][r] = 0.f;
			}
		}
		return out;
	}

	bhMat4 bhMat4_Identity()
	{
		bhMat4 out = bhMat4_Zero();
		out.m[0][0] = out.m[1][1] = out.m[2][2] = out.m[3][3] = 1.f;
		return out;
	}

	bhMat4 bhMat4_Transpose(const bhMat4* src)
	{
		bhMat4 out;
		for (int c = 0; c < 4; ++c)
		{
			for (int r = 0; r < 4; ++r)
			{
				out.m[r][c] = src->m[c][r];
			}
		}
		return out;
	}

	bhVec4 bhMat4_Vec4Mult(const bhMat4* mat, bhVec4 vec)
	{
		bhMat4 matT = bhMat4_Transpose(mat);
		bhVec4 out = {
			bhVec4_Dot(matT.cols[0], vec),
			bhVec4_Dot(matT.cols[1], vec),
			bhVec4_Dot(matT.cols[2], vec),
			bhVec4_Dot(matT.cols[3], vec)
		};
		return out;
	}

	bhMat4 bhMat4_Vec4Mult(const bhMat4* left, const bhMat4* right)
	{
		bhMat4 leftT = bhMat4_Transpose(left);
		bhMat4 out;
		for (int c = 0; c < 4; ++c)
		{
			for (int r = 0; r < 4; ++r)
			{
				out.m[r][c] = bhVec4_Dot(leftT.cols[r], right->cols[c]); // TODO: verify this!
			}
		}
		return out;
	}

	bhMat4 bhMat4_Perspective(float fovyRads, float aspect, float nearPlane, float farPlane)
	{
		float tanHalfFov = tanf(fovyRads / 2.f);
		bhMat4 out;
		out.m[0][0] = 1.f / (aspect * tanHalfFov);
		out.m[1][1] = 1.f / tanHalfFov;
		out.m[2][2] = (nearPlane + farPlane) / (nearPlane - farPlane); // We need the negation in the denominator, for right-handed coord systems (OpenGL, Vulkan)
		out.m[2][3] = -1.f;
		out.m[3][2] = 2.f * nearPlane * farPlane / (nearPlane - farPlane);
		return out;
	}

	bhMat4 bhMat4_LookAt(bhVec3 eye, bhVec3 target, bhVec3 up)
	{
		bhMat4 out;
		bhVec3 fwd = bhVec3_Normalize(bhVec3_Sub(target, eye));
		bhVec3 right = bhVec3_Normalize(bhVec3_Cross(fwd, up));
		bhVec3 nup = bhVec3_Cross(right, fwd);

		out.m[0][0] = right.x;
		out.m[0][1] = nup.x;
		out.m[0][2] = -fwd.x;
		out.m[0][3] = 0.f;

		out.m[1][0] = right.y;
		out.m[1][1] = nup.y;
		out.m[1][2] = -fwd.y;
		out.m[1][3] = 0.f;

		out.m[2][0] = right.z;
		out.m[2][1] = nup.z;
		out.m[2][2] = -fwd.z;
		out.m[2][3] = 0.f;

		out.m[3][0] = -Dot(right, eye);
		out.m[3][1] = -Dot(nup, eye);
		out.m[3][2] = Dot(fwd, eye);
		out.m[3][3] = 1.f;
	}

	////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif //BH_MAT4_H

