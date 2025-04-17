#include <SDL3/SDL.h>
#include <graphics.h>
#include <linmath.h>
#include <fileio.h>
#include <demos.h>

static float deltatime;
static float lastframe;
static float last_x, last_y;
static float mouse_x, mouse_y;
static bool first_mouse;
static Camera cam_1;
Model testmodel = { 0 };
static SDL_GPUTexture* texturedepth;
static Sampler *sampler;

void Demo_Set1_Setup()
{
	deltatime = lastframe = 0.0f;
	last_x = context.width / 2.0f;
	last_y = context.height / 2.0f;
	mouse_x = last_x;
	mouse_y = last_y;
	first_mouse = true;
	//need to bring lookat, or a camera update thing
	Graphics_InitCameraBasic(&cam_1, (Vector3){1.0f, 1.0f, 8.0f});
	Shader modelvsshader = { 0 };
	if(!Graphics_LoadShaderFromFS(&modelvsshader, "shaders/demoset2.vert.spv", "main", SHADERSTAGE_VERTEX, 0, 1, 0, 0))
	{
		SDL_Log("Failed to load VS shader.");
		return;
	}
	Shader modelfsshader = { 0 };
	if(!Graphics_LoadShaderFromFS(&modelfsshader, "shaders/demoset2.frag.spv", "main", SHADERSTAGE_FRAGMENT, 1, 0, 0, 0))
	{
		SDL_Log("Failed to load FS shader.");
		return;
	}
	SDL_GPUGraphicsPipelineCreateInfo pipeline_createinfo =
	{
		.target_info =
		{
			.num_color_targets = 1,
			.color_target_descriptions = (SDL_GPUColorTargetDescription[]){{
				.format = SDL_GetGPUSwapchainTextureFormat(context.device, context.window)
			}},
			.has_depth_stencil_target = true,
			.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM
		},
		.depth_stencil_state = (SDL_GPUDepthStencilState){
			.enable_depth_test = true,
			.enable_depth_write = true,
			.enable_stencil_test = false,
			.compare_op = SDL_GPU_COMPAREOP_LESS,
			.write_mask = 0xFF
		},
		.rasterizer_state = (SDL_GPURasterizerState){
			.cull_mode = SDL_GPU_CULLMODE_NONE,
			.fill_mode = SDL_GPU_FILLMODE_FILL,
			.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE
		},
		.vertex_input_state = (SDL_GPUVertexInputState){
			.num_vertex_buffers = 1,
			.vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]){{
				.slot = 0,
				.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
				.instance_step_rate = 0,
				.pitch = sizeof(Vertex)
			}},
			.num_vertex_attributes = 5,
			.vertex_attributes = (SDL_GPUVertexAttribute[]){{
				//position
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
				.location = 0,
				.offset = 0
			}, {
				//uv
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
				.location = 1,
				.offset = (sizeof(float) * 3)
			}, {
				//normal
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
				.location = 2,
				.offset = (sizeof(float) * 3) + (sizeof(float) * 2)
			}, {
				//tangent
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
				.location = 3,
				.offset = (sizeof(float) * 3) + (sizeof(float) * 2) + (sizeof(float) * 3)
			}, {
				//color
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
				.location = 4,
				.offset = (sizeof(float) * 3) + (sizeof(float) * 2) + (sizeof(float) * 3) + (sizeof(float) * 4)
			}}
		},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.vertex_shader = modelvsshader.shader,
		.fragment_shader = modelfsshader.shader
	};
	testmodel.pipeline = SDL_CreateGPUGraphicsPipeline(context.device, &pipeline_createinfo);
	if(testmodel.pipeline == NULL)
		return;

	SDL_ReleaseGPUShader(context.device, modelvsshader.shader);
	SDL_ReleaseGPUShader(context.device, modelfsshader.shader);

	if(!Graphics_ImportIQM(&testmodel, "test_models/house/house.iqm", "test_models/house/house.material", testmodel.pipeline))
	{
		return;
	}
	/*if(!Graphics_ImportIQM(&testmodel, "test_models/avatarsampleb_teste/avatarsampleb.iqm", "test_models/avatarsampleb_teste/avatarsampleb.material", testmodel.pipeline))
	{
		return;
	}*/
	//sponza is wack somehow
	/*if(!Graphics_ImportIQM(&testmodel, "test_models/sponza/sponza.iqm", "test_models/sponza/sponza.material", testmodel.pipeline))
	{
		return;
	}*/

	sampler = Graphics_GenerateSampler(SAMPLER_FILTER_LINEAR, SAMPLER_MODE_CLAMPTOEDGE);

	texturedepth = SDL_CreateGPUTexture(
		context.device,
		&(SDL_GPUTextureCreateInfo) {
			.type = SDL_GPU_TEXTURETYPE_2D,
			.width = context.width,
			.height = context.height,
			.layer_count_or_depth = 1,
			.num_levels = 1,
			.sample_count = SDL_GPU_SAMPLECOUNT_1,
			.format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
			.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET
		}
	);

	Graphics_UploadModel(&testmodel, true);

	Graphics_MoveModel(&testmodel, (Vector3){0.0f, 0.0f, 0.0f});
	//Graphics_ScaleModel(&testmodel, 4.0f);
	return;
}

