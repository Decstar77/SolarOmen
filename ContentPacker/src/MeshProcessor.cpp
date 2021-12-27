#include "MeshProcessor.h"


namespace cm
{
	CString MetaFileProcessor::Find(const CString& name) const
	{
		for (int32 i = 0; i < metaNames.size(); i++)
		{
			if (metaNames.at(i) == name)
			{
				return metaPaths.at(i);
			}
		}

		return name;
	}

	ModelProcessor::ModelProcessor()
	{
	}

	std::vector<Model> ModelProcessor::LoadModels(const std::vector<CString>& modelPaths, const MetaFileProcessor& metaProcessor)
	{
		std::vector<CString> modelNames;
		for (int32 i = 0; i < modelPaths.size(); i++) { modelNames.push_back(Util::StripFilePathAndExtentions(modelPaths.at(i))); }

		CString modelPath = modelPaths.at(0);
		CString metaPath = metaProcessor.Find(modelNames.at(0));

		if (metaPath.GetLength() != 0)
		{
			TextFileReader reader;
			reader.Read(metaPath);


			Model model = Model(modelPath);
			const Mesh& mesh = model.meshes[0];

			BinaryFile file;
			for (int32 i = 0; i < mesh.positions.size(); i++)
			{
				file.Write(mesh.positions.at(i));
			}

			file.SaveToDisk("somedata.bin");

		}
		else
		{
			LOG("No meta file for " << modelPath.GetCStr() << " creating one; id =");
		}


		return std::vector<Model>();
	}

	void Model::LoadModel(const CString& path)
	{
		// read file via ASSIMP
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path.GetCStr(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		// check for errors
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
			return;
		}

		// process ASSIMP's root node recursively
		ProcessNode(scene->mRootNode, scene);

		name = Util::StripFilePathAndExtentions(path);
	}

	void Model::ProcessNode(aiNode* node, const aiScene* scene)
	{
		// process each mesh located at the current node
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			// the node object only contains indices to index the actual objects in the scene. 
			// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(ProcessMesh(mesh, scene));
		}
		// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene);
		}

	}

	Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		Mesh resultingMesh;

		// walk through each of the mesh's vertices
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex = {};
			// we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
			Vec3f vector;
			// positions
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			resultingMesh.positions.push_back(vector);
			vertex.position = vector;
			// normals
			if (mesh->HasNormals())
			{
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;

				resultingMesh.normals.push_back(vector);
				vertex.normal = vector;
			}
			else
			{
				resultingMesh.hasNormals = false;
			}

			// texture coordinates
			if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
			{
				Vec2f vec;

				// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
				// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				resultingMesh.uvs.push_back(vec);
				vertex.texCoords = vec;

				// tangent
				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				resultingMesh.tangets.push_back(vector);
				vertex.tangent = vector;

				// bitangent
				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
				resultingMesh.bitangets.push_back(vector);
				vertex.bitangent = vector;
			}
			else
			{
				vertex.texCoords = Vec2f(0.0f, 0.0f);
				resultingMesh.hasUVs = false;
				resultingMesh.hasTangets = false;
				resultingMesh.hastBitangets = false;
			}

			resultingMesh.vertices.push_back(vertex);
		}
		// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
		for (uint32 i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			// retrieve all indices of the face and store them in the indices vector
			for (uint32 j = 0; j < face.mNumIndices; j++)
			{
				resultingMesh.indices.push_back(face.mIndices[j]);
			}
		}

		return resultingMesh;
	}

}