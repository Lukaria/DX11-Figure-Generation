//--------------------------------------------------------------------------------------
// File: Tutorial06.cpp
//
// This application demonstrates simple lighting in the vertex shader
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include "resource.h"



//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
    XMFLOAT3 Pos;
    XMFLOAT3 Normal;
};


struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMFLOAT4 vLightDir[3];
	XMFLOAT4 vLightColor[3];
	XMFLOAT4 vOutputColor;
};


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE               g_hInst = NULL;
HWND                    g_hWnd = NULL;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pd3dDevice = NULL; 
ID3D11DeviceContext*    g_pImmediateContext = NULL;
IDXGISwapChain*         g_pSwapChain = NULL;
ID3D11RenderTargetView* g_pRenderTargetView = NULL;
ID3D11Texture2D*        g_pDepthStencil = NULL;
ID3D11DepthStencilView* g_pDepthStencilView = NULL;

ID3D11VertexShader*     g_pVertexShader = NULL;
ID3D11PixelShader*      g_pPixelShader = NULL;
ID3D11PixelShader*      g_pPixelShaderSolid = NULL; //ps for light
ID3D11InputLayout*      g_pVertexLayout = NULL;
ID3D11Buffer*           g_pVertexBuffer = NULL;
ID3D11Buffer*           g_pIndexBuffer = NULL;
ID3D11Buffer*           g_pConstantBuffer = NULL;
XMMATRIX                g_World;
XMMATRIX                g_View;
XMMATRIX                g_Projection;


//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
void Render();

//rules of nature
#include <iostream>
XMMATRIX g_World2;
ID3D11Buffer* g_pVertexBuffer2 = NULL;
ID3D11Buffer* g_pIndexBuffer2 = NULL;
ID3D11Buffer* g_pConstantBuffer2 = NULL;

SimpleVertex vertices2[60];
WORD indices2[60];

HRESULT figure2();
XMMATRIX g_World1;//delete this
float Radius = 5.0f;
float Side = 4 * Radius / sqrt(2 * (5 + sqrt(5)));
float Height = Side * sqrt(3) / 2;
float Length = Radius - Height / 2;
float X = 0.0f;
float Y = 0.0f;
float Z = 0.0f;
float Alpha = 0.0f;

XMFLOAT3 getNormalized(XMFLOAT3 vector) {
    float length = sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
    return XMFLOAT3{ vector.x / length, vector.y / length, vector.z / length };
}

XMFLOAT4 getNormalized4(XMFLOAT4 vector) {
    float length = sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
    return XMFLOAT4{ vector.x / length, vector.y / length, vector.z / length, 1.0f };
}

XMFLOAT3 getNormal(const XMFLOAT3 dot1, const XMFLOAT3 dot2, const XMFLOAT3 dot3) {
    float res_x, res_y, res_z;
    res_x = (dot2.y - dot1.y) * (dot3.z - dot1.z) - ((dot2.z - dot1.z) * (dot3.y - dot1.y));
    res_y = (dot2.x - dot1.x) * (dot3.z - dot1.z) - ((dot2.z - dot1.z) * (dot3.x - dot1.x));
    res_z = (dot2.x - dot1.x) * (dot3.y - dot1.y) - ((dot2.y - dot1.y) * (dot3.x - dot1.x));
    //return getNormalized(XMFLOAT3{ res_y, -res_x, res_z});
    return getNormalized(XMFLOAT3{ res_y, -res_x, res_z});
}
XMFLOAT3 array[14];


