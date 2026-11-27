#include <iostream>
#include <fstream>
#include <SFML/Graphics.hpp>
#include <optional>
#include <cstdlib>
#include <ctime>
#include <string>

using namespace std;

const int ROWS = 21;
const int COLS = 19;
const float TILE = 30.f;
const float HUD_HEIGHT = 50.f;

const int GHOST_COUNT = 4;
const int START_LIVES = 3;
const int PLAYER_MOVE_FRAMES = 20;
const int GHOST_MOVE_FRAMES = 20;
const int BLUE_GHOST_MOVE_FRAMES = 20;
const int BLUE_TIME_FRAMES = 5 * 60; // 5 seconds at 60 FPS

const int MENU = 0;
const int PLAYING = 1;
const int GAME_OVER = 2;

char maze[ROWS][COLS];
char originalMaze[ROWS][COLS];

int playerRow = 0, playerCol = 0;
int playerStartRow = 0, playerStartCol = 0;
int playerDir = 0; // 0=still, 1=up, 2=down, 3=left, 4=right

int ghostRow[GHOST_COUNT], ghostCol[GHOST_COUNT];
int ghostStartRow[GHOST_COUNT], ghostStartCol[GHOST_COUNT];
int ghostDir[GHOST_COUNT];

int pelletsLeft = 0;
int score = 0;
int lives = START_LIVES;
int gameState = MENU;
int menuSelection = 0;
int playerFrameCounter = 0;
int ghostFrameCounter = 0;
int blueFramesLeft = 0;
bool mazeLoaded = false;
bool playerWon = false;
bool showMenuMessage = false;
string menuMessage;

sf::RectangleShape wallBlock({ TILE, TILE });
sf::CircleShape pellet(3.f);
sf::CircleShape powerPellet(8.f);
sf::CircleShape playerShape(10.f);
sf::CircleShape ghostShape(13.f);
sf::Font font;

const int DR[5] = { 0, -1, 1, 0, 0 };
const int DC[5] = { 0, 0, 0, -1, 1 };
const int DIRECTION_ORDER[4] = { 1, 3, 2, 4 }; // up, left, down, right

bool insideMaze(int r, int c)
{
    return r >= 0 && r < ROWS && c >= 0 && c < COLS;
}

bool isOpenCell(int r, int c)
{
    return insideMaze(r, c) && maze[r][c] != '#';
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

                ghostStartRow[ghostCount] = r;
                ghostStartCol[ghostCount] = c;
                ghostRow[ghostCount] = r;
                ghostCol[ghostCount] = c;
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

    if (ghostCount != GHOST_COUNT)
        return false;

    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            originalMaze[r][c] = maze[r][c];

    return true;
}

void resetPositions()
{
    playerRow = playerStartRow;
    playerCol = playerStartCol;
    playerDir = 0;

    for (int i = 0; i < GHOST_COUNT; i++)
    {
        ghostRow[i] = ghostStartRow[i];
        ghostCol[i] = ghostStartCol[i];
        ghostDir[i] = 0;
    }

    playerFrameCounter = 0;
    ghostFrameCounter = 0;
    blueFramesLeft = 0;
}

void startNewGame()
{
    mazeLoaded = loadMaze();
    if (!mazeLoaded)
    {
        menuMessage = "Could not load maze.txt";
        showMenuMessage = true;
        gameState = MENU;
        return;
    }

    score = 0;
    lives = START_LIVES;
    playerWon = false;
    resetPositions();
    gameState = PLAYING;
    showMenuMessage = false;
}

void requestPlayerDirection(int direction)
{
    int nextRow = playerRow + DR[direction];
    int nextCol = playerCol + DC[direction];

    // A turn into a wall is ignored; the current direction remains unchanged.
    if (isOpenCell(nextRow, nextCol))
        playerDir = direction;
}

