/***
 * Demonstrike Core
 */

#include "ModelInstance.h"
#include "WorldModel.h"
#include "MapTree.h"
#include "VMapDefinitions.h"

using G3D::Vector3;
using G3D::Ray;

namespace VMAP
{
	ModelInstance::ModelInstance(const ModelSpawn &spawn, WorldModel *model): ModelSpawn(spawn), iModel(model)
	{
		iInvRot = G3D::Matrix3::fromEulerAnglesZYX(G3D::pi()*iRot.y/180.f, G3D::pi()*iRot.x/180.f, G3D::pi()*iRot.z/180.f).inverse();
		iInvScale = 1.f/iScale;
	}

	bool ModelInstance::intersectRay(const G3D::Ray& pRay, float& pMaxDist, bool pStopAtFirstHit) const
	{
		if (!iModel)
		{
#ifdef VMAP_DEBUG
			DEBUG_LOG("ModelInstance", "<object not loaded>");
#endif
			return false;
		}
		float time = pRay.intersectionTime(iBound);
		if (time == G3D::inf())
		{
#ifdef VMAP_DEBUG
			DEBUG_LOG("ModelInstance", "Ray does not hit '%s'", name.c_str());
#endif
			return false;
		}
		// child bounds are defined in object space:
		Vector3 p = iInvRot * (pRay.origin() - iPos) * iInvScale;
		Ray modRay(p, iInvRot * pRay.direction());
		float distance = pMaxDist * iInvScale;
		bool hit = iModel->IntersectRay(modRay, distance, pStopAtFirstHit);
		if(hit)
		{
			distance *= iScale;
			pMaxDist = distance;
		}
		return hit;
	}

	void ModelInstance::intersectPoint(const G3D::Vector3& p, AreaInfo &info) const
	{
		if (!iModel)
		{
#ifdef VMAP_DEBUG
			DEBUG_LOG("ModelInstance", "<object not loaded>");
#endif
			return;
		}

		// M2 files don't contain area info, only WMO files
		if (flags & MOD_M2)
			return;
		if (!iBound.contains(p))
			return;
		// child bounds are defined in object space:
		Vector3 pModel = iInvRot * (p - iPos) * iInvScale;
		Vector3 zDirModel = iInvRot * Vector3(0.f, 0.f, -1.f);
		float zDist;
		if (iModel->IntersectPoint(pModel, zDirModel, zDist, info))
		{
			Vector3 modelGround = pModel + zDist * zDirModel;
			// Transform back to world space. Note that:
			// Mat * vec == vec * Mat.transpose()
			// and for rotation matrices: Mat.inverse() == Mat.transpose()
			float world_Z = ((modelGround * iInvRot) * iScale + iPos).z;
			if (info.ground_Z < world_Z)
			{
				info.ground_Z = world_Z;
				info.adtId = adtId;
			}
		}
	}

	bool ModelInstance::GetLocationInfo(const G3D::Vector3& p, LocationInfo &info) const
	{
		if (!iModel)
		{
#ifdef VMAP_DEBUG
			DEBUG_LOG("ModelInstance", "<object not loaded>");
#endif
			return false;
		}

		// M2 files don't contain area info, only WMO files
		if (flags & MOD_M2)
			return false;
		if (!iBound.contains(p))
			return false;
		// child bounds are defined in object space:
		Vector3 pModel = iInvRot * (p - iPos) * iInvScale;
		Vector3 zDirModel = iInvRot * Vector3(0.f, 0.f, -1.f);
		float zDist;
		if (iModel->GetLocationInfo(pModel, zDirModel, zDist, info))
		{
			Vector3 modelGround = pModel + zDist * zDirModel;
			// Transform back to world space. Note that:
			// Mat * vec == vec * Mat.transpose()
			// and for rotation matrices: Mat.inverse() == Mat.transpose()
			float world_Z = ((modelGround * iInvRot) * iScale + iPos).z;
			if (info.ground_Z < world_Z) // hm...could it be handled automatically with zDist at intersection?
			{
				info.ground_Z = world_Z;
				info.hitInstance = this;
				return true;
			}
		}
		return false;
	}

