/*
No CMakeLists principal
option(ROCKPORT_ASSIMP_VENDORED "I don't have assimp installed" ON)
if(ROCKPORT_ASSIMP_VENDORED)
	set(ASSIMP_NO_EXPORT ON)
	#set(ASSIMP_BUILD_ZLIB ON)
	set(ASSIMP_BUILD_TESTS OFF)
	set(ASSIMP_INSTALL OFF)
	set(ASSIMP_BUILD_ASSIMP_VIEW OFF)
	#FetchContent_Declare(
	#	assimp
	#	GIT_REPOSITORY https://github.com/assimp/assimp/
	#	GIT_TAG v5.4.3
	#)
	#FetchContent_MakeAvailable(assimp)
	add_subdirectory(vendored/assimp)
else()
	find_package(assimp REQUIRED CONFIG REQUIRED COMPONENTS assimp)
endif()

No CMakeLists principal, do módulo graphics e módulo demo
target_link_libraries(ProjectLeiden PRIVATE assimp::assimp)
*/

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <assimp/cimport.h>        // Plain-C interface
#include <assimp/scene.h>          // Output data structure
#include <assimp/postprocess.h>    // Post processing flags
#include <graphics.h>
#include <fileio.h>
#include <demos.h>

//TODO depth testing

//Shader codes used for this demo are
//vertex
/*#version 450
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec4 in_tangent;
layout (location = 4) in vec4 in_vcolor;

layout (location = 0) out vec3 FragPos;
layout (location = 1) out vec2 TexCoord;

layout (set = 1, binding = 0) uniform UniformBlock
{
	mat4 mvp;
};

void main()
{
	FragPos = in_pos;
	TexCoord = in_uv;
	gl_Position = mvp * vec4(in_pos, 1.0);
}*/

//fragment
/*#version 450
layout (location = 0) in vec3 FragPos;
layout (location = 1) in vec2 TexCoord;

layout (location = 0) out vec4 FragColor;

layout (set = 2, binding = 0) uniform sampler2D diffuse;

void main()
{
	FragColor = texture(diffuse, TexCoord);
}*/

typedef struct Demo2Vertex
{
	float pos_x, pos_y, pos_z;
	float u, v;
	float norm_x, norm_y, norm_z;
	float tan_x, tan_y, tan_z;
	float r, g, b, a;
} Demo2Vertex;

typedef struct Demo2DynArrayVertex
{
	size_t count;
	size_t capacity;
	Demo2Vertex *items;
} Demo2DynArrayVertex;

typedef struct Demo2DynArrayIndices
{
	size_t count;
	size_t capacity;
	Uint32 *items;
} Demo2DynArrayIndices;

typedef struct Demo2Mesh
{
	//if false, vbuffer and ibuffers will be empty, vertices and indices full
	//when true, vbuffer and ibuffers are used, vertices and indices freed and NULL
	bool uploaded;
	Demo2DynArrayVertex arrv;
	Demo2DynArrayIndices arri;
	SDL_GPUTexture *diffuse;
	SDL_GPUSampler *sampler;
	char meshname[64];
	char matname[64];
	SDL_GPUBuffer *vbuffer;
	SDL_GPUBuffer *ibuffer;
} Demo2Mesh;

typedef struct Demo2DynArrayMeshes
{
	size_t count;
	size_t capacity;
	Demo2Mesh *items;
} Demo2DynArrayMeshes;

typedef struct Demo2Model
{
	Demo2DynArrayMeshes meshes;
} Demo2Model;

static bool DynarrayRealocVertex(Demo2DynArrayVertex *arr, size_t new_size)
{
	if(arr->items != NULL)
	{
		Demo2Vertex *aux = (Demo2Vertex*)SDL_realloc(arr->items, sizeof(Demo2Vertex) * new_size);
		if(aux != NULL)
		{
			arr->items = aux;
			arr->capacity = new_size;
			return true;
		}
	}
	return false;
}

static void DynarrayInitVertex(Demo2DynArrayVertex *arr)
{
	arr->capacity = 1;
	arr->count = 0;
	arr->items = (Demo2Vertex*)SDL_calloc(arr->capacity, sizeof(Demo2Vertex));
}

static bool DynarrayPushLastVertex(Demo2DynArrayVertex *arr, Demo2Vertex value)
{
	if(arr->capacity == arr->count)
	{
		if(!DynarrayRealocVertex(arr, arr->capacity + 1))
		{
			return false;
		}
	}
	arr->count++;
	arr->items[arr->count - 1] = value;
	return true;
}

