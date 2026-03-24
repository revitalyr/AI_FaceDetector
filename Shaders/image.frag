#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform float uExposure;
uniform float uContrast;
uniform float uGamma;
uniform bool uGrayscale;
uniform bool uShowOriginal;
uniform vec2 uMousePos;
uniform bool uShowPixelInfo;

// Helper functions for color processing
vec3 adjustExposure(vec3 color, float exposure) {
    return color * pow(2.0, exposure);
}

vec3 adjustContrast(vec3 color, float contrast) {
    return (color - 0.5) * contrast + 0.5;
}

vec3 adjustGamma(vec3 color, float gamma) {
    return pow(color, vec3(1.0 / gamma));
}

vec3 toGrayscale(vec3 color) {
    return vec3(dot(color, vec3(0.299, 0.587, 0.114)));
}

// AI Detail Enhancement (simulated)
vec3 enhanceDetails(vec3 color, vec2 texCoord) {
    vec2 texelSize = 1.0 / textureSize(uTexture, 0);
    
    // Sample neighboring pixels for edge detection
    vec3 center = texture(uTexture, texCoord).rgb;
    vec3 left   = texture(uTexture, texCoord + vec2(-texelSize.x, 0.0)).rgb;
    vec3 right  = texture(uTexture, texCoord + vec2(texelSize.x, 0.0)).rgb;
    vec3 top    = texture(uTexture, texCoord + vec2(0.0, -texelSize.y)).rgb;
    vec3 bottom = texture(uTexture, texCoord + vec2(0.0, texelSize.y)).rgb;
    
    // Simple edge detection using Sobel-like kernel
    vec3 gx = -left + right;
    vec3 gy = -top + bottom;
    vec3 edge = sqrt(gx * gx + gy * gy);
    
    // Mix original with edge-enhanced version
    float enhancement = 0.3;
    return color + edge * enhancement;
}

void main()
{
    vec4 originalColor = texture(uTexture, TexCoord);
    vec3 color = originalColor.rgb;
    
    if (!uShowOriginal) {
        // Apply image processing
        color = adjustExposure(color, uExposure);
        color = adjustContrast(color, uContrast);
        color = adjustGamma(color, uGamma);
        
        // Apply AI detail enhancement
        color = enhanceDetails(color, TexCoord);
        
        if (uGrayscale) {
            color = toGrayscale(color);
        }
    }
    
    // Show pixel info overlay near mouse
    if (uShowPixelInfo && distance(TexCoord, uMousePos) < 0.05) {
        // Create a small info box
        vec2 boxCoord = (TexCoord - uMousePos) * 20.0;
        if (abs(boxCoord.x) < 1.0 && abs(boxCoord.y) < 0.5) {
            color = mix(color, vec3(0.0), 0.7); // Dark background
        }
    }
    
    FragColor = vec4(color, originalColor.a);
}
