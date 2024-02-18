#include <glfw3.h>
#include <Stdio.h>
#include <windows.h>
#include <conio.h>
#include <time.h>

#include <stdlib.h>
#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define LENGTH 20
#define WIDTH 10

double time_Draw_fig = 0.0;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

typedef struct {
    char name[20];
    double vert[8];
}TButton;

char score[] = "000";

char txt[] = "0:00";

int w = 5; // размер блока

int field[LENGTH][WIDTH] = { 0 };

typedef struct {
    int x; int y;
}point;

point a[4];

int tex_textID;
int Fig[7][4] = {
    1, 3, 5, 7,
    2, 4, 5, 7,
    3, 5, 4, 6,
    3, 5, 4, 7,
    2, 3, 5, 7,
    3, 5, 7, 6,
    2, 3, 4, 5,
};

TButton btn[] = {
    {"START", {250.0,450.0, 750.0, 450.0, 750.0, 550.0, 250.0, 550.0}},
    {"PAUSE", {600.0, 600.0,  900.0, 600.0,  900.0, 500.0,  600.0, 500.0}},
    {"RESTART", { 600.0, 750.0,  900.0, 750.0,  900.0, 650.0,  600.0, 650.0 }},
    {"FINISH", { 600.0, 900.0,  900.0, 900.0,  900.0, 800.0,  600.0, 800.0 }},
    {"EXIT", {100.0, 900.0,  550.0, 900.0,   550.0, 800.0,   100.0,  800.0}}
};

int btnCnt = sizeof(btn) / sizeof(btn[0]);

int flag_start = 0;
int flag_pause = 0;
int flag_restart = 0;
int flag_finish = 0;
int flag_entered = 0;
int flag_exit = 0;

void TButton_Show(TButton btn, int k) { // MNZ FLAG убрал проверку лишних условий
    glEnableClientState(GL_VERTEX_ARRAY);
    switch (k)
    {
    case 0:
        glColor3f(1, 1, 1);
        break;
    case 1:
        glColor3f(1, 0, 0);
        break;
    default:
        break;
    }
    glVertexPointer(2, GL_DOUBLE, 0, btn.vert);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void Show_Menu(int k) {  // MNZ FLAG убрал проверку лишних условий
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, WIDTH * 10 * w + 500, LENGTH * 10 * w, 0, -1, 1);
    switch (k)
    {
    case 0:
        TButton_Show(btn[k], 0);
        break;
    case 1:
        for (int i = 1; i < btnCnt - 1; i++)
            TButton_Show(btn[i], 1);
        break;
    default:
        break;
    }
    glPopMatrix();
}

void Show_Final_Menu() {

    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, WIDTH * 10 * w + 500, LENGTH * 10 * w, 0, -1, 1);
    TButton_Show(btn[4], 0);
    glPopMatrix();

}

int PointInButton(double x, double y, TButton* btn) {
    if ((x > btn->vert[0]) && (x < btn->vert[4]) && (y > btn->vert[1]) && (y < btn->vert[5])) {
        //printf("POPAL\n");
        return (1);
    }
    else
        return (0);
}

void LoadTexture(char* file_name, int* target) {
    int width, height, cnt;
    unsigned char* data = stbi_load(file_name, &width, &height, &cnt, 0);
    glGenTextures(1, target);
    glBindTexture(GL_TEXTURE_2D, *target);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
        0, cnt == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
}






static float texCoords[512][4];

void initTextCoords() {
    const float charSize = 1.0f / 16.0f; //просчитываем для каждого выводимого символа смещение заранее <- алгоритмическая оптимизация
    for (int c = -16 * 16; c < 16 * 16; c++) {
        int y = c >> 4;
        int x = c & 0b1111;
        struct { float left, right, top, bottom; } rct;
        rct.left = x * charSize;
        rct.right = rct.left + charSize;
        rct.top = y * charSize;
        rct.bottom = rct.top + charSize;
        texCoords[c + 256][0] = rct.left;
        texCoords[c + 256][1] = rct.right;
        texCoords[c + 256][2] = rct.bottom;
        texCoords[c + 256][3] = rct.top;
    }
}

