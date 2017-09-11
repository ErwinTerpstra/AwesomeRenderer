#include "stdafx.h"
#include "awesomerenderer.h"
#include "objloader.h"
#include "texturefactory.h"
#include "model.h"
#include "filereader.h"
#include "mesh.h"
#include "phongmaterial.h"
#include "microfacetmaterial.h"
#include "phongshader.h"
#include "sampler.h"
#include "blinndistribution.h"

using namespace AwesomeRenderer;

const float ObjLoader::HEIGHT_MAP_NORMAL_STRENGTH = 2.0f;

ObjLoader::ObjLoader(TextureFactory& textureFactory) : textureFactory(textureFactory)
{
	defaultShader = new PhongShader();

	defaultMaterial = new Material();
	defaultMaterial->shader = defaultShader;

	defaultMaterialImplementation = new PhongMaterial(*defaultMaterial);
}

void ObjLoader::Load(const char* fileName, Model& model)
{
	FileReader reader;
	
	if (!reader.Open(fileName))
	{
		printf("[ObjLoader]: Failed to load file \"%s\"\n", fileName);
		return;
	}

	Mesh::VertexAttributes defaultAttributes = (Mesh::VertexAttributes) 0;

	// Buffers that hold the vertex lists
	std::vector<Vector3> vertexBuffer;
	std::vector<Vector3> normalBuffer;
	std::vector<Vector2> texcoordBuffer;

	Mesh* mesh = NULL;
	Material* material = NULL;
	
	while (char* lineBuffer = reader.ReadLine())
	{		
		std::string line(lineBuffer);

		uint32_t charIdx = 0;
		
		// Skip whitespace at the start of the line
		for (charIdx = 0; charIdx < line.length(); ++charIdx)
		{
			char c = line[charIdx];
			if (c != ' ' || c != '\t')
				break;

		}

		if (charIdx == line.length())
			continue;

		lineBuffer += charIdx;
		line = line.substr(charIdx);
		charIdx = 0;

		switch (line[charIdx])
		{
			case '\0':
			case '\r':	
			case '\n':
			case '#':
				// Empty line or comment, skip it
				break;

			case 'g':
			{
				// New submesh
				// TODO: Manage memory registered by factory
				mesh = new Mesh(defaultAttributes);
				material = NULL;
				
				model.AddMesh(mesh, defaultMaterial);
				
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
						Vector3 normal(v[0], v[1], v[2]);
						normal.normalize();
						normalBuffer.push_back(normal);
						break;
					}

					printf("[ObjLoader]: Invalid vertex definition in OBJ \"%s\"\n", lineBuffer);

				} while (false);

				break;
			}

			case 'f':
			{
				assert(mesh != NULL && "Face definition found before group definition!");

				IndexReader reader;
				uint32_t vertices = reader.Parse(lineBuffer, 2, line.length() - 2);

				assert(vertices == 3 || vertices == 4);

				for (uint32_t vertexIdx = 0; vertexIdx < vertices; ++vertexIdx)
				{
					IndexReader::VertexIndices& vi = reader.vertexIndices[vertexIdx];

					// Negative indices are relative to the buffer arrays
					// Others are index in the buffer array + 1

					if (vi.vertexIdx < 0)
						vi.vertexIdx = vertexBuffer.size() + vi.vertexIdx;
					else
						--vi.vertexIdx;

					if (vi.normalIdx < 0)
						vi.normalIdx = normalBuffer.size() + vi.normalIdx;
					else
						--vi.normalIdx;

					if (vi.texcoordIdx < 0)
						vi.texcoordIdx = texcoordBuffer.size() + vi.texcoordIdx;
					else
						--vi.texcoordIdx;
				}

				// Triangle indices
				const uint32_t triangles[] =
				{
					0, 1, 2,
					0, 2, 3
				};

				// Iterate through all vertices of the face
				for (uint32_t triIdx = 0; triIdx < (vertices - 2); ++triIdx)
				{
					//for (uint32_t vertexIdx = 0; vertexIdx < 3; ++vertexIdx)
					for (int32_t vertexIdx = 2; vertexIdx >= 0; --vertexIdx)
					{
						IndexReader::VertexIndices& vi = reader.vertexIndices[triangles[triIdx * 3 + vertexIdx]];
							
						if (vi.vertexIdx >= 0)
						{
							mesh->vertices.push_back(vertexBuffer[vi.vertexIdx]);
							mesh->attributes = (Mesh::VertexAttributes) (mesh->attributes | Mesh::VERTEX_POSITION);
						}

						if (vi.texcoordIdx >= 0)
						{
							mesh->texcoords.push_back(texcoordBuffer[vi.texcoordIdx]);
							mesh->attributes = (Mesh::VertexAttributes) (mesh->attributes | Mesh::VERTEX_TEXCOORD);
						}

						if (vi.normalIdx >= 0)
						{
							mesh->normals.push_back(normalBuffer[vi.normalIdx]);
							mesh->attributes = (Mesh::VertexAttributes) (mesh->attributes | Mesh::VERTEX_NORMAL);
						}
							
						mesh->indices.push_back(mesh->vertices.size() - 1);
					}
				}


				break;
			}

			default:
				// Material library directive
				if (line.compare(0, 6, "mtllib") == 0)
				{
					std::string mtlLib = GetRelativeFileName(line.substr(7), fileName);					
					LoadMaterialLib(mtlLib.c_str());
					break;
				}

				// Material reference directive
				if (line.compare(0, 6, "usemtl") == 0)
				{
					std::string name = line.substr(7);

					Material* linkedMaterial = materialLib[name];
					
					if (material != NULL)
					{
						// If the mesh already has a material, create a new submesh
						mesh = new Mesh(defaultAttributes);
						model.AddMesh(mesh, linkedMaterial);
					}
					else
					{
						// Assign this material to the last created submesh
						model.materials[model.materials.size() - 1] = linkedMaterial;
					}

					material = linkedMaterial;

					break;
				}

				//printf("[ObjLoader]: Invalid line in OBJ \"%s\"\n", lineBuffer);
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

		// If the mesh's material has a normal map, calculate tangent space basis vectors
		PhongMaterial* material = model.materials[cMesh]->As<PhongMaterial>();
		if (material != NULL && material->normalMap != NULL)
			mesh.CalculateTangentBasis();
	}

	model.CalculateBounds();

	// Release buffers
	vertexBuffer.clear();
	normalBuffer.clear();
	texcoordBuffer.clear();

	printf("[ObjLoader]: Loaded \"%s\" with %d vertices and %d triangles in %d meshes.\n", fileName, verts, tris, meshes.size());
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
	PhongMaterial* phongMaterial = NULL;
	MicrofacetMaterial* microfacetMaterial = NULL;
	
	int32_t lineLength;
	char* lineBuffer;

	while ((lineLength = reader.ReadLine(&lineBuffer)) >= 0)
	{		
		std::string line(lineBuffer);

		uint32_t charIdx;

		// Skip whitespace at the start of the line
		for (charIdx = 0; charIdx < line.length(); ++charIdx)
		{
			char c = line[charIdx];

			if (c != ' ' && c != '\t')
				break;
		}

		if (charIdx == line.length())
			continue;

		lineBuffer += charIdx;
		line = line.substr(charIdx);
		charIdx = 0;

		switch (line[0])
		{
			case '\0':
			case '\r':	
			case '\n':
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

				switch (lineBuffer[1])
				{
					case 'a':
						// Ambient color, but often set as duplicate for diffuse
						// Ignore for now...
						break;

					case 'd':
					{
						Color color;
						ParseColor(lineBuffer + 2, color);

						phongMaterial->diffuseColor = color;
						microfacetMaterial->albedo = color;
						break;
					}

					case 's':
					{
						Color color;
						ParseColor(lineBuffer + 2, color);

						phongMaterial->specularColor = color;
						microfacetMaterial->specular = color;
						break;
					}

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

				switch (lineBuffer[1])
				{
					case 's':
						float shininess = strtof(lineBuffer + 2, NULL);

						phongMaterial->shininess = shininess;
						phongMaterial->specularColor[3] = shininess;

						microfacetMaterial->roughness = RayTracing::BlinnDistribution::ShininessToRoughness(shininess);
						break;
				}

				break;
			}
			case 'd':
			{
				float alpha = strtof(lineBuffer + 1, NULL);
				phongMaterial->diffuseColor[3] = alpha;
				microfacetMaterial->albedo[3] = alpha;

				if (alpha < 1.0f)
					material->translucent = TRUE;

				break;
			}

			default:

				// New material directive
				if (line.compare(0, 6, "newmtl") == 0)
				{					
					// TODO: Move memory allocation to somewhere else
					material = new Material();
					material->name = line.substr(7);
					material->shader = defaultShader;

					phongMaterial = new PhongMaterial(*material);
					microfacetMaterial = new MicrofacetMaterial(*material);
					
					materialLib[material->name] = material;
					break;
				}

				// Diffuse color map
				if (line.compare(0, 6, "map_Kd") == 0)
				{
					// Create a filename relative to the filename of the material library
					std::string textureFile = GetRelativeFileName(line.substr(7), fileName);
					
					Sampler* sampler = textureFactory.GetTexture(textureFile);
					
					if (sampler)
					{
						phongMaterial->diffuseMap = sampler;
						microfacetMaterial->albedoMap = sampler;
					}
					else
						printf("[ObjLoader]: Failed to load diffuse map for material.\n");

					break;
				}


				// Specular color map
				if (line.compare(0, 6, "map_Ks") == 0)
				{
					// Create a filename relative to the filename of the material library
					std::string textureFile = GetRelativeFileName(line.substr(7), fileName);

					Sampler* sampler = textureFactory.GetTexture(textureFile);

					if (sampler)
					{
						phongMaterial->specularMap = sampler;
						microfacetMaterial->specularMap = sampler;
					}
					else
						printf("[ObjLoader]: Failed to load specular map for material.\n");

					break;
				}

				// Bump map
				if (line.compare(0, 8, "map_bump") == 0)
				{
					// Create a filename relative to the filename of the material library
					std::string textureFile = GetRelativeFileName(line.substr(9), fileName);

					Texture* heightMap = NULL;
				
					if (textureFactory.GetAsset(textureFile, &heightMap))
					{
						Texture* normalMap = textureFactory.ConvertHeightMapToNormalMap(heightMap, HEIGHT_MAP_NORMAL_STRENGTH);

						if (!normalMap)
						{
							printf("[ObjLoader]: Failed to convert height map to normal map.\n");
							break;
						}

						Sampler* sampler = textureFactory.CreateSampler(normalMap);

						if (sampler)
						{
							phongMaterial->normalMap = sampler;
							microfacetMaterial->normalMap = sampler;
						}
						else
							printf("[ObjLoader]: Failed to create sampler for normal map.\n");
					}
					else
						printf("[ObjLoader]: Failed to load height map for material.\n");

					break;
				}


				// Alpha map
				if (line.compare(0, 5, "map_d") == 0)
				{
					Sampler* diffuseMap = phongMaterial->diffuseMap;

					if (diffuseMap == NULL)
					{
						printf("[ObjLoader]: Expected diffuse map before alpha map.\n");
						break;
					}

					// Create a filename relative to the filename of the material library
					std::string textureFile = GetRelativeFileName(line.substr(6), fileName);

					Texture* alphaTexture = NULL;
					if (textureFactory.GetAsset(textureFile, &alphaTexture))
					{
						Texture* mergedTexture = textureFactory.MergeAlphaChannel(diffuseMap->texture, alphaTexture);

						if (!mergedTexture)
						{
							printf("[ObjLoader]: Failed to merge alpha map with diffuse map.\n");
							break;
						}

						if (diffuseMap->texture->HasMipmaps())
							mergedTexture->GenerateMipMaps();

						diffuseMap->texture = mergedTexture;
						material->translucent = TRUE;
					}
					else
						printf("[ObjLoader]: Failed to load alpha map for material.\n");

					break;
				}


				//printf("[ObjLoader]: Invalid line in MTL \"%s\"\n", lineBuffer);
				break;
		}
	}
	
	reader.Close();

	printf("[ObjLoader]: Loaded %d materials\n", materialLib.size());
}

