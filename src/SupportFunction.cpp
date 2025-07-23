
#include <QLabel>
#include <QThread>
#include <QString>

#include "SupportFunction.hpp"

struct AllWindowsData {
	std::vector<std::pair<std::wstring, HWND>> data;
};


namespace SupportFunction {

	static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
		AllWindowsData* data = reinterpret_cast<AllWindowsData*>(lParam);
		// Get the window title length
		int length = GetWindowTextLength(hwnd);

		// If the window has a title and is visible, retrieve it
		if (length > 0 && IsWindowVisible(hwnd)) {
			std::vector<wchar_t> buffer(length + 2);
			GetWindowTextW(hwnd, buffer.data(), length + 1);
			data->data.emplace_back(buffer.data(), hwnd);
		}
		return TRUE; // 繼續列舉
	}

	std::vector<DetectElement> dataInit() {
		std::vector<DetectElement> data;
		data.emplace_back(L"Youtube", std::vector<std::wstring>{L" - YouTube", L" - Microsoft​ Edge"});
		data.emplace_back(L"VLC", std::vector<std::wstring>{L" - VLC"});
		return data;
	}

	std::vector<std::pair<std::wstring, HWND>> getAllWindowsTitle()
	{
		AllWindowsData titles;
		EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&titles));
		return { std::move(titles.data) };
	}

#pragma region TextScroller

	TextScroller::TextScroller(QLabel* label, QFontMetrics fontMetrics, QString text, int ui_width, int ui_height , QObject* parent)
		: QObject(parent), m_label(label), m_fontMetrics(fontMetrics), m_text(text), m_running(false), m_uiWidth(ui_width), m_uiHeight(ui_height)
	{
	}

	void TextScroller::stop()
	{
		m_running = false;
	}

	void TextScroller::run()
	{
		m_running = true;
		int textWidth = m_fontMetrics.horizontalAdvance(m_text);

		// 如果文字寬度沒有超過 Label 寬度，則不滾動
		if (textWidth <= m_uiWidth) {
			// 仍然需要設定幾何尺寸
			m_label->setGeometry(0, 0, textWidth, m_uiHeight);
			emit finished();
			return;
		}

		while (m_running) {
			// 重設到初始位置
			emit updateLabelPosition(0, 0);
			QThread::sleep(1); // 等待 1 秒

			if (!m_running) break;

			// 從右向左滾動
			for (int i = 0; i <= textWidth - m_uiWidth; ++i) {
				if (!m_running) break;
				emit updateLabelPosition(-i, 0);
				QThread::msleep(30); // 等待 30 毫秒
			}

			if (!m_running) break;
			QThread::sleep(1); // 滾動完畢後等待 1 秒
		}

		emit finished(); // 迴圈結束後發射 finished 訊號
	}

#pragma endregion
}