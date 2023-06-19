#version 450

in vec3 outColor;
in vec2 outTextureCoordinate;
in vec3 outPosition;
in vec3 outNormal;

out vec4 color;

//Propriedades da superficie
uniform float ka;
uniform float kd;
uniform float ks;
uniform float q;

//Propriedades da fonte de luz
uniform vec3 lightPos;
uniform vec3 lightColor;

// pixels da textura
uniform sampler2D tex_buffer;

//Posição da Camera
uniform vec3 cameraPos;

void main()
{
    //Cálculo da parcela de iluminação ambiente
	vec3 ambient = ka * lightColor;

	//Cálculo da parcela de iluminação difusa
	vec3 N = normalize(outNormal);
	vec3 L = normalize(lightPos - outPosition);
	float diff = max(dot(N,L),0.0);
	vec3 diffuse = kd * diff * lightColor;

	//Cálculo da parcela de iluminação especular
	vec3 V = normalize(cameraPos - outPosition);
	vec3 R = normalize(reflect(-L,N));
	float spec = max(dot(R,V),0.0);
	spec = pow(spec,q);
	vec3 specular = ks * spec * lightColor;

	vec3 result = (ambient + diffuse) * outColor + specular;

	color = vec4(result,1.0);
}