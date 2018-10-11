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

class GLGraphics {
private:
	OpenGLContext &openGLContext;
	std::unique_ptr<Shape> fillShape;
	std::unique_ptr<Shape> strokeShape;
	Shader *currentShader = nullptr;

public:
	GLGraphics(OpenGLContext &openGLContext);
	virtual ~GLGraphics();

	/*
	inline void fill(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
		fillShape->setColor(r, g, b, a);
	}
	inline void fill(GLfloat r, GLfloat g, GLfloat b) {
		fillShape->setColor(r, g, b);
	}
	inline void fill(GLfloat gray, GLfloat a) {
		fillShape->setColor(gray, a);
	}
	inline void fill(GLfloat gray) {
		fillShape->setColor(gray);
	}
	inline void noFill() {
		fillShape->setNoColor();
	}
	inline void stroke(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
		strokeShape->setColor(r, g, b, a);
	}
	inline void stroke(GLfloat r, GLfloat g, GLfloat b) {
		strokeShape->setColor(r, g, b);
	}
	inline void stroke(GLfloat gray, GLfloat a) {
		strokeShape->setColor(gray, a);
	}
	inline void stroke(GLfloat gray) {
		strokeShape->setColor(gray);
	}
	inline void noStroke() {
		strokeShape->setNoColor();
	}
	void filter(Shader &shader) {
		currentShader = &shader;
	}
	*/
};