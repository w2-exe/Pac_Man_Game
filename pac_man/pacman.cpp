#include <iostream>
#include <fstream>
#include <string>
#include <SFML/Graphics.hpp>
#include <optional>
using namespace std;

const int ROWS = 21;
const int COLS = 19;
const float TILE = 30.f;
const float HUD_HEIGHT = 50.f;
const int PLAYER_MOVE_FRAMES = 8;
const int BLUE_FRAMES = 300;   // 5 seconds at 60 FPS

char maze[ROWS][COLS];
int playerRow, playerCol;
int ghostStartRow[4], ghostStartCol[4];
int pelletsLeft = 0;
bool mazeLoaded = false;

int score = 0;
int blueTimer = 0;           // counts down while ghosts are blue (Feature 5)

int playerDir = 0;   // 0 none, 1 up, 2 down, 3 left, 4 right
int wantedDir = 0;   // last key the player pressed
int moveTimer = 0;

sf::RectangleShape wallBlock({ TILE, TILE });
sf::CircleShape pellet(3.f);
sf::CircleShape powerPellet(8.f);
sf::CircleShape player(12.f);
sf::Font font;

// Reads maze.txt into the maze array and saves P, G and pellet info
bool loadMaze()
{
    ifstream file("maze.txt");
    if (!file.is_open())
        return false;

    int rows, cols;
    file >> rows >> cols;

    int ghostCount = 0;
    for (int r = 0; r < ROWS; r++)
    {
        for (int c = 0; c < COLS; c++)
        {
            file >> maze[r][c];

            if (maze[r][c] == 'P')
            {
                playerRow = r;
                playerCol = c;
            }
            else if (maze[r][c] == 'G')
            {
                ghostStartRow[ghostCount] = r;
                ghostStartCol[ghostCount] = c;
                ghostCount++;
            }
            else if (maze[r][c] == '.' || maze[r][c] == 'o')
            {
                pelletsLeft++;
            }
        }
    }
    return true;
}

// true if the cell is outside the maze or is a wall
bool isWall(int r, int c)
{
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS)
        return true;
    return maze[r][c] == '#';
}

void handleInput(sf::RenderWindow& window)
{
    while (const optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
            window.close();
        else if (const auto* key = event->getIf<sf::Event::KeyPressed>())
        {
            if (key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::W)
                wantedDir = 1;
            else if (key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::S)
                wantedDir = 2;
            else if (key->code == sf::Keyboard::Key::Left || key->code == sf::Keyboard::Key::A)
                wantedDir = 3;
            else if (key->code == sf::Keyboard::Key::Right || key->code == sf::Keyboard::Key::D)
                wantedDir = 4;
        }
    }
}

void updateGame()
{
    if (!mazeLoaded)
        return;

    moveTimer++;
    if (moveTimer < PLAYER_MOVE_FRAMES)
        return;              // not time to move yet
    moveTimer = 0;

    // Part 2: turn if the wanted direction is free
    int nextRow = playerRow;
    int nextCol = playerCol;
    if (wantedDir == 1)
        nextRow = playerRow - 1;
    else if (wantedDir == 2)
        nextRow = playerRow + 1;
    else if (wantedDir == 3)
        nextCol = playerCol - 1;
    else if (wantedDir == 4)
        nextCol = playerCol + 1;
    if (!isWall(nextRow, nextCol))
        playerDir = wantedDir;

    // Part 3: move one cell in playerDir if it is free
    nextRow = playerRow;
    nextCol = playerCol;
    if (playerDir == 1)
        nextRow = playerRow - 1;
    else if (playerDir == 2)
        nextRow = playerRow + 1;
    else if (playerDir == 3)
        nextCol = playerCol - 1;
    else if (playerDir == 4)
        nextCol = playerCol + 1;
    if (!isWall(nextRow, nextCol))
    {
        playerRow = nextRow;
        playerCol = nextCol;
    }

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
        blueTimer = BLUE_FRAMES;
    }

    // YOUR PART 4: eat pellets on the cell the player stands on.
    //   '.'  -> score += 10, set the cell to '-', pelletsLeft--
    //   'o'  -> score += 50, set the cell to '-', pelletsLeft--,
    //           blueTimer = BLUE_FRAMES
    //   Use else if so one cell is only one kind.
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

// Score text in the 50 px strip at the top
void drawHUD(sf::RenderWindow& window)
{
    sf::Text scoreText(font, "SCORE " + to_string(score), 22);
    scoreText.setPosition({ 15.f, 10.f });
    window.draw(scoreText);
}

void drawGame(sf::RenderWindow& window)
{
    window.clear(sf::Color(10, 10, 30));

    if (mazeLoaded)
    {
        drawMaze(window);
        player.setPosition({ playerCol * TILE + 3.f, playerRow * TILE + HUD_HEIGHT + 3.f });
        window.draw(player);
        drawHUD(window);
    }
    else
    {
        sf::Text errorText(font, "maze.txt not found", 24);
        errorText.setPosition({ 120.f, 300.f });
        window.draw(errorText);
    }

    window.display();
}

int main()
{
    mazeLoaded = loadMaze();
    font.openFromFile("arial.ttf");

    wallBlock.setFillColor(sf::Color(33, 33, 222));
    pellet.setFillColor(sf::Color::White);
    powerPellet.setFillColor(sf::Color::White);
    player.setFillColor(sf::Color::Yellow);

    sf::RenderWindow window(sf::VideoMode({ 570, 680 }), "Pac-Man");
    window.setFramerateLimit(60);

    while (window.isOpen())
    {
        handleInput(window);
        updateGame();
        drawGame(window);
    }
    return 0;
}