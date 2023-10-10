#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "raylib.h"
#include <string.h>
#define FPS 60
#define OPCOES 3
#define LINHAS 16
#define COLUNAS 24
#define MAXOBS 40
#define MAXMONSTROS 10
#define SCREENHEIGHT 800
#define SCREENWIDTH 1200

enum direcoes {Norte, Sul, Leste, Oeste};

typedef struct Jogador{
    Rectangle rec;
    char orientacao;
    int pontos;
    int vidas;
    Texture2D texturas[4];
    Texture2D textura;
}Jogador;

typedef struct Monstro{
    Rectangle rec;
    int sentido;
    bool ativo;
    int velx;
    int vely;

}Monstro;

typedef struct Mapa{

    char matriz[LINHAS][COLUNAS];
    Rectangle rec[LINHAS][COLUNAS];
    int qnt_monstros;
    int qnt_obs;
    int nivel;
    int monstros_mortos;

} Mapa;

typedef struct Obstaculo{
    Rectangle rec;
    Texture2D textura;
}Obstaculo;

typedef struct Espada{
    Rectangle rec;
    Texture2D texturas[4];
    Texture2D textura;
    bool ativa;
}Espada;

typedef struct Arquivo{
    char nome[40];
    int pontos;
    bool ativo;

}Arquivo;

typedef struct Nome{
    Rectangle caixa_de_texto;
    char nome_jogador[30];
    int letra;
    int contador;
    bool mouse;
}Nome;*/

typedef struct Menu{
    Font font;
    Texture2D texture_menu;
    Vector2 posicao_titulo;
    Vector2 posicoes[OPCOES];

}Menu;


static bool gameOver = false;
static bool jogo_terminou = false;
static bool pause = false;
static bool salva_jogador = false;


void LoadSoundEffects(Sound sounds[],Music* menu_music, Music* game_music){
    sounds[0] = LoadSound("./audio/Sword_Slash.wav"); // Slash sword
    sounds[1] = LoadSound("./audio/Enemy_Die.wav");  // Enemy die
    *menu_music = LoadMusicStream("./audio/intro.mp3"); // Intro music
    *game_music = LoadMusicStream("./audio/overworld.mp3");
}

void UnloadSoundEffects(Sound sounds[], Music* menu_music, Music* game_music){
     UnloadSound(sounds[0]);
     UnloadSound(sounds[1]);
     UnloadMusicStream(*menu_music);
     UnloadMusicStream(*game_music);

}

void unloadTextures(Texture2D textures[],Texture2D tex_monstros[], Jogador* jogador, Monstro monstros[],Espada* espada ){

    for(int i = 0; i < 4; i++){
        UnloadTexture(jogador -> texturas[i]);
        UnloadTexture(tex_monstros[i]);
        UnloadTexture(espada->texturas[i]);
    }
     UnloadTexture(textures[0]);
     UnloadTexture(textures[1]);
}

void salva_arquivo(Arquivo vetor[], Arquivo* add_jogador){
    FILE *fp;
    int i;
    for(i = 0; i< 5; i++){
        if(strcmp(vetor[i].nome,add_jogador->nome) == 0){
           if(vetor[i].pontos > add_jogador->pontos)
                break;
            else{
                vetor[i] = *add_jogador;
                break;
                }
           }
        if((i == 4) && (strcmp(vetor[i].nome,"Jogador") == 0))
            vetor[i] = *add_jogador;
        }

    fp = fopen("./Ranking/ranking.bin","w+b");
    if(fp != NULL){
        for(i = 0; i < 5; i++){
            if(fwrite(&vetor[i],sizeof(Arquivo),1,fp))
                printf("salvo com sucesso!");
            else
                printf("erro no salvamento dos dados");
        }
    }
    else
        printf( "erro na abertura do Arquivo");
    fclose(fp);
}

