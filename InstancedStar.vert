#version 150 core

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec3 pos;

out vec3 worldPosition;
out vec3 worldNormal;

uniform mat4 modelView;
uniform mat3 modelViewNormal;
uniform mat4 modelViewProjection;

in float scale;

mat4 trf = mat4(1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1);

void main()
{
	vec4 offsetPos = trf * vec4(vertexPosition, 1.0) + vec4((pos * scale), 0.0);

	worldNormal = normalize(mat3(trf) * vertexNormal);
	worldPosition = vec3(offsetPos);

	gl_Position = modelViewProjection * offsetPos;
}
