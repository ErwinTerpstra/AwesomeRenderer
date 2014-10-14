#include "awesomerenderer.h"

using namespace AwesomeRenderer;

ObjLoader::ObjLoader(TextureFactory& textureFactory) : textureFactory(textureFactory)
{

}

void ObjLoader::Load(const char* fileName, Model& model)
{
	FileReader reader;
	
	if (!reader.Open(fileName))
	{
		printf("[ObjLoader]: Failed to load file \"%s\"\n", fileName);
		return;
	}

	Mesh::VertexAttributes defaultAttributes = (Mesh::VertexAttributes) (Mesh::VERTEX_POSITION | Mesh::VERTEX_NORMAL | Mesh::VERTEX_TEXCOORD);

	// Buffers that hold the vertex lists
	std::vector<Vector3> vertexBuffer;
	std::vector<Vector3> normalBuffer;
	std::vector<Vector2> texcoordBuffer;

	Mesh* mesh = NULL;
	Material* material = NULL;

	while (char* lineBuffer = reader.ReadLine())
	{		
		std::string line(lineBuffer);

		if (line.length() == 0)
			continue;

		switch (line[0])
		{
			case '\0':
			case '\r':	
			case '\n':
			case ' ':
			case '#':
				// Empty line or comment, skip it
				break;

			case 'g':
			{
				// New submesh
				// TODO: Manage memory registered by factory
				mesh = new Mesh(defaultAttributes);
				
				material = new Material();
				material->shader = defaultShader;

				model.AddMesh(mesh, material);
				
				break;
			}

			case 'v':
			{
				float v[3];

				do
				{
					// Vertex
					if (sscanf_s(lineBuffer, "v %f %f %f", &v[0], &v[1], &v[2]) > 0)
					{
						vertexBuffer.push_back(Vector3(v[0], v[1], v[2]));
						break;
					}

					// Texture coordinate
					if (sscanf_s(lineBuffer, "vt %f %f", &v[0], &v[1]) > 0)
					{
						texcoordBuffer.push_back(Vector2(v[0], v[1]));
						break;
					}

					// Normal
					if (sscanf_s(lineBuffer, "vn %f %f %f", &v[0], &v[1], &v[2]) > 0)
					{
						normalBuffer.push_back(Vector3(v[0], v[1], v[2]));
						break;
					}

					printf("[ObjLoader]: Invalid vertex definition in OBJ \"%s\"\n", lineBuffer);

				} while (false);

				break;
			}

			case 'f':
			{
				int vi[] = { 0, 0, 0 };
				int ti[] = { 0, 0, 0 };
				int ni[] = { 0, 0, 0 };
				
				// TODO: Add support for Quads (Split) and Polygons(Triangulate)

				int values = sscanf_s(lineBuffer, "f %d/%d/%d %d/%d/%d %d/%d/%d", 
										&vi[0], &ti[0], &ni[0],  
									    &vi[1], &ti[1], &ni[1],	
										&vi[2], &ti[2], &ni[2]);

				if (values > 0)
				{
					if (values < 9)
						printf("[ObjLoader]: Warning! face definition with only %d indices (\"%s\")\n", values, lineBuffer);

					// Iterate through all 3 vertices of the face
					for (int i = 0; i < 3; ++i)
					{
						Vector3 vertex = vi[i] - 1 >= 0 ? vertexBuffer[vi[i] - 1] : Vector3(0.0f, 0.0f, 0.0f);
						Vector3 normal = ni[i] - 1 >= 0 ? normalBuffer[ni[i] - 1] : Vector3(0.0f, 0.0f, 0.0f);
						Vector2 texcoord = ti[i] - 1 >= 0 ? texcoordBuffer[ti[i] - 1] : Vector2(0.0f, 0.0f);

						mesh->vertices.push_back(vertex);
						mesh->texcoords.push_back(texcoord);
						mesh->normals.push_back(normal);

						mesh->indices.push_back(mesh->vertices.size() - 1);
					}
				}
				else
					printf("[ObjLoader]: Invalid face definition in OBJ \"%s\"\n", lineBuffer);


				break;
			}

			default:
				// Material library directive
				if (line.compare(0, 6, "mtllib") == 0)
				{
					// TODO: make this an utility function somewhere
					std::string mtlLib(fileName);
					size_t idx = mtlLib.find_last_of('/');
					mtlLib.resize(idx != std::string::npos ? idx + 1 : 0);
					mtlLib += line.substr(7);
					
					LoadMaterialLib(mtlLib.c_str());
					break;
				}

				// Material reference directive
				if (line.compare(0, 6, "usemtl") == 0)
				{
					std::string name = line.substr(7);
					mesh = new Mesh(defaultAttributes);
					model.AddMesh(mesh, materialLib[name]);
					break;
				}

				printf("[ObjLoader]: Invalid line in OBJ \"%s\"\n", lineBuffer);
				break;
		}
	}
	
	reader.Close();
	
	// Count total number of vertices/triangles
	int verts = 0, tris = 0;

	std::vector<Mesh*>& meshes = model.meshes;
	
	for (unsigned int cMesh = 0; cMesh < meshes.size(); ++cMesh)
	{
		Mesh& mesh = *meshes[cMesh];
		verts += mesh.vertices.size();
		tris += mesh.indices.size() / 3;

		mesh.CalculateBounds();
	}

	model.CalculateBounds();

	// Release buffers
	vertexBuffer.clear();
	normalBuffer.clear();
	texcoordBuffer.clear();

	printf("[ObjLoader]: Loaded \"%s\" with %d vertices and %d triangles\n", fileName, verts, tris);
}