void Text_Out(int texture, char* txt) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPushMatrix();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    float rectCoord[] = { 0,0,  1,0,  1,1, 0,1 };
    float recttext[8] = { 0,1,  1,1,  1,0,  0,0 };

    glVertexPointer(2, GL_FLOAT, 0, rectCoord);
    glTexCoordPointer(2, GL_FLOAT, 0, recttext);

    while (*txt)
    {
        char c = *txt;
        recttext[0] = recttext[6] = texCoords[c + 256][0]; //берём просчитанные заранее значения
        recttext[2] = recttext[4] = texCoords[c + 256][1];
        recttext[1] = recttext[3] = texCoords[(c)+256][2];
        recttext[5] = recttext[7] = texCoords[(c)+256][3];
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        txt++;
        glTranslatef(1, 0, 0);
    }
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
}


void Show_Text(int k) { //MNZ убрал проверку лишних условий

    glPushMatrix();
    switch (k)
    {
    case 1:
        glTranslatef(-35, -5, 0);
        glScalef(15, 15, 0);
        glColor3f(1, 0, 0);
        Text_Out(tex_textID, "СТАРТ");
        break;
    case 2:
        glTranslatef(123, 57, 0);
        glScalef(8, 8, 0);
        glColor3f(1, 1, 1);
        Text_Out(tex_textID, "РЕСТАРТ");
        glTranslatef(0.75, 3.7, 0);
        glColor3f(1, 1, 1);
        Text_Out(tex_textID, "ПАУЗА");
        glTranslatef(0, -7.4, 0);
        glColor3f(1, 1, 1);
        Text_Out(tex_textID, "ВЫЙТИ");

        glTranslatef(-1.0, 17.55, 0);
        glColor3f(0, 0, 0);
        Text_Out(tex_textID, "CЧЁТ: ");
        glPopMatrix();
        break;
    default:
        break;
    }
    glPopMatrix();
}

void Show_Timer() {
    glPushMatrix();
    glTranslatef(135, 180, 0);
    glScalef(8, 8, 0);
    glColor3f(0, 0, 0);
    Text_Out(tex_textID, txt);

    glTranslatef(3, -1.5, 0);
    glColor3f(0, 0, 0);
    Text_Out(tex_textID, score);
    glPopMatrix();
}

void Show_Final_Text() {
    glPushMatrix();
    glTranslatef(-69.3, -76.5, 0);
    glScalef(15, 15, 0);
    glColor3f(1, 0, 0);
    Text_Out(tex_textID, "ВЫЙТИ");

    glTranslatef(0, 5, 0);
    glColor3f(1, 1, 1);
    Text_Out(tex_textID, "СЧЁТ: ");

    glTranslatef(5, 0, 0);
    glColor3f(1, 1, 1);
    Text_Out(tex_textID, score);

    glTranslatef(-5.2, 3, 0);
    glColor3f(1, 1, 1);
    Text_Out(tex_textID, "ВРЕМЯ: ");

    glTranslatef(6, 0, 0);
    glColor3f(1, 1, 1);
    Text_Out(tex_textID, txt);

    glPopMatrix();
}

void Text_Init() {
    LoadTexture("Verdana_B_alpha.png", &tex_textID);
    glBindTexture(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.99);
}

void Show_Game_Map() {
    glLineWidth(2.0);
    glColor3f(0, 0, 0);
    glBegin(GL_LINE_STRIP);
    glVertex2f(0, 1);
    glVertex2f(1, 1);
    glEnd();
    for (int i = 0; i < LENGTH * 10; i += 10) {
        glBegin(GL_LINE_STRIP);
        glVertex2f(0, i);
        glVertex2f(WIDTH * 10, i);
        glEnd();
    }
    for (int j = 0; j < WIDTH * 10 + 10; j += 10) {
        glBegin(GL_LINE_STRIP);
        glVertex2f(j, 0);
        glVertex2f(j, LENGTH * 10);
        glEnd();
    }
}

int Random(int min, int max)
{
    srand(time(NULL));
    int num = min + rand() % (max - min + 1);
    return num;
}

void New_Fig(point* a, int n) { //MNZ
    for (int j = 0; j < 4; j++) {
        a[j].x = 50 + (10 * (Fig[n][j] % 2));
        a[j].y = 180 + (10 * (Fig[n][j] / 2));
        //printf("%d, %d\n", a[j].x, a[j].y);
    }
    Draw_Fig(a, n);
}

