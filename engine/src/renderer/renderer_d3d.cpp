#if !defined(VIPOC_WIN32)

#include <renderer/renderer.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <renderer/d3dx12.h>
// #include <dxgi1_4.h>
// #include <dxgi1_6.h>

#define BUFFER_COUNT 3
#define CHECK(result, message)                      \
	if (result != S_OK)                             \
	{                                               \
		char _printable_error_message[4096] = {};   \
		strcat(_printable_error_message, message);  \
		strcat(_printable_error_message, ": %x");   \
		assert(false);                              \
		VP_FATAL(_printable_error_message, result); \
	}

typedef struct Win32_State
{
	HINSTANCE instance;
	HWND window;
	int w;
	int h;
} Win32_State;

struct Shaders
{
	ID3DBlob *vertex;
	ID3DBlob *pixel;
};

struct D3D_State
{
	D3D12_COMMAND_QUEUE_DESC command_queue_desc;
	ID3D12Device *device;
	ID3D12CommandQueue *command_queue;
	ID3D12CommandAllocator *command_allocators[BUFFER_COUNT];
	ID3D12PipelineState *pipeline;
	ID3D12RootSignature *root_signature;
	ID3D12DescriptorHeap *rtv_desc_heap;
	D3D12_CPU_DESCRIPTOR_HANDLE render_target_descs[BUFFER_COUNT];
	ID3D12Resource *render_target_buffers[BUFFER_COUNT];
	D3D12_INDEX_BUFFER_VIEW index_buff_desc;
	D3D12_VERTEX_BUFFER_VIEW vertex_buff_desc;
	ID3D12Debug *debug;
	IDXGIFactory2 *factory;
	IDXGISwapChain3 *swap_chain;
	Win32_State *win32_state;
	b32 vsync;
};

internal D3D_State renderer;

static wchar_t *char_to_wchar(const char *str)
{
	const size_t size = vstd_strlen((char *)str) + 1;
	wchar_t *result = (wchar_t *)vp_allocate_temp(size);
	mbstowcs(result, str, size);
	return result;
}