void ordena_ranking(Arquivo vetor[]){

    //ordernar posicoes;
    int i,n,posicao_maior;
    bool achou_maior;
    n = 0;
    Arquivo maior;
    while(n < 5){
        maior = vetor[n];
        achou_maior = false;
        posicao_maior = n;

        //procura a struct q armazena o maior numero de pontos.
        for(i = n; i< 5; i++ ){
            if(maior.pontos < vetor[i].pontos){
                maior = vetor[i];
                posicao_maior = i;
                achou_maior = true;
                }
            }
        // se o vetor[n] nao for o maior, entao é preciso trocar as posicoes no vetor.
        if(achou_maior){
            vetor[posicao_maior] = vetor[n]; // na posicao do maior entra o vetor[n] (fixado a cada iteracao do while).
            vetor[n] = maior; // vetor[n] passa a ser o player com mais pontos;
        }
        n++;
    }
}

void le_ranking(Arquivo vetor[]){

    FILE *fp1;

    fp1 = fopen("./Ranking/ranking.bin","r+b");
    if(fp1 != -1){
        if(fread(&vetor[0],sizeof(Arquivo),1,fp1) != 1 ){
            for(int i = 0; i < 5; i++){
                vetor[i].pontos = 0;
                strcpy(vetor[i].nome,"Jogador");
                fwrite(&vetor[i],sizeof(Arquivo),1,fp1);
          }
        }
        else{
            for(int i = 1; i< 5; i++){
            fread(&vetor[i],sizeof(Arquivo),1,fp1);
        }
      }
    }
      fclose(fp1);
}

void get_name(Mapa *mapa, Arquivo vetor[], Nome *nome, Arquivo* add_jogador,Jogador* jogador, Espada* espada, Monstro monstros[], Obstaculo obstaculos[],bool *show_menu){

     if (CheckCollisionPointRec(GetMousePosition(), nome->caixa_de_texto))
            nome->mouse = true;
    else
            nome->mouse = false;
        if (nome->mouse)
        {
            SetMouseCursor(MOUSE_CURSOR_IBEAM);
             nome->letra = GetCharPressed();
            while(nome->letra > 0){
                if (( nome->letra>= 32) && (nome->letra <= 125) && (nome->contador < 20)){
                    nome->nome_jogador[nome->contador] = (char)nome->letra;
                    nome->nome_jogador[nome->contador +1] = '\0';
                    nome->contador++;
              }
                nome->letra = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE))
            {
                nome->contador--;
                if (nome->contador < 0)
                    nome->contador = 0;
                nome->nome_jogador[nome->contador] = '\0';
            }
            restartGame(jogador,mapa,nome,add_jogador,espada,monstros,vetor,obstaculos,show_menu);
        }
        else
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
}

void restartGame(Jogador* jogador, Mapa* mapa,Nome* nome, Arquivo* add_jogador,Espada* espada, Monstro monstros[], Arquivo vetor[],Obstaculo obstaculos[],bool *show_menu){

    if(IsKeyPressed(KEY_ENTER)){
                strcpy(add_jogador->nome,nome->nome_jogador);
                add_jogador->pontos = jogador->pontos;
                salva_arquivo(vetor,add_jogador);
                ordena_ranking(vetor);
                gameOver = false;
                salva_jogador = false;
                *show_menu = true;
                jogador ->pontos = 0;
                mapa -> nivel = 1;
                iniciaJogo(mapa,jogador,monstros,espada,obstaculos,nome);
    }

}

void reinicia_posicoes(Mapa* mapa, Jogador* jogador, Monstro monstros[]){
    int i,j,contador;
    contador = 0;
    for (i = 0; i < LINHAS ;i++){
        for(j = 0; j < COLUNAS; j++){
            if(mapa -> matriz[i][j] == 'J'){
                jogador->rec.x = 50*j;
                jogador->rec.y = 50*i;
            }
            else if(mapa -> matriz[i][j] == 'M' && monstros[contador].ativo){
                    monstros[contador].rec.x = 50*j;
                    monstros[contador].rec.y = 50*i;
                    contador++;
            }
        }
    }
}

void iniciaMonstro(Mapa *mapa, Monstro monstros[]){

    int i,velx,vely;
    // define uma velocidade aleatoria p cada monstro

    for(i = 0; i < mapa -> qnt_monstros; i++ ){

        velx = GetRandomValue(-20,20);
        vely = GetRandomValue(-20,20);
        while(velx == 0 || vely == 0){

            velx = GetRandomValue(-20,20);
            vely = GetRandomValue(-20,20);

            }
        monstros[i].velx = velx;
        monstros[i].vely = vely;
        monstros[i].sentido = 1;
  }
}