static bool DynarrayPopAtVertex(Demo2DynArrayVertex *arr, unsigned int index, Demo2Vertex *value)
{
	if(arr->count == 0)
	{
		return false;
	}
	*value = arr->items[index];
	SDL_memmove(&arr->items[index], &arr->items[index + 1], sizeof(Demo2Vertex) * ((arr->count - 1) - index));
	arr->count--;
	DynarrayRealocVertex(arr, arr->capacity - 1);
	return true;
}

static void DynarrayDestroyVertex(Demo2DynArrayVertex *arr)
{
	SDL_free(arr->items);
	arr->items = NULL;
}

static void DynarrayClearVertex(Demo2DynArrayVertex *arr)
{
	DynarrayDestroyVertex(arr);
	DynarrayInitVertex(arr);
}

static bool DynarrayRealocIndices(Demo2DynArrayIndices *arr, size_t new_size)
{
	if(arr->items != NULL)
	{
		Uint32 *aux = (Uint32*)SDL_realloc(arr->items, sizeof(Uint32) * new_size);
		if(aux != NULL)
		{
			arr->items = aux;
			arr->capacity = new_size;
			return true;
		}
	}
	return false;
}

static void DynarrayInitIndices(Demo2DynArrayIndices *arr)
{
	arr->capacity = 1;
	arr->count = 0;
	arr->items = (Uint32*)SDL_calloc(arr->capacity, sizeof(Uint32));
}

static bool DynarrayPushLastIndices(Demo2DynArrayIndices *arr, Uint32 value)
{
	if(arr->capacity == arr->count)
	{
		if(!DynarrayRealocIndices(arr, arr->capacity + 1))
		{
			return false;
		}
	}
	arr->count++;
	arr->items[arr->count - 1] = value;
	return true;
}

static bool DynarrayPopAtIndices(Demo2DynArrayIndices *arr, unsigned int index, Uint32 *value)
{
	if(arr->count == 0)
	{
		return false;
	}
	*value = arr->items[index];
	SDL_memmove(&arr->items[index], &arr->items[index + 1], sizeof(Uint32) * ((arr->count - 1) - index));
	arr->count--;
	DynarrayRealocIndices(arr, arr->capacity - 1);
	return true;
}

static void DynarrayDestroyIndices(Demo2DynArrayIndices *arr)
{
	SDL_free(arr->items);
	arr->items = NULL;
}

static void DynarrayClearIndices(Demo2DynArrayIndices *arr)
{
	DynarrayDestroyIndices(arr);
	DynarrayInitIndices(arr);
}

static bool DynarrayRealocMeshes(Demo2DynArrayMeshes *arr, size_t new_size)
{
	if(arr->items != NULL)
	{
		Demo2Mesh *aux = (Demo2Mesh*)SDL_realloc(arr->items, sizeof(Demo2Mesh) * new_size);
		if(aux != NULL)
		{
			arr->items = aux;
			arr->capacity = new_size;
			return true;
		}
	}
	return false;
}

static void DynarrayInitMeshes(Demo2DynArrayMeshes *arr)
{
	arr->capacity = 1;
	arr->count = 0;
	arr->items = (Demo2Mesh*)SDL_calloc(arr->capacity, sizeof(Demo2Mesh));
}

static bool DynarrayPushLastMeshes(Demo2DynArrayMeshes *arr, Demo2Mesh value)
{
	if(arr->capacity == arr->count)
	{
		if(!DynarrayRealocMeshes(arr, arr->capacity + 1))
		{
			return false;
		}
	}
	arr->count++;
	arr->items[arr->count - 1] = value;
	return true;
}

static bool DynarrayPopAtMeshes(Demo2DynArrayMeshes *arr, unsigned int index, Demo2Mesh *value)
{
	if(arr->count == 0)
	{
		return false;
	}
	*value = arr->items[index];
	SDL_memmove(&arr->items[index], &arr->items[index + 1], sizeof(Demo2Mesh) * ((arr->count - 1) - index));
	arr->count--;
	DynarrayRealocMeshes(arr, arr->capacity - 1);
	return true;
}

static void DynarrayDestroyMeshes(Demo2DynArrayMeshes *arr)
{
	SDL_free(arr->items);
	arr->items = NULL;
}

static void DynarrayClearMeshes(Demo2DynArrayMeshes *arr)
{
	DynarrayDestroyMeshes(arr);
	DynarrayInitMeshes(arr);
}

