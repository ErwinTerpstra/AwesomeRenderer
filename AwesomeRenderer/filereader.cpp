#include "stdafx.h"
#include "awesomerenderer.h"
#include "filereader.h"

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

int FileReader::Read(char* buffer)
{
	int lineLength;
	char* current = buffer;

	// Read lines until no more are available
	while ((lineLength = ReadLine(current)) >= 0)
	{
		// Advance the current buffer pointer and save a newline after the line
		current += lineLength;
		*(current++) = '\n';
	}

	// Finish string with null byte
	*current = '\0';

	// The total read length is difference between current and start pointers
	return (current - buffer - 1);
}

char* FileReader::ReadLine()
{	
	char* bufferPtr;
	
	if (ReadLine(&bufferPtr) >= 0)
		return bufferPtr;

	return NULL;
}

int FileReader::ReadLine(char* buffer)
{
	char* current = buffer;

	// If we couldn't read a single character without EOF, let the caller know
	if (feof(ptr))
		return -1;

	// Read until End-of-file
	char c;
	while ((c = (char)fgetc(ptr)) != EOF)
	{
		// Ignore carriage return since they are always followed by line-feed (\n)
		if (c == '\r')
			continue;

		if (c == '\n')
			break;

		*(current++) = c;
	}

	// Length is difference between start and end of buffer
	int lineLength = (current - buffer);
	*current = '\0';	// Finish string with a NULL byte

	return lineLength;
}

int FileReader::ReadLine(char** buffer)
{
	// Pass the buffer pointer to the caller
	*buffer = &lineBuffer[0];

	return ReadLine(*buffer);
}