void handleInput(sf::RenderWindow& window)
{
    while (const optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window.close();
        }
        else if (const auto* key = event->getIf<sf::Event::KeyPressed>())
        {
            if (gameState == MENU)
            {
                if (key->code == sf::Keyboard::Key::Up ||
                    key->code == sf::Keyboard::Key::W)
                {
                    menuSelection = (menuSelection + 2) % 3;
                }
                else if (key->code == sf::Keyboard::Key::Down ||
                    key->code == sf::Keyboard::Key::S)
                {
                    menuSelection = (menuSelection + 1) % 3;
                }
                else if (key->code == sf::Keyboard::Key::Enter)
                {
                    if (menuSelection == 0)
                        startNewGame();
                    else if (menuSelection == 1)
                    {
                        // Save/Continue is Feature 8. Until it is added, show a message.
                        menuMessage = "Continue is not implemented yet";
                        showMenuMessage = true;
                    }
                    else
                        window.close();
                }
            }
            else if (gameState == PLAYING)
            {
                if (key->code == sf::Keyboard::Key::Up ||
                    key->code == sf::Keyboard::Key::W)
                    requestPlayerDirection(1);
                else if (key->code == sf::Keyboard::Key::Down ||
                    key->code == sf::Keyboard::Key::S)
                    requestPlayerDirection(2);
                else if (key->code == sf::Keyboard::Key::Left ||
                    key->code == sf::Keyboard::Key::A)
                    requestPlayerDirection(3);
                else if (key->code == sf::Keyboard::Key::Right ||
                    key->code == sf::Keyboard::Key::D)
                    requestPlayerDirection(4);
                else if (key->code == sf::Keyboard::Key::Escape)
                {
                    gameState = MENU;
                    menuSelection = 0;
                    menuMessage = "Game paused - choose New Game or Exit";
                    showMenuMessage = true;
                }
            }
            else if (gameState == GAME_OVER &&
                key->code == sf::Keyboard::Key::Enter)
            {
                gameState = MENU;
                menuSelection = 0;
                showMenuMessage = false;
            }
        }
    }
}

void eatPelletAtPlayer()
{
    char cell = maze[playerRow][playerCol];

    if (cell == '.')
    {
        score += 10;
        pelletsLeft--;
        maze[playerRow][playerCol] = '-';
    }
    else if (cell == 'o')
    {
        score += 50;
        pelletsLeft--;
        maze[playerRow][playerCol] = '-';
        blueFramesLeft = BLUE_TIME_FRAMES;
    }

    if (pelletsLeft <= 0)
    {
        pelletsLeft = 0;
        playerWon = true;
        gameState = GAME_OVER;
    }
}

