#ifndef _COLOR_H_
#define _COLOR_H_

namespace AwesomeRenderer
{
	class Color : public Vector4
	{
	public:
		static const Color BLACK;
		static const Color GRAY;
		static const Color WHITE;

		static const Color RED;
		static const Color GREEN;
		static const Color BLUE;

		static const Color YELLOW;
		static const Color PURPLE;
		static const Color CYAN;

		Color() : Vector4(0.0f, 0.0f, 0.0f, 1.0f)
		{

		}

		Color(const Color& other) : Color(other[0], other[1], other[2], other[3])
		{

		}

		Color(const Vector4& other) : Color(other[0], other[1], other[2], other[3])
		{

		}

		Color(const Vector3& v, float alpha = 1.0f) : Color(v[0], v[1], v[2], alpha)
		{

		}

		
		Color(float r, float g, float b, float a = 1.0f) : Vector4(r, g, b, a)
		{

		}

		Color(int r, int g, int b, int a = 255) : Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f)
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
			this->set(this->operator[](0) * rhs, this->operator[](1) * rhs, this->operator[](2) * rhs, this->operator[](3) * rhs);

			return *this;
		}

		Color& operator*=(const Color& rhs)
		{
			this->set(this->operator[](0) * rhs[0], this->operator[](1) * rhs[1], this->operator[](2) * rhs[2], this->operator[](3) * rhs[3]);

			return *this;
		}

		Color& operator+=(const Color& rhs)
		{
			this->set(this->operator[](0) + rhs[0], this->operator[](1) + rhs[1], this->operator[](2) + rhs[2], this->operator[](3) + rhs[3]);

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