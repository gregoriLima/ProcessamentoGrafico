#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

/* ===== CONSTANTES ===== */

const int WIDTH = 1000;
const int HEIGHT = 700;

const float PASSO_ROTACAO = 5.0f;
const float PASSO_ESCALA = 15.0f;
const float PASSO_MOVIMENTO = 10.0f;
const float ESCALA_MIN = 80.0f;
const float ESCALA_MAX = 420.0f;

/* ===== ESTADO DA APLICACAO ===== */

int filtroAtual = 0;
int adesivoAtual = 0;
int flipHorizontal = 0;
int flipVertical = 0;

float adesivoX = WIDTH * 0.50f;
float adesivoY = HEIGHT * 0.48f;
float adesivoRotacao = 180.0f;
float adesivoEscala = 220.0f;

/* ===== FONTE BITMAP 5x7 PARA TEXTO NA TELA ===== */

static const unsigned char fonte[59][7] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x04,0x04,0x04,0x04,0x04,0x00,0x04},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x02,0x04,0x08,0x08,0x08,0x04,0x02},
    {0x08,0x04,0x02,0x02,0x02,0x04,0x08},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x04,0x04,0x1F,0x04,0x04,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x04,0x08},
    {0x00,0x00,0x00,0x1F,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C},
    {0x01,0x02,0x04,0x04,0x04,0x08,0x10},
    {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E},
    {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E},
    {0x0E,0x11,0x01,0x02,0x04,0x08,0x1F},
    {0x0E,0x11,0x01,0x06,0x01,0x11,0x0E},
    {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02},
    {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E},
    {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E},
    {0x1F,0x01,0x02,0x04,0x08,0x08,0x08},
    {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E},
    {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C},
    {0x00,0x0C,0x0C,0x00,0x0C,0x0C,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x0E,0x11,0x01,0x02,0x04,0x00,0x04},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11},
    {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E},
    {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E},
    {0x1C,0x12,0x11,0x11,0x11,0x12,0x1C},
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F},
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10},
    {0x0E,0x11,0x10,0x17,0x11,0x11,0x0E},
    {0x11,0x11,0x11,0x1F,0x11,0x11,0x11},
    {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E},
    {0x07,0x02,0x02,0x02,0x02,0x12,0x0C},
    {0x11,0x12,0x14,0x18,0x14,0x12,0x11},
    {0x10,0x10,0x10,0x10,0x10,0x10,0x1F},
    {0x11,0x1B,0x15,0x15,0x11,0x11,0x11},
    {0x11,0x11,0x19,0x15,0x13,0x11,0x11},
    {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E},
    {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10},
    {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D},
    {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11},
    {0x0E,0x11,0x10,0x0E,0x01,0x11,0x0E},
    {0x1F,0x04,0x04,0x04,0x04,0x04,0x04},
    {0x11,0x11,0x11,0x11,0x11,0x11,0x0E},
    {0x11,0x11,0x11,0x11,0x0A,0x0A,0x04},
    {0x11,0x11,0x11,0x15,0x15,0x15,0x0A},
    {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11},
    {0x11,0x11,0x0A,0x04,0x04,0x04,0x04},
    {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F}
};

/* ===== FUNCOES AUXILIARES ===== */

void resetarAdesivo() {
    adesivoX = WIDTH * 0.50f;
    adesivoY = HEIGHT * 0.48f;
    adesivoRotacao = 180.0f;
    adesivoEscala = 220.0f;
    flipHorizontal = 0;
    flipVertical = 0;
}

void mouse(double mx, double my) {
    adesivoX = (float)mx;
    adesivoY = (float)my;
}

const char* nomeFiltro(int filtro) {
    if (filtro == 0) return "NORMAL";
    if (filtro == 1) return "CINZA";
    if (filtro == 2) return "NEGATIVO";
    return "SEPIA";
}

const char* nomeAdesivo(int adesivo) {
    if (adesivo == 0) return "SKATE";
    return "PAREDE";
}

int modoDescarteAtual() {
    return 2;
}

int checarShader(GLuint shader, const char* nome) {
    GLint compilado = 0;
    char infoLog[512];

    glGetShaderiv(shader, GL_COMPILE_STATUS, &compilado);
    if (!compilado) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "ERRO DE COMPILACAO NO SHADER %s\n%s\n", nome, infoLog);
        return 0;
    }

    return 1;
}