void movePlayer()
{
    if (playerDir == 0)
        return;

    int newRow = playerRow + DR[playerDir];
    int newCol = playerCol + DC[playerDir];

    // Ignore a direction that would move into a wall.
    if (isOpenCell(newRow, newCol))
    {
        playerRow = newRow;
        playerCol = newCol;
        eatPelletAtPlayer();
    }
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
        int nr = ghostRow[index] + DR[d];
        int nc = ghostCol[index] + DC[d];

        if (isOpenCell(nr, nc))
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

    // Do not reverse unless it is the only available direction.
    for (int i = 0; i < count; i++)
        if (openDirections[i] != reverse)
            choices[choiceCount++] = openDirections[i];

    if (choiceCount == 0)
    {
        choices[0] = reverse;
        choiceCount = 1;
    }

    // Wanderers and frightened ghosts choose a random legal direction.
    if (index == 1 || index == 3 || blueFramesLeft > 0)
        return choices[rand() % choiceCount];

    // Chasers 0 and 2 choose the nearest open cell by Manhattan distance.
    // Iterate in the required tie order: up, left, down, right.
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

void resetAfterCaught()
{
    lives--;
    resetPositions();

    if (lives <= 0)
    {
        lives = 0;
        playerWon = false;
        gameState = GAME_OVER;
    }
}

void checkGhostCollision(int oldPlayerRow, int oldPlayerCol,
    int oldGhostRows[], int oldGhostCols[])
{
    for (int i = 0; i < GHOST_COUNT; i++)
    {
        bool sameCell = (playerRow == ghostRow[i] && playerCol == ghostCol[i]);
        bool crossed = (oldPlayerRow == ghostRow[i] &&
            oldPlayerCol == ghostCol[i] &&
            playerRow == oldGhostRows[i] &&
            playerCol == oldGhostCols[i]);

        if (sameCell || crossed)
        {
            if (blueFramesLeft > 0)
            {
                score += 200;
                ghostRow[i] = ghostStartRow[i];
                ghostCol[i] = ghostStartCol[i];
                ghostDir[i] = 0;
            }
            else
            {
                resetAfterCaught();
                return;
            }
        }
    }
}

void moveGhosts()
{
    int oldPlayerRow = playerRow;
    int oldPlayerCol = playerCol;
    int oldGhostRows[GHOST_COUNT];
    int oldGhostCols[GHOST_COUNT];

    for (int i = 0; i < GHOST_COUNT; i++)
    {
        oldGhostRows[i] = ghostRow[i];
        oldGhostCols[i] = ghostCol[i];
    }

    for (int i = 0; i < GHOST_COUNT; i++)
    {
        int direction = chooseGhostDirection(i);
        if (direction == 0)
            continue;

        int nr = ghostRow[i] + DR[direction];
        int nc = ghostCol[i] + DC[direction];

        if (isOpenCell(nr, nc))
        {
            ghostRow[i] = nr;
            ghostCol[i] = nc;
            ghostDir[i] = direction;
        }
    }

    checkGhostCollision(oldPlayerRow, oldPlayerCol, oldGhostRows, oldGhostCols);
}

void updateGame()
{
    if (gameState != PLAYING)
        return;

    if (blueFramesLeft > 0)
        blueFramesLeft--;

    playerFrameCounter++;
    if (playerFrameCounter >= PLAYER_MOVE_FRAMES)
    {
        playerFrameCounter = 0;

        int oldPlayerRow = playerRow;
        int oldPlayerCol = playerCol;
        int oldGhostRows[GHOST_COUNT];
        int oldGhostCols[GHOST_COUNT];

        for (int i = 0; i < GHOST_COUNT; i++)
        {
            oldGhostRows[i] = ghostRow[i];
            oldGhostCols[i] = ghostCol[i];
        }

        movePlayer();

        // Check player moving into a ghost before the next ghost move.
        if (gameState == PLAYING)
            checkGhostCollision(oldPlayerRow, oldPlayerCol, oldGhostRows, oldGhostCols);
    }

    if (gameState != PLAYING)
        return;

    ghostFrameCounter++;
    int ghostDelay = (blueFramesLeft > 0) ?
        BLUE_GHOST_MOVE_FRAMES : GHOST_MOVE_FRAMES;

    if (ghostFrameCounter >= ghostDelay)
    {
        ghostFrameCounter = 0;
        moveGhosts();
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

void drawCenteredText(sf::RenderWindow& window, const string& text,
    float y, unsigned int size)
{
    sf::Text label(font, text, size);
    sf::FloatRect bounds = label.getLocalBounds();
    label.setOrigin({ bounds.position.x + bounds.size.x / 2.f,
                      bounds.position.y + bounds.size.y / 2.f });
    label.setPosition({ 285.f, y });
    label.setFillColor(sf::Color::White);
    window.draw(label);
}

void drawHUD(sf::RenderWindow& window)
{
    sf::Text scoreText(font, "SCORE: " + to_string(score), 20);
    scoreText.setPosition({ 15.f, 12.f });
    scoreText.setFillColor(sf::Color::White);
    window.draw(scoreText);

    sf::Text livesText(font, "LIVES:", 20);
    livesText.setPosition({ 390.f, 12.f });
    livesText.setFillColor(sf::Color::White);
    window.draw(livesText);

    for (int i = 0; i < lives; i++)
    {
        sf::CircleShape lifeDot(7.f);
        lifeDot.setFillColor(sf::Color(255, 220, 40));
        lifeDot.setPosition({ 465.f + i * 22.f, 17.f });
        window.draw(lifeDot);
    }
}

void drawPlayer(sf::RenderWindow& window)
{
    playerShape.setFillColor(sf::Color(0, 255, 40));
    playerShape.setPosition({
        playerCol * TILE + TILE / 2.f - 12.f,
        playerRow * TILE + HUD_HEIGHT + TILE / 2.f - 12.f
        });
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
        ghostShape.setFillColor(blueFramesLeft > 0 ?
            sf::Color(40, 80, 255) : ghostColors[i]);

        ghostShape.setPosition({
            ghostCol[i] * TILE + TILE / 2.f - 12.f,
            ghostRow[i] * TILE + HUD_HEIGHT + TILE / 2.f - 12.f
            });
        window.draw(ghostShape);
    }
}

void drawMenu(sf::RenderWindow& window)
{
    drawCenteredText(window, "PAC-MAN", 170.f, 42);

    const string options[3] = { "New Game", "Continue", "Exit" };
    for (int i = 0; i < 3; i++)
    {
        string option = (i == menuSelection ? "> " : "  ") + options[i];
        drawCenteredText(window, option, 300.f + i * 45.f, 26);
    }

    drawCenteredText(window, "Use UP/DOWN and ENTER", 465.f, 18);
    if (showMenuMessage)
        drawCenteredText(window, menuMessage, 530.f, 17);
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
            drawHUD(window);
            drawMaze(window);
            drawPlayer(window);
            drawGhosts(window);
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
