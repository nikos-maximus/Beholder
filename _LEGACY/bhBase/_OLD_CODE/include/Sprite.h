#ifndef SPRITE_H
#define SPRITE_H

#include "Rect.h"
#include "Color.h"

////////////////////////////////////////////////////////////////////////////////

namespace GTCore
{
	class Sprite
	{
	public:

		Sprite(void);
		~Sprite(void);
		//inline void EnableColorize(bool enable = true){colorize = enable;}
		virtual void Render();

		inline void SetPosition(const GTMath::vec2<int>& iPos)
		{
			rect.x = iPos.x;
			rect.y = iPos.y;
		}

		inline void SetPosition(int x,int y)
		{
			rect.x = x;
			rect.y = y;
		}

		inline void SetSize(const GTMath::vec2<int> iSz)
		{
			rect.w = iSz.x;
			rect.h = iSz.y;
		}

		inline void SetSize(int w,int h)
		{
			rect.w = w;
			rect.h = h;
		}

		inline GTMath::vec2<int> GetPosition()
		{
			return GTMath::vec2<int>(rect.x,rect.y);
		}

		//inline void SetRegPoint(const vec2f& rp){regPoint = rp;}
		//inline void SetRegPoint(const GTFloat x,const GTFloat y){regPoint = vec2f(x,y);}
		//inline void Rotate(const GTFloat degrees){rotation = degrees;}

		inline void AddChild(Sprite* newChild)
		{
			newChild->parent = this;
			children.push_back(newChild);
		}

		void RemoveChild(Sprite* child);
		void ClearChildren();
		void BringToFront(Sprite* child);
		void SendToBack(Sprite* child);
		
		//inline bool IsLeaf()
		//{
		//	return children.empty();
		//}

		//Rectf ImageToTextureCoords(Recti refGeom);
		
		inline void SetVisible(bool enable = true)
		{
			visible = enable;
		}

		inline bool IsVisible()
		{
			return visible;
		}

		//inline void SetColor(Color ioc){color = ioc;}
		//inline void EnableTexture(bool enable = true){useTexture = enable;}
		
		//virtual bool LoadTexture(const std::string& name);
		
		//inline Frame* GetCurrFrame(){return animations[currAnimation].GetCurrFrame();}
		virtual void Step();
		void StepChildren();

		inline void SetAnimation(const unsigned short animIndex)
		{
			assert(animIndex < animations.size());
			currAnimation = animations[animIndex];
			framesSkipped = 0;
		}
		
		inline void Play()
		{
			currAnimation->Play();
			//if(currAnimation->currFrame == (currAnimation->frames.size() - 1))
			//{
			//	currAnimation->currFrame = 0;
			//}
		}

		inline void Pause()
		{
			currAnimation->Pause();
		}

		inline void Stop()
		{
			currAnimation->Stop();
		}

	protected:

		////////////////////////////////////////////////////////////

		class AnimationStrip
		{
		public:

			AnimationStrip();
			~AnimationStrip();
			inline void Advance()
			{
				currFrame = (++currFrame) % frames.size();
			}

		protected:

			struct Frame
			{
				Frame();
				Frame(const Sint16 x,const Sint16 y,const Uint16 w,const Uint16 h)
				{
					
				}
				Frame(const SDL_Rect& irect)
				{
					rect = irect;
				}
				SDL_Rect rect;
				unsigned short frameSkip;
			};

			bool looping;
			bool playing;
			bool finished;

		private:

			Frame* GetCurrFrame(){return frames[currFrame];}
			//inline void Reset(){currFrame = 0;}
			unsigned int nextAnimationStrip;
			unsigned short currFrame;
			std::vector<Frame*> frames;
		};

		////////////////////////////////////////////////////////////

		inline bool Contains(const int x,const int y)
		{
			return ((rectetry.x <= x) && (x < (rectetry.x + rectetry.w)) && (rectetry.y <= y) && (y < (rectetry.y + rectetry.h)));
		}

		void RenderChildren();
		Sprite* GetChild(const int x,const int y);
		void Cleanup();
		void ClearAnimations();

		Sprite* parent;
		std::list<Sprite*> children;

		//Color color;
		
		//Recti rectetry;
		//vec2f regPoint;

		//Texture* texture;
		//Rectf* textureGeometry;
		//GTFloat rotation;

		bool visible;
		bool colorize;
		bool horizontalFlip,verticalFlip;
		//bool useTexture;

		AnimationStrip* currAnimation;
		std::vector<AnimationStrip*> animations;
		//unsigned int lastUpdate;
		bool persistent;
		unsigned short framesSkipped;

	private:

		Rect rect;
	};
};

////////////////////////////////////////////////////////////////////////////////

#endif
