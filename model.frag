#version 330 core

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float shininess;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirLight;

vec3 calcDirLight(DirLight light, Material mat, vec3 normal, vec3 viewDir);
float linearizeDepth(float depth);

void main() {
    vec3 result;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 norm = normalize(Normal);
    result += calcDirLight(dirLight, material, norm, viewDir);

    color = vec4(vec3(calcDirLight(dirLight, material, norm, viewDir)), 1.0f);
}

vec3 calcDirLight(DirLight light, Material mat, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(lightDir, normal), 0.0f);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), mat.shininess);

    vec3 ambient = light.ambient * vec3(texture(mat.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(mat.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(mat.texture_specular1, TexCoords));

    return (ambient + diffuse + specular);
}

float linearizeDepth(float depth) {
    float near = 0.1f;
    float far = 100.0f;
    float z = depth * 2.0f - 1.0f;
    return (2.0f * near) / (far + near - z * (far - near));
}