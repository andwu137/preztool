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
    vec3 color = flashlight.color;

    if (d > flashlight.outer) {
        alpha = flashlight.outerAlpha;
    } else if (d < flashlight.inner) {
        alpha = flashlight.innerAlpha;
    } else {
        float strength = (d - flashlight.inner) / (flashlight.outer - flashlight.inner);
        float strengthSquared = strength * strength; // TODO(andrew): look into making this exp?
        alpha = mix(flashlight.innerAlpha, flashlight.outerAlpha, strengthSquared);
    }

    finalColor = vec4(color, alpha);
}
