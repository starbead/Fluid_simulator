//
//  TexView.hpp
//  Fluid
//
//  Created by Hyun Joon Shin on 2021/05/12.
//

#ifndef TexView_h
#define TexView_h
#include <JGL/JGL_Widget.hpp>
#include <functional>

struct Tex {
	GLuint tex = 0;
	int w=0, h=0;
	GLuint f=GL_RGB, t=GL_UNSIGNED_BYTE;
	void create( int ww, int hh, GLuint ff, GLuint tt, void* data=nullptr ) {
		if( ww!=w || hh!=h || ff!=f || tt!=t ) {
			if( tex>0 ) glDeleteTextures(1,&tex);
			tex = 0;
		}
		w = ww;
		h = hh;
		f = ff;
		t = tt;
		if( tex<1 ) {
			glGenTextures(1, &tex);
			glBindTexture(GL_TEXTURE_2D, tex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, f, t, data);
		}
		else
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, f, t, data);
	}
	void sub( void* data ) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, f, t, data);
	}
	void bind( GLuint prog, const std::string& uniName, int idx=0 ) {
		glActiveTexture( GL_TEXTURE0+idx );
		glBindTexture( GL_TEXTURE_2D, tex );
		setUniform( prog, uniName.c_str(), idx);
	}
};



struct TexView: JGL::Widget {
	float lastT = 0;
	bool animating = true;
	GLuint prog=0, vert=0, frag=0;

	Tex texture;
	
	std::function<void()> initFunction=[](){};
	std::function<void(float)> frameFunction = [](float){};
	std::function<void(int)> keyFunction = [](int){};
	std::function<void(Tex&)> updateFunction = [](Tex&){};
	std::function<void(float, float)> pushFunction = [](float,float){};
	std::function<void(float, float)> dragFunction = [](float,float){};
	std::function<void(float, float)> moveFunction = [](float,float){};
	std::function<void()> releaseFunction = [](){};


	TexView(float x, float y, float w, float h, const std::string& name="")
	: JGL::Widget(x,y,w,h,name) {}
	
	bool handle(int e) override {
		glm::vec2 p = JGL::_JGL::eventPt();
		p = glm::vec2( p.x/w(), p.y/h() );
		if( e == JGL::EVENT_PUSH ) {
			pushFunction( p.x, p.y );
			return true;
		}
		else if( e == JGL::EVENT_MOVE ) {
			moveFunction( p.x, p.y );
			return true;
		}
		else if( e == JGL::EVENT_DRAG ) {
			dragFunction( p.x, p.y );
			return true;
		}
		else if( e == JGL::EVENT_RELEASE ) {
			releaseFunction();
			return true;
		}
		else if( e == JGL::EVENT_KEYDOWN ) {
			keyFunction(JGL::_JGL::eventKey());
/*			if( JGL::_JGL::eventKey() == ' ' ) {
				animating = !animating;
				if( animating ) {
					lastT = glfwGetTime();
					animate();
				}
				return true;
			}
			else*/ if( JGL::_JGL::eventKey() == '0' ) {
//				animating = false;
				initFunction();
				redraw();
				return true;
			}
		}
		return JGL::Widget::handle( e );
	}
	virtual void drawGL() override {
		glClearColor(0,0,0,0);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		if( prog<1 ) {
			std::tie(prog,vert,frag) = loadProgram( "shader.vert", "shader.frag");
		}
		glUseProgram( prog );
		updateFunction(texture);
		if( texture.tex>0 )
			texture.bind(prog,"tex");
		
		drawQuad();
	}
	virtual void drawContents(NVGcontext* vg, const glm::rect&r, int a ) override {
		if( animating ) {
			float t = glfwGetTime();
			frameFunction(t-lastT);
			animate();
			lastT = t;
		}
	}
};

#endif /* AnimView_h */
