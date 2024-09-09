
#include <d3d12.h>
#include <wrl/client.h>

class Renderer
{

public:
private:
    Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;
    Microsoft::WRL::ComPtr<ID3D12DeviceContext> context = nullptr;
}