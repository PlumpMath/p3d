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
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

varying vec2 vUv;
varying vec3 vNormal;
varying vec3 vViewPosition;

void main(void)
{
    vNormal = normalMatrix * aNormal;
#ifdef HAS_UV
    vUv = aUv;
#else
    vUv = vec2(0.0, 0.0);
#endif

    vec4 mvPosition = modelViewMatrix * vec4( aPosition, 1.0 );
    vViewPosition = -mvPosition.xyz;

    gl_Position = projectionMatrix * mvPosition;
}
