/**
 * @file WordGraphProcessor.cpp
 * @brief 软件工程lab1：基于文本的有向图构建与操作（最终版，含编码设置）
 * @version 1.3
 * 
 * 编译命令（g++）：
 *   g++ -std=c++17 WordGraphProcessor.cpp -o WordGraphProcessor
 * 
 * 注意：
 * - 在Windows下，程序会自动将控制台输出代码页设为UTF-8以正确显示中文。
 * - 若仍出现乱码，请确保源文件保存为UTF-8，并在运行程序前手动执行 chcp 65001。
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <queue>
#include <stack>
#include <algorithm>
#include <cctype>
#include <random>
#include <cmath>
#include <functional>
#include <chrono>

#ifdef _WIN32
#include <windows.h>  // 用于设置控制台编码
#endif

// ==================== 辅助函数：路径处理（不使用filesystem）====================

// 获取父目录路径（末尾带路径分隔符）
std::string getParentDir(const std::string& filepath) {
#ifdef _WIN32
    const char sep = '\\';
#else
    const char sep = '/';
#endif
    size_t pos = filepath.find_last_of(sep);
    if (pos == std::string::npos) {
        return "";
    }
    return filepath.substr(0, pos + 1);
}

// 获取文件名（不带扩展名）
std::string getFileStem(const std::string& filepath) {
#ifdef _WIN32
    const char sep = '\\';
#else
    const char sep = '/';
#endif
    size_t start = filepath.find_last_of(sep);
    if (start == std::string::npos) start = 0;
    else start++;

    size_t dot = filepath.find_last_of('.');
    if (dot == std::string::npos || dot < start) {
        return filepath.substr(start);
    }
    return filepath.substr(start, dot - start);
}

// 检查文件是否存在（可读）
bool fileExists(const std::string& filepath) {
    std::ifstream f(filepath.c_str());
    return f.good();
}

// ==================== 全局图对象 ====================
class Graph {
private:
    std::vector<std::string> nodes;
    std::unordered_map<std::string, int> wordToIndex;
    std::vector<std::unordered_map<int, int>> adjList;

    int getOrAddNode(const std::string& word) {
        auto it = wordToIndex.find(word);
        if (it != wordToIndex.end()) {
            return it->second;
        }
        int idx = nodes.size();
        nodes.push_back(word);
        wordToIndex[word] = idx;
        adjList.emplace_back();
        return idx;
    }

public:
    static std::string toLower(const std::string& s) {
        std::string res;
        res.reserve(s.size());
        for (char c : s) {
            res.push_back(std::tolower(static_cast<unsigned char>(c)));
        }
        return res;
    }

    void addEdge(const std::string& word1, const std::string& word2) {
        int u = getOrAddNode(word1);
        int v = getOrAddNode(word2);
        adjList[u][v]++;
    }

    void buildFromWordList(const std::vector<std::string>& words) {
        for (size_t i = 0; i + 1 < words.size(); ++i) {
            addEdge(words[i], words[i + 1]);
        }
    }

    bool hasWord(const std::string& word) const {
        return wordToIndex.find(toLower(word)) != wordToIndex.end();
    }

    int getIndex(const std::string& word) const {
        auto it = wordToIndex.find(toLower(word));
        return (it != wordToIndex.end()) ? it->second : -1;
    }

    int nodeCount() const { return nodes.size(); }
    const std::string& getWord(int idx) const { return nodes[idx]; }
    const std::unordered_map<int, int>& getOutEdges(int idx) const { return adjList[idx]; }

    int outDegreeSum(int idx) const {
        int sum = 0;
        for (const auto& p : adjList[idx]) sum += p.second;
        return sum;
    }

    void showDirectedGraph(const std::string& inputFilePath) const {
        std::cout << "\n===== 有向图信息 =====\n";
        std::cout << "节点总数: " << nodes.size() << "\n";
        for (size_t i = 0; i < nodes.size(); ++i) {
            std::cout << "节点 \"" << nodes[i] << "\" 的出边: ";
            if (adjList[i].empty()) {
                std::cout << "无";
            } else {
                for (const auto& [v, w] : adjList[i]) {
                    std::cout << "\"" << nodes[v] << "\"(" << w << ") ";
                }
            }
            std::cout << "\n";
        }
        std::cout << "========================\n";

        std::string dir = getParentDir(inputFilePath);
        std::string stem = getFileStem(inputFilePath);
        std::string dotPath = dir + stem + ".dot";
        std::string picPath = dir + stem + "pic.png";

        std::ofstream dotFile(dotPath);
        if (!dotFile) {
            std::cerr << "无法创建dot文件: " << dotPath << std::endl;
            return;
        }

        dotFile << "digraph G {\n";
        dotFile << "  node [shape=box];\n";
        for (size_t u = 0; u < nodes.size(); ++u) {
            for (const auto& [v, w] : adjList[u]) {
                dotFile << "  \"" << nodes[u] << "\" -> \"" << nodes[v]
                        << "\" [label=\"" << w << "\"];\n";
            }
        }
        dotFile << "}\n";
        dotFile.close();

        std::string cmd = "dot -Tpng \"" + dotPath + "\" -o \"" + picPath + "\"";
        int ret = std::system(cmd.c_str());
        if (ret == 0) {
            std::cout << "有向图已保存至: " << picPath << std::endl;
        } else {
            std::cerr << "调用dot命令失败，请确保Graphviz已安装且dot在PATH中。\n";
        }
    }

    std::string queryBridgeWords(const std::string& word1, const std::string& word2) const {
        std::string w1 = toLower(word1);
        std::string w2 = toLower(word2);
        int u = getIndex(w1);
        int v = getIndex(w2);
        if (u == -1 && v == -1) return "No \"" + w1 + "\" and \"" + w2 + "\" in the graph!";
        if (u == -1) return "No \"" + w1 + "\" in the graph!";
        if (v == -1) return "No \"" + w2 + "\" in the graph!";

        std::vector<std::string> bridges;
        for (size_t k = 0; k < nodes.size(); ++k) {
            auto it1 = adjList[u].find(k);
            auto it2 = adjList[k].find(v);
            if (it1 != adjList[u].end() && it2 != adjList[k].end()) {
                bridges.push_back(nodes[k]);
            }
        }

        if (bridges.empty()) return "No bridge words from \"" + w1 + "\" to \"" + w2 + "\"!";

        std::ostringstream oss;
        oss << "The bridge word" << (bridges.size() > 1 ? "s" : "")
            << " from \"" << w1 << "\" to \"" << w2 << "\" "
            << (bridges.size() > 1 ? "are" : "is") << ": ";
        for (size_t i = 0; i < bridges.size(); ++i) {
            if (i > 0) {
                if (i == bridges.size() - 1) oss << " and ";
                else oss << ", ";
            }
            oss << "\"" << bridges[i] << "\"";
        }
        oss << ".";
        return oss.str();
    }

    std::string generateNewText(const std::string& inputText) const {
        std::vector<std::string> words;
        std::string current;
        for (char ch : inputText) {
            if (std::isalpha(static_cast<unsigned char>(ch))) {
                current.push_back(std::tolower(static_cast<unsigned char>(ch)));
            } else {
                if (!current.empty()) {
                    words.push_back(current);
                    current.clear();
                }
            }
        }
        if (!current.empty()) words.push_back(current);
        if (words.empty()) return "";

        static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

        std::vector<std::string> newWords;
        newWords.push_back(words[0]);

        for (size_t i = 0; i + 1 < words.size(); ++i) {
            const std::string& w1 = words[i];
            const std::string& w2 = words[i + 1];
            int u = getIndex(w1);
            int v = getIndex(w2);
            std::vector<std::string> bridges;

            if (u != -1 && v != -1) {
                for (size_t k = 0; k < nodes.size(); ++k) {
                    auto it1 = adjList[u].find(k);
                    auto it2 = adjList[k].find(v);
                    if (it1 != adjList[u].end() && it2 != adjList[k].end()) {
                        bridges.push_back(nodes[k]);
                    }
                }
            }

            if (!bridges.empty()) {
                std::uniform_int_distribution<size_t> dist(0, bridges.size() - 1);
                newWords.push_back(bridges[dist(rng)]);
            }
            newWords.push_back(w2);
        }

        std::ostringstream oss;
        for (size_t i = 0; i < newWords.size(); ++i) {
            if (i > 0) oss << ' ';
            oss << newWords[i];
        }
        return oss.str();
    }

    std::string calcShortestPath(const std::string& word1, const std::string& word2) const {
        std::string w1 = toLower(word1);
        std::string w2 = toLower(word2);
        int src = getIndex(w1);
        int dst = getIndex(w2);
        if (src == -1 && dst == -1) return "No \"" + w1 + "\" and \"" + w2 + "\" in the graph!";
        if (src == -1) return "No \"" + w1 + "\" in the graph!";
        if (dst == -1) return "No \"" + w2 + "\" in the graph!";

        const int INF = 1e9;
        int n = nodes.size();
        std::vector<int> dist(n, INF);
        std::vector<bool> visited(n, false);
        dist[src] = 0;

        using P = std::pair<int, int>;
        std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
        pq.emplace(0, src);

        while (!pq.empty()) {
            auto [d, u] = pq.top();
            pq.pop();
            if (visited[u]) continue;
            visited[u] = true;
            if (u == dst) break;

            for (const auto& [v, w] : adjList[u]) {
                if (d + w < dist[v]) {
                    dist[v] = d + w;
                    pq.emplace(dist[v], v);
                }
            }
        }

        if (dist[dst] == INF) return "No path from \"" + w1 + "\" to \"" + w2 + "\"!";

        std::vector<std::vector<int>> paths;
        std::vector<int> currentPath;
        currentPath.push_back(src);

        std::function<void(int)> dfs = [&](int u) {
            if (u == dst) {
                paths.push_back(currentPath);
                return;
            }
            for (const auto& [v, w] : adjList[u]) {
                if (dist[u] + w == dist[v]) {
                    currentPath.push_back(v);
                    dfs(v);
                    currentPath.pop_back();
                    if (paths.size() >= 3) return;
                }
            }
        };

        dfs(src);

        std::ostringstream oss;
        oss << "Shortest path length (sum of weights): " << dist[dst] << "\n";
        oss << "Path" << (paths.size() > 1 ? "s" : "") << ":\n";
        for (size_t i = 0; i < paths.size(); ++i) {
            oss << "  ";
            for (size_t j = 0; j < paths[i].size(); ++j) {
                if (j > 0) oss << " -> ";
                oss << nodes[paths[i][j]];
            }
            oss << "\n";
        }
        return oss.str();
    }

    std::string calcShortestPathsFromWord(const std::string& word) const {
        std::string w = toLower(word);
        int src = getIndex(w);
        if (src == -1) return "No \"" + w + "\" in the graph!";

        int n = nodes.size();
        const int INF = 1e9;
        std::vector<int> dist(n, INF);
        std::vector<int> prev(n, -1);
        dist[src] = 0;
        using P = std::pair<int, int>;
        std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
        pq.emplace(0, src);

        while (!pq.empty()) {
            auto [d, u] = pq.top();
            pq.pop();
            if (d > dist[u]) continue;
            for (const auto& [v, w] : adjList[u]) {
                if (d + w < dist[v]) {
                    dist[v] = d + w;
                    prev[v] = u;
                    pq.emplace(dist[v], v);
                }
            }
        }

        std::ostringstream oss;
        oss << "Shortest paths from \"" << w << "\" to all other nodes:\n";
        for (int i = 0; i < n; ++i) {
            if (i == src) continue;
            oss << "To \"" << nodes[i] << "\": ";
            if (dist[i] == INF) {
                oss << "unreachable\n";
            } else {
                oss << "distance = " << dist[i] << ", path: ";
                std::vector<int> path;
                for (int v = i; v != -1; v = prev[v]) path.push_back(v);
                std::reverse(path.begin(), path.end());
                for (size_t j = 0; j < path.size(); ++j) {
                    if (j > 0) oss << " -> ";
                    oss << nodes[path[j]];
                }
                oss << "\n";
            }
        }
        return oss.str();
    }

    double calPageRank(const std::string& word) const {
        std::string w = toLower(word);
        int idx = getIndex(w);
        if (idx == -1) return -1.0;

        int n = nodes.size();
        if (n == 0) return 0.0;

        const double d = 0.85;
        const double eps = 1e-8;
        const int maxIter = 200;

        std::vector<double> pr(n, 1.0 / n);

        for (int iter = 0; iter < maxIter; ++iter) {
            std::vector<double> newPr(n, (1.0 - d) / n);
            double danglingSum = 0.0;
            for (int u = 0; u < n; ++u) {
                if (adjList[u].empty()) danglingSum += pr[u];
            }
            double addToAll = d * danglingSum / n;

            for (int u = 0; u < n; ++u) {
                if (adjList[u].empty()) continue;
                double outSum = outDegreeSum(u);
                for (const auto& [v, w] : adjList[u]) {
                    newPr[v] += d * pr[u] * (w / outSum);
                }
            }

            for (int v = 0; v < n; ++v) newPr[v] += addToAll;

            double diff = 0.0;
            for (int i = 0; i < n; ++i) diff += std::abs(newPr[i] - pr[i]);
            pr = std::move(newPr);
            if (diff < eps) break;
        }

        return pr[idx];
    }

    std::string randomWalk() const {
        if (nodes.empty()) return "";

        static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<int> nodeDist(0, nodes.size() - 1);
        int current = nodeDist(rng);
        std::vector<std::string> path;
        path.push_back(nodes[current]);

        std::set<std::pair<int, int>> visitedEdges;

        std::cout << "\n随机游走开始，当前路径: " << nodes[current];
        while (true) {
            std::cout << "\n按 q 停止游走，按其他键继续... ";
            char ch;
            std::cin >> ch;
            if (ch == 'q' || ch == 'Q') {
                path.push_back("INTERRUPT");
                break;
            }

            if (adjList[current].empty()) break;

            std::vector<int> targets, weights;
            for (const auto& [v, w] : adjList[current]) {
                targets.push_back(v);
                weights.push_back(w);
            }
            std::discrete_distribution<int> dist(weights.begin(), weights.end());
            int next = targets[dist(rng)];

            auto edge = std::make_pair(current, next);
            if (visitedEdges.count(edge)) break;
            visitedEdges.insert(edge);

            current = next;
            path.push_back(nodes[current]);
            std::cout << " -> " << nodes[current];
        }

        std::ostringstream oss;
        for (size_t i = 0; i < path.size(); ++i) {
            if (i > 0) oss << ' ';
            oss << path[i];
        }
        return oss.str();
    }
};

Graph g_graph;

std::vector<std::string> readWordsFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) throw std::runtime_error("无法打开文件: " + filePath);

    std::vector<std::string> words;
    std::string line, currentWord;
    auto flushWord = [&]() {
        if (!currentWord.empty()) {
            words.push_back(Graph::toLower(currentWord));
            currentWord.clear();
        }
    };

    while (std::getline(file, line)) {
        for (char ch : line) {
            if (std::isalpha(static_cast<unsigned char>(ch))) {
                currentWord.push_back(ch);
            } else {
                flushWord();
            }
        }
        flushWord();
    }
    return words;
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::cout << "========== 软件工程lab1：基于文本的有向图处理 ==========\n";

    std::string filePath;
    if (argc > 1) {
        filePath = argv[1];
    } else {
        std::cout << "请输入文本文件路径: ";
        std::getline(std::cin, filePath);
    }

    if (!fileExists(filePath)) {
        std::cerr << "文件不存在或无法打开: " << filePath << std::endl;
        std::cout << "请重新输入文件路径: ";
        std::getline(std::cin, filePath);
        if (!fileExists(filePath)) {
            std::cerr << "仍然无法打开，程序退出。" << std::endl;
            return 1;
        }
    }

    try {
        auto words = readWordsFromFile(filePath);
        g_graph.buildFromWordList(words);
        std::cout << "成功读取 " << words.size() << " 个单词，构建有向图完成。\n";
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    g_graph.showDirectedGraph(filePath);

    while (true) {
        std::cout << "\n========== 功能菜单 ==========\n";
        std::cout << "1. 查询桥接词\n";
        std::cout << "2. 根据桥接词生成新文本\n";
        std::cout << "3. 计算两个单词的最短路径\n";
        std::cout << "4. 计算单个单词到所有其他单词的最短路径（进阶）\n";
        std::cout << "5. 计算单词的PageRank值\n";
        std::cout << "6. 随机游走\n";
        std::cout << "0. 退出\n";
        std::cout << "请选择: ";

        int choice;
        std::cin >> choice;
        std::cin.ignore();

        if (choice == 0) break;

        // 辅助函数：从字符串提取第一个单词（连续字母）
        auto extractFirstWord = [](const std::string& s) -> std::string {
            std::string word;
            bool inWord = false;
            for (char ch : s) {
                if (std::isalpha(static_cast<unsigned char>(ch))) {
                    word.push_back(ch);
                    inWord = true;
                } else if (inWord) {
                    break;  // 遇到非字母且已经在单词中，则结束
                }
            }
            return word;
        };

        switch (choice) {
            case 1: {
                std::cout << "请输入两个单词（可空格/逗号分隔）: ";
                std::string line;
                std::getline(std::cin, line);
                std::vector<std::string> words;
                std::string cur;
                for (char ch : line) {
                    if (std::isalpha(static_cast<unsigned char>(ch))) {
                        cur.push_back(ch);
                    } else {
                        if (!cur.empty()) {
                            words.push_back(cur);
                            cur.clear();
                        }
                    }
                }
                if (!cur.empty()) words.push_back(cur);

                if (words.size() == 1) {
                    std::cout << "请输入第二个单词: ";
                    std::string w2;
                    std::getline(std::cin, w2);
                    std::string w2clean = extractFirstWord(w2);
                    if (w2clean.empty()) {
                        std::cout << "输入无效，请重新开始。\n";
                        break;
                    }
                    words.push_back(w2clean);
                } else if (words.size() != 2) {
                    std::cout << "输入单词数量不对，请重新开始。\n";
                    break;
                }

                std::cout << g_graph.queryBridgeWords(words[0], words[1]) << std::endl;
                break;
            }
            case 2: {
                std::cout << "请输入一行新文本: ";
                std::string text;
                std::getline(std::cin, text);
                std::cout << "生成的新文本: " << g_graph.generateNewText(text) << std::endl;
                break;
            }
            case 3: {
                std::cout << "请输入两个单词: ";
                std::string line;
                std::getline(std::cin, line);
                std::vector<std::string> words;
                std::string cur;
                for (char ch : line) {
                    if (std::isalpha(static_cast<unsigned char>(ch))) {
                        cur.push_back(ch);
                    } else {
                        if (!cur.empty()) {
                            words.push_back(cur);
                            cur.clear();
                        }
                    }
                }
                if (!cur.empty()) words.push_back(cur);
                if (words.size() == 1) {
                    std::cout << "请输入第二个单词: ";
                    std::string w2;
                    std::getline(std::cin, w2);
                    std::string w2clean = extractFirstWord(w2);
                    if (w2clean.empty()) {
                        std::cout << "输入无效。\n";
                        break;
                    }
                    words.push_back(w2clean);
                } else if (words.size() != 2) {
                    std::cout << "输入无效。\n";
                    break;
                }
                std::cout << g_graph.calcShortestPath(words[0], words[1]) << std::endl;
                break;
            }
            case 4: {
                std::cout << "请输入一个单词: ";
                std::string word;
                std::getline(std::cin, word);
                std::string w = extractFirstWord(word);
                if (w.empty()) {
                    std::cout << "输入无效。\n";
                    break;
                }
                std::cout << g_graph.calcShortestPathsFromWord(w) << std::endl;
                break;
            }
            case 5: {
                std::cout << "请输入一个单词: ";
                std::string word;
                std::getline(std::cin, word);
                std::string w = extractFirstWord(word);
                if (w.empty()) {
                    std::cout << "输入无效。\n";
                    break;
                }
                double pr = g_graph.calPageRank(w);
                if (pr < 0) std::cout << "单词 \"" << w << "\" 不在图中。\n";
                else std::cout << "PageRank值 of \"" << w << "\": " << pr << std::endl;
                break;
            }
            case 6: {
                std::string walk = g_graph.randomWalk();
                std::cout << "\n随机游走路径: " << walk << std::endl;

                std::string dir = getParentDir(filePath);
                std::string stem = getFileStem(filePath);
                std::string outPath = dir + stem + "random.txt";
                std::ofstream ofs(outPath);
                if (ofs) {
                    ofs << walk;
                    std::cout << "随机游走文本已保存至: " << outPath << std::endl;
                } else {
                    std::cerr << "无法保存文件。" << std::endl;
                }
                break;
            }
            default:
                std::cout << "无效选择，请重新输入。\n";
        }
    }

    std::cout << "程序结束。\n";
    return 0;
}
