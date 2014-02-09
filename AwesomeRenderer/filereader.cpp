#include "awesomerenderer.h"

using namespace AwesomeRenderer;

FileReader::FileReader() : ptr(NULL)
{

}


bool FileReader::Open(const char* fileName)
{	
	if (ptr != NULL)
		Close();

	// TODO: Handle different error codes
	errno_t error = fopen_s(&ptr, fileName, "r");

	if (error != 0)
		return false;

	return true;
}

void FileReader::Close()
{
	if (ptr == NULL)
		return;

	fclose(ptr);
	ptr = NULL;
}

char* FileReader::ReadLine()
{	
	char* bufferPtr;
	
	if (ReadLine(&bufferPtr) >= 0)
		return bufferPtr;

	return NULL;
}

int FileReader::ReadLine(char** buffer)
{
	char* lineBufferPtr = &lineBuffer[0];

	// If we couldn't read a single character without EOF, let the caller know
	if (feof(ptr))
		return -1;

	// Read until EOF
	while (!feof(ptr))
	{
		char c = (char) fgetc(ptr);
		
		if (c == '\n')
			break;
		
		*(lineBufferPtr++) = c;
	}
	
	// Length is difference between start and end of buffer
	int lineLength = (lineBufferPtr - lineBuffer);
	*lineBufferPtr = '\0';	// Finish string with a NULL byte

	// Pass the buffer pointer to the caller
	*buffer = &lineBuffer[0];

	return lineLength;
}