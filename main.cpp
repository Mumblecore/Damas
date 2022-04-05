#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>

#define BOARD_SIZE_PX 94
#define SCALE_FACTOR 8.f

int piezaJugador,piezaIA;

struct Nodo
{
    int config[64];
    int nro_rojas, nro_negras;
    int value;
    std::vector<Nodo*> hijos;

    Nodo(int *A, int rojas, int negras){
        nro_rojas = rojas;
        nro_negras = negras;
        value = (piezaIA == 1)?(rojas - negras):(negras - rojas);
        for (int i = 0; i < 64; i++)
            config[i] = A[i];
    }

    void print(){
        for (int i = 0; i < 8; i++){
            for (int j = 0; j < 8; j++)
                std::cout << config[i*8+j] << " ";
            std::cout << "\n";
        }
    }
};

struct Tree
{
    Tree(int *config, int nro_rojas, int nro_negras, int profundidad);
    ~Tree();
    // funcion q modifica el tablero a la jugada escogida por minmax
    bool calcular_jugada(int *Tablero);
    Nodo *root;
    int p;  // limite de la profundidad del arbol

    void insertHijos(Nodo *nodo, int nivel);
    int min_max(Nodo *nodo, int nivel, int &mejor);
    void delNodo(Nodo *nodo);
};

Tree::Tree (int *config, int nro_rojas, int nro_negras, int profundidad)
{
    root = new Nodo(config,nro_rojas,nro_negras);
    p = profundidad;
}

void Tree::delNodo(Nodo *nodo)
{
    if (nodo->hijos.size() == 0){
        delete nodo;
    }else{
        for (int i = 0; i < nodo->hijos.size(); i++){
            delNodo(nodo->hijos[i]);
        }
    }
}

Tree::~Tree()
{
    delNodo(root);
}

int Tree::min_max(Nodo *nodo, int nivel, int &mejor)
{
    // std::cout << "MINMAX_call\n";
    // nodo->print();
    if (nivel < p)
        insertHijos(nodo,nivel);
    // std::cout << "Hijos: " << nodo->hijos.size() << "\n";
    if (nodo->hijos.size() == 0){
        return nodo->value;
    }
    else{
        int min = 10000;
        int max = -10000;
        if (nivel & 1){ //min
            for (int i = 0; i < nodo->hijos.size(); i++){
                int val = min_max(nodo->hijos[i],nivel+1,mejor);
                if (val < min) min = val;
            }
            return min;
        }else{          //max
            for (int i = 0; i < nodo->hijos.size(); i++){
                int val = min_max(nodo->hijos[i],nivel+1,mejor);
                if (val > max){max = val; mejor = i;}
            }
            return max;
        }
    }
}

bool Tree::calcular_jugada(int *Tablero)
{
    // hacer uso del algoritmo min_max para encontrar el mejor camino
    // aqui tmb se genera todo el arbol
    int mejor;
    min_max(root,0,mejor);

    if (root->hijos.size() == 0)
        return false;
    // elaborar la jugada del mejor camino
    for (int j = 0; j < 64; j++)
        Tablero[j] = root->hijos[mejor]->config[j];
    return true;
}

