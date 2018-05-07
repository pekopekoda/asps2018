#define FIELD_SELECTED 1
#define GOAL_SELECTED  2

struct VS_FIELDOUTPUT
{
	float	index				: INDEX;
    float4	sv_position         : SV_Position;         //position of the field
	float3  posCamSpace			: POSITION;
	float	intensity			: INTENSITY;
	float3	direction			: DIRECTION;
};

struct VS_FieldsInstanceIn
{
	float3	instancePos		 : POSITION1;
	float3	instanceNorm	 : NORMAL1;
	float2  texcoords		 : TEXCOORD1;
	float	index			 : INDEX;
	unsigned int isSelected  : ISSELECTED;
	float type : FIELDTYPE;
	float size : SIZE;
	float offset : OFFSET;
	float3 pos : POSITION;
	float3 goal : GOAL;
	float force : FORCE;
	float3 svPos : SV_Position;
	float4 color : FIELDCOLOR;
};
struct VS_FieldsInstanceOut
{
	float3 normal			: NORMAL1;
	float3 binormal			: BINORMAL;
	float3 tangent			: TANGENT;
	float3 pos			: POSITION;
	float4 svPos				: SV_Position;
	float2 texcoords		: TEXCOORD1;
};

struct PS_FIELDOUTPUT
{
	float4	position	: COLOR0;	//position of the field
	float4	goal		: COLOR1;
	float4	strength	: COLOR2;
	float4	typeSize    : COLOR3;
};