static void uploadmesh(Demo2Mesh *mesh)
{
	if(mesh == NULL) return;

	mesh->vbuffer = SDL_CreateGPUBuffer(
		context.device,
		&(SDL_GPUBufferCreateInfo) {
			.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
			.size = sizeof(Demo2Vertex) * mesh->arrv.count
		}
	);

	mesh->ibuffer = SDL_CreateGPUBuffer(
		context.device,
		&(SDL_GPUBufferCreateInfo) {
			.usage = SDL_GPU_BUFFERUSAGE_INDEX,
			.size = sizeof(Uint32) * mesh->arri.count
		}
	);

	SDL_GPUTransferBuffer* vbufferTransferBuffer = SDL_CreateGPUTransferBuffer(
		context.device,
		&(SDL_GPUTransferBufferCreateInfo) {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = (sizeof(Demo2Vertex) * mesh->arrv.count)
		}
	);
	SDL_GPUTransferBuffer* ibufferTransferBuffer = SDL_CreateGPUTransferBuffer(
		context.device,
		&(SDL_GPUTransferBufferCreateInfo) {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = (sizeof(Uint32) * mesh->arri.count)
		}
	);

	//TODO maybe separate two buffers, one for vertices and other for indices, maybe this helps
	Demo2Vertex* bufferTransferData = SDL_MapGPUTransferBuffer(context.device, vbufferTransferBuffer, false);
	SDL_memcpy(bufferTransferData, mesh->arrv.items, sizeof(Demo2Vertex) * mesh->arrv.count);
	SDL_UnmapGPUTransferBuffer(context.device, vbufferTransferBuffer);
	Uint32* indexData = SDL_MapGPUTransferBuffer(context.device, ibufferTransferBuffer, false);
	SDL_memcpy(indexData, mesh->arri.items, sizeof(Uint32) * mesh->arri.count);
	SDL_UnmapGPUTransferBuffer(context.device, ibufferTransferBuffer);

	// Upload the transfer data to the GPU buffers
	SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(context.device);
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdbuf);

	SDL_UploadToGPUBuffer(
		copyPass,
		&(SDL_GPUTransferBufferLocation) {
			.transfer_buffer = vbufferTransferBuffer,
			.offset = 0
		},
		&(SDL_GPUBufferRegion) {
			.buffer = mesh->vbuffer,
			.offset = 0,
			.size = sizeof(Demo2Vertex) * mesh->arrv.count
		},
		false
	);

	SDL_UploadToGPUBuffer(
		copyPass,
		&(SDL_GPUTransferBufferLocation) {
			.transfer_buffer = ibufferTransferBuffer,
			.offset = 0
		},
		&(SDL_GPUBufferRegion) {
			.buffer = mesh->ibuffer,
			.offset = 0,
			.size = sizeof(Uint32) * mesh->arri.count
		},
		false
	);
	SDL_EndGPUCopyPass(copyPass);

	SDL_ReleaseGPUTransferBuffer(context.device, vbufferTransferBuffer);
	SDL_ReleaseGPUTransferBuffer(context.device, ibufferTransferBuffer);

	SDL_SubmitGPUCommandBuffer(cmdbuf);

	//DynarrayClearVertex(&mesh->arrv);
	//DynarrayClearIndices(&mesh->arri);
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Mesh %s uploaded.", mesh->meshname);
}

static SDL_Surface *loadtexture(SDL_GPUTexture **texture, SDL_GPUSampler **sampler, const char *path)
{
	SDL_Surface *surface;
	SDL_PixelFormat format;
	SDL_IOStream *stream;
	size_t size;
	Uint8 *texdfile = FileIOReadBytes(path, &size);
	stream = SDL_IOFromMem(texdfile, size);
	surface = IMG_Load_IO(stream, true);
	if(surface == NULL)
	{
		return NULL;
	}
	format = SDL_PIXELFORMAT_ABGR8888;
	if(surface->format != format)
	{
		SDL_Surface *next = SDL_ConvertSurface(surface, format);
		SDL_DestroySurface(surface);
		surface = next;
	}
	*texture = SDL_CreateGPUTexture(context.device, &(SDL_GPUTextureCreateInfo){
		.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
		.type = SDL_GPU_TEXTURETYPE_2D,
		.width = surface->w,
		.height = surface->h,
		.layer_count_or_depth = 1,
		.num_levels = 1,
		.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER
	});
	*sampler = SDL_CreateGPUSampler(context.device, &(SDL_GPUSamplerCreateInfo){
		.min_filter = SDL_GPU_FILTER_LINEAR, //NEAREST if you want funky ps1 graphics
		.mag_filter = SDL_GPU_FILTER_LINEAR,
		.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
		.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
	});
	return surface;
}