	bool ModelInstance::GetLiquidLevel(const G3D::Vector3& p, LocationInfo &info, float &liqHeight) const
	{
		// child bounds are defined in object space:
		Vector3 pModel = iInvRot * (p - iPos) * iInvScale;
		//Vector3 zDirModel = iInvRot * Vector3(0.f, 0.f, -1.f);
		float zDist;
		if (info.hitModel->GetLiquidLevel(pModel, zDist))
		{
			// calculate world height (zDist in model coords):
			// assume WMO not tilted (wouldn't make much sense anyway)
			liqHeight = zDist * iScale + iPos.z;
			return true;
		}
		return false;
	}

	bool ModelSpawn::readFromFile(FILE *rf, ModelSpawn &spawn)
	{
		uint32 check=0, nameLen;
		check += (uint32)fread(&spawn.flags, sizeof(uint32), 1, rf);
		// EoF?
		if (!check)
		{
			if (ferror(rf))
				ERROR_LOG("Error reading ModelSpawn!");
			return false;
		}
		check += (uint32)fread(&spawn.adtId, sizeof(uint16), 1, rf);
		check += (uint32)fread(&spawn.ID, sizeof(uint32), 1, rf);
		check += (uint32)fread(&spawn.iPos, sizeof(float), 3, rf);
		check += (uint32)fread(&spawn.iRot, sizeof(float), 3, rf);
		check += (uint32)fread(&spawn.iScale, sizeof(float), 1, rf);
		bool has_bound = ((spawn.flags & MOD_HAS_BOUND) ? true : false);
		if (has_bound) // only WMOs have bound in MPQ, only available after computation
		{
			Vector3 bLow, bHigh;
			check += (uint32)fread(&bLow, sizeof(float), 3, rf);
			check += (uint32)fread(&bHigh, sizeof(float), 3, rf);
			spawn.iBound = G3D::AABox(bLow, bHigh);
		}
		check += (uint32)fread(&nameLen, sizeof(uint32), 1, rf);
		if(check != (has_bound ? 17 : 11))
		{
			ERROR_LOG("Error reading ModelSpawn!");
			return false;
		}
		char nameBuff[500];
		if (nameLen>500) // file names should never be that long, must be file error
		{
			ERROR_LOG("Error reading ModelSpawn, file name too long!");
			return false;
		}
		check = (uint32)fread(nameBuff, sizeof(char), nameLen, rf);
		if (check != nameLen)
		{
			ERROR_LOG("Error reading name string of ModelSpawn!");
			return false;
		}
		spawn.name = std::string(nameBuff, nameLen);
		return true;
	}

	bool ModelSpawn::writeToFile(FILE *wf, const ModelSpawn &spawn)
	{
		uint32 check=0;
		check += (uint32)fwrite(&spawn.flags, sizeof(uint32), 1, wf);
		check += (uint32)fwrite(&spawn.adtId, sizeof(uint16), 1, wf);
		check += (uint32)fwrite(&spawn.ID, sizeof(uint32), 1, wf);
		check += (uint32)fwrite(&spawn.iPos, sizeof(float), 3, wf);
		check += (uint32)fwrite(&spawn.iRot, sizeof(float), 3, wf);
		check += (uint32)fwrite(&spawn.iScale, sizeof(float), 1, wf);
		bool has_bound = ((spawn.flags & MOD_HAS_BOUND) ? true : false);
		if(has_bound) // only WMOs have bound in MPQ, only available after computation
		{
			check += (uint32)fwrite(&spawn.iBound.low(), sizeof(float), 3, wf);
			check += (uint32)fwrite(&spawn.iBound.high(), sizeof(float), 3, wf);
		}
		uint32 nameLen = uint32(spawn.name.length());
		check += (uint32)fwrite(&nameLen, sizeof(uint32), 1, wf);
		if(check != (has_bound ? 17 : 11)) return false;
		check = (uint32)fwrite(spawn.name.c_str(), sizeof(char), nameLen, wf);
		if(check != nameLen) return false;
		return true;
	}

}
