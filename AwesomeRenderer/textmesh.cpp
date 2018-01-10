
#include "awesomerenderer.h"

#include "textmesh.h"
#include "texture.h"

using namespace AwesomeRenderer;

TextMesh::TextMesh() : Mesh((Mesh::VertexAttributes) (Mesh::VERTEX_POSITION | Mesh::VERTEX_TEXCOORD))
{

}

void TextMesh::Configure(const Texture* texture, uint32_t glyphWidth, uint32_t glyphHeight, uint32_t tableOffset)
{
	this->texture = texture;

	this->glyphWidth = glyphWidth;
	this->glyphHeight = glyphHeight;

	this->tableOffset = tableOffset;

	uvScale = Vector2(((float) glyphWidth) / (texture->width - 1), ((float) glyphHeight) / (texture->height - 1));
	pixelSize = Vector2(1.0f / texture->width, 1.0f / texture->height);
}

void TextMesh::SetText(const std::string& text)
{
	this->text = text;

	CreateMesh();
}

const std::string& TextMesh::GetText() const
{
	return text;
}

void TextMesh::CreateMesh()
{
	Clear();

	uint32_t characters = 0;

	// Count how many printable characters our text has
	for (auto it = text.begin(); it != text.end(); ++it)
	{
		switch (*it)
		{
		case '\0':
		case '\t':
		case '\r':
		case '\n':
			break;

		default:
			++characters;
		}
	}

	// Reserve space for each character
	Reserve(characters * 2, characters * 4);
	
	uint32_t line = 0;
	uint32_t column = 0;
	uint32_t character = 0;

	for (auto it = text.begin(); it != text.end(); ++it)
	{
		switch (*it)
		{
		case '\t':
			// Tab-stop, align our column counter to the next breakpoint
			column = ((column / TAB_SIZE) + 1) * TAB_SIZE;
			break;

		case '\r':
			// Ignore carriage return since it will be followed by line feed
			break;

		case '\n':
			// Line feed, start a new line
			++line;
			column = 0;
			break;

		default:
			// Printable character, add it to the mesh
			Point2 glyphLocation;
			GetGlyphLocation(*it, glyphLocation);

			Vector3 base(column * glyphWidth, line * glyphHeight, 0.0f);

			// Create four vertices for this glyph
			vertices.push_back(base + Vector3(0.0f, 0.0f, 0.0f));
			vertices.push_back(base + Vector3(glyphWidth, 0.0f, 0.0f));
			vertices.push_back(base + Vector3(glyphWidth, glyphHeight, 0.0f));
			vertices.push_back(base + Vector3(0.0f, glyphHeight, 0.0f));

			texcoords.push_back(LocationToUV(glyphLocation + Point2(0, 0)));
			texcoords.push_back(LocationToUV(glyphLocation + Point2(1, 0)));
			texcoords.push_back(LocationToUV(glyphLocation + Point2(1, 1)));
			texcoords.push_back(LocationToUV(glyphLocation + Point2(0, 1)));

			uint32_t n = vertices.size();

			// Top-left triangle
			indices.push_back(n - 4);
			indices.push_back(n - 3);
			indices.push_back(n - 1);

			// Bottom-rightt triangle
			indices.push_back(n - 3);
			indices.push_back(n - 2);
			indices.push_back(n - 1);

			++character;
			++column;
			break;
		}
	}
}

void TextMesh::GetGlyphLocation(char glyph, Point2& location) const
{
	glyph -= tableOffset;

	uint32_t gridWidth = (texture->width / glyphWidth);
	uint32_t gridHeight = (texture->height / glyphHeight);

	location[0] = (glyph % gridWidth);
	location[1] = gridHeight - (glyph / gridWidth) - 1;
}

Vector2 TextMesh::LocationToUV(Point2 location) const
{
	return Vector2(location[0] * uvScale[0], location[1] * uvScale[1]);
}