int checarPrograma(GLuint programa, const char* nome) {
    GLint linkado = 0;
    char infoLog[512];

    glGetProgramiv(programa, GL_LINK_STATUS, &linkado);
    if (!linkado) {
        glGetProgramInfoLog(programa, 512, NULL, infoLog);
        fprintf(stderr, "ERRO DE LINK NO PROGRAMA %s\n%s\n", nome, infoLog);
        return 0;
    }

    return 1;
}

int carregarTexturaArquivo(const char* file_name, GLuint* tex) {
    int x, y, n;
    int force_channels = 4;
    unsigned char* image_data = stbi_load(file_name, &x, &y, &n, force_channels);

    if (!image_data) {
        return 0;
    }

    glGenTextures(1, tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, *tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    stbi_image_free(image_data);
    return 1;
}

void obterDiretorioFonte(char* diretorio, int tamanho) {
    int i;

    snprintf(diretorio, tamanho, "%s", __FILE__);

    for (i = (int)strlen(diretorio) - 1; i >= 0; i--) {
        if (diretorio[i] == '\\' || diretorio[i] == '/') {
            diretorio[i + 1] = '\0';
            return;
        }
    }

    diretorio[0] = '\0';
}

int carregarTexturaRelativaAoFonte(const char* caminhoRelativo, GLuint* tex) {
    char diretorioFonte[1024];
    char caminhoCompleto[1024];

    obterDiretorioFonte(diretorioFonte, sizeof(diretorioFonte));
    snprintf(caminhoCompleto, sizeof(caminhoCompleto), "%s%s", diretorioFonte, caminhoRelativo);

    if (carregarTexturaArquivo(caminhoCompleto, tex)) {
        printf("TEXTURA CARREGADA: %s\n", caminhoCompleto);
        return 1;
    }

    fprintf(stderr, "ERRO: NAO FOI POSSIVEL CARREGAR %s\n", caminhoCompleto);
    return 0;
}

void desenharTexto(GLuint program, GLuint VAO,
                   float px, float py, const char* texto,
                   float cr, float cg, float cb, float escala) {

    float ox = px;
    int i;

    for (i = 0; texto[i] != '\0'; i++) {
        int c = (int)texto[i];

        if (c < 32 || c > 90) {
            ox = ox + 6.0f * escala;
            continue;
        }

        int idx = c - 32;
        int linha, col;

        for (linha = 0; linha < 7; linha++) {
            unsigned char bits = fonte[idx][linha];

            for (col = 0; col < 5; col++) {
                if (bits & (0x10 >> col)) {
                    float xp = ox + col * escala;
                    float yp = py + linha * escala;

                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, glm::vec3(xp, yp, 0.0f));
                    model = glm::scale(model, glm::vec3(escala, escala, 1.0f));

                    glUniformMatrix4fv(glGetUniformLocation(program, "model"),
                                       1, GL_FALSE, glm::value_ptr(model));
                    glUniform4f(glGetUniformLocation(program, "cor"), cr, cg, cb, 1.0f);

                    glBindVertexArray(VAO);
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                    glBindVertexArray(0);
                }
            }
        }

        ox = ox + 6.0f * escala;
    }
}

/* ===== CALLBACK DE TECLADO ===== */

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) {
        return;
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, 1);
        return;
    }

    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        adesivoAtual = 0;
        return;
    }

    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        adesivoAtual = 1;
        return;
    }

    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        filtroAtual = filtroAtual + 1;
        if (filtroAtual > 3) {
            filtroAtual = 0;
        }
        return;
    }

    if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        flipHorizontal = 1 - flipHorizontal;
        return;
    }

    if (key == GLFW_KEY_V && action == GLFW_PRESS) {
        flipVertical = 1 - flipVertical;
        return;
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        resetarAdesivo();
        return;
    }

    if (key == GLFW_KEY_Q) {
        adesivoRotacao = adesivoRotacao - PASSO_ROTACAO;
        return;
    }

    if (key == GLFW_KEY_E) {
        adesivoRotacao = adesivoRotacao + PASSO_ROTACAO;
        return;
    }

    if (key == GLFW_KEY_Z) {
        adesivoEscala = adesivoEscala - PASSO_ESCALA;
        if (adesivoEscala < ESCALA_MIN) {
            adesivoEscala = ESCALA_MIN;
        }
        return;
    }

    if (key == GLFW_KEY_X) {
        adesivoEscala = adesivoEscala + PASSO_ESCALA;
        if (adesivoEscala > ESCALA_MAX) {
            adesivoEscala = ESCALA_MAX;
        }
        return;
    }

    if (key == GLFW_KEY_LEFT) {
        adesivoX = adesivoX - PASSO_MOVIMENTO;
        return;
    }

    if (key == GLFW_KEY_RIGHT) {
        adesivoX = adesivoX + PASSO_MOVIMENTO;
        return;
    }

    if (key == GLFW_KEY_UP) {
        adesivoY = adesivoY - PASSO_MOVIMENTO;
        return;
    }

    if (key == GLFW_KEY_DOWN) {
        adesivoY = adesivoY + PASSO_MOVIMENTO;
        return;
    }
}

