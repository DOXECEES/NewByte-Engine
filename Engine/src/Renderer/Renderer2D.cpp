#include "Renderer2D.hpp"

Renderer2D::Renderer2D()
{
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);
}

Renderer2D::~Renderer2D()
{
    rbrush->Release();
    borderBrush->Release();
    renderTarget->Release();
    factory->Release();
}

void Renderer2D::createResources(HWND hwnd)
{
    if (!renderTarget)
    {
        RECT rc;
        GetClientRect(currentHwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(
            rc.right - rc.left,
            rc.bottom - rc.top);

        factory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(
                currentHwnd, size),
            &renderTarget);

        renderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::DimGray),
            &rbrush);

        renderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Black),
            &borderBrush);
    }
}

void Renderer2D::resize(const UINT32 width, const UINT32 height) noexcept
{
    if (renderTarget)
    {
        D2D1_SIZE_U size = D2D1::SizeU(width, height);
        if (renderTarget->Resize(size) != S_OK)
        {
            renderTarget->Release();
        }
    }
}

void Renderer2D::changeBrush()
{
    renderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Green),
        &rbrush);
}

void Renderer2D::drawQuad()
{

    createResources(currentHwnd);

    renderTarget->BeginDraw();

    renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

    D2D1_RECT_F rectangle = D2D1::RectF(100, 100, 300, 200);
    renderTarget->FillRectangle(&rectangle, rbrush);

    D2D1_POINT_2F begin = {100, 150};
    D2D1_POINT_2F end = {2000, 150};

    renderTarget->DrawLine(begin, end, rbrush);

    D2D1_SIZE_F renderTargetSize = renderTarget->GetSize();

    HRESULT hr = renderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET)
    {
        if (rbrush)
            rbrush->Release();
        if (renderTarget)
            renderTarget->Release();
        rbrush = nullptr;
        renderTarget = nullptr;
    }
}

void Renderer2D::drawUi(Ui *ui)
{
    // createResources(currentHwnd);

    // renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

    D2D1_RECT_F rectangle = D2D1::Rect(ui->aabb.minPoint.x, ui->aabb.minPoint.y, ui->aabb.maxPoint.x, ui->aabb.maxPoint.y);

    // D2D1_RECT_F rectangle = D2D1::Rect(100, 100, 350, 600);
    renderTarget->FillRectangle(&rectangle, rbrush);
    renderTarget->DrawRectangle(&rectangle, borderBrush, 2.0f);

    /*D2D1_POINT_2F begin = { 100, 150 };
    D2D1_POINT_2F end = { 2000, 150 };

    renderTarget->DrawLine(begin, end, rbrush);*/
    D2D1_POINT_2F start = {ui->aabb.minPoint.x, ui->aabb.maxPoint.y / 2};
    D2D1_POINT_2F end = {ui->aabb.minPoint.x + 150, ui->aabb.maxPoint.y / 2};
    D2D1_POINT_2F upperPartOfArrow = {ui->aabb.minPoint.x + 150 - 10, ui->aabb.maxPoint.y / 2 + 10};
    D2D1_POINT_2F lowerPartOfArrow = {ui->aabb.minPoint.x + 150 - 10, ui->aabb.maxPoint.y / 2 - 10};

    renderTarget->DrawLine(start, end, rbrush);
    renderTarget->DrawLine(end, upperPartOfArrow, rbrush);
    renderTarget->DrawLine(end, lowerPartOfArrow, rbrush);

    D2D1_SIZE_F renderTargetSize = renderTarget->GetSize();
}
