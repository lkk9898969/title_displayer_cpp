# Title Displayer 
~~可顯示任意視窗上的標題~~ C++版尚未實作。  
針對 **VLC** 及 **edge的youtube分頁** 有做優化

支援Edge瀏覽器
- 要觸發必須開啟Youtube分頁才能啟動標題顯示功能  
- 顯示功能啟動後則可顯示任意分頁的標題

VLC 需要開啟檔案後才能顯示標題。

### 以下為舊版介紹 :  
*chrome-edge_title_displayer  
針對edge youtube做的。主要功能為顯示Youtube影片標題。  
支援Edge及Chrome瀏覽器。  
要觸發必須開啟Youtube分頁才能啟動顯示功能，  
顯示功能啟動後則可顯示任意分頁的標題。*

## 2025/7/24更新:
由Python專案轉為C\++專案。轉的原因問就是我以前寫的Python是一坨  
因為Python專案的UI架構太過複雜，而且C\++執行效率更高，還有現在AI很發達要轉很好轉，索性直接重寫。  
選擇視窗功能仍在開發中。

## 2024/1/26更新:
主程式所有window controller重構、所有UI.py重構  
新增物件detectelement，作為偵測視窗的主物件  
新增"自行選擇視窗"功能，可選擇有標題的視窗進行顯示。

## 2023/12/16更新:
更新啟動後介面 : 需先選擇要偵測的視窗標題樣式。  
追加支援 VLC media player。  
UI架構重新建構。將UI介面分開為單一py檔案。

### ** 注意事項 : **
1.不支援相同視窗標題樣式但顯示不同實例的標題。(ex:可能無法正確偵測兩個(或以上)不同的VLC media player)  
2.Chrome瀏覽器可能不支援(2023/12/16更新後)