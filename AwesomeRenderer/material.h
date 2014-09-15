#ifndef _MATERIAL_H_
#define _MATERIAL_H_


namespace AwesomeRenderer
{
	class Texture;
	class Shader;

	class Material
	{
	public:
		bool translucent;

		Sampler* diffuseMap;
		Sampler* specularMap;
		Sampler* normalMap;
		
		Color diffuseColor;
		Color specularColor;

		float shininess;

		Shader* shader;

	public:
		Material();

	};


}


#endif