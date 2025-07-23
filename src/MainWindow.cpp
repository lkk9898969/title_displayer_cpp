
#include <QCloseEvent>
#include <QMessagebox>
#include <QThread>
#include <QTimer>
#include <thread>

#include "MainWindow.hpp"
#include "SupportFunction.hpp"

#pragma region MainWindow

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::UI_MainWindow) // 建立 UI 物件
    , m_menu(this)
{
    ui->setupUi(this); // 將 UI 設計套用到這個 QMainWindow 上
    setWindowIcon(QIcon(":/title_displayer/icon/icon.png")); // 設定視窗圖示
    m_selectWindowController = new SelectWindowController(this, [this](DetectElement& element) {
        checkElement(element); // 當按鈕被點擊時，呼叫 checkElement
        });
    auto data = SupportFunction::dataInit();
    data.emplace_back(L"自行選擇視窗", nullptr);
    for (auto& var : data)
    {
        m_selectWindowController->addButton((var));
    }
    showWidget(m_selectWindowController);
}

MainWindow::~MainWindow()
{
    delete ui; // 釋放 UI 物件佔用的記憶體
}

void MainWindow::customEvent(QEvent* event)
{
    if (event->type() == m_eventType) 
    { // Detect 完成
		m_displayingWindowController = new DisplayingWindowController(*m_currentElement,this); // 建立 DisplayingWindowController
		showWidget(m_displayingWindowController);
        //m_displayingWindowController->show();
		m_detectingWindowController->deleteLater();
		m_displayingWindowController->setup();
        setupDisplayingMainWindow();
    } 
    else 
    {
        QMainWindow::customEvent(event); // 呼叫基類的 customEvent 處理其他事件
    }
}

void MainWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        // 記錄滑鼠按下時的通用螢幕座標
        m_oldPos = event->globalPosition();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isDragging) {
        // 計算滑鼠移動的向量
        const QPointF delta = event->globalPosition() - m_oldPos;
        // 將視窗移動相同的向量
        this->move(this->pos() + delta.toPoint());
        // 更新舊位置以供下次計算
        m_oldPos = event->globalPosition();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
    }
}

void MainWindow::contextMenuEvent(QContextMenuEvent* event)
{
    // 使用 exec() 顯示選單，參數是事件的通用位置
    m_menu.exec(event->globalPos());
}

void MainWindow::showWidget(QWidget* widget)
{
    if (widget) {
        auto index = ui->MainWidget->addWidget(widget); // 將 Widget 加入到主 Widget 中
        ui->MainWidget->setCurrentIndex(index); // 設定當前顯示的 Widget
		this->setFixedSize(widget->size());
    } else {
        qDebug() << "Error: Attempted to show a null widget.";
	}
}

void MainWindow::checkElement(DetectElement& element)
{
    qDebug() << "Selected element:" << QString::fromStdWString(element.getText());
    
    if (element) 
    {
        m_detectingWindowController = new DetectingWindowController(this, std::move(&element)); // 建立 DetectingWindowController
        showWidget(m_detectingWindowController);
		m_selectWindowController->deleteLater();
        m_detectingWindowController->startDetectingT([this](DetectElement* elemnet) { onDetectCompleted(elemnet); });
		//QMessageBox::information(this, "偵測中", "正在偵測視窗，請稍候...", QMessageBox::StandardButton::Ok);
    }
    else
    {
        m_titleListWindowController = new TitleListWindowController(this, [this](const HWND hwnd) {
            (void)hwnd;
			QMessageBox::information(this, "Not Implemented", "This feature is not implemented yet.", QMessageBox::StandardButton::Ok);
			});
        showWidget(m_titleListWindowController);
		m_selectWindowController->deleteLater();
    }
}

void MainWindow::onDetectCompleted(DetectElement* element)
{
    m_currentElement = element;
	QCoreApplication::postEvent(this, new QEvent(m_eventType)); // 發送自訂事件
}

