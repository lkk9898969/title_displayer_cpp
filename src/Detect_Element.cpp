#include "Detect_Element.hpp"
#include <stdexcept>

// 用來傳遞資料給 EnumWindows 的結構
struct EnumData {
    std::vector<std::wstring> filters;
    HWND hwnd = nullptr;
};

// EnumWindows 的回呼函式
static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    EnumData* data = reinterpret_cast<EnumData*>(lParam);
    const int bufferSize = 256;
    wchar_t windowTitle[bufferSize];

    if (GetWindowTextW(hwnd, windowTitle, bufferSize) > 0) {
        std::wstring title(windowTitle);
		bool match = true;
        for (const auto& filter : data->filters) {
            if (title.find(filter) == std::wstring::npos) {
                match = false; // 如果標題中不包含過濾字串，則不匹配
				break;
            }
        }
        if (match) {
            data->hwnd = hwnd;
            return FALSE; // 找到就停止列舉
        }
    }
    return TRUE; // 繼續列舉
}

DetectElement::DetectElement(std::wstring text, std::vector<std::wstring> filters)
    : m_text(std::move(text)), m_filters(std::move(filters)) {
    if (m_filters.empty()) {
        throw std::invalid_argument("Filters cannot be empty.");
    }
}

DetectElement::DetectElement(std::wstring text, std::wstring title)
    : m_text(std::move(text)){
    if (title.empty()) {
        throw std::invalid_argument("Title cannot be empty.");
	}
	EnumData data;
    data.filters.push_back(std::move(title));
	data.filters.push_back(std::wstring());
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&data));
    m_hwnd = data.hwnd;
    if (!m_hwnd) {
        throw std::runtime_error("Window not found.");
	}
}

DetectElement::DetectElement(std::wstring text, std::nullptr_t null_)
	: m_text(std::move(text)), m_filters()  // 空的 filters
{
    if (null_ != nullptr) {
        throw std::invalid_argument("Null pointer should not be used with this constructor.");
    }
	// m_filters remains empty
}

HWND DetectElement::findWindow() {
    if (m_filters.empty()) {
		return nullptr;
	}
    EnumData data;
    std::vector<std::wstring> filters;
	filters.assign(m_filters.begin(), m_filters.end());
	filters.push_back(std::wstring());
    data.filters = filters;
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&data));
    m_hwnd = data.hwnd;
    return m_hwnd;
}

std::wstring DetectElement::getOriginalWindowTitle() {
    if (!m_hwnd) {
        findWindow();
    }
    if (!m_hwnd) {
        throw std::runtime_error("Window not found.");
    }
    const int bufferSize = 512;
    wchar_t windowTitle[bufferSize];
    GetWindowTextW(m_hwnd, windowTitle, bufferSize);
    return std::wstring(windowTitle);
}

std::wstring DetectElement::getFilteredWindowTitle() {
    std::wstring title = getOriginalWindowTitle();
    size_t pos, minPos = title.length();
    for (const auto& filter : m_filters) {
        if ((pos = title.find(filter)) != std::wstring::npos && (pos < minPos)) {
            minPos = pos;
		}
    }
    title = title.substr(0, minPos);
    return title;
}