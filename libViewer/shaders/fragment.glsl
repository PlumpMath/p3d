#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

varying vec2 vUv;
varying vec3 vNormal;

void main(void)
{
    vec3 normal = normalize(vNormal);
    
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 diffuse = vec3(1.0, 1.0, 1.0)
      * max(0.0, dot(normal, lightDir));

    gl_FragColor = vec4(diffuse, 1.0);
}
