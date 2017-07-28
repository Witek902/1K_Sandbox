uint seed;

float rand()
{
    seed ^= seed << 1U;
    seed ^= seed >> 3U;
    seed ^= seed << 10U;
    //return uintBitsToFloat((seed >> 9U) | 0x3f800000U) - 1.0;
    return float(seed % 10000) / 10000.0;
}

vec2 boxes(vec3 p)
{
    vec3 c = vec3(6.0, 0, 6.0);
    vec3 q = mod(p, c) - 0.5*c;
    return vec2(length(max(abs(q) - vec3(1.0, 1.0, 1.0), 0.0)), 3.0);
}

vec2 plane(vec3 p)
{
    return vec2(p.y + 1.0, 50.0);
}

vec2 opUnion(vec2 d1, vec2 d2)
{
    return d1.x < d2.x ? d1 : d2;
}

// distance function to the Menger Sponge
vec2 scene(vec3 p)
{
    vec2 r = plane(p);
    r = opUnion(r, boxes(p));
    return r;
}

void main()
{
    vec3 finalColor = vec3(0.0);
    vec2 eps = vec2(0.0001, 0.0);
    vec2 coord = gl_FragCoord.xy;
    seed = 1000U * uint(coord.x) + uint(coord.y);

    vec2 resolution = vec2(1280.0, 720.0);
    vec2 q = coord / resolution;

    for (int it = 0; it < 100; it++)
    {
        vec3 color = 1.0; // recursive color depth
        vec3 ro = vec3(0.0, 3, -8.0); // ray origin

        // ray direction
        vec3 rd = vec3((coord + 2.0 * vec2(rand(), rand())) / resolution.y - vec2(0.889, 0.5), 1.0);

        // Depth of Field
        vec3 pos = ro + 1.5 * rd;
        ro.xy += vec2(rand(), rand()) * 0.001;
        rd = normalize(pos - ro);

        for (int j = 0; j < 3; j++) // path tracing
        {
            float maxt = 1000.0, t = 0.001, m = -1.0;

            // raymarching algorithm
            for (int i = 0; i < 500; i++)
            {
                vec2 res = scene(pos=(ro+rd*t));
                if (abs(res.x) < eps.x || t > maxt) break;
                t += res.x;
                m = res.y;

                //float h = scene(pos=(ro+rd*t));
                //t += h;
                //if (t > maxt || abs(h) < eps.x) break;
            }

            vec3 skyColor = vec3(0.5, 0.8, 1.2) + vec3(5.0, 1.0, 0.0) * pow(max(0.0, dot(rd, vec3(-0.3, 0.1, 0.8))), 10.0);
    
            // we hit sky
            if (t >= maxt)
            {
                finalColor += color * skyColor;
                break;
            }

            // calculate normal vector (position derivative)
            vec3 normal = normalize(vec3(scene(pos + eps.xyy).x - scene(pos - eps.xyy).x,
                                         scene(pos + eps.yxy).x - scene(pos - eps.yxy).x,
                                         scene(pos + eps.yyx).x - scene(pos - eps.yyx).x));


            ro = pos + 0.001 * normal; // secondary ray origin
            rd = normalize(vec3(rand(), rand(), rand()) * 2.0 - 1.0);
            rd = dot(rd, normal) < 0.0 ? -rd : rd;
            color *= dot(rd, normal);

            vec3 materialColor = 0.5 + 0.5 * sin(vec3(0.15, 0.08, 0.1) * (m - 1.0));
            color *= materialColor;

            // fog
            color = lerp(skyColor, color, exp(-0.001 * t));
        }
    }

    // gamma correction
    finalColor *= 3.0; // exposure
    finalColor = sqrt(finalColor / 100.0);
    finalColor = 1.0 - exp(-finalColor);
  
    gl_FragColor = vec4(finalColor, 1.0);
    //gl_FragColor = vec4(finalColor * (0.8 + 0.2 * pow(16.0 * q.x * q.y * (1.0 - q.x) * (1.0 - q.y), 0.2)), 1.0);
}