#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

varying vec2 vUv;
varying vec3 vNormal;

uniform vec3 uDiffuseColor;
#ifdef USE_DIFFUSE_TEXTURE
    uniform bool enableDiffuse;
    uniform sampler2D tDiffuse;
#endif


void main(void)
{
    vec3 normal = normalize(vNormal);
    
    vec3 lightDir1 = normalize(vec3(1.0, 1.0, 1.0));
    vec3 lightDir2 = normalize(vec3(-1.0, -1.0, -1.0));
    vec3 diffColor = uDiffuseColor;
#ifdef USE_DIFFUSE_TEXTURE
    if(enableDiffuse)
    {
        diffColor *= texture2D(tDiffuse, vUv);
    }
#endif
    vec3 diffuse = diffColor * max(0.0, dot(normal, lightDir1));
    diffuse += 0.2 * diffColor * max(0.0, dot(normal, lightDir2));

    gl_FragColor = vec4(diffuse, 1.0);

    // for debugging uvs
    //gl_FragColor = vec4(vUv, 0.5, 1.0);
}
