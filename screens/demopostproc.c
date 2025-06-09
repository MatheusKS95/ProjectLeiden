/*
 * Copyright (C) 2025 Matheus Klein Schaefer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

//this currently not work :(

#include <screens.h>
#include <leiden.h>

typedef struct effectvert
{
	float x, y, z;
	float u, v;
} effectvert;

static Pipeline* EffectPipeline;
static GPUBuffer* EffectVertexBuffer;
static GPUBuffer* EffectIndexBuffer;
static Sampler* EffectSampler;
static GPUTexture *SceneColorTexture;

static Model *vroid_test;
static Model *mulher;
static Sampler *sampler;
static GPUTexture *depth_texture;
static Pipeline *simple_pipeline;

static float deltatime;
static float lastframe;
static float last_x, last_y;
static float mouse_x, mouse_y;
static bool first_mouse;
static Camera cam_1;

static Pipeline *TEMP_Generate3DPipelineEffect(Shader *vs, Shader *fs,
												bool release_shaders)
{
	if(vs == NULL || fs == NULL)
	{
		return NULL;
	}

	SDL_GPUGraphicsPipelineCreateInfo pipeline_createinfo = { 0 };
	pipeline_createinfo = (SDL_GPUGraphicsPipelineCreateInfo)
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
				.pitch = sizeof(effectvert)
			}},
			.num_vertex_attributes = 2,
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
			}} //there's more, but I need only these now
		},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.vertex_shader = vs,
		.fragment_shader = fs
	};
	Pipeline *pipeline = SDL_CreateGPUGraphicsPipeline(context.device, &pipeline_createinfo);
	if(release_shaders)
	{
		SDL_ReleaseGPUShader(context.device, vs);
		SDL_ReleaseGPUShader(context.device, fs);
	}

	return pipeline;
}

bool DemoPostProc_Setup()
{
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Starting simple test screen...");
	deltatime = lastframe = 0.0f;
	last_x = context.width / 2.0f;
	last_y = context.height / 2.0f;
	mouse_x = last_x;
	mouse_y = last_y;
	first_mouse = true;
	//need to bring lookat, or a camera update thing
	Graphics_InitCameraBasic(&cam_1, (Vector3){0.0f, 0.5f, 0.0f});

	Shader *vsshader = Graphics_LoadShader("shaders/simple/simple.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX, 0, 1, 0, 0);
	if(vsshader == NULL)
	{
		SDL_Log("Failed to load skybox vertex shader.");
		return NULL;
	}
	Shader *fsshader = Graphics_LoadShader("shaders/simple/simple.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 0, 0);
	if(fsshader == NULL)
	{
		SDL_Log("Failed to load skybox fragment shader.");
		return NULL;
	}
	simple_pipeline = Graphics_Generate3DPipelineFull(vsshader, fsshader, true);

	vroid_test = (Model*)SDL_malloc(sizeof(Model));
	if(vroid_test != NULL)
	{
		Graphics_ImportIQM(vroid_test, "test_models/avatarsamplek_teste/avatarsamplek.iqm");
		Graphics_LoadModelMaterials(vroid_test, "test_models/avatarsamplek_teste/avatarsamplek.material");
		Graphics_UploadModel(vroid_test, true);
		Graphics_RotateModel(vroid_test, (Vector3){0.0f, 1.0f, 0.0f}, DegToRad(120));
		Graphics_MoveModel(vroid_test, (Vector3){0.0f, 0.0f, 2.0f});
	}

	mulher = (Model*)SDL_malloc(sizeof(Model));
	if(mulher != NULL)
	{
		Graphics_ImportIQM(mulher, "test_models/mulher2/mulher_face2.iqm");
		Graphics_LoadModelMaterials(mulher, "test_models/mulher2/mulher_face2.material");
		Graphics_UploadModel(mulher, true);
		Graphics_ScaleModel(mulher, 0.1);
		Graphics_MoveModel(mulher, (Vector3){3.0f, 0.0f, 2.0f});
	}

	//TODO: the correct order for transform a model is scale > rotation > translation
	//this is the issue i have when rotating it using the deltatime and begin orbiting the middle at mach speeds
	//keep this in mind when doing stuff in the loop - besides, transform shouldn't be here at all, but...

	sampler = Graphics_GenerateSampler(SAMPLER_FILTER_LINEAR, SAMPLER_MODE_CLAMPTOEDGE);

	depth_texture = Graphics_GenerateDepthTexture(context.width, context.height);

	SceneColorTexture = Graphics_GenerateRenderTexture(context.width, context.height);

	//effect stuff
	Shader *effectvs = Graphics_LoadShader("shaders/effects/default.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX, 0, 0, 0, 0);
	if(vsshader == NULL)
	{
		SDL_Log("Failed to load skybox vertex shader.");
		return NULL;
	}
	Shader *effectfs = Graphics_LoadShader("shaders/effects/blur.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 0, 0);
	if(fsshader == NULL)
	{
		SDL_Log("Failed to load skybox fragment shader.");
		return NULL;
	}
	EffectPipeline = TEMP_Generate3DPipelineEffect(effectvs, effectfs, true);

	//i think this might cause issues, if it looks off this might be the culprit, should allow to change mipmap mode
	EffectSampler = Graphics_GenerateSampler(SAMPLER_FILTER_NEAREST, SAMPLER_MODE_REPEAT);

	//NOTE, this is based from the SDL GPU examples repo, zlib license, I'll change this later
	{
		EffectVertexBuffer = SDL_CreateGPUBuffer(
			context.device,
			&(SDL_GPUBufferCreateInfo) {
				.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
				.size = sizeof(effectvert) * 4
			}
		);

		EffectIndexBuffer = SDL_CreateGPUBuffer(
			context.device,
			&(SDL_GPUBufferCreateInfo) {
				.usage = SDL_GPU_BUFFERUSAGE_INDEX,
				.size = sizeof(Uint32) * 6
			}
		);

		SDL_GPUTransferBuffer* bufferTransferBuffer = SDL_CreateGPUTransferBuffer(
			context.device,
			&(SDL_GPUTransferBufferCreateInfo) {
				.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
				.size = (sizeof(effectvert) * 4) + (sizeof(Uint32) * 6)
			}
		);

		effectvert* transferData = SDL_MapGPUTransferBuffer(
			context.device,
			bufferTransferBuffer,
			false
		);

		transferData[0] = (effectvert) { -1,  1, 0, 0, 0 };
		transferData[1] = (effectvert) {  1,  1, 0, 1, 0 };
		transferData[2] = (effectvert) {  1, -1, 0, 1, 1 };
		transferData[3] = (effectvert) { -1, -1, 0, 0, 1 };

		Uint32* indexData = (Uint32*) &transferData[4];
		indexData[0] = 0;
		indexData[1] = 1;
		indexData[2] = 2;
		indexData[3] = 0;
		indexData[4] = 2;
		indexData[5] = 3;

		SDL_UnmapGPUTransferBuffer(context.device, bufferTransferBuffer);

		SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(context.device);
		SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

		SDL_UploadToGPUBuffer(
			copyPass,
			&(SDL_GPUTransferBufferLocation) {
				.transfer_buffer = bufferTransferBuffer,
				.offset = 0
			},
			&(SDL_GPUBufferRegion) {
				.buffer = EffectVertexBuffer,
				.offset = 0,
				.size = sizeof(effectvert) * 4
			},
			false
		);

		SDL_UploadToGPUBuffer(
			copyPass,
			&(SDL_GPUTransferBufferLocation) {
				.transfer_buffer = bufferTransferBuffer,
				.offset = sizeof(effectvert) * 4
			},
			&(SDL_GPUBufferRegion) {
				.buffer = EffectIndexBuffer,
				.offset = 0,
				.size = sizeof(Uint32) * 6
			},
			false
		);

		SDL_EndGPUCopyPass(copyPass);
		SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
		SDL_ReleaseGPUTransferBuffer(context.device, bufferTransferBuffer);
	}

	return true;
}

void DemoPostProc_Logic(InputState *state)
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
	if(Input_IsKeyPressedTEMP(state, SDL_SCANCODE_ESCAPE))
	{
		//TEST, remove this later
		current_screen = SCREEN_SPLASH;
		DemoPostProc_Destroy();
	}
	if(first_mouse)
	{
		last_x = mouse_x;
		last_y = mouse_y;
		first_mouse = false;
	}
	//i only did keyboard stuff but i forgot mouse lmao
	//i'll be very sad when working with gamepads
	//steering wheels will be truly a painful experience
	mouse_x = state->mouse_x;
	mouse_y = state->mouse_y;
	float x_offset = state->mouse_x + last_x;
	float y_offset = last_y + state->mouse_y;
	last_x = state->mouse_x;
	last_y = state->mouse_y;
	Graphics_TestCameraFreecam(&cam_1, x_offset, y_offset, true);
	state->mouse_x = state->mouse_y = 0;
	return;
}

void DemoPostProc_Draw()
{
	CommandBuffer *cmdbuf = Graphics_SetupCommandBuffer();
	if(cmdbuf == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Splash screen: failed to acquire command buffer: %s", SDL_GetError());
		return;
	}

	GPUTexture *swapchain_texture = Graphics_AcquireSwapchainTexture(cmdbuf);

	if (swapchain_texture == NULL)
	{
		return;
	}

	Color clearcolor = { 0.4f, 0.2f, 0.6f, 1.0f };

	RenderPass *render_pass_scene = Graphics_BeginRenderPass(cmdbuf, SceneColorTexture, depth_texture, clearcolor);
	Graphics_BindPipeline(render_pass_scene, simple_pipeline);
	Matrix4x4 viewproj;
	viewproj = Matrix4x4_Mul(cam_1.view, cam_1.projection);
	Matrix4x4 mvp1 = Matrix4x4_Mul(vroid_test->transform, viewproj);
	Matrix4x4 mvp2 = Matrix4x4_Mul(mulher->transform, viewproj);

	for(size_t i = 0; i < vroid_test->meshes.count; i++)
	{
		Mesh *mesh = &vroid_test->meshes.meshes[i];

		//binding vertex and index buffers
		Graphics_BindMeshBuffers(render_pass_scene, mesh);

		//texture samplers
		Material *material = Graphics_GetMaterialByName(&vroid_test->materials, mesh->material_name);
		if(material == NULL) continue;
		Texture2D *diffuse = material->diffuse_map != NULL ? material->diffuse_map : &default_textures.default_diffuse;
		Graphics_BindFragmentSampledTexture(render_pass_scene, diffuse, sampler, 0, 1);

		//UBO
		Graphics_PushVertexUniforms(cmdbuf, 0, &mvp1, sizeof(mvp1));

		Graphics_DrawPrimitives(render_pass_scene, mesh->indices.count, 1, 0, 0, 0);
	}

	for(size_t i = 0; i < mulher->meshes.count; i++)
	{
		Mesh *mesh = &mulher->meshes.meshes[i];

		//binding vertex and index buffers
		Graphics_BindMeshBuffers(render_pass_scene, mesh);

		//texture samplers
		Material *material = Graphics_GetMaterialByName(&mulher->materials, mesh->material_name);
		if(material == NULL) continue;
		Texture2D *diffuse = material->diffuse_map != NULL ? material->diffuse_map : &default_textures.default_diffuse;
		Graphics_BindFragmentSampledTexture(render_pass_scene, diffuse, sampler, 0, 1);

		//UBO
		Graphics_PushVertexUniforms(cmdbuf, 0, &mvp2, sizeof(mvp2));

		Graphics_DrawPrimitives(render_pass_scene, mesh->indices.count, 1, 0, 0, 0);
	}
	Graphics_EndRenderPass(render_pass_scene);

	//effect drawing
	RenderPass *render_pass_effect = Graphics_BeginRenderPass(cmdbuf, swapchain_texture, NULL, clearcolor);
	Graphics_BindPipeline(render_pass_effect, EffectPipeline);
	Graphics_BindVertexBuffers(render_pass_effect, EffectVertexBuffer, 0, 0, 1);
	Graphics_BindIndexBuffers(render_pass_effect, EffectIndexBuffer, 0);
	/*SDL_BindGPUFragmentSamplers(render_pass_effect, 0, (SDL_GPUTextureSamplerBinding[]){
				{ .texture = SceneColorTexture, .sampler = EffectSampler },
				{ .texture = depth_texture, .sampler = EffectSampler }
			}, 2);*/
	//Graphics_BindFragmentSampledGPUTexture don't support more than 1 texture, FIXME
	Graphics_BindFragmentSampledGPUTexture(render_pass_effect, SceneColorTexture, EffectSampler, 0, 1);
	Graphics_DrawPrimitives(render_pass_effect, 6, 1, 0, 0, 0);
	Graphics_EndRenderPass(render_pass_effect);

	Graphics_CommitCommandBuffer(cmdbuf);
	return;
}

void DemoPostProc_Destroy()
{
	Graphics_ReleaseModel(vroid_test);
	Graphics_ReleaseModel(mulher);
	SDL_free(vroid_test);
	SDL_free(mulher);
	Graphics_ReleaseSampler(sampler);
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Finishing postproc screen...");
	return;
}