void iniciaJogo(Mapa *mapa, Jogador* jogador, Monstro monstros[], Espada* espada, Obstaculo obstaculos[], Nome* nome){

    int i,j,num_obs,num_monstros;
    char arquivo[40],caracter;

    sprintf(arquivo,"./Mapas/mapa%d.txt", mapa -> nivel);
    num_monstros = 0;
    num_obs = 0;
    FILE *fp;
    fp = fopen(arquivo,"r");
    if(fp != NULL){
            for( i = 0; i < LINHAS; i++){
                for( j = 0; j < COLUNAS; j++){
                        if(fscanf(fp," %c",&caracter) != -1)
                            mapa->matriz[i][j] = caracter;
            }
        }
        fclose(fp);
    }
    else
        printf("erro ao abrir o Arquivo!");

    //define posicoes iniciais dos elementos do jogo.

    for(i = 0; i< LINHAS; i++){
        for(j = 0; j< COLUNAS; j++){
            caracter = mapa -> matriz[i][j];
            if( caracter == 'J'){
            jogador->rec = (Rectangle){50*j, 50*i,50,50};
            jogador->vidas = 3;
            jogador-> orientacao = 'S';
            jogador->textura = jogador->texturas[1];
            }
            else if ( caracter == 'M'){
                monstros[num_monstros].rec = (Rectangle){50*j, 50*i,50,50};
                monstros[num_monstros].ativo = true;
                num_monstros++;
                    }
            else if (caracter == 'O'){
                obstaculos[num_obs].rec = (Rectangle){50*j, 50*i,50,50};
                num_obs++;}

            }
        }
       espada->rec.width = 150;
       espada->rec.height = 150;
       mapa -> qnt_monstros = num_monstros;
       mapa -> qnt_obs = num_obs;
       mapa -> monstros_mortos = 0;
       nome->caixa_de_texto =(Rectangle){ SCREENWIDTH/2 - 200, 180, 470, 50};
       nome->contador = 0;
       nome->letra = 0;
       nome->mouse = false;
       iniciaMonstro(mapa, monstros);
    }