void render_update()
{
	START_DTIMER();

	unsigned current_buffer_index = renderer.swap_chain->GetCurrentBackBufferIndex();
	D3D12_CPU_DESCRIPTOR_HANDLE current_render_target = renderer.render_target_descs[current_buffer_index];

	HRESULT hr;

	renderer.command_allocators[current_buffer_index]->Reset();
	ID3D12GraphicsCommandList *command_list;
	hr = renderer.device->CreateCommandList(0, renderer.command_queue_desc.Type, renderer.command_allocators[current_buffer_index], 0, IID_PPV_ARGS(&command_list));
	CHECK(hr, "Failed to create command list");

	D3D12_RESOURCE_BARRIER render_target_resource_barrier = {};
	render_target_resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	render_target_resource_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc;
	renderer.swap_chain->GetDesc1(&swap_chain_desc);

	D3D12_VIEWPORT viewport = {};
	viewport.Width = swap_chain_desc.Width;
	viewport.Height = swap_chain_desc.Height;

	D3D12_RECT scissor = {};
	scissor.right = viewport.Width;
	scissor.bottom = viewport.Height;

	command_list->RSSetViewports(1, &viewport);
	command_list->RSSetScissorRects(1, &scissor);

	f32 background[] = {0, 1, 1, 1};
	command_list->ClearRenderTargetView(current_render_target, background, 0, 0);

	CD3DX12_RESOURCE_BARRIER start_barrier = CD3DX12_RESOURCE_BARRIER::Transition(renderer.render_target_buffers[current_buffer_index], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CD3DX12_RESOURCE_BARRIER end_barrier = CD3DX12_RESOURCE_BARRIER::Transition(renderer.render_target_buffers[current_buffer_index], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	command_list->ResourceBarrier(1, &start_barrier);

	command_list->SetGraphicsRootSignature(renderer.root_signature);
	command_list->SetPipelineState(renderer.pipeline);
	command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	/*
	command_list->IASetIndexBuffer(&renderer.index_buff_desc);
	command_list->IASetVertexBuffers(0, 1, &renderer.vertex_buff_desc);
	*/
	command_list->OMSetRenderTargets(1, &current_render_target, false, 0);
	command_list->DrawInstanced(3, 1, 0, 0);
	command_list->ResourceBarrier(1, &end_barrier);

	command_list->Close();

	ID3D12CommandList *command_lists[] = {command_list};
	renderer.command_queue->ExecuteCommandLists(1, command_lists);

	renderer.swap_chain->Present(1, 0);

	// TODO(Vasko): fencing
	command_list->Release();
	STOP_DTIMER();
}

// TODO(Vasko): look more into this function
void renderer_resize(int new_w, int new_h)
{
	//	if (renderer.swap_chain)
	//		renderer.swap_chain->ResizeBuffers(BUFFER_COUNT, (unsigned)new_w, (unsigned)new_h, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	//	renderer.swap_chain->ResizeTarget(&resize_desc);
}

b32 renderer_toggle_vsync(b32 toggle)
{
	renderer.vsync = toggle;
	return true;
}

// TODO(Vasko): Implement the functions below
// [

void vp_clear_screen()
{
}

void vp_draw_text(char *text, float x, float y, u32 color, float scaler, int layer_id)
{
}

void vp_draw_rectangle(m2 location, v4 color, int layer_id)
{
}

camera get_camera()
{
	camera result = {};
	return result;
}

m4 calculate_3d_uniforms()
{
	m4 result = {};
	return result;
}

void set_shader_uniform_vec4(char *str, v4 vector)
{
}

void set_shader_uniform_mat4(char *str, m4 mat)
{
}

void make_draw_call(size_t offset, u32 num_of_elements)
{
}

void vp_camera_mouse_callback(double xpos, double ypos)
{
}

void vp_update_mouse_pos(double xpos, double ypos)
{
}

float vp_get_dtime()
{
	return 1.0f;
}

void vp_unlock_camera()
{
}

void vp_lock_camera(float yaw, float pitch, v3 xyz)
{
}

void vp_toggle_polygons()
{
}

void vp_cast_ray(i32 x, i32 y)
{
}

void vp_move_camera(vp_direction direction, f32 speed)
{
}

void vp_parse_font_fnt(entire_file file)
{
}

void vp_load_text_atlas(char *path)
{
}

vp_mesh_identifier
vp_load_simple_obj(char *path)
{
	vp_mesh_identifier result = {};
	return result;
}

// ]

ID3DBlob *compile_shader(char *path, const char *entry_point, const char *version, unsigned int flags)
{
	ID3DBlob *shader;
	ID3DBlob *error_msg;
	HRESULT hr;
	hr = D3DCompileFromFile(char_to_wchar(path), 0, 0, entry_point, version, flags, 0, &shader, &error_msg);
	if (hr != S_OK)
	{
		VP_FATAL("Failed to compile shader: %s", error_msg->GetBufferPointer());
	}
	return shader;
}

Shaders compile_shaders()
{
	Shaders result = {};
	unsigned int compile_flags = 0;
#if defined(VIPOC_DEBUG)
	compile_flags |= D3DCOMPILE_DEBUG;
#endif

	// TODO(Vasko): hard code shaders
	char output[MAX_PATH] = {};
	platform_get_absolute_path(output);

	char vert_path[MAX_PATH] = {};
	vstd_strcat(vert_path, output);
	vstd_strcat(vert_path, "engine\\src\\renderer\\d3d_shaders\\shader.hlsl");
	result.vertex = compile_shader(vert_path, "vs_main", "vs_5_0", compile_flags);

	char pixel_path[MAX_PATH] = {};
	vstd_strcat(pixel_path, output);
	vstd_strcat(pixel_path, "engine\\src\\renderer\\d3d_shaders\\shader.hlsl");
	result.pixel = compile_shader(pixel_path, "ps_main", "ps_5_0", compile_flags);
	return result;
}

unsigned int get_multisample_quality_level()
{
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS feature = {
		DXGI_FORMAT_B8G8R8A8_UNORM,
		1,
		D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE};
	HRESULT hr = renderer.device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &feature, sizeof(feature));
	if (hr != S_OK)
	{
		VP_ERROR("Couldn't queary multisample quality level");
		return 0;
	}
	return feature.NumQualityLevels != 0 ? feature.NumQualityLevels - 1 : 0;
}

void renderer_init()
{
	renderer.win32_state = (Win32_State *)platform_get_state();
	HRESULT hr;

	// TODO(Vasko): look at this

	unsigned int dxgi_factory_flags = 0;

#if defined(VIPOC_DEBUG)
	{

		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&renderer.debug))))
		{
			renderer.debug->EnableDebugLayer();
			dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	hr = CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&renderer.factory));
	CHECK(hr, "Couldn't create dxgi factory");

	hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&renderer.device));
	if (renderer.device == vp_nullptr)
	{
		VP_FATAL("Coudldn't get d3d 12 device", hr);
	}

	renderer.command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hr = renderer.device->CreateCommandQueue(&renderer.command_queue_desc, IID_PPV_ARGS(&renderer.command_queue));
	CHECK(hr, "Failed to create command queue");

	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
	swap_chain_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.SampleDesc.Quality = get_multisample_quality_level();
	swap_chain_desc.BufferCount = BUFFER_COUNT;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	IDXGISwapChain1 *old_swap_chain;
	hr = renderer.factory->CreateSwapChainForHwnd(renderer.command_queue, renderer.win32_state->window, &swap_chain_desc, 0, 0, &old_swap_chain);
	CHECK(hr, "Failed to create a old swap chain");

	hr = old_swap_chain->QueryInterface(IID_PPV_ARGS(&renderer.swap_chain));
	CHECK(hr, "Failed to create new swap chain");

	CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc;
	root_signature_desc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob *root_signature_blob;
	ID3DBlob *root_signature_error_blob;

	hr = D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &root_signature_blob, &root_signature_error_blob);
	CHECK(hr, "Failed to serialize root signature");

	hr = renderer.device->CreateRootSignature(0, root_signature_blob->GetBufferPointer(), root_signature_blob->GetBufferSize(), IID_PPV_ARGS(&renderer.root_signature));
	CHECK(hr, "Failed to create root signature");

	Shaders shaders = compile_shaders();

	// TODO(Vasko): More research on this pipeline description stuff
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_desc = {};
	pipeline_desc.VS.pShaderBytecode = shaders.vertex->GetBufferPointer();
	pipeline_desc.VS.BytecodeLength = shaders.vertex->GetBufferSize();
	pipeline_desc.PS.pShaderBytecode = shaders.pixel->GetBufferPointer();
	pipeline_desc.PS.BytecodeLength = shaders.pixel->GetBufferSize();
	pipeline_desc.pRootSignature = renderer.root_signature;
	pipeline_desc.NumRenderTargets = 1;
	pipeline_desc.RTVFormats[0] = swap_chain_desc.Format;
	pipeline_desc.SampleDesc = swap_chain_desc.SampleDesc;
	pipeline_desc.SampleMask = UINT_MAX; // 0xFFFFFFFF
	pipeline_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipeline_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	pipeline_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	pipeline_desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	pipeline_desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	hr = renderer.device->CreateGraphicsPipelineState(&pipeline_desc, IID_PPV_ARGS(&renderer.pipeline));
	CHECK(hr, "Failed to create graphics pipeline state");

	D3D12_DESCRIPTOR_HEAP_DESC desc_heap_desc = {};
	desc_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc_heap_desc.NumDescriptors = BUFFER_COUNT;

	hr = renderer.device->CreateDescriptorHeap(&desc_heap_desc, IID_PPV_ARGS(&renderer.rtv_desc_heap));
	CHECK(hr, "Failed to create descriptor heap");

	D3D12_CPU_DESCRIPTOR_HANDLE render_target_descriptor = renderer.rtv_desc_heap->GetCPUDescriptorHandleForHeapStart();
	size_t render_target_desc_size = renderer.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (size_t i = 0; i < BUFFER_COUNT; ++i)
	{
		ID3D12Resource *buffer;
		hr = renderer.swap_chain->GetBuffer(i, IID_PPV_ARGS(&buffer));
		CHECK(hr, "Failed to get render buffer");
		renderer.render_target_buffers[i] = buffer;
		renderer.device->CreateRenderTargetView(buffer, 0, render_target_descriptor);
		renderer.render_target_descs[i] = render_target_descriptor;
		render_target_descriptor.ptr += render_target_desc_size;

		hr = renderer.device->CreateCommandAllocator(renderer.command_queue_desc.Type, IID_PPV_ARGS(&renderer.command_allocators[i]));
		CHECK(hr, "Failed to create a command allocator");
	}

	return;
}

#endif