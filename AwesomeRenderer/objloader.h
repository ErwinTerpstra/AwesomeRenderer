#ifndef _OBJ_LOADER_H_
#define _OBJ_LOADER_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{

	class Model;
	class Mesh;
	class Material;
	class PhongMaterial;
	class Shader;
	class TextureFactory;

	class ObjLoader
	{
		
	public:

		class IndexReader
		{
		public:
			static const uint32_t MAX_VERTICES = 10;

			struct VertexIndices
			{
				int32_t vertexIdx, texcoordIdx, normalIdx;

				VertexIndices() : vertexIdx(0), texcoordIdx(0), normalIdx(0)
				{

				}
			};

			VertexIndices vertexIndices[MAX_VERTICES];
		private:
			int32_t vertexIdx;
			int32_t attributeIdx;
			int32_t sign;
			int32_t currentIdx;

		public:
			IndexReader();
			uint32_t Parse(const char* buffer, uint32_t offset, uint32_t amount);

		private:
			void SaveCurrentIdx();
			void NextVertex();
			void ResetIndex();
		};
	private:
		std::map<std::string, Material*> materialLib;

		TextureFactory& textureFactory;

		Material* defaultMaterial;
		PhongMaterial* defaultMaterialImplementation;

		Shader* defaultShader;


	public:
		ObjLoader(TextureFactory& textureFactory);

		void Load(const char* fileName, Model& model);
		void LoadMaterialLib(const char* fileName);

	private:
		std::string GetRelativeFileName(std::string fileName, const char* basePath);
		void ParseColor(const char* input, Color& color);
	};


}


#endif