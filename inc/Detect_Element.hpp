// detectelement.h
#pragma once
#include <string>
#include <vector>
#include <Windows.h>

class DetectElement {

public:

    DetectElement(std::wstring text, std::vector<std::wstring> filters);
    DetectElement(std::wstring text, std::wstring title);
    DetectElement(std::wstring text, std::nullptr_t null_);

    HWND findWindow();
    std::wstring getOriginalWindowTitle();
    std::wstring getFilteredWindowTitle();
	std::wstring getText() const { return m_text; }
	HWND getHwnd() const { return m_hwnd; }

    operator bool() const {
        return !m_text.empty() && !m_filters.empty();
	}

private:

    std::wstring m_text;
    std::vector<std::wstring> m_filters;
    HWND m_hwnd = nullptr;
};