struct VS_SCREENINPUT
{
	float3 pos : POSITION;
	unsigned int index  : INDEX;
};

struct VS_SCREENOUTPUT
{
	float2	inTxr	: TEXCOORD;
    float4	pos     : SV_Position;         //position of the field
	unsigned int index		: INDEX;
};


int getCoordsIndex()
{
	for (int i = 0; i < g_dispCoordsSize; i++)
	{
		if (g_dispCoords[i].index == g_userInterface)
			return i;
	}
	return 0;
}
VS_SCREENOUTPUT VSRenderScreen(VS_SCREENINPUT input)
{
	VS_SCREENOUTPUT output = (VS_SCREENOUTPUT) 0;
	output.pos	 = float4(input.pos, 1);
	//Parse screen texture coords
	output.inTxr = float2((output.pos.x + 1) * 0.5, 1-(output.pos.y + 1) * 0.5);
	
	output.index = input.index;
	if (g_dispCoords[input.index].index != g_userInterface)
		output.index = getCoordsIndex();
	 return output;
}

float4 PSRenderScreenWithText(VS_SCREENOUTPUT In) : SV_Target
{
	float4 _temp = g_dispCoords[In.index].coords;

	float2 _begin = _temp.xy;
	float2 _end   = _temp.zw;
	float2 _coords= saturate(sign(In.inTxr - _begin)) * saturate(sign(_end - In.inTxr));
	float2 inTxr = In.inTxr;
	float _test = _coords.x * _coords.y;
	//Get texture areas in HUD which match with shaders user wants to be used
	_begin = g_colorCoords.xy;
	_end   = g_colorCoords.zw;
	_coords= saturate(sign(inTxr - _begin)) * saturate(sign(_end - inTxr)) * g_color;
	_test += _coords.x * _coords.y;
	_begin = g_texCoords.xy;
	_end   = g_texCoords.zw;
	_coords= saturate(sign(inTxr - _begin)) * saturate(sign(_end - inTxr)) * g_tex;
	_test += _coords.x * _coords.y;
	_begin = g_toonCoords.xy;
	_end   = g_toonCoords.zw;
	_coords= saturate(sign(inTxr - _begin)) * saturate(sign(_end - inTxr)) * g_toon;
	_test += _coords.x * _coords.y;
	_begin = g_diffSpecCoords.xy;
	_end   = g_diffSpecCoords.zw;
	_coords= saturate(sign(inTxr - _begin)) * saturate(sign(_end - inTxr)) * g_diffSpec;
	_test += _coords.x * _coords.y;
	_begin = g_bumpCoords.xy;
	_end   = g_bumpCoords.zw;
	_coords= saturate(sign(inTxr - _begin)) * saturate(sign(_end - inTxr)) * g_bump;
	_test += _coords.x * _coords.y;
	_begin = g_DOFCoords.xy;
	_end   = g_DOFCoords.zw;
	_coords= saturate(sign(inTxr - _begin)) * saturate(sign(_end - inTxr)) * g_DOF;
	_test += _coords.x * _coords.y;
	_begin = g_glowCoords.xy;
	_end   = g_glowCoords.zw;
	_coords= saturate(sign(inTxr - _begin)) * saturate(sign(_end - inTxr)) * g_glow;		
	_test += _coords.x * _coords.y;
	/*_begin = g_clickExpl.xy;
	_end   = g_clickExpl.zw;
	_coords = saturate(sign(inTxr - _begin)) * saturate(sign(_end - inTxr));// * (1 - g_released);		
	_test += _coords.x * _coords.y;*/
	//Compute final result
	_test  = saturate(_test);
	float4 colorHUD = txHUD.Sample(samLinear, In.inTxr * _test);
	//And return
	return colorHUD;
}

float4 PSGaussianBlur1(VS_SCREENOUTPUT In) : SV_Target
{
	float4 sumPar			 = txParticles.Sample	  (samLinear, In.inTxr);
	float4 sumFields		 = txFields.Sample		  (samLinear, In.inTxr);
	float4 colorParParam	 = 1 - txParticlesParam.Sample( samLinear, In.inTxr); //Depth texture for particles
	float4 colorTemp;
	//Used with field texture in order to not render field DOF
	float _centerOfInterest  = length(In.inTxr - 0.5);
	_centerOfInterest		 = pow(_centerOfInterest, 4);
	//Depth for particles texture
	float _depthPar			 = colorParParam.a * 0.05;
	float _depthField		 = _centerOfInterest;
	//Gaussian blur, first pass
	for (int i = 0; i < 12; i++)
	{
	  sumPar	+= txParticles.Sample (samLinear, In.inTxr + _depthPar   * 0.5 * samples[i] / 2);
	  sumFields	+= txFields   .Sample (samLinear, In.inTxr + _depthField * 0.5 * samples[i] / 2);
	}
	sumPar      /= 12;
	sumFields   /= 12;
	return lerp( sumFields * 0.5, sumPar, sumPar.a);
}
float4 PSGaussianBlur2(VS_SCREENOUTPUT In) : SV_Target
{
	float4 colorParParam	 = 1 - txParticlesParam.Sample(samLinear, In.inTxr);
	float4 colorFields		 = txFields.Sample		  (samLinear, In.inTxr);
	float4 sum				 = txScreen0.Sample		  (samLinear, In.inTxr);
	float4 colorTemp;
	//Used with field texture in order to not render field DOF
	float  _centerOfInterest = length(In.inTxr - 0.5);
	_centerOfInterest		 = pow   (_centerOfInterest, 4);
	_centerOfInterest		 = lerp  (_centerOfInterest, 0.1, g_glow); //Active only if user want a DOF
	//We need depth again
	float _depth			 = colorParParam.a * _centerOfInterest * 0.5;
	//Gaussian blur, second pass
	for (int i = 0; i < 12; i++)
	{
	  sum		+= txScreen0.Sample(samLinear, In.inTxr + _depth * 0.5 * samples[i] / 2);
	}
	sum /= 12;
	if(g_display)
		sum += PSRenderScreenWithText(In);
	return sum;
}

