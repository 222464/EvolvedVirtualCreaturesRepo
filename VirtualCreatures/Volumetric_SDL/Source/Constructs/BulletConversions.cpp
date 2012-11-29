#include <Constructs/BulletConversions.h>

btVector3 bt(const Vec3f &vec)
{
	return btVector3(vec.x, vec.y, vec.z);
}

btQuaternion bt(const Quaternion &quat)
{
	return btQuaternion(quat.x, quat.y, quat.z, quat.w);
}

Vec3f cons(const btVector3 &bt)
{
	return Vec3f(bt.getX(), bt.getY(), bt.getZ());
}

Quaternion cons(const btQuaternion &bt)
{
	return Quaternion(bt.getW(), bt.getX(), bt.getY(), bt.getZ());
}