void ObjLoader::LoadMaterialLib(const char* fileName)
{
	materialLib.clear();
	
	// TODO: Gracefull handling of error
	FileReader reader;
	
	if (!reader.Open(fileName))
	{
		printf("[ObjLoader]: Failed to load file \"%s\"\n", fileName);
		return;
	}

	Material* material = NULL;
	
	int32_t lineLength;
	char* lineBuffer;

	while ((lineLength = reader.ReadLine(&lineBuffer)) >= 0)
	{		
		std::string line(lineBuffer);

		if (line.length() == 0)
			continue;

		switch (line[0])
		{
			case '\0':
			case '\r':	
			case '\n':
			case ' ':
			case '#':
				// Empty line or comment, skip it
				break;

			case 'K':
				if (line.length() < 3)
				{
					printf("[ObjLoader]: Invalid material color definition \"%s\"\n", lineBuffer);
					break;
				}

				if (material == NULL)
				{
					printf("[ObjLoader]: Defined material color before newmtl definition \"%s\"\n", lineBuffer);
					break;
				}

				switch (line[1])
				{
					case 'a':
						break;

					case 'd':
						ParseColor(lineBuffer + 2, material->diffuseColor);
						break;

					case 's':
						ParseColor(lineBuffer + 2, material->specularColor);
						break;

				}

				break;

			case 'N':
			{
				if (line.length() < 3)
				{
					printf("[ObjLoader]: Invalid material definition \"%s\"\n", lineBuffer);
					break;
				}

				if (material == NULL)
				{
					printf("[ObjLoader]: Defined material color before newmtl definition \"%s\"\n", lineBuffer);
					break;
				}

				switch (line[1])
				{
					case 's':
						material->shininess = strtof(lineBuffer + 2, NULL);
						break;
				}

				break;
			}
			case 'd':
			{
				float alpha = strtof(lineBuffer + 1, NULL);
				material->diffuseColor[3] = alpha;

				if (alpha < 1.0f)
					material->translucent = TRUE;

				break;
			}

			default:

				// New material directive
				if (line.compare(0, 6, "newmtl") == 0)
				{
					std::string name = line.substr(7);
					
					// TODO: Move memory allocation to somewhere else
					material = new Material();
					material->shader = defaultShader;

					materialLib[name] = material;
					break;
				}

				// Diffuse color map
				if (line.compare(0, 6, "map_Kd") == 0)
				{
					// Create a filename relative to the filename of the material library
					std::string textureFile(fileName);
					size_t idx = textureFile.find_last_of('/');
					textureFile.resize(idx != std::string::npos ? idx + 1 : 0);
					textureFile += line.substr(7);
					
					Texture* texture = NULL;
					Sampler* sampler = new Sampler(); // TODO: Move memory allocation to somewhere else

					if (textureFactory.GetAsset(textureFile, &texture))
					{
						sampler->texture = texture;
						material->diffuseMap = sampler;
					}
					else
						printf("[ObjLoader]: Failed to load diffuse map for material.\n");

					break;
				}



				printf("[ObjLoader]: Invalid line in MTL \"%s\"\n", lineBuffer);

				break;
		}
	}
	
	reader.Close();

	printf("[ObjLoader]: Loaded %d materials\n", materialLib.size());
}

void ObjLoader::ParseColor(const char* input, Color& color)
{
	const char* base = input;
	uint8_t channel = 0;
	
	// Read a maximum of 4 channels (r, g, b, a)
	while (channel < 4)
	{
		char* next = NULL;
		color[channel] = strtof(base, &next);

		if (*next == '\0' || *next == '\r' || *next == '\n')
			break;

		base = next;
		++channel;
	}
}