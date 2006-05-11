#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <fstream>
#include <cassert>

#include "md5.hpp"

using namespace KlayGE;
using namespace std;


#pragma pack(push, 1)

struct
{
	uint8_t		infoLength;
	uint8_t		colorMapType;
	uint8_t		imageTypeCode;

	int16_t		colorMapEntry;
	int16_t		colorMapLength;
	uint8_t		colorMapBits;

	int16_t		leftbottomX;
	int16_t		leftbottomY;

	int16_t		width;
	int16_t		height;

	uint8_t		pixelSize;
	uint8_t		imageDescriptor;
} TGAHeader;

#pragma pack(pop)


struct Weight
{
	KlayGE::uint8_t joint;
	float weight;
};

bool WeightCmp(const Weight& lhs, const Weight& rhs)
{
	return lhs.weight > rhs.weight;
}

boost::shared_ptr<MD5SkinnedModel> LoadModel(const std::string& fileName)
{
	boost::shared_ptr<MD5SkinnedModel> model(new MD5SkinnedModel);

	FILE* fp = fopen(fileName.c_str(), "r");
	assert(fp != NULL);

	int version;
	fscanf(fp, "MD5Version %d\n\n", &version);
	char commandline[256];
	fgets(commandline, sizeof(commandline), fp);
	fscanf(fp, "\n\n");

	int numbones;
	fscanf(fp, "numbones %d\n", &numbones);
	std::vector<Joint> joints;
	joints.reserve(numbones);

	for (short i = 0; i < numbones; ++ i)
	{
		Joint joint;

		int boneindex;
		char name[64];
		Matrix4 bind_mat = Matrix4::Identity();
		fscanf(fp, "bone %d {\n name \"%s\nbindpos %f %f %f\nbindmat %f %f %f %f %f %f %f %f %f\n", &boneindex,
			name, &joint.bind_pos.x(), &joint.bind_pos.y(), &joint.bind_pos.z(),
			&bind_mat(0, 0), &bind_mat(0, 1), &bind_mat(0, 2),
			&bind_mat(1, 0), &bind_mat(1, 1), &bind_mat(1, 2),
			&bind_mat(2, 0), &bind_mat(2, 1), &bind_mat(2, 2));
		joint.bind_quat = MathLib::ToQuaternion(bind_mat);
		Matrix4 origin_mat = bind_mat;
		origin_mat *= MathLib::Translation(joint.bind_pos);
		Matrix4 inverse_origin_mat = MathLib::Inverse(origin_mat);
		joint.inverse_origin_quat = MathLib::ToQuaternion(inverse_origin_mat);
		joint.inverse_origin_pos = Vector3(inverse_origin_mat(3, 0), inverse_origin_mat(3, 1), inverse_origin_mat(3, 2));

		joint.name = name;

		char parent[64] = { '\0' };
		fscanf(fp, "parent \"%s\n", parent);
		if (parent[0] != '\0')
		{
			for (short j = 0; j < i; ++ j)
			{
				if (joints[j].name == parent)
				{
					joint.parent = j;
				}
			}
		}
		else
		{
			joint.parent = -1;
		}

		fscanf(fp, "}\n\n");

		joints.push_back(joint);
	}
	model->AssignJoints(joints.begin(), joints.end());


	int nummeshes;
	fscanf(fp, "nummeshes %d\n\n", &nummeshes);
	std::vector<boost::shared_ptr<MD5SkinnedMesh> > meshes;
	meshes.reserve(nummeshes);

	for (int i = 0; i < nummeshes; ++ i)
	{
		boost::shared_ptr<MD5SkinnedMesh> mesh(new MD5SkinnedMesh(model));

		int meshindex;
		fscanf(fp, "mesh %d {\n", &meshindex);
		char shader[64];
		fscanf(fp, "shader \"%s\n\n", shader);
		// È¥µôºóÒýºÅ
		int s = 0;
		while ((s < 64) && (shader[s] != 0))
		{
			++ s;
		}
		shader[s - 5] = 0;
		mesh->SetShaderName(shader);

		int numverts;
		fscanf(fp, "numverts %d\n", &numverts);
		std::vector<Vector3> xyzs(numverts);
		std::vector<std::vector<Vector2> > texs(1);
		texs[0].reserve(numverts);
		std::vector<std::pair<uint32_t, uint32_t> > weightIndexCounts;
		weightIndexCounts.reserve(numverts);
		for (int j = 0; j < numverts; ++ j)
		{
			uint32_t weightIndex;
			uint32_t weightCount;
			Vector2 tex;

			fscanf(fp, "vert %d %f %f %d %d\n",
				&meshindex,
				&tex.x(),
				&tex.y(),
				&weightIndex,
				&weightCount);

			texs[0].push_back(tex);
			weightIndexCounts.push_back(std::make_pair(weightIndex, weightCount));
		}
		mesh->AssignMultiTexs(texs.begin(), texs.end());

		fscanf(fp, "\n");

		int numtris;
		fscanf(fp, "numtris %d\n\n", &numtris);
		std::vector<uint16_t> indices;
		indices.reserve(numtris * 3);
		for (int j = 0; j < numtris; ++ j)
		{
			int triangleindex;
			int a, b, c;
			fscanf(fp, "tri %d %d %d %d\n", &triangleindex, &a, &b, &c);

			indices.push_back(b);
			indices.push_back(a);
			indices.push_back(c);
		}
		mesh->AssignIndices(indices.begin(), indices.end());

		fscanf(fp, "\n");

		int numweights;
		fscanf(fp, "numweights %d\n\n", &numweights);
		std::vector<Weight> weights;
		std::vector<Vector3> positions;
		weights.reserve(numweights);
		positions.reserve(numweights);
		for (int j = 0; j < numweights; ++ j)
		{
			Weight weight;
			Vector3 pos;

			int weightindex;
			int joint;
			fscanf(fp, "weight %d %d %f %f %f %f\n", &weightindex, &joint,
				&weight.weight, &pos.x(), &pos.y(), &pos.z());
			weight.joint = static_cast<uint8_t>(joint);

			weights.push_back(weight);
			positions.push_back(pos);
		}

		fscanf(fp, "}\n\n");


		std::vector<float> blend_weights;
		std::vector<KlayGE::uint8_t> blend_indices;
		for (int i = 0; i < numverts; ++ i)
		{
			std::vector<Weight> w(&weights[weightIndexCounts[i].first],
				&weights[weightIndexCounts[i].first] + weightIndexCounts[i].second);

			xyzs[i] = MathLib::TransformCoord(positions[weightIndexCounts[i].first],
				MathLib::ToMatrix(joints[w[0].joint].bind_quat) * MathLib::Translation(joints[w[0].joint].bind_pos));

			if (w.size() > 4)
			{
				std::nth_element(w.begin(), w.begin() + 4, w.end(), WeightCmp);
				w.resize(4);

				float totalweight(0);
				for (size_t j = 0; j < 4; ++ j)
				{
					totalweight += w[j].weight;
				}

				for (size_t j = 0; j < 4; ++ j)
				{
					w[j].weight /= totalweight;
				}
			}
			else
			{
				for (size_t j = w.size(); j < 4; ++ j)
				{
					Weight emptyWeight = { 0, 0 };
					w.push_back(emptyWeight);
				}
			}

			for (size_t j = 0; j < 4; ++ j)
			{
				blend_weights.push_back(w[j].weight);
				blend_indices.push_back(w[j].joint);
			}
		}
		mesh->AssignXYZs(xyzs.begin(), xyzs.end());
		mesh->AssignBlendWeights(blend_weights.begin(), blend_weights.end());
		mesh->AssignBlendIndices(blend_indices.begin(), blend_indices.end());

		meshes.push_back(mesh);
	}

	model->AssignMeshes(meshes.begin(), meshes.end());

	fclose(fp);

	return model;
}

