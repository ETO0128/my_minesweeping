经典像素风格的复数扫雷，Windows x64 首个桌面版本。

下载 `complex-minesweeper-windows-x64.zip`，解压整个文件夹后运行 `complex_minesweeper.exe`。压缩包包含 Qt 运行库与平台插件，无需安装 Qt；请勿只复制 exe。

功能：三档难度、自定义棋盘、计时和旗帜计数、四种雷标记、首次翻格安全、新游戏和重玩本局。数字显示复数雷总和的模长，非整数以根号显示；揭示全部安全格即可获胜。

Windows 构建会检查翻格、插旗、胜负、计时停止和重玩，再验证部署包能够在移除 Qt SDK PATH 后启动。提供 SHA256SUMS.txt 供校验下载文件。