float GetGoalSize(float _fieldSize)
{
	return _fieldSize * 0.3;
}
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_FIELDPARAMETERSOUTPUT VSRenderFields(VS_FIELDPARAMETERSOUTPUT In)
{
	return In;
}
//--------------------------------------------------------------------------------------
VS_FIELDPARAMETERSOUTPUT VSRenderFieldsParameters(VS_FIELDPARAMETERSOUTPUT In)
{
	VS_FIELDPARAMETERSOUTPUT Out = (VS_FIELDPARAMETERSOUTPUT) 0;
	Out.index	   = In.index;
	Out.isSelected = In.isSelected;
	Out.type	   = In.type;
	Out.size	   = In.size;
	Out.offset	   = In.offset;
	Out.pos		   = In.pos;
	Out.goal	   = In.goal;
	Out.strength   = In.strength;
	//Place the fields datas along a 1d texture
	//To do so, get the field index and reduces it into the range 0:1
	//Store the result in position.x
	Out.position.x = In.index * 2;
	Out.position.x /= FIELDSTOTAL * g_fieldSizeUpdate;
	Out.position.x -= 0.998455; // ?
	Out.position.y = 1;
	Out.position.z = 0;
	Out.position.w = 1;
	Out.color	   = In.color;
	Out.dummy	   = In.dummy;
	//If field index is above the current max field number
	//Set its side to -1
	if(Out.index >= g_fieldNbr)
		Out.size = -1;
	else if (Out.size <= 0.0)
	{
		Out.size = 8.0;
		Out.pos = 0.0;
	}
	return Out;
}
//Get the cursor and camera position and direction
//And store them as a matrix
float3 GetPickWorldPos(float3 _pos)
{
	float3 _camPos;
	float3 _pickDir;
	float3 _camDir;
	float3 _camObjDir;
	float3 _pickWorldPos;
	float _projObjCam, _pickLength;

	_camPos = float3(matPickInfos._m00, matPickInfos._m10, matPickInfos._m20);
	_pickDir = float3(matPickInfos._m01, matPickInfos._m11, matPickInfos._m21);
	_camDir = float3(matPickInfos._m02, matPickInfos._m12, matPickInfos._m22);

	_camObjDir = _pos - _camPos;

	//projection of camToObj vector on view vector
	_projObjCam = dot(_camDir, normalize(_camObjDir));
	_projObjCam *= length(_camObjDir);

	//calcul intersection point between pick vector and plane
	_pickLength = _projObjCam / dot(_camDir, _pickDir);
	_pickLength = clamp(_pickLength, 0, _projObjCam);
	_pickWorldPos = _pickDir * _pickLength;

	//transforms cam space vector into world space point
	_pickWorldPos += _camPos;
	
	return _pickWorldPos;
}

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
VS_FIELDPARAMETERSOUTPUT GSPick(VS_FIELDPARAMETERSOUTPUT In, inout VS_FIELDPARAMETERSOUTPUT Out)
{
	float3 _cursorToField, _cursorToGoal, offset;
	float _goalSize;
	float3 _offset, _pos,  _goalPos;
	int _fieldSelected, _goalSelected;

	_goalSize		 = (In.type == PORTAL_TYPE)? In.size : GetGoalSize(In.size); //In case field is a PORTAL_TYPE, goal will be same size

	// At left mouse button release event, offset is set to 0
	_offset = In.offset;
	_pos = In.pos;

	_fieldSelected = In.isSelected == FIELD_SELECTED;
	//_fieldSelected *= !g_released;	// Test if left mouse button has been released at this frame
	_goalSelected  = In.isSelected == GOAL_SELECTED;
	//_goalSelected *= !g_released;	// Test if left mouse button has been released at this frame
	Out.isSelected = 0;
	float3 _fieldPWP = GetPickWorldPos(In.pos);
	_goalPos = In.pos + In.goal;
	float3 _goalPWP = GetPickWorldPos(_goalPos);

	if (g_clickEvent == 1)
	{
		_cursorToField = _fieldPWP - In.pos;
		_goalPos = In.pos + In.goal;
		_cursorToGoal  = _goalPWP - _goalPos;
		// Test if cursor is in a field and if right mouse has been pushed precisely at this frame
		_fieldSelected = sign(saturate(In.size - length(_cursorToField)));
		// Test if cursor is in a goal and if right mouse has been pushed precisely at this frame.
		// GOAL_SELECTED equals 2 to differenciate goal selection and field selection
		_goalSelected  = sign(saturate(_goalSize - length(_cursorToGoal)));
		_offset = (_fieldSelected) ? _cursorToField : _cursorToGoal;
	}
	

	if (_fieldSelected)
	{
		// If selected, updates position with cursor world space position
		_pos = _fieldPWP - _offset;
		//We don't want the goal to be moved along the field moves, so we test the field selected is not a bouncer
		if (In.type != BOUNCER_TYPE)
			// In case selection is not a goal, goal position is not affected by field deplacement.
			Out.goal -= (_pos - In.pos);
		Out.isSelected = FIELD_SELECTED;
	}
	else if (_goalSelected)// If selection is a goal ...
	{
		Out.goal = _goalPWP - In.pos - _offset;
		Out.isSelected = GOAL_SELECTED;
	}
	Out.offset = _offset;
	Out.pos = _pos;
	return Out;
}

float3 GSReinitStrength(float3 _strength, float _type, float _typeHasChanged)
{
	float3 _temp = lerp(float3(1,0.1,1), float3(1,5,1), saturate(_type));
	// In case field type has been changed by user, strength is reset with field type typical value
	return lerp(_strength, _temp, _typeHasChanged);
}

VS_FIELDPARAMETERSOUTPUT GSUpdateForceType(VS_FIELDPARAMETERSOUTPUT Out, VS_FIELDPARAMETERSOUTPUT In[2], bool _isNewType)
{
	if (Out.isSelected)
	{
		if (_isNewType)
			Out.strength = float3(1.0, 5.0, 1.0);
		else
		{
			Out.strength.x = clamp(In[0].strength.x + g_fieldCenterForceUpdate * 0.1, 0.0, 10.0);
			Out.strength.y = clamp(In[0].strength.y + g_fieldExtremityForceUpdate * 0.1, 0.0, 10.0);
			Out.strength.z = clamp(In[0].strength.z + g_fieldInterpolationForceUpdate * 0.1, 0.0, 10.0);
		}
	}
	return Out;
}

