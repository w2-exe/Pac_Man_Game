#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <SFML/Graphics.hpp>
#include <optional>
using namespace std;

const int ROWS = 21;
const int COLS = 19;
const float TILE = 30.f;
const float HUD_HEIGHT = 50.f;
const int PLAYER_MOVE_FRAMES = 8;
const int GHOST_MOVE_FRAMES = 10;
const int BLUE_GHOST_MOVE_FRAMES = 16;
const int BLUE_FRAMES = 5 * 60;
const int GHOST_COUNT = 4;
const int START_LIVES = 3;
const string SAVE_FILE = "savegame.txt";

const int MENU = 0;
const int PLAYING = 1;
const int GAME_OVER = 2;

const int DR[5] = { 0, -1, 1, 0, 0 };
const int DC[5] = { 0, 0, 0, -1, 1 };
const int DIRECTION_ORDER[4] = { 1, 3, 2, 4 };

char maze[ROWS][COLS];
int playerRow = 0, playerCol = 0;
int playerStartRow = 0, playerStartCol = 0;
int playerDir = 0;
int wantedDir = 0;
int moveTimer = 0;
int pelletsLeft = 0;
bool mazeLoaded = false;

int score = 0;
int lives = START_LIVES;
int blueTimer = 0;

int ghostRow[GHOST_COUNT], ghostCol[GHOST_COUNT], ghostDir[GHOST_COUNT];
int ghostStartRow[GHOST_COUNT], ghostStartCol[GHOST_COUNT];
int ghostTimer[GHOST_COUNT];
bool ghostBlue[GHOST_COUNT];

int gameState = MENU;
int menuSelection = 0;
bool playerWon = false;
bool showMenuMessage = false;
string menuMessage;

sf::RectangleShape wallBlock({ TILE, TILE });
sf::CircleShape pellet(3.f);
sf::CircleShape powerPellet(8.f);
sf::CircleShape playerShape(10.f);
sf::CircleShape ghostShape(13.f);
sf::Font font;

bool isWall(int r, int c)
{
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS)
        return true;
    return maze[r][c] == '#';
}

bool isOpenCell(int r, int c)
{
    return !isWall(r, c);
}

bool loadMaze()
{
    ifstream file("maze.txt");
    if (!file.is_open())
        return false;

    int rows, cols;
    if (!(file >> rows >> cols) || rows != ROWS || cols != COLS)
        return false;

    pelletsLeft = 0;
    int ghostCount = 0;

    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            if (!(file >> maze[r][c]))
                return false;

            if (maze[r][c] == 'P')
            {
                playerStartRow = playerRow = r;
                playerStartCol = playerCol = c;
                maze[r][c] = '-';
            }
            else if (maze[r][c] == 'G')
            {
                if (ghostCount >= GHOST_COUNT)
                    return false;
                ghostStartRow[ghostCount] = ghostRow[ghostCount] = r;
                ghostStartCol[ghostCount] = ghostCol[ghostCount] = c;
                ghostDir[ghostCount] = 0;
                ghostCount++;
                maze[r][c] = '-';
            }
            else if (maze[r][c] == '.' || maze[r][c] == 'o')
            {
                pelletsLeft++;
            }
        }
    }
    return ghostCount == GHOST_COUNT;
}

void startBlueMode()
{
    blueTimer = BLUE_FRAMES;
    for (int i = 0; i < GHOST_COUNT; i++)
        ghostBlue[i] = true;
}

void eatPellet()
{
    if (maze[playerRow][playerCol] == '.')
    {
        score += 10;
        maze[playerRow][playerCol] = '-';
        pelletsLeft--;
    }
    else if (maze[playerRow][playerCol] == 'o')
    {
        score += 50;
        maze[playerRow][playerCol] = '-';
        pelletsLeft--;
        startBlueMode();
    }
}

void movePlayer()
{
    if (wantedDir != 0 && isOpenCell(playerRow + DR[wantedDir], playerCol + DC[wantedDir]))
        playerDir = wantedDir;

    if (playerDir != 0 && isOpenCell(playerRow + DR[playerDir], playerCol + DC[playerDir]))
    {
        playerRow += DR[playerDir];
        playerCol += DC[playerDir];
    }
    eatPellet();
}

void resetPositions()
{
    playerRow = playerStartRow;
    playerCol = playerStartCol;
    playerDir = 0;
    wantedDir = 0;
    moveTimer = 0;
    blueTimer = 0;
    for (int i = 0; i < GHOST_COUNT; i++)
    {
        ghostRow[i] = ghostStartRow[i];
        ghostCol[i] = ghostStartCol[i];
        ghostDir[i] = 0;
        ghostTimer[i] = 0;
        ghostBlue[i] = false;
    }
}

