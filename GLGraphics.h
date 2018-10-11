/*
  ==============================================================================

    Shape.h
    Created: 10 Oct 2018 9:28:27pm
    Author:  0214t

  ==============================================================================
*/

#pragma once

#include <vector>
#include <string>
#include "../JuceLibraryCode/JuceHeader.h"
#include "Shape.h"

class Shape;

class GLGraphics {
private:
	OpenGLContext &openGLContext;
	std::unique_ptr<Shape> fillShape;
	std::unique_ptr<Shape> stcokeShape;

public:
	GLGraphics(OpenGLContext &openGLContext);
	virtual ~GLGraphics();
};