#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

varying vec2 vUv;
varying vec3 vNormal;

uniform mat4 viewMatrix;

uniform vec3 uDiffuseColor;
#ifdef USE_DIFFUSE_TEXTURE
    uniform bool enableDiffuse;
    uniform sampler2D tDiffuse;
#endif

#if MAX_DIR_LIGHTS> 0
    uniform vec3 directionalLightColor[ MAX_DIR_LIGHTS ];
    uniform vec3 directionalLightDirection[ MAX_DIR_LIGHTS ];
#endif

void main(void)
{
    gl_FragColor = vec4( 1.0 );
    vec3 normal = vNormal;
    
    // diffuse
    gl_FragColor.xyz *= uDiffuseColor;
    vec4 diffuseColor = gl_FragColor;
#ifdef USE_DIFFUSE_TEXTURE
    if( enableDiffuse ) {
        diffuseColor = texture2D( tDiffuse, vUv );
#ifdef GAMMA_INPUT
        vec4 texelColor = diffuseColor;
        texelColor.xyz *= texelColor.xyz;
        gl_FragColor = gl_FragColor * texelColor;
#else
        gl_FragColor = gl_FragColor * diffuseColor;
#endif
    }
#endif

    // lights
#if MAX_DIR_LIGHTS > 0
    vec3 dirDiffuse = vec3( 0.0 );
    vec3 dirSpecular = vec3( 0.0 );

    for( int i = 0; i < MAX_DIR_LIGHTS; i++ ) {

        vec4 lDirection = viewMatrix * vec4(directionalLightDirection[i], 0.0);
        vec3 dirVector = normalize( lDirection.xyz );

        // diffuse
        float dirDiffuseWeight = max( dot( normal, dirVector ), 0.0 );

        dirDiffuse += directionalLightColor[ i ] * dirDiffuseWeight;

        // specular
#if 0 //disable spec
        vec3 dirHalfVector = normalize( dirVector + viewPosition );
        float dirDotNormalHalf = max( dot( normal, dirHalfVector ), 0.0 );
        vec3 dirSpecularWeight = specularTex * max( pow( dirDotNormalHalf, specShininess + 0.0001 ), 0.0 );

#ifdef PHYSICALLY_BASED_SHADING
        // 2.0 => 2.0001 is hack to work around ANGLE bug
        float specularNormalization = ( specShininess + 2.0001 ) / 8.0;
        vec3 schlick = uSpecularColor + uSpecularColor * vec3( 1.0 - uSpecularColor.r ) * pow( 1.0 - dot( dirVector, dirHalfVector ), 5.0 );
        dirSpecular += schlick * directionalLightColor[ i ] * dirSpecularWeight * dirDiffuseWeight * specularNormalization;
#else
        dirSpecular += directionalLightColor[ i ] * uSpecularColor * dirSpecularWeight * dirDiffuseWeight;
#endif
#endif // disbale spec
    }

    //gl_FragColor.xyz = gl_FragColor.xyz * ( dirDiffuse + ambientLightColor * uAmbientColor) + dirSpecular;
    gl_FragColor.xyz = gl_FragColor.xyz * dirDiffuse + dirSpecular;
#endif

    // for debugging
    //gl_FragColor = vec4(vUv, 0.5, 1.0);
    //gl_FragColor = vec4(vNormal, 1.0);
}
