#ifndef _FILE_READER_H_
#define _FILE_READER_H_

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

		char* ReadLine();
		int ReadLine(char** buffer);
	};

}

#endif