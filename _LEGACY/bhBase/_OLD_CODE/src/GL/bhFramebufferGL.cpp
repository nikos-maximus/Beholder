#include "GL/bhDeviceGL.hpp"

////////////////////////////////////////////////////////////////////////////////
bhResourceID bhDeviceGL::CreateFramebuffer(bhSize2Di size, uint32_t numColorAttachments, bool createDepthStencil)
{
    // TODO: Adjust size if necessary?
    bhFramebufferGL* newFB = new bhFramebufferGL();
    newFB->size = size;

    GLint currFramebuffer = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currFramebuffer);

    glGenFramebuffers(1, &(newFB->framebuffer));
    glBindFramebuffer(GL_FRAMEBUFFER, newFB->framebuffer); //GL_READ_FRAMEBUFFER GL_DRAW_FRAMEBUFFER

    newFB->colorAttachments_v.resize(numColorAttachments);
    std::vector<GLenum> attachmentIdx_v(numColorAttachments);
    glGenTextures(numColorAttachments, newFB->colorAttachments_v.data());
    for (GLsizei ca = 0; ca < numColorAttachments; ++ca)
    {
        glBindTexture(GL_TEXTURE_2D, newFB->colorAttachments_v[ca]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.width, size.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        attachmentIdx_v[ca] = GL_COLOR_ATTACHMENT0 + ca;
        glFramebufferTexture(GL_FRAMEBUFFER, attachmentIdx_v[ca], newFB->colorAttachments_v[ca], 0);
        //glNamedFramebufferTexture(newFB->framebuffer, attachmentIdx_v[ca], newFB->colorAttachments_v[ca], 0);
    }

    if (createDepthStencil)
    {
        glGenRenderbuffers(1, &(newFB->depthStencilAttachment));
        glBindRenderbuffer(GL_RENDERBUFFER, newFB->depthStencilAttachment);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.width, size.height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, newFB->depthStencilAttachment);
        //glNamedFramebufferRenderbuffer(newFB->framebuffer, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, newFB->depthStencilAttachment);
    }

    //glNamedFramebufferDrawBuffers(newFB->framebuffer, attachmentIdx_v.size(), attachmentIdx_v.data());
    glDrawBuffers(attachmentIdx_v.size(), attachmentIdx_v.data());

    //GLenum status = glCheckNamedFramebufferStatus(newFB->framebuffer, GL_FRAMEBUFFER);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        assert(false);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, currFramebuffer);
    
    return static_cast<bhResourceID>(newFB);
}

void bhDeviceGL::DestroyFramebuffer(bhResourceID fbId)
{
    bhFramebufferGL* fb = static_cast<bhFramebufferGL*>(fbId);
    glDeleteRenderbuffers(1, &(fb->depthStencilAttachment));
    fb->depthStencilAttachment = GL_ZERO;

    glDeleteTextures((GLsizei)fb->colorAttachments_v.size(), fb->colorAttachments_v.data());
    fb->colorAttachments_v.clear();

    glDeleteFramebuffers(1, &(fb->framebuffer));
    fb->framebuffer = GL_ZERO;
}

void bhDeviceGL::UseFramebuffer(bhResourceID fbId)
{
    assert(fbId);
    bhFramebufferGL* fb = static_cast<bhFramebufferGL*>(fbId);
    glBindFramebuffer(GL_FRAMEBUFFER, fb->framebuffer);
    glViewport(0, 0, fb->size.width, fb->size.height);
}

void* bhDeviceGL::GetFramebufferColorAttachment(bhResourceID fbId, size_t attachmentIdx)
{
    bhFramebufferGL* fb = static_cast<bhFramebufferGL*>(fbId);
    assert(attachmentIdx < fb->colorAttachments_v.size());
    return reinterpret_cast<void*>(fb->colorAttachments_v[attachmentIdx]);
}

bhSize2Di bhDeviceGL::GetFramebufferSize(bhResourceID fbId)
{
    bhFramebufferGL* fb = static_cast<bhFramebufferGL*>(fbId);
    return fb->size;
}
