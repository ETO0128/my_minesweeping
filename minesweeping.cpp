#include "minesweeping.h"
#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>
#include <queue>

namespace minesweeping
{
    Mine_sweeping::Mine_sweeping(int row, int col,
                                 int MineCount)
    {
        if (row <= 0 || col <= 0 || MineCount < 0 ||
            static_cast<long long>(MineCount) >= static_cast<long long>(row) * col)
            throw std::invalid_argument("Invalid dimensions or mine count");
        Grid.row_num = row;
        Grid.col_num = col;
        Grid.grd.resize(row);
        for (int r = 0; r < row; ++r)
        {
            Grid.grd[r].resize(col);
            for (int c = 0; c < col; ++c)
            {
                Grid.grd[r][c].row = r;
                Grid.grd[r][c].col = c;
            }
        }
        mine_count = MineCount;
    }

    void Mine_sweeping::validatePosition(int row, int col) const
    {
        if (row < 0 || row >= Grid.row_num || col < 0 || col >= Grid.col_num)
            throw std::out_of_range("Invalid position");
    }

    void Mine_sweeping::getAdjacentStatus(int row, int col)
    {
        Grid.grd[row][col].adjacentMineCount = 0;
        Grid.grd[row][col].sum = {0.0, 0.0};
        for (int r = row - 1; r <= row + 1; r++)
        {
            for (int c = col - 1; c <= col + 1; c++)
            {
                if (r < 0 || r >= Grid.row_num || c < 0 ||
                    c >= Grid.col_num || (c == col && r == row))
                    continue;
                if (Grid.grd[r][c].val != std::complex<double>{0.0, 0.0})
                {
                    Grid.grd[row][col].adjacentMineCount++;
                    Grid.grd[row][col].sum += Grid.grd[r][c].val;
                }
            }
        }
    }

    void Mine_sweeping::placeMines(int safe_row, int safe_col, int seed)
    {
        std::vector<std::pair<int, int>> candidates;

        for (int r = 0; r < Grid.row_num; ++r)
        {
            for (int c = 0; c < Grid.col_num; ++c)
            {
                if (r == safe_row && c == safe_col)
                    continue;

                candidates.emplace_back(r, c);
            }
        }

        if (mine_count < 0 ||
            static_cast<std::size_t>(mine_count) > candidates.size())
        {
            throw std::invalid_argument("Number of Mines is too large!");
        }

        std::mt19937 rng(seed);
        std::shuffle(candidates.begin(), candidates.end(), rng);

        const std::complex<double> types[] = {
            {1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        std::uniform_int_distribution<int> choose_type(0, 3);

        for (int i = 0; i < mine_count; ++i)
        {
            const auto [r, c] = candidates[i]; // C++17
            Grid.grd[r][c].val = types[choose_type(rng)];
        }

        for (int r = 0; r < Grid.row_num; ++r)
        {
            for (int c = 0; c < Grid.col_num; ++c)
            {
                getAdjacentStatus(r, c);
            }
        }

        mines_placed = true;
    }

    void Mine_sweeping::start(int row, int col, int seed)
    {
        validatePosition(row, col);
        // Preserve pregame flags so the first flood fill respects them.
        const bool preserve_flags = !mines_placed;
        for (auto &line : Grid.grd)
        {
            for (auto &item : line)
            {
                const int r = item.row, c = item.col;
                const flag previous_flag = item.flg;
                item = cell{};
                if (preserve_flags)
                    item.flg = previous_flag;
                item.row = r;
                item.col = c;
            }
        }
        st = status::PLAYING;
        revealed = 0;
        mines_placed = false;
        placeMines(row, col, seed);
        reveal(row, col);
    }

    void Mine_sweeping::restart(int row, int col)
    {
        validatePosition(row, col);
        if (!mines_placed)
            throw std::logic_error("Call start before restart");
        st = status::PLAYING;
        revealed = 0;
        for (int r = 0; r < Grid.row_num; r++)
        {
            for (int c = 0; c < Grid.col_num; c++)
            {
                Grid.grd[r][c].flg = flag::NO_FLAG;
                Grid.grd[r][c].is_revealed = false;
            }
        }
        reveal(row, col);
    }

    std::vector<cell> Mine_sweeping::reveal(int row, int col)
    {
        validatePosition(row, col);
        if (!mines_placed)
            throw std::logic_error("Call start before reveal");
        if (st != status::PLAYING)
            return {};

        std::vector<cell> changed;

        auto &first = Grid.grd[row][col];
        if (first.is_revealed || first.flg != flag::NO_FLAG)
        {
            return changed;
        }

        first.is_revealed = true;
        changed.push_back(first);

        if (first.val != 0.0)
        {
            st = status::LOST;
            return changed;
        }
        revealed++;

        std::queue<std::pair<int, int>> pending;
        if (first.adjacentMineCount == 0)
            pending.emplace(row, col);

        while (!pending.empty())
        {
            auto [r, c] = pending.front(); // C++17
            pending.pop();

            for (int dr = -1; dr <= 1; ++dr)
            {
                for (int dc = -1; dc <= 1; ++dc)
                {
                    if (dr == 0 && dc == 0)
                        continue;

                    int nr = r + dr;
                    int nc = c + dc;

                    if (nr < 0 || nr >= Grid.row_num ||
                        nc < 0 || nc >= Grid.col_num)
                        continue;

                    auto &next = Grid.grd[nr][nc];

                    if (next.is_revealed ||
                        next.flg != flag::NO_FLAG ||
                        next.val != 0.0)
                        continue;

                    next.is_revealed = true;
                    revealed++;
                    changed.push_back(next);

                    if (next.adjacentMineCount == 0)
                        pending.emplace(nr, nc);
                }
            }
        }

        if (revealed == static_cast<long long>(Grid.col_num) * Grid.row_num - mine_count)
            st = status::WIN;
        return changed;
    }

    void Mine_sweeping::setFlag(int row, int col, flag flg)
    {
        validatePosition(row, col);
        switch (flg)
        {
        case flag::NO_FLAG:
        case flag::PLUS_ONE:
        case flag::NEG_ONE:
        case flag::PLUS_I:
        case flag::NEG_I:
            break;
        default:
            throw std::invalid_argument("Invalid flag");
        }
        if (st != status::PLAYING || Grid.grd[row][col].is_revealed)
            return;
        Grid.grd[row][col].flg = flg;
    }

    status Mine_sweeping::getState() const
    {
        return st;
    }

    const grid &Mine_sweeping::getGrid() const
    {
        return Grid;
    }

} // minesweeping
