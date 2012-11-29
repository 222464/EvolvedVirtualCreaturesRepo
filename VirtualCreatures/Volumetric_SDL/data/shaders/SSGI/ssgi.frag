/*
*CSSGI shader (Coherent Screen Space Global Illumination)
*This shader requires a depth pass and a normal map pass.
*/


#define NUM_SAMPLES 8

uniform sampler2D som;// Depth
uniform sampler2D normal;
uniform sampler2D color;

//noise producing function to eliminate banding (got it from someone else´s shader):
float rand(vec2 co){

        return 0.5+(fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453))*0.5;

}

void main()
{    
//calculate sampling rates:
float ratex = (1.0/800.0);
float ratey = (1.0/600.0);

//initialize occlusion sum and gi color:
float sum = 0.0;
vec3 fcolor = vec3(0,0,0);

//far and near clip planes:
float zFar = 80.0;
float zNear = 0.5;

//get depth at current pixel:
float prof = texture2D(som, gl_TexCoord[0].st).x;
//scale sample number with depth:
int samples = round(NUM_SAMPLES/(0.5+prof));
prof = zFar * zNear / (prof * (zFar - zNear) - zFar);  //linearize z sample

//obtain normal and color at current pixel:
vec3 norm = normalize(vec3(texture2D(normal,gl_TexCoord[0].st).xyz)*2.0-vec3(1.0));
vec3 dcolor1 = texture2D(color, gl_TexCoord[0].st);

int hf = samples/2;

//calculate kernel steps:
float incx = ratex*30;//gi radius
float incy = ratey*30;

float incx2 = ratex*8;//ao radius
float incy2 = ratey*8;

//do the actual calculations:
for(int i=-hf; i < hf; i++){
      for(int j=-hf; j < hf; j++){

      if (i != 0 || j!= 0) {
 
      vec2 coords = vec2(i*incx,j*incy)/prof;
      vec2 coords2 = vec2(i*incx2,j*incy2)/prof;

      float prof2 = texture2D(som,gl_TexCoord[0].st+coords*rand(gl_TexCoord[0])).x;
      prof2 = zFar * zNear / (prof2 * (zFar - zNear) - zFar);  //linearize z sample

      float prof2g = texture2D(som,gl_TexCoord[0].st+coords2*rand(gl_TexCoord[0])).x;
      prof2g = zFar * zNear / (prof2g * (zFar - zNear) - zFar);  //linearize z sample

      vec3 norm2g = normalize(vec3(texture2D(normal,gl_TexCoord[0].st+coords2*rand(gl_TexCoord[0])).xyz)*2.0-vec3(1.0)); 

      vec3 dcolor2 = texture2D(color, gl_TexCoord[0].st+coords*rand(gl_TexCoord[0]));

      //OCCLUSION:

      //calculate approximate pixel distance:
      vec3 dist2 = vec3(coords2,prof-prof2g);

      //calculate normal and sampling direction coherence:
      float coherence2 = dot(normalize(-coords2),normalize(vec2(norm2g.xy)));

      //if there is coherence, calculate occlusion:
      if (coherence2 > 0){
          float pformfactor2 = 0.5*((1.0-dot(norm,norm2g)))/(3.1416*pow(abs(length(dist2*2)),2.0)+0.5);//el 4: depthscale
          sum += clamp(pformfactor2*0.2,0.0,1.0);//ao intensity; 
      }

      //COLOR BLEEDING:

         if (length(dcolor2)>0.3){//color threshold
           vec3 norm2 = normalize(vec3(texture2D(normal,gl_TexCoord[0].st+coords*rand(gl_TexCoord[0])).xyz)*2.0-vec3(1.0)); 
           
           //calculate approximate pixel distance:
           vec3 dist = vec3(coords,abs(prof-prof2));

           //calculate normal and sampling direction coherence:
           float coherence = dot(normalize(-coords),normalize(vec2(norm2.xy)));

           //if there is coherence, calculate bleeding:
           if (coherence > 0){
              float pformfactor = ((1.0-dot(norm,norm2)))/(3.1416*pow(abs(length(dist*2)),2.0)+0.5);//el 4: depthscale
              fcolor += dcolor2*(clamp(pformfactor,0.0,1.0));
           }
        }
      
     }
   }
}

vec3 bleeding = (fcolor/samples)*0.5;
float occlusion = 1.0-(sum/samples);
gl_FragColor = vec4(vec3(dcolor1*occlusion+bleeding*0.5),1.0);
}
