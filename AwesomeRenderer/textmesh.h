#ifndef _TEXT_MESH_H_
#define _TEXT_MESH_H_

#include "awesomerenderer.h"
#include "mesh.h"

namespace AwesomeRenderer
{
	class Texture;

	class TextMesh : public Mesh
	{

	private:
		const int TAB_SIZE = 4;

		std::string text;

		const Texture* texture;

		uint32_t glyphWidth;
		uint32_t glyphHeight;
		uint32_t tableOffset;
		Vector2 uvScale;
		Vector2 pixelSize;

	public:
		TextMesh();

		void Configure(const Texture* texture, uint32_t glyphWidth, uint32_t glyphHeight, uint32_t tableOffset = 0);

		void SetText(const std::string& text);
		const std::string& GetText() const;

	private:
		void CreateMesh();

		void GetGlyphLocation(char glyph, Point2& location) const;
		Vector2 LocationToUV(Point2 location) const;
	};
}

#endif