HRESULT figure2() {
    //формирование точек

    for (int i = 0, j = 0; i < 13; i++) {
        if (i == 12) {
            array[i] = {0.0f, 2 * Length + Height / 2, 0.0f};

            array[i + 1] = {0.0f, 0.0f, 0.0f};
            break;
        }
        else {
            if (i % 2 == 0) {
                Y = Length + Height / 2;
                Alpha = 0.0f;
            }
            else {
                Y = Length;
                Alpha = -XM_2PI / 10;
                ++j;
            }

            X = (Side / 2) * cos(XM_2PI / 5 * j + Alpha);
            Z = (Side / 2) * sin(XM_2PI / 5 * j + Alpha);

            array[i] = {X, Y, Z};
        };
    }

    //формирование нормалей
    //средние нормали
    XMFLOAT3 normals[20];
    for (int i = 0; i < 10; i++) {
        if(i%2)
        normals[i] = getNormal(array[i], array[i+1], array[i+2]);
        else
        normals[i] = getNormal(array[i], array[i+2], array[i+1]);
    }
    for (int i = 10, j = 0, k = 1; i < 20; i++) {
        //верхние нормали
        if (i < 15) {
            normals[i] = getNormal(array[j+2], array[12], array[j]);
            j += 2;
        }
        //нижние нормали
        else {
            normals[i] = getNormal(array[k], array[13], array[k+2]);
            k += 2;

        }
    }

    //формирование вершин
    //формирование боковых вершин
    for (int i = 0, j = 0, k = 0; i < 30; i += 3, j += 1, k++) {
        vertices2[i] = { array[j], normals[k] };
        vertices2[i + 1] = { array[j + 1], normals[k] };
        vertices2[i + 2] = { array[j + 2], normals[k] };
    } 

    for (int i = 30, j = 0, c = 1, k = 10; i < 60; k++, i+=3) {
        //верхние вершины
        if (i < 45) {
            vertices2[i] = { array[j], normals[k] };
            vertices2[i + 1] = { array[12], normals[k] };
            vertices2[i + 2] = { array[j + 2], normals[k] };
            j += 2;
        }
        //нижние вершины
        else {          
            vertices2[i] = { array[c], normals[k] };
            vertices2[i + 1] = { array[13], normals[k] };
            vertices2[i + 2] = { array[c + 2], normals[k] };
            c += 2;
        }
    }

    //формирование индексов
    //боковые индексы
    for (int i = 0, j = 0; i < 30; i += 3, j++) {
        if (j % 2 == 0) {
            indices2[i] = i;
            indices2[i + 1] = i + 2;
            indices2[i + 2] = i + 1;
        }
        else {
            indices2[i] = i;
            indices2[i + 1] = i + 1;
            indices2[i + 2] = i + 2;
        }
    }

    for (int i = 30, j = 0; i < 60; i += 3, j++) {
        //верхние индексы
        if (j < 5) {
            indices2[i] = i;
            indices2[i + 1] = i + 1;
            indices2[i + 2] = i + 2;
        }
        //нижние индексы
        else {
            indices2[i] = i;
            indices2[i + 1] = i + 2;
            indices2[i + 2] = i + 1;
        }

    }

    HRESULT hr = S_OK;

    D3D11_BUFFER_DESC bd2;
    ZeroMemory(&bd2, sizeof(bd2));
    bd2.Usage = D3D11_USAGE_DEFAULT;
    bd2.ByteWidth = sizeof(SimpleVertex) * 60;
    bd2.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd2.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData2;
    ZeroMemory(&InitData2, sizeof(InitData2));
    InitData2.pSysMem = vertices2;
    hr = g_pd3dDevice->CreateBuffer(&bd2, &InitData2, &g_pVertexBuffer2);
    if (FAILED(hr))
        return hr;
    bd2.Usage = D3D11_USAGE_DEFAULT;
    bd2.ByteWidth = sizeof(WORD) * 60;
    bd2.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd2.CPUAccessFlags = 0;
    InitData2.pSysMem = indices2;
    hr = g_pd3dDevice->CreateBuffer(&bd2, &InitData2, &g_pIndexBuffer2);
    if (FAILED(hr))
        return hr;
    // Create the constant buffer
    bd2.Usage = D3D11_USAGE_DEFAULT;
    bd2.ByteWidth = sizeof(ConstantBuffer);
    bd2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd2.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer(&bd2, NULL, &g_pConstantBuffer2);
    if (FAILED(hr))
        return hr;
}


