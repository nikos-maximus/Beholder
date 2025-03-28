#ifndef BH_CAMERA_HPP
#define BH_CAMERA_HPP

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

class bhCamera
{
public:
    ////////////////////////////////////////////////////////////////////////////////
    struct ViewData
    {
        glm::mat4 view, projection;
        glm::vec3 pos;
    };

    ////////////////////////////////////////////////////////////////////////////////
    bhCamera();
    void SetPosition(const glm::vec3& pos);
    void LookAt(const glm::vec3& target);
    void SetProjection(float fovy, float aspect, float near, float far);
    void Yaw(float degrees);
    void Pitch(float degrees);
    void Roll(float degrees);

    const glm::vec3& GetFwdVec() const
    { 
        return fwd; 
    }

    const glm::vec3& GetRightVec() const
    { 
        return right; 
    }

    //const glm::mat4* GetView() const 
    //{ 
    //    return viewData.view; 
    //}

    //const glm::mat4* GetProjection() const 
    //{ 
    //    return viewData.projection; 
    //}

    const ViewData& GetViewData();

    static const float DEFAULT_FOV_Y_DEGS; // In DEGREES
    static const glm::vec3 WORLD_UP_VEC;

protected:
    ViewData viewData;
    glm::vec3 fwd, right, up;

private:
};

#if 0
////////////////////////////////////////////////////////////////////////////////
// Camera Component
class bhCamera_Cmp : public bhComponent
{
public:
    bhCamera_Cmp(bhCTransform& _transform);

    bhCTransform& transform;
    bhCamera camera;

protected:
private:
};

////////////////////////////////////////////////////////////////////////////////
// Camera Entity
class bhCamera_Ent : public bhEntity
{
public:
    void Move(const glm::vec3& inputAxis);
    glm::vec3 GetXYVelocity(const glm::vec3& inputAxis);
    void UpdateView() { cam.UpdateView(transform.GetPosition()); }

    bhCTransform transform;
    bhCamera cam;

protected:
private:
};
#endif

#endif //BH_CAMERA_HPP
