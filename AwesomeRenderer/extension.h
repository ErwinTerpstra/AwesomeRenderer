#ifndef _EXTENSION_H_
#define _EXTENSION_H_
	

namespace AwesomeRenderer
{

	template<class ProviderType>
	class BaseExtension
	{
	public:
		ProviderType& provider;

	public:
		BaseExtension(ProviderType& provider, uint32_t extensionID) : provider(provider)
		{
			provider.Extend(this, extensionID);
		}

		virtual uint32_t GetExtensionID() = 0;
	};

	template<class ProviderType, class ExtensionType>
	class Extension : public BaseExtension<ProviderType>
	{

	public:
		Extension(ProviderType& provider) : BaseExtension(provider, GetExtensionID())
		{

		}

		uint32_t GetExtensionID() { return ExtensionType::ExtensionID(); }
	};


}

#endif