void iniciaTexturasJogo(Menu *dados_menu,Jogador* jogador, Texture2D textures[],Texture2D tex_monstros[], Espada* espada){

    char arquivo[40];
    int i;
    Image imagem_link;
    for( i = 0; i < 4; i++)
    {
        sprintf(arquivo,"./sprites_link/Link%d.png", i);
        jogador ->texturas[i] = LoadTexture(arquivo);
        sprintf(arquivo,"./texturas/Enemy%d.png", i);
        tex_monstros[i] = LoadTexture(arquivo);
        sprintf(arquivo,"./texturas/Attack%d.png",i);
        espada ->texturas[i] = LoadTexture(arquivo);
    }
    textures[0] = LoadTexture("./texturas/Ground.png");
    textures[1] = LoadTexture("./texturas/Obstacle.png");

    dados_menu -> posicao_titulo = (Vector2){SCREENWIDTH/2 - (float)MeasureText("ZIIL", 150)/2, 20};
    for(int i = 0; i < OPCOES; i++){
        dados_menu->posicoes[i] = (Vector2){(float)50,(float)250 + (100*i)};
    }
    dados_menu -> font = LoadFont("./fontes/alagard.png");
    imagem_link = LoadImage("./sprites_link/link1.png");
    ImageFormat(&imagem_link, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    ImageResizeNN(&imagem_link, 300, 300);
    dados_menu ->texture_menu = LoadTextureFromImage(imagem_link);
    SetWindowIcon(imagem_link);
    UnloadImage(imagem_link);


}

void checkCollision(Jogador* jogador, Obstaculo obstaculos[], Mapa* mapa){
    int i;

    if (jogador ->orientacao == 'L'){
        for(i = 0; i < mapa -> qnt_obs; i++){
            while(CheckCollisionRecs(jogador ->rec, obstaculos[i].rec)){
                jogador ->rec.x--;
                jogador ->rec.x = ceil(jogador ->rec.x);
            }
        }
    }
    if(jogador ->orientacao == 'O'){
        for(i = 0; i < mapa -> qnt_obs; i++){
            while(CheckCollisionRecs(jogador ->rec, obstaculos[i].rec)){
                jogador ->rec.x++;
                jogador ->rec.x = floor(jogador ->rec.x);
            }
        }
    }

    if(jogador ->orientacao == 'N'){
        for(i = 0; i < mapa -> qnt_obs; i++){
            while(CheckCollisionRecs(jogador ->rec, obstaculos[i].rec)){
                    jogador ->rec.y++;
                    jogador ->rec.y = floor(jogador ->rec.y);
            }
        }
    }

    if(jogador ->orientacao == 'S'){
         for(i = 0; i < mapa -> qnt_obs; i++){
            while(CheckCollisionRecs(jogador ->rec, obstaculos[i].rec)){
                    jogador ->rec.y--;
                    jogador ->rec.y = ceil(jogador ->rec.y);
            }
        }
    }
}

void checkScreenBoundaries(Jogador* jogador){

    if(jogador ->orientacao == 'L'){
        if( jogador ->rec.x + jogador ->rec.width >= SCREENWIDTH )
            jogador ->rec.x = SCREENWIDTH - jogador ->rec.width;
    }
    if(jogador ->orientacao == 'O'){
        if(jogador ->rec.x <= 0 )
           jogador ->rec.x = 0;
    }
      if(jogador ->orientacao == 'N'){
        if(jogador ->rec.y <= 50)
            jogador ->rec.y = 50;
    }

    if(jogador ->orientacao == 'S'){
        if(jogador ->rec.y + jogador ->rec.height >= SCREENHEIGHT)
            jogador ->rec.y = SCREENHEIGHT - jogador ->rec.height;
    }
}

void checkCollisionSword(Espada* espada, Monstro monstros[], Mapa* mapa, Jogador* jogador, Sound sounds[]){

    for(int i = 0; i < mapa -> qnt_monstros;i++){
        if(CheckCollisionRecs(monstros[i].rec, espada ->rec)){
            PlaySound(sounds[1]);
            monstros[i].rec.x = -10;
            monstros[i].rec.y = -10;
            monstros[i].rec.width = 0;
            monstros[i].rec.height = 0;
            monstros[i].ativo = false;
            mapa -> monstros_mortos++;
            jogador -> pontos += 50;

    }
  }
}

void checkMonstersAlive(Mapa* mapa, Jogador* jogador, Monstro monstros[],Espada* espada, Obstaculo obstaculos[], Nome* nome){

    if(mapa -> monstros_mortos == mapa -> qnt_monstros){
        mapa -> nivel++;
        iniciaJogo(mapa,jogador,monstros,espada,obstaculos,nome);
   }
}

void checkCollisionObs(Mapa* mapa, Monstro monstros[], int monster_index, Obstaculo obstaculos[]){

    for(int i =0; i < mapa -> qnt_obs;i++){
            if(CheckCollisionRecs(monstros[monster_index].rec, obstaculos[i].rec)){
                    if((monstros[monster_index].sentido == Norte) || (monstros[monster_index].sentido == Sul)){
                            monstros[monster_index].vely *= -1;
                            monstros[monster_index].rec.y += monstros[monster_index].vely;
                            if(monstros[monster_index].sentido == Norte)
                                monstros[monster_index].sentido == Sul;
                    }
                    else{
                        monstros[monster_index].velx *= -1;
                        monstros[monster_index].rec.x += monstros[monster_index].velx;
                            if(monstros[monster_index].sentido == Leste)
                                    monstros[monster_index].sentido == Oeste;
            }
        }
    }
}

void mov_jogador(Mapa* mapa, Jogador* jogador, Espada* espada,Obstaculo obstaculos[], Monstro monstros[], Sound sounds[], Music* game_music){
    int i;

if(!gameOver){

    if(!jogo_terminou){
        if(IsKeyPressed(KEY_P)){
            pause = !pause;
            checkMusicPlaying(game_music);
        }
      if(!pause){

        if(IsKeyPressed(KEY_RIGHT)){
           jogador ->orientacao = 'L';
           jogador -> rec.x += (float)30;
           jogador -> textura = jogador ->texturas[2];
           checkCollision(jogador,obstaculos,mapa);
        }
        checkScreenBoundaries(jogador);

        if(IsKeyPressed(KEY_LEFT)){
           jogador ->orientacao = 'O';
           jogador -> rec.x -= (float)30;
           jogador -> textura = jogador -> texturas[3];
           checkCollision(jogador,obstaculos,mapa);
        }
        checkScreenBoundaries(jogador);

        if(IsKeyPressed(KEY_UP)){
           jogador ->orientacao = 'N';
           jogador -> rec.y -= (float)30;
           jogador -> textura = jogador -> texturas[0];
           checkCollision(jogador,obstaculos,mapa);
        }
        checkScreenBoundaries(jogador);

        if(IsKeyPressed(KEY_DOWN)){
           jogador ->orientacao = 'S';
           jogador -> rec.y += (float)30;
           jogador -> textura = jogador -> texturas[1];
           checkCollision(jogador,obstaculos,mapa);
           }
           checkScreenBoundaries(jogador);

        swordLogic(espada,jogador,mapa,sounds,monstros);
        playerLifes(jogador,mapa,monstros);
        }
    }

    else{

        checkMusicPlaying(game_music);
        if(IsKeyPressed(KEY_ENTER))
                jogo_terminou = false;
                PlayMusicStream(*game_music);
        }
   }
   else{
        checkMusicPlaying(game_music);
        if(IsKeyPressed(KEY_T)){
            salva_jogador = true;
            //strcpy(add_jogador.nome,"");
          }
        }
}

void swordLogic(Espada* espada, Jogador* jogador, Mapa* mapa, Sound sounds[], Monstro monstros[]){

    if(IsKeyDown(KEY_J)){
         PlaySound(sounds[0]);

        if(jogador ->orientacao == 'S'){
             espada->ativa = true;
             espada->textura = espada->texturas[1];
             espada->rec.x = jogador ->rec.x;
             espada->rec.y = jogador ->rec.y + 50;
        }
        else
            if(jogador ->orientacao == 'N'){
                espada->ativa = true;
                espada->textura = espada->texturas[0];
                espada->rec.x = jogador ->rec.x;
                espada->rec.y = jogador ->rec.y - 50;
        }
            else
                if(jogador ->orientacao == 'L'){
                    espada->ativa = true;
                    espada->textura = espada->texturas[2];
                    espada->rec.x = jogador ->rec.x + 50;
                    espada->rec.y = jogador ->rec.y;
        }
                else{
                    espada->ativa = true;
                    espada->textura = espada ->texturas[3];
                    espada->rec.x = jogador ->rec.x - 50;
                    espada->rec.y = jogador ->rec.y;
                    }

        checkCollisionSword(espada,monstros,mapa,jogador,sounds);

        }
        else{
            espada->ativa = false;
            espada->rec.x = -10;
            espada->rec.y = -10;
            espada->rec.width = 0;
            espada->rec.width = 0;
        }
}

void playerLifes(Jogador* jogador, Mapa* mapa, Monstro monstros[]){

     for(int i= 0 ; i < mapa -> qnt_monstros;i++){
            if(CheckCollisionRecs(jogador ->rec,monstros[i].rec)){
                    if (jogador -> vidas > 1){
                        jogador -> vidas--;
                        jogo_terminou = true;
                        reinicia_posicoes(mapa,jogador,monstros);
                    }
                    else
                        gameOver = true;
        }
    }
}

void movimento_monstros(Mapa *mapa, Monstro monstros[],Jogador* jogador,Espada* espada, Obstaculo obstaculos[],Nome* nome){

    int i,direcao;
    static int cont_frames=0;

    if(!pause){
        if(cont_frames == 10){
            cont_frames = 0;
        for(i = 0; i < mapa -> qnt_monstros; i++){
            if (monstros[i].ativo){
                 direcao = GetRandomValue(0,50);
                if(monstros[i].rec.x >= SCREENWIDTH - monstros[i].rec.width - monstros[i].velx || monstros[i].rec.x + monstros[i].velx <= 0)
                    monstros[i].velx *= -1;
                if(monstros[i].rec.y >= SCREENHEIGHT - monstros[i].rec.height - monstros[i].vely  || monstros[i].rec.y + monstros[i].vely  <= 50)
                    monstros[i].vely *= -1;

                    if(direcao > 20){
                        monstros[i].rec.x += monstros[i].velx;
                        if(monstros[i].velx> 0)
                            monstros[i].sentido  = 2; //aloca sprite leste
                        else
                            monstros[i].sentido  = 3;// aloca sprite oeste
                    }
                    else{
                        monstros[i].rec.y += monstros[i].vely;
                        if(monstros[i].vely > 0)
                            monstros[i].sentido  = 1;//aloca sprite sul
                        else
                              monstros[i].sentido  = 0;//aloca sprite norte
                        }
            checkCollisionObs(mapa,monstros,i,obstaculos);
        }
      }
    }
   checkMonstersAlive(mapa,jogador,monstros,espada,obstaculos,nome);
  cont_frames++;
  }
 }

void desenha_jogo(Mapa *mapa, Arquivo vetor[], Nome* nome,Arquivo* add_jogador, Texture2D textures[], Texture2D tex_monstros[],Jogador* jogador, Espada* espada, Monstro monstros[],Obstaculo obstaculos[],bool* show_menu){
    int i,j;
    char dados_game[30];
    BeginDrawing();
          ClearBackground(RAYWHITE);
          if(!gameOver){
                if(!jogo_terminou){
            for(i = 0; i < LINHAS; i++){
                for(j = 0; j < COLUNAS; j++){
                    if(mapa -> matriz[i][j] != 'O')
                        DrawTexture(textures[0], 50*j,50*i,RAYWHITE);// desenha fundo
                    else
                        DrawTexture(textures[1], 50*j, 50*i,RAYWHITE);// desenha obstaculos
                }
            }

                DrawTexture(jogador -> textura, jogador->rec.x,jogador->rec.y, RAYWHITE);
                if(espada -> ativa)
                    DrawTexture(espada->textura, espada->rec.x,espada->rec.y, RAYWHITE);

                for( i = 0; i < mapa -> qnt_monstros; i++){
                    if(monstros[i].ativo){
                        if(monstros[i].sentido == Norte)
                            DrawTexture(tex_monstros[0], monstros[i].rec.x, monstros[i].rec.y, WHITE);
                        else if(monstros[i].sentido == Sul)
                                DrawTexture(tex_monstros[1], monstros[i].rec.x, monstros[i].rec.y, WHITE);
                                else if(monstros[i].sentido ==  Leste )
                                    DrawTexture(tex_monstros[2], monstros[i].rec.x, monstros[i].rec.y, WHITE);
                                    else
                                        DrawTexture(tex_monstros[3], monstros[i].rec.x, monstros[i].rec.y, WHITE);
                }
            }

            if (pause){
                DrawText("Jogo Pausado", SCREENWIDTH/2 - MeasureText("Jogo Pausado", 40)/2, SCREENHEIGHT/2 - 40, 40, BLACK);
            }
            sprintf(dados_game, "Nivel: %d \t\t Vidas: %d \t\tPontos: %d", mapa -> nivel, jogador->vidas,jogador->pontos);
            DrawRectangle(0, 0, 1200, 50, BLACK);
            DrawText(dados_game, 5, 10, 30, WHITE);
            DrawText("p - pause",1000,10,30, WHITE);

        }
        else{
            ClearBackground(BLACK);
            DrawText("Pressione [ENTER] para continuar", SCREENWIDTH/2 - MeasureText("Pressione [ENTER] para continuar", 30)/2, SCREENHEIGHT/2 - 30, 30, LIGHTGRAY);

        }
     }
     else{
            if(salva_jogador){
                    ClearBackground(BLACK);
                    get_name(mapa,vetor,nome,add_jogador,jogador,espada,monstros,obstaculos,show_menu);
                    DrawText("Clique na caixa de texto", 410, 115, 35, LIGHTGRAY);
                    DrawText("Digite seu nome:", 150, 190, 30, LIGHTGRAY);
                    DrawRectangleRec(nome->caixa_de_texto, LIGHTGRAY);
                    if (nome->mouse)
                        DrawRectangleLines((int)nome->caixa_de_texto.x, (int)nome->caixa_de_texto.y, (int)nome->caixa_de_texto.width, (int)nome->caixa_de_texto.height, RED);
                    else
                        DrawRectangleLines((int)nome->caixa_de_texto.x, (int)nome->caixa_de_texto.y, (int)nome->caixa_de_texto.width, (int)nome->caixa_de_texto.height, RAYWHITE);

                    DrawText(nome->nome_jogador, (int)nome->caixa_de_texto.x + 5, (int)nome->caixa_de_texto.y + 8, 40, BLACK);
                    DrawText(TextFormat(" %i/%i", nome->contador, 20), 600, 240, 20, LIGHTGRAY);
                    if (nome->mouse){
                            if (nome->contador >= 20)
                            DrawText("Pressione [BACKSPACE] para deletar caracteres", 400, 300, 20, GRAY);
                    }
                }
            else{
                ClearBackground(BLACK);
                DrawText("GAME OVER", SCREENWIDTH/2 - MeasureText("GAME OVER", 80)/2, SCREENHEIGHT/2 - 80, 80, LIGHTGRAY);
                DrawText("Pressione T para continuar", SCREENWIDTH/2 - MeasureText("Pressione T para continuar", 20)/2, SCREENHEIGHT/2 + 100 , 20, LIGHTGRAY);
            }

  }
  EndDrawing();
}

void setupWindow(void){

    InitWindow(SCREENWIDTH, SCREENHEIGHT, "ZIIL");
    SetTargetFPS(FPS);
}

int menu(Menu *dados_menu, Music* menu_music){

    int contador_setas = 0;
    char *opcoes_menu[3] = {"Jogar","Ranking","Sair"};
    PlayMusicStream(*menu_music);
    //Texture2D img_fundo = LoadTexture("./texturas/fundo_menu.png");

    while(!IsKeyPressed(KEY_ENTER)){
        UpdateMusicStream(*menu_music);
        if (IsKeyPressed(KEY_DOWN) && contador_setas < 2)
            contador_setas++;
        if (IsKeyPressed(KEY_UP) && contador_setas > 0)
            contador_setas--;

        BeginDrawing();
            ClearBackground(BLACK);
            for(int i = 0; i < OPCOES ; i++){
                //DrawTexture(img_fundo,150,80,WHITE);
                //desenha o link no menu
                DrawTexture(dados_menu ->texture_menu, 800,400, WHITE);
                // Desenha o Titulo
                DrawTextEx(dados_menu -> font,"ZIIL",dados_menu ->posicao_titulo,150,10,RAYWHITE);

                //Desenha as opcoes do Menu
                if (i == contador_setas)
                    DrawTextEx(dados_menu -> font , opcoes_menu[i], dados_menu ->posicoes[i], 50, 10, DARKGRAY);

                else
                    DrawTextEx(dados_menu -> font, opcoes_menu[i], dados_menu ->posicoes[i], 50, 10, RAYWHITE);
                }
            EndDrawing();
            }
    StopMusicStream(*menu_music);
    return contador_setas;
}

void updateJogo(bool *show_menu, Mapa* mapa, Arquivo vetor[],Nome* nome, Arquivo* add_jogador, Texture2D textures[],Texture2D tex_monstros[], Jogador* jogador, Espada* espada, Monstro monstros[], Obstaculo obstaculos[],Sound sounds[], Music* game_music){

    while(!IsKeyPressed(KEY_END) && !*show_menu ){
            UpdateMusicStream(*game_music);
            mov_jogador(mapa,jogador,espada,obstaculos,monstros,sounds,game_music);
            movimento_monstros(mapa,monstros,jogador,espada,obstaculos,nome);
            desenha_jogo(mapa,vetor,nome,add_jogador,textures,tex_monstros,jogador,espada,monstros,obstaculos,show_menu);
        }

    *show_menu = true;
}

void desenha_ranking(Arquivo vetor[]){
         char pontos[5];
         Vector2 posicoes_nome[5];
         Vector2 posicoes_pontos[5];
         for(int i = 0; i < 5;i++){
            posicoes_nome[i].x = (float)50;
            posicoes_nome[i].y = (float)250 + (100*i);
                        }
        for(int i = 0; i < 5;i++){
            posicoes_pontos[i].x = (float)500;
            posicoes_pontos[i].y = (float)265 + (100*i);
                        }

        BeginDrawing();
        ClearBackground(BLACK);

        for(int i = 0; i< 5;i++){
                DrawText("RANKING",150,100,50,RAYWHITE);
                DrawText(vetor[i].nome,posicoes_nome[i].x,posicoes_nome[i].y,50,LIGHTGRAY);
                sprintf(pontos, "%d", vetor[i].pontos);
                DrawText(pontos,posicoes_pontos[i].x,posicoes_pontos[i].y,30,LIGHTGRAY);

        }
        EndDrawing();
}

void checkMusicPlaying(Music* game_music){
    if(gameOver || jogo_terminou)
        StopMusicStream(*game_music);
    else
        ResumeMusicStream(*game_music);
    if(pause)
        PauseMusicStream(*game_music);
    else
        ResumeMusicStream(*game_music);
}

void firstScreen(void){

    static bool stop_intro = false;
    bool loading_game = true;
    int framesCounter = 0;
    int nextRec=0;
    int current_message = 0;
    char *messagesIntro[2] = {"Carregando jogo...","Pressione S para continuar"};

    if(!stop_intro){
            Image background = LoadImage("./texturas/LOFZ2.png");
            ImageFormat(&background, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
            ImageResize(&background,1000,700);
            Texture2D textura = LoadTextureFromImage(background);
            UnloadImage(background);

            while(loading_game){
                framesCounter++;

                if(framesCounter == 60){
                    framesCounter = 0;
                    nextRec++;
                    }
                if(nextRec >=7){
                    nextRec = 7;
                    current_message=1;
                    if(IsKeyPressed(KEY_S))
                        loading_game = false;
                }

                BeginDrawing();
                     ClearBackground(RAYWHITE);
                        DrawTexture(textura,125,0,RAYWHITE);
                        DrawRectangleLines(525,750,175,25,RED);
                            for(int i=0; i < nextRec;i++){
                                DrawRectangle(525+25*i,750,25,25,RED);
                                DrawText(messagesIntro[current_message],525,725,20,BLACK);
                            }


                EndDrawing();
        }
                stop_intro = true;
                UnloadTexture(textura);
   }
}

int main(){

   int menu_option;
   bool show_menu = true;
   Jogador jogador;
   Espada espada;
   Monstro monstros[MAXMONSTROS];
   Obstaculo obstaculos[MAXOBS];
   Menu dados_menu;
   Sound sounds[3];
   Music menu_music;
   Music game_music;
   Arquivo vetor[5];
   Arquivo add_jogador;
   Texture2D textures[2];
   Texture2D tex_monstros[4];
   Nome nome;
   Mapa mapa;
   mapa.nivel = 1;
   setupWindow();
   InitAudioDevice();
   LoadSoundEffects(sounds,&menu_music,&game_music);
   iniciaTexturasJogo(&dados_menu,&jogador,textures,tex_monstros,&espada);
   iniciaJogo(&mapa,&jogador,monstros,&espada,obstaculos,&nome);
   le_ranking(vetor);
   ordena_ranking(vetor);

    while(!WindowShouldClose()){

        firstScreen();

        if (show_menu){
        menu_option = menu(&dados_menu,&menu_music);
        show_menu = false;
        }

        switch(menu_option){
            case 0:
                PlayMusicStream(game_music);
                updateJogo(&show_menu,&mapa,vetor,&nome,&add_jogador,textures,tex_monstros,&jogador,&espada,monstros,obstaculos,sounds,&game_music);
                break;
            case 1:
                if(!IsKeyPressed(KEY_Z))
                    desenha_ranking(vetor);
                else
                    show_menu = true;
                break;
            case 2:
                 unloadTextures(textures,tex_monstros,&jogador,monstros,&espada);
                 UnloadSoundEffects(sounds,&menu_music,&game_music);
                 CloseAudioDevice();
                 CloseWindow();
                 break;
        }
    }
    return 0;
}