float4 PSRenderScreen(VS_SCREENOUTPUT In) : SV_Target
{
	float2 camTxCoords;
	float4 _camRight = matView[0];
	float4 _camUp = matView[1];
	float3 _camDir = float3(matPickInfos._m02, matPickInfos._m12, matPickInfos._m22);
	float3 _camPos = float3(matPickInfos._m00, matPickInfos._m10, matPickInfos._m20);
	//_pivot is used to scale the texture coordinates from the middle point on the screen
	float2 _pivot = 0.5;
	float _dotUp = dot(_camUp, float3(0.0, 1.0, 0.0));
	_camRight.w = matView._m30;

	//_distHorizontal and _distVertical are the distances to the center of the world
	//On the 2D plane perpendicular to the camera lookat
	float _viewDepth = dot(_camDir, -_camPos);
	float3 _displace = _camPos + _camDir * _viewDepth;
	float _distHorizontal = sign(_camRight.w) * length(_displace.xz);
	float _distVertical = _displace.y;

	

	/*We want to map a texture on a sphere representing camera rotations. To do so, inTxr.x
	will go along 0:1 meanwhile inTxr.y will go along range -1:1.
	As samEnv is mirrored on vertical, negative coordinates will read the texture backwards.
	This way, when the camera rotation on the x/z axis will be over 180 degrees, we will get the illusion
	the camera is upside down*/

	//Get the current texture coordinates
	camTxCoords = In.inTxr;
	//Scale them from range 0:1 to range 0:1/2PI
	//That way we only see a portion of the texture on screen.
	camTxCoords /= PI*2.0;
	_pivot /= PI*2.0;
	//We want to look at the center of the texture when the camera has no rotation.
	camTxCoords -= PI/2.0;
	_pivot -= PI / 2.0;
	

	if (_camDir.y > 0.0)
	{
		camTxCoords.y -= (1.0 - _dotUp)*0.5;
		_pivot.y -= (1.0 - _dotUp)*0.5;
	}
	else
	{
		camTxCoords.y += (1.0 - _dotUp)*0.5;
		_pivot.y += (1.0 - _dotUp)*0.5;
	}
	//Map the texture horizontal coordinates with the camera rotation on the 2D plane xz
	//atan2 will return the rotation on range 0:2PI so we scale it back to 1
	float _horizontal = atan2(-_camRight.x, _camRight.z) / (2 * PI);
	camTxCoords.x += _horizontal;
	_pivot.x += _horizontal;

	//With the camera rotation, we have to manage the camera translation in the 3D world
	//When camera is moved, we add to the texture coordinates its translation on
	//the 2D plane perpendicular to camera lookat
	camTxCoords.x -= _distHorizontal * 0.001;
	camTxCoords.y -= _distVertical   * 0.001;
	//When the camera moves backward or forward, scale the texture coordinates to get the impression
	//of depth in the 3D world
	camTxCoords += (camTxCoords - _pivot) * _viewDepth * 0.001;


	float4 colorPar			 = txParticles.Sample	  (samLinear, In.inTxr);
	float4 colorField		 = txFields   .Sample	  (samLinear, In.inTxr);
	float4 colorBackground	 = txEnv.Sample(samEnv, camTxCoords);
	float4 _lerp = lerp( colorField * 0.5, colorPar, colorPar.a);
	
	if(g_display)
		_lerp += PSRenderScreenWithText(In);
	_lerp = lerp(colorBackground, _lerp, _lerp.a);
	return _lerp;
}

technique10 RenderScreen
{
	pass P0
    {
		SetVertexShader( CompileShader( vs_4_0, VSRenderScreen() ) );
		SetGeometryShader(NULL);
        SetPixelShader( CompileShader( ps_4_0, PSGaussianBlur1() ) );
		SetDepthStencilState( DisableDepth, 0 );
		SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetRasterizerState(Rasterizer);
    }
    pass P1
    {
        SetVertexShader( CompileShader( vs_4_0, VSRenderScreen() ) );
		SetGeometryShader(NULL);
        SetPixelShader( CompileShader( ps_4_0, PSGaussianBlur2() ) );
		SetDepthStencilState( DisableDepth, 0 );
		SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetRasterizerState(Rasterizer);
    }
	pass P2
    {
        SetVertexShader( CompileShader( vs_4_0, VSRenderScreen() ) );
		SetGeometryShader(NULL);
        SetPixelShader( CompileShader( ps_4_0, PSRenderScreen() ) );
		SetDepthStencilState( DisableDepth, 0 );
		SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetRasterizerState(Rasterizer);
    }
}