static void processmesh(Demo2Mesh *mesh, struct aiMesh *aimesh, const struct aiScene *scene)
{
	mesh->uploaded = false;
	DynarrayInitVertex(&mesh->arrv);
	DynarrayInitIndices(&mesh->arri);
	//no texture processed

	for(unsigned int i = 0; i < aimesh->mNumVertices; ++i)
	{
		Demo2Vertex vertex = { 0 };
		vertex.pos_x = aimesh->mVertices[i].x;
		vertex.pos_y = aimesh->mVertices[i].y;
		vertex.pos_z = aimesh->mVertices[i].z;

		vertex.norm_x = aimesh->mNormals[i].x;
		vertex.norm_y = aimesh->mNormals[i].y;
		vertex.norm_z = aimesh->mNormals[i].z;

		vertex.u = aimesh->mTextureCoords[0][i].x;
		vertex.v = aimesh->mTextureCoords[0][i].y;

		vertex.tan_x = aimesh->mTangents[i].x;
		vertex.tan_y = aimesh->mTangents[i].y;
		vertex.tan_z = aimesh->mTangents[i].z;

		if(aimesh->mColors[0] != NULL)
		{
			vertex.r = aimesh->mColors[0][i].r;
			vertex.g = aimesh->mColors[0][i].g;
			vertex.b = aimesh->mColors[0][i].b;
			vertex.a = aimesh->mColors[0][i].a;
		}
		else
		{
			vertex.r = 0.0f;
			vertex.g = 0.0f;
			vertex.b = 0.0f;
			vertex.a = 0.0f;
		}

		DynarrayPushLastVertex(&mesh->arrv, vertex);
	}

	//debug, safety purposes
	if(aimesh->mNumVertices != mesh->arrv.count)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Assimp vertex count and mesh vertex count are not equal");
	}

	for(unsigned int i = 0; i < aimesh->mNumFaces; ++i)
	{
		struct aiFace face = aimesh->mFaces[i];
		for(unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			//printf("%d ", face.mIndices[j]);
			DynarrayPushLastIndices(&mesh->arri, face.mIndices[j]);
		}
	}
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Vertices loaded: %d of %d. Indices loaded: %d of %d", (int)mesh->arrv.count, aimesh->mNumVertices, (int)mesh->arri.count, aimesh->mNumFaces * 3);
	SDL_snprintf(mesh->meshname, 64, "%s", aimesh->mName.data);

	//just to test, might require a lot of work
	struct aiMaterial *material = scene->mMaterials[aimesh->mMaterialIndex];
	struct aiString name;
	struct aiString path;
	aiGetMaterialTexture(material, aiTextureType_DIFFUSE, 0, &path, NULL, NULL,	NULL, NULL, NULL, NULL);
	SDL_snprintf(mesh->matname, 64, "%s", aimesh->mName.data);
}

static void processnode(Demo2Model *model, struct aiNode *node, const struct aiScene *scene)
{
	for(unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		Demo2Mesh d2mesh = { 0 };
		processmesh(&d2mesh, mesh, scene);
		DynarrayPushLastMeshes(&model->meshes, d2mesh);
	}
	for(unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processnode(model, node->mChildren[i], scene);
	}
}

static bool loadmodel(Demo2Model *model, const char *path)
{
	if(model == NULL) return false;
	size_t file_len;
	Uint8 *fbuf = FileIOReadBytes(path, &file_len);
	if(fbuf == NULL) return false;

	const struct aiScene *scene = aiImportFileFromMemory((char*)fbuf, file_len,
															aiProcess_CalcTangentSpace |
															aiProcess_Triangulate |
															aiProcess_JoinIdenticalVertices |
															aiProcess_GenSmoothNormals |
															aiProcess_FlipUVs,
															NULL);

	if(scene == NULL || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == NULL)
		return false;

	DynarrayInitMeshes(&model->meshes);

	processnode(model, scene->mRootNode, scene);

	aiReleaseImport(scene);
	return true;
}

