#ifndef _TRIANGLE_2D_H_
#define _TRIANGLE_2D_H_


namespace AwesomeRenderer
{

	class Triangle2D : public Triangle<Vector2>
	{


	public:
		Triangle2D(const Vector2& a, const Vector2& b, const Vector2& c);
		Triangle2D(const Triangle2D& other);
		
		void CalculateBounds(Vector2& lower, Vector2& upper);
	};

}


#endif