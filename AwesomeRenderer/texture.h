#ifndef _TEXTURE_H_
#define _TEXTURE_H_


namespace AwesomeRenderer
{

	class Texture : public MemoryBuffer
	{
	public:

	public:
		Texture();

		Color Sample(const Vector2& uv) const;
		void Sample(const Vector2& uv, Color& sample) const;

	};

}

#endif