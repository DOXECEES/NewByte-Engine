#include "Button.hpp"



Ui::Ui(NBPoint begin, const int32_t width, const int32_t height, std::string_view _text)
	: aabb(begin.x, begin.y, width, height)
	, text(_text)
{
	//store.addElement(this);
}

bool Ui::onClick(const NBPoint pos) noexcept
{
	return aabb.isPointInside(pos);
}


Button::Button(NBPoint begin, const int32_t width, const int32_t height, std::string_view _text)
	: Ui(begin, width, height, _text)
{

}

void Button::handler()
{

}


