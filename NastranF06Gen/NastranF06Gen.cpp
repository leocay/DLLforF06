#include "pch.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <regex>
#include <map>
#include <filesystem>
#include <locale>
#include <codecvt>  
namespace fs = std::filesystem;
using namespace std;

struct EigenVectorInfo {
    int lineIndex;
    int vectorNo;
};

vector<EigenVectorInfo> FindComplexEigenvectorLines(const vector<wstring>& lines) {
    vector<EigenVectorInfo> result;
    wregex pattern(LR"(C O M P L E X\s+E I G E N\s+V E C T O R\s+NO\.\s+(\d+))");

    for (int i = 0; i < (int)lines.size(); ++i) {
        wsmatch match;
        if (regex_search(lines[i], match, pattern)) {
            int no = stoi(match[1]);
            result.push_back({ i, no });
        }
    }
    return result;
}

vector<wstring> ExtractZeroLines(const vector<wstring>& lines, int start, int end) {
    vector<wstring> result;

    for (int i = start; i < end && i < (int)lines.size(); ++i) {
        const wstring& s = lines[i];
        if (s.empty()) continue;
        if (s[0] != L'0') continue;

        bool onlyZero = true;
        for (wchar_t c : s) {
            if (c != L'0' && c != L' ' && c != L'\t' && c != L'\r' && c != L'\n') {
                onlyZero = false;
                break;
            }
        }
        if (onlyZero) continue;

        result.push_back(s);
    }
    return result;
}

wstring safe_substr(const wstring& s, int start, int end) {
    if (start >= (int)s.size()) return L"";
    int len = max(0, min((int)s.size(), end) - start);
    return s.substr(start, len);
}

vector<wstring> ExtractSUMARY(const wstring& line) {
    vector<wstring> row(6);
    row[0] = safe_substr(line, 0, 23);
    row[1] = safe_substr(line, 23, 35);
    row[2] = safe_substr(line, 35, 57);
    row[3] = safe_substr(line, 57, 74);
    row[4] = safe_substr(line, 74, 96);
    row[5] = safe_substr(line, 96, 118);
    return row;
}

vector<vector<wstring>> ExtractSUMARYBlock(const vector<wstring>& lines) {
    int indexA = -1, indexB = -1;

    for (int i = 0; i < (int)lines.size(); ++i) {
        if (lines[i].find(L"C O M P L E X   E I G E N V A L U E   S U M M A R Y") != wstring::npos) {
            indexA = i;
            break;
        }
    }

    if (indexA == -1) return {};

    for (int i = indexA + 1; i < (int)lines.size(); ++i) {
        if (!lines[i].empty() && lines[i][0] == L'1') {
            indexB = i;
            break;
        }
    }

    if (indexB == -1) return {};

    vector<vector<wstring>> SUMARY;
    for (int i = indexA + 3; i < indexB; ++i)
        SUMARY.push_back(ExtractSUMARY(lines[i]));

    return SUMARY;
}

