#ifndef _COLOR_H_
#define _COLOR_H_

namespace AwesomeRenderer
{
	class Color : public Vector4
	{
	public:
		static const Color BLACK;
		static const Color WHITE;

		static const Color RED;
		static const Color GREEN;
		static const Color BLUE;

		Color() : Vector4(0.0f, 0.0f, 0.0f, 1.0f)
		{

		}

		Color(float r, float g, float b, float a = 1.0f) : Vector4(r, g, b, a)
		{

		}
	};

}


#endif