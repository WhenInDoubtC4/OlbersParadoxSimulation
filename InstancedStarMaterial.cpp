#include "InstancedStarMaterial.h"

InstancedStarMaterial::InstancedStarMaterial(Qt3DCore::QNode* parent) : Qt3DRender::QMaterial(parent)
  , _ambient(new Qt3DRender::QParameter("ka", QColor(212, 210, 165)))
  , _diffuse(new Qt3DRender::QParameter("kd", QColor(255, 253, 196)))
  , _specular(new Qt3DRender::QParameter("ks", QColor(1, 1, 1)))
  , _shininess(new Qt3DRender::QParameter("shininess", 150.f))
{
	addParameter(_ambient);
	addParameter(_diffuse);
	addParameter(_specular);
	addParameter(_shininess);

	auto effect = new Qt3DRender::QEffect();

	auto technique = new Qt3DRender::QTechnique();
	technique->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::OpenGL);
	technique->graphicsApiFilter()->setProfile(Qt3DRender::QGraphicsApiFilter::CoreProfile);
	technique->graphicsApiFilter()->setMajorVersion(3);
	technique->graphicsApiFilter()->setMinorVersion(1);
	auto filterKey = new Qt3DRender::QFilterKey();
	filterKey->setName(QStringLiteral("renderingStyle"));
	filterKey->setValue(QStringLiteral("forward"));
	technique->addFilterKey(filterKey);

	auto renderPass = new Qt3DRender::QRenderPass();
	auto shaderProgram = new Qt3DRender::QShaderProgram();
	shaderProgram->setVertexShaderCode(Qt3DRender::QShaderProgram::loadSource(QUrl("qrc:/InstancedStar.vert")));
	shaderProgram->setFragmentShaderCode(Qt3DRender::QShaderProgram::loadSource(QUrl("qrc:/InstancedStar.frag")));

	renderPass->setShaderProgram(shaderProgram);
	technique->addRenderPass(renderPass);
	effect->addTechnique(technique);
	setEffect(effect);
}
