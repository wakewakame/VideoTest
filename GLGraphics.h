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
protected:
	OpenGLContext *openGLContextPtr;

private:
	std::unique_ptr<Shape> shape;
	Shape::Color fillColor, strokeColor;
	Shader *currentShader = nullptr;

public:
	GLGraphics();
	virtual ~GLGraphics();

	void initialise(OpenGLContext &openGLContext);

	virtual void setup() = 0;
	virtual void draw() = 0;

	inline void fill(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
		fillColor.r = r;
		fillColor.g = g;
		fillColor.b = b;
		fillColor.a = a;
	}
	inline void fill(GLfloat r, GLfloat g, GLfloat b) {
		fill(r, g, b, 1.0);
	}
	inline void fill(GLfloat gray, GLfloat a) {
		fill(gray, gray, gray, a);
	}
	inline void fill(GLfloat gray) {
		fill(gray, gray, gray, 1.0);
	}
	inline void noFill() {
		fill(0.0, 0.0, 0.0, 0.0);
	}
	inline void stroke(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
		strokeColor.r = r;
		strokeColor.g = g;
		strokeColor.b = b;
		strokeColor.a = a;
	}
	inline void stroke(GLfloat r, GLfloat g, GLfloat b) {
		stroke(r, g, b, 1.0);
	}
	inline void stroke(GLfloat gray, GLfloat a) {
		stroke(gray, gray, gray, a);
	}
	inline void stroke(GLfloat gray) {
		stroke(gray, gray, gray, 1.0);
	}
	inline void noStroke() {
		stroke(0.0, 0.0, 0.0, 0.0);
	}
	inline void setShader(Shader &shader) {
		currentShader = &shader;
	}
	inline void setNoShader() {
		currentShader = nullptr;
	}
	void line(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
	void rect(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
};