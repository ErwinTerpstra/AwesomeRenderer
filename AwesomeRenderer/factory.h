#ifndef _FACTORY_H_
#define _FACTORY_H_

namespace AwesomeRenderer
{
	template<class FactoryType, class AssetType>
	class Factory
	{
	public:
		typedef bool (FactoryType::*LoadFunction) (const std::string& fileName, AssetType** instance) const;

	private:
		std::map<std::string, AssetType*> assets;

		std::map<std::string, LoadFunction> loadFunctions;

	public:

		Factory()
		{

		}

		~Factory()
		{
			Release();
		}

		bool Allocate(AssetType** instance)
		{
			*instance = static_cast<AssetType*>(malloc(sizeof(AssetType)));
			return *instance != NULL && Instantiate(instance);
		}

		void Destroy(AssetType** instance)
		{
			delete *instance;
			*instance = NULL;
		}

		void Release()
		{
			// Destroy all assets
			for (auto it = assets.begin(); it != assets.end(); ++it)
				Destroy(&it->second);

			assets.clear();
			loadFunctions.clear();
		}

		bool HasAsset(const std::string fileName)
		{
			auto it = assets.find(fileName);
			return it != assets.end();
		}

		bool GetAsset(const std::string fileName, AssetType** instance)
		{
			auto it = assets.find(fileName);

			// Check if the asset with this file name exists
			if (it != assets.end())
			{
				*instance = it->second;
				return true;
			}

			// Asset not found, load it
			return Load(fileName, instance);
		}

		bool Load(const std::string fileName, AssetType** instance)
		{
			int32_t idx = fileName.find_last_of('.');

			// Select correct loading function based on file extension
			if (idx >= 0)
			{
				const std::string extension = fileName.substr(idx + 1);
				auto it = loadFunctions.find(extension);

				if (it != loadFunctions.end())
				{
					// Allocate memory for new asset
					bool success;
					success = Allocate(instance);

					if (!success)
					{
						printf("[Factory]: Failed to allocate memory for file: \"%s\"\n", fileName.c_str());
						return false;
					}

					LoadFunction loadFunction = it->second;
					const FactoryType* factory = static_cast<const FactoryType*>(this);

					// Execute load function on asset
					success = (factory->*loadFunction)(fileName, instance);

					if (!success)
					{
						printf("[Factory]: Error while loading file: \"%s\"\n", fileName.c_str());
						Destroy(instance);
						return false;
					}

					// Save asset in map
					assets.insert(std::pair<std::string, AssetType*>(fileName, *instance));

					return true;
				}

			}

			printf("[Factory]: Unsupported file format for file \"%s\"\n", fileName.c_str());
			return false;
		}

	protected:

		virtual bool Instantiate(AssetType** instance) const = 0;

		void AddLoadFunction(const std::string extension, LoadFunction function)
		{
			loadFunctions.insert(std::pair<std::string, LoadFunction>(extension, function));
		}
	};
}

#endif