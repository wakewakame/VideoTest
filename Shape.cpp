/*
  ==============================================================================

    Shape.cpp
    Created: 11 Oct 2018 4:09:27pm
    Author:  0214t

  ==============================================================================
*/

#include "Shape.h"

Shape::Shape(OpenGLContext &openGLContext) : openGLContext(openGLContext) {
	// generate opengl buffer
	vertexBuffer = 0;
	indexBuffer = 0;
	openGLContext.extensions.glGenBuffers(1, &vertexBuffer);
	openGLContext.extensions.glGenBuffers(1, &indexBuffer);
	// new default shader
	defaultShader.reset(new Shader(openGLContext));
}

Shape::~Shape() {
	// delete opengl buffer
	openGLContext.extensions.glDeleteBuffers(1, &vertexBuffer);
	openGLContext.extensions.glDeleteBuffers(1, &indexBuffer);
}

// if you want to use Shape::index, you need set useIndex true.
// otherwise vertices index is added automatic.
void Shape::beginShape(VertexType vertexType, bool useIndex) {
	// reset arrays
	std::vector<Vertex> initVertices;
	vertices.swap(initVertices);
	std::vector<GLuint> initIndices;
	indices.swap(initIndices);
	// set vertex type
	currentVertexType = vertexType;
	useIndexFlag = useIndex;
	// reset current color
	color.r = color.g = color.b = 0.0;
	color.a = 1.0;
	// reset current shader
	currentShader = defaultShader.get();
}

void Shape::endShape() {
	openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	openGLContext.extensions.glBufferData(
		GL_ARRAY_BUFFER,
		vertices.size() * sizeof(Vertex),
		vertices.data(),
		GL_DYNAMIC_DRAW
	);
	openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	openGLContext.extensions.glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		indices.size() * sizeof(GLuint),
		indices.data(),
		GL_DYNAMIC_DRAW
	);
}

void Shape::draw() {
	openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	attributesEnable();
	glDrawElements(currentVertexType, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
	attributesDisable();
}

void Shape::attributesEnable() {
	OpenGLShaderProgram::Attribute *ptr = (OpenGLShaderProgram::Attribute*)nullptr;

	// position
	ptr = currentShader->getAttribute("position");
	if ((ptr != nullptr) && (currentShader->getAttributeType("position") == "vec4"))
	{
		openGLContext.extensions.glVertexAttribPointer(ptr->attributeID, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(GLfloat) * 0));
		openGLContext.extensions.glEnableVertexAttribArray(ptr->attributeID);
	}

	// uv
	ptr = currentShader->getAttribute("uv");
	if ((ptr != nullptr) && (currentShader->getAttributeType("uv") == "vec2"))
	{
		openGLContext.extensions.glVertexAttribPointer(ptr->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(GLfloat) * 4));
		openGLContext.extensions.glEnableVertexAttribArray(ptr->attributeID);
	}

	// color
	ptr = currentShader->getAttribute("color");
	if ((ptr != nullptr) && (currentShader->getAttributeType("color") == "vec4"))
	{
		openGLContext.extensions.glVertexAttribPointer(ptr->attributeID, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(GLfloat) * 6));
		openGLContext.extensions.glEnableVertexAttribArray(ptr->attributeID);
	}
}

void Shape::attributesDisable() {
	OpenGLShaderProgram::Attribute *ptr = (OpenGLShaderProgram::Attribute*)nullptr;

	// position
	ptr = currentShader->getAttribute("position");
	if ((ptr != nullptr) && (currentShader->getAttributeType("position") == "vec4"))
	{
		openGLContext.extensions.glDisableVertexAttribArray(ptr->attributeID);
	}

	// uv
	ptr = currentShader->getAttribute("uv");
	if ((ptr != nullptr) && (currentShader->getAttributeType("uv") == "vec2"))
	{
		openGLContext.extensions.glDisableVertexAttribArray(ptr->attributeID);
	}

	// color
	ptr = currentShader->getAttribute("color");
	if ((ptr != nullptr) && (currentShader->getAttributeType("color") == "vec4"))
	{
		openGLContext.extensions.glDisableVertexAttribArray(ptr->attributeID);
	}
}