#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>

#define BOARD_SIZE_PX 94
#define SCALE_FACTOR 8.f
#define PROFUNDIDAD 2

struct Nodo
{
    int config[64];
    int nro_rojas, nro_negras;
    std::vector<Nodo*> hijos;

    Nodo(int *A, int rojas, int negras){
        nro_rojas = rojas;
        nro_negras = negras;
        for (int i = 0; i < 64; i++)
            config[i] = A[i];
    }
};

class Tree
{
public:
    Tree (int *config, int nro_rojas, int nro_negras);
    // funcion q modifica el tablero a la jugada escogida por minmax
    void calcular_jugada(int *Tablero);
private:
    Nodo *root;
    int p;  // limite de la profundidad del arbol

    // ya te las arreglas ...
    // void insert(Nodo *padre, Nodo *hijo);
};

Tree::Tree (int *config, int nro_rojas, int nro_negras)
{
    root = new Nodo(config,nro_rojas,nro_negras);
    p = PROFUNDIDAD;
}

class Game
{
public:
    Game();
    void run();

private:
    void processEvents();
    void update(sf::Time dT);
    void render();
    void handleClick(sf::Event::MouseButtonEvent evt);

    sf::RenderWindow mWindow;
    sf::Texture boardTexture;
    sf::Sprite boardSprite;
    sf::Sprite redSprites[12];
    sf::Sprite blackSprites[12];
    sf::RectangleShape opshapes[2];

    sf::Time TimePerFrame;

    int *Tablero;
    int piezaJugador,piezaIA;
    std::vector<int> opciones;  //[x1,y1,come,x2,y2,come]
    int sel_i,sel_j;
    bool selectPiece;
};

Game::Game():mWindow(sf::VideoMode(BOARD_SIZE_PX * SCALE_FACTOR, BOARD_SIZE_PX * SCALE_FACTOR), "Damas"),
boardTexture(),
boardSprite()
{
    // cargar texturas
    if (!boardTexture.loadFromFile("media/texture.png"))
        std::cout << "No se pudo cargar la imagen\n";

    // aplicar textura al sprite del tablero
    sf::IntRect TableroBounds(0,0,94,94);
    boardSprite.setTexture(boardTexture);
    boardSprite.setTextureRect(TableroBounds);
    boardSprite.setPosition(0.f, 0.f);
    boardSprite.scale(SCALE_FACTOR,SCALE_FACTOR);

    // aplicar textura a las fichas
    sf::IntRect FichaNegraBounds(94,0,11,11);
    sf::IntRect FichaRojaBounds(94,11,11,11);
    for (int i = 0; i < 12; i ++){
        blackSprites[i].setTexture(boardTexture);
        blackSprites[i].setTextureRect(FichaNegraBounds);
        blackSprites[i].scale(SCALE_FACTOR,SCALE_FACTOR);
        redSprites[i].setTexture(boardTexture);
        redSprites[i].setTextureRect(FichaRojaBounds);
        redSprites[i].scale(SCALE_FACTOR,SCALE_FACTOR);
    }

    // diseñar los sprites de seleccion
    sf::Vector2f sel_size(SCALE_FACTOR * 11, SCALE_FACTOR * 11);
    opshapes[0].setSize(sel_size);
    opshapes[1].setSize(sel_size);
    opshapes[0].setFillColor(sf::Color(252, 230, 106));
    opshapes[1].setFillColor(sf::Color(252, 230, 106));

    TimePerFrame = sf::seconds(1.f / 15.f);

    Tablero = new int[64];
    int config_ia[64] = {
        0,1,0,1,0,1,0,1,
        1,0,1,0,1,0,1,0,
        0,1,0,1,0,1,0,1,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        2,0,2,0,2,0,2,0,
        0,2,0,2,0,2,0,2,
        2,0,2,0,2,0,2,0
    };
    int config_human[64] = {
        0,2,0,2,0,2,0,2,
        2,0,2,0,2,0,2,0,
        0,2,0,2,0,2,0,2,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        1,0,1,0,1,0,1,0,
        0,1,0,1,0,1,0,1,
        1,0,1,0,1,0,1,0
    };
    for (int i = 0; i < 64; i++)
        Tablero[i] = config_human[i];
    piezaJugador = 1;
    piezaIA = 2;
    selectPiece = false;
}

// bucle principal
void Game::run()
{
    render();
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    while (mWindow.isOpen())
    {
        processEvents();
        timeSinceLastUpdate += clock.restart();
        while (timeSinceLastUpdate > TimePerFrame)
        {
            timeSinceLastUpdate -= TimePerFrame;
            processEvents();
            update(TimePerFrame);
        }
    }
}