// 🧩 HÀM EXPORT RA DLL — DÙNG ĐƯỜNG DẪN UNICODE
extern "C" __declspec(dllexport)
bool GenerateF06File(const wchar_t* inputPath, const wchar_t* outputPath, wchar_t* errorMsg, int errorSize)
{
    try {
        fs::path inPath(inputPath);
        fs::path outPath(outputPath);
        fs::path outDir = outPath.parent_path();

        if (!fs::exists(outDir))
            fs::create_directories(outDir);

        wifstream file(inPath);
        file.imbue(locale("en_US.UTF-8"));
        if (!file.is_open())
            throw std::runtime_error("file input fail");

        vector<wstring> lines;
        wstring line;
        while (getline(file, line)) {
            lines.push_back(line);
        }
        file.close();

        auto SUMARY = ExtractSUMARYBlock(lines);
        auto eigenLines = FindComplexEigenvectorLines(lines);

        map<int, vector<wstring>> groupedBlocks;
        for (size_t k = 0; k < eigenLines.size(); ++k) {
            int start = eigenLines[k].lineIndex;
            int end = (k + 1 < eigenLines.size()) ? eigenLines[k + 1].lineIndex : (int)lines.size();
            int no = eigenLines[k].vectorNo;

            auto block = ExtractZeroLines(lines, start, end);
            groupedBlocks[no].insert(groupedBlocks[no].end(), block.begin(), block.end());
        }

        vector<wstring> outputLines;

        vector<wstring> header = {
        L"1",
        L" ",
        L" ",
        L" ",
        L"                     THIS PROGRAM IS CONFIDENTIAL AND A TRADE SECRET OF MSC.SOFTWARE CORPORATION.  THE RECEIPT OR    ",
        L"                  POSSESSION OF THIS PROGRAM DOES NOT CONVEY ANY RIGHTS TO REPRODUCE OR DISCLOSE ITS CONTENTS, OR TO ",
        L"                 MANUFACTURE, USE, OR SELL ANYTHING HEREIN, IN WHOLE OR IN PART, WITHOUT THE SPECIFIC WRITTEN CONSENT",
        L"                                                    OF MSC.SOFTWARE CORPORATION.                                     ",
        L" ",
        L" ",
        L"                                              * * * * * * * * * * * * * * * * * * * *",
        L"                                              * * * * * * * * * * * * * * * * * * * *",
        L"                                              * *                                 * *",
        L"                                              * *           MSC.SOFTWARE          * *",
        L"                                              * *               CORP              * *",
        L"                                              * *                                 * *",
        L"                                              * *      M S C . N a s t r a n      * *",
        L"                                              * *                                 * *",
        L"                                              * *                                 * *",
        L"                                              * *                                 * *",
        L"                                              * *        Version 2008.0.0         * *",
        L"                                              * *            2008 r1              * *",
        L"                                              * *                                 * *",
        L"                                              * *                                 * *",
        L"                                              * *          JUN 19, 2008           * *",
        L"                                              * *                                 * *",
        L"                                              * *              Intel              * *",
        L"                                              * *MODEL PentiumIII/10724 (konnai-P * *",
        L"                                              * *   Windows XP 6.2 (Build 9200)   * *",
        L"                                              * *        Compiled for 8664        * *",
        L"                                              * *                                 * *",
        L"                                              * * * * * * * * * * * * * * * * * * * *",
        L"                                              * * * * * * * * * * * * * * * * * * * *",
        L"1                                                                          NOVEMBER   7, 2025  MSC.NASTRAN  6/19/08   PAGE     1",
        L" ",
        L"0        N A S T R A N    F I L E    A N D    S Y S T E M    P A R A M E T E R    E C H O                                           ",
        L"0                                                                                                                                    ",
        L" ",
        L"     NASTRAN OP2NEW=0 $ MSC.NASTRAN .LCL FILE                                        ",
        L"     NASTRAN BUFFSIZE=8193 $(C:/USERS/KONNAI/APPDATA/ROAMING/MSC.SOFTWARE/MSC_NASTRAN",
        L"     NASTRAN REAL=2144075776 $(PROGRAM DEFAULT)                                      ",
        L"     INIT MASTER(S)                                                                  ",
        L"     NASTRAN SYSTEM(319)=1                                                           ",
        L"1                                                                          NOVEMBER   7, 2025  MSC.NASTRAN  6/19/08   PAGE     2",
        L" ",
        L"0        N A S T R A N    E X E C U T I V E    C O N T R O L    E C H O                                                             ",
        L"0                                                                                                                                    ",
        L" ",
        L"     ID NASTRAN,FEMAP                                                                ",
        L"     SOL SEMODES                                                                     ",
        L"     TIME 10000                                                                      ",
        L"     CEND                                                                            ",
        L"1    SK2-2025_C0                                                           NOVEMBER   7, 2025  MSC.NASTRAN  6/19/08   PAGE     3",
        L" "
        };


        outputLines.insert(outputLines.end(), header.begin(), header.end());

        for (const auto& [no, block] : groupedBlocks) {
            outputLines.push_back(L"      EIGENVALUE ANALYSIS");
            outputLines.push_back(L"0");

            if (no - 1 < (int)SUMARY.size()) {
                auto row = SUMARY[no - 1];
                auto trim = [](wstring s) {
                    s.erase(0, s.find_first_not_of(L" \t\r\n"));
                    s.erase(s.find_last_not_of(L" \t\r\n") + 1);
                    return s;
                    };

                wstring header = L"No" + to_wstring(no)
                    + L"      CYCLES = " + trim(row[4])
                    + L" h=" + trim(row[5])
                    + L" ReE=" + trim(row[2])
                    + L" ReE=" + trim(row[3]);
                outputLines.push_back(header);

                wstring title = L"                                         R E A L   E I G E N V E C T O R   N O ."
                    + wstring(11 - to_wstring(no).length(), L' ') + to_wstring(no);
                outputLines.push_back(title);
                outputLines.push_back(L" ");
                outputLines.push_back(L"      POINT ID.   TYPE          T1             T2             T3             R1             R2             R3");
            }

            for (auto s : block) {
                size_t pos = s.find_first_not_of(L' ');
                if (pos != wstring::npos && s[pos] == L'0') s[pos] = L' ';
                outputLines.push_back(s);
            }
            outputLines.push_back(L" ");
        }

        if (!outputLines.empty()) {
            outputLines.pop_back();
            outputLines.pop_back();
        }

        // Footer
        outputLines.push_back(L" --------------------------------------------------------------------------------------------------------------------------");
        outputLines.push_back(L"     0     0    1    1 '        '    T    T    T    T    T     T      F      T      0     F     -1            0           F");
        outputLines.push_back(L" ");
        outputLines.push_back(L"1                                        * * * END OF JOB * * *");
        outputLines.push_back(L" ");
        outputLines.push_back(L" ");
        outputLines.push_back(L" No Symbolic Replacement variables or values were specified.");

        wofstream fout(outPath);
        fout.imbue(locale("en_US.UTF-8"));
        if (!fout.is_open())
            throw runtime_error("file output error");

        for (const auto& l : outputLines)
            fout << l << L"\n";
        fout.close();

        return true;
    }
    catch (const std::exception& ex)
    {
        std::wstring werr(ex.what(), ex.what() + strlen(ex.what()));
        wcsncpy_s(errorMsg, errorSize, werr.c_str(), _TRUNCATE);
        return false;
    }
    catch (...)
    {
        wcsncpy_s(errorMsg, errorSize, L"Lỗi không xác định trong DLL.", _TRUNCATE);
        return false;
    }
  
}
