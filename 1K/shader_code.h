/* File generated with Shader Minifier 1.1.4
 * http://www.ctrl-alt-test.fr
 */
#ifndef SHADER_CODE_H_
# define SHADER_CODE_H_

const char *shader_glsl =
 "#version 330\n"
 "uint g;"
 "float t()"
 "{"
   "return g^=g<<13U,g^=g>>17U,g^=g<<5U,float(g%10000U)/10000.;"
 "}"
 "void main()"
 "{"
   "vec2 r=gl_FragCoord.xy;"
   "g=(uint(r.x)<<16U)+uint(r.y);"
   "gl_FragColor=vec4(t());"
 "}";

#endif // SHADER_CODE_H_