int Draw_Fig(point* a, int n) { //MNZ
    for (int i = 0; i < 4; i++) {
        glBegin(GL_QUADS);
        if (n == 0) glColor3f(1, 0, 0);
        if (n == 1) glColor3f(0, 1, 0);
        if (n == 2) glColor3f(0, 0, 1);
        if (n == 3) glColor3f(1, 1, 0);
        if (n == 4) glColor3f(1, 0, 1);
        if (n == 5) glColor3f(0, 1, 1);
        if (n == 6) glColor3f(0.5, 0.5, 0.5);
        glVertex2f(a[i].x, a[i].y);
        glVertex2f(a[i].x + 10, a[i].y);
        glVertex2f(a[i].x + 10, a[i].y + 10);
        glVertex2f(a[i].x, a[i].y + 10);
        glEnd();
    }
}

int Check_Floor(point* a) {
    for (int i = 0; i < 4; i++) {
        if (a[i].y == 0)
            return (1);
    }
    return (0);
}

int Check_Wall(point* a) {
    for (int i = 0; i < 4; i++) {
        if (a[i].x == 0 || a[i].x == 90)
            return(1);
    }
    return(0);
}

int Check_Save_Fig(point* a, int f[LENGTH][WIDTH]) {
    int X, Y;
    for (int k = 0; k < 4; k++) {
        Y = a[k].y / 10;
        X = a[k].x / 10;
        if (f[Y - 1][X] == 1)
            return (1);
    }
    return (0);
}

int Check_Save_Fig_Wall(point* a, int f[LENGTH][WIDTH]) {
    int X, Y;
    for (int k = 0; k < 4; k++) {
        Y = a[k].y / 10;
        X = a[k].x / 10;
        if (f[Y][X + 1] == 1 || f[Y][X - 1] == 1)
            return (1);
    }
    return (0);
}

int Check_Ceiling(int f[LENGTH][WIDTH]) {
    for (int k = 0; k < 10; k++) {
        if (f[19][k] == 1)
            return (1);
    }
    return (0);
}

void Check_Line(int f[LENGTH][WIDTH]) { //MNZ развёртка цикла
    int line_flag = 0; int l = 0;
    for (int i = 0; i < 20; i += 2) {
        l = 0;
        for (int j = 0; j < 10; j++) {
            if (f[i][j] == 1)
                l++;
        }
        int k = i;
        if (l == 10) {
            for (k; k < 19; k++) {
                for (int j = 0; j < 10; j++) {
                    f[k][j] = f[k + 1][j];
                }
            }
        }
        l = 0;
        for (int j = 0; j < 10; j++) {
            if (f[i + 1][j] == 1)
                l++;
        }
        k = i + 1;
        if (l == 10) {
            for (k; k < 19; k++) {
                for (int j = 0; j < 10; j++) {
                    f[k][j] = f[k + 1][j];
                }
            }
        }
    }
}

void Check_Wall_Perev(point* a) {
    for (int i = 0; i < 4; i++) {
        if (a[i].x == 0 || a[i].x == 80)
            return(1);
    }
    return(0);
}

int Check_Wall_Left(point* a) {
    for (int i = 0; i < 4; i++) {
        if (a[i].x == 0)
            return(1);
    }
    return(0);
}

int Check_Wall_Right(point* a) {
    for (int i = 0; i < 4; i++) {
        if (a[i].x == 90)
            return(1);
    }
    return(0);
}