struct Channel
{
	int		range[2];

	typedef std::vector<float> KeysType;
	KeysType keys;

	float Key(int frame) const
	{
		if (keys.empty())
		{
			return 0;
		}
		else
		{
			const int lframe(frame % keys.size());

			if ((lframe >= range[0]) && (lframe <= range[1]))
			{
				return keys[lframe];
			}
			else
			{
				return keys[0];
			}
		}
	}
};

boost::shared_ptr<KlayGE::KeyFramesType> LoadAnim(const std::string& fileName)
{
	boost::shared_ptr<KlayGE::KeyFramesType> anim(new KlayGE::KeyFramesType);

	FILE* fp = fopen(fileName.c_str(), "r");
	assert(fp != NULL);

	int version;
	fscanf(fp, "MD5Version %d\n\n", &version);
	char commandline[256];
	fgets(commandline, sizeof(commandline), fp);
	fscanf(fp, "\n\n");

	int numchannels;
	fscanf(fp, "numchannels %d\n", &numchannels);

	typedef KlayGE::MapVector<std::string, boost::array<Channel, 6> > ChannelsType;
	ChannelsType channels;

	for (int i = 0; i < numchannels; ++ i)
	{
		int channelindex;
		fscanf(fp, "channel %d {\n", &channelindex);

		char joint[64];
		fscanf(fp, "joint \"%s\n", joint);
		std::string jointName(joint);

		char attr[8];
		fscanf(fp, "attribute \"%s\n", attr);
		int s = 0;
		while ((s < 8) && (attr[s] != 0))
		{
			++ s;
		}
		attr[s - 1] = 0;

		static std::string attributeCode[6] = { "x", "y", "z", "roll", "pitch", "yaw" };
		int attribute(find(&attributeCode[0], &attributeCode[6], std::string(attr)) - &attributeCode[0]);

		Channel channel;

		float starttime, endtime, framerate;
		fscanf(fp, "starttime %f\n", &starttime);
		fscanf(fp, "endtime %f\n", &endtime);
		fscanf(fp, "framerate %f\n\n", &framerate);

		int strings;
		fscanf(fp, "strings %d\n\n", &strings);

		fscanf(fp, "range %d %d\n", &channel.range[0], &channel.range[1]);

		int keys;
		fscanf(fp, "keys %d\n", &keys);
		channel.keys.reserve(keys);
		for (int i = 0; i < keys; ++ i)
		{
			float key;
			fscanf(fp, "%f", &key);
			channel.keys.push_back(key);
		}

		fscanf(fp, "\n}\n\n");

		channels[jointName][attribute] = channel;
	}

	fclose(fp);

	for (ChannelsType::iterator iter = channels.begin(); iter != channels.end(); ++ iter)
	{
		KeyFrames kf;
		for (int i = 0; i < iter->second[0].range[1]; ++ i)
		{
			kf.bind_pos.push_back(Vector3(iter->second[0].Key(i), iter->second[1].Key(i), iter->second[2].Key(i)));

			Matrix4 matX = MathLib::RotationX(MathLib::Deg2Rad(iter->second[3].Key(i)));
			Matrix4 matY = MathLib::RotationY(MathLib::Deg2Rad(iter->second[4].Key(i)));
			Matrix4 matZ = MathLib::RotationZ(MathLib::Deg2Rad(iter->second[5].Key(i)));

			kf.bind_quat.push_back(MathLib::ToQuaternion(matX * matY * matZ));
		}
		anim->insert(std::make_pair(iter->first, kf));
	}

	return anim;
}
