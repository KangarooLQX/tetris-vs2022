#include <graphics.h>
#include <windows.h>
#include <tchar.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include <ctime>

// 窗口尺寸
const int WIDTH = 1000;
const int HEIGHT = 1200;

// 按钮结构体
struct Button 
{
    int x1, y1, x2, y2;
    const char* text;
};

// 按钮定义
Button btnStart = { WIDTH / 2 - 80, HEIGHT / 2 - 60, WIDTH / 2 + 80, HEIGHT / 2 - 20, "开始游戏" };
Button btnExit = { WIDTH / 2 - 80, HEIGHT / 2 + 20, WIDTH / 2 + 80, HEIGHT / 2 + 60, "退出游戏" };
Button btnPause = { WIDTH - 110, 10, WIDTH - 10, 50, "暂停" };
Button btnBack = { WIDTH - 110, 60, WIDTH - 10, 100, "返回菜单" };
Button btnDifficulty = { WIDTH / 2 - 80, HEIGHT / 2 + 80, WIDTH / 2 + 80, HEIGHT / 2 + 120, "难度选择" };
Button btnEasy = { WIDTH / 2 - 180, HEIGHT / 2 + 160, WIDTH / 2 - 20, HEIGHT / 2 + 200, "简单" };
Button btnNormal = { WIDTH / 2 - 60,  HEIGHT / 2 + 160, WIDTH / 2 + 100, HEIGHT / 2 + 200, "普通" };
Button btnHard = { WIDTH / 2 + 60,  HEIGHT / 2 + 160, WIDTH / 2 + 220, HEIGHT / 2 + 200, "困难" };

// 将 const char* 转换为 TCHAR[]
void toTCHAR(const char* src, TCHAR* dest, int size) 
{
#ifdef _UNICODE
    MultiByteToWideChar(CP_ACP, 0, src, -1, dest, size);
#else
    strncpy_s(dest, size, src, _TRUNCATE);
#endif
}

// 判断按钮点击
bool isClicked(const Button& btn, int x, int y) 
{
    return x >= btn.x1 && x <= btn.x2 && y >= btn.y1 && y <= btn.y2;
}

// 绘制按钮
void drawButton(const Button& btn) 
{
    setfillcolor(RGB(200, 200, 200));
    solidrectangle(btn.x1, btn.y1, btn.x2, btn.y2);
    settextstyle(20, 0, _T("Consolas"));
    settextcolor(BLACK);
    TCHAR buf[64];
    toTCHAR(btn.text, buf, 64);
    int tw = textwidth(buf), th = textheight(buf);
    outtextxy((btn.x1 + btn.x2 - tw) / 2, (btn.y1 + btn.y2 - th) / 2, buf);
}

// 地图尺寸及信息区
const int MAP_W = 18;
const int MAP_H = 30;
int gameMap[MAP_H][MAP_W] = { 0 };
const int INFO_X = 50 + MAP_W * 25 + 40;