void Control_Fig(point* a, int n, int f[LENGTH][WIDTH], char left, char right, char circle, char Speed) { //MNZ объединение условий и развёртка цикла
    point b[4] = { 0 };
    int X, Y;
    if (GetKeyState(left) < 0 && !Check_Save_Fig_Wall(a, f) && !Check_Wall_Left(a)) {
        a[0].x -= 10;
        a[1].x -= 10;
        a[2].x -= 10;
        a[3].x -= 10;
    }
    if (GetKeyState(right) < 0 && !Check_Save_Fig_Wall(a, f) && !Check_Wall_Right(a)) {
        a[0].x += 10;
        a[1].x += 10;
        a[2].x += 10;
        a[3].x += 10;
    }
    if (GetKeyState(Speed) < 0) {
        if (!Check_Floor(a) && !Check_Save_Fig(a, f)) {
            a[0].y -= 10;
            a[1].y -= 10;
            a[2].y -= 10;
            a[3].y -= 10;
        }
        if (!Check_Floor(a) && !Check_Save_Fig(a, f)) {
            a[0].y += 10;
            a[1].y += 10;
            a[2].y += 10;
            a[3].y += 10;

        }
    }
    if (GetKeyState(circle) < 0) {
        point p = a[1];
        for (int i = 0; i < 4; i++) {
            b[i] = a[i];
            int x = a[i].y - p.y;
            int y = a[i].x - p.x;
            a[i].x = p.x - x;
            a[i].y = p.y + y;
            //printf("%d, %d\n", a[i].x, a[i].y);
        }
        if (Check_Floor(a) || Check_Save_Fig(a, f) || Check_Floor(a) || Check_Wall(a)) {
            for (int i = 0; i < 4; i++) {
                a[i].y = b[i].y;
                a[i].x = b[i].x;
            }
        }

    }
}

void Save_Fig(int f[LENGTH][WIDTH], int n) { //MNZ проверка лишних условий
    for (int i = 0; i < LENGTH; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (f[i][j] != 0) {
                switch (f[i][j])
                {
                case 1:
                    glColor3f(0.5, 0.5, 0.5);
                    break;
                case 2:
                    glColor3f(0, 1, 0);
                    break;
                case 3:
                    glColor3f(0, 0, 1);
                    break;
                case 4:
                    glColor3f(1, 1, 0);
                    break;
                case 5:
                    glColor3f(1, 0, 1);
                    break;
                case 6:
                    glColor3f(0, 1, 1);
                    break;
                case 7:
                    glColor3f(0.5, 0.5, 0.5);
                    break;
                default:
                    break;
                }
                glBegin(GL_QUADS);
                glVertex2f((j) * 10, (i) * 10);
                glVertex2f((j) * 10 + 10, (i) * 10);
                glVertex2f((j) * 10 + 10, (i) * 10 + 10);
                glVertex2f((j) * 10, (i) * 10 + 10);
                glEnd();
            }
        }
    }

}

void Move_Fig(point* a, int f[LENGTH][WIDTH], int n) { // MNZ развёртка цикла
    if (!Check_Floor(a) && !Check_Save_Fig(a, f)) {
        a[0].y -= 10;
        a[1].y -= 10;
        a[2].y -= 10;
        a[3].y -= 10;
    }
    Control_Fig(a, n, f, 'A', 'D', 'W', 'S');
    Draw_Fig(a, n);
}

void Timer_move(int time) {
    if (time < 10) {
        txt[3]++;
    }
    else if (time < 60) {
        if ((time % 10) == 0) {
            txt[2]++;
            txt[3] = 48;
        }
        else txt[3]++;
    }
    else {
        if ((time % 60) == 0) {
            txt[0]++;
            txt[2] = 48;
            txt[3] = 48;
        }
        else if ((time % 10) == 0) {
            txt[2]++;
            txt[3] = 48;
        }
        else txt[3]++;
    }
}

void Move_Score(int Tablo) {
    if (Tablo < 10) {
        score[2]++;
    }
    else if (Tablo < 100) {
        if ((Tablo % 10) == 0) {
            score[1]++;
            score[2] = 48;
        }
        else score[2]++;
    }
    else {
        if ((Tablo % 100) == 0) {
            score[0]++;
            score[1] = 48;
            score[2] = 48;
        }
        else score[2]++;
    }
}

//{"START", { 250.0,450.0, 750.0, 450.0, 750.0, 550.0, 250.0, 550.0 }},
//{ "PAUSE", {600.0, 600.0,  900.0, 600.0,  900.0, 500.0,  600.0, 500.0} },
//{ "RESTART", { 600.0, 750.0,  900.0, 750.0,  900.0, 650.0,  600.0, 650.0 } },
//{ "FINISH", { 600.0, 900.0,  900.0, 900.0,  900.0, 800.0,  600.0, 800.0 } }
//{ "EXIT", {100.0, 900.0,  550.0, 900.0,   550.0, 750.0,   100.0,  750.0}}    

