#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_

namespace AwesomeRenderer
{
	// Atomic types
	typedef unsigned char uchar;

	// Geometric types
	template <int Size>
	struct cml_t
	{
		typedef cml::vector<float, cml::fixed<Size> > Vector;
		typedef cml::vector<int, cml::fixed<Size> > Point;
	};

	typedef cml_t<2>::Vector Vector2;
	typedef cml_t<3>::Vector Vector3;
	typedef cml_t<4>::Vector Vector4;
	
	typedef cml_t<2>::Point Point2;
	typedef cml_t<3>::Point Point3;
	typedef cml_t<4>::Point Point4;

	typedef cml::quaternionf_p Quaternion;

	typedef cml::matrix44f_r Matrix44;
	typedef cml::matrix43f_r Matrix43;
	typedef cml::matrix33f_r Matrix33;

	typedef cml::matrix22i_r IntMatrix22;
	typedef cml::matrix32i_r IntMatrix23;

	Vector3 operator-(const float& lhs, const Vector3& rhs);
	Vector3 operator*(const Vector3& lhs, const Vector3& rhs);
	Vector2 operator*(const Vector2& lhs, const Vector2& rhs);
}

#endif