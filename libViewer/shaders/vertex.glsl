#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

attribute vec3 aPosition;
attribute vec3 aNormal;
#ifdef HAS_UV
attribute vec2 aUv;
#endif

uniform mat4 uMVP;
uniform mat3 normalMatrix;

varying vec2 vUv;
varying vec3 vNormal;

void main(void)
{
    vNormal = normalMatrix * aNormal;
#ifdef HAS_UV
    vUv = aUv;
#else
    vUv = vec2(0.0, 0.0);
#endif

    gl_Position = uMVP * vec4(aPosition, 1.0);
}