void clearSave()
{
    ofstream file(SAVE_FILE);
    file.close();
}

void endGame(bool won)
{
    playerWon = won;
    gameState = GAME_OVER;
    clearSave();
}

void loseLife()
{
    lives--;
    resetPositions();
    if (lives <= 0)
    {
        lives = 0;
        endGame(false);
    }
}

void checkWin()
{
    if (pelletsLeft <= 0)
        endGame(true);
}

void startNewGame()
{
    mazeLoaded = loadMaze();
    if (!mazeLoaded)
    {
        menuMessage = "Could not load maze.txt";
        showMenuMessage = true;
        return;
    }
    score = 0;
    lives = START_LIVES;
    playerWon = false;
    resetPositions();
    showMenuMessage = false;
    gameState = PLAYING;
}

void saveGame()
{
    ofstream file(SAVE_FILE);
    if (!file.is_open())
        return;

    file << score << " " << lives << "\n";
    file << playerRow << " " << playerCol << "\n";
    for (int i = 0; i < GHOST_COUNT; i++)
    {
        file << ghostRow[i] << " " << ghostCol[i];
        file << (i < GHOST_COUNT - 1 ? " " : "\n");
    }
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
            file << maze[r][c];
        file << "\n";
    }
}

bool loadSave()
{
    ifstream file(SAVE_FILE);
    if (!file.is_open())
        return false;

    int s, l, pr, pc;
    int gr[GHOST_COUNT], gc[GHOST_COUNT];
    char tmp[ROWS][COLS];

    if (!(file >> s >> l >> pr >> pc))
        return false;
    for (int i = 0; i < GHOST_COUNT; i++)
        if (!(file >> gr[i] >> gc[i]))
            return false;
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            if (!(file >> tmp[r][c]))
                return false;

    score = s;
    lives = l;
    playerRow = pr;
    playerCol = pc;
    for (int i = 0; i < GHOST_COUNT; i++)
    {
        ghostRow[i] = gr[i];
        ghostCol[i] = gc[i];
        ghostDir[i] = 0;
        ghostTimer[i] = 0;
        ghostBlue[i] = false;
    }

    pelletsLeft = 0;
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
        {
            maze[r][c] = tmp[r][c];
            if (maze[r][c] == '.' || maze[r][c] == 'o')
                pelletsLeft++;
        }

    playerDir = 0;
    wantedDir = 0;
    moveTimer = 0;
    blueTimer = 0;
    return true;
}

void continueGame()
{

    if (!loadMaze())
    {
        menuMessage = "Could not load maze.txt";
        showMenuMessage = true;
        return;
    }
    if (!loadSave())
    {
        menuMessage = "No saved game";
        showMenuMessage = true;
        return;
    }
    mazeLoaded = true;
    playerWon = false;
    showMenuMessage = false;
    gameState = PLAYING;
}

int oppositeDirection(int direction)
{
    if (direction == 1) return 2;
    if (direction == 2) return 1;
    if (direction == 3) return 4;
    if (direction == 4) return 3;
    return 0;
}

int getOpenDirections(int index, int directions[])
{
    int count = 0;
    for (int d = 1; d <= 4; d++)
    {
        if (isOpenCell(ghostRow[index] + DR[d], ghostCol[index] + DC[d]))
            directions[count++] = d;
    }
    return count;
}

int chooseGhostDirection(int index)
{
    int openDirections[4];
    int count = getOpenDirections(index, openDirections);

    if (count == 0)
        return 0;

    int reverse = oppositeDirection(ghostDir[index]);
    int choices[4];
    int choiceCount = 0;

    for (int i = 0; i < count; i++)
        if (openDirections[i] != reverse)
            choices[choiceCount++] = openDirections[i];

    if (choiceCount == 0)
    {
        choices[0] = reverse;
        choiceCount = 1;
    }

    if (index == 1 || index == 3 || ghostBlue[index])
        return choices[rand() % choiceCount];

    int bestDirection = choices[0];
    int bestDistance = 100000;

    for (int order = 0; order < 4; order++)
    {
        int d = DIRECTION_ORDER[order];
        bool allowed = false;

        for (int i = 0; i < choiceCount; i++)
            if (choices[i] == d)
                allowed = true;

        if (!allowed)
            continue;

        int nr = ghostRow[index] + DR[d];
        int nc = ghostCol[index] + DC[d];
        int distance = abs(nr - playerRow) + abs(nc - playerCol);

        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestDirection = d;
        }
    }

    return bestDirection;
}

