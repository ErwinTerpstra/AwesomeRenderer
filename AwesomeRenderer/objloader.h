#ifndef _OBJ_LOADER_H_
#define _OBJ_LOADER_H_


namespace AwesomeRenderer
{

	class ObjLoader
	{
		
	public:

		Shader* defaultShader;

	private:
		std::map<std::string, Material*> materialLib;

		const TextureFactory& textureFactory;

	public:
		ObjLoader(const TextureFactory& textureFactory);

		void Load(const char* fileName, Model& model);
		void LoadMaterialLib(const char* fileName);

	private:
		void Reset();

		void ParseColor(const char* input, Color& color);
	};


}


#endif