/*
  ==============================================================================

    Shader.cpp
    Created: 11 Oct 2018 4:09:44pm
    Author:  0214t

  ==============================================================================
*/

#include "Shader.h"

/*
==============================================================================

Shader.h
Created: 11 Oct 2018 4:09:36pm
Author:  0214t

==============================================================================
*/

#pragma once

const std::string Shader::defaultVertexShader =
R"(
attribute vec4 position;
attribute vec2 uv;
attribute vec4 color;
varying vec2 vUv;
varying vec4 vColor;
void main()
{
    vUv = uv;
    vColor = color;
    gl_Position = position;
};
)";

const std::string Shader::defaultFragmentShader =
R"(
varying vec2 vUv;
varying vec4 vColor;
void main()
{
    gl_FragColor = vColor;
};
)";

Shader::Shader(OpenGLContext &openGLContext) : openGLContext(openGLContext) {
	loadDefaultShader();
}

Shader::~Shader() {}

void Shader::loadShader(std::string vertexShader, std::string fragmentShader) {
	// compile
	std::unique_ptr<OpenGLShaderProgram> newShader(new OpenGLShaderProgram(openGLContext));
	if (
		newShader->addVertexShader(vertexShader) &&
		newShader->addFragmentShader(fragmentShader) &&
		newShader->link()
	)
	{
		// reset attributes and uniforms
		findAttributes(vertexShader);
		findUniforms(vertexShader + "\n" + fragmentShader);
		// reset shader
		shader.reset(newShader.release());
		errorMessage = "";
	}
	else
	{
		errorMessage = std::string(newShader->getLastError().toUTF8());
	}
}

void Shader::loadShader(std::string fragmentShader) {
	loadShader(defaultVertexShader, fragmentShader);
}

void Shader::loadDefaultShader() {
	loadShader(defaultVertexShader, defaultFragmentShader);
}

void Shader::use() {
	shader->use();
}

std::vector<std::string> Shader::getAttributeNames() {
	std::vector<std::string> ret;
	for (
		std::map<std::string, std::string>::iterator it = attributesType.begin();
		it != attributesType.end();
		it++
		) {
		ret.push_back(it->first);
	}
	return ret;
}

std::vector<std::string> Shader::getUniformNames() {
	std::vector<std::string> ret;
	for (
		std::map<std::string, std::string>::iterator it = uniformsType.begin();
		it != uniformsType.end();
		it++
		) {
		ret.push_back(it->first);
	}
	return ret;
}

void Shader::findAttributes(std::string src) {
	attributesType.clear();
	attributes.clear();
	// delete comments
	const std::regex comments("/\\*[\\s\\S]*?\\*/|//.*");
	src = std::regex_replace(src, comments, "");
	// search all attributes
	const std::regex target("attribute[^;]+;");
	std::vector<std::string> dst;
	std::sregex_iterator end, ite{ src.begin(), src.end(), target };
	for (; ite != end; ++ite) {
		dst.push_back(ite->str());
	}
	// get type and name
	const std::regex word("[\\w_]+");
	for (std::string s : dst) {
		std::sregex_iterator end, ite{ s.begin(), s.end(), word };
		std::string type, name;
		uint16_t count = 0;
		for (; ite != end; ++ite) {
			if (count == 1) type = ite->str();
			if (count == 2) name = ite->str();
			count++;
		}
		if (
			count == 3 || 
			openGLContext.extensions.glGetAttribLocation(
				shader->getProgramID(),
				name.c_str()
			) >= 0
		) {
			attributesType[name] = type;
			attributes[name].reset(new OpenGLShaderProgram::Attribute(*shader, name.c_str()));
		}
	}
}

void Shader::findUniforms(std::string src) {
	uniformsType.clear();
	uniforms.clear();
	// delete comments
	const std::regex comments("/\\*[\\s\\S]*?\\*/|//.*");
	src = std::regex_replace(src, comments, "");
	// search all uniforms
	const std::regex target("uniform[^;]+;");
	std::vector<std::string> dst;
	std::sregex_iterator end, ite{ src.begin(), src.end(), target };
	for (; ite != end; ++ite) {
		dst.push_back(ite->str());
	}
	// get type and name
	const std::regex word("[\\w_]+");
	for (std::string s : dst) {
		std::sregex_iterator end, ite{ s.begin(), s.end(), word };
		std::string type, name;
		uint16_t count = 0;
		for (; ite != end; ++ite) {
			if (count == 1) type = ite->str();
			if (count == 2) name = ite->str();
			count++;
		}
		if (
			count == 3 ||
			openGLContext.extensions.glGetUniformLocation(
				shader->getProgramID(),
				name.c_str()
			) >= 0
			) {
			uniformsType[name] = type;
			uniforms[name].reset(new OpenGLShaderProgram::Uniform(*shader, name.c_str()));
		}
	}
}