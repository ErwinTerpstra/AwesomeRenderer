#ifndef _EXTENSION_H_
#define _EXTENSION_H_


namespace AwesomeRenderer
{

	template<class ExtendeeType>
	class Extension
	{

	public:
		ExtendeeType& base;

	public:
		Extension(ExtendeeType& base) : base(base)
		{
			base.Extend(this);
		}

	};


}

#endif