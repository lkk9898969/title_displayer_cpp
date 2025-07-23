#pragma once
#include <QEvent>
#include <QMenu>
#include <QThread>
#include <QWidget>
//#include "boost/signals2.hpp"
#include "boost/signals2/signal.hpp"

#include "Detect_Element.hpp"
#include "SupportFunction.hpp"

// 包含由 uic 自動產生的標頭檔
#include "ui_UI_MainWindow.h"
#include "ui_UI_detecting.h"
#include "ui_UI_display_music_name.h"
#include "ui_UI_selecting.h"
#include "ui_UI_titlefilter.h"
#include "ui_UI_titlelist.h"

// 預先宣告 UI 類別，可以加速編譯
namespace Ui {
    // 名稱來自 .ui 檔案中根元件的 objectName
    class UI_MainWindow;
    class UI_Detecting_Window;
    class UI_Main_Display;
    class UI_Select_Window;
    class UI_Title_Filter; 
    class UI_Title_List;
}

class SelectWindowController;
class TitleListWindowController;
class TitleFilterWindowController;
class DetectingWindowController;
class DisplayingWindowController;
template <typename T>
class workerThread;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void customEvent(QEvent* event) override; // 處理自訂事件

    // 覆寫 QWidget 的事件處理函式來實現自訂行為
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    Ui::UI_MainWindow* ui;
    QEvent::Type m_eventType = static_cast<QEvent::Type>(QEvent::registerEventType(QEvent::Type::User + 1)); // 自訂事件類型

    SelectWindowController* m_selectWindowController = nullptr;
    TitleListWindowController* m_titleListWindowController = nullptr;
    TitleFilterWindowController* m_titleFilterWindowController = nullptr;
    DetectingWindowController* m_detectingWindowController = nullptr;
	DisplayingWindowController* m_displayingWindowController = nullptr;

	DetectElement* m_currentElement = nullptr; // 儲存當前選擇的元素

    QAction* m_stayOnTopAction;
    QAction* m_exitAction;

    QMenu m_menu;

    QPointF m_oldPos;   // 用於計算拖曳的位移，使用 QPointF 以支援高 DPI
    bool m_isDragging; // 用於追蹤滑鼠是否正在拖曳視窗

    void showWidget(QWidget* widget);
    void checkElement(DetectElement& element);

    void onDetectCompleted(DetectElement* element);

    // 用於設定 UI 和動作的私有函式
    void setupDisplayingMainWindow();


private slots:
    // 用於處理 "保持在最上層" 動作的槽函式
    void stayOnTopToggle(bool checked);

};


class SelectWindowController : public QWidget
{
    Q_OBJECT

public:
    explicit SelectWindowController(QWidget* parent = nullptr, std::function<void(DetectElement&)> callback = nullptr);
    ~SelectWindowController();

    void addButton(DetectElement& element);

private:
    Ui::UI_Select_Window* ui;
    const QFont m_font = QFont("Microsoft JhengHei", 12); // 設定按鈕字型;

    std::function<void(DetectElement&)> m_onButtonClicked; // 按鈕點擊事件處理函式

};

class TitleListWindowController : public QWidget
{
    Q_OBJECT

public:
    explicit TitleListWindowController(QWidget* parent = nullptr, std::function<void(const HWND)> callback = nullptr);
    ~TitleListWindowController();

    void refreshList();

private:
    Ui::UI_Title_List* ui;
    std::function<void(const HWND)> m_onButtonClicked;

    void onOKClicked(); // 處理 OK 按鈕點擊事件
    void onRefreshClicked(); // 處理刷新按鈕點擊事件
};

class TitleFilterWindowController : public QWidget
{
    Q_OBJECT

public:
    explicit TitleFilterWindowController(QWidget* parent = nullptr);
    ~TitleFilterWindowController();

private:
    Ui::UI_Title_Filter* ui;

    void onEnterClicked(); // 處理按鈕點擊事件
};


class DetectingWindowController : public QWidget
{
    Q_OBJECT

public:
    explicit DetectingWindowController(QWidget* parent = nullptr, DetectElement* element = nullptr);
    ~DetectingWindowController();

    void startDetectingT(std::function<void(DetectElement*)> slot);

private:
    Ui::UI_Detecting_Window* ui;

    workerThread<DetectElement*>* m_workerThread = nullptr;

    DetectElement m_element; // 儲存要檢測的元素

    void DetectingWork();

};

#pragma region WorkerThread

class _workerThread : public QThread
{
    Q_OBJECT
public:
    explicit _workerThread(QObject* parent = nullptr)
        : QThread(parent) {
    }
};

template <typename T>
class workerThread : public _workerThread
{
public:
    workerThread(QObject* parent = nullptr, std::function<T()> task = nullptr);

    void bindSignal(const std::function<void(T)>& slot);
    void unbindSignal(const std::function<void(T)>& slot);
    void unbindAllSignals();

protected:
    void run() override;

private:
    std::function<T()> m_task; // 儲存要執行的任務
	boost::signals2::signal<void(T)> m_signal;
};

template<typename T>
inline workerThread<T>::workerThread(QObject* parent, std::function<T()> task) :
    _workerThread(parent), m_task(), m_signal()
{
    if (!task) {
        qDebug() << "Error: Task function cannot be null.";
        throw std::invalid_argument("Task function cannot be null");
	}
	m_task = std::move(task); // 儲存任務函式
}

template<typename T>
inline void workerThread<T>::bindSignal(const std::function<void(T)>& slot)
{
    if (!slot) {
        qDebug() << "Error: Slot function cannot be null.";
        throw std::invalid_argument("Slot function cannot be null");
    }
    m_signal.connect(std::move(slot)); // 連接信號到槽函式
}

template<typename T>
inline void workerThread<T>::unbindSignal(const std::function<void(T)>& slot)
{
    m_signal.disconnect(&slot); // 解除連接信號到槽函式
}

template<typename T>
inline void workerThread<T>::unbindAllSignals() 
{
    m_signal.disconnect_all_slots(); // 解除所有信號連接
}

template<typename T>
inline void workerThread<T>::run()
{
	T result = m_task(); // 執行任務
    m_signal(result); // 發送結果信號
}

#pragma endregion


class DisplayingWindowController : public QWidget
{
    Q_OBJECT

public:
    explicit DisplayingWindowController(DetectElement element, QWidget* parent = nullptr);
    ~DisplayingWindowController();

    void setup();

protected:
    // 在 Widget 關閉時自動呼叫，確保執行緒被清理
    void closeEvent(QCloseEvent* event) override;

private slots:
    // 定時檢查視窗標題是否變更
    void checkWindowTitle();
    // 啟動或重設文字滾動
    void startOrResetScrolling();

private:
    Ui::UI_Main_Display* ui;
    DetectElement m_element;
    QString m_currentTitle;

    QTimer* m_checkTimer;
    QThread* m_scrollThread; // 指向執行緒的指標
    SupportFunction::TextScroller* m_scroller; // 指向 Worker 的指標

	int m_uiWidth, m_uiHeight; // 儲存 UI 的寬度和高度
};