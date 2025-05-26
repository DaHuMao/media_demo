#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexture;

out vec3 Normal;
out vec3 FragPos;
out vec3 ourColor;
out vec2 ourTexture;
out vec3 LampPos;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lampPos;
void main()
{
  gl_Position = projection * view * model * vec4(aPos, 1.0f);
  FragPos = vec3(view * model * vec4(aPos, 1.0f));
  Normal = mat3(transpose(inverse(view * model))) * aNormal;
  LampPos = vec3(view * vec4(lampPos, 1.0));
  TexCoords = aTexture;
}