bool checkGhostCollision(int i, int oldPlayerRow, int oldPlayerCol, int oldGhostRow, int oldGhostCol)
{
    bool sameCell = (playerRow == ghostRow[i] && playerCol == ghostCol[i]);
    bool crossed = (oldPlayerRow == ghostRow[i] && oldPlayerCol == ghostCol[i] &&
        playerRow == oldGhostRow && playerCol == oldGhostCol);

    if (!sameCell && !crossed)
        return false;

    if (ghostBlue[i])
    {
        score += 200;
        ghostRow[i] = ghostStartRow[i];
        ghostCol[i] = ghostStartCol[i];
        ghostDir[i] = 0;
        ghostTimer[i] = 0;
        ghostBlue[i] = false;
        return false;
    }

    loseLife();
    return true;
}

void moveGhosts()
{
    for (int i = 0; i < GHOST_COUNT; i++)
    {
        ghostTimer[i]++;
        int delay = ghostBlue[i] ? BLUE_GHOST_MOVE_FRAMES : GHOST_MOVE_FRAMES;
        if (ghostTimer[i] < delay)
            continue;
        ghostTimer[i] = 0;

        int direction = chooseGhostDirection(i);
        if (direction == 0)
            continue;

        int oldRow = ghostRow[i];
        int oldCol = ghostCol[i];
        ghostRow[i] += DR[direction];
        ghostCol[i] += DC[direction];
        ghostDir[i] = direction;

        if (checkGhostCollision(i, playerRow, playerCol, oldRow, oldCol))
            return;
    }
}

void handleMenuKey(sf::Keyboard::Key code, sf::RenderWindow& window)
{
    if (code == sf::Keyboard::Key::Up || code == sf::Keyboard::Key::W)
        menuSelection = (menuSelection + 2) % 3;
    else if (code == sf::Keyboard::Key::Down || code == sf::Keyboard::Key::S)
        menuSelection = (menuSelection + 1) % 3;
    else if (code == sf::Keyboard::Key::Enter)
    {
        if (menuSelection == 0)
            startNewGame();
        else if (menuSelection == 1)
            continueGame();
        else
            window.close();
    }
}

void handlePlayingKey(sf::Keyboard::Key code)
{
    if (code == sf::Keyboard::Key::Up || code == sf::Keyboard::Key::W)
        wantedDir = 1;
    else if (code == sf::Keyboard::Key::Down || code == sf::Keyboard::Key::S)
        wantedDir = 2;
    else if (code == sf::Keyboard::Key::Left || code == sf::Keyboard::Key::A)
        wantedDir = 3;
    else if (code == sf::Keyboard::Key::Right || code == sf::Keyboard::Key::D)
        wantedDir = 4;
    else if (code == sf::Keyboard::Key::Escape)
    {
        saveGame();
        menuSelection = 0;
        menuMessage = "Game saved";
        showMenuMessage = true;
        gameState = MENU;
    }
}

void handleInput(sf::RenderWindow& window)
{
    while (const optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
            window.close();
        else if (const auto* key = event->getIf<sf::Event::KeyPressed>())
        {
            switch (gameState)
            {
            case MENU:
                handleMenuKey(key->code, window);
                break;
            case PLAYING:
                handlePlayingKey(key->code);
                break;
            case GAME_OVER:
                if (key->code == sf::Keyboard::Key::Enter)
                {
                    menuSelection = 0;
                    showMenuMessage = false;
                    gameState = MENU;
                }
                break;
            }
        }
    }
}

void playerStep()
{
    int oldPlayerRow = playerRow;
    int oldPlayerCol = playerCol;

    movePlayer();

    for (int i = 0; i < GHOST_COUNT; i++)
    {

        if (checkGhostCollision(i, oldPlayerRow, oldPlayerCol, ghostRow[i], ghostCol[i]))
            return;
    }
    checkWin();
}

void updatePlaying()
{
    if (blueTimer > 0)
    {
        blueTimer--;
        if (blueTimer == 0)
            for (int i = 0; i < GHOST_COUNT; i++)
                ghostBlue[i] = false;
    }

    moveTimer++;
    if (moveTimer >= PLAYER_MOVE_FRAMES)
    {
        moveTimer = 0;
        playerStep();
    }

    if (gameState == PLAYING)
        moveGhosts();
}

void updateGame()
{
    switch (gameState)
    {
    case MENU:
    case GAME_OVER:
        break;
    case PLAYING:
        if (mazeLoaded)
            updatePlaying();
        break;
    }
}

