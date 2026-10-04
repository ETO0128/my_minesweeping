// 复数扫雷游戏
// 规则：四种雷，分别对应正负1和正负i，数字表示周围8格内雷的总和的模长，0代表模长为0，空格代表周围没有雷
// 玩家不需要判断正确雷的种类，只需要找出所有安全格
#ifndef MINESWEEPING_H
#define MINESWEEPING_H

#include <complex>
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

    enum class status
    {
        PLAYING,
        WIN,
        LOST
    }; // 游戏状态

    struct cell
    {
        int row = 0, col = 0;
        flag flg = flag::NO_FLAG;
        bool is_revealed = false;
        int adjacentMineCount = 0;          // 周围雷数
        std::complex<double> sum{0.0, 0.0}; // 周围雷的总和
        std::complex<double> val{0.0, 0.0}; // 如果这格不是雷则为0，是则为正负1，正负i的一种
    };

    struct grid
    {
        int row_num = 0;
        int col_num = 0;
        std::vector<std::vector<cell>> grd;
    };

    class Mine_sweeping
    {
        grid Grid;
        status st = status::PLAYING;
        int mine_count = 0;
        long long revealed = 0;
        bool mines_placed = false;
        void validatePosition(int row, int col) const;
        void placeMines(int safe_row, int safe_col, int seed);
        void getAdjacentStatus(int row, int col);

    public:
        Mine_sweeping(int row, int col, int MineCount);
        void start(int row, int col, int seed);     // 按种子重新布雷并翻开安全的首格
        void restart(int row, int col);             // 重置本局并翻格，雷不重排，所选格子可能是雷
        std::vector<cell> reveal(int row, int col); // 翻格，返回所有变化的格子
        void setFlag(int row, int col, flag flg);   // 标记
        status getState() const;                    // 获取游戏状态
        const grid &getGrid() const;                // 获取棋盘
    };

} // minesweeping

#endif
