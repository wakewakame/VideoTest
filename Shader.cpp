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

Shader::Shader(OpenGLContext &openGLContextRef) : openGLContextRef(openGLContextRef) {
	loadDefaultShader();
}

Shader::~Shader() {}

void Shader::loadShader(std::string vertexShader, std::string fragmentShader) {
	// compile
	std::unique_ptr<OpenGLShaderProgram> newShader(new OpenGLShaderProgram(openGLContextRef));
	if (
		newShader->addVertexShader(vertexShader) &&
		newShader->addFragmentShader(fragmentShader) &&
		newShader->link()
	)
	{
		// reset shader
		shader.reset(newShader.release());
		// reset attributes and uniforms
		findAttributes(vertexShader);
		findUniforms(vertexShader + "\n" + fragmentShader);
		// reset error
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
	const std::regex comments(R"(/\*[\s\S]*?\*/|//.*)");
	src = std::regex_replace(src, comments, "");
	// search all attributes
	const std::regex target(R"(attribute[^;]+;)");
	std::vector<std::string> dst;
	std::sregex_iterator comment_end, comment_ite{ src.begin(), src.end(), target };
	for (; comment_ite != comment_end; ++comment_ite) {
		dst.push_back(comment_ite->str());
	}
	// get type and name
	const std::regex word(R"([\w_]+)");
	for (std::string s : dst) {
		std::sregex_iterator word_end, word_ite{ s.begin(), s.end(), word };
		std::string type, name;
		uint16_t count = 0;
		for (; word_ite != word_end; ++word_ite) {
			if (count == 1) type = word_ite->str();
			if (count == 2) name = word_ite->str();
			count++;
		}
		if (
			(count == 3) && 
			(openGLContextRef.extensions.glGetAttribLocation(
				shader->getProgramID(),
				name.c_str()
			) >= 0)
		) {
			attributesType[name] = type;
			attributes[name].reset(new OpenGLShaderProgram::Attribute(*shader.get(), name.c_str()));
		}
	}
}

void Shader::findUniforms(std::string src) {
	uniformsType.clear();
	uniforms.clear();
	// delete comments
	const std::regex comment(R"(/\*[\s\S]*?\*/|//.*)");
	src = std::regex_replace(src, comment, "");
	// search all uniforms
	const std::regex target(R"(uniform[^;]+;)");
	std::vector<std::string> dst;
	std::sregex_iterator comment_end, comment_ite{ src.begin(), src.end(), target };
	for (; comment_ite != comment_end; ++comment_ite) {
		dst.push_back(comment_ite->str());
	}
	// get type and name
	const std::regex word(R"([\w_]+)");
	for (std::string s : dst) {
		std::sregex_iterator word_end, word_ite{ s.begin(), s.end(), word };
		std::string type, name;
		uint16_t count = 0;
		for (; word_ite != word_end; ++word_ite) {
			if (count == 1) type = word_ite->str();
			if (count == 2) name = word_ite->str();
			count++;
		}
		if (
			(count == 3) &&
			(openGLContextRef.extensions.glGetUniformLocation(
				shader->getProgramID(),
				name.c_str()
			) >= 0)
		) {
			uniformsType[name] = type;
			uniforms[name].reset(new OpenGLShaderProgram::Uniform(*shader.get(), name.c_str()));
		}
	}
}