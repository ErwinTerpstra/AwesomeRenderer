#ifndef _EXTENDEE_H_
#define _EXTENDEE_H_

namespace AwesomeRenderer
{

	template <class T>
	class Extendee
	{

	private:
		Extension<T>* ex;

	public:
		Extendee() : ex(NULL) { }

		void Extend(Extension<T>* extension) { ex = extension; }

		template<class T2>
		T2* As() const { return static_cast<T2*>(ex); }

		bool HasExtension() const { return ex != NULL; }
	};
}

#endif