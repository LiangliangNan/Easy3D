#version 150

in float clipped;

out vec4 FragColor;	// Ouput data

void main(void) {
        if (clipped > 0.0)
                discard;

        // write the depth into the RED component
        FragColor = vec4(gl_FragCoord.z, 0, 0, 1.0);
}