void Tree::insertHijos(Nodo *nodo, int nivel)
{
    int p[64];  // puntero auxiliar que modificaremos en lugar de config
    for (int k = 0; k < 64; k++)
        p[k] = nodo->config[k];
    if (nivel & 1){     // turno del humano
        for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
        if (nodo->config[i*8+j] == piezaJugador){
            // mov izq
            if (j > 0 && i > 0 && nodo->config[(i-1)*8+j-1] == 0){
                p[(i-1)*8+j-1] = piezaJugador;
                p[i*8+j] = 0;
                Nodo *n = new Nodo(p,nodo->nro_rojas,nodo->nro_negras);
                nodo->hijos.push_back(n);
                // revierto cambios en p
                p[(i-1)*8+j-1] = 0;
                p[i*8+j] = piezaJugador;
                // std::cout << i << "," << j << "human:mov izq\n";
            }
            // mov der
            if (j < 7 && i > 0 && nodo->config[(i-1)*8+j+1] == 0){
                p[(i-1)*8+j+1] = piezaJugador;
                p[i*8+j] = 0;
                Nodo *n = new Nodo(p,nodo->nro_rojas,nodo->nro_negras);
                nodo->hijos.push_back(n);
                // revierto cambios en p
                p[(i-1)*8+j+1] = 0;
                p[i*8+j] = piezaJugador;
                // std::cout << i << "," << j << "human:mov der\n";
            }
            // comer izq
            if (j > 1 && i > 1 && nodo->config[(i-2)*8+j-2] == 0 && nodo->config[(i-1)*8+j-1] == piezaIA){
                p[(i-2)*8+j-2] = piezaJugador;
                p[(i-1)*8+j-1] = 0;
                p[i*8+j] = 0;
                Nodo *n = (piezaIA == 1) ? new Nodo(p,nodo->nro_rojas-1,nodo->nro_negras) : new Nodo(p,nodo->nro_rojas,nodo->nro_negras-1);
                nodo->hijos.push_back(n);
                // revierto cambios en p
                p[(i-2)*8+j-2] = 0;
                p[(i-1)*8+j-1] = piezaIA;
                p[i*8+j] = piezaJugador;
                // std::cout << i << "," << j << "human:comer izq\n";
            }
            // comer der
            if (j < 6 && i > 1 && nodo->config[(i-2)*8+j+2] == 0 && nodo->config[(i-1)*8+j+1] == piezaIA){
                p[(i-2)*8+j+2] = piezaIA;
                p[(i-1)*8+j+1] = 0;
                p[i*8+j] = 0;
                Nodo *n = (piezaIA == 1) ? new Nodo(p,nodo->nro_rojas-1,nodo->nro_negras) : new Nodo(p,nodo->nro_rojas,nodo->nro_negras-1);
                nodo->hijos.push_back(n);
                // revierto cambios en p
                p[(i-2)*8+j+2] = 0;
                p[(i-1)*8+j+1] = piezaJugador;
                p[i*8+j] = piezaIA;
                // std::cout << i << "," << j << "human:comer der\n";
            }
        }
    }else{              // turno de la IA
        for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
        if (nodo->config[i*8+j] == piezaIA){
            // mov izq
            if (j > 0 && i < 7 && nodo->config[(i+1)*8+j-1] == 0){
                p[(i+1)*8+j-1] = piezaIA;
                p[i*8+j] = 0;
                Nodo *n = new Nodo(p,nodo->nro_rojas,nodo->nro_negras);
                nodo->hijos.push_back(n);
                // revierto cambios en p
                p[(i+1)*8+j-1] = 0;
                p[i*8+j] = piezaIA;
                // std::cout << i << "," << j << "ia:mov izq\n";
            }
            // mov der
            if (j < 7 && i < 7 && nodo->config[(i+1)*8+j+1] == 0){
                p[(i+1)*8+j+1] = piezaIA;
                p[i*8+j] = 0;
                Nodo *n = new Nodo(p,nodo->nro_rojas,nodo->nro_negras);
                nodo->hijos.push_back(n);
                // revierto cambios en p
                p[(i+1)*8+j+1] = 0;
                p[i*8+j] = piezaIA;
                // std::cout << i << "," << j << "ia:mov der\n";
            }
            // comer izq
            if (j > 1 && i < 6 && nodo->config[(i+2)*8+j-2] == 0 && nodo->config[(i+1)*8+j-1] == piezaJugador){
                p[(i+2)*8+j-2] = piezaIA;
                p[(i+1)*8+j-1] = 0;
                p[i*8+j] = 0;
                Nodo *n = (piezaIA == 1) ? new Nodo(p,nodo->nro_rojas,nodo->nro_negras-1) : new Nodo(p,nodo->nro_rojas-1,nodo->nro_negras);
                nodo->hijos.push_back(n);
                // revierto cambios en p
                p[(i+2)*8+j-2] = 0;
                p[(i+1)*8+j-1] = piezaJugador;
                p[i*8+j] = piezaIA;
                // std::cout << i << "," << j << " ia:comer izq\n";
            }
            // comer der
            if (j < 6 && i < 6 && nodo->config[(i+2)*8+j+2] == 0 && nodo->config[(i+1)*8+j+1] == piezaJugador){
                p[(i+2)*8+j+2] = piezaIA;
                p[(i+1)*8+j+1] = 0;
                p[i*8+j] = 0;
                Nodo *n = (piezaIA == 1) ? new Nodo(p,nodo->nro_rojas,nodo->nro_negras-1) : new Nodo(p,nodo->nro_rojas-1,nodo->nro_negras);
                nodo->hijos.push_back(n);
                // revierto cambios en p
                p[(i+2)*8+j+2] = 0;
                p[(i+1)*8+j+1] = piezaJugador;
                p[i*8+j] = piezaIA;
                // std::cout << i << "," << j << "ia:comer der\n";
            }
        }
    }
}

