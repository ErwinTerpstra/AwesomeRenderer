#ifndef _EXTENDEE_H_
#define _EXTENDEE_H_

namespace AwesomeRenderer
{

	template <class ProviderType>
	class ExtensionProvider
	{

	private:
		typedef std::map<uint32_t, BaseExtension<ProviderType>*> ExtensionMap;
		
		ExtensionMap extensions;

	public:
		ExtensionProvider() : extensions() { }

		void Extend(BaseExtension<ProviderType>* extension)
		{
			Extend(extension, extension->GetExtensionID());
		}

		void Extend(BaseExtension<ProviderType>* extension, uint32_t extensionID)
		{
			extensions.insert(std::pair<uint32_t, BaseExtension<ProviderType>*>(extensionID, extension));
		}

		template<class ExtensionType>
		ExtensionType* As() const 
		{
			ExtensionMap::const_iterator it = extensions.find(ExtensionType::ExtensionID());

			if (it == extensions.end())
				return NULL;

			return static_cast<ExtensionType*>(it->second);
		}

		template<class ExtensionType>
		bool HasExtension() const 
		{
			return extensions.find(ExtensionType::ExtensionID()) != extensions.end();
		}
	};
}

#endif