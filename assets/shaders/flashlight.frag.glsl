#version 330

out vec4 finalColor;

struct light {
    vec2 pos;
    vec3 color;
    float inner;
    float innerAlpha;
    float outer;
    float outerAlpha;
};

uniform light flashlight;

void main() {
    float d = distance(gl_FragCoord.xy, flashlight.pos);
    float alpha = flashlight.outerAlpha;

    if (d > flashlight.outer) {
        alpha = flashlight.outerAlpha;
    } else if (d < flashlight.inner) {
        alpha = flashlight.innerAlpha;
    } else {
        alpha = mix(flashlight.innerAlpha,
                    flashlight.outerAlpha,
                    (d - flashlight.inner) / (flashlight.outer - flashlight.inner));
    }

    finalColor = vec4(flashlight.color, alpha);
}