void MainWindow::setupDisplayingMainWindow()
{
    QScreen* screen = QGuiApplication::primaryScreen();

    // 最好檢查一下指標是否有效
    if (screen) {
        // 2. 取得螢幕可用區域的中心點
        QPoint centerPoint = screen->availableGeometry().center();

        // 3. 將視窗的框架矩形移動到中心點
        QRect frameRect = frameGeometry();
        frameRect.moveCenter(centerPoint);
        move(frameRect.topLeft());
    }
    setWindowFlags(Qt::FramelessWindowHint); // 設定視窗為無邊框且置頂
    // 建立 "離開" 動作
    m_exitAction = new QAction("離開", this);
    m_exitAction->setIcon(this->style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    m_exitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_X));
    connect(m_exitAction, &QAction::triggered, this, &MainWindow::close);

    // 建立 "保持在最上層" 動作
    m_stayOnTopAction = new QAction("保持在最上層", this);
    m_stayOnTopAction->setCheckable(true);
    connect(m_stayOnTopAction, &QAction::triggered, this, &MainWindow::stayOnTopToggle);

    // 將動作加入到 Widget，這樣它們就可以透過右鍵選單顯示
    addAction(m_stayOnTopAction);
    addAction(m_exitAction);

	m_menu.addAction(m_stayOnTopAction);
	m_menu.addAction(m_exitAction);

	m_menu.popup(mapToGlobal(QPoint(0, 0)));

    hide();
    //show();
    showMaximized();
    showNormal();
}

void MainWindow::stayOnTopToggle(bool checked)
{
    // 取得目前的視窗旗標
    Qt::WindowFlags flags = this->windowFlags();
    if (checked) {
        // 新增 "保持在最上層" 旗標
        flags |= Qt::WindowStaysOnTopHint;
    }
    else {
        // 移除 "保持在最上層" 旗標
        flags &= ~Qt::WindowStaysOnTopHint;
    }
    // 重新設定旗標並顯示視窗以使其生效
    setWindowFlags(flags);
    show();
}

#pragma endregion

#pragma region SelectWindow

SelectWindowController::SelectWindowController(QWidget* parent, std::function<void(DetectElement&)> callback) :
    QWidget(parent),
    ui(new Ui::UI_Select_Window) // 建立 UI 物件
{
    if (callback == nullptr) {
        qDebug() << "Error: Callback function cannot be null.";
        throw std::invalid_argument("Callback function cannot be null");
	}
    ui->setupUi(this); // 將 UI 設計套用到這個 Widget 上
    m_onButtonClicked = std::move(callback); // 儲存按鈕點擊事件處理函式
}

SelectWindowController::~SelectWindowController()
{
    delete ui; // 釋放 UI 物件佔用的記憶體
}

void SelectWindowController::addButton(DetectElement& element)
{
    QPushButton* button = new QPushButton(QString::fromStdWString(element.getText()), this);
    button->setFont(m_font); // 設定按鈕字型
	button->setFixedHeight(30); // 設定按鈕高度
    connect(button, &QPushButton::clicked, [this, element]() mutable {
        qDebug() << "Button clicked for:" << QString::fromStdWString(element.getText());
        m_onButtonClicked(element); // 呼叫按鈕點擊事件處理函式
        });
	ui->ButtonLayout->addWidget(button); // 將按鈕加入到佈局中
}

#pragma endregion


#pragma region TitleListWindowController


TitleListWindowController::TitleListWindowController(QWidget* parent, std::function<void(const HWND)> callback) :
	QWidget(parent),
	ui(new Ui::UI_Title_List) // 建立 UI 物件
{
    if (callback == nullptr) {
        qDebug() << "Error: Callback function cannot be null.";
        throw std::invalid_argument("Callback function cannot be null");
    }
    ui->setupUi(this); // 將 UI 設計套用到這個 Widget 上
	ui->refresh->setIcon(QIcon(":/title_displayer/icon/icons8-refresh-26.png")); // 設定刷新按鈕圖示
    m_onButtonClicked = std::move(callback); // 儲存按鈕點擊事件處理函式
    connect(ui->OK, &QPushButton::clicked, this, &TitleListWindowController::onOKClicked); // 連接按鈕點擊事件
	refreshList(); // 初始化時刷新列表
    connect(ui->refresh, &QPushButton::clicked, this, &TitleListWindowController::onRefreshClicked); // 連接刷新按鈕事件
}

TitleListWindowController::~TitleListWindowController()
{
	delete ui; // 釋放 UI 物件佔用的記憶體
}

void TitleListWindowController::refreshList()
{
    ui->titlelist->clear();
    for (auto& i : SupportFunction::getAllWindowsTitle()) {
        QListWidgetItem* item = new QListWidgetItem(QString::fromStdWString(i.first), ui->titlelist);
        item->setData(Qt::UserRole, QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(i.second))); // 儲存 HWND 為 quintptr
		ui->titlelist->addItem(item); // 將項目加入到列表中
    }
}

