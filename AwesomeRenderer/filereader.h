#ifndef _FILE_READER_H_
#define _FILE_READER_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{


	class FileReader
	{

	private:

		FILE* ptr;
		char lineBuffer[512];

	public:
		FileReader();

		bool Open(const char* fileName);
		void Close();

		int Read(char* buffer);
		char* ReadLine();
		int ReadLine(char* buffer);
		int ReadLine(char** buffer);
	};

}

#endif