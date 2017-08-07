uint seed;

float rand()
{
    seed ^= seed << 1;
    seed ^= seed >> 3;
    seed ^= seed << 10;
    return float(seed % 1000) / 1000;
}

void main()
{  
    vec3 v0, normal, pos, matColor, finalColor = 0, rayDir, color, rayOrigin, spheresColors[10];
    vec4 c = vec4(-101, 101, 100, 0), coord = gl_FragCoord, spheres[10];
    seed = uint(coord.y) + (uint(coord.x) << 10);
    spheres[0] = c.ywwz;
    spheres[1] = c.xwwz;
    spheres[2] = c.wywz;
    spheres[3] = c.wxwz;
    spheres[4] = c.wwyz;
    spheres[5] = c.wwxz;
    spheres[6] = vec4(0.6, -0.7, 0.2, 0.3);
    spheres[7] = vec4(-0.3, -0.6, 0.2, 0.4);
    spheres[8] = vec4(-0.6, -0.8, -0.7, 0.2);
    spheres[9] = vec4(0.5, -0.8, -0.8, 0.2);

    spheresColors[0] = vec3(0.8, 0.1, 0.1);
    spheresColors[1] = spheresColors[0].zzx;
    spheresColors[4] = spheresColors[0].zxz;
    spheresColors[2] = spheresColors[3] = 0.5;
    // spheresColors[5] = spheresColors[6] = spheresColors[7] = spheresColors[8] = spheresColors[9] = 0.9;

    for (int it = 0; it < 200; it++)
    {   
        float tmin, t0, t1, t2 = 6.28 * rand(), aberrationOffset = rand();
        color = 1;

        // Depth of Field (circle)
        rayOrigin = vec3(0, -0.4, -2.5);
        // TODO adjust resolution
        pos = rayOrigin + 2 * vec3((coord.xy + 2 * vec2(rand(), rand())) / 1080 - vec2(0.889, 0.5), 1.2 + aberrationOffset * 0.03);
        rayOrigin.xy += 0.1 * sqrt(rand()) * vec2(cos(t2), sin(t2)); // aperture
        rayDir = normalize(pos - rayOrigin);

        for (int j = 0; j < 10; j++) // path tracing
        {
            tmin = 10;
            for (int i = 0; i < 10; i++)
            {
                t2 = spheres[i].w;
                v0 = rayOrigin - spheres[i].xyz;
                t0 = dot(v0, rayDir);
                t1 = t0 * t0 - dot(v0, v0) + t2 * t2;
                if (t1 > 0)
                {
                    t0 = -t0 - sqrt(t1);
                    if (t0 > 0) if (tmin > t0)
                    {
                        tmin = t0;
                        pos = rayOrigin + rayDir * tmin;
                        normal = (pos - spheres[i].xyz) / t2;
                        matColor = i > 4 ? vec3(0.9) : spheresColors[i];
                    }
                }
            }

            rayOrigin = pos + normal / 500;

            // light
            if (abs(pos.x) < 0.3) if (abs(pos.z) < 0.3) if (pos.y > 0.9)
            {
                t1 = 3 * aberrationOffset - 1.5;
                finalColor += 80 * color * clamp(vec3(-t1, 1 - abs(t1), t1), 0, 1);
                matColor = 0; // break;
            }

            color *= matColor;
            

            // generate secondary ray
            if (matColor == 0.9) // reflection
            {
                rayDir = reflect(rayDir, normal);
            }
            else // diffusion
            {
                t1 = rand();
                t2 = 6.28 * rand();
                v0 = normalize(cross(normal, rayDir));
                rayDir = sqrt(t1) * (cos(t2) * cross(normal, v0) + sin(t2) * v0) + normal * sqrt(1 - t1);            
            }
        }
    }

    gl_FragColor = vec4(sqrt(1 - exp(-finalColor / 100)), 1);
}