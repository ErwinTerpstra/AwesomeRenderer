#ifndef _MATERIAL_H_
#define _MATERIAL_H_


namespace AwesomeRenderer
{
	class Texture;
	class Shader;

	class Material
	{

	public:
		Texture* texture;

		Shader* shader;

	public:
		Material();

	};


}


#endif