void cursorPositionCallBack(GLFWwindow* window, double xPos, double yPos)
{
    if (xPos > 250 && yPos > 450 && xPos < 750 && yPos < 550) {
        flag_entered = 1;
    }
    if (xPos > 600 && yPos > 500 && xPos < 900 && yPos < 600) {
        flag_entered = 2;
        //printf("PAUSE\n");
    }
    if (xPos > 600 && yPos > 650 && xPos < 900 && yPos < 750) {
        flag_entered = 3;
        //printf("RESTART\n");
    }
    if (xPos > 600 && yPos > 800 && xPos < 900 && yPos < 900) {
        flag_entered = 4;
        //printf("FINISH\n");
    }
    if (xPos > 100 && yPos > 800 && xPos < 550 && yPos < 900) {
        flag_entered = 5;
        //printf("EXIT\n");
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (flag_entered == 1) {
            //printf("START\n");
            flag_start = 1;
            //printf("flag_start = %d\n", flag_start);
        }
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (flag_entered == 2) {
            /*          printf("PAUSE\n");*/
            flag_pause = 1;
            //printf("flag_pause = %d\n", flag_pause);
        }

    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (flag_entered == 3) {
            //printf("RESTART\n");
            flag_restart = 1;
            //printf("flag_restart = %d\n", flag_restart);
        }
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (flag_entered == 4) {
            //printf("FINISH\n");
            flag_finish = 1;
            //printf("flag_finish = %d\n", flag_finish);
        }
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (flag_entered == 5) {
            //printf("EXIT_OUT\n");
            flag_exit = 1;
            //printf("flag_finish = %d\n", flag_exit);
        }
    }
}

int main(void)
{
    clock_t start, stop, time_now;
    int f, g = 0;
    int l = 0;
    GLFWwindow* window;
    int i = 0, k = 0;
    int X, Y;
    int n;
    int Chislo_Fig = 1;
    int key = 0;

    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(WIDTH * 10 * w + 500, LENGTH * 10 * w, "Tetris", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwSetCursorPosCallback(window, cursorPositionCallBack);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwMakeContextCurrent(window);

    Text_Init();
    initTextCoords();
    int sch = 0;
    clock_t beg, end;
    beg = clock();
    while (!glfwWindowShouldClose(window))
    {
        start = clock();
        while (!flag_start) {
            glClear(GL_COLOR_BUFFER_BIT);
            glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
            glLoadIdentity();
            glScalef(0.01, 0.01, 1);
            Show_Menu(0);
            Show_Text(1);
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();
        glTranslatef(-1, -1, 0);
        glScalef(0.01, 0.01, 1);

        Show_Game_Map();
        Show_Menu(1);
        Show_Text(2);

        if (k == Chislo_Fig - 1)
        {
            n = Random(0, 6);
            New_Fig(&a, n);
        }
        Save_Fig(field, n);
        k = Chislo_Fig;
        if (Check_Floor(&a) || Check_Save_Fig(&a, field)) {
            Move_Score(Chislo_Fig);
            //printf("Chislo Fig: %d\n", Chislo_Fig);
            Chislo_Fig++;
            for (int i = 0; i < 4; i++) {
                X = a[i].x / 10;
                Y = a[i].y / 10;
                field[Y][X] = 1;
            }
            Check_Line(field);
        }
        Move_Fig(&a, field, n);





        if (flag_finish || Check_Ceiling(field)) {
            while (!flag_exit) {
                glClear(GL_COLOR_BUFFER_BIT);
                glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
                glLoadIdentity();
                glScalef(0.01, 0.01, 1);
                Show_Final_Menu();
                Show_Final_Text();
                glfwSwapBuffers(window);
                glfwPollEvents();
            }
            break;
        }


        Sleep(100);

        time_now = clock();
        f = (time_now - start) / CLK_TCK;
        //printf("%d, %d\n", f, g);
        if (f == g + 1) {
            Timer_move(f);
        }
        Show_Timer();

        g = f;
        glfwSwapBuffers(window);
        glfwPollEvents();
        if (sch == 200) {
            end = clock();
            printf("wait and timer : %.40f\n", (double)(end - beg) / CLOCKS_PER_SEC);
        }
        sch++;

    }
    stop = clock();
    f = (stop - start) / CLK_TCK;
    //printf("%d\n", f);

    glfwTerminate();
    return 0;
}