VS_FIELDPARAMETERSOUTPUT GSUpdatePortalType(VS_FIELDPARAMETERSOUTPUT Out, VS_FIELDPARAMETERSOUTPUT In[2])
{
	Out.strength = 0.0;
	return Out;
}

VS_FIELDPARAMETERSOUTPUT GSUpdateEmitterType(VS_FIELDPARAMETERSOUTPUT Out, VS_FIELDPARAMETERSOUTPUT In[2])
{
	Out.strength = 0.0;
	return Out;
}

VS_FIELDPARAMETERSOUTPUT GSUpdateBouncerType(line VS_FIELDPARAMETERSOUTPUT Out, VS_FIELDPARAMETERSOUTPUT In[2], bool _isNewType)
{
	if (Out.isSelected)
	{
		if (_isNewType)
			Out.strength = float3(1.0, 1.0, 1.0);
		else
		{
			Out.strength.x = clamp(In[0].strength.x + g_fieldCenterForceUpdate * 0.1, 0.0, 10.0);
			Out.strength.y = clamp(In[0].strength.y + g_fieldExtremityForceUpdate * 0.1, 0.0, 10.0);
		}
	}
	Out.goal.y = abs(Out.goal.y);
	Out.goal = normalize(Out.goal) * Out.size * 1.5;

	return Out;
}

[maxvertexcount(2)]
void GSUpdateFields(line VS_FIELDPARAMETERSOUTPUT In[2], inout LineStream<VS_FIELDPARAMETERSOUTPUT> PtStream )
{
	//float _tempSize;
	int _oldType;
	float3 _translate;
	VS_FIELDPARAMETERSOUTPUT Out = (VS_FIELDPARAMETERSOUTPUT) 0;
	Out.index			=		    In[0].index;
	if(g_time == 0) // First frame in program
	{
		Out.index			=		    In[0].index;
		Out.size			=			-1;
		Out.offset			=			0;	
		Out.position		=			In[0].position;
		Out.color			=			In[0].color;
		Out.dummy			=			In[0].dummy;
		Out.pos				=			In[0].pos;
		PtStream.Append(GSFieldsStart(Out));
		Out.index			=		    In[1].index;
		Out.position		=			In[1].position;
		PtStream.Append(Out);
		return;
	}

	Out.isSelected = In[0].isSelected;
	Out.goal = In[0].goal;
	Out.offset = In[0].offset;
	Out.pos = In[0].pos;
	if(!g_released)
		GSPick(In[0], Out);
	Out.position		=			In[0].position;
	Out.color			=			In[0].color;
	Out.dummy			=			In[0].dummy;																			      // Prevents potential division by 0. Subsequently deactivates field
	Out.size = In[0].size;
	Out.type = In[0].type;
	_oldType = In[0].type;
	if (Out.isSelected)
	{
		Out.size += g_fieldSizeUpdate * 0.1;
		Out.type = clamp(Out.type+ g_fieldTypeUpdate, EMITTER_TYPE, BOUNCER_TYPE);;

	}
	Out.strength = In[0].strength;
	if (Out.size <= 0.0)
		Out.isSelected = 0;

	else if(Out.type == FORCE_TYPE)
		Out = GSUpdateForceType(Out, In, _oldType != Out.type);
	else if(Out.type == PORTAL_TYPE)
		Out = GSUpdatePortalType(Out, In);
	else if (Out.type == EMITTER_TYPE)
		Out = GSUpdateEmitterType(Out, In);
	else if (Out.type == BOUNCER_TYPE)
		Out = GSUpdateBouncerType(Out, In, _oldType != Out.type);

	UpdateFieldsWithScenario(Out);
	Out.pos.xyz += g_fieldPositionUpdate * (Out.isSelected == FIELD_SELECTED) * 3.0;
	Out.goal += g_fieldPositionUpdate * (Out.isSelected == GOAL_SELECTED) * 3.0;
	PtStream.Append(Out);
	Out.index			=		    In[1].index;
	Out.position		=			In[1].position;
	PtStream.Append(Out);
}
//--------------------------------------------------------------------------------------
void GSPortalTypeAppearance(point VS_FIELDPARAMETERSOUTPUT In[1], inout TriangleStream<VS_FIELDOUTPUT> TriStream )
{
	VS_FIELDOUTPUT output = (VS_FIELDOUTPUT) 0;
	matrix mwvp = mul(mul(matWorld, matView), matProj);
	float totalSize = In[0].size;
	float size = 0.5;
	float precision = 12;
	float4 position0 = float4 (In[0].pos,1);
	bool _isExtremity = 0;
	float _tempSize;
	float3 _direction, _temp;
	if (In[0].size <= 0.0)
		return;
	// Draws PORTAL_TYPE
	for (precise float angle = -PI/(precision); angle <= PI*2.0; angle+= PI/(precision))
	{
		output.intensity = 0.1;
		output.direction = In[0].color.rgb;
		_direction = float3 (cos(angle), sin(angle), 0);
		output.sv_position = float4(_direction * In[0].size, 0); // New vertex's position
		if (!_isExtremity)
		{
			output.sv_position.xyz *= 0.5; // New vertex's position
			output.direction += saturate(In[0].isSelected);
		}
		else
		{
			output.sv_position = float4(_direction, 0)* In[0].size;// New vertex's position
		}
		output.sv_position += position0;
		output.sv_position = mul(output.sv_position, mwvp);

		output.posCamSpace = output.sv_position.xyz;
		output.index		= In[0].index;
		TriStream.Append (output);
		_isExtremity = !_isExtremity; //One time extreme vertex is drawn, and the other time center vertex is drawn. that's why _test is used.
	}
	TriStream.RestartStrip ();
}