void TitleListWindowController::onOKClicked()
{
    HWND hwnd;
    QListWidgetItem* currentItem = ui->titlelist->currentItem(); // 獲取當前選中的項目
    if (currentItem) {
        hwnd = reinterpret_cast<HWND>(currentItem->data(Qt::UserRole).value<quintptr>()); // 從項目資料中獲取 HWND
        if (hwnd) {
            qDebug() << "Button clicked for HWND:" << hwnd;
            m_onButtonClicked(hwnd); // 呼叫按鈕點擊事件處理函式
        }
        else {
            qDebug() << "No HWND found for the selected item.";
        }
    }
    else {
        qDebug() << "No item selected.";
		QMessageBox::critical(this, "錯誤", "請從列表當中選擇一個視窗");
    }
}

void TitleListWindowController::onRefreshClicked()
{
    QPushButton* senderObj = qobject_cast<QPushButton*>(sender()); // 獲取發送信號的物件
	senderObj->setEnabled(false);
    refreshList();
    QTimer::singleShot(500, this, [senderObj]() {
        senderObj->setEnabled(true); // 1秒後重新啟用按鈕
        });
}

#pragma endregion



#pragma region TitleFilterWindowController

TitleFilterWindowController::TitleFilterWindowController(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::UI_Title_Filter) // 建立 UI 物件
{
    ui->setupUi(this); // 將 UI 設計套用到這個 Widget 上

    connect(ui->Enter, &QPushButton::clicked, this, &TitleFilterWindowController::onEnterClicked);
}

TitleFilterWindowController::~TitleFilterWindowController()
{
    delete ui; // 釋放 UI 物件佔用的記憶體
}

void TitleFilterWindowController::onEnterClicked()
{
    QString filterText = ui->titleEdit->toPlainText();
    if (!filterText.isEmpty()) {
        qDebug() << "Filter text entered:" << filterText;
    }
    else {
        qDebug() << "No filter text entered.";
    }
}

#pragma endregion

#pragma region DetectWindow


DetectingWindowController::DetectingWindowController(QWidget* parent, DetectElement* element) :
    QWidget(parent),
	ui(new Ui::UI_Detecting_Window),
    m_element(*element)
{
    if (element == nullptr) {
        qDebug() << "Error: DetectElement cannot be null.";
        throw std::invalid_argument("DetectElement cannot be null");
	}
	ui->setupUi(this); // 將 UI 設計套用到這個 Widget 上
	auto str = QString::fromStdWString(m_element.getText());
    auto text = ui->text->text();
	text.replace("{}", str); 
    ui->text->setText(text); 
}

DetectingWindowController::~DetectingWindowController()
{
    if (m_workerThread->isRunning())
    {
        m_workerThread->quit();
		m_workerThread->wait();
    }
	m_workerThread->deleteLater(); // 釋放工作線程物件佔用的記憶體
	delete ui; // 釋放 UI 物件佔用的記憶體
}

void DetectingWindowController::startDetectingT(std::function<void(DetectElement*)> slot)
{
    m_workerThread = new workerThread<DetectElement*>(this, [this]() {
        qDebug() << "Worker thread started for detecting element:" << QString::fromStdWString(m_element.getText());
        DetectingWork(); // 開始偵測
		return &m_element; // 返回偵測結果
        }); // 傳遞 this 指標以便在 workerThread 中使用
	m_workerThread->bindSignal(slot); // 綁定信號到回調函式
    qDebug() << "Detecting thread started for element:" << QString::fromStdWString(m_element.getText());
	m_workerThread->start(); // 啟動工作線程
}

void  DetectingWindowController::DetectingWork()
{
    qDebug() << "Starting detection for element:" << QString::fromStdWString(m_element.getText());
    HWND hwnd;
    while (!(hwnd = m_element.findWindow())) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
	qDebug() << "Detection completed for element:" << QString::fromStdWString(m_element.getText());
}


#pragma endregion


#pragma region DisplayingWindowController

DisplayingWindowController::DisplayingWindowController(DetectElement element, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::UI_Main_Display),
    m_element(element),
    m_scrollThread(nullptr), // 初始化指標為 nullptr
    m_scroller(nullptr),
    m_checkTimer(nullptr)
{
    ui->setupUi(this);
	m_uiWidth = ui->music_name->width(); // 儲存 UI 的寬度
    m_uiHeight = ui->music_name->height(); // 儲存 UI 的高度
	setFixedSize(m_uiWidth, m_uiHeight); // 設定視窗大小為固定大小
}

