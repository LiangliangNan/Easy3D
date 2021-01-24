#version 150

in float clipped;

void main(void) {
    if (clipped > 0.0)
        discard;
}