// 7种方块数据
const int BLOCKS[7][4][4][4] = 
{
    // I
    {{{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
     {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}},
     {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
     {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}}},
     // O
     {{{0,0,0,0},{0,2,2,0},{0,2,2,0},{0,0,0,0}},
      {{0,0,0,0},{0,2,2,0},{0,2,2,0},{0,0,0,0}},
      {{0,0,0,0},{0,2,2,0},{0,2,2,0},{0,0,0,0}},
      {{0,0,0,0},{0,2,2,0},{0,2,2,0},{0,0,0,0}}},
      // T
      {{{0,0,0,0},{3,3,3,0},{0,3,0,0},{0,0,0,0}},
       {{0,3,0,0},{3,3,0,0},{0,3,0,0},{0,0,0,0}},
       {{0,3,0,0},{3,3,3,0},{0,0,0,0},{0,0,0,0}},
       {{0,3,0,0},{0,3,3,0},{0,3,0,0},{0,0,0,0}}},
       // S
       {{{0,0,0,0},{0,4,4,0},{4,4,0,0},{0,0,0,0}},
        {{4,0,0,0},{4,4,0,0},{0,4,0,0},{0,0,0,0}},
        {{0,0,0,0},{0,4,4,0},{4,4,0,0},{0,0,0,0}},
        {{4,0,0,0},{4,4,0,0},{0,4,0,0},{0,0,0,0}}},
        // Z
        {{{0,0,0,0},{5,5,0,0},{0,5,5,0},{0,0,0,0}},
         {{0,5,0,0},{5,5,0,0},{5,0,0,0},{0,0,0,0}},
         {{0,0,0,0},{5,5,0,0},{0,5,5,0},{0,0,0,0}},
         {{0,5,0,0},{5,5,0,0},{5,0,0,0},{0,0,0,0}}},
         // J
         {{{0,0,0,0},{6,6,6,0},{0,0,6,0},{0,0,0,0}},
          {{0,6,0,0},{0,6,0,0},{6,6,0,0},{0,0,0,0}},
          {{6,0,0,0},{6,6,6,0},{0,0,0,0},{0,0,0,0}},
          {{0,6,6,0},{0,6,0,0},{0,6,0,0},{0,0,0,0}}},
          // L
          {{{0,0,0,0},{7,7,7,0},{7,0,0,0},{0,0,0,0}},
           {{7,7,0,0},{0,7,0,0},{0,7,0,0},{0,0,0,0}},
           {{0,0,7,0},{7,7,7,0},{0,0,0,0},{0,0,0,0}},
           {{0,7,0,0},{0,7,0,0},{0,7,7,0},{0,0,0,0}}}
};

// 方块结构体
struct Block 
{
    int type, rotation, x, y;
};
Block curBlock, nextBlock;

enum Difficulty { EASY, NORMAL, HARD };
Difficulty currentDifficulty = NORMAL;
int fallInterval = 500, scoreMultiplier = 2;

// 分数板
struct ScoreEntry { int score; };
std::vector<ScoreEntry> scoreBoard;
int highScore = 0;
const char* SCORE_FILE = "tetris_score.txt";

void loadScoreBoard() 
{
    scoreBoard.clear();
    std::ifstream fin(SCORE_FILE);
    int s;
    while (fin >> s) scoreBoard.push_back({ s });
    fin.close();
    std::sort(scoreBoard.begin(), scoreBoard.end(), [](auto& a, auto& b) { return a.score > b.score; });
    if (!scoreBoard.empty()) highScore = scoreBoard.front().score;
    if (scoreBoard.size() > 10) scoreBoard.resize(10);
}

void saveScoreBoard() 
{
    std::ofstream fout(SCORE_FILE);
    for (auto& e : scoreBoard) fout << e.score << std::endl;
}

void addScore(int score) 
{
    scoreBoard.push_back({ score });
    saveScoreBoard();
}

void drawScoreBoard() 
{
    settextstyle(24, 0, _T("Consolas"));
    settextcolor(RGB(80, 80, 200));
    outtextxy(WIDTH / 2 - 80, HEIGHT / 2 + 160, _T("排行榜(Top10):"));
    settextcolor(BLACK);
    for (int i = 0; i < scoreBoard.size(); i++) 
    {
        TCHAR buf[64];
        swprintf_s(buf, 64, L"%2d. %d", i + 1, scoreBoard[i].score);
        outtextxy(WIDTH / 2 - 80, HEIGHT / 2 + 200 + i * 30, buf);
    }
}

const char* getDifficultyText() 
{
    switch (currentDifficulty) 
    {
    case EASY:   return "简单";
    case NORMAL: return "普通";
    case HARD:   return "困难";
    }
    return "";
}

void drawHighScore() 
{
    TCHAR buf[64];
    settextstyle(24, 0, _T("Consolas"));
    settextcolor(RGB(200, 80, 80));
    swprintf_s(buf, 64, L"历史最高: %d", highScore);
    outtextxy(INFO_X, 320, buf);

    settextstyle(20, 0, _T("Consolas"));
    settextcolor(RGB(80, 80, 200));
    TCHAR d[32];
    toTCHAR(getDifficultyText(), d, 32);
    swprintf_s(buf, 64, L"难度: %s", d);
    outtextxy(INFO_X, 360, buf);
}

void showDifficultyMenu() 
{
    settextstyle(28, 0, _T("Consolas"));
    settextcolor(RGB(80, 80, 200));
    outtextxy(WIDTH / 2 - 80, HEIGHT / 2 + 150, _T("请选择难度:"));
    drawButton(btnEasy);
    drawButton(btnNormal); 
    drawButton(btnHard);
    while (true) 
    {
        if (MouseHit()) 
        {
            MOUSEMSG m = GetMouseMsg();
            if (m.uMsg == WM_LBUTTONDOWN) 
            {
                if (isClicked(btnEasy, m.x, m.y)) 
                { 
                    currentDifficulty = EASY; 
                    fallInterval = 1000; 
                    scoreMultiplier = 1; 
                    return; 
                }
                if (isClicked(btnNormal, m.x, m.y)) 
                { 
                    currentDifficulty = NORMAL;
                    fallInterval = 500; 
                    scoreMultiplier = 2; 
                    return; 
                }
                if (isClicked(btnHard, m.x, m.y)) 
                { 
                    currentDifficulty = HARD;  
                    fallInterval = 200; 
                    scoreMultiplier = 3; 
                    return; 
                }
            }
        }
        Sleep(10);
    }
}

int showMenu() 
{
    initgraph(WIDTH, HEIGHT);
    setbkcolor(WHITE);
    loadScoreBoard();
    ExMessage m;

    while (true) 
    {
        // 处理点击
        if (peekmessage(&m, EX_MOUSE)) 
        {
            if (m.message == WM_LBUTTONDOWN) 
            {
                if (isClicked(btnStart, m.x, m.y)) 
                    return 1;
                if (isClicked(btnExit, m.x, m.y))  
                    return 0;
                if (isClicked(btnDifficulty, m.x, m.y)) 
                {
                    showDifficultyMenu();
                    loadScoreBoard();
                }
            }
            m.message = 0;
        }

        // 使用双缓冲绘制菜单，避免闪烁
        BeginBatchDraw();
        cleardevice();

        // 标题
        settextstyle(40, 0, _T("Consolas"));
        settextcolor(BLUE);
        outtextxy(WIDTH / 2 - 100, 100, _T("俄罗斯方块"));

        // 按钮
        drawButton(btnStart);
        drawButton(btnExit);
        drawButton(btnDifficulty);

        // 当前难度显示
        TCHAR dt[32], buf[64];
        toTCHAR(getDifficultyText(), dt, 32);
        swprintf_s(buf, 64, L"(已选: %s)", dt);
        settextstyle(20, 0, _T("Consolas"));
        settextcolor(RGB(80, 80, 200));
        outtextxy(btnDifficulty.x2 + 10,
            (btnDifficulty.y1 + btnDifficulty.y2) / 2 - 10,
            buf);

        // 排行榜
        drawScoreBoard();

        EndBatchDraw();
        Sleep(20);
    }
}

bool canMove(const Block& b, int nx, int ny, int rot) 
{
    for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) 
    {
        int v = BLOCKS[b.type][rot][i][j];
        if (!v) continue;
        int x = nx + j, y = ny + i;
        if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) return false;
        if (gameMap[y][x]) return false;
    }
    return true;
}