void GSForceTypeAppearance(point VS_FIELDPARAMETERSOUTPUT In[1], inout TriangleStream<VS_FIELDOUTPUT> TriStream )
{
	VS_FIELDOUTPUT output = (VS_FIELDOUTPUT) 0;
	matrix mwvp = mul(mul(matWorld, matView), matProj);
	float totalSize		  = In[0].size;
	float precision		  = 12;
	float _powCosGoal;
	float4 position0	  = float4 (In[0].pos,1);
	bool _isExtremity	  = 0;
	float3 _direction;
	float _extrIntensity, _centerIntensity;


	//Returns which field opposite has stronger value (center or extreme)
	float _strongest = max(In[0].strength.x, In[0].strength.y);

	//Divides these values by the strongest to get a range (0:1) (Needed to transfer these values to fragment shader)
	_extrIntensity = In[0].strength.y / _strongest;
	_centerIntensity = In[0].strength.x / _strongest;
	//Draws field
	for (precise float angle = -(PI / (precision)); angle <= (PI * 2.0); angle += PI / (precision))
	{
		output.direction = In[0].color.rgb;
		if (!_isExtremity)
		{
			output.intensity = _centerIntensity;
			output.sv_position = 0;
			output.direction += saturate(In[0].isSelected);
		}
		else
		{
			_direction = float3 (cos(angle), sin(angle), 0);
			output.intensity = _extrIntensity;
			output.sv_position = float4(_direction, 0) * In[0].size;// New vertex's position
			//output.direction += (length(normalize(In[0].goal) - _direction)); //Blend it with field's personal color
			_powCosGoal = pow(saturate(dot(normalize(In[0].goal), _direction)), 5);
			output.direction += _powCosGoal;
			output.intensity = 0.2 + _powCosGoal;
		}
		output.sv_position += position0;
		output.sv_position = mul(output.sv_position, mwvp);
		//output.sv_position.y += 1 * _test;
		output.posCamSpace = output.sv_position.xyz;
		
		//output.intensity *= In[0].color.a;
		
		output.index = In[0].index;
		TriStream.Append(output);
		_isExtremity =!_isExtremity; //One time extreme vertex is drawn, and the other time center vertex is drawn. that's why _test is used.
	}
	TriStream.RestartStrip ();
}

