#version 330 core

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform vec2 uResolution;
uniform float uZoom;
uniform vec2 uOffset;

void main()
{
    TexCoord = aTexCoord;
    
    // Transform position for zoom and pan
    vec2 position = aPosition * uZoom + uOffset;
    
    // Convert to normalized device coordinates
    vec2 ndc = (position / uResolution) * 2.0 - 1.0;
    ndc.y *= -1.0; // Flip Y for OpenGL
    
    gl_Position = vec4(ndc, 0.0, 1.0);
}
