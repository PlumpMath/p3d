#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

attribute vec3 aPosition;
attribute vec3 aNormal;

uniform mat4 uMVP;

varying vec2 vUv;
varying vec3 vNormal;

void main(void)
{
    vNormal = aNormal;
    vUv = vec2(0.0, 0.0);
    gl_Position = uMVP * vec4(aPosition, 1.0);
}
