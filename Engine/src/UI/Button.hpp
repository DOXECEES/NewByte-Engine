
#pragma once

#include <string>
#include <vector>
#include <array>
#include <string_view>

#include <d2d1.h>

#include "../Math/AABB.hpp"
#include "../Math/Vector2.hpp"
#include "../Renderer/Renderer2D.hpp"

struct Style
{
    int32_t borderSize = 0;
    D2D1::ColorF color = D2D1::ColorF::Black;
};

//
//
// class AABBTree
//{
//	struct Node
//	{
//		Node(Ui* ui)
//			:ui(ui)
//		{
//
//		}
//
//		Ui* ui;
//
//		Node* left = nullptr;
//		Node* right = nullptr;
//
//	};
//
//
// public:
//	AABBTree() = default;
//
//	void insert(Ui* ui)
//	{
//		if (root == nullptr)
//		{
//			root = new Node(ui);
//		}
//
//
//	}
//
//
//	Node* root = nullptr;
//
//};
//
//
// class UiStorage
//{
//
// public:
//	UiStorage() = default;
//
//	inline void addElement(Ui* ui) { container.emplace_back(ui); };
//
//	bool onClick() const { return false; };// TODO
//
//
//
// private:
//	std::vector<Ui*> container;
//
//
//
//};

class Ui
{

public:
    explicit Ui(nb::Math::Vector2 begin, const int32_t width, const int32_t height, std::string_view _text);

    virtual void handler() = 0;

    bool onClick(const nb::Math::Vector2 pos) noexcept;
    inline void setStyle(const Style &newStyle) noexcept { style = newStyle; };

    // void setHandler();

    friend class Renderer2D;

protected:
    nb::Math::AABB aabb;
    Style style = {};
    std::string text;

    // static UiStorage store;
};

class Button : public Ui
{

public:
    explicit Button(nb::Math::Vector2 begin, const int32_t width, const int32_t height, std::string_view _text);

    void handler() override;

private:
private:
};