//camera control
float x = 0.0f, y = 0.0f, z = 0.0f, povorotX = 0.0f, povorotY = 0.0f, povorotZ = 0.0f;
XMVECTOR Eye, Up, At;
XMMATRIX RotationX, RotationY, RotationZ;
void cameraView() {
    RotationX = XMMatrixRotationX(povorotX);
    RotationY = XMMatrixRotationY(povorotY);
    RotationZ = XMMatrixRotationZ(povorotZ);
    if (GetAsyncKeyState('S')) {
        y -= 0.01f;
    }
    else if (GetAsyncKeyState('W')) {
        y += 0.01f;
    }
    else if (GetAsyncKeyState('Q')) {
        z -= 0.1f;
    }
    else if (GetAsyncKeyState('E')) {
        z += 0.1f;
    }
    else if (GetAsyncKeyState('A')) {
        x -= 0.1f;
    }
    else if (GetAsyncKeyState('D')) {
        x += 0.1f;
    }
    else if (GetAsyncKeyState(VK_LBUTTON)) {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        povorotX = 0.0f;
        povorotY = 0.0f;
        povorotZ = 0.0f;

    }
    else if (GetAsyncKeyState('N')) {
        povorotX += 0.001f;
    }
    else if (GetAsyncKeyState('M')) {
        povorotX -= 0.001f;
    }
    else if (GetAsyncKeyState('L')) {
        povorotY += 0.001f;
    }
    else if (GetAsyncKeyState('J')) {
        povorotY -= 0.001f;
    }
    else if (GetAsyncKeyState('I')) {
        povorotZ += 0.001f;
    }
    else if (GetAsyncKeyState('K')) {
        povorotZ -= 0.001f;
    }

    Eye = XMVectorSet(x * 0.1f, 4.0f + y, -15.0f + z * 0.1f, 0.0f);
    At = XMVectorSet(x * 0.1f, 1.0f + y, z * 0.1f, 0.0f);
    Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    Eye = XMVector3TransformCoord(Eye, RotationX * RotationY * RotationZ);
    At = XMVector3TransformCoord(At, RotationX * RotationY * RotationZ);
    Up = XMVector3TransformCoord(Up, RotationX * RotationY * RotationZ);
    g_View = XMMatrixLookAtLH(Eye, At, Up);

}



