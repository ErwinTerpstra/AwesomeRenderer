#ifndef _BRANCHED_SHADER_H_
#define _BRANCHED_SHADER_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	class ProgramGL;

	class BranchedShader
	{

	private:
		static const uint32_t MAX_KEYWORDS = 32;
		static const uint32_t MAX_KEYWORD_LENGTH = 32;
		static const uint32_t MAX_KEYWORD_SOURCE_LENGTH = MAX_KEYWORD_LENGTH + 12;
		static const char* KEYWORD_FORMAT;
		static const char* SOURCE_HEADER;

		char* vertexSource;
		char* fragmentSource;

		uint32_t currentBranch;
		uint32_t currentKeywordIdx;

		std::map<std::string, uint32_t> keywordIndices;

		std::map<uint32_t, ProgramGL*> branches;

		char* keywordSource[MAX_KEYWORDS];
		const char* shaderSourceBuffers[MAX_KEYWORDS + 2]; // Add space for the fixed header and the actual file source

	public:
		BranchedShader();
		~BranchedShader();

		void SetSource(const char* vertexSource, const char* fragmentSource);
		void SetKeyword(const char* keyword, bool state);

		ProgramGL* GetCurrentBranch();

	private:
		ProgramGL* CompileCurrentBranch();
	};

}

#endif