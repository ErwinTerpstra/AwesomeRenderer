#ifndef _SETUP_H_
#define _SETUP_H_

namespace AwesomeRenderer
{
	class Context;

	class Setup
	{
	private:
		Context& context;

	public:
		Setup(Context& context);

		void SetupScene();
		void SetupLighting();

		void SetupCornellBox();
		void SetupSpheres();
		void SetupFractal();
	};
}

#endif