void GSEmitterTypeAppearance(point VS_FIELDPARAMETERSOUTPUT In[1], inout TriangleStream<VS_FIELDOUTPUT> TriStream)
{
	VS_FIELDOUTPUT output = (VS_FIELDOUTPUT)0;
	matrix mwvp = mul(mul(matWorld, matView), matProj);
	float _size = In[0].size;
	float precision = 12;
	float4 position0 = float4 (In[0].pos, 1);
	bool _isExtremity = 0;
	float3 _direction;

	for (precise float angle = -PI / (precision); angle <= PI*2.0; angle += PI / (precision))
	{
		
		_direction = float3 (cos(angle), sin(angle), 0);
		if (!_isExtremity)
		{
			output.sv_position = float4(_direction * 0.7 * In[0].size, 0); // New vertex's position
			output.intensity = 0.7;
		}
		else
		{
			output.intensity = 1.0;
			output.sv_position = float4(_direction, 0) * In[0].size;// New vertex's position
		}
		output.sv_position += position0;
		output.sv_position = mul(output.sv_position, mwvp);
		output.posCamSpace = output.sv_position.xyz;
		//Field's result color will be the same as its initial color
		output.direction = In[0].isSelected + In[0].color.rgb;
		output.index = In[0].index;
		TriStream.Append(output);
		_isExtremity = !_isExtremity;								   //One time extreme vertex is drawn, and the other time center vertex is drawn. that's why _test is used. Center vertex : _test = 0. Extremity vertex : _test = 1;
	}
	TriStream.RestartStrip();
}

void GSBouncerTypeAppearance(point VS_FIELDPARAMETERSOUTPUT In[1], inout TriangleStream<VS_FIELDOUTPUT> TriStream)
{
	VS_FIELDOUTPUT output = (VS_FIELDOUTPUT)0;
	matrix mwvp = mul(mul(matWorld, matView), matProj);
	float _size = In[0].size;
	float precision = 12;
	float4 position0 = float4 (In[0].pos, 1);
	bool _isExtremity = 0;
	float3 _direction;

	for (precise float angle = -PI / (precision); angle <= PI*2.0; angle += PI / (precision))
	{

		_direction = float3 (cos(angle), sin(angle), 0);
		if (!_isExtremity)
			output.sv_position = float4(0.0, 0.0, 0.0, 0.0); // New vertex's position
		else
			output.sv_position = float4(_direction, 0) * In[0].size;// New vertex's position
		output.sv_position += position0;
		output.sv_position = mul(output.sv_position, mwvp);
		output.posCamSpace = output.sv_position.xyz;
		//Field's result color will be the same as its initial color
		output.direction = In[0].isSelected + In[0].color.rgb;
		output.intensity = 1.0;
		output.index = In[0].index;
		TriStream.Append(output);
		_isExtremity = !_isExtremity;								   //One time extreme vertex is drawn, and the other time center vertex is drawn. that's why _test is used. Center vertex : _test = 0. Extremity vertex : _test = 1;
	}
	TriStream.RestartStrip();
}

