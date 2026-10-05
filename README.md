# 扫雷：普通与复数模式

C++17 内核 + Qt 6 Widgets 桌面界面，采用经典扫雷的灰色立体方格、红色数码计数器和像素风格雷与旗帜。

## 模式

v1.0.2 默认进入普通扫雷。在“模式”菜单选择“普通扫雷”或“复数扫雷”；切换会开始新游戏，保留难度与棋盘尺寸。

普通模式数字表示周围八格的雷数（1–8），旗帜不分类型。双击已揭示数字时，周围旗帜数等于数字才展开其余邻格；错误旗帜可能导致踩雷。

## 操作

- 左键翻格，右键插旗或取消；首次翻格保证不是雷。
- 复数模式可从菜单选择旗帜类型，或按 1 / 2 / 3 / 4 选择 +1 / -1 / +i / -i；普通模式禁用这些类型选项。
- F2 或笑脸按钮开始新游戏；“游戏 → 重玩本局”保留雷布局并翻开原首次点击格。
- 提供初级（9×9 / 10 雷）、中级（16×16 / 40 雷）、高级（30×16 / 99 雷）及自定义棋盘。
- 左上角显示雷数减旗帜数；右上角从首次翻格计时，最多显示 999 秒。
- 胜负通过笑脸及底部状态栏提示；失败后显示所有雷和错误位置的旗帜。

## 复数模式规则

雷值为 +1、-1、+i、-i。数字显示周围八格雷值总和的模长，非整数使用精确的根号形式，例如 √2。空白表示周围无雷；数字 0 表示存在雷但总和抵消。仅空白格自动展开。揭示全部安全格即获胜，无需标对雷种类。

双击已揭示数字格可尝试快速展开：至少有一面相邻旗帜，且旗帜复数总和的模长与显示值一致。复数抵消使匹配存在歧义，错误旗帜可能导致踩雷。此规则与普通扫雷按旗帜数量展开不同。

## Windows 构建与打包

安装 Qt 6.4 或更新版本，选择 **MSVC 2022 64-bit** 组件，以及 Visual Studio 2022 的“使用 C++ 的桌面开发”和 CMake 工具。

在 **x64 Native Tools Command Prompt for VS 2022** 中进入项目目录，执行（Qt 路径按实际安装版本修改）：

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:/Qt/6.8.3/msvc2022_64"
cmake --build build --config Release
cmake --install build --config Release --prefix "%CD%/dist"
```

`dist/complex_minesweeper.exe` 为启动程序。安装步骤会部署 Qt DLL 和平台插件；分发时保留整个 `dist` 目录，可压缩后发送到未安装 Qt 的 Windows 电脑。目标电脑可能需要安装 Microsoft Visual C++ 2015–2022 Redistributable（x64）。

也可使用 Qt Creator 打开 `CMakeLists.txt`，选择对应的 Desktop Qt 6 Kit 构建运行。MSVC 与 MinGW 的 Qt 套件不能混用。

## Linux 构建

需要 CMake、C++17 编译器与 Qt 6.4+ 开发包：

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/complex_minesweeper
```

## 内核接口

`getGrid().grd[row][col]` 存储 `cell` 值，使用 `.` 访问。`reveal()` 返回变化格子的副本。首次 `start()` 保留预先放置的旗帜，后续 `start()` 会重置棋盘；`restart()` 保留布局、清除旗帜并翻开指定位置，该位置未必安全。界面重玩时使用原首次点击位置。

## 自动发布

推送 `v*` 标签会触发 `.github/workflows/windows-release.yml`：在 Windows 2022 上编译，运行 GUI 检查，部署 Qt 运行库，验证独立部署包，最后发布含 ZIP 和 SHA256 校验文件的 GitHub Release。可手动运行工作流只生成构建附件。

```sh
git tag v1.0.2
git push origin v1.0.2
```

## GUI 检查

```sh
QT_QPA_PLATFORM=offscreen ./build/complex_minesweeper --smoke-test --screenshot /tmp/minesweeping-preview.png
```

检查覆盖两种模式的数字显示、模式切换和双击展开，以及首次翻格、预先插旗与取消、踩雷、胜利、重玩保留布局、计时停止和高级棋盘尺寸。它不能替代人工检查鼠标操作和不同 Windows 显示缩放下的效果。