/* ===== FUNCAO PRINCIPAL ===== */

int main() {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Tarefa Parte 4", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "ERRO: NAO FOI POSSIVEL CRIAR A JANELA\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "ERRO: FALHA AO INICIALIZAR GLAD\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const char* vertex_shader_textura =
        "#version 330 core\n"
        "layout (location = 0) in vec3 vp;\n"
        "layout (location = 1) in vec2 vt;\n"
        "uniform mat4 proj;\n"
        "uniform mat4 model;\n"
        "out vec2 text_map;\n"
        "void main(){\n"
        "  text_map = vt;\n"
        "  gl_Position = proj * model * vec4(vp, 1.0);\n"
        "}";

    const char* fragment_shader_textura =
        "#version 330 core\n"
        "in vec2 text_map;\n"
        "uniform sampler2D basic_texture;\n"
        "uniform int filtro;\n"
        "uniform int flip_horizontal;\n"
        "uniform int flip_vertical;\n"
        "uniform int descarte_modo;\n"
        "out vec4 frag_color;\n"
        "void main(){\n"
        "  vec2 uv = text_map;\n"
        "  if (flip_horizontal == 1) uv.x = 1.0 - uv.x;\n"
        "  if (flip_vertical == 1) uv.y = 1.0 - uv.y;\n"
        "  vec4 cor = texture(basic_texture, uv);\n"
        "  if (descarte_modo == 1) {\n"
        "    if (cor.a < 0.05) discard;\n"
        "    if (cor.r > 0.95 && cor.g > 0.95 && cor.b > 0.95) discard;\n"
        "  }\n"
        "  if (descarte_modo == 2) {\n"
        "    if (cor.a < 0.05) discard;\n"
        "    if (cor.r < 0.08 && cor.g < 0.08 && cor.b < 0.08) discard;\n"
        "  }\n"
        "  if (filtro == 1) {\n"
        "    float cinza = (cor.r + cor.g + cor.b) / 3.0;\n"
        "    cor.rgb = vec3(cinza, cinza, cinza);\n"
        "  } else if (filtro == 2) {\n"
        "    cor.rgb = vec3(1.0, 1.0, 1.0) - cor.rgb;\n"
        "  } else if (filtro == 3) {\n"
        "    vec3 sepia;\n"
        "    sepia.r = cor.r * 0.393 + cor.g * 0.769 + cor.b * 0.189;\n"
        "    sepia.g = cor.r * 0.349 + cor.g * 0.686 + cor.b * 0.168;\n"
        "    sepia.b = cor.r * 0.272 + cor.g * 0.534 + cor.b * 0.131;\n"
        "    cor.rgb = clamp(sepia, 0.0, 1.0);\n"
        "  }\n"
        "  frag_color = cor;\n"
        "}";

    const char* vertex_shader_cor =
        "#version 330 core\n"
        "layout (location = 0) in vec3 vp;\n"
        "uniform mat4 proj;\n"
        "uniform mat4 model;\n"
        "void main(){\n"
        "  gl_Position = proj * model * vec4(vp, 1.0);\n"
        "}";

    const char* fragment_shader_cor =
        "#version 330 core\n"
        "uniform vec4 cor;\n"
        "out vec4 FragColor;\n"
        "void main(){\n"
        "  FragColor = cor;\n"
        "}";

    GLuint vsTextura = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsTextura, 1, &vertex_shader_textura, NULL);
    glCompileShader(vsTextura);
    if (!checarShader(vsTextura, "VERTEX TEXTURA")) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    GLuint fsTextura = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fsTextura, 1, &fragment_shader_textura, NULL);
    glCompileShader(fsTextura);
    if (!checarShader(fsTextura, "FRAGMENT TEXTURA")) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    GLuint programaTextura = glCreateProgram();
    glAttachShader(programaTextura, vsTextura);
    glAttachShader(programaTextura, fsTextura);
    glLinkProgram(programaTextura);
    if (!checarPrograma(programaTextura, "TEXTURA")) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    GLuint vsCor = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsCor, 1, &vertex_shader_cor, NULL);
    glCompileShader(vsCor);
    if (!checarShader(vsCor, "VERTEX COR")) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    GLuint fsCor = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fsCor, 1, &fragment_shader_cor, NULL);
    glCompileShader(fsCor);
    if (!checarShader(fsCor, "FRAGMENT COR")) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    GLuint programaCor = glCreateProgram();
    glAttachShader(programaCor, vsCor);
    glAttachShader(programaCor, fsCor);
    glLinkProgram(programaCor);
    if (!checarPrograma(programaCor, "COR")) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,   0.0f, 1.0f,
         0.5f, -0.5f, 0.0f,   1.0f, 1.0f,
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f,
        -0.5f, -0.5f, 0.0f,   0.0f, 1.0f,
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f,
        -0.5f,  0.5f, 0.0f,   0.0f, 0.0f
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    GLuint texturaFundo;
    GLuint texturaAdesivo1;
    GLuint texturaAdesivo2;

    if (!carregarTexturaRelativaAoFonte("../assets/tex/pixelWall.png", &texturaFundo)) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    if (!carregarTexturaRelativaAoFonte("../assets/Modelos3D/unnamed.png", &texturaAdesivo1)) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glm::mat4 proj = glm::ortho(0.0f, (float)WIDTH, (float)HEIGHT, 0.0f, -1.0f, 1.0f);

    printf("=== TAREFA PARTE 4 - ADESIVOS TEXTURIZADOS ===\n");
    printf("MOUSE ESQUERDO: move o adesivo\n");
    printf("Q/E: rotacao\n");
    printf("Z/X: escala\n");
    printf("SETAS: movimento fino\n");
    printf("H/V: flip horizontal e vertical\n");
    printf("F: troca filtro\n");
    printf("1/2: troca a textura do adesivo\n");
    printf("R: reset do adesivo\n");
    printf("ESC: sair\n\n");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            double mx, my;
            glfwGetCursorPos(window, &mx, &my);
            mouse(mx, my);
        }

        char titulo[256];
        sprintf(titulo,
                "PARTE 4 - FILTRO: %s - ADESIVO: %s",
                nomeFiltro(filtroAtual), nomeAdesivo(adesivoAtual));
        glfwSetWindowTitle(window, titulo);

        glViewport(0, 0, WIDTH, HEIGHT);
        glClearColor(0.06f, 0.06f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(programaTextura);
        glUniformMatrix4fv(glGetUniformLocation(programaTextura, "proj"),
                           1, GL_FALSE, glm::value_ptr(proj));

        glm::mat4 modelFundo = glm::mat4(1.0f);
        modelFundo = glm::translate(modelFundo, glm::vec3(WIDTH * 0.5f, HEIGHT * 0.5f, 0.0f));
        modelFundo = glm::scale(modelFundo, glm::vec3((float)WIDTH, (float)HEIGHT, 1.0f));
        glUniformMatrix4fv(glGetUniformLocation(programaTextura, "model"),
                           1, GL_FALSE, glm::value_ptr(modelFundo));

        glUniform1i(glGetUniformLocation(programaTextura, "filtro"), filtroAtual);
        glUniform1i(glGetUniformLocation(programaTextura, "flip_horizontal"), 0);
        glUniform1i(glGetUniformLocation(programaTextura, "flip_vertical"), 0);
        glUniform1i(glGetUniformLocation(programaTextura, "descarte_modo"), 0);
        glUniform1i(glGetUniformLocation(programaTextura, "basic_texture"), 0);

        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texturaFundo);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        GLuint texturaAdesivo = texturaAdesivo1;
        if (adesivoAtual == 1) {
            texturaAdesivo = texturaAdesivo2;
        }

        glm::mat4 modelAdesivo = glm::mat4(1.0f);
        modelAdesivo = glm::translate(modelAdesivo, glm::vec3(adesivoX, adesivoY, 0.0f));
        modelAdesivo = glm::rotate(modelAdesivo, glm::radians(adesivoRotacao), glm::vec3(0.0f, 0.0f, 1.0f));
        modelAdesivo = glm::scale(modelAdesivo, glm::vec3(adesivoEscala, adesivoEscala, 1.0f));
        glUniformMatrix4fv(glGetUniformLocation(programaTextura, "model"),
                           1, GL_FALSE, glm::value_ptr(modelAdesivo));

        glUniform1i(glGetUniformLocation(programaTextura, "filtro"), filtroAtual);
        glUniform1i(glGetUniformLocation(programaTextura, "flip_horizontal"), flipHorizontal);
        glUniform1i(glGetUniformLocation(programaTextura, "flip_vertical"), flipVertical);
        glUniform1i(glGetUniformLocation(programaTextura, "descarte_modo"), modoDescarteAtual());
        glUniform1i(glGetUniformLocation(programaTextura, "basic_texture"), 0);

        glBindTexture(GL_TEXTURE_2D, texturaAdesivo);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glUseProgram(programaCor);
        glUniformMatrix4fv(glGetUniformLocation(programaCor, "proj"),
                           1, GL_FALSE, glm::value_ptr(proj));

        glm::mat4 painel = glm::mat4(1.0f);
        painel = glm::translate(painel, glm::vec3(WIDTH * 0.5f, HEIGHT - 85.0f, 0.0f));
        painel = glm::scale(painel, glm::vec3((float)WIDTH, 170.0f, 1.0f));
        glUniformMatrix4fv(glGetUniformLocation(programaCor, "model"),
                           1, GL_FALSE, glm::value_ptr(painel));
        glUniform4f(glGetUniformLocation(programaCor, "cor"), 0.0f, 0.0f, 0.0f, 0.70f);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        char linha1[128];
        char linha2[128];
        char linha3[128];
        char linha4[128];
        char linha5[128];

        sprintf(linha1, "PARTE 4 ADESIVOS TEXTURIZADOS");
        sprintf(linha2, "MOUSE ESQ MOVE  Q/E ROTACAO  Z/X ESCALA");
        sprintf(linha3, "SETAS MOVEM  H/V FLIP  F FILTRO  R RESET");
        sprintf(linha4, "1/2 TROCA ADESIVO  ADESIVO: %s", nomeAdesivo(adesivoAtual));
        sprintf(linha5, "FILTRO: %s  ROT: %.0f  ESC: %.0f  H: %s  V: %s",
                nomeFiltro(filtroAtual),
                adesivoRotacao,
                adesivoEscala,
                flipHorizontal ? "SIM" : "NAO",
                flipVertical ? "SIM" : "NAO");

        desenharTexto(programaCor, VAO, 20.0f, HEIGHT - 150.0f, linha1, 1.0f, 1.0f, 1.0f, 2.5f);
        desenharTexto(programaCor, VAO, 20.0f, HEIGHT - 122.0f, linha2, 0.95f, 0.85f, 0.20f, 2.0f);
        desenharTexto(programaCor, VAO, 20.0f, HEIGHT - 98.0f, linha3, 0.95f, 0.85f, 0.20f, 2.0f);
        desenharTexto(programaCor, VAO, 20.0f, HEIGHT - 74.0f, linha4, 0.60f, 0.90f, 1.00f, 2.0f);
        desenharTexto(programaCor, VAO, 20.0f, HEIGHT - 50.0f, linha5, 0.75f, 1.00f, 0.75f, 2.0f);

        glfwSwapBuffers(window);
    }

    glDeleteTextures(1, &texturaFundo);
    glDeleteTextures(1, &texturaAdesivo1);
    glDeleteTextures(1, &texturaAdesivo2);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(programaTextura);
    glDeleteProgram(programaCor);
    glDeleteShader(vsTextura);
    glDeleteShader(fsTextura);
    glDeleteShader(vsCor);
    glDeleteShader(fsCor);

    glfwTerminate();
    return EXIT_SUCCESS;
}
