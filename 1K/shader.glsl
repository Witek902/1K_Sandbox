#version 330 // TODO remove in final

uint seed;

float rand()
{
    seed ^= (seed << 13U);
    seed ^= (seed >> 17U);
    seed ^= (seed << 5U);
    // TODO why it doesn't work ?!
    //return uintBitsToFloat((seed & 0x007fffffU) | 0x3f800000U) - 1.0;
    return float(seed % 10000U) / 10000.0;
}

void main()
{
    vec2 coord = gl_FragCoord.xy;
    seed = (uint(coord.x) << 16U) + uint(coord.y);

    gl_FragColor = vec4(rand());
}