void mergeBlock(const Block& b) 
{
    for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) 
    {
        int v = BLOCKS[b.type][b.rotation][i][j];
        if (v) gameMap[b.y + i][b.x + j] = v;
    }
}

int clearLines() 
{
    int cnt = 0;
    for (int i = MAP_H - 1; i >= 0; i--) 
    {
        bool full = true;
        for (int j = 0; j < MAP_W; j++) 
        {
            if (!gameMap[i][j]) 
            { 
                full = false; 
                break; 
            }
        }
        if (full) 
        {
            cnt++;
            for (int k = i; k > 0; k--) for (int j = 0; j < MAP_W; j++) 
                gameMap[k][j] = gameMap[k - 1][j];
            for (int j = 0; j < MAP_W; j++) 
                gameMap[0][j] = 0;
            i++;
        }
    }
    return cnt;
}

void newBlock() 
{
    curBlock = nextBlock;
    nextBlock.type = rand() % 7;
    nextBlock.rotation = 0;
    nextBlock.x = MAP_W / 2 - 2;
    nextBlock.y = 0;
}

void drawGameButtons() 
{ 
    drawButton(btnPause); 
    drawButton(btnBack); 
}

void drawGameArea() 
{
    setlinecolor(BLACK);
    rectangle(50, 50, 50 + MAP_W * 25, 50 + MAP_H * 25);
    for (int i = 1; i <= MAP_H; i++) 
        line(50, 50 + i * 25, 50 + MAP_W * 25, 50 + i * 25);
    for (int j = 1; j <= MAP_W; j++) 
        line(50 + j * 25, 50, 50 + j * 25, 50 + MAP_H * 25);

    for (int i = 0;i < MAP_H;i++) for (int j = 0;j < MAP_W;j++) 
    {
        if (gameMap[i][j]) 
        {
            setfillcolor(RGB(50 * gameMap[i][j], 100, 200));
            solidrectangle(50 + j * 25, 50 + i * 25, 75 + j * 25, 75 + i * 25);
        }
    }
    for (int i = 0;i < 4;i++) for (int j = 0;j < 4;j++) 
    {
        int v = BLOCKS[curBlock.type][curBlock.rotation][i][j];
        if (v) 
        {
            int x = curBlock.x + j, y = curBlock.y + i;
            setfillcolor(RGB(50 * v, 200, 100));
            solidrectangle(50 + x * 25, 50 + y * 25, 75 + x * 25, 75 + y * 25);
        }
    }
}

