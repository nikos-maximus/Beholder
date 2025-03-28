#include <glm/ext.hpp>
#include "bhCamera.hpp"

const float bhCamera::DEFAULT_FOV_Y_DEGS = 45.0f; // In DEGREES
const glm::vec3 bhCamera::WORLD_UP_VEC(0.0f, 0.0f, 1.0f);
static glm::mat4 g_utilityMat4f;

bhCamera::bhCamera()
	: fwd(0.0f, 1.0f, 0.0f)
	, right(1.0f, 0.0f, 0.0f)
	, up(0.0f, 0.0f, 1.0f)
{
	SetProjection(DEFAULT_FOV_Y_DEGS, 1.0f, 0.1f, 1000.0f); // Just setting this to an obviously incorrect default
}

void bhCamera::SetProjection(float fovy, float aspect, float near, float far)
{
	projection = glm::perspective(fovy, aspect, near, far);
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

const glm::mat4& bhCamera::GetProjection()
{
	//viewData.view = glm::lookAt(viewData.pos, viewData.pos + fwd, up);
	return projection;
}
