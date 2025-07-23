#pragma once
#include <QObject>
#include <QString>
#include <QLabel>
#include <QFont>
#include <string>
#include <vector>

#include "Detect_Element.hpp"

namespace SupportFunction {
	std::vector<DetectElement> dataInit();
	std::vector<std::pair<std::wstring, HWND>> getAllWindowsTitle();



    // Worker 類別，專門處理文字滾動的邏輯
    class TextScroller : public QObject
    {
        Q_OBJECT

    public:
        // 建構函式接收所有需要的資訊
        TextScroller(QLabel* label, QFontMetrics fontMetrics, QString text, int ui_width, int ui_height, QObject* parent = nullptr);

    public slots:
        // 這個函式將在新的執行緒中被呼叫
        void run();
        // 這個槽函式用來從外部停止滾動迴圈
        void stop();

    signals:
        // 當工作完成時，發射此訊號通知主執行緒
        void finished();
        // 用於在執行緒中安全地更新 QLabel 的位置
        void updateLabelPosition(int x, int y);

    private:
        QLabel* m_label;
        QFontMetrics m_fontMetrics;
        QString m_text;
        int m_uiWidth, m_uiHeight;
        volatile bool m_running; // 使用 volatile 確保執行緒間的可見性
    };
}