class Game
{
public:
    Game(int _iaProfundidad, bool iniciaHumano);
    void run();

private:
    void processEvents();
    void update(sf::Time dT);
    void render();
    bool handleClick(sf::Event::MouseButtonEvent evt);

    sf::RenderWindow mWindow;
    sf::Texture boardTexture;
    sf::Sprite boardSprite;
    sf::Sprite redSprites[12];
    sf::Sprite blackSprites[12];
    sf::RectangleShape opshapes[2];

    sf::Time TimePerFrame;

    int *Tablero;
    std::vector<int> opciones;  //[x1,y1,come,x2,y2,come]
    int sel_i,sel_j;
    bool selectPiece;
    int IAProfundidad;
};

Game::Game(int _iaProfundidad, bool iniciaHumano):mWindow(sf::VideoMode(BOARD_SIZE_PX * SCALE_FACTOR, BOARD_SIZE_PX * SCALE_FACTOR), "Damas"),
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

    selectPiece = false;
    IAProfundidad = _iaProfundidad;
    // quien empieza?
    if (iniciaHumano){
        for (int i = 0; i < 64; i++)
            Tablero[i] = config_human[i];
        piezaJugador = 1;
        piezaIA = 2;
    }
    else{
        for (int i = 0; i < 64; i++)
            Tablero[i] = config_ia[i];
        piezaJugador = 2;
        piezaIA = 1;
        Tree IATree(Tablero, 12, 12, IAProfundidad);
        IATree.calcular_jugada(Tablero);
    }
}

// bucle principal
void Game::run()
{
    render();
    while (mWindow.isOpen())
    {
        processEvents();
    }
}

void Game::processEvents()
{
    sf::Event event;
    while (mWindow.pollEvent(event))
    {
        switch (event.type)
        {
        case sf::Event::MouseButtonPressed:
            if (handleClick(event.mouseButton) == false){
                mWindow.close();
            }
            break;
        case sf::Event::Closed:
            mWindow.close();
            break;
        }
    }
}

bool Game::handleClick(sf::Event::MouseButtonEvent evt)
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
                
                // Hacer jugada IA
                int rojas = 0, negras = 0;
                for (int i = 0; i < 64; i++){
                    if (Tablero[i] == 1) rojas++;
                    else if (Tablero[i] == 2) negras++;
                }
                Tree *IATree = new Tree(Tablero, rojas, negras, IAProfundidad);
                if (IATree->calcular_jugada(Tablero) == false){
                    std::cout << "IA No tiene mas jugadas disponibles.\n";
                    return false;
                }
                delete IATree;

                // comprobar si el humano perdio
                rojas = 0; negras = 0;
                for (int i = 0; i < 64; i++){
                    if (Tablero[i] == 1) rojas++;
                    else if (Tablero[i] == 2) negras++;
                }
                if ((piezaJugador == 1 && rojas == 0) || (piezaJugador == 2 && negras == 0)){
                    std::cout << "El humano perdio.\n";
                    return false;
                }

                // comprobrar si le quedan movimientos al humano
                Tree *T = new Tree(Tablero,rojas,negras,IAProfundidad);
                T->insertHijos(T->root,1);
                if (T->root->hijos.size() == 0){
                    std::cout << "Humano No tiene mas jugadas disponibles.\n";
                    return false;
                }
                delete T;

                // preparar para la proxima jugada
                selectPiece = false;
                opciones.clear();
                break;
            }
        }
    }
    render();
    return true;
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

int main(int argc, char* argv[])
{
    if (argc < 3){
        std::cout << "Faltan argumentos. Intente con:\n";
        std::cout << "./damas [profundidad] [jugador inicial: humano|IA]\n";
        std::cout << "Ejem: ./damas 3 humano\n";
        std::cout << "Ejem: ./damas 4 IA\n";
    }
    std::string player(argv[2]);
    bool inicia_el_humano = true;
    if (player == "humano")
        inicia_el_humano = true;
    else if (player == "IA")
        inicia_el_humano = false;
    
    Game game(atoi(argv[1]),inicia_el_humano);
    game.run();
    
    return 0;
}