static void uploadtexture(SDL_GPUTexture *texture, SDL_GPUSampler *sampler, SDL_Surface *surface)
{
	SDL_GPUTransferBuffer* textureTransferBuffer = SDL_CreateGPUTransferBuffer(
		context.device,
		&(SDL_GPUTransferBufferCreateInfo) {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = surface->w * surface->h * 4
		}
	);

	Uint8* textureTransferPtr = SDL_MapGPUTransferBuffer(
		context.device,
		textureTransferBuffer,
		false
	);
	SDL_memcpy(textureTransferPtr, surface->pixels, surface->w * surface->h * 4);
	SDL_UnmapGPUTransferBuffer(context.device, textureTransferBuffer);

	// Upload the transfer data to the GPU resources
	SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(context.device);
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

	SDL_UploadToGPUTexture(
		copyPass,
		&(SDL_GPUTextureTransferInfo) {
			.transfer_buffer = textureTransferBuffer,
			.offset = 0, /* Zeros out the rest */
		},
		&(SDL_GPUTextureRegion){
			.texture = texture,
			.w = surface->w,
			.h = surface->h,
			.d = 1
		},
		false
	);

	SDL_EndGPUCopyPass(copyPass);
	SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
	SDL_ReleaseGPUTransferBuffer(context.device, textureTransferBuffer);
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Image uploaded.");

}

static float deltatime;
static float lastframe;
static float last_x, last_y;
static float mouse_x, mouse_y;
static bool first_mouse;
static Camera cam_1;
static SDL_GPUGraphicsPipeline* model_pipeline;
Demo2Model testmodel = { 0 };
static SDL_GPUTexture* texturedepth;

void Demo_Set2_Setup()
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
				.pitch = sizeof(Demo2Vertex)
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
	model_pipeline = SDL_CreateGPUGraphicsPipeline(context.device, &pipeline_createinfo);
	if(model_pipeline == NULL)
		return;

	SDL_ReleaseGPUShader(context.device, modelvsshader.shader);
	SDL_ReleaseGPUShader(context.device, modelfsshader.shader);

	if(!loadmodel(&testmodel, "test_models/house/house.glb"))
	{
		return;
	}
	SDL_GPUTexture *texture;
	SDL_GPUSampler *sampler;
	SDL_Surface *surface = loadtexture(&texture, &sampler, "test_models/house/texture2.png");
	if(surface == NULL)
	{
		//TODO clear up memory first
		return;
	}

	uploadtexture(texture, sampler, surface);

	SDL_DestroySurface(surface);

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

	for(int i = 0; i < testmodel.meshes.count; ++i)
	{
		testmodel.meshes.items[i].diffuse = texture;
		testmodel.meshes.items[i].sampler = sampler;
		uploadmesh(&testmodel.meshes.items[i]);
	}

	return;
}

void Demo_Set2_Logic(InputState *state)
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
	return;
}

void Demo_Set2_Draw()
{
	//Graphics_TestCameraUpdate(&cam_1);
	Matrix4x4 viewproj;//TODO rendering whole stuff later
	viewproj = Matrix4x4_Mul(cam_1.view, cam_1.projection);
	Matrix4x4 model = { 0 };
	model.aa = model.bb = model.cc = model.dd = 1.0f;
	//model = Matrix4x4_Scale(model, (Vector3){0.01f, 0.01f, 0.01f});
	Matrix4x4 mvp = Matrix4x4_Mul(model, viewproj);

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
			SDL_BindGPUGraphicsPipeline(renderPass, model_pipeline);
			SDL_BindGPUVertexBuffers(renderPass, 0, &(SDL_GPUBufferBinding){ testmodel.meshes.items[i].vbuffer, 0 }, 1);
			SDL_BindGPUIndexBuffer(renderPass, &(SDL_GPUBufferBinding){ testmodel.meshes.items[i].ibuffer, 0 }, SDL_GPU_INDEXELEMENTSIZE_32BIT);
			SDL_BindGPUFragmentSamplers(renderPass, 0, &(SDL_GPUTextureSamplerBinding){ testmodel.meshes.items[i].diffuse, testmodel.meshes.items[i].sampler }, 1);
			SDL_PushGPUVertexUniformData(cmdbuf, 0, &mvp, sizeof(mvp));
			SDL_DrawGPUIndexedPrimitives(renderPass, testmodel.meshes.items[i].arri.count, 1, 0, 0, 0);
		}

		SDL_EndGPURenderPass(renderPass);
	}

	SDL_SubmitGPUCommandBuffer(cmdbuf);
	return;
}
