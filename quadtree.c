#include "quadtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else

#include <GL/gl.h>     /* OpenGL functions */

#endif

unsigned int first = 1;
char desenhaBorda = 1;

QuadNode* newNode(int x, int y, int width, int height)
{
    QuadNode* n = malloc(sizeof(QuadNode));
    n->x = x;
    n->y = y;
    n->width = width;
    n->height = height;
    n->NW = n->NE = n->SW = n->SE = NULL;
    n->color[0] = n->color[1] = n->color[2] = 0;
    n->id = first++;
    return n;
}

QuadNode* scanImg(QuadNode* n, RGB (* pixels)[(int) n->width], float minDetail)
{
    //RGB (*pixels)[pic->width] = (RGB(*)[pic->width]) pic->img;
    float sumR = 0, sumG = 0, sumB = 0;
    int pixelAmt = n->width * n->height;


    for (int i = n->y; i < n->y + n->height; ++i)
    {
        for (int j = n->x; j < n->x + n->width; ++j)
        {
            sumR += pixels[i][j].r;
            sumG += pixels[i][j].g;
            sumB += pixels[i][j].b;
        }
    }
    n->color[0] = sumR / pixelAmt;
    n->color[1] = sumG / pixelAmt;
    n->color[2] = sumB / pixelAmt;


    float dif = 0;
    for (int i = n->y; i < n->y + n->height; ++i)
    {
        for (int j = n->x; j < n->x + n->width; ++j)
        {
            dif += sqrtf(
                    pow(pixels[i][j].r - n->color[0], 2) +
                    pow(pixels[i][j].g - n->color[1], 2) +
                    pow(pixels[i][j].b - n->color[2], 2));
        }
    }
    dif /= pixelAmt;

    if (dif <= minDetail)
    {
        int newW = n->width / 2;
        int newH = n->height / 2;
        n->NW = scanImg(newNode(n->x, n->y, newW, newH), pixels, minDetail);
        n->NE = scanImg(newNode(n->x + newW, n->y, n->width - newW, newH), pixels, minDetail);
        n->SW = scanImg(newNode(n->x, n->y + newH, newW, n->height - newH), pixels, minDetail);
        n->SE = scanImg(newNode(n->x + newW, n->y + newH, n->width - newW, n->height - newH), pixels, minDetail);
        n->status = PARCIAL;
    } else
    {
        n->status = CHEIO;
    }
    return n;
}

QuadNode* geraQuadtree(Img* pic, float minDetail)
{
    if (minDetail < 0)
    {
        printf("minDetail deve ser no mínimo 0.");
        return NULL;
    }

    // Converte o vetor RGB para uma MATRIZ que pode acessada por pixels[linha][coluna]

    // Veja como acessar os primeiros 10 pixels da imagem, por exemplo:

    int width = pic->width;
    int height = pic->height;

    //////////////////////////////////////////////////////////////////////////
    // Implemente aqui o algoritmo que gera a quadtree, retornando o nodo raiz
    //////////////////////////////////////////////////////////////////////////
    QuadNode* raiz = scanImg(newNode(0, 0, width, height), pic, minDetail);

// COMENTE a linha abaixo quando seu algoritmo ja estiver funcionando
// Caso contrario, ele ira gerar uma arvore de teste com 3 nodos

#define DEMO
#ifdef DEMO

    /************************************************************/
    /* Teste: criando uma raiz e dois nodos a mais              */
    /************************************************************/

    raiz = newNode(0, 0, width, height);
    raiz->status = PARCIAL;
    raiz->color[0] = 0;
    raiz->color[1] = 0;
    raiz->color[2] = 255;

    QuadNode* nw = newNode(width / 2, 0, width / 2, height / 2);
    nw->status = PARCIAL;
    nw->color[0] = 0;
    nw->color[1] = 0;
    nw->color[2] = 255;

    // Aponta da raiz para o nodo nw
    raiz->NW = nw;

    QuadNode* nw2 = newNode(width / 2 + width / 4, 0, width / 4, height / 4);
    nw2->status = CHEIO;
    nw2->color[0] = 255;
    nw2->color[1] = 0;
    nw2->color[2] = 0;

    // Aponta do nodo nw para o nodo nw2
    nw->NW = nw2;

#endif
    // Finalmente, retorna a raiz da árvore
    return raiz;
}

// Limpa a memória ocupada pela árvore
void clearTree(QuadNode* n)
{
    if (n == NULL) return;
    if (n->status == PARCIAL)
    {
        clearTree(n->NE);
        clearTree(n->NW);
        clearTree(n->SE);
        clearTree(n->SW);
    }
    //printf("Liberando... %d - %.2f %.2f %.2f %.2f\n", n->status, n->x, n->y, n->width, n->height);
    free(n);
}

// Ativa/desativa o desenho das bordas de cada região
void toggleBorder()
{
    desenhaBorda = !desenhaBorda;
    printf("Desenhando borda: %s\n", desenhaBorda ? "SIM" : "NÃO");
}

// Desenha toda a quadtree
void drawTree(QuadNode* raiz)
{
    if (raiz != NULL)
        drawNode(raiz);
}

// Grava a árvore no formato do Graphviz
void writeTree(QuadNode* raiz)
{
    FILE* fp = fopen("quad.dot", "w");
    fprintf(fp, "digraph quadtree {\n");
    if (raiz != NULL)
        writeNode(fp, raiz);
    fprintf(fp, "}\n");
    fclose(fp);
    printf("\nFim!\n");
}

void writeNode(FILE* fp, QuadNode* n)
{
    if (n == NULL) return;

    if (n->NE != NULL) fprintf(fp, "%d -> %d;\n", n->id, n->NE->id);
    if (n->NW != NULL) fprintf(fp, "%d -> %d;\n", n->id, n->NW->id);
    if (n->SE != NULL) fprintf(fp, "%d -> %d;\n", n->id, n->SE->id);
    if (n->SW != NULL) fprintf(fp, "%d -> %d;\n", n->id, n->SW->id);
    writeNode(fp, n->NE);
    writeNode(fp, n->NW);
    writeNode(fp, n->SE);
    writeNode(fp, n->SW);
}

// Desenha todos os nodos da quadtree, recursivamente
void drawNode(QuadNode* n)
{
    if (n == NULL) return;

    glLineWidth(0.1);

    if (n->status == CHEIO)
    {
        glBegin(GL_QUADS);
        glColor3ubv(n->color);
        glVertex2f(n->x, n->y);
        glVertex2f(n->x + n->width - 1, n->y);
        glVertex2f(n->x + n->width - 1, n->y + n->height - 1);
        glVertex2f(n->x, n->y + n->height - 1);
        glEnd();
    } else if (n->status == PARCIAL)
    {
        if (desenhaBorda)
        {
            glBegin(GL_LINE_LOOP);
            glColor3ubv(n->color);
            glVertex2f(n->x, n->y);
            glVertex2f(n->x + n->width - 1, n->y);
            glVertex2f(n->x + n->width - 1, n->y + n->height - 1);
            glVertex2f(n->x, n->y + n->height - 1);
            glEnd();
        }
        drawNode(n->NE);
        drawNode(n->NW);
        drawNode(n->SE);
        drawNode(n->SW);
    }
    // Nodos vazios não precisam ser desenhados... nem armazenados!
}

