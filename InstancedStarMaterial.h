#pragma once

#include <QObject>
#include <QColor>
#include <QUrl>

#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QFilterKey>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QShaderProgram>

class InstancedStarMaterial : public Qt3DRender::QMaterial
{
	Q_OBJECT
public:
	InstancedStarMaterial(Qt3DCore::QNode* parent = nullptr);

private:
	Qt3DRender::QParameter* _ambient = nullptr;
	Qt3DRender::QParameter* _diffuse = nullptr;
	Qt3DRender::QParameter* _specular = nullptr;
	Qt3DRender::QParameter* _shininess = nullptr;
};