//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

    if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
        return 0;

    if( FAILED( InitDevice() ) )
    {
        CleanupDevice();
        return 0;
    }

    // Main message loop
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            Render();
        }
    }

    CleanupDevice();

    return ( int )msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon( hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window
    g_hInst = hInstance;
    RECT rc = { 0, 0, 640, 480 };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( L"TutorialWindowClass", L"Direct3D 11 Tutorial 6", WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
                           NULL );
    if( !g_hWnd )
        return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile( szFileName, NULL, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL )
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        if( pErrorBlob ) pErrorBlob->Release();
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect( g_hWnd, &rc );
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE( driverTypes );

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
	UINT numFeatureLevels = ARRAYSIZE( featureLevels );

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain( NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
                                            D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );
        if( SUCCEEDED( hr ) )
            break;
    }
    if( FAILED( hr ) )
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    if( FAILED( hr ) )
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRenderTargetView );
    pBackBuffer->Release();
    if( FAILED( hr ) )
        return hr;

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory( &descDepth, sizeof(descDepth) );
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = g_pd3dDevice->CreateTexture2D( &descDepth, NULL, &g_pDepthStencil );
    if( FAILED( hr ) )
        return hr;

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory( &descDSV, sizeof(descDSV) );
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = g_pd3dDevice->CreateDepthStencilView( g_pDepthStencil, &descDSV, &g_pDepthStencilView );
    if( FAILED( hr ) )
        return hr;

    g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, g_pDepthStencilView );

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports( 1, &vp );


	// Compile the vertex shader
	ID3DBlob* pVSBlob = NULL;
    hr = CompileShaderFromFile( L"Tutorial06.fx", "VS", "vs_4_0", &pVSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader );
	if( FAILED( hr ) )
	{	
		pVSBlob->Release();
        return hr;
	}

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE( layout );

    // Create the input layout
	hr = g_pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
                                          pVSBlob->GetBufferSize(), &g_pVertexLayout );
	pVSBlob->Release();
	if( FAILED( hr ) )
        return hr;

    // Set the input layout
    g_pImmediateContext->IASetInputLayout( g_pVertexLayout );

	// Compile the pixel shader for main figure
	ID3DBlob* pPSBlob = NULL;
    hr = CompileShaderFromFile( L"Tutorial06.fx", "PS", "ps_4_0", &pPSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader );
	pPSBlob->Release();
    if( FAILED( hr ) )
        return hr;

	// Compile the pixel shader for light source (PSSolid)
	pPSBlob = NULL;
	hr = CompileShaderFromFile( L"Tutorial06.fx", "PSSolid", "ps_4_0", &pPSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShaderSolid );
	pPSBlob->Release();
    if( FAILED( hr ) )
        return hr;

    // Create vertex buffer
    SimpleVertex vertices[] =
    {
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },

        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        //
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },

        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },

        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },

        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
    };

    D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( SimpleVertex ) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory( &InitData, sizeof(InitData) );
    InitData.pSysMem = vertices;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer );
    if( FAILED( hr ) )
        return hr;

    // Create index buffer
    // Create vertex buffer
    WORD indices[] =
    {
        3,1,0,
        2,1,3,

        6,4,5,
        7,4,6,

        11,9,8,
        10,9,11,

        14,12,13,
        15,12,14,

        19,17,16,
        18,17,19,

        22,20,21,
        23,20,22
    };
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( WORD ) * 36;        // 36 vertices needed for 12 triangles in a triangle list
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pIndexBuffer );
    if( FAILED( hr ) )
        return hr;




	// Create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pConstantBuffer );
    if( FAILED( hr ) )
        return hr;

    //---------------------------------
    //generate mesh

    figure2();

    //----------------------------------------------------
    // Initialize the world matrices
    g_World = XMMatrixIdentity();
    g_World2 = XMMatrixIdentity();
    // Set primitive topology
    g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



    // Initialize the projection matrix
	g_Projection = XMMatrixPerspectiveFovLH( XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f );



    return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
    if( g_pImmediateContext ) g_pImmediateContext->ClearState();
    if( g_pConstantBuffer ) g_pConstantBuffer->Release();
    if( g_pVertexBuffer ) g_pVertexBuffer->Release();
    if( g_pIndexBuffer ) g_pIndexBuffer->Release();
    //
    if (g_pConstantBuffer2) g_pConstantBuffer2->Release();
    if (g_pVertexBuffer2) g_pVertexBuffer2->Release();
    if (g_pIndexBuffer2) g_pIndexBuffer2->Release();
    //
    if( g_pVertexLayout ) g_pVertexLayout->Release();
    if( g_pVertexShader ) g_pVertexShader->Release();
    if( g_pPixelShaderSolid ) g_pPixelShaderSolid->Release();
    if( g_pPixelShader ) g_pPixelShader->Release();
    if( g_pDepthStencil ) g_pDepthStencil->Release();
    if( g_pDepthStencilView ) g_pDepthStencilView->Release();
    if( g_pRenderTargetView ) g_pRenderTargetView->Release();
    if( g_pSwapChain ) g_pSwapChain->Release();
    if( g_pImmediateContext ) g_pImmediateContext->Release();
    if( g_pd3dDevice ) g_pd3dDevice->Release();

   
}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
        case WM_PAINT:
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}


//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------

