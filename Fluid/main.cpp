//
//  main.cpp
//  Fluid
//
//  Created by Hyun Joon Shin on 2021/05/31.
//

#include <iostream>
#include <JGL/JGL_Window.hpp>
#include <JGL/JGL.hpp>
#include "GLTools.hpp"
#include "TexView.hpp"
#include <algorithm>

using namespace glm;

TexView* view;
const int GRID_W = 256;
const int GRID_H = 256;

float* density = nullptr;
float* densityTemp = nullptr;
float* p = nullptr;
vec2* velocity = nullptr;
vec2* velocityTemp = nullptr;
ivec2 mousePt;
ivec2 lastPt;
bool pressed = false;

float H = 1;
float kappa = 10;

#define IX(X,Y) ((std::min(GRID_H - 1, (std::max(0,Y)))*GRID_W) + std::min(GRID_W - 1, (std::max(0,X))))

template<typename T> T sample( T* buf, int w, int h, const vec2& pp ) {
    vec2 p = max(vec2(0,0),min(vec2(w-1.000001, h-1.000001),pp));
    int x = int(floor(p.x));
    int y = int(floor(p.y));
    float s = p.x-x;
    float t = p.y-y;
    T v1 = glm::mix( buf[IX(x,y)], buf[IX(x+1,y)], s );
    T v2 = glm::mix( buf[IX(x,y+1)], buf[IX(x+1,y+1)], s);
	return glm::mix(v1, v2, t);
}

template<typename T>
void diffuse( float k, T* D, float dt ) {
    for( auto iter = 0; iter < 10; iter++)
        for(auto y=0; y<GRID_H; y++) for( auto x=0; x<GRID_W; x++)
            D[IX(x,y)] //D^(n+1)
                = (D[IX(x,y)] + k*dt/H/H * ( D[IX(x-1,y)] + D[IX(x+1,y)] + D[IX(x,y-1)] + D[IX(x,y+1)])) / (1+4*k*dt/H/H);
}

template<typename T>
void advection( T* D, T* D2, glm::vec2* v, float dt ) {
    for( auto y=0; y<GRID_H; y++)   for( auto x=0; x<GRID_W; x++)
        D2[IX(x,y)] = sample(D, GRID_W, GRID_H, glm::vec2(x,y)-dt*v[IX(x,y)]);
    
    for( auto i=0; i<GRID_W*GRID_H; i++)
        D[i] = D2[i];
}

void convserveMass( glm::vec2* v, glm::vec2* v2, float* p ) {
    for( auto iter = 0; iter < 10; iter++)
        for(auto y=0; y<GRID_H; y++) for( auto x=0; x<GRID_W; x++)
            p[IX(x,y)] = (p[IX(x-1,y)] + p[IX(x+1,y)] + p[IX(x,y-1)] + p[IX(x,y+1)] + (v[IX(x+1,y)].x-v[IX(x-1,y)].x + v[IX(x,y+1)].y-v[IX(x,y-1)].y)*H)/-4;
    
    for(auto y=0; y<GRID_H; y++) for( auto x=0; x<GRID_W; x++){
        glm::vec2 gradientP = glm::vec2( (p[IX(x+1,y)] - p[IX( x-1, y)]) / (2*H), (p[IX(x,y+1)] - p[IX( x, y-1)]) / (2*H));
        v2[IX(x,y)] = v[IX(x,y)] - gradientP;
    }
    
    for( auto i=0; i<GRID_W*GRID_H; i++)
        v[i] = v2[i];
}

void init() {
	if( !density ) {
		density = new float[GRID_W*GRID_H];
		densityTemp = new float[GRID_W*GRID_H];
		velocity = new glm::vec2[GRID_W*GRID_H];
		velocityTemp = new glm::vec2[GRID_W*GRID_H];
		p = new float[GRID_W*GRID_H];
	}
	for( int i=0; i<GRID_H*GRID_W; i++ ) {
		density[i] = 0;
		velocity[i] = glm::vec2(0,0);
		p[i] = 1;
	}
}



void frame(float dt) {
	if( pressed )
		density[IX(mousePt.x,mousePt.y)]+=5;
    
	diffuse( kappa, density, dt );
	advection( density, densityTemp, velocity, dt );
	diffuse( 100, velocity, dt );
	advection( velocity, velocityTemp, velocity, dt );
	convserveMass( velocity, velocityTemp, p );
}

void push( float x, float y ) {
	mousePt = glm::ivec2( x*GRID_W, y*GRID_H );
	pressed = true;
}

void release() {
	pressed =false;
}

void move( float x, float y ) {
	lastPt = mousePt;
	mousePt = glm::ivec2( x*GRID_W, y*GRID_H );
	velocity[IX(lastPt.x,lastPt.y)] = vec2(mousePt-lastPt)*1000.f;
}

void drag( float x, float y ) {
	mousePt = glm::ivec2( x*GRID_W, y*GRID_H );
}

void update(Tex& tex) {
	tex.create(GRID_W,GRID_H,GL_RED,GL_FLOAT, density );
}

int main(int argc, const char * argv[]) {
	
	init();

	JGL::Window* window = new JGL::Window(1024,1024,"Fluid");
	view = new TexView(0,0,1024,1024);
	view->initFunction = init;
	view->updateFunction = update;
	view->frameFunction = frame;
	view->dragFunction = drag;
	view->moveFunction = move;
	view->pushFunction = push;
	view->releaseFunction = release;
	window->end();
	window->show();

	JGL::_JGL::run();
	return 0;
}