DisplayingWindowController::~DisplayingWindowController()
{
    // 確保執行緒在物件銷毀前已停止
    if (m_scrollThread && m_scrollThread->isRunning()) {
        m_scroller->stop();
        disconnect(m_scroller, nullptr, this, nullptr);
        m_scrollThread->quit();// 使用有限等待時間，防止主執行緒卡死
        if (!m_scrollThread->wait(1000)) {
            // 如果等待超時，表示執行緒卡住，強制終止它
            qWarning() << "TextScroller thread did not quit in time, terminating.";
            m_scrollThread->terminate();
            m_scrollThread->wait(); // 等待終止完成
        }

        // 安全地排程刪除舊的物件
        m_scroller->deleteLater();
        m_scrollThread->deleteLater();
    }
    m_checkTimer->deleteLater();
    delete ui;

}

void DisplayingWindowController::setup()
{
    // 獲取初始標題並開始滾動
    m_currentTitle = QString::fromStdWString(m_element.getOriginalWindowTitle());
    ui->music_name->setText(QString::fromStdWString(m_element.getFilteredWindowTitle()));
    ui->music_name->setGeometry(0, 0, ui->music_name->fontMetrics().horizontalAdvance(ui->music_name->text()), m_uiHeight);
    startOrResetScrolling();

    // 設定計時器，每 200ms 檢查一次標題
    m_checkTimer = new QTimer(this);
    connect(m_checkTimer, &QTimer::timeout, this, &DisplayingWindowController::checkWindowTitle);
    m_checkTimer->start(200);
}

void DisplayingWindowController::closeEvent(QCloseEvent* event)
{
    m_checkTimer->stop();
    if (m_scroller) {
        m_scroller->stop();
    }
    event->accept();
}

void DisplayingWindowController::checkWindowTitle()
{
    QString newTitle = QString::fromStdWString(m_element.getOriginalWindowTitle());
    if (newTitle != m_currentTitle && !newTitle.isEmpty()) {
        m_currentTitle = newTitle;
        ui->music_name->setText(QString::fromStdWString(m_element.getFilteredWindowTitle()));
		ui->music_name->setGeometry(0, 0, ui->music_name->fontMetrics().horizontalAdvance(ui->music_name->text()), m_uiHeight);
        startOrResetScrolling(); // 標題變更，重設滾動
    }
}

void DisplayingWindowController::startOrResetScrolling()
{
    // 如果已有執行緒在執行，先停止它
    if (m_scrollThread && m_scrollThread->isRunning()) {
        // 告訴 Worker 停止其內部迴圈
        m_scroller->stop();

        // *** 關鍵：斷開所有與舊 Worker 的連接，防止其 finished() 訊號觸發不必要的行為 ***
        disconnect(m_scroller, nullptr, this, nullptr);

        // 請求執行緒退出事件迴圈並等待其結束
        m_scrollThread->quit();
        // 使用有限等待時間，防止主執行緒卡死
        if (!m_scrollThread->wait(1000)) {
            // 如果等待超時，表示執行緒卡住，強制終止它
            qWarning() << "TextScroller thread did not quit in time, terminating.";
            m_scrollThread->terminate();
            m_scrollThread->wait(); // 等待終止完成
        }

        // 安全地排程刪除舊的物件
        m_scroller->deleteLater();
        m_scrollThread->deleteLater();
    }

    // 建立新的執行緒和 Worker
    m_scrollThread = new QThread(this);
	m_scrollThread->setObjectName("TextScrollerThread"); // 設定執行緒名稱，方便除錯
    m_scroller = new SupportFunction::TextScroller(ui->music_name, QFontMetrics(ui->music_name->font()), ui->music_name->text(),
        m_uiWidth, m_uiHeight);

    // 將 Worker 移動到新執行緒
    m_scroller->moveToThread(m_scrollThread);

    // 連接訊號與槽
    // 1. 當執行緒啟動時，呼叫 worker 的 run() 函式
    connect(m_scrollThread, &QThread::started, m_scroller, &SupportFunction::TextScroller::run);
    // 2. Worker 需要在執行緒中更新 UI，透過訊號傳遞給主執行緒的 QLabel
    connect(m_scroller, &SupportFunction::TextScroller::updateLabelPosition,
        this, [this](int x, int y) { ui->music_name->move(x, y); } );

    // 啟動執行緒
    m_scrollThread->start();
}

#pragma endregion
