#ifndef _TRANSFORMATION_H_
#define _TRANSFORMATION_H_

#include "awesomerenderer.h"
#include "component.h"

namespace AwesomeRenderer
{

	class Transformation : public Component
	{
	public:
		static const int ID;

	private:
		Transformation* parent;

		Vector3 position;
		
		Quaternion rotation;

		Vector3 scale;

		Matrix44 localMtx, worldMtx;

		bool dirty;

	public:

		Transformation();

		void SetPosition(const Vector3& position);
		void SetRotation(const Quaternion& rotation);
		void SetScale(const Vector3& scale);

		void GetPosition(Vector3& position) const { position = this->position; }
		void GetRotation(Quaternion& rotation) const { rotation = this->rotation; }
		void GetScale(Vector3& scale) const { scale = this->scale; }

		Vector3 GetPosition() const { return position; }
		Quaternion GetRotation() const { return rotation; }
		Vector3 GetScale() const { return scale; }

		void SetParent(Transformation* parent) { this->parent = parent; }
		
		const Matrix44& WorldMtx() const;
		const Matrix44& LocalMtx() const;

		void CalculateMtx();
	};

}


#endif