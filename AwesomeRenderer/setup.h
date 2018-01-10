#ifndef _SETUP_H_
#define _SETUP_H_

namespace AwesomeRenderer
{
	class Context;

	class Setup
	{
	private:
		Context& context;
		
		bool calculateExtendedMeshData;

	public:
		Setup(Context& context, bool calculateExtendedMeshData);

		void SetupScene();
		void SetupLighting();

		void SetupCornellBox();
		void SetupSpheres();
		void SetupSponza();
		void SetupFractal();
	};
}

#endif