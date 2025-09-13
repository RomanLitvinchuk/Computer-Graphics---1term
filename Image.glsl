vec2 getCoord() {
    return vec2(
    	texelFetch(iChannel0, ivec2(0, 0), 0).r,
        texelFetch(iChannel0, ivec2(1, 0), 0).r
    );
}

float hash(float n) {
    return fract(sin(n) * 43758.5453123);
}

float sdCircle(vec2 uv, float r, vec2 offset) {
    float x = uv.x - offset.x;
    float y = uv.y - offset.y;

    return length(vec2(x, y)) - r;
}


float sdRectangle(vec2 uv, vec2 size, vec2 offset) {

    float x = uv.x - offset.x;
    float y = uv.y - offset.y;

    vec2 d = abs(vec2(x, y)) - size;
    

    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
}

float sdCar(vec2 uv, vec2 carSize, vec2 offset, float wheelRadius) {

    float body = sdRectangle(uv, carSize, offset);
    

    vec2 wheelOffset1 = offset + vec2(-carSize.x * 0.9, -carSize.y - wheelRadius * 0.2);
    vec2 wheelOffset2 = offset + vec2( carSize.x * 0.9, -carSize.y - wheelRadius * 0.2);
    vec2 wheelOffset3 = offset + vec2(-carSize.x * 0.9,  carSize.y + wheelRadius * 0.2);
    vec2 wheelOffset4 = offset + vec2( carSize.x * 0.9,  carSize.y + wheelRadius * 0.2);
    

    float wheel1 = sdCircle(uv, wheelRadius, wheelOffset1);
    float wheel2 = sdCircle(uv, wheelRadius, wheelOffset2);
    float wheel3 = sdCircle(uv, wheelRadius, wheelOffset3);
    float wheel4 = sdCircle(uv, wheelRadius, wheelOffset4);
    

    float car = min(body, wheel1);
    car = min(car, wheel2);
    car = min(car, wheel3);
    car = min(car, wheel4);
    
    return car;
}

vec3 drawScene(vec2 uv, vec2 uvNormal) {
  vec3 col = vec3(0.0, 0.58, 0.58);
  if (uv.x < -0.6 || uv.x > 0.6) {
  col = vec3(0.0, 0.45, 0.0);
  }
  else {
      col = texture(iChannel1, uvNormal).rgb;
  }
  vec2 carPosition = getCoord() - vec2(0.0, 0.3);
  float car = sdCar(uv, vec2(0.05, 0.1), carPosition, 0.03);
  //float rock1 = sdCircle(uv, 0.07, vec2(-0.3, 0.3));
  float fallTime = 4.0;  
  float waitMax  = 1.0;   
  float t = iTime;
  float cycle = 0.0;
  float localTime = 0.0;
    
  
  for (int i = 0; i < 1200; i++) {
      float rnd = hash(cycle);
      float wait = rnd * waitMax;
      float cycleLen = fallTime + wait;
        
      if (t < cycleLen) {
          localTime = t;
          break;
      }
      t -= cycleLen;
      cycle += 1.0;
  }

  bool visible = localTime < fallTime;


  float y = 0.0;
  float yRect = 0.0;
  float x = 0.0; 
  if (visible) {
      float progress = localTime / fallTime;
      y = 0.7 - progress * 2.4;
      yRect = 0.7 - progress * 2.8;
  }
  
  float rock1 = sdCircle(uv-vec2(x,y), 0.07, vec2(-0.3, 0.3));
  float rock2 = sdCircle(uv-vec2(x,y), 0.07, vec2(0.3, 0));
  float rock3 = sdCircle(uv-vec2(x,y), 0.07, vec2(0.6, 0.5));
  float rock4 = sdCircle(uv-vec2(x,y), 0.07, vec2(-0.6, 0.7));
  float rectRock = sdRectangle(uv-vec2(x,yRect), vec2(0.08, 0.1), vec2(0.0, 0.7));
  if (visible && rock1 < 0.0) {
      col = vec3(1.0,0.45,0.0);
  }
  if (visible && rock2 < 0.0) {
      col = vec3(1.0,0.0,0.3);
  }
  if (visible && rectRock < 0.0) {
       col = vec3(0.0, 1.0, 0.0);
  }
  if (visible && rock3 < 0.0) {
      col = vec3(1.0, 0.0, 1.0);
  }
  if (visible && rock4 < 0.0) {
      col = vec3(0.0, 0.0, 1.0);
  }
  col = mix(vec3(0, 0, 1), col, step(0., car));
  col = mix(vec3(0.0, 1.0, 0.0), col, step(0., rectRock));
  //col = mix(vec3(1, 0, 0), col, step(0., rock1));
  
  return col;
}



void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
  vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;
  vec2 uvNormal = fragCoord/iResolution.xy;
  //vec3 col = texture(iChannel1, uvNormal).rgb;
  vec3 col = drawScene(uv, uvNormal);


  fragColor = vec4(col,1.0);
}