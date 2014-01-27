#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

varying vec2 vUv;
varying vec3 vNormal;

struct lightSource
{
  vec4 position;
  vec4 diffuse;
};

lightSource light0 = lightSource (
    vec4(1.0, 1.0, 1.0, 0.0),
    vec4(1.0, 1.0, 1.0, 1.0)
);

void main(void)
{
    vec3 normal = normalize(vNormal);
    vec3 lightDir = normalize(vec3(light0.position));

    vec3 diffuse = vec3(light0.diffuse)
      * max(0.0, dot(normal, lightDir));

    gl_FragColor = vec4(diffuse, 1.0);

    //gl_FragColor = vec4 ( vNormal * 0.5 + vec3(0.5, 0.5, 0.5), 1.0 );
}