void Render()
{
    // Update our time
    static float t = 0.0f;
    if( g_driverType == D3D_DRIVER_TYPE_REFERENCE )
    {
        t += ( float )XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();
        if( dwTimeStart == 0 )
            dwTimeStart = dwTimeCur;
        t = ( dwTimeCur - dwTimeStart ) / 1000.0f;
    }

    // Initialize the view matrix
    cameraView();


    // Rotate cube around the origin
	//g_World = XMMatrixRotationY( t );

    // Setup our lighting parameters
    XMFLOAT4 vLightDirs[3] =
    {
        getNormalized4(XMFLOAT4{ 0.0f, .5f, 0.0f, 1.0f }),
        getNormalized4(XMFLOAT4{ 0.0f, 0.0f, -1.0f, 1.0f }),
        getNormalized4(XMFLOAT4{-1.0f, 0.0f, 0.0f, 1.0f }),
    };
    XMFLOAT4 vLightColors[3] =
    {
        XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ),
        XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ),
        XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f )
    };
   
    // Rotate the second light around the origin
    XMMATRIX mRotate = XMMatrixRotationZ(1.0f * t);
    XMVECTOR vLightDir = XMLoadFloat4(&vLightDirs[0]); //current pos
    vLightDir = XMVector3Transform(vLightDir, mRotate);
    XMStoreFloat4(&vLightDirs[0], vLightDir);
	mRotate = XMMatrixRotationY(-1.0f * t );
	vLightDir = XMLoadFloat4( &vLightDirs[1] ); //current pos
	vLightDir = XMVector3Transform( vLightDir, mRotate );
	XMStoreFloat4( &vLightDirs[1], vLightDir );
    mRotate = XMMatrixRotationZ(1.0f * t);
    vLightDir = XMLoadFloat4(&vLightDirs[2]); //current pos
    vLightDir = XMVector3Transform(vLightDir, mRotate);
    XMStoreFloat4(&vLightDirs[2], vLightDir);

	//
    // Clear the back buffer
    //
    float ClearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; // red, green, blue, alpha
    g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, ClearColor );

    //
    // Clear the depth buffer to 1.0 (max depth)
    //
    g_pImmediateContext->ClearDepthStencilView( g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );

    //
    // Update matrix variables and lighting variables
    //
    ConstantBuffer cb1;
    cb1.mWorld = XMMatrixTranspose(g_World2);
    cb1.mView = XMMatrixTranspose(g_View);
    cb1.mProjection = XMMatrixTranspose(g_Projection);
	cb1.vLightDir[0] = vLightDirs[0];
	cb1.vLightDir[1] = vLightDirs[1];
	cb1.vLightDir[2] = vLightDirs[2];
	cb1.vLightColor[0] = vLightColors[0];
	cb1.vLightColor[1] = vLightColors[1];
	cb1.vLightColor[2] = vLightColors[2];
	cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);
	

    //
    // Render the figure
    //
    g_pImmediateContext->UpdateSubresource(g_pConstantBuffer2, 0, NULL, &cb1, 0, 0);
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;

    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer2, &stride, &offset);
    g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer2, DXGI_FORMAT_R16_UINT, 0);


	g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
	g_pImmediateContext->VSSetConstantBuffers( 0, 1, &g_pConstantBuffer2);
	g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
	g_pImmediateContext->PSSetConstantBuffers( 0, 1, &g_pConstantBuffer2);

    g_World2 = XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixRotationY(0.0f) *
        XMMatrixTranslation(0.0f, -3.0f, 0.0f);
    g_pImmediateContext->DrawIndexed(60, 0, 0);


    //back
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
    g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
    g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

    //
    // Render each light
    //
    for( int m = 0; m < 3; m++ )
    {
		XMMATRIX mLight = XMMatrixTranslationFromVector( 5.0f * XMLoadFloat4( &vLightDirs[m] ) );
		XMMATRIX mLightScale = XMMatrixScaling( 0.2f, 0.2f, 0.2f );
        mLight = mLightScale * mLight;

        // Update the world variable to reflect the current light
		cb1.mWorld = XMMatrixTranspose( mLight );
		cb1.vOutputColor = vLightColors[m];
		g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cb1, 0, 0 );

		g_pImmediateContext->PSSetShader( g_pPixelShaderSolid, NULL, 0 );
		g_pImmediateContext->DrawIndexed( 36, 0, 0 );
    }

    

    //
    // Present our back buffer to our front buffer
    //
    g_pSwapChain->Present( 0, 0 );
}