void drawNextBlock() 
{
    outtextxy(INFO_X, 30, _T("下一个:"));
    for (int i = 0;i < 4;i++) for (int j = 0;j < 4;j++) 
    {
        int v = BLOCKS[nextBlock.type][nextBlock.rotation][i][j];
        if (v) 
        {
            setfillcolor(RGB(50 * v, 200, 100));
            solidrectangle(INFO_X + 30 + j * 20, 60 + i * 20,
                INFO_X + 30 + (j + 1) * 20, 60 + (i + 1) * 20);
        }
    }
}

void runGame() 
{
    cleardevice(); 
    memset(gameMap, 0, sizeof(gameMap));
    srand((unsigned)time(nullptr));
    nextBlock.type = rand() % 7; 
    nextBlock.rotation = 0;
    nextBlock.x = MAP_W / 2 - 2; 
    nextBlock.y = 0;
    newBlock();

    DWORD lastFall = GetTickCount(), lastDraw = GetTickCount();
    ExMessage msg; 
    bool paused = false, gameOver = false;
    int score = 0;

    while (!gameOver) 
    {
        while (peekmessage(&msg, EX_MOUSE | EX_KEY)) 
        {
            if (msg.message == WM_LBUTTONDOWN) 
            {
                if (isClicked(btnPause, msg.x, msg.y)) 
                    paused = !paused;
                if (isClicked(btnBack, msg.x, msg.y)) 
                    return;
            }
            if (msg.message == WM_KEYDOWN && !paused) 
            {
                switch (msg.vkcode) 
                {
                case 'A': case 'a': case VK_LEFT:
                    if (canMove(curBlock, curBlock.x - 1, curBlock.y, curBlock.rotation)) 
                        curBlock.x--;
                    break;
                case 'D': case 'd': case VK_RIGHT:
                    if (canMove(curBlock, curBlock.x + 1, curBlock.y, curBlock.rotation)) 
                        curBlock.x++;
                    break;
                case 'S': case 's': case VK_DOWN:
                    if (canMove(curBlock, curBlock.x, curBlock.y + 1, curBlock.rotation)) 
                        curBlock.y++;
                    lastFall = GetTickCount();
                    break;
                case 'W': case 'w': case VK_UP: 
                {
                    int r = (curBlock.rotation + 1) % 4;
                    if (canMove(curBlock, curBlock.x, curBlock.y, r)) 
                        curBlock.rotation = r;
                    break;
                }
                case VK_SPACE:
                    while (canMove(curBlock, curBlock.x, curBlock.y + 1, curBlock.rotation)) 
                        curBlock.y++;
                    break;
                }
            }
            msg.message = 0;
        }
        if (!paused && GetTickCount() - lastFall > fallInterval) 
        {
            if (canMove(curBlock, curBlock.x, curBlock.y + 1, curBlock.rotation)) 
                curBlock.y++;
            else 
            {
                mergeBlock(curBlock);
                int lines = clearLines();
                score += lines * 100 * scoreMultiplier;
                newBlock();
                if (!canMove(curBlock, curBlock.x, curBlock.y, curBlock.rotation)) 
                    gameOver = true;
            }
            lastFall = GetTickCount();
        }
        if (GetTickCount() - lastDraw > 8) 
        {
            BeginBatchDraw(); 
            cleardevice();
            drawGameArea(); 
            drawGameButtons(); 
            drawNextBlock();
            TCHAR buf2[32]; 
            swprintf_s(buf2, 32, L"分数: %d", score);
            outtextxy(INFO_X, 180, buf2);
            if (paused)   
                outtextxy(INFO_X, 230, _T("已暂停"));
            if (gameOver) 
                outtextxy(INFO_X, 280, _T("游戏结束"));
            drawHighScore(); 
            EndBatchDraw();
            lastDraw = GetTickCount();
        }
        Sleep(2);
    }
    addScore(score);
    while (true) 
    {
        if (peekmessage(&msg, EX_MOUSE) && msg.message == WM_LBUTTONDOWN && isClicked(btnBack, msg.x, msg.y)) 
            return;
        Sleep(20);
    }
}

int main() 
{
    initgraph(WIDTH, HEIGHT);
    loadScoreBoard();
    while (true) 
    {
        int c = showMenu();
        if (c == 0) break;
        runGame();
    }
    closegraph();
    return 0;
}
