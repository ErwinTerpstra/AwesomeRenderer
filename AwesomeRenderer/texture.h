#ifndef _TEXTURE_H_
#define _TEXTURE_H_


namespace AwesomeRenderer
{

	class Texture : public MemoryBuffer
	{
	public:

		enum WrapMode
		{
			WM_CLAMP,
			WM_REPEAT,

			WM_DEFAULT = WM_REPEAT
		};

	public:
		Texture();

		Color Sample(const Vector2& uv, WrapMode wrapMode = WM_DEFAULT) const;
		void Sample(const Vector2& uv, Color& sample, WrapMode wrapMode = WM_DEFAULT) const;

	};

}

#endif