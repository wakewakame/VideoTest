/*
  ==============================================================================

    OpenGL.cpp
    Created: 11 Oct 2018 3:25:50pm
    Author:  0214t

  ==============================================================================
*/

#include "GLGraphics.h"

GLGraphics::GLGraphics() {}

GLGraphics::~GLGraphics() {}

void GLGraphics::initialise(OpenGLContext &openGLContext) {
	openGLContextPtr = &openGLContext;
	shape.reset(new Shape(*openGLContextPtr));
}

void GLGraphics::line(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
	if (strokeColor.a != 0.0) {
		shape->beginShape(Shape::LINES);
		if (currentShader != nullptr) shape->setShader(*currentShader);
		shape->setColor(strokeColor.r, strokeColor.g, strokeColor.b, strokeColor.a);
		shape->vertex(x1, y1);
		shape->vertex(x2, y2);
		shape->endShape();
		shape->draw();
	}
}

void GLGraphics::rect(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
	if (x1 > x2) {
		GLfloat tmp_x = x1;
		x1 = x2; x2 = tmp_x;
	}
	if (y1 > y2) {
		GLfloat tmp_y = y1;
		y1 = y2; y2 = tmp_y;
	}
	if (strokeColor.a != 0.0) {
		shape->beginShape(Shape::LINE_LOOP);
		if (currentShader != nullptr) shape->setShader(*currentShader);
		shape->setColor(strokeColor.r, strokeColor.g, strokeColor.b, strokeColor.a);
		shape->vertex(x1, y1);
		shape->vertex(x2, y1);
		shape->vertex(x2, y2);
		shape->vertex(x1, y2);
		shape->endShape();
		shape->draw();
	}
	if (x1 != x2 && y1 != y2 && fillColor.a != 0.0) {
		shape->beginShape(Shape::TRIANGLE_STRIP, true);
		if (currentShader != nullptr) shape->setShader(*currentShader);
		shape->setColor(fillColor.r, fillColor.g, fillColor.b, fillColor.a);
		shape->vertex(x1, y1, 0.0, 0.0); shape->index(0);
		shape->vertex(x2, y1, 1.0, 0.0); shape->index(3);
		shape->vertex(x2, y2, 1.0, 1.0); shape->index(1);
		shape->vertex(x1, y2, 0.0, 1.0); shape->index(2);
		shape->endShape();
		shape->draw();
	}
}