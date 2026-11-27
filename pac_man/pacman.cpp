#include <iostream>
#include <fstream>
#include <SFML/Graphics.hpp>
#include <optional>
using namespace std;

const int ROWS = 21;
const int COLS = 19;
const float TILE = 30.f;
const float HUD_HEIGHT = 50.f;

char maze[ROWS][COLS];
int playerRow, playerCol;
int ghostStartRow[4], ghostStartCol[4];
int pelletsLeft = 0;
bool mazeLoaded = false;

sf::RectangleShape wallBlock({ TILE, TILE });
sf::CircleShape pellet(3.f);
sf::CircleShape powerPellet(8.f);
sf::Font font;

bool loadMaze()
{
    ifstream file("maze.txt");
    if (!file.is_open())
        return false;

    int rows, cols;
    file >> rows >> cols;   // skips the "21 19" line

    int ghostCount = 0;
    for (int r = 0;r < ROWS;r++)
    {
        for (int c = 0;c < COLS;c++)
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
                ghostStartRow[ghostCount] = c;
                ghostCount++;
            }
            else if (maze[r][c] == '.' || maze[r][c] == 'o')
            {
                pelletsLeft++;
            }
        }

    }

    // YOUR PART: two nested for loops, r from 0 to ROWS-1, c from 0 to COLS-1.
    // Inside: read one character with  file >> maze[r][c];
    // Then check that character:
    //   'P'      -> save r and c into playerRow, playerCol
    //   'G'      -> save r and c into ghostStartRow[ghostCount],
    //               ghostStartCol[ghostCount], then ghostCount++
    //   '.'/'o'  -> pelletsLeft++

    return true;
}

void handleInput(sf::RenderWindow& window)
{
    while (const optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
            window.close();
    }
}

void updateGame() {}

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

void drawGame(sf::RenderWindow& window)
{
    window.clear(sf::Color(10, 10, 30));

    if (mazeLoaded)
        drawMaze(window);
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