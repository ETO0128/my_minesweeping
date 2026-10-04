// 复数扫雷游戏
// 规则：四种雷，分别对应正负1和正负i，数字表示周围8格内雷的总和的模长，0代表模长为0，空格代表周围没有雷
// 玩家不需要判断正确雷的种类，只需要找出所有安全格
#ifndef MINESWEEPING_H
#define MINESWEEPING_H

#include <complex.h>
#include <cmath>
#include <vector>

namespace minesweeping
{

    enum class flag
    {
        NO_FLAG,
        PLUS_ONE,
        NEG_ONE,
        PLUS_I,
        NEG_I
    }; // 玩家判定的flag

    struct cell
    {
        int row, col;
        flag flg;
        bool is_revealed;
        int adjacentMineCount;    // 周围雷数
        int magnitudeSquared;     // 周围雷模长的平方
        std::complex<double> val; // 如果这格不是雷则为0，是则为正负1，正负i的一种
    };

    struct grid
    {
        int row_num;
        int col_num;
        std::vector<std::vector<cell>> grd;
    };

    class Mine_sweeping
    {
        grid Grid;
        bool is_failed;

        void getAdjacentStatus(int row, int col);

    public:
        Mine_sweeping(int row, int col, int MineCount, int seed);
        std::vector<cell> reveal(int row, int col); // 翻格，返回所有变化的格子
        void setFlag(int row, int col, flag flg);   // 标记
        void getState();                            // 获取棋盘和游戏状态
    };

    Mine_sweeping newGame(int rows, int cols, int MineCount, int seed); // 初始化棋盘

} // minesweeping

#endif