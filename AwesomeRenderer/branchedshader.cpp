

#include "branchedshader.h"

#include "shader_gl.h"
#include "program_gl.h"

#include "mesh_gl.h"

using namespace AwesomeRenderer;

const char* BranchedShader::KEYWORD_FORMAT = "#define %s\n";
const char* BranchedShader::SOURCE_HEADER = "#version 330\n";

BranchedShader::BranchedShader() : vertexSource(NULL), fragmentSource(NULL), currentBranch(0), currentKeywordIdx(0), keywordIndices(), branches()
{
	for (uint32_t keywordIdx = 0; keywordIdx < MAX_KEYWORDS; ++keywordIdx)
		keywordSource[keywordIdx] = NULL;
}

BranchedShader::~BranchedShader()
{
	if (vertexSource != NULL)
		delete vertexSource;

	if (fragmentSource != NULL)
		delete fragmentSource;

	for (uint32_t keywordIdx = 0; keywordIdx < MAX_KEYWORDS; ++keywordIdx)
	{
		if (keywordSource[keywordIdx] != NULL)
			delete keywordSource[keywordIdx];
	}

	for (auto it = branches.begin(); it != branches.end(); ++it)
		delete it->second;
}

void BranchedShader::SetSource(const char* vertexSource, const char* fragmentSource)
{
	// Create buffers to hold the vertex and fragment shader source and copy them to the new buffer
	uint32_t vertexSourceLength = strlen(vertexSource);
	uint32_t fragmentSourceLength = strlen(fragmentSource);

	this->vertexSource = new char[vertexSourceLength + 1];
	this->fragmentSource = new char[fragmentSourceLength + 1];

	memcpy(this->vertexSource, vertexSource, vertexSourceLength + 1);
	memcpy(this->fragmentSource, fragmentSource, fragmentSourceLength + 1);
}

void BranchedShader::SetKeyword(const char* keyword, bool state)
{
	assert(strlen(keyword) <= MAX_KEYWORD_LENGTH);

	std::string keywordString(keyword);

	uint32_t keywordIdx;

	// Check if the keyword already has an index
	auto keywordIt = keywordIndices.find(keywordString);
	if (keywordIt == keywordIndices.end())
	{
		assert(currentKeywordIdx < MAX_KEYWORDS);

		// Save the new index for the current keyword
		keywordIdx = currentKeywordIdx++;
		keywordIndices.insert(std::pair<std::string, uint32_t>(keywordString, keywordIdx));

		// Create the buffer holding the keyword source code
		char* buffer = new char[MAX_KEYWORD_SOURCE_LENGTH];
		sprintf_s(buffer, MAX_KEYWORD_SOURCE_LENGTH, KEYWORD_FORMAT, keyword);
		keywordSource[keywordIdx] = buffer;
	}
	else
		keywordIdx = keywordIt->second;

	// Set the correct bit in the current branch
	if (state)
		currentBranch |= (1 << keywordIdx);
	else
		currentBranch &= ~(1 << keywordIdx);
}

ProgramGL* BranchedShader::GetCurrentBranch()
{
	// Check if the current branch is already compiled
	auto branchIt = branches.find(currentBranch);
	if (branchIt == branches.end())
	{
		ProgramGL* branch = CompileCurrentBranch();
		branches.insert(std::pair<uint32_t, ProgramGL*>(currentBranch, branch));

		return branch;
	}

	return branchIt->second;
}

ProgramGL* BranchedShader::CompileCurrentBranch()
{
	assert(vertexSource != NULL && fragmentSource != NULL && "Shader source not set!");
	
	shaderSourceBuffers[0] = SOURCE_HEADER;

	uint32_t sourceCount = 1;

	// Iterate over all known keywords to check if they are active
	for (auto keywordIt = keywordIndices.begin(); keywordIt != keywordIndices.end(); ++keywordIt)
	{
		// Test if this keyword is active
		if ((currentBranch & (1 << keywordIt->second)) != 0)
		{
			shaderSourceBuffers[sourceCount] = keywordSource[keywordIt->second];
			++sourceCount;
		}
	}

	bool result;

	// Compile vertex shader
	ShaderGL* vertex = new ShaderGL(GL_VERTEX_SHADER);
	shaderSourceBuffers[sourceCount] = vertexSource;
	result = vertex->Compile((const char**) &shaderSourceBuffers[0], sourceCount + 1);
	assert(result && "Failed to compile vertex shader!");

	// Compile fragment shader
	ShaderGL* fragment = new ShaderGL(GL_FRAGMENT_SHADER);
	shaderSourceBuffers[sourceCount] = fragmentSource;
	result = fragment->Compile((const char**) &shaderSourceBuffers[0], sourceCount + 1);
	assert(result && "Failed to compile fragment shader!");

	// Attach shaders to program
	ProgramGL* program = new ProgramGL();
	program->Attach(vertex);
	program->Attach(fragment);

	// Set attribute locations for mesh data
	// TODO: make these names configurable?
	program->SetAttribLocation("inPosition", MeshGL::ATTR_POSITION);
	program->SetAttribLocation("inNormal", MeshGL::ATTR_NORMAL);
	program->SetAttribLocation("inColor", MeshGL::ATTR_COLOR);
	program->SetAttribLocation("inTexcoord", MeshGL::ATTR_TEXCOORD);
	program->SetAttribLocation("inTangent", MeshGL::ATTR_TANGENT);
	program->SetAttribLocation("inBitangent", MeshGL::ATTR_BITANGENT);

	program->Link();

	return program;
}