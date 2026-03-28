#version 330 core
out vec4 FragColor;
in vec3 localPos; 

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183); // 1/2PI, 1/PI

vec2 SampleEquirectangularMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
	uv.y = 1.0 - uv.y;
    return uv;
}

void main() {		
    vec2 uv = SampleEquirectangularMap(normalize(localPos));
    vec3 color = texture(equirectangularMap, uv).rgb;
    
	FragColor = vec4(color, 1.0);
    //FragColor = vec4(normalize(localPos) * 0.5 + 0.5, 1.0);
}