#include "MeshProcessor.h"


namespace sol
{
	ModelProcessor::ModelProcessor()
	{
	}

	std::vector<Model> ModelProcessor::LoadModels(const std::vector<String>& modelPaths, const MetaProcessor& metaProcessor)
	{
		std::vector<Model> models;
		std::vector<String> modelNames;
		for (int32 i = 0; i < modelPaths.size(); i++) { modelNames.push_back(Util::StripFilePathAndExtentions(modelPaths.at(i))); }


		for (int32 i = 0; i < modelPaths.size(); i++)
		{
			String modelPath = modelPaths.at(i);
			String metaPath = metaProcessor.Find(modelNames.at(i));

			if (metaPath.GetLength() != 0)
			{
				ModelMetaFile metaFile = metaProcessor.ParseModelMetaFile(metaPath);
				Assert(metaFile.id.IsValid(), "");

				Model model = Model(modelPath, Util::StripFilePathAndExtentions(modelPath), metaFile);
				models.push_back(model);
			}
			else
			{
				LOG("No meta file for " << modelPath.GetCStr() << " something went very wrong !!");
			}
		}


		return models;
	}

	void Model::LoadModel(const String& path)
	{
		// read file via ASSIMP
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path.GetCStr(), aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace);
		// check for errors
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
			return;
		}

		// process ASSIMP's root node recursively
		ProcessNode(scene->mRootNode, scene);
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
		resultingMesh.hasColours = mesh->GetNumColorChannels() > 0;

		Assert(!resultingMesh.hasColours, "LOAD COLOURS");

		Mat3f scaleMatrix = ScaleCardinal(Mat3f(1.0f), Vec3f(metaData.scale));

		// walk through each of the mesh's vertices
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			FatVertex vertex = {};
			// we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
			Vec3f vector;
			// positions
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.position = vector;

			// normals
			if (mesh->HasNormals())
			{
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
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

				vertex.texCoords = vec;

				// tangent
				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				vertex.tangent = vector;

				// bitangent
				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
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

		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			resultingMesh.material.name = material->GetName().C_Str();

			//uint32 diffuseCount = material->GetTextureCount(aiTextureType_DIFFUSE);
			//uint32 specularCount = material->GetTextureCount(aiTextureType_SPECULAR);
			//uint32 ambientCount = material->GetTextureCount(aiTextureType_AMBIENT);
			//uint32 emssiveCount = material->GetTextureCount(aiTextureType_EMISSIVE);
			//uint32 normalCount = material->GetTextureCount(aiTextureType_NORMALS);
			//uint32 heightCount = material->GetTextureCount(aiTextureType_HEIGHT);
			//uint32 shininessCount = material->GetTextureCount(aiTextureType_SHININESS);
			//uint32 opacityCount = material->GetTextureCount(aiTextureType_OPACITY);
			//uint32 displacementCount = material->GetTextureCount(aiTextureType_DISPLACEMENT);
			//normalCount = material->GetTextureCount(aiTextureType_LIGHTMAP);
			//normalCount = material->GetTextureCount(aiTextureType_BASE_COLOR);
			//normalCount = material->GetTextureCount(aiTextureType_NORMAL_CAMERA);
			//normalCount = material->GetTextureCount(aiTextureType_EMISSION_COLOR);
			//normalCount = material->GetTextureCount(aiTextureType_METALNESS);
			//normalCount = material->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS);
			//normalCount = material->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION);
			//normalCount = material->GetTextureCount(aiTextureType_UNKNOWN);
		}

		return resultingMesh;
	}

	void Model::SaveBinaryData(BinaryFile* file) const
	{
		if (meshes.size() > 1)
			LOG("WARNING MESH SIZE MORE THAN ZERO: " << meshes.size() << " " << name.GetCStr());

		const Mesh& mesh = meshes[0];
		file->Write(metaData.id);
		file->Write(name);

		if (mesh.hasColours)
		{
			file->Write((uint8)VertexLayoutType::Value::PNTC);
			file->Write((uint32)mesh.vertices.size());
			for (const FatVertex& v : mesh.vertices)
			{
				file->Write(v.position);
				file->Write(v.normal);
				file->Write(v.texCoords);
				file->Write(v.colours);
			}
		}
		else
		{
			file->Write((uint8)VertexLayoutType::Value::PNT);
			file->Write((uint32)mesh.vertices.size());
			for (const FatVertex& v : mesh.vertices)
			{
				file->Write(v.position);
				file->Write(v.normal);
				file->Write(v.texCoords);
			}
		}


		file->Write(mesh.indices);
	}
}