float4 main_fp(float2 iHealth : TEXCOORD0) : COLOR
{
  float4 oColor = float4(0, 0, 0, 1);
  
  // Fully transparant if below 3 pixels from the top
  if (iHealth.y > 3) oColor.a = 0;
  
  // If u <= 1, make the pixel green instead of red
  if (iHealth.x <= 1.0) oColor.y = 1;
  else oColor.x = 1;
  
  return oColor;
}