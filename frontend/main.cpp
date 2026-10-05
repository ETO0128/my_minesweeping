#include "minesweeping.h"
#include <QApplication>
#include <QAction>
#include <QActionGroup>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QDialog>
#include <QDialogButtonBox>
#include <QElapsedTimer>
#include <QFormLayout>
#include <QLabel>
#include <QLCDNumber>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QRandomGenerator>
#include <QScrollArea>
#include <QSpinBox>
#include <QStatusBar>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace minesweeping;
namespace {
constexpr int tile = 32;
QString flagText(flag f) {
    switch (f) {
    case flag::PLUS_ONE: return "+1";
    case flag::NEG_ONE: return "-1";
    case flag::PLUS_I: return "+i";
    case flag::NEG_I: return "-i";
    default: return {};
    }
}
// All artwork uses integer coordinates with antialiasing disabled.
void bevel(QPainter &p, QRect r, bool raised) {
    p.fillRect(r, QColor("#c0c0c0"));
    p.setPen(QPen(raised ? Qt::white : QColor("#808080"), 2));
    p.drawLine(r.topLeft()+QPoint(1,1), r.topRight()+QPoint(-1,1));
    p.drawLine(r.topLeft()+QPoint(1,1), r.bottomLeft()+QPoint(1,-1));
    p.setPen(QPen(raised ? QColor("#808080") : Qt::white, 2));
    p.drawLine(r.bottomLeft()+QPoint(1,-1), r.bottomRight()+QPoint(-1,-1));
    p.drawLine(r.topRight()+QPoint(-1,1), r.bottomRight()+QPoint(-1,-1));
}
class Board : public QWidget {
public:
    const Mine_sweeping *game = nullptr;
    bool normalMode = true;
    QString clueText(const cell &c) const {
        if(c.adjacentMineCount==0) return {};
        if(normalMode) return QString::number(c.adjacentMineCount);
        const int squared=static_cast<int>(std::norm(c.sum));
        const int root=static_cast<int>(std::sqrt(squared));
        return root*root==squared ? QString::number(root) : QString(QChar(0x221a))+QString::number(squared);
    }
    std::function<void(int,int,Qt::MouseButton)> clicked;
    std::function<void(int,int)> chord;
    explicit Board(QWidget *parent=nullptr) : QWidget(parent) {
        setMouseTracking(true);
    }
    void sync() {
        const auto &g=game->getGrid();
        setFixedSize(g.col_num*tile, g.row_num*tile);
        update();
    }
protected:
    void paintEvent(QPaintEvent *) override {
        if (!game) return;
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, false);
        QFont f("Consolas", 12, QFont::Bold); p.setFont(f);
        const auto &g=game->getGrid();
        const bool lost=game->getState()==status::LOST;
        const bool won=game->getState()==status::WIN;
        const QColor colors[]={QColor("#0000cc"),QColor("#008000"),QColor("#cc0000"),QColor("#000080"),QColor("#800000"),QColor("#008080"),QColor("#000000"),QColor("#808080")};
        for (const auto &line:g.grd) for (const auto &c:line) {
            QRect r(c.col*tile,c.row*tile,tile,tile);
            bool mine=c.val!=0.0;
            if (c.is_revealed || (lost && mine)) {
                p.fillRect(r, lost && mine && c.is_revealed ? QColor("#ff3333") : QColor("#c0c0c0"));
                p.setPen(QColor("#808080")); p.drawRect(r.adjusted(0,0,-1,-1));
                if (mine) {
                    const QPoint center=r.center()+QPoint(0,normalMode ? 0 : -5);
                    p.setPen(QPen(Qt::black,2));
                    p.drawLine(center+QPoint(-7,0),center+QPoint(7,0));
                    p.drawLine(center+QPoint(0,-7),center+QPoint(0,7));
                    p.drawLine(center+QPoint(-5,-5),center+QPoint(5,5));
                    p.drawLine(center+QPoint(-5,5),center+QPoint(5,-5));
                    p.fillRect(QRect(center-QPoint(4,4),QSize(9,9)),Qt::black);
                    p.fillRect(QRect(center-QPoint(2,2),QSize(2,2)),Qt::white);
                    if(!normalMode) {
                        const QString value=c.val.real()!=0 ? (c.val.real()>0?"+1":"-1") : (c.val.imag()>0?"+i":"-i");
                        p.drawText(r.adjusted(0,15,0,0),Qt::AlignCenter,value);
                    }
                } else if (c.adjacentMineCount>0) {
                    const int number=normalMode ? c.adjacentMineCount : static_cast<int>(std::abs(c.sum));
                    p.setPen(colors[std::clamp(number-1,0,7)]);
                    p.drawText(r,Qt::AlignCenter,clueText(c));
                }
            } else {
                bevel(p,r,true);
                if (c.flg!=flag::NO_FLAG || (won && mine)) {
                    p.fillRect(r.x()+10,r.y()+5,2,13,Qt::black);
                    p.fillRect(r.x()+7,r.y()+17,9,2,Qt::black);
                    p.fillRect(r.x()+12,r.y()+5,9,6,QColor("#df0000"));
                    p.setPen(Qt::black);
                    if(!normalMode) p.drawText(r.adjusted(0,16,0,0),Qt::AlignCenter,flagText(c.flg));
                }
                if (lost && c.flg!=flag::NO_FLAG && !mine) {
                    p.setPen(QPen(Qt::red,3));
                    p.drawLine(r.topLeft()+QPoint(5,5),r.bottomRight()-QPoint(5,5));
                    p.drawLine(r.topRight()+QPoint(-5,5),r.bottomLeft()+QPoint(5,-5));
                }
            }
        }
    }
    void mouseReleaseEvent(QMouseEvent *e) override {
        const QPoint pos=e->position().toPoint();
        if (!rect().contains(pos)) return;
        if (clicked) clicked(pos.y()/tile,pos.x()/tile,e->button());
    }
    void mouseDoubleClickEvent(QMouseEvent *e) override {
        const QPoint pos=e->position().toPoint();
        if (rect().contains(pos) && e->button()==Qt::LeftButton && chord)
            chord(pos.y()/tile,pos.x()/tile);
    }
};
class Window : public QMainWindow {
    std::unique_ptr<Mine_sweeping> game;
    Board *board;
    QLCDNumber *remaining,*clock;
    QPushButton *face;
    QTimer timer;
    QElapsedTimer elapsed;
    int rows=9,cols=9,mines=10;
    bool started=false;
    bool normalMode=true;
    flag selected=flag::PLUS_ONE;
    QAction *replayAction;
    QActionGroup *difficulty;
    std::vector<QAction*> presets;
    QAction *customAction;
    QAction *normalAction,*complexAction;
    QMenu *flagMenu;
    QActionGroup *flagActions;
    QLabel *hint;
    int firstRow=0,firstCol=0;
public:
    Window() {
        setWindowTitle(QStringLiteral("复数扫雷"));
        auto *menu=menuBar()->addMenu(QStringLiteral("游戏"));
        auto *fresh=menu->addAction(QStringLiteral("新游戏")); fresh->setShortcut(QKeySequence("F2"));
        connect(fresh,&QAction::triggered,this,[this]{newGame();});
        replayAction=menu->addAction(QStringLiteral("重玩本局"));
        connect(replayAction,&QAction::triggered,this,[this]{replay();});
        menu->addSeparator(); difficulty=new QActionGroup(this);
        const int settings[3][3]={{9,9,10},{16,16,40},{16,30,99}};
        const QString names[]={QStringLiteral("初级 · 9 × 9 · 10 雷"),QStringLiteral("中级 · 16 × 16 · 40 雷"),QStringLiteral("高级 · 30 × 16 · 99 雷")};
        for (int i=0;i<3;++i) {
            auto *a=menu->addAction(names[i]); a->setCheckable(true); difficulty->addAction(a); presets.push_back(a);
            const int r=settings[i][0],c=settings[i][1],m=settings[i][2];
            connect(a,&QAction::triggered,this,[this,r,c,m]{rows=r;cols=c;mines=m;newGame();});
        }
        presets.front()->setChecked(true);
        customAction=menu->addAction(QStringLiteral("自定义…")); customAction->setCheckable(true); difficulty->addAction(customAction);
        connect(customAction,&QAction::triggered,this,[this]{custom();});
        menu->addSeparator();
        connect(menu->addAction(QStringLiteral("退出")),&QAction::triggered,this,&QWidget::close);
        auto *modes=menuBar()->addMenu(QStringLiteral("模式"));
        auto *modeActions=new QActionGroup(this);
        normalAction=modes->addAction(QStringLiteral("普通扫雷"));
        complexAction=modes->addAction(QStringLiteral("复数扫雷"));
        for(auto *action:{normalAction,complexAction}) {action->setCheckable(true);modeActions->addAction(action);}
        connect(normalAction,&QAction::triggered,this,[this]{setMode(true);});
        connect(complexAction,&QAction::triggered,this,[this]{setMode(false);});
        flagMenu=menuBar()->addMenu(QStringLiteral("旗帜类型"));
        flagActions=new QActionGroup(this);
        for (int i=1;i<=4;++i) {
            const auto type=static_cast<flag>(i);
            auto *a=flagMenu->addAction(flagText(type)); a->setCheckable(true); flagActions->addAction(a); a->setChecked(i==1);
            a->setShortcut(QKeySequence(QString::number(i)));
            connect(a,&QAction::triggered,this,[this,type]{selected=type;});
        }
        auto *help=menuBar()->addMenu(QStringLiteral("帮助"));
        connect(help->addAction(QStringLiteral("玩法")),&QAction::triggered,this,[this]{
            if(normalMode) {
                QMessageBox::information(this,QStringLiteral("普通扫雷"),QStringLiteral(
                    "左键翻格，右键插旗或取消。数字表示周围八格中的雷数，空白格自动展开。\n"
                    "揭示全部安全格即可获胜，首次翻格保证安全。\n\n"
                    "双击已揭示数字：当周围旗帜数等于数字时，翻开其余邻格。标错位置仍会踩雷。\n"
                    "F2 或笑脸按钮开始新游戏；重玩本局保留布局。\n"
                    "可在“模式”菜单切换到复数扫雷；切换模式会开始新游戏。"));
                return;
            }
            QMessageBox::information(this,QStringLiteral("复数扫雷"),QStringLiteral(
                "左键翻格，右键插旗或取消；数字键 1–4 选择旗帜类型。\n"
                "揭示全部安全格即可获胜，不需要标对雷的种类。首次翻格安全。\n\n"
                "雷值为 +1、-1、+i、-i。数字是周围八格雷值总和的模长，"
                "非整数以 √n 显示。0 表示有雷但总和抵消，空白表示周围没有雷。\n\n"
                "F2 新游戏；重玩本局保留布局并恢复原首次翻格。\n"
                "双击已揭示数字可尝试展开：相邻旗帜的复数总和必须匹配模长。"
                "匹配并不保证旗帜正确，展开仍可能踩雷。"));
        });
        auto *panel=new QWidget; auto *layout=new QVBoxLayout(panel); layout->setContentsMargins(12,12,12,12);
        auto *top=new QHBoxLayout;
        remaining=new QLCDNumber(3); clock=new QLCDNumber(3);
        for (auto *lcd:{remaining,clock}) { lcd->setSegmentStyle(QLCDNumber::Flat); lcd->setFixedSize(80,44); lcd->setStyleSheet("background:#180000;color:#ff2020;border:2px inset #eee;"); }
        face=new QPushButton(":)"); face->setFixedSize(44,44); face->setFont(QFont("Consolas",18,QFont::Bold));
        connect(face,&QPushButton::clicked,this,[this]{newGame();});
        top->addWidget(remaining);top->addStretch();top->addWidget(face);top->addStretch();top->addWidget(clock); layout->addLayout(top);
        board=new Board; board->clicked=[this](int r,int c,Qt::MouseButton b){click(r,c,b);}; board->chord=[this](int r,int c){chord(r,c);};
        auto *scroll=new QScrollArea; scroll->setWidget(board);scroll->setAlignment(Qt::AlignCenter);scroll->setWidgetResizable(false);layout->addWidget(scroll);
        hint=new QLabel; hint->setWordWrap(true); layout->addWidget(hint);
        setCentralWidget(panel);
        connect(&timer,&QTimer::timeout,this,[this]{clock->display(static_cast<int>(std::min<qint64>(999,elapsed.elapsed()/1000)));});
        timer.setInterval(200);
        setMode(true);
    }
    // Exercise the same handlers used by the GUI, including end-of-game states.
    void smokeCheck(const QString &screenshot) {
        auto require=[](bool ok,const char *message) {
            if(!ok) throw std::runtime_error(message);
        };
        for(bool normal:{true,false}) {
            rows=9;cols=9;mines=10;setMode(normal);
            require(board->normalMode==normal && normalAction->isChecked()==normal,
                    "Mode selection failed");
            require(flagMenu->isEnabled()!=normal,"Flag menu availability failed");
            cell sample;
            sample.adjacentMineCount=2;
            require(board->clueText(sample)==(normal ? "2" : "0"),"Clue mode rendering failed");
            sample.sum={1.0,1.0};
            require(board->clueText(sample)==(normal ? QString("2") : QString(QChar(0x221a))+"2"),
                    "Complex magnitude rendering failed");
            click(8,8,Qt::RightButton);
            require(game->getGrid().grd[8][8].flg==flag::PLUS_ONE,"Right click flag failed");
            click(0,0,Qt::LeftButton);
            require(started && game->getGrid().grd[0][0].val==0.0,"First click was not safe");
            require(game->getGrid().grd[8][8].flg==flag::PLUS_ONE &&
                    !game->getGrid().grd[8][8].is_revealed,"Pregame flag was not preserved");
            click(8,8,Qt::RightButton);
            require(game->getGrid().grd[8][8].flg==flag::NO_FLAG,"Flag removal failed");
            const auto original=game->getGrid();
            int mineRow=-1,mineCol=-1;
            for(const auto &line:original.grd) for(const auto &item:line)
                if(item.val!=0.0) {mineRow=item.row;mineCol=item.col;}
            require(mineRow>=0,"No mines generated");
            click(mineRow,mineCol,Qt::LeftButton);
            require(game->getState()==status::LOST && !timer.isActive(),"Loss or timer stop failed");
            replay();
            require(game->getState()==status::PLAYING && timer.isActive(),"Replay failed");
            for(const auto &line:original.grd) for(const auto &item:line)
                require(game->getGrid().grd[item.row][item.col].val==item.val,"Replay changed mine layout");
            for(const auto &line:original.grd) for(const auto &item:line)
                if(item.val==0.0) click(item.row,item.col,Qt::LeftButton);
            require(game->getState()==status::WIN && !timer.isActive(),"Win or timer stop failed");
            newGame();
            require(!started && game->getState()==status::PLAYING,"New game failed");
            rows=16;cols=30;mines=99;newGame();
            require(board->width()==30*tile && board->height()==16*tile,"Expert board size failed");
            checkChord(normal);
        }
        rows=9;cols=9;mines=10;setMode(true);
        require(!started && clock->intValue()==0,"Mode switch did not reset game");
        click(0,0,Qt::LeftButton);
        if(!screenshot.isEmpty()) {
            QApplication::processEvents();
            require(grab().save(screenshot),"Screenshot save failed");
        }
    }
private:
    void checkChord(bool normal) {
        rows=9;cols=9;mines=10;setMode(normal);
        game->start(0,0,12345);
        started=true;firstRow=0;firstCol=0;elapsed.start();timer.start();
        replayAction->setEnabled(true);
        const auto layout=game->getGrid();
        for(const auto &line:layout.grd) for(const auto &center:line) {
            if(center.val!=0.0 || center.adjacentMineCount==0) continue;
            // Use a clue whose ordinary count differs from its complex magnitude.
            if(normal && std::norm(center.sum)==center.adjacentMineCount*center.adjacentMineCount)
                continue;
            std::vector<cell> neighboringMines,coveredSafe;
            for(int dr=-1;dr<=1;++dr) for(int dc=-1;dc<=1;++dc) {
                const int r=center.row+dr,c=center.col+dc;
                if((dr==0 && dc==0)||r<0||r>=rows||c<0||c>=cols) continue;
                const auto item=game->getGrid().grd[r][c];
                if(item.val!=0.0) neighboringMines.push_back(item);
                else if(!item.is_revealed) coveredSafe.push_back(item);
            }
            if(coveredSafe.empty()) continue;
            click(center.row,center.col,Qt::LeftButton);
            // No flags must leave the covered neighbors untouched.
            const auto before=game->getGrid();
            chord(center.row,center.col);
            for(const auto &item:coveredSafe)
                if(game->getGrid().grd[item.row][item.col].is_revealed!=before.grd[item.row][item.col].is_revealed)
                    throw std::runtime_error("Chord expanded without matching flags");
            for(const auto &item:neighboringMines) {
                selected=item.val.real()>0 ? flag::PLUS_ONE : item.val.real()<0 ? flag::NEG_ONE :
                         item.val.imag()>0 ? flag::PLUS_I : flag::NEG_I;
                click(item.row,item.col,Qt::RightButton);
            }
            chord(center.row,center.col);
            for(const auto &item:coveredSafe)
                if(!game->getGrid().grd[item.row][item.col].is_revealed)
                    throw std::runtime_error("Chord failed to reveal safe neighbor");
            if(game->getState()==status::LOST)
                throw std::runtime_error("Correct chord hit a mine");
            selected=flag::PLUS_ONE;
            return;
        }
        throw std::runtime_error("Could not find a numbered cell for chord checks");
    }
    void setMode(bool normal) {
        normalMode=normal;
        board->normalMode=normal;
        normalAction->setChecked(normal);
        complexAction->setChecked(!normal);
        flagMenu->setEnabled(!normal);
        for(auto *action:flagActions->actions()) action->setEnabled(!normal);
        setWindowTitle(normal ? QStringLiteral("普通扫雷 · v1.0.2") : QStringLiteral("复数扫雷 · v1.0.2"));
        hint->setText(normal ? QStringLiteral("左键翻格 · 右键插旗 · 双击数字展开 · F2 新游戏")
                             : QStringLiteral("左键翻格 · 右键插旗 · 1–4 选择雷标记 · F2 新游戏"));
        newGame();
    }
    void newGame() {
        timer.stop(); started=false; game=std::make_unique<Mine_sweeping>(rows,cols,mines);
        remaining->setDigitCount(rows*cols>999 ? 5 : 3);
        remaining->setFixedWidth(rows*cols>999 ? 110 : 80);
        board->game=game.get();board->sync(); clock->display(0);face->setText(":)");replayAction->setEnabled(false);
        refresh(); resize(std::max(360,std::min(cols*tile+52,1100)),std::min(rows*tile+170,800));
    }
    void refresh() {
        int flags=0;
        for (const auto &line:game->getGrid().grd) for (const auto &c:line) if(c.flg!=flag::NO_FLAG) ++flags;
        remaining->display(game->getState()==status::WIN ? 0 : mines-flags);
        board->update();
        const auto st=game->getState();
        if (st!=status::PLAYING) { timer.stop(); clock->display(static_cast<int>(std::min<qint64>(999,elapsed.elapsed()/1000))); }
        if(st==status::WIN) {face->setText("B)");statusBar()->showMessage(QStringLiteral("胜利！所有安全格已揭示。"));}
        else if(st==status::LOST) {face->setText(":(");statusBar()->showMessage(QStringLiteral("踩雷了。按 F2 新游戏，或从菜单重玩本局。"));}
        else statusBar()->showMessage(started ? QStringLiteral("进行中 · 旗帜只是推测，找到所有安全格即可获胜") : QStringLiteral("准备就绪 · 首次翻格安全"));
    }
    void replay() {
        if(!started) return;
        elapsed.start();clock->display(0);face->setText(":)");timer.start();
        game->restart(firstRow,firstCol);refresh();
    }
    void click(int r,int c,Qt::MouseButton button) {
        if(game->getState()!=status::PLAYING) return;
        const auto &cell=game->getGrid().grd[r][c];
        if(button==Qt::RightButton) {
            game->setFlag(r,c,cell.flg==flag::NO_FLAG?(normalMode ? flag::PLUS_ONE : selected):flag::NO_FLAG);
        } else if(button==Qt::LeftButton) {
            if(cell.is_revealed || cell.flg!=flag::NO_FLAG) return;
            if(!started) {
                const int seed=static_cast<int>(QRandomGenerator::global()->bounded(2147483647u));
                firstRow=r;firstCol=c;started=true;elapsed.start();timer.start();replayAction->setEnabled(true);
                game->start(r,c,seed);
            } else game->reveal(r,c);
        } else return;
        refresh();
    }
    void chord(int r,int c) {
        if(!started || game->getState()!=status::PLAYING) return;
        const auto center=game->getGrid().grd[r][c];
        if(!center.is_revealed || center.val!=0.0 || center.adjacentMineCount==0) return;
        std::complex<double> sum{}; int count=0;
        std::vector<std::pair<int,int>> neighbors;
        for(int dr=-1;dr<=1;++dr) for(int dc=-1;dc<=1;++dc) {
            const int nr=r+dr,nc=c+dc;
            if((dr==0 && dc==0)||nr<0||nr>=rows||nc<0||nc>=cols) continue;
            const auto &item=game->getGrid().grd[nr][nc];
            switch(item.flg) {
            case flag::PLUS_ONE:sum+=1.0;++count;break;
            case flag::NEG_ONE:sum-=1.0;++count;break;
            case flag::PLUS_I:sum+=std::complex<double>(0,1);++count;break;
            case flag::NEG_I:sum-=std::complex<double>(0,1);++count;break;
            default: if(!item.is_revealed) neighbors.emplace_back(nr,nc);break;
            }
        }
        if(normalMode) {
            if(count!=center.adjacentMineCount) return;
        } else if(count==0 || std::norm(sum)!=std::norm(center.sum)) return;
        for(const auto &[nr,nc]:neighbors) {
            game->reveal(nr,nc);
            if(game->getState()!=status::PLAYING) break;
        }
        refresh();
    }
    void custom() {
        QDialog dialog(this);dialog.setWindowTitle(QStringLiteral("自定义棋盘"));
        QFormLayout form(&dialog);QSpinBox r,c,m;
        r.setRange(2,40);c.setRange(2,60);m.setRange(0,rows*cols-1);
        r.setValue(rows);c.setValue(cols);m.setValue(mines);
        auto updateMax=[&]{m.setMaximum(r.value()*c.value()-1);};updateMax();
        connect(&r,qOverload<int>(&QSpinBox::valueChanged),&dialog,[&](int){updateMax();});
        connect(&c,qOverload<int>(&QSpinBox::valueChanged),&dialog,[&](int){updateMax();});
        form.addRow(QStringLiteral("行数"),&r);form.addRow(QStringLiteral("列数"),&c);form.addRow(QStringLiteral("雷数"),&m);
        QDialogButtonBox buttons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
        form.addRow(&buttons);connect(&buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);connect(&buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
        if(dialog.exec()==QDialog::Accepted) {rows=r.value();cols=c.value();mines=m.value();newGame();}
        else {const int sizes[3][3]={{9,9,10},{16,16,40},{16,30,99}};for(int i=0;i<3;++i) if(rows==sizes[i][0]&&cols==sizes[i][1]&&mines==sizes[i][2]) presets[i]->setChecked(true);}
    }
};
}
int main(int argc,char **argv) {
    QApplication app(argc,argv);app.setStyle("Fusion");
    app.setApplicationName("Complex Minesweeper");
    app.setStyleSheet("QMainWindow,QDialog,QWidget{background:#c0c0c0;color:#111;} QMenu::item:selected{background:#000080;color:white;} QPushButton{border:2px outset #eee;padding:3px;} QPushButton:pressed{border:2px inset #eee;} QScrollArea{border:3px inset #eee;}");
    Window window;window.show();
    if(app.arguments().contains("--smoke-test")) {
        try {
            const int index=app.arguments().indexOf("--screenshot");
            window.smokeCheck(index>=0 ? app.arguments().value(index+1) : QString{});
            std::cout << "GUI smoke checks passed\n";
            return 0;
        } catch(const std::exception &error) {
            std::cerr << error.what() << '\n';
            return 1;
        }
    }
    return app.exec();
}
