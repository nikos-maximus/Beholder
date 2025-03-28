#include "Sprite.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

namespace GTCore
{
	Sprite::Frame::Frame()
	:frameSkip(0)
	{
	}

	Sprite::Frame::Frame(const Rectf& irect)
	:frameSkip(0)
	,rect(irect)
	{
	}

	////////////////////////////////////////////////////////////////////////////////

	Sprite::Animation::Animation()
	:looping(true)
	,playing(true)
	,finished(false)
	,currFrame(0)
	{
	}

	Sprite::Animation::~Animation()
	{
		for(vector<Frame*>::iterator fi = frames.begin();fi != frames.end();fi++)
		{
			delete (*fi);
		}
		frames.clear();
	}

	void Sprite::Animation::Advance()
	{
		if(playing)
		{
			if(!looping && (currFrame == (frames.size() - 1)))
			{
				playing = false;
				finished = true;
				return;
			}
			//else
			//{
			//}
			currFrame = (++currFrame) % frames.size();
		}
	}

	////////////////////////////////////////////////////////////

	Sprite::Sprite(void)
	:rotation((GTFloat)0.0)
	,colorize(false)
	,horizontalFlip(false)
	,verticalFlip(false)
	,visible(true)
	,texture(0)
	,textureGeometry(0)
	,useTexture(false)
	,parent(0)
	,currAnimation(0)
	,persistent(true)
	,framesSkipped(0)
	{
		Animation* nullAnim = new Animation();
		animations.push_back(nullAnim);
		currAnimation = nullAnim;
	 
		Frame* staticFrame = new Frame();
		staticFrame->frameSkip = 0;
		nullAnim->frames.push_back(staticFrame);
		textureGeometry = &(staticFrame->rect);
	}

	Sprite::~Sprite(void)
	{
		Cleanup();
	}

	void Sprite::ClearAnimations()
	{
		for(vector<Animation*>::iterator ai = animations.begin();ai != animations.end();ai++)
		{
			delete (*ai);
		}
		animations.clear();
	}

	void Sprite::Cleanup()
	{
		ClearChildren();
		ClearAnimations();
		textureGeometry = 0;
		texture->Release();
	}

	void Sprite::Render()
	{
		if(useTexture)
		{
			texture->Bind();
		}
		//glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef((GLfloat)(rectetry.x) - regPoint.x,(GLfloat)(rectetry.y) - regPoint.y,0.0f);
		glRotatef((GLfloat)rotation,0.0f,0.0f,1.0f);

		glColor4f(1.0,1.0,1.0,1.0);
		if(colorize)
		{
			glColor4fv(&(color.r));
		}
		
		textureGeometry = &(currAnimation->GetCurrFrame()->rect);
		glBegin(GL_QUADS);
			glTexCoord2f(textureGeometry->x,textureGeometry->y);
			glVertex2i(0,0);
			glTexCoord2f(textureGeometry->w,textureGeometry->y);
			glVertex2i(rectetry.w,0);
			glTexCoord2f(textureGeometry->w,textureGeometry->h);
			glVertex2i(rectetry.w,rectetry.h);
			glTexCoord2f(textureGeometry->x,textureGeometry->h);
			glVertex2i(0,rectetry.h);
		glEnd();

		RenderChildren();
		glPopMatrix();
	}

	bool Sprite::LoadTexture(const string& name)
	{
		texture = Texture::LoadTexture(name);
		texture ? (useTexture = true) : (useTexture = false);
		return (texture != 0);
	}

	void Sprite::RemoveChild(Sprite* child)
	{
		for(list<Sprite*>::const_iterator ci = children.begin();ci != children.end();ci++)
		{
			if(child == (*ci))
			{
				children.erase(ci);
				break;
			}
		}
	}

	void Sprite::StepChildren()
	{
		for(list<Sprite*>::const_iterator ci = children.begin();ci != children.end();ci++)
		{
			(*ci)->Step();
		}
	}

	void Sprite::RenderChildren()
	{
		for(list<Sprite*>::const_iterator ci = children.begin();ci != children.end();ci++)
		{
			if((*ci)->visible)
			{
				(*ci)->Render();
			}
		}
	}

	void Sprite::ClearChildren()
	{
		for(list<Sprite*>::iterator ci = children.begin();ci != children.end();ci++)
		{
			delete (*ci);
		}
		children.clear();
	}

	void Sprite::BringToFront(Sprite* child)
	{
		for(list<Sprite*>::const_iterator ci = children.begin();ci != children.end();ci++)
		{
			if(child == (*ci))
			{
				children.erase(ci);
				break;
			}
		}
		children.push_back(child);
	}

	void Sprite::SendToBack(Sprite* child)
	{
		for(list<Sprite*>::const_iterator ci = children.begin();ci != children.end();ci++)
		{
			if(child == (*ci))
			{
				children.erase(ci);
				break;
			}
		}
		children.push_front(child);
	}

	Rectf Sprite::ImageToTextureCoords(Recti refGeom)
	{
		Rectf texGeom;
		texGeom.x = (GTFloat)refGeom.x / (GTFloat)texture->GetWidth();
		texGeom.y = (GTFloat)refGeom.y / (GTFloat)texture->GetHeight();
		texGeom.w = ((GTFloat)refGeom.x + (GTFloat)refGeom.w) / (GTFloat)texture->GetWidth();
		texGeom.h = ((GTFloat)refGeom.y + (GTFloat)refGeom.h) / (GTFloat)texture->GetHeight();
		return texGeom;
	}

	Sprite* Sprite::GetChild(const int x,const int y)
	{
		for(list<Sprite*>::reverse_iterator ci = children.rbegin();ci != children.rend();ci++)
		{
			if((*ci)->Contains(x,y))
			{
				return (*ci);
			}
		}
		return 0;
	}

	void Sprite::Step()
	{
		if(framesSkipped < currAnimation->GetCurrFrame()->frameSkip)
		{
			++framesSkipped;
			return;
		}
		else
		{
			framesSkipped = 0;
			currAnimation->Advance();
			if(!persistent)
			{
				visible = false;
			}
		}
		StepChildren();
	}
};

////////////////////////////////////////////////////////////////////////////////