[maxvertexcount(72)]
void GSRenderFields(line VS_FIELDPARAMETERSOUTPUT In[2], inout TriangleStream<VS_FIELDOUTPUT> TriStream )
{
	VS_FIELDOUTPUT output = (VS_FIELDOUTPUT) 0;
	matrix mwvp = mul(mul(matWorld, matView), matProj);
	float4 position0;
	float3 _direction;
	float precision = 12;
	float _goalSize;
	bool  _isAPortal;
	bool _isExtremity;
	bool _hasGoal = (In[0].type != EMITTER_TYPE && In[0].type != BOUNCER_TYPE);
	if (In[0].size <= 0.0)			 // If field isn't deactivated
		return;
	_isAPortal = In[0].type == PORTAL_TYPE;
	if(In[0].type == FORCE_TYPE)
			GSForceTypeAppearance (In[0], TriStream);

	else if (In[0].type == EMITTER_TYPE)
		GSEmitterTypeAppearance(In[0], TriStream);

	else if (In[0].type == BOUNCER_TYPE)
		GSBouncerTypeAppearance(In[0], TriStream);

	else GSPortalTypeAppearance(In[0], TriStream);
	_isExtremity = 0;

	position0 = float4 (In[0].pos, 1);
	if (!_hasGoal)
		return;
	// Draws field goal
	for (precise float angle = -PI / (precision); angle <= PI*2.0; angle += PI / (precision))
	{
		if (!_isExtremity)
		{
			output.intensity = !_isAPortal;
			output.sv_position = float4(0, 0, 0, 0);
		}
		else
		{
			output.intensity = 1;
			_goalSize = (In[0].type == PORTAL_TYPE) ? In[0].size  : GetGoalSize(In[0].size);//If field is PORTAL_TYPE type it's goal is same size
			_goalSize = (In[0].type == BOUNCER_TYPE) ? _goalSize * 2 : _goalSize;//If field is bouncer type, goal will be a bit bigger than the other types
			output.sv_position = float4(_direction * _goalSize, 0);
		}
		output.sv_position += position0 + float4(In[0].goal, 0);
		output.sv_position = mul(output.sv_position, mwvp);
		_direction = float3 (cos (angle), sin (angle),0);
		
		output.posCamSpace = output.sv_position.xyz;
		//Field's result color will be the same as its initial color
		output.direction = (In[0].isSelected) + In[0].color.rgb;
		output.index	   = In[0].index;
		TriStream.Append (output);
		_isExtremity = !_isExtremity;								   //One time extreme vertex is drawn, and the other time center vertex is drawn. that's why _test is used. Center vertex : _test = 0. Extremity vertex : _test = 1;
	}
	TriStream.RestartStrip ();
}
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSRenderFields(VS_FIELDOUTPUT In) : SV_Target
{
	float3 _camPos	= float3(matPickInfos._m00, matPickInfos._m10, matPickInfos._m20);
	float3 _camDir	= float3(matPickInfos._m02, matPickInfos._m12, matPickInfos._m22);
	float _depth	= saturate(length(In.posCamSpace - (FocusDist * _camDir)) * FocusRange);
	_depth = pow(_depth, 6);
	_depth *= g_DOF;
	return float4(In.direction, In.intensity);
}

PS_FIELDOUTPUT PSRenderFieldsParameters(VS_FIELDPARAMETERSOUTPUT In) : SV_Target
{
	PS_FIELDOUTPUT Out = (PS_FIELDOUTPUT)0;
	Out.position = float4(In.pos		, 1);
	Out.goal	 = float4(In.goal		, 1);
	Out.strength = float4(In.strength	, 1);
	Out.typeSize = float4(In.type		, In.size, 0, 1);
	return Out;
}
//--------------------------------------------------------------------------------------

GeometryShader gsStreamOutFields = ConstructGSWithSO( CompileShader( gs_4_0, GSUpdateFields() ),
"INDEX.x; ISSELECTED.x; FIELDTYPE.x; SIZE.x; OFFSET.xyz; POSITION.xyz; GOAL.xyz; FORCE.xyz; SV_Position.xyzw; FIELDCOLOR.xyzw; DUMMY.xyzw" );
technique10 UpdateFields
{
 pass P0
  {
	SetVertexShader( CompileShader( vs_4_0, VSRenderFieldsParameters() ) );
	SetGeometryShader(gsStreamOutFields);
    SetPixelShader( CompileShader( ps_4_0, PSRenderFieldsParameters() ) );
	SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
  }

  pass P1
  {
    SetVertexShader( CompileShader( vs_4_0, VSRenderFields() ) );
	SetGeometryShader( CompileShader( gs_4_0, GSRenderFields() ));
    SetPixelShader( CompileShader( ps_4_0, PSRenderFields() ) );
	SetDepthStencilState( DisableDepth, 0 );
	SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
	SetRasterizerState(Rasterizer);
  }
}

/*technique10 RenderFields
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, VSRenderFields()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PSRenderFields()));
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(Blend, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(RasterizerParams);
	}
}*/