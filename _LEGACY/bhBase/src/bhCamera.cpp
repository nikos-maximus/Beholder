#include "bhCamera.hpp"
#include "bhDefines.h"
#include <glm/ext.hpp>

////////////////////////////////////////////////////////////////////////////////
const float bhCamera::DEFAULT_FOV_Y_DEGS = 45.0f; // In DEGREES
const glm::vec3 bhCamera::WORLD_UP_VEC(0.0f, 0.0f, 1.0f);
static glm::mat4 g_utilityMat4f;

////////////////////////////////////////////////////////////////////////////////
bhCamera::bhCamera()
	: fwd(0.0f, 1.0f, 0.0f)
	, right(1.0f, 0.0f, 0.0f)
	, up(0.0f, 0.0f, 1.0f)
{
	SetProjection(DEFAULT_FOV_Y_DEGS, 1.0f, 0.1f, 1000.0f); // Just setting this to an obviously incorrect default
}

void bhCamera::SetPosition(const glm::vec3& pos)
{
	viewData.pos = pos;
}

void bhCamera::LookAt(const glm::vec3& target)
{
	fwd = glm::normalize(target - viewData.pos);
	right = glm::normalize(glm::cross(fwd, up));
}

void bhCamera::SetProjection(float fovy, float aspect, float near, float far)
{
	viewData.projection = glm::perspective(fovy, aspect, near, far);
}

void bhCamera::Yaw(float degrees)
{
	g_utilityMat4f = glm::rotate(glm::mat4(1.0f), glm::radians(degrees), WORLD_UP_VEC);
	fwd = glm::normalize(glm::vec3(g_utilityMat4f * glm::vec4(fwd, 1.0f)));
	right = glm::normalize(glm::cross(fwd, WORLD_UP_VEC));
	up = glm::normalize(glm::cross(right, fwd));
}

void bhCamera::Pitch(float degrees)
{
	g_utilityMat4f = glm::rotate(glm::mat4(1.0f), glm::radians(degrees), right);
	fwd = glm::normalize(glm::vec3(g_utilityMat4f * glm::vec4(fwd, 1.0f)));
	up = glm::normalize(glm::cross(right, fwd));
}

void bhCamera::Roll(float degrees)
{
	g_utilityMat4f = glm::rotate(glm::mat4(1.0f), glm::radians(degrees), fwd);
	right = glm::normalize(glm::vec3(g_utilityMat4f * glm::vec4(right, 1.0f)));
	up = glm::normalize(glm::cross(right, fwd));
}

const bhCamera::ViewData& bhCamera::GetViewData()
{
	viewData.view = glm::lookAt(viewData.pos, viewData.pos + fwd, up);
	return viewData;
}

#if 0
////////////////////////////////////////////////////////////////////////////////
inline void Move_Common(bhCTransform& transform, bhCamera const& cam, glm::vec3 const& inputAxis)
{
	glm::vec3 relMove;
	relMove += cam.GetFwdVec() * inputAxis.x;
	relMove += cam.GetRightVec() * inputAxis.y;
	transform.AddTranslation(relMove);
}

inline glm::vec3 GetXYVelocity_Common(bhCTransform& transform, bhCamera const& cam, glm::vec3 const& inputAxis)
{
	glm::vec3 relMove;
	relMove += cam.GetFwdVec() * inputAxis.x;
	relMove += cam.GetRightVec() * inputAxis.y;
	return glm::normalize(relMove);
}

////////////////////////////////////////////////////////////////////////////////
// Camera Component
bhCamera_Cmp::bhCamera_Cmp(bhCTransform& _transform)
	: transform(_transform)
{}

////////////////////////////////////////////////////////////////////////////////
// Camera Entity
void bhCamera_Ent::Move(glm::vec3 const& inputAxis)
{
	GetXYVelocity_Common(transform, cam, inputAxis);
}

glm::vec3 bhCamera_Ent::GetXYVelocity(glm::vec3 const& inputAxis)
{
	return GetXYVelocity_Common(transform, cam, inputAxis);
}
#endif