void Game::handleClick(sf::Event::MouseButtonEvent evt)
{
    // if hizo click en una de las fichas del jugador:
    if (selectPiece == false){
        for (int i = 1; i < 8; i++) // i = 1 porq las fichas de arriba ya no se mueven
        for (int j = 0; j < 8; j++)
        if (Tablero[i*8+j] == piezaJugador){
            bool in_x = evt.x > SCALE_FACTOR * (3+j*11) && evt.x < SCALE_FACTOR * (3+(j+1)*11);
            bool in_y = evt.y > SCALE_FACTOR * (3+i*11) && evt.y < SCALE_FACTOR * (3+(i+1)*11);
            // si el jugador hizo click en una de sus piezas:
            if (in_x && in_y){
                sel_i = i;
                sel_j = j;
                // mov izq
                if (j > 0 && Tablero[(i-1)*8+j-1] == 0){
                    opciones.push_back(i-1);
                    opciones.push_back(j-1);
                    opciones.push_back(0);
                }
                // mov der
                if (j < 7 && Tablero[(i-1)*8+j+1] == 0){
                    opciones.push_back(i-1);
                    opciones.push_back(j+1);
                    opciones.push_back(0);
                }
                // comer izq
                if (j > 1 && i > 1 && Tablero[(i-2)*8+j-2] == 0 && Tablero[(i-1)*8+j-1] == piezaIA){
                    opciones.push_back(i-2);
                    opciones.push_back(j-2);
                    opciones.push_back(1);
                }
                // comer der
                if (j < 6 && i > 1 && Tablero[(i-2)*8+j+2] == 0 && Tablero[(i-1)*8+j+1] == piezaIA){
                    opciones.push_back(i-2);
                    opciones.push_back(j+2);
                    opciones.push_back(1);
                }
                // si hay movimientos disponibles se muestran las opciones
                if (!opciones.empty())
                    selectPiece = true;
                break;
            }
        }
    } 
    else 
    {
        for (int i = 0; i < opciones.size(); i+=3){
            bool in_x = evt.x > SCALE_FACTOR * (3+opciones[i+1]*11) && evt.x < SCALE_FACTOR * (3+(opciones[i+1]+1)*11);
            bool in_y = evt.y > SCALE_FACTOR * (3+opciones[i]*11) && evt.y < SCALE_FACTOR * (3+(opciones[i]+1)*11);
            if (in_x && in_y){
                // cambiar tablero
                Tablero[8*opciones[i]+opciones[i+1]] = piezaJugador;
                Tablero[8*sel_i+sel_j] = 0;
                if (opciones[i+2]){  // si comio a una pieza
                    // calcular la pos del enemigo
                    if (sel_j < opciones[i+1])  // estaba a la der
                        Tablero[8*(sel_i-1)+sel_j+1] = 0;
                    else                        // estaba a la izq
                        Tablero[8*(sel_i-1)+sel_j-1] = 0;
                }
                selectPiece = false;
                opciones.clear();

                // Hacer jugada IA
                int rojas = 0, negras = 0;
                for (int i = 0; i < 64; i++){
                    if (Tablero[i] == 1) rojas++;
                    else if (Tablero[i] == 2) negras++;
                }

                Tree IATree(Tablero, rojas, negras);
                IATree.calcular_jugada(Tablero);
            }
        }
    }
    
    render();
}

void Game::processEvents()
{
    sf::Event event;
    while (mWindow.pollEvent(event))
    {
        switch (event.type)
        {
        case sf::Event::MouseButtonPressed:
            handleClick(event.mouseButton);
            break;
        case sf::Event::Closed:
            mWindow.close();
            break;
        }
    }
}

// dT = tiempo entre frames
void Game::update(sf::Time dT)
{
    
}

void Game::render()
{
    mWindow.clear();

    mWindow.draw(boardSprite);

    // dibujar las piezas en el tablero
    int p_r = 0;
    int p_n = 0;
    float pos_x,pos_y;
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
        {
            if (Tablero[i*8+j] == 1)
            {
                pos_x = SCALE_FACTOR * (3 + j * 11);
                pos_y = SCALE_FACTOR * (3 + i * 11);
                redSprites[p_r].setPosition(pos_x,pos_y);
                mWindow.draw(redSprites[p_r]);
                p_r++;
            }
            else if (Tablero[i*8+j] == 2)
            {
                pos_x = SCALE_FACTOR * (3 + j * 11);
                pos_y = SCALE_FACTOR * (3 + i * 11);
                blackSprites[p_n].setPosition(pos_x,pos_y);
                mWindow.draw(blackSprites[p_n]);
                p_n++;
            }
        }
    // dibujar los movimientos disponibles tras la seleccion de una ficha
    for (int i = 0; i < opciones.size(); i+=3){
        pos_x = SCALE_FACTOR * (3 + opciones[i+1] * 11);
        pos_y = SCALE_FACTOR * (3 + opciones[i] * 11);
        opshapes[i/3].setPosition(pos_x,pos_y);
        mWindow.draw(opshapes[i/3]);
    }

    mWindow.display();
}

int main()
{
    Game game;
    game.run();
    
    return 0;
}