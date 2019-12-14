#version 150

out vec4 FragColor;	// Ouput data


void main(void) {
        // write the depth into the RED component
        FragColor = vec4(gl_FragCoord.z, 0, 0, 1.0);
}
