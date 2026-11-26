#include <iostream>
#include <SFML/Graphics.hpp>
#include<optional>
using namespace std;
void handleInput(sf::RenderWindow& window) 
{
    while (const optional event = window.pollEvent()) //check a queue of user action one by one
    {
        if (event->is<sf::Event::Closed>()) //check if user clicked X then close the window
        {
            window.close();
        }
    }
}
void updateGame() {

}
void drawGame(sf::RenderWindow& window) {
    window.clear();
    window.display();
}
int main()
{
    sf::RenderWindow window(sf::VideoMode({ 570, 680 }), "Pac-Man");
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        handleInput(window);
        updateGame();
        drawGame(window);
    }
    return 0;
}