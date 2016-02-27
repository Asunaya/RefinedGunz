#ifndef MSTEERINGDRIVER_H
#define MSTEERINGDRIVER_H

class MSDObstacle{
};

class MSDCircleObstacle : public MSDObstacle{
};

class MSteeringDriver{
public:
	void AddObstacle(MSDObstacle* pObstacle);
};

#endif