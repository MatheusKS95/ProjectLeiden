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

#include <SDL3/SDL.h>
#include <screens.h>
#include <leiden.h>

static SDL_GPUGraphicsPipeline* effect_pipeline;
static EffectBuffers *effect_buffers;
static SDL_GPUSampler* effect_sampler;
static SDL_GPUTexture *scene_colortexture;

static Model *vroid_test;
static Model *mulher;
static Sampler *sampler;
static SDL_GPUTexture *depth_texture;
static SDL_GPUGraphicsPipeline *simple_pipeline;

static float deltatime;
static float lastframe;
static float last_x, last_y;
static float mouse_x, mouse_y;
static bool first_mouse;
static Camera cam_1;

bool CelShading_Setup()
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
		Graphics_ImportIQM(vroid_test, "test_models/michikocasual/michikocasual.iqm");
		Graphics_LoadModelMaterials(vroid_test, "test_models/michikocasual/michikocasual.material");
		Graphics_UploadModel(vroid_test, true);
		Graphics_RotateModel(vroid_test, (Vector3){1.0f, 0.0f, 0.0f}, DegToRad(90)); //iqm blender exporter problems
		Graphics_RotateModel(vroid_test, (Vector3){0.0f, 1.0f, 0.0f}, DegToRad(179));
		Graphics_MoveModel(vroid_test, (Vector3){0.0f, 0.0f, 1.0f});
	}

	mulher = (Model*)SDL_malloc(sizeof(Model));
	if(mulher != NULL)
	{
		Graphics_ImportIQM(mulher, "test_models/mulher2/mulher_face2.iqm");
		Graphics_LoadModelMaterials(mulher, "test_models/mulher2/mulher_face2.material");
		Graphics_UploadModel(mulher, true);
		Graphics_ScaleModel(mulher, 0.1);
		Graphics_MoveModel(mulher, (Vector3){1.0f, 0.0f, 1.0f});
	}

	//TODO: the correct order for transform a model is scale > rotation > translation
	//this is the issue i have when rotating it using the deltatime and begin orbiting the middle at mach speeds
	//keep this in mind when doing stuff in the loop - besides, transform shouldn't be here at all, but...

	sampler = Graphics_GenerateSampler(SAMPLER_FILTER_LINEAR, SAMPLER_MIPMAPMODE_LINEAR, SAMPLER_MODE_CLAMPTOEDGE);

	depth_texture = Graphics_GenerateDepthTexture(context.width, context.height);
	scene_colortexture = Graphics_GenerateRenderTexture(context.width, context.height);

	//effect stuff
	Shader *effectvs = Graphics_LoadShader("shaders/effects/default.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX, 0, 0, 0, 0);
	if(vsshader == NULL)
	{
		SDL_Log("Failed to load effect vertex shader.");
		return NULL;
	}
	//tried with blur, some sharpening, etc. all did work
	Shader *effectfs = Graphics_LoadShader("shaders/effects/blur.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 0, 0);
	if(fsshader == NULL)
	{
		SDL_Log("Failed to load effect fragment shader.");
		return NULL;
	}
	effect_pipeline = Graphics_GenerateEffectsPipeline(effectvs, effectfs, true);

	effect_sampler = Graphics_GenerateSampler(SAMPLER_FILTER_NEAREST, SAMPLER_MIPMAPMODE_NEAREST, SAMPLER_MODE_REPEAT);

	effect_buffers = Graphics_GenerateEffectBuffers();

	return true;
}

void CelShading_Logic(InputState *state)
{
	return;
}

void CelShading_Draw()
{
	return;
}

void CelShading_Destroy()
{
	return;
}