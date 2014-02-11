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

		Color(const Color& other) : Color(other[0], other[1], other[2], other[3])
		{

		}

		Color(const Vector4& other) : Color(other[0], other[1], other[2], other[3])
		{

		}

		
		Color(float r, float g, float b, float a = 1.0f) : Vector4(cml::clamp(r, 0.0f, 1.0f), cml::clamp(g, 0.0f, 1.0f), cml::clamp(b, 0.0f, 1.0f), cml::clamp(a, 0.0f, 1.0f))
		{

		}
		
		Color& operator=(const Color &rhs)
		{
			if (this == &rhs)
				return *this;

			this->set(rhs[0], rhs[1], rhs[2], rhs[3]);

			return *this;
		}

		Color& operator=(const Vector4 &rhs)
		{
			if (this == &rhs)
				return *this;

			this->set(rhs[0], rhs[1], rhs[2], rhs[3]);

			return *this;
		}

		Color& operator*=(const float& rhs)
		{
			this->set(cml::clamp(this->operator[](0) * rhs, 0.0f, 1.0f),
				      cml::clamp(this->operator[](1) * rhs, 0.0f, 1.0f),
				      cml::clamp(this->operator[](2) * rhs, 0.0f, 1.0f),
				      cml::clamp(this->operator[](3) * rhs, 0.0f, 1.0f));

			return *this;
		}

		Color& operator*=(const Color& rhs)
		{
			this->set(cml::clamp(this->operator[](0) * rhs[0], 0.0f, 1.0f),
					  cml::clamp(this->operator[](1) * rhs[1], 0.0f, 1.0f),
					  cml::clamp(this->operator[](2) * rhs[2], 0.0f, 1.0f),
					  cml::clamp(this->operator[](3) * rhs[3], 0.0f, 1.0f));

			return *this;
		}

		Color& operator+=(const Color& rhs)
		{
			this->set(cml::clamp(this->operator[](0) + rhs[0], 0.0f, 1.0f), 
					  cml::clamp(this->operator[](1) + rhs[1], 0.0f, 1.0f),
					  cml::clamp(this->operator[](2) + rhs[2], 0.0f, 1.0f),
					  cml::clamp(this->operator[](3) + rhs[3], 0.0f, 1.0f));

			return *this;
		}

		const Color operator*(const Color& rhs) const
		{
			Color result = *this;
			result *= rhs;

			return result;
		}

		const Color operator*(const float& rhs) const
		{
			Color result = *this;
			result *= rhs;

			return result;
		}

		const Color operator+(const Color& rhs) const
		{
			Color result = *this;
			result += rhs;

			return result;
		}
	};

}


#endif