void Demo_Set1_Logic(InputState *state)
{
	float current_frame = (float)SDL_GetTicks();
	deltatime = current_frame - lastframe;
	lastframe = current_frame;
	float velocity = 0.01f * deltatime;
	if(Input_IsKeyPressedTEMP(state, SDL_SCANCODE_RIGHT))
	{
		Vector3 aux = { 0 };
		aux.x = cam_1.right.x * velocity;
		aux.y = cam_1.right.y * velocity;
		aux.z = cam_1.right.z * velocity;
		Graphics_UpdateCameraPosition(&cam_1, Vector3_Add(cam_1.position, aux));
	}
	if(Input_IsKeyPressedTEMP(state, SDL_SCANCODE_LEFT))
	{
		Vector3 aux = { 0 };
		aux.x = cam_1.right.x * velocity;
		aux.y = cam_1.right.y * velocity;
		aux.z = cam_1.right.z * velocity;
		Vector3 newpos = cam_1.position;
		newpos.x = newpos.x - aux.x;
		newpos.y = newpos.y - aux.y;
		newpos.z = newpos.z - aux.z;
		Graphics_UpdateCameraPosition(&cam_1, newpos);
	}
	if(first_mouse)
	{
		last_x = mouse_x;
		last_y = mouse_y;
		first_mouse = false;
	}
	mouse_x = state->mouse_x;
	mouse_y = state->mouse_y;
	float x_offset = state->mouse_x + last_x;
	float y_offset = last_y + state->mouse_y;
	last_x = state->mouse_x;
	last_y = state->mouse_y;
	Graphics_TestCameraFreecam(&cam_1, x_offset, y_offset, true);
	state->mouse_x = state->mouse_y = 0;
	Graphics_RotateModel(&testmodel, (Vector3){0.0f, 1.0f, 0.0f}, DegToRad(deltatime / 10));
	return;
}

void Demo_Set1_Draw()
{
	//Graphics_TestCameraUpdate(&cam_1);
	Matrix4x4 viewproj;//TODO rendering whole stuff later
	viewproj = Matrix4x4_Mul(cam_1.view, cam_1.projection);
	Matrix4x4 mvp = Matrix4x4_Mul(testmodel.transform, viewproj);

	SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(context.device);
	if (cmdbuf == NULL)
	{
		SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());
		return;
	}

	SDL_GPUTexture* swapchainTexture;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, context.window, &swapchainTexture, NULL, NULL)) {
		SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());;
	}

	if (swapchainTexture != NULL)
	{
		SDL_GPUColorTargetInfo colorTargetInfo = { 0 };
		colorTargetInfo.texture = swapchainTexture;
		colorTargetInfo.clear_color = (SDL_FColor){ 0.0f, 0.0f, 0.0f, 1.0f };
		colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
		colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

		SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo = { 0 };
		depthStencilTargetInfo.texture = texturedepth;
		depthStencilTargetInfo.cycle = true;
		depthStencilTargetInfo.clear_depth = 1;
		depthStencilTargetInfo.clear_stencil = 0;
		depthStencilTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
		depthStencilTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
		depthStencilTargetInfo.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
		depthStencilTargetInfo.stencil_store_op = SDL_GPU_STOREOP_STORE;

		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(
			cmdbuf,
			&colorTargetInfo,
			1,
			&depthStencilTargetInfo
		);

		for(int i = 0; i < testmodel.meshes.count; ++i)
		{
			SDL_BindGPUGraphicsPipeline(renderPass, testmodel.pipeline);
			SDL_BindGPUVertexBuffers(renderPass, 0, &(SDL_GPUBufferBinding){ testmodel.meshes.meshes[i].vbuffer, 0 }, 1);
			SDL_BindGPUIndexBuffer(renderPass, &(SDL_GPUBufferBinding){ testmodel.meshes.meshes[i].ibuffer, 0 }, SDL_GPU_INDEXELEMENTSIZE_32BIT);
			SDL_BindGPUFragmentSamplers(renderPass, 0, &(SDL_GPUTextureSamplerBinding){ testmodel.meshes.meshes[i].material.textures[TEXTURE_DIFFUSE]->texture, sampler }, 1);
			SDL_PushGPUVertexUniformData(cmdbuf, 0, &mvp, sizeof(mvp));
			SDL_DrawGPUIndexedPrimitives(renderPass, testmodel.meshes.meshes[i].indices.count, 1, 0, 0, 0);
		}

		SDL_EndGPURenderPass(renderPass);
	}

	SDL_SubmitGPUCommandBuffer(cmdbuf);
	return;
}

void Demo_Set1_Destroy()
{
	Graphics_ReleaseModel(&testmodel);
	Graphics_ReleaseSampler(sampler);
}