#include "InstancedStar.h"

InstancedStar::InstancedStar(Qt3DCore::QNode* parent) : Qt3DExtras::QSphereGeometry(parent)
  , _positionAttribute(new Qt3DRender::QAttribute(this))
  , _positionBuffer(new Qt3DRender::QBuffer(this))
  , _scaleAttribute(new Qt3DRender::QAttribute(this))
  , _scaleBuffer(new Qt3DRender::QBuffer(this))
{
	_positionAttribute->setAttributeType(Qt3DRender::QAttribute::AttributeType::VertexAttribute);
	_positionAttribute->setBuffer(_positionBuffer);
	_positionAttribute->setVertexBaseType(Qt3DRender::QAttribute::VertexBaseType::Float);
	_positionAttribute->setVertexSize(3);
	_positionAttribute->setName("pos");
	_positionAttribute->setDivisor(1);
	_positionAttribute->setByteStride(3 * sizeof(float));

	_scaleAttribute->setBuffer(_scaleBuffer);
	_scaleAttribute->setVertexBaseType(Qt3DRender::QAttribute::VertexBaseType::Float);
	_scaleAttribute->setVertexSize(1);
	_scaleAttribute->setName("scale");
	_scaleAttribute->setDivisor(1);
	_scaleAttribute->setByteStride(sizeof(float));

	addAttribute(_positionAttribute);
	setBoundingVolumePositionAttribute(_positionAttribute);

	addAttribute(_scaleAttribute);
}

void InstancedStar::setPoints(const QList<QVector3D>& points)
{
	_points = points;

	_positionBufferData.resize(points.size() * sizeof(QVector3D));
	auto vertexArray = reinterpret_cast<QVector3D*>(_positionBufferData.data());
	_scaleBufferData.resize(points.size() * sizeof(float));
	auto scaleArray = reinterpret_cast<float*>(_scaleBufferData.data());

	for (int i = 0; i < points.size(); i++)
	{
		vertexArray[i] = points[i];
		scaleArray[i] = 1.f;
	}

	_positionBuffer->setData(_positionBufferData);
	_positionAttribute->setCount(_points.size());

	_scaleBuffer->setData(_scaleBufferData);
	_scaleAttribute->setCount(_points.size());
}

void InstancedStar::addPoint(const QVector3D& point, const float& scale)
{
	_positionBufferData.resize((_points.size() + 1) * sizeof(QVector3D));
	auto vertexArray = reinterpret_cast<QVector3D*>(_positionBufferData.data());
	_scaleBufferData.resize((_points.size() + 1) * sizeof(float));
	auto scaleArray = reinterpret_cast<float*>(_scaleBufferData.data());

	vertexArray[_points.size()] = point;
	scaleArray[_points.size()] = scale;

	_positionBuffer->setData(_positionBufferData);
	_positionAttribute->setCount(_points.size() + 1);

	_scaleBuffer->setData(_scaleBufferData);
	_scaleAttribute->setCount(_points.size() + 1);

	_points << point;
}

int InstancedStar::getCount()
{
	return _points.size();
}
