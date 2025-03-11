#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;  // The screen texture


void main() {
    vec4 color = texture(texture0, fragTexCoord); // Get screen pixel color
    const float epsilon = 0.01;

    if (
        abs(color.r - 55.0 / 255.0) < epsilon &&
        abs(color.g - 155.0 / 255.0) < epsilon &&
        abs(color.b - 255.0 / 255.0) < epsilon
    ) {
        color = vec4(fragTexCoord + 0.1, 0.5, 1);
    }


    finalColor = color;
}
