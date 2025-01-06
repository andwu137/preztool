#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

struct Light {
    vec2 pos;
    float inner;
    float outer;
};

uniform Light flashlight;

void main() {
    float d = distance(vec2(gl_FragCoord.x, gl_FragCoord.y), flashlight.pos);
    float alpha = 1.0;

    if (d > flashlight.outer) {
        alpha = 1.0;
    } else if (d < flashlight.inner) {
        alpha = 0.0;
    } else {
        alpha = (d - flashlight.inner) / (flashlight.outer - flashlight.inner);
    }

    finalColor = vec4(0, 0, 0, alpha);
}