void drawMaze(sf::RenderWindow& window)
{
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            float x = c * TILE;
            float y = r * TILE + HUD_HEIGHT;

            if (maze[r][c] == '#')
            {
                wallBlock.setPosition({ x, y });
                window.draw(wallBlock);
            }
            else if (maze[r][c] == '.')
            {
                pellet.setPosition({ x + TILE / 2 - 3.f, y + TILE / 2 - 3.f });
                window.draw(pellet);
            }
            else if (maze[r][c] == 'o')
            {
                powerPellet.setPosition({ x + TILE / 2 - 8.f, y + TILE / 2 - 8.f });
                window.draw(powerPellet);
            }
        }
    }
}

void drawPlayer(sf::RenderWindow& window)
{
    playerShape.setPosition({ playerCol * TILE + 3.f, playerRow * TILE + HUD_HEIGHT + 3.f });
    window.draw(playerShape);
}

void drawGhosts(sf::RenderWindow& window)
{
    sf::Color ghostColors[GHOST_COUNT] = {
        sf::Color(255, 70, 90),
        sf::Color(255, 150, 210),
        sf::Color(40, 220, 240),
        sf::Color(255, 170, 40)
    };

    for (int i = 0; i < GHOST_COUNT; i++)
    {
        ghostShape.setFillColor(ghostBlue[i] ? sf::Color(40, 80, 255) : ghostColors[i]);
        ghostShape.setPosition({ ghostCol[i] * TILE + 3.f, ghostRow[i] * TILE + HUD_HEIGHT + 3.f });
        window.draw(ghostShape);
    }
}

void drawCenteredText(sf::RenderWindow& window, const string& text, float y, unsigned int size)
{
    sf::Text label(font, text, size);
    sf::FloatRect b = label.getLocalBounds();
    label.setOrigin({ b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f });
    label.setPosition({ 285.f, y });
    window.draw(label);
}

void drawHUD(sf::RenderWindow& window)
{
    sf::Text scoreText(font, "SCORE " + to_string(score), 22);
    scoreText.setPosition({ 15.f, 10.f });
    window.draw(scoreText);

    sf::Text livesText(font, "LIVES", 22);
    livesText.setPosition({ 400.f, 10.f });
    window.draw(livesText);

    for (int i = 0; i < lives; i++)
    {
        sf::CircleShape dot(8.f);
        dot.setFillColor(sf::Color::Yellow);
        dot.setPosition({ 475.f + i * 25.f, 15.f });
        window.draw(dot);
    }
}

void drawMenu(sf::RenderWindow& window)
{
    drawCenteredText(window, "PAC-MAN", 170.f, 42);
    const string options[3] = { "New Game", "Continue", "Exit" };
    for (int i = 0; i < 3; i++)
        drawCenteredText(window, (i == menuSelection ? "> " : "  ") + options[i], 300.f + i * 45.f, 26);
    drawCenteredText(window, "UP/DOWN and ENTER", 465.f, 18);
    if (showMenuMessage)
        drawCenteredText(window, menuMessage, 530.f, 18);
}

void drawGameOver(sf::RenderWindow& window)
{
    drawCenteredText(window, playerWon ? "YOU WIN" : "GAME OVER", 260.f, 42);
    drawCenteredText(window, "Final score: " + to_string(score), 330.f, 25);
    drawCenteredText(window, "Press ENTER to return to menu", 410.f, 20);
}

void drawGame(sf::RenderWindow& window)
{
    window.clear(sf::Color(10, 10, 30));

    switch (gameState)
    {
    case MENU:
        drawMenu(window);
        break;
    case PLAYING:
        if (mazeLoaded)
        {
            drawMaze(window);
            drawPlayer(window);
            drawGhosts(window);
            drawHUD(window);
        }
        break;
    case GAME_OVER:
        drawGameOver(window);
        break;
    }

    window.display();
}

int main()
{
    srand(static_cast<unsigned int>(time(nullptr)));

    mazeLoaded = loadMaze();
    font.openFromFile("arial.ttf");

    wallBlock.setFillColor(sf::Color(33, 33, 222));
    pellet.setFillColor(sf::Color::White);
    powerPellet.setFillColor(sf::Color::White);
    playerShape.setFillColor(sf::Color::Green);

    sf::RenderWindow window(sf::VideoMode({ 570, 680 }), "Pac-Man");
    window.setFramerateLimit(60);

    if (!mazeLoaded)
    {
        menuMessage = "Could not load maze.txt";
        showMenuMessage = true;
    }

    while (window.isOpen())
    {
        handleInput(window);
        updateGame();
        drawGame(window);
    }

    return 0;
}
