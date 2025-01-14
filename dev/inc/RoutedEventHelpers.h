﻿// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

// These helpers are for attaching to UIElement AddHandler RoutedEvents. In C++/WinRT the pattern
// is more obtuse than in C# because you have to manually box the delegate. But even in C# the pattern
// is not type-safe. These helpers do two things:
// 1) Add type safety by templating over an enum of the supported RoutedEvent types to make sure that the
//    event and the delegate are always correctly paired.
// 2) Adds auto_revoke semantics to AddHandler to avoid manual RemoveHandler calls and manual tracking
//    of the boxed delegate.


struct RoutedEventHandler_revoker
{
    RoutedEventHandler_revoker() noexcept = default;
    RoutedEventHandler_revoker(RoutedEventHandler_revoker const&) = delete;
    RoutedEventHandler_revoker& operator=(RoutedEventHandler_revoker const&) = delete;
    RoutedEventHandler_revoker(RoutedEventHandler_revoker&& other)
    {
        move_from(other);
    }

    RoutedEventHandler_revoker& operator=(RoutedEventHandler_revoker&& other)
    {
        move_from(other);
        return *this;
    }

    RoutedEventHandler_revoker(winrt::UIElement const& object, winrt::RoutedEvent event, winrt::IInspectable handler) :
        m_object(object),
        m_event(std::move(event)),
        m_handler(std::move(handler))
    {}

    ~RoutedEventHandler_revoker() noexcept
    {
        revoke();
    }

    void revoke() noexcept
    {
        if (!m_object)
        {
            return;
        }

        if (auto object = m_object.get())
        {
            object.RemoveHandler(m_event, m_handler);
        }

        m_object = nullptr;
    }

    explicit operator bool() const noexcept
    {
        return static_cast<bool>(m_object);
    }
private:
    void move_from(RoutedEventHandler_revoker& other)
    {
        if (this != &other)
        {
            revoke();
            std::swap(m_object, other.m_object);
            std::swap(m_event, other.m_event);
            std::swap(m_handler, other.m_handler);
        }
    }

    weak_ref<winrt::UIElement> m_object;
    winrt::RoutedEvent m_event{ nullptr };
    winrt::IInspectable m_handler{};
};

// Enum to help with type traits
enum class RoutedEventType
{
    GettingFocus,
    LosingFocus,
    KeyDown,
};

template<RoutedEventType eventType>
struct RoutedEventTraits
{
};

template <>
struct RoutedEventTraits<RoutedEventType::GettingFocus>
{
    static winrt::RoutedEvent Event() { return winrt::UIElement::GettingFocusEvent(); }
    using HandlerT = winrt::TypedEventHandler<winrt::UIElement, winrt::GettingFocusEventArgs>;
};

template <>
struct RoutedEventTraits<RoutedEventType::LosingFocus>
{
    static winrt::RoutedEvent Event() { return winrt::UIElement::LosingFocusEvent(); }
    using HandlerT = winrt::TypedEventHandler<winrt::UIElement, winrt::LosingFocusEventArgs>;
};

template <>
struct RoutedEventTraits<RoutedEventType::KeyDown>
{
    static winrt::RoutedEvent Event() { return winrt::UIElement::KeyDownEvent(); }
    using HandlerT = winrt::KeyEventHandler;
};

template<RoutedEventType eventType, typename traits = RoutedEventTraits<eventType>>
inline RoutedEventHandler_revoker AddRoutedEventHandler(winrt::UIElement const& object, typename traits::HandlerT const& callback, bool handledEventsToo)
{
    auto handler = winrt::box_value<traits::HandlerT>(callback);
    auto event = traits::Event();
    object.AddHandler(event, handler, handledEventsToo);
    return { object, event, handler };
}
