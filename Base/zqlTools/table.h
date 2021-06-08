#ifndef _TABLE_H__
#define _TABLE_H__
#include "zqlTools.h"
#include <string>
#include <vector>

static inline size_t max_size_t(size_t a, size_t b) { return a > b ? a : b; }

namespace zql {

class Table {
private:
    using _string = std::wstring;
    using _line = std::vector<_string>;
    using _table = std::vector<_line>;

    size_t m_row;                   // 行数
    size_t m_col;                   // 列数
    _table m_table;                 // 表内容
    std::vector<size_t> m_colWidth; // 每列的宽度
    std::vector<size_t> m_minWidth; // 指定的宽度

public:
    Table(size_t row = 2, size_t col = 2)
        : m_row(row), m_col(col), m_table(m_row, _line(m_col, _string())),
          m_colWidth(col, 0), m_minWidth(col, 0) {}

    void Set(size_t x, size_t y, const std::wstring& str) {
        m_table[x][y] = str;

        // col length
        m_colWidth[y] = 0;
        for (size_t i = 0; i < m_row; i++)
            m_colWidth[y] = max_size_t(m_colWidth[y], CallSize(m_table[i][y]));
        m_colWidth[y] = max_size_t(m_colWidth[y], m_minWidth[y]);
    }

    void AddLine(_line line) {
        m_row++;
        m_table.push_back(line);

        for (size_t y = 0; y < m_col; y++) {
            // col length
            m_colWidth[y] = 0;
            for (size_t i = 0; i < m_row; i++)
                m_colWidth[y] =
                    max_size_t(m_colWidth[y], CallSize(m_table[i][y]));
            m_colWidth[y] = max_size_t(m_colWidth[y], m_minWidth[y]);
        }
    }

    void Set(size_t x, size_t y, const std::string& str) {
        if (zql::GetCoding(str) == UTF8)
            Set(x, y, zql::UTF8ToUnicode(str));
        else
            Set(x, y, zql::ANSIToUnicode(str));
    }

    // 用户设置每列的宽度，为零表示使用最短长度，若比最长字符串长度小，则不会采用此值
    void SetWidth(size_t y, size_t width) {
        m_minWidth[y] = width;
        m_colWidth[y] = max_size_t(m_colWidth[y], m_minWidth[y]);
    }

    std::string GetUTF8() { return zql::UnicodeToUTF8(GetTable()); }
    std::string GetANSI() { return zql::UnicodeToANSI(GetTable()); }
    std::wstring GetUnicode() { return GetTable(); }

private:
    _string GetTable() {
        _string ret;
        // 第一行
        ret += SplitLine(1);
        // 中间行
        for (size_t i = 0; i < m_row; i++) {
            // 中间分割行
            if (i != 0)
                ret += SplitLine(2);
            // 行内容
            for (size_t j = 0; j < m_col; j++)
                // 和表格两边的 | 空一格格子
                ret += L"│ " + Align(i, j) + L' ';
            ret += L"│\n";
        }
        // 最后一行
        ret += SplitLine(3);

        return ret;
    }

    size_t CallSize(_string& str) {
        size_t n = zql::UnicodeToUTF8(str).size() - str.size();
        return str.size() + n / 2;
    }

    // 补齐字符以实现对其
    _string Align(size_t x, size_t y) {
        _string& src = m_table[x][y];
        size_t srcSize = CallSize(src);
        // console show for ANSI, so use ANSI size
        if (m_colWidth[y] > srcSize)
            return src + _string(m_colWidth[y] - srcSize, L' ');
        else
            return src;
    }

    // 1表示第一行，2表示中间行的分割线，3表示最后一行分割线
    _string SplitLine(int type) {
        wchar_t a, b, c;
        switch (type) {
        case 1:
            a = L'┌';
            b = L'┬';
            c = L'┐';
            break;
        case 2:
            a = L'├';
            b = L'┼';
            c = L'┤';
            break;
        case 3:
        default:
            a = L'└';
            b = L'┴';
            c = L'┘';
            break;
        }

        _string ret;
        ret += a;
        for (size_t i = 0; i < m_col; i++)
            // 和表格两边的 | 空一格格子
            ret += _string(m_colWidth[i] + 2, L'─') + b;
        ret.pop_back();
        ret += c;
        ret += L'\n';

        return ret;
    }
};
} // namespace zql
#endif // !_TABLE_H__
