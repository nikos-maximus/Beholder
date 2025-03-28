#ifndef VOXEL_H
#define VOXEL_H

////////////////////////////////////////////////////////////

class Mesh;

class VoxelSpace
{
public:

	VoxelSpace(const unsigned int xdim,const unsigned int ydim,const unsigned int zdim);
	~VoxelSpace();
	void SetElement(const int value,const unsigned int xpos,const unsigned int ypos,const unsigned int zpos);
	int GetElement(const unsigned int xpos,const unsigned int ypos,const unsigned int zpos);
	
	static VoxelSpace* BuildVoxelSpaceFromMesh(Mesh* mesh,const unsigned int detail = 10);
	//detail stands for number of voxel objects per 3D space unit

protected:

	int* voxelMatrix;
	unsigned int _xdim,_ydim,_zdim,_zoffset;

private:
};

////////////////////////////////////////////////////////////

#endif
