#version 330 core
in vec3 ourColor;
in vec2 ourTexture;
out vec4 color;
uniform sampler2D selectTexture1;
uniform sampler2D selectTexture2;
void main()
{
  color = mix(texture(selectTexture1, ourTexture),
            texture(selectTexture2, vec2(ourTexture.x, 1.0 - ourTexture.y)), 0.5)
              * vec4(ourColor, 1.0f);
}
