/*
  ==============================================================================

    OpenGL.cpp
    Created: 11 Oct 2018 3:25:50pm
    Author:  0214t

  ==============================================================================
*/

#include "GLGraphics.h"

GLGraphics::GLGraphics(OpenGLContext &openGLContext) : openGLContext(openGLContext) {
	fillShape.reset(new Shape(openGLContext));
	strokeShape.reset(new Shape(openGLContext));
}

GLGraphics::~GLGraphics() {

}