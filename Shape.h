/*
  ==============================================================================

    Shape.h
    Created: 11 Oct 2018 4:09:20pm
    Author:  0214t

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Shader.h"

class Shape {
public:
	struct Vertex {
		GLfloat x, y, z, w;
		GLfloat u, v;
		GLfloat r, g, b, a;
	};
	enum VertexType {
		POINTS = GL_POINTS,
		LINES = GL_LINES,
		LINE_LOOP = GL_LINE_LOOP,
		LINE_STRIP = GL_LINE_STRIP,
		TRIANGLES = GL_TRIANGLES,
		TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
		TRIANGLE_FAN = GL_TRIANGLE_FAN,
		QUADS = GL_QUADS,
		QUAD_STRIP = GL_QUAD_STRIP,
		POLYGON = GL_POLYGON
	};
	struct Color {
		GLfloat r = 0.0, g = 0.0, b = 0.0, a = 1.0;
	};
private:
	// current OpenGLContext
	OpenGLContext &openGLContextRef;
	// opengl buffer
	GLuint vertexBuffer, indexBuffer;
	// vertices
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	VertexType currentVertexType;
	bool useIndexFlag;
	// current color
	Color color;
	// current shader
	std::unique_ptr<Shader> defaultShader;
	Shader *currentShader;
	void attributesEnable();
	void attributesDisable();

public:
	Shape(OpenGLContext &openGLContext);
	virtual ~Shape();
	// if you want to use Shape::index, you need set useIndex true.
	// otherwise vertices index is added automatic.
	void beginShape(VertexType vertexType, bool useIndex = false);
	void endShape();
	void draw();
	inline void vertex(GLfloat x, GLfloat y, GLfloat z, GLfloat u, GLfloat v) {
		if (!useIndexFlag) index((GLuint)indices.size());
		vertices.push_back({
			x, y, z, 1.0,
			u, v,
			color.r, color.g, color.b, color.a
		});
	}
	inline void vertex(GLfloat x, GLfloat y, GLfloat u, GLfloat v) {
		vertex(x, y, 0.0, u, v);
	}
	inline void vertex(GLfloat v) {
		vertex(0.0, 0.0, 0.0, 0.0, v);
	}
	inline void vertex(GLfloat x, GLfloat y, GLfloat z) {
		vertex(x, y, z, 0.0, 0.0);
	}
	inline void vertex(GLfloat x, GLfloat y) {
		vertex(x, y, 0.0, 0.0, 0.0);
	}
	inline void index(GLuint n) {
		indices.push_back(n);
	}
	inline void setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
		color.r = r;
		color.g = g;
		color.b = b;
		color.a = a;
	}
	inline void setColor(GLfloat r, GLfloat g, GLfloat b) {
		setColor(r, g, b, 1.0);
	}
	inline void setColor(GLfloat gray, GLfloat a) {
		setColor(gray, gray, gray, a);
	}
	inline void setColor(GLfloat gray) {
		setColor(gray, gray, gray, 1.0);
	}
	inline void setNoColor() { 
		setColor(0.0, 0.0, 0.0, 0.0); 
	}
	inline void setShader(Shader &shader){
		currentShader = &shader;
	}
};