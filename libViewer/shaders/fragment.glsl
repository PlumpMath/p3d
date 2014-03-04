#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

varying vec2 vUv;
varying vec3 vNormal;

uniform vec3 uDiffuse;

void main(void)
{
    vec3 normal = normalize(vNormal);
    
    vec3 lightDir1 = normalize(vec3(1.0, 1.0, 1.0));
    vec3 lightDir2 = normalize(vec3(-1.0, -1.0, -1.0));
    vec3 diffuse = uDiffuse * max(0.0, dot(normal, lightDir1));
    diffuse += 0.2 * uDiffuse * max(0.0, dot(normal, lightDir2));

    gl_FragColor = vec4(diffuse, 1.0);
}