std::string ObjLoader::GetRelativeFileName(std::string fileName, const char* basePath)
{
	std::string textureFile(basePath);
	size_t idx = textureFile.find_last_of('/');
	textureFile.resize(idx != std::string::npos ? idx + 1 : 0);
	textureFile += fileName;

	return textureFile;
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

uint32_t ObjLoader::IndexReader::Parse(const char* buffer, uint32_t offset, uint32_t amount)
{
	vertexIdx = -1;
	NextVertex();

	ResetIndex();

	for (uint32_t charIdx = offset; charIdx < offset + amount + 1; ++charIdx)
	{
		const char c = buffer[charIdx];

		switch (c)
		{
		case '\0':
		case ' ':
			// Extra whitespace without an index, ignore it
			if (currentIdx == 0)
				break;

			// We finished the last index in a vertex, save it and move to the next one 
			SaveCurrentIdx();
			NextVertex();

			break;

		case '/':
			// Save the index, even if it is zero
			SaveCurrentIdx();
			break;

		case '#':
			// Comment incoming, skip rest of line
			charIdx = offset + amount;
			break;

		case '-':
			// Unary operator should be before any digits
			if (currentIdx != 0)
			{
				printf("[ObjLoader]: Unexpected character '-' in face definition.\n");
				break;
			}

			// Flip sign (yeah we support --10 == 10 as an index)
			sign *= -1;
			break;

		default:
			// Check if character is a digit
			if (c >= '0' && c <= '9')
			{
				// Add digit to current total index
				currentIdx = (currentIdx * 10) + ((c - '0') * sign);
				break;
			}

			printf("[ObjLoader]: Unkown character '%c' in face definition.\n", c);
			break;
		}
	}
	
	return vertexIdx;
}

ObjLoader::IndexReader::IndexReader()
{

}

void ObjLoader::IndexReader::SaveCurrentIdx()
{
	// Retrieve pointer to the VertexIndices element we want to save the index to
	int32_t* base = reinterpret_cast<int32_t*>(&vertexIndices[vertexIdx]);

	// Save index in the correct attribute
	*(base + attributeIdx) = currentIdx;

	// Reset index and move to the next attribute
	++attributeIdx;

	ResetIndex();
}

void ObjLoader::IndexReader::NextVertex()
{
	// Clear the next vertex indices struct
	++vertexIdx;
	attributeIdx = 0;

	if (vertexIdx < MAX_VERTICES)
		vertexIndices[vertexIdx] = VertexIndices();
}

void ObjLoader::IndexReader::ResetIndex()
{
	sign = 1;